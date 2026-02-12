/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs CEF Initialization
 *
 * CRITICAL: multi_threaded_message_loop = true
 *           external_message_pump = false
 * This avoids conflicts with LibreOffice's VCL event loop.
 */

#ifdef HAVE_FEATURE_CEF

#ifdef _WIN32
#include <prewin.h>
#include <windows.h>
#include <postwin.h>
#endif

#include <officelabs/CefInit.hxx>

#include <include/cef_app.h>
#include <include/cef_browser.h>

#include <sal/log.hxx>
#include <osl/file.hxx>
#include <osl/module.hxx>
#include <rtl/bootstrap.hxx>

namespace officelabs {

CefInit& CefInit::instance()
{
    static CefInit sInstance;
    return sInstance;
}

CefInit::CefInit()
    : m_bInitialized(false)
{
}

CefInit::~CefInit()
{
    shutdown();
}

bool CefInit::initialize()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_bInitialized)
        return true;

#ifdef _WIN32
    CefMainArgs main_args(GetModuleHandle(nullptr));
#else
    CefMainArgs main_args(0, nullptr);
#endif

    CefSettings settings;

    // CRITICAL: Use multi-threaded message loop so CEF doesn't block VCL
    settings.multi_threaded_message_loop = true;
    settings.external_message_pump = false;

    // No sandbox - LibreOffice doesn't support CEF's sandbox model
    settings.no_sandbox = true;

    // Remote debugging for Chrome DevTools
    settings.remote_debugging_port = 9222;

    // Disable windowless rendering - we use a real HWND
    settings.windowless_rendering_enabled = false;

    // Set subprocess path
    OUString subprocessPath = getSubprocessPath();
    OString utf8Path = OUStringToOString(subprocessPath, RTL_TEXTENCODING_UTF8);
    CefString(&settings.browser_subprocess_path).FromASCII(utf8Path.getStr());

    // Log settings
    settings.log_severity = LOGSEVERITY_INFO;
    CefString(&settings.log_file).FromASCII("officelabs_cef.log");

    SAL_INFO("officelabs.cef", "Initializing CEF with subprocess: " << utf8Path);

    if (!CefInitialize(main_args, settings, nullptr, nullptr))
    {
        SAL_WARN("officelabs.cef", "CefInitialize() FAILED");
        return false;
    }

    m_bInitialized = true;
    SAL_INFO("officelabs.cef", "CEF initialized successfully (debug port 9222)");
    return true;
}

void CefInit::shutdown()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_bInitialized)
        return;

    SAL_INFO("officelabs.cef", "Shutting down CEF...");
    CefShutdown();
    m_bInitialized = false;
    SAL_INFO("officelabs.cef", "CEF shutdown complete");
}

OUString CefInit::getSubprocessPath() const
{
    // The subprocess exe lives next to soffice.exe in instdir/program/
    OUString sInstDir(u"$BRAND_BASE_DIR/$BRAND_SHARE_SUBDIR/.."_ustr);
    rtl::Bootstrap::expandMacros(sInstDir);

    OUString sFileUrl;
    osl::FileBase::getAbsoluteFileURL(OUString(), sInstDir, sFileUrl);

    // Convert file:// URL to system path
    OUString sSystemPath;
    osl::FileBase::getSystemPathFromFileURL(sFileUrl, sSystemPath);

#ifdef _WIN32
    return sSystemPath + "\\program\\officelabs_cef_subprocess.exe";
#else
    return sSystemPath + "/program/officelabs_cef_subprocess";
#endif
}

} // namespace officelabs

#endif // HAVE_FEATURE_CEF

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
