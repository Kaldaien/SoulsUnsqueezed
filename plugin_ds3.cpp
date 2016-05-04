#include <string>

#include "ini.h"
#include "parameter.h"
#include "utility.h"

#include "log.h"
#include "config.h"
#include "core.h"

//
// Hook Special K's shutdown function
//
typedef bool (WINAPI *ShutdownPlugin_pfn)(const wchar_t *);
static ShutdownPlugin_pfn BMF_ShutdownCore_Original = nullptr;
extern "C" bool WINAPI SK_DS3_ShutdownPlugin (const wchar_t *);


typedef int (WINAPI *GetSystemMetrics_pfn)(
  _In_ int nIndex
);

typedef BOOL (WINAPI *EnumDisplaySettingsA_pfn)(
  _In_  LPCSTR    lpszDeviceName,
  _In_  DWORD     iModeNum,
  _Out_ DEVMODEA *lpDevMode
);

GetSystemMetrics_pfn     GetSystemMetrics_Original     = nullptr;
EnumDisplaySettingsA_pfn EnumDisplaySettingsA_Original = nullptr;

#include <d3d11.h>

typedef void (WINAPI *D3D11_RSSetViewports_pfn)(
        ID3D11DeviceContext*,
        UINT,
  const D3D11_VIEWPORT*
);

typedef HRESULT (STDMETHODCALLTYPE *DXGISwap_ResizeTarget_pfn)(
    _In_       IDXGISwapChain *,
    _In_ const DXGI_MODE_DESC *
);

typedef HRESULT (STDMETHODCALLTYPE *DXGISwap_ResizeBuffers_pfn)(
    _In_ IDXGISwapChain *,
    _In_ UINT,
    _In_ UINT,
    _In_ UINT,
    _In_ DXGI_FORMAT,
    _In_ UINT
);

typedef HRESULT (STDMETHODCALLTYPE *DXGISwap_GetFullscreenState_pfn)(
    _In_       IDXGISwapChain  *This,
    _Out_opt_  BOOL            *pFullscreen,
    _Out_opt_  IDXGIOutput    **ppTarget
);


typedef HRESULT (STDMETHODCALLTYPE *DXGISwap_SetFullscreenState_pfn)(
    _In_  IDXGISwapChain *This,
    _In_  BOOL            Fullscreen,
    _Out_ IDXGIOutput    *pTarget
);

static D3D11_RSSetViewports_pfn        D3D11_RSSetViewports_Original        = nullptr;

static DXGISwap_ResizeTarget_pfn       DXGISwap_ResizeTarget_Original       = nullptr;
static DXGISwap_ResizeBuffers_pfn      DXGISwap_ResizeBuffers_Original      = nullptr;
static DXGISwap_GetFullscreenState_pfn DXGISwap_GetFullscreenState_Original = nullptr;
static DXGISwap_SetFullscreenState_pfn DXGISwap_SetFullscreenState_Original = nullptr;


extern "C" void    WINAPI D3D11_RSSetViewports_Override     ( ID3D11DeviceContext*,
                                                              UINT,
                                                        const D3D11_VIEWPORT* );
extern "C" HRESULT WINAPI D3D11Dev_CreateTexture2D_Override ( ID3D11Device*,
                                                        const D3D11_TEXTURE2D_DESC*,
                                                        const D3D11_SUBRESOURCE_DATA*,
                                                              ID3D11Texture2D** );

extern "C" HRESULT STDMETHODCALLTYPE
  DXGISwap_ResizeTarget_Override ( IDXGISwapChain *,
                        _In_ const DXGI_MODE_DESC * );

extern "C" HRESULT STDMETHODCALLTYPE
  DXGISwap_ResizeBuffers_Override ( IDXGISwapChain *,
                               _In_ UINT,
                               _In_ UINT,
                               _In_ UINT,
                               _In_ DXGI_FORMAT,
                               _In_ UINT );

extern "C" HRESULT STDMETHODCALLTYPE
  DXGISwap_GetFullscreenState_Override (
    _In_       IDXGISwapChain  *This,
    _Out_opt_  BOOL            *pFullscreen,
    _Out_opt_  IDXGIOutput    **ppTarget );

extern "C" HRESULT STDMETHODCALLTYPE
  DXGISwap_SetFullscreenState_Override (
    _In_  IDXGISwapChain *This,
    _In_  BOOL            Fullscreen,
    _Out_ IDXGIOutput    *pTarget );


extern __declspec (noinline) void CALLBACK SK_PluginKeyPress (BOOL, BOOL, BOOL, BYTE);


void
WINAPI
SK_DS3_RSSetViewports ( ID3D11DeviceContext* This,
                        UINT                 NumViewports,
                  const D3D11_VIEWPORT*      pViewports );

HRESULT
WINAPI
SK_DS3_CreateTexture2D (
    _In_            ID3D11Device           *This,
    _In_      const D3D11_TEXTURE2D_DESC   *pDesc,
    _In_opt_  const D3D11_SUBRESOURCE_DATA *pInitialData,
    _Out_opt_       ID3D11Texture2D        **ppTexture2D );


HRESULT
STDMETHODCALLTYPE
SK_DS3_ResizeTarget (
    _In_       IDXGISwapChain *This,
    _In_ const DXGI_MODE_DESC *pNewTargetParameters );

HRESULT
STDMETHODCALLTYPE
SK_DS3_ResizeBuffers (
    _In_ IDXGISwapChain *This,
    _In_ UINT            BufferCount,
    _In_ UINT            Width,
    _In_ UINT            Height,
    _In_ DXGI_FORMAT     NewFormat,
    _In_ UINT            SwapChainFlags );

HRESULT
STDMETHODCALLTYPE
SK_DS3_GetFullscreenState (
    _In_       IDXGISwapChain  *This,
    _Out_opt_  BOOL            *pFullscreen,
    _Out_opt_  IDXGIOutput    **ppTarget );

HRESULT
STDMETHODCALLTYPE
SK_DS3_SetFullscreenState (
    _In_  IDXGISwapChain *This,
    _In_  BOOL            Fullscreen,
    _Out_ IDXGIOutput    *pTarget );



void
CALLBACK
SK_DS3_PluginKeyPress ( BOOL Control,
                        BOOL Shift,
                        BOOL Alt,
                        BYTE vkCode );


bmf::ParameterFactory ds3_factory;

bmf::INI::File*       ds3_prefs         = nullptr;

bmf::ParameterInt*    ds3_hud_res_x     = nullptr;
bmf::ParameterInt*    ds3_hud_res_y     = nullptr;
bmf::ParameterInt*    ds3_hud_offset_x  = nullptr;
bmf::ParameterInt*    ds3_hud_offset_y  = nullptr;
bmf::ParameterBool*   ds3_hud_stretch   = nullptr;

bmf::ParameterInt*    ds3_default_res_x  = nullptr;
bmf::ParameterInt*    ds3_default_res_y  = nullptr;
bmf::ParameterInt*    ds3_sacrificial_x  = nullptr;
bmf::ParameterInt*    ds3_sacrificial_y  = nullptr;

bmf::ParameterBool*   ds3_fullscreen    = nullptr;
bmf::ParameterBool*   ds3_borderless    = nullptr;
bmf::ParameterBool*   ds3_center        = nullptr;

bmf::ParameterBool*   ds3_flip_mode     = nullptr;


// VERY BAD (no) DESIGN, move this stuff completely into plugin_ds3.cpp
bool __DS3_FULLSCREEN = false;

bool __DS3_CENTER     = false;
bool __DS3_MAX_WINDOW = false;
HWND __DS3_WINDOW     = 0;

int  __DS3_WIDTH  = 0;
int  __DS3_HEIGHT = 0;

int  __DS3_MON_X = 0;
int  __DS3_MON_Y = 0;


struct {
  struct {
    int  res_x       = 1280;
    int  res_y       = 720;
    int  offset_x    = 0;
    int  offset_y    = 0;
    bool stretch     = false;
  } hud;

  struct {
    bool flip_mode   = false;
    int  res_x       = 1920;
    int  res_y       = 1080;

    //
    // Sacrificial resolution:  This will no longer be selectable in-game;
    //                            it will be replaced with the resolution:
    //                              (<res_x> x <res_y>).
    uint32_t
         sacrifice_x = 800;
    uint32_t
         sacrifice_y = 450;
  } render;

  struct {
    bool borderless  = true;
    bool fullscreen  = false;
    bool center      = true;
  } window;
} ds3_cfg;


#include "core.h"

extern void
__stdcall
SK_SetPluginName (std::wstring name);

#define SUS_VERSION_NUM L"0.2.0"
#define SUS_VERSION_STR L"Souls Unsqueezed v " SUS_VERSION_NUM

void*
SK_Scan (uint8_t* pattern, size_t len, uint8_t* mask)
{
  uint8_t* base_addr = (uint8_t *)GetModuleHandle (nullptr);

  MEMORY_BASIC_INFORMATION mem_info;
  VirtualQuery (base_addr, &mem_info, sizeof mem_info);

  IMAGE_DOS_HEADER* pDOS =
    (IMAGE_DOS_HEADER *)mem_info.AllocationBase;
  IMAGE_NT_HEADERS* pNT  =
    (IMAGE_NT_HEADERS *)((intptr_t)(pDOS + pDOS->e_lfanew));

  uint8_t* end_addr = base_addr + pNT->OptionalHeader.SizeOfImage;

  uint8_t*  begin = (uint8_t *)base_addr;
  uint8_t*  it    = begin;
  int       idx   = 0;

  while (it < end_addr)
  {
    VirtualQuery (it, &mem_info, sizeof mem_info);

    // Bail-out once we walk into an address range that is not resident, because
    //   it does not belong to the original executable.
    if (mem_info.RegionSize == 0)
      break;

    uint8_t* next_rgn =
     (uint8_t *)mem_info.BaseAddress + mem_info.RegionSize;

    if ( (! (mem_info.Type    & MEM_IMAGE))  ||
         (! (mem_info.State   & MEM_COMMIT)) ||
             mem_info.Protect & PAGE_NOACCESS ) {
      it    = next_rgn;
      idx   = 0;
      begin = it;
      continue;
    }

    // Do not search past the end of the module image!
    if (next_rgn >= end_addr)
      break;

    while (it < next_rgn) {
      uint8_t* scan_addr = it;

      bool match = (*scan_addr == pattern [idx]);

      // For portions we do not care about... treat them
      //   as matching.
      if (mask != nullptr && (! mask [idx]))
        match = true;

      if (match) {
        if (++idx == len)
          return (void *)begin;

        ++it;
      }

      else {
        // No match?!
        if (it > end_addr - len)
          break;

        it  = ++begin;
        idx = 0;
      }
    }
  }

  return nullptr;
}

BOOL
SK_InjectMemory ( LPVOID   base_addr,
                  uint8_t* new_data,
                  size_t   data_size,
                  DWORD    permissions,
                  uint8_t* old_data = nullptr
                )
{
  DWORD dwOld;

  if (VirtualProtect (base_addr, data_size, permissions, &dwOld))
  {
    if (old_data != nullptr)
      memcpy (old_data, base_addr, data_size);

    memcpy (base_addr, new_data, data_size);

    VirtualProtect (base_addr, data_size, dwOld, &dwOld);

    return TRUE;
  }

  return FALSE;
}

// This should be unnecessary on x86/x64 due to cache snooping, but
//   do it anyway for completeness.
void
SK_FlushInstructionCache ( LPCVOID base_addr,
                           size_t  code_size )
{
  FlushInstructionCache ( GetCurrentProcess (),
                            base_addr,
                              code_size );
}

void
SK_InjectMachineCode ( LPVOID   base_addr,
                       uint8_t* new_code,
                       size_t   code_size,
                       DWORD    permissions,
                       uint8_t* old_code = nullptr )
{
  if (SK_InjectMemory ( base_addr,
                          new_code,
                            code_size,
                              permissions,
                                old_code) )
    SK_FlushInstructionCache (base_addr, code_size);
}


#if 0
BOOL
WINAPI
EnumDisplaySettingsA_Detour ( _In_  LPCSTR    lpszDeviceName,
                              _In_  DWORD     iModeNum,
                              _Out_ DEVMODEA* lpDevMode )
{
  //dll_log.Log ( L"[Resolution] EnumDisplaySettingsA (%hs, %lu, %ph)",
                  //lpszDeviceName, iModeNum, lpDevMode );

  if (config.display.match_desktop) {
    if (iModeNum == 0)
      return EnumDisplaySettingsA_Original (lpszDeviceName, ENUM_CURRENT_SETTINGS, lpDevMode);

    EnumDisplaySettingsA_Original (lpszDeviceName, ENUM_CURRENT_SETTINGS, lpDevMode);

    return 0;
  }

  return EnumDisplaySettingsA_Original (lpszDeviceName, iModeNum, lpDevMode);
}

typedef LONG (WINAPI *ChangeDisplaySettingsA_pfn)(DEVMODEA* dontcare, DWORD dwFlags);
ChangeDisplaySettingsA_pfn ChangeDisplaySettingsA_Original = nullptr;

LONG
WINAPI
ChangeDisplaySettingsA_Detour (DEVMODEA* dontcare, DWORD dwFlags)
{
  bool override = false;

  if (dontcare != nullptr)
    dll_log.LogEx ( true, L"[Resolution] ChangeDisplaySettingsA - %4lux%-4lu@%#2lu Hz",
                      dontcare->dmPelsWidth, dontcare->dmPelsHeight,
                        dontcare->dmDisplayFrequency );
  else {
    dll_log.LogEx (true, L"[Resolution] ChangeDisplaySettingsA - RESET");
  }

  if (config.display.match_desktop) {
    override = true;

    dll_log.LogEx (false, L" { Override: Desktop }\n");

    //
    // Fix Display Scaling Problems
    //
    if ( dontcare != nullptr && dwFlags & CDS_TEST &&
         (dontcare->dmPelsWidth  != GetSystemMetrics_Original (SM_CXSCREEN) ||
          dontcare->dmPelsHeight != GetSystemMetrics_Original (SM_CYSCREEN)) ) {
      return DISP_CHANGE_FAILED;
    }

    return DISP_CHANGE_SUCCESSFUL;
  }

  if ( dontcare != nullptr && ( config.display.height  > 0 ||
                                config.display.width   > 0 ||
                                config.display.refresh > 0 ||
                                config.display.monitor > 0 ) ) {
    override = true;

    LONG ret = -1;

    int width   = dontcare->dmPelsWidth;
    int height  = dontcare->dmPelsHeight;
    int refresh = dontcare->dmDisplayFrequency;
    int monitor = 0;

    int test_width  = width;
    int test_height = height;

    if (config.display.height > 0) {
      height                 = config.display.height;
      dontcare->dmFields    |= DM_PELSHEIGHT;
      dontcare->dmPelsHeight = height;
    }

    if (config.display.width > 0) {
      width                 = config.display.width;
      dontcare->dmFields   |= DM_PELSWIDTH;
      dontcare->dmPelsWidth = width;
    }

    if (config.display.width > 0) {
      refresh                      = config.display.refresh;
      dontcare->dmFields          |= DM_DISPLAYFREQUENCY;
      dontcare->dmDisplayFrequency = refresh;
    }

    if (config.display.monitor > 0) {
      monitor = config.display.monitor;

      DISPLAY_DEVICEA dev = { 0 };
      dev.cb = sizeof DISPLAY_DEVICEA;

      if (EnumDisplayDevicesA (nullptr, monitor, &dev, 0x00)) {
        ret =
          ChangeDisplaySettingsExA ( dev.DeviceString,
                                       dontcare,
                                         NULL,
                                           dwFlags,
                                             nullptr );

#if 0
       DEVMODEA settings;
       settings.dmSize = sizeof DEVMODEA;
       EnumDisplaySettingsExA (dev.DeviceName,
                               ENUM_CURRENT_SETTINGS,
                               &settings,
                               EDS_ROTATEDMODE);
#endif
      }
    }

    dll_log.LogEx ( false, L" { Override: %4lux%-4lu@%#2luHz on"
                           L" Monitor %lu }\n",
                      width, height, refresh, monitor );

    //
    // Fix Display Scaling Problems
    //
    if ( dontcare != nullptr && dwFlags & CDS_TEST &&
         (test_width  != width ||
          test_height != height) ) {
      return DISP_CHANGE_FAILED;
    }

    if (! ret)
      return ret;
  }

  if (! override)
    dll_log.LogEx (false, L"\n");

  return ChangeDisplaySettingsA_Original (dontcare, dwFlags);
}
#endif

int
WINAPI
GetSystemMetrics_Detour (_In_ int nIndex)
{
  int nRet = GetSystemMetrics_Original (nIndex);

#if 0
  if (config.display.width > 0 && nIndex == SM_CXSCREEN)
    return config.display.width;

  if (config.display.height > 0 && nIndex == SM_CYSCREEN)
    return config.display.height;

  if (config.display.width > 0 && nIndex == SM_CXFULLSCREEN) {
    return config.display.width;
  }

  if (config.display.height > 0 && nIndex == SM_CYFULLSCREEN) {
    return config.display.height;
  }

  if (config.window.borderless) {
    if (nIndex == SM_CYCAPTION)
      return 0;
    if (nIndex == SM_CXBORDER)
      return 0;
    if (nIndex == SM_CYBORDER)
      return 0;
    if (nIndex == SM_CXDLGFRAME)
      return 0;
    if (nIndex == SM_CYDLGFRAME)
      return 0;
  }
#else
  dll_log.Log ( L"[Resolution] GetSystemMetrics (%lu) : %lu",
                  nIndex, nRet );
#endif

  return nRet;
}

void
BMF_DS3_InitPlugin (void)
{
  if (ds3_prefs == nullptr) {
    // Make the graphics config file read-only while running
    DWORD    dwConfigAttribs;
    uint32_t dwLen = MAX_PATH;
    wchar_t  wszGraphicsConfigPath [MAX_PATH];

    BMF_GetUserProfileDir (wszGraphicsConfigPath, &dwLen);

    wcscat ( wszGraphicsConfigPath,
               L"\\AppData\\Roaming\\DarkSoulsIII\\GraphicsConfig.xml" );

    dwConfigAttribs = GetFileAttributesW (wszGraphicsConfigPath);
                      SetFileAttributesW ( wszGraphicsConfigPath,
                                             dwConfigAttribs |
                                             FILE_ATTRIBUTE_READONLY );

    SK_SetPluginName (SUS_VERSION_STR);

    std::wstring ds3_prefs_file =
      std::wstring (L"SoulsUnsqueezed.ini");

    ds3_prefs = new bmf::INI::File (ds3_prefs_file.c_str ());
    ds3_prefs->parse ();
  }

  ds3_hud_res_x = 
      static_cast <bmf::ParameterInt *>
        (ds3_factory.create_parameter <int> (L"HUDResX"));
  ds3_hud_res_x->register_to_ini ( ds3_prefs,
                                    L"SUS.Display",
                                      L"HUDResX" );

  if (ds3_hud_res_x->load ())
    ds3_cfg.hud.res_x = ds3_hud_res_x->get_value ();

  ds3_hud_res_y = 
      static_cast <bmf::ParameterInt *>
        (ds3_factory.create_parameter <int> (L"HUDResY"));
  ds3_hud_res_y->register_to_ini ( ds3_prefs,
                                    L"SUS.Display",
                                      L"HUDResY" );

  if (ds3_hud_res_y->load ())
    ds3_cfg.hud.res_y = ds3_hud_res_y->get_value ();

  ds3_hud_offset_x = 
      static_cast <bmf::ParameterInt *>
        (ds3_factory.create_parameter <int> (L"HUDOffsetX"));
  ds3_hud_offset_x->register_to_ini ( ds3_prefs,
                                        L"SUS.Display",
                                          L"HUDOffsetX" );

  if (ds3_hud_offset_x->load ())
    ds3_cfg.hud.offset_x = ds3_hud_offset_x->get_value ();

  ds3_hud_offset_y = 
      static_cast <bmf::ParameterInt *>
        (ds3_factory.create_parameter <int> (L"HUDOffsetY"));
  ds3_hud_offset_y->register_to_ini ( ds3_prefs,
                                        L"SUS.Display",
                                          L"HUDOffsetY" );

  if (ds3_hud_offset_y->load ())
    ds3_cfg.hud.offset_y = ds3_hud_offset_y->get_value ();

  ds3_hud_stretch = 
      static_cast <bmf::ParameterBool *>
        (ds3_factory.create_parameter <bool> (L"StretchHUD"));
  ds3_hud_stretch->register_to_ini ( ds3_prefs,
                                        L"SUS.Display",
                                          L"StretchHUD" );

  if (ds3_hud_stretch->load ())
    ds3_cfg.hud.stretch = ds3_hud_stretch->get_value ();


  ds3_flip_mode =
    static_cast <bmf::ParameterBool *>
      (ds3_factory.create_parameter <bool> (L"FlipMode"));
  ds3_flip_mode->register_to_ini ( ds3_prefs,
                                     L"SUS.Render",
                                       L"FlipMode" );

  if (ds3_flip_mode->load ())
    ds3_cfg.render.flip_mode = ds3_flip_mode->get_value ();


  ds3_borderless =
    static_cast <bmf::ParameterBool *>
      (ds3_factory.create_parameter <bool> (L"Borderless"));
  ds3_borderless->register_to_ini ( ds3_prefs,
                                      L"SUS.Window",
                                        L"Borderless" );

  if (ds3_borderless->load ())
    ds3_cfg.window.borderless = ds3_borderless->get_value ();


  ds3_fullscreen =
    static_cast <bmf::ParameterBool *>
      (ds3_factory.create_parameter <bool> (L"Forceful Fullscreen Windows"));
  ds3_fullscreen->register_to_ini ( ds3_prefs,
                                      L"SUS.Window",
                                        L"Fullscreen" );

  if (ds3_fullscreen->load ()) {
    ds3_cfg.window.fullscreen = ds3_fullscreen->get_value ();

    // TEMP HACK
    __DS3_MAX_WINDOW = ds3_cfg.window.fullscreen;
  }


  ds3_center =
    static_cast <bmf::ParameterBool *>
      (ds3_factory.create_parameter <bool> (L"Center Windows"));
  ds3_center->register_to_ini ( ds3_prefs,
                                  L"SUS.Window",
                                    L"Center" );

  if (ds3_center->load ()) {
    ds3_cfg.window.center = ds3_center->get_value ();

    // TEMP HACK
    __DS3_CENTER = ds3_cfg.window.center;
  }


  ds3_default_res_x =
    static_cast <bmf::ParameterInt *>
      (ds3_factory.create_parameter <int> (L"Base (Windowed) Resolution"));
  ds3_default_res_x->register_to_ini ( ds3_prefs,
                                         L"SUS.Render",
                                           L"DefaultResX" );

  ds3_default_res_x->load ();

  ds3_default_res_y =
    static_cast <bmf::ParameterInt *>
      (ds3_factory.create_parameter <int> (L"Base (Windowed) Resolution"));
  ds3_default_res_y->register_to_ini ( ds3_prefs,
                                         L"SUS.Render",
                                           L"DefaultResY" );

  ds3_default_res_y->load ();


  ds3_sacrificial_x =
    static_cast <bmf::ParameterInt *>
    (ds3_factory.create_parameter <int> (L"Sacrificial (Windowed) Resolution"));
  ds3_sacrificial_x->register_to_ini ( ds3_prefs,
                                         L"SUS.Render",
                                           L"SacrificialResX" );

  if (ds3_sacrificial_x->load ())
    ds3_cfg.render.sacrifice_x = ds3_sacrificial_x->get_value ();

  ds3_sacrificial_y =
    static_cast <bmf::ParameterInt *>
    (ds3_factory.create_parameter <int> (L"Sacrificial (Windowed) Resolution"));
  ds3_sacrificial_y->register_to_ini ( ds3_prefs,
                                         L"SUS.Render",
                                           L"SacrificialResY" );

  if (ds3_sacrificial_y->load ())
    ds3_cfg.render.sacrifice_y = ds3_sacrificial_y->get_value ();


  uint8_t* sac_x = (uint8_t *)&ds3_cfg.render.sacrifice_x;
  uint8_t* sac_y = (uint8_t *)&ds3_cfg.render.sacrifice_y;

  // We are going to scan for an array of resolutions, it should be the only array
  //   in the executable image of the form <uint32_t : ResX>,<uint32_t : ResY>,...
  uint8_t res_sig  [] = { sac_x [0], sac_x [1], sac_x [2], sac_x [3],
                          sac_y [0], sac_y [1], sac_y [2], sac_y [3] };
  uint8_t res_mask [] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

  uint32_t res_x = ds3_default_res_x->get_value ();
  uint32_t res_y = ds3_default_res_y->get_value ();

  void* res_addr =
    SK_Scan (res_sig, 8, res_mask);

  void* res_addr_x = res_addr;
  void* res_addr_y = (uint8_t *)res_addr + 4;

  if (res_addr != nullptr) {
    if (res_x != 1920 || res_y != 1080) {
      SK_InjectMemory (res_addr_x, (uint8_t *)&res_x, 4, PAGE_EXECUTE_READWRITE);
      SK_InjectMemory (res_addr_y, (uint8_t *)&res_y, 4, PAGE_EXECUTE_READWRITE);
      dll_log.Log ( L"[AspectRatio] Custom Default Resolution: (%lux%lu) {%3.2f}",
                      res_x, res_y,
                        (float)res_x / (float)res_y );
    }
  } else {
    dll_log.Log (L"[AspectRatio] >> ERROR: Unable to locate memory address for 1920x1080... <<");
  }

#if 0
  BMF_CreateDLLHook ( L"user32.dll",
                      "GetSystemMetrics",
                       GetSystemMetrics_Detour,
            (LPVOID *)&GetSystemMetrics_Original );
#endif

  BMF_CreateFuncHook ( L"ID3D11DeviceContext::RSSetViewports",
                         D3D11_RSSetViewports_Override,
                           SK_DS3_RSSetViewports,
                             (LPVOID *)&D3D11_RSSetViewports_Original );
  BMF_EnableHook (D3D11_RSSetViewports_Override);

  BMF_CreateFuncHook ( L"IDXGISwapChain::ResizeTarget",
                         DXGISwap_ResizeTarget_Override,
                           SK_DS3_ResizeTarget,
                             (LPVOID *)&DXGISwap_ResizeTarget_Original );
  BMF_EnableHook (DXGISwap_ResizeTarget_Override);

  BMF_CreateFuncHook ( L"IDXGISwapChain::ResizeBuffers",
                         DXGISwap_ResizeBuffers_Override,
                           SK_DS3_ResizeBuffers,
                             (LPVOID *)&DXGISwap_ResizeBuffers_Original );
  BMF_EnableHook (DXGISwap_ResizeBuffers_Override);


  BMF_CreateFuncHook ( L"IDXGISwapChain::GetFullscreenState",
                         DXGISwap_GetFullscreenState_Override,
                           SK_DS3_GetFullscreenState,
                             (LPVOID *)&DXGISwap_GetFullscreenState_Original );
  BMF_EnableHook (DXGISwap_GetFullscreenState_Override);

  BMF_CreateFuncHook ( L"IDXGISwapChain::SetFullscreenState",
                         DXGISwap_SetFullscreenState_Override,
                           SK_DS3_SetFullscreenState,
                             (LPVOID *)&DXGISwap_SetFullscreenState_Original );
  BMF_EnableHook (DXGISwap_SetFullscreenState_Override);


  LPVOID lpvPluginKeyPress = nullptr;

  BMF_CreateFuncHook ( L"SK_PluginKeyPress",
                         SK_PluginKeyPress,
                           SK_DS3_PluginKeyPress,
                             (LPVOID *)&lpvPluginKeyPress );
  BMF_EnableHook (SK_PluginKeyPress);



#if 0
  BMF_CreateFuncHook ( L"BMF_ShutdownCore",
                         BMF_ShutdownCore,
                           SK_DS3_ShutdownPlugin,
                             (LPVOID *)&BMF_ShutdownCore_Original );
  BMF_EnableHook (BMF_ShutdownCore);
#endif
}


HRESULT
WINAPI
SK_DS3_CreateTexture2D (
    _In_            ID3D11Device           *This,
    _In_      const D3D11_TEXTURE2D_DESC   *pDesc,
    _In_opt_  const D3D11_SUBRESOURCE_DATA *pInitialData,
    _Out_opt_       ID3D11Texture2D        **ppTexture2D )
{
#if 0
  dll_log.Log (L"[!]ID3D11Device::CreateTexture2D (..., { (%lux%lu : %lu LODs - Fmt: %lu - BindFlags: 0x%04X) }, ...) %c%c%c",
                pDesc->Width, pDesc->Height, pDesc->MipLevels, pDesc->Format, pDesc->BindFlags,
                  pDesc->BindFlags & D3D11_BIND_RENDER_TARGET   ? L'r' : L'-',
                  pDesc->BindFlags & D3D11_BIND_DEPTH_STENCIL   ? L'd' : L'-',
                  pDesc->BindFlags & D3D11_BIND_SHADER_RESOURCE ? L's' : L'-' );
#endif

  HRESULT hr;

  D3D11_TEXTURE2D_DESC *pDescNew = new D3D11_TEXTURE2D_DESC (*pDesc);

  bool rt           = pDescNew->BindFlags & D3D11_BIND_RENDER_TARGET;
  bool depthstencil = pDescNew->BindFlags & D3D11_BIND_DEPTH_STENCIL;

  //bool is_16by9 = false;

  //if (pDescNew->Width >= 16.0f * ((float)pDescNew->Height / 9.0f) - 0.001f &&
      //pDescNew->Width <= 16.0f * ((float)pDescNew->Height / 9.0f) + 0.001f)
    //is_16by9 = true;

  if ( (rt || depthstencil ) &&
        pDescNew->Width      == 1280 && pDescNew->Height      == 720 && (
      ds3_cfg.hud.res_x != 1280 || ds3_cfg.hud.res_y != 720 ) ) {
    dll_log.Log (L" >> Rescaling rendertarget from (%lux%lu) to (%lux%lu)",
                    pDescNew->Width, pDescNew->Height,
                    ds3_cfg.hud.res_x, ds3_cfg.hud.res_y);
    hr = 
      D3D11Dev_CreateTexture2D_Override (This, pDescNew, pInitialData, ppTexture2D);
  }
#if 0
  else if ( (rt || depthstencil )&&
        is_16by9 ) {
    dll_log.Log (L" >> Rescaling rendertarget from (%lux%lu) to (%lux%lu)",
                    pDescNew->Width, pDescNew->Height,
                    3440, 1440);// (), BMF_DS3_GetHUDResY ());

    pDescNew->Height = 1440;
    pDescNew->Width  = 3440;

    hr = 
      D3D11Dev_CreateTexture2D_Override (This, pDescNew, pInitialData, ppTexture2D);
  }
#endif
  else 
  {
    hr =
      D3D11Dev_CreateTexture2D_Override (This, pDesc, pInitialData, ppTexture2D);
  }

  delete pDescNew;

  return hr;
}

void
SK_DS3_FinishResize (void)
{
  if (ds3_cfg.window.borderless)
    SetWindowLongW (__DS3_WINDOW, GWL_STYLE, WS_POPUP | WS_MINIMIZEBOX);

  int x_off = 0;
  int y_off = 0;

  if (__DS3_CENTER && (__DS3_MON_X > 0 && __DS3_MON_Y > 0)) {
    x_off = (__DS3_MON_X - __DS3_WIDTH)  / 2;
    y_off = (__DS3_MON_Y - __DS3_HEIGHT) / 2;
  }

  SetActiveWindow     (__DS3_WINDOW);
  SetForegroundWindow (__DS3_WINDOW);
  BringWindowToTop    (__DS3_WINDOW);

  SetWindowPos ( __DS3_WINDOW, HWND_NOTOPMOST,
                   0+x_off, 0+y_off,
                     __DS3_WIDTH, __DS3_HEIGHT,
                       SWP_FRAMECHANGED | SWP_SHOWWINDOW );
}

HRESULT
STDMETHODCALLTYPE
SK_DS3_GetFullscreenState (
  _In_      IDXGISwapChain  *This,
  _Out_opt_ BOOL            *pFullscreen,
  _Out_opt_  IDXGIOutput   **ppTarget )
{
  if (! ds3_cfg.window.borderless)
    return DXGISwap_GetFullscreenState_Original (This, pFullscreen, ppTarget);

  if (pFullscreen != nullptr)
    *pFullscreen = __DS3_FULLSCREEN;

  return S_OK;
  //return DXGISwap_GetFullscreenState_Original (This, nullptr, nullptr);
}

HRESULT
STDMETHODCALLTYPE
SK_DS3_SetFullscreenState (
  _In_ IDXGISwapChain *This,
  _In_ BOOL            Fullscreen,
  _In_ IDXGIOutput    *pTarget )
{
  __DS3_FULLSCREEN = Fullscreen;

  DXGI_SWAP_CHAIN_DESC swap_desc;
  if (SUCCEEDED (This->GetDesc (&swap_desc))) {
    __DS3_WINDOW = swap_desc.OutputWindow;

    DXGI_OUTPUT_DESC out_desc;

    if (pTarget != nullptr)
      pTarget->GetDesc (&out_desc);
    else {
      // pTarget is often NULL, so just fill-in the handfull of necessary parameters with
      //   values from the monitor belonging to the window.
      MONITORINFO minfo = { 0 };
      minfo.cbSize      = sizeof MONITORINFO;

      GetMonitorInfo ( MonitorFromWindow (swap_desc.OutputWindow, MONITOR_DEFAULTTOPRIMARY),
                         &minfo );
      out_desc.DesktopCoordinates = minfo.rcWork;
    }

    const RECT& krcDesktop = out_desc.DesktopCoordinates;

    __DS3_MON_X = out_desc.DesktopCoordinates.right  - out_desc.DesktopCoordinates.left;
    __DS3_MON_Y = out_desc.DesktopCoordinates.bottom - out_desc.DesktopCoordinates.top;

    if (ds3_cfg.window.borderless && __DS3_MAX_WINDOW) {
      DEVMODE devmode = { 0 };
      devmode.dmSize = sizeof DEVMODE;
      EnumDisplaySettings (nullptr, ENUM_CURRENT_SETTINGS, &devmode);

      // We may need to do a full-on temporary device mode change, since I don't want to
      //   bother with viewport hacks at the moment.
      //
      //  XXX: Later on, we can restore the game's usual viewport hackery.
      if (__DS3_FULLSCREEN) {
        if (devmode.dmPelsHeight != swap_desc.BufferDesc.Height ||
            devmode.dmPelsWidth  != swap_desc.BufferDesc.Width) {
          devmode.dmPelsWidth  = swap_desc.BufferDesc.Width;
          devmode.dmPelsHeight = swap_desc.BufferDesc.Height;
          ChangeDisplaySettings (&devmode, CDS_FULLSCREEN);

          __DS3_MON_X = swap_desc.BufferDesc.Width;
          __DS3_MON_Y = swap_desc.BufferDesc.Height;
        }
      } else {
        ChangeDisplaySettings (0, CDS_RESET);

        __DS3_MON_X = out_desc.DesktopCoordinates.right  - out_desc.DesktopCoordinates.left;
        __DS3_MON_Y = out_desc.DesktopCoordinates.bottom - out_desc.DesktopCoordinates.top;
      }
    }
  }

  if (ds3_cfg.window.borderless) {
    //SK_DS3_FinishResize ();

    HRESULT ret = S_OK;
    //DXGI_CALL (ret, (S_OK))
    return ret;
  }

  return DXGISwap_SetFullscreenState_Original (This, Fullscreen, pTarget);
}

COM_DECLSPEC_NOTHROW
__declspec (noinline)
HRESULT
STDMETHODCALLTYPE
SK_DS3_ResizeBuffers (IDXGISwapChain *This,
                 _In_ UINT            BufferCount,
                 _In_ UINT            Width,
                 _In_ UINT            Height,
                 _In_ DXGI_FORMAT     NewFormat,
                 _In_ UINT            SwapChainFlags)
{
  HRESULT hr =
    DXGISwap_ResizeBuffers_Original ( This,
                                        BufferCount,
                                          Width, Height,
                                            NewFormat,
                                              SwapChainFlags );

  if (SUCCEEDED (hr)) {
    if (Width != 0)
      __DS3_WIDTH  = Width;

    if (Height != 0)
      __DS3_HEIGHT = Height;

    //SK_DS3_FinishResize ();
  }

  return hr;
}

COM_DECLSPEC_NOTHROW
__declspec (noinline)
HRESULT
STDMETHODCALLTYPE
SK_DS3_ResizeTarget ( IDXGISwapChain *This,
           _In_ const DXGI_MODE_DESC *pNewTargetParameters )
{
  HRESULT ret =
    DXGISwap_ResizeTarget_Original (This, pNewTargetParameters);

  if (SUCCEEDED (ret) && (ds3_cfg.window.borderless || ((! __DS3_FULLSCREEN) && __DS3_CENTER))) {
    SK_DS3_FinishResize ();

    //DXGISwap_ResizeBuffers_Override (This, 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0x02);
  }

  return ret;
}

void
WINAPI
SK_DS3_RSSetViewports ( ID3D11DeviceContext* This,
                        UINT                 NumViewports,
                  const D3D11_VIEWPORT*      pViewports )
{
  D3D11_VIEWPORT* pNewViewports = new D3D11_VIEWPORT [NumViewports];

  for (int i = 0; i < NumViewports; i++) {
    pNewViewports [i] = pViewports [i];

    //bool is_16by9 = false;

    //dll_log.Log (L"[!] BEFORE { %i <%f,%f::%f,%f [%f,%f]> }",
                    //i, pNewViewports [i].Width,    pNewViewports [i].Height,
                      //pNewViewports [i].TopLeftX, pNewViewports [i].TopLeftY,
                      //pNewViewports [i].MinDepth, pNewViewports [i].MaxDepth );

    //if (pNewViewports [i].Width >= 16.0f * (pNewViewports [i].Height / 9.0f) - 0.001f &&
        //pNewViewports [i].Width <= 16.0f * (pNewViewports [i].Height / 9.0f) + 0.001f)
    //is_16by9 = true;

    // The game may do this to the UI for certain resolutions, we need to be proactive.
    bool incorrectly_centered = false;
      //( (pViewports [i].TopLeftY != 0.0f && pViewports [i].Height == (float)__DS3_HEIGHT + (-2.0f * pViewports [i].TopLeftY)) ||
        //(pViewports [i].TopLeftX != 0.0f && pViewports [i].Width  == (float)__DS3_WIDTH  + (-2.0f * pViewports [i].TopLeftX)) );


    if (pViewports [i].Width == 1280.0f && pViewports [i].Height == 720.0f &&
        pViewports [i].MinDepth == 0.0f && pViewports [i].MaxDepth == 0.0f) {
      pNewViewports [i].TopLeftX = (float)ds3_cfg.hud.offset_x;
      pNewViewports [i].TopLeftY = (float)ds3_cfg.hud.offset_y;
      pNewViewports [i].Width    = (float)ds3_cfg.hud.res_x;
      pNewViewports [i].Height   = (float)ds3_cfg.hud.res_y;
    }

    else if (ds3_cfg.hud.stretch &&
             (((pViewports [i].MinDepth == pViewports [i].MaxDepth) &&
                pViewports [i].Width  > (float)__DS3_WIDTH  - 4.0f &&
                pViewports [i].Width  < (float)__DS3_WIDTH  + 4.0f &&
                pViewports [i].Height > (float)__DS3_HEIGHT - 4.0f &&
                pViewports [i].Height < (float)__DS3_HEIGHT + 4.0f) ||
               (incorrectly_centered))) {
      if ((float)__DS3_WIDTH / (float)__DS3_HEIGHT >= (16.0f / 9.0f)) {
        float rescaled_width = pNewViewports [i].Width * ((float)__DS3_WIDTH / (float)__DS3_HEIGHT) / (16.0f / 9.0f);
        float excess_width   = rescaled_width - pNewViewports [i].Width;

        pNewViewports [i].Width    *= ((float)__DS3_WIDTH / (float)__DS3_HEIGHT) / (16.0f / 9.0f);
        pNewViewports [i].Height   = __DS3_HEIGHT;
        pNewViewports [i].TopLeftX = -excess_width / 2.0f;
        pNewViewports [i].TopLeftY = 0.0f;
      } else {
        float rescaled_height = pNewViewports [i].Height * (16.0f / 9.0f) / ((float)__DS3_WIDTH / (float)__DS3_HEIGHT);
        float excess_height   = rescaled_height - pNewViewports [i].Height;

        pNewViewports [i].Width    = __DS3_WIDTH;
        pNewViewports [i].Height   *= (16.0f / 9.0f) / ((float)__DS3_WIDTH / (float)__DS3_HEIGHT);
        pNewViewports [i].TopLeftX = 0.0f;
        pNewViewports [i].TopLeftY = -excess_height / 2.0f;
      }
      pNewViewports [i].MinDepth = 0.0f;
      pNewViewports [i].MaxDepth = 0.0f;
    }

#if 0
    dll_log.Log (L"[!] AFTER { %i <%f,%f::%f,%f [%f,%f]> }",
                    i, pNewViewports [i].Width,    pNewViewports [i].Height,
                      pNewViewports [i].TopLeftX, pNewViewports [i].TopLeftY,
                      pNewViewports [i].MinDepth, pNewViewports [i].MaxDepth );
#endif
  }

  D3D11_RSSetViewports_Original (This, NumViewports, pNewViewports);

  delete [] pNewViewports;
}


bool
BMF_DS3_UseFlipMode (void)
{
  return ds3_cfg.render.flip_mode;
}

void
CALLBACK
SK_DS3_PluginKeyPress ( BOOL Control,
                        BOOL Shift,
                        BOOL Alt,
                        BYTE vkCode )
{
  if (Control && Shift && Alt && vkCode == VK_OEM_PERIOD) {
    ds3_cfg.hud.stretch = (! ds3_cfg.hud.stretch);
  }
}

HRESULT
STDMETHODCALLTYPE
SK_DS3_PresentFirstFrame ( IDXGISwapChain *This,
                           UINT            SyncInterval,
                           UINT            Flags )
{
  DXGI_SWAP_CHAIN_DESC desc;
  This->GetDesc (&desc);

  __DS3_WIDTH  = desc.BufferDesc.Width;
  __DS3_HEIGHT = desc.BufferDesc.Height;

  if (ds3_cfg.window.borderless || (! __DS3_FULLSCREEN)) {
    DXGISwap_SetFullscreenState_Override (This, FALSE, nullptr);
  } else {
    DXGISwap_SetFullscreenState_Override (This, TRUE,  nullptr);
  }

  SK_DS3_FinishResize ();

  ////DXGISwap_ResizeBuffers_Original (This, 0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

  return S_OK;
}

bool
SK_DS3_IsBorderless (void)
{
  return ds3_cfg.window.borderless;
}

extern "C"
{
bool
WINAPI
SK_DS3_ShutdownPlugin (const wchar_t* backend)
{
  // Allow the graphics config file to be written again at shutdown...
  DWORD    dwConfigAttribs;
  uint32_t dwLen = MAX_PATH;
  wchar_t  wszGraphicsConfigPath [MAX_PATH];

  BMF_GetUserProfileDir (wszGraphicsConfigPath, &dwLen);

  wcscat ( wszGraphicsConfigPath,
             L"\\AppData\\Roaming\\DarkSoulsIII\\GraphicsConfig.xml" );

  dwConfigAttribs = GetFileAttributesW (wszGraphicsConfigPath);
                      SetFileAttributesW ( wszGraphicsConfigPath,
                                             dwConfigAttribs &
                                             (~FILE_ATTRIBUTE_READONLY) );


  return true;
}
}
