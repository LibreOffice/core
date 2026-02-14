# LibreOffice Fork - Architecture

> Last updated: Feb 13, 2026

## Overview

This is a fork of LibreOffice with a custom `officelabs/` module that embeds a CEF (Chromium Embedded Framework) browser panel in the Writer sidebar. The CEF panel hosts the React chat UI and provides IPC between JavaScript and the C++ document layer via cefQuery.

## Module Structure

```
libreoffice-fork/
+-- officelabs/                         # Custom OfficeLabs module
|   +-- source/
|   |   +-- CefInit.cxx                # CEF lifecycle singleton (3.4 KB)
|   |   +-- WebViewPanel.cxx           # CEF sidebar panel host (15 KB)
|   |   +-- WebViewMessageHandler.cxx  # cefQuery IPC routing (7.7 KB)
|   |   +-- DocumentController.cxx     # UNO document API (13.4 KB)
|   |   +-- AgentConnection.cxx        # HTTP client for VCL fallback (38.7 KB)
|   |   +-- cef_subprocess_main.cxx    # CEF subprocess entry point
|   +-- inc/officelabs/
|   |   +-- CefInit.hxx
|   |   +-- WebViewPanel.hxx
|   |   +-- WebViewMessageHandler.hxx
|   |   +-- DocumentController.hxx
|   |   +-- AgentConnection.hxx
|   |   +-- officelabsdllapi.h          # DLL export macros
|   +-- ui/
|   |   +-- webviewpanel.ui             # VCL UI definition
|   +-- Library_officelabs.mk           # gbuild library definition
|   +-- Module_officelabs.mk            # gbuild module definition
|   +-- Executable_officelabs_cef_subprocess.mk
|   +-- Package_cef.mk
|   +-- Makefile
+-- sw/source/uibase/sidebar/
|   +-- AIAssistantPanel.cxx            # VCL fallback (non-CEF builds)
|   +-- SwPanelFactory.cxx             # Panel registration (#ifdef HAVE_FEATURE_CEF)
+-- external/cef/                       # CEF v144 SDK
```

## Key Components

### CefInit.cxx - CEF Lifecycle
- Singleton pattern: `CefInit::instance()`
- Thread-safe initialization with mutex
- Critical settings:
  - `multi_threaded_message_loop = true` (avoids VCL event loop conflicts)
  - `no_sandbox = true`
  - Remote debugging port 9222
- Manages CefApp, CefBrowserProcessHandler

### WebViewPanel.cxx - CEF Sidebar Host
- Embeds CefBrowser in a VCL SystemChildWindow
- URL loading priority:
  1. `OFFICELABS_UI_DEV_URL` env var (dev: `http://localhost:5173`)
  2. Bundled `file://` path to `instdir/program/officelabs-ui/index.html`
- Resize handling: CefBrowser resized on VCL panel resize
- `postToVclThread()` helper: dispatches callbacks from CEF threads to VCL main thread using `PostUserEvent(LINK_NONMEMBER())`

### WebViewMessageHandler.cxx - IPC Routing
- Handles `cefQuery` messages from JavaScript
- Routes to appropriate C++ handler based on message type
- Supported messages:
  - `getDocument` - Returns full document text
  - `getSelection` - Returns selected text
  - `applyEdit` - Apply document changes
- Responses sent back via `cefQuery` callback

### DocumentController.cxx - UNO Document API
- Wraps UNO API calls for common document operations
- Used by WebViewMessageHandler for document access
- Operations: read text, get selection, apply edits

### AgentConnection.cxx - VCL Fallback
- HTTP client for the VCL-based AIAssistantPanel (non-CEF builds)
- Communicates with the Python agent service
- SSE parsing for streaming responses
- Only active when CEF is not available

## Build System

### gbuild Macros

```makefile
# Library_officelabs.mk
$(eval $(call gb_Library_Library,officelabs))
$(eval $(call gb_Library_add_exception_objects,officelabs, \
    officelabs/source/AgentConnection \
    officelabs/source/DocumentController \
))

# CEF-only sources (conditional)
ifeq ($(ENABLE_CEF),TRUE)
$(eval $(call gb_Library_add_exception_objects,officelabs, \
    officelabs/source/CefInit \
    officelabs/source/WebViewPanel \
    officelabs/source/WebViewMessageHandler \
))
endif
```

### Dependencies
comphelper, cppu, cppuhelper, sal, sfx, svl, svt, svx, svxcore, tk, tl, utl, vcl, i18nlangtag, boost, curl

### CEF Build Targets (conditional on `--with-cef`)
1. `Library_officelabs` - Main library
2. `Executable_officelabs_cef_subprocess` - CEF child process
3. `Package_cef` - CEF resource files
4. `UIConfig_officelabs` - UI configuration

### Building

```bash
# Full build via Cygwin
/c/cygwin64/bin/bash.exe -l -c "cd /cygdrive/c/.../libreoffice-fork && make officelabs"

# CEF wrapper (CMake, separate)
# VS 2026, /MD runtime, CEF v144
```

## Critical Patterns & Gotchas

### Header Order
Must include Windows headers BEFORE CEF or VCL system headers:
```cpp
#include <prewin.h>
#include <windows.h>
#include <postwin.h>
// THEN CEF/VCL headers
```

### VCL Thread Dispatch
Use `LINK_NONMEMBER` macro for PostUserEvent callbacks, NOT `Link<>()` constructor (private):
```cpp
postToVclThread([=]() {
    // Safe to access VCL objects here
});
```

### CEF 144 API Changes (vs older versions)
- `OnRenderProcessTerminated` has 4 parameters
- `OnAfterCreated` removed from CefMessageRouterBrowserSide
- Message router Handler is NOT ref-counted (use `std::unique_ptr`, not `CefRefPtr`)
- Resource files renamed: `resources.pak` not `cef.pak`, `chrome_*_percent.pak` not `cef_*`

### Conditional Compilation
```cpp
#ifdef HAVE_FEATURE_CEF
    // CEF sidebar (WebViewPanel)
#else
    // VCL fallback (AIAssistantPanel)
#endif
```

## Config Files

| File | Purpose |
|------|---------|
| `config_host.mk` | ENABLE_CEF, CEF_DIR, CEF_CFLAGS, CEF_LIBS |
| `officelabs/Sidebar.xcu` | Sidebar panel registration |
| `officelabs/Accelerators.xcu` | Keyboard shortcuts |
