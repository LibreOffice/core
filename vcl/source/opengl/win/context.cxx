/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <string_view>
#include <thread>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>

#include <sal/log.hxx>
#include <comphelper/windowserrorstring.hxx>
#include <opengl/zone.hxx>
#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salframe.h>
#include <win/salinst.h>
#include <epoxy/wgl.h>
#include <ControlCacheKey.hxx>

static std::vector<HGLRC> g_vShareList;
static bool g_bAnyCurrent;

namespace {

class GLWinWindow : public GLWindow
{
public:
    HWND                    hWnd;
    HDC                     hDC;
    HGLRC                   hRC;
    GLWinWindow();
};

}

GLWinWindow::GLWinWindow()
    : hWnd(nullptr)
    , hDC(nullptr)
    , hRC(nullptr)
{
}

namespace {

class WinOpenGLContext : public OpenGLContext
{
public:
    virtual void initWindow() override;
private:
    GLWinWindow m_aGLWin;
    virtual const GLWindow& getOpenGLWindow() const override { return m_aGLWin; }
    virtual GLWindow& getModifiableOpenGLWindow() override { return m_aGLWin; }
    virtual bool ImplInit() override;
    virtual void makeCurrent() override;
    virtual void destroyCurrentContext() override;
    virtual bool isCurrent() override;
    virtual bool isAnyCurrent() override;
    virtual void resetCurrent() override;
    virtual void swapBuffers() override;
};

}

void WinOpenGLContext::swapBuffers()
{
    OpenGLZone aZone;

    SwapBuffers(m_aGLWin.hDC);

    BuffersSwapped();
}

void WinOpenGLContext::resetCurrent()
{
    clearCurrent();

    OpenGLZone aZone;

    wglMakeCurrent(nullptr, nullptr);
    g_bAnyCurrent = false;
}

static void ensureDispatchTable()
{
    thread_local bool bEpoxyDispatchMakeCurrentCalled = false;
    if (!bEpoxyDispatchMakeCurrentCalled)
    {
        epoxy_handle_external_wglMakeCurrent();
        bEpoxyDispatchMakeCurrentCalled = true;
    }
}

bool WinOpenGLContext::isCurrent()
{
    OpenGLZone aZone;
    if (!g_bAnyCurrent || !m_aGLWin.hRC)
        return false;
    ensureDispatchTable();
    return wglGetCurrentContext() == m_aGLWin.hRC && wglGetCurrentDC() == m_aGLWin.hDC;
}

bool WinOpenGLContext::isAnyCurrent()
{
    return g_bAnyCurrent && wglGetCurrentContext() != nullptr;
}

void WinOpenGLContext::makeCurrent()
{
    if (isCurrent())
        return;

    OpenGLZone aZone;

    clearCurrent();

    ensureDispatchTable();

    if (!wglMakeCurrent(m_aGLWin.hDC, m_aGLWin.hRC))
    {
        g_bAnyCurrent = false;
        DWORD nLastError = GetLastError();
        if (nLastError != ERROR_SUCCESS)
            SAL_WARN("vcl.opengl", "wglMakeCurrent failed: " << WindowsErrorString(nLastError));
        return;
    }

    g_bAnyCurrent = true;

    registerAsCurrent();
}

void WinOpenGLContext::initWindow()
{
    if( !m_pChildWindow )
    {
        SystemWindowData winData = generateWinData(mpWindow, false);
        m_pChildWindow = VclPtr<SystemChildWindow>::Create(mpWindow, 0, &winData, false);
    }

    if (m_pChildWindow)
    {
        InitChildWindow(m_pChildWindow.get());
        const SystemEnvData* sysData(m_pChildWindow->GetSystemData());
        m_aGLWin.hWnd = sysData->hWnd;
    }

    m_aGLWin.hDC = GetDC(m_aGLWin.hWnd);
}

void WinOpenGLContext::destroyCurrentContext()
{
    if (m_aGLWin.hRC)
    {
        std::vector<HGLRC>::iterator itr = std::remove(g_vShareList.begin(), g_vShareList.end(), m_aGLWin.hRC);
        if (itr != g_vShareList.end())
            g_vShareList.erase(itr);

        if (wglGetCurrentContext() != nullptr)
        {
            wglMakeCurrent(nullptr, nullptr);
            g_bAnyCurrent = false;
        }
        wglDeleteContext( m_aGLWin.hRC );
        ReleaseDC( m_aGLWin.hWnd, m_aGLWin.hDC );
        m_aGLWin.hRC = nullptr;
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        return 0;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_DESTROY:
        return 0;
    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

static bool InitTempWindow(HWND& hwnd, int width, int height, const PIXELFORMATDESCRIPTOR& inPfd, GLWinWindow& glWin)
{
    OpenGLZone aZone;

    PIXELFORMATDESCRIPTOR  pfd = inPfd;
    int ret;
    WNDCLASSW wc;
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = wc.cbWndExtra = 0;
    wc.hInstance = nullptr;
    wc.hIcon = nullptr;
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = L"GLRenderer";
    RegisterClassW(&wc);
    hwnd = CreateWindowW(wc.lpszClassName, nullptr, WS_DISABLED, 0, 0, width, height, nullptr, nullptr, wc.hInstance, nullptr);
    glWin.hDC = GetDC(hwnd);

    int nPixelFormat = ChoosePixelFormat(glWin.hDC, &pfd);
    if (!nPixelFormat)
    {
        ReleaseDC(hwnd, glWin.hDC);
        DestroyWindow(hwnd);
        return false;
    }
    ret = SetPixelFormat(glWin.hDC, nPixelFormat, &pfd);
    if(!ret)
    {
        ReleaseDC(hwnd, glWin.hDC);
        DestroyWindow(hwnd);
        return false;
    }
    glWin.hRC = wglCreateContext(glWin.hDC);
    if(!(glWin.hRC))
    {
        ReleaseDC(hwnd, glWin.hDC);
        DestroyWindow(hwnd);
        return false;
    }
    ret = wglMakeCurrent(glWin.hDC, glWin.hRC);
    if(!ret)
    {
        wglMakeCurrent(nullptr, nullptr);
        g_bAnyCurrent = false;
        wglDeleteContext(glWin.hRC);
        ReleaseDC(hwnd, glWin.hDC);
        DestroyWindow(hwnd);
        return false;
    }
    g_bAnyCurrent = false;

    return true;
}

static bool WGLisExtensionSupported(const char *extension)
{
    OpenGLZone aZone;

    const size_t extlen = strlen(extension);
    const char *supported = nullptr;

    // Try to use wglGetExtensionStringARB on current DC, if possible
    PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");

    if (wglGetExtString)
        supported = reinterpret_cast<char*(__stdcall*)(HDC)>(wglGetExtString)(wglGetCurrentDC());
    // If that failed, try standard OpenGL extensions string
    if (supported == nullptr)
        supported = reinterpret_cast<char const *>(glGetString(GL_EXTENSIONS));
    // If that failed too, must be no extensions supported
    if (supported == nullptr)
        return false;

    // Begin examination at start of string, increment by 1 on false match
    for (const char* p = supported; ; p++)
    {
        // Advance p up to the next possible match
        p = strstr(p, extension);

        if (p == nullptr)
            return false; // No Match

        // Make sure that match is at the start of the string or that
        // the previous char is a space, or else we could accidentally
        // match "wglFunkywglExtension" with "wglExtension"

        // Also, make sure that the following character is space or null
        // or else "wglExtensionTwo" might match "wglExtension"
        if ((p==supported || p[-1]==' ') && (p[extlen]=='\0' || p[extlen]==' '))
            return true; // Match
    }
}

static bool InitMultisample(const PIXELFORMATDESCRIPTOR& pfd, int& rPixelFormat,
        bool bUseDoubleBufferedRendering, bool bRequestVirtualDevice)
{
    OpenGLZone aZone;

    HWND hWnd = nullptr;
    GLWinWindow glWin;
    // Create a temp window to check whether support multi-sample, if support, get the format
    if (!InitTempWindow(hWnd, 32, 32, pfd, glWin))
    {
        SAL_WARN("vcl.opengl", "Can't create temp window to test");
        return false;
    }

    // See if the string exists in WGL
    if (!WGLisExtensionSupported("WGL_ARB_multisample"))
    {
        SAL_WARN("vcl.opengl", "Device doesn't support multisample");
        wglMakeCurrent(nullptr, nullptr);
        g_bAnyCurrent = false;
        wglDeleteContext(glWin.hRC);
        ReleaseDC(hWnd, glWin.hDC);
        DestroyWindow(hWnd);
        return false;
    }
    // Get our pixel format
    PFNWGLCHOOSEPIXELFORMATARBPROC fn_wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(wglGetProcAddress("wglChoosePixelFormatARB"));
    if (!fn_wglChoosePixelFormatARB)
    {
        wglMakeCurrent(nullptr, nullptr);
        g_bAnyCurrent = false;
        wglDeleteContext(glWin.hRC);
        ReleaseDC(hWnd, glWin.hDC);
        DestroyWindow(hWnd);
        return false;
    }
    // Get our current device context
    HDC hDC = GetDC(hWnd);

    int pixelFormat;
    int valid;
    UINT    numFormats;
    float   fAttributes[] = {0,0};
    // These attributes are the bits we want to test for in our sample.
    // Everything is pretty standard, the only one we want to
    // really focus on is the WGL_SAMPLE_BUFFERS_ARB and WGL_SAMPLES_ARB.
    // These two are going to do the main testing for whether or not
    // we support multisampling on this hardware.
    int iAttributes[] =
    {
        WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
        WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
        WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB,24,
        WGL_ALPHA_BITS_ARB,8,
        WGL_DEPTH_BITS_ARB,24,
        WGL_STENCIL_BITS_ARB,0,
        WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
        WGL_SAMPLES_ARB,8,
        0,0
    };

    if (!bUseDoubleBufferedRendering)
    {
        // Use asserts to make sure the iAttributes array is not changed without changing these ugly
        // hardcode indexes into it.
        assert(iAttributes[0] == WGL_DOUBLE_BUFFER_ARB);
        iAttributes[1] = GL_FALSE;
    }

    if (bRequestVirtualDevice)
    {
        assert(iAttributes[2] == WGL_DRAW_TO_WINDOW_ARB);
        iAttributes[2] = WGL_DRAW_TO_BITMAP_ARB;
    }

    bool bArbMultisampleSupported = false;

    // First we check to see if we can get a pixel format for 8 samples
    valid = fn_wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
    // If we returned true, and our format count is greater than 1
    if (valid && numFormats >= 1)
    {
        bArbMultisampleSupported = true;
        rPixelFormat = pixelFormat;
        wglMakeCurrent(nullptr, nullptr);
        g_bAnyCurrent = false;
        wglDeleteContext(glWin.hRC);
        ReleaseDC(hWnd, glWin.hDC);
        DestroyWindow(hWnd);
        return bArbMultisampleSupported;
    }
    // Our pixel format with 8 samples failed, test for 2 samples
    assert(iAttributes[18] == WGL_SAMPLES_ARB);
    iAttributes[19] = 2;
    valid = fn_wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
    if (valid && numFormats >= 1)
    {
        bArbMultisampleSupported = true;
        rPixelFormat = pixelFormat;
        wglMakeCurrent(nullptr, nullptr);
        g_bAnyCurrent = false;
        wglDeleteContext(glWin.hRC);
        ReleaseDC(hWnd, glWin.hDC);
        DestroyWindow(hWnd);
        return bArbMultisampleSupported;
    }
    // Return the valid format
    wglMakeCurrent(nullptr, nullptr);
    g_bAnyCurrent = false;
    wglDeleteContext(glWin.hRC);
    ReleaseDC(hWnd, glWin.hDC);
    DestroyWindow(hWnd);

    return bArbMultisampleSupported;
}

namespace
{

bool tryShaders(const OUString& rVertexShader, const OUString& rFragmentShader, const OUString& rGeometryShader = "", std::string_view rPreamble = "")
{
    GLint nId;

    // Somewhat mysteriously, the OpenGLHelper::LoadShaders() API saves a compiled binary of the
    // shader only if you give it the digest of the shaders. We have API to calculate the digest
    // only of the combination of vertex and fragment (but not geometry) shader. So if we have a
    // geometry shader, we should not save the binary.
    if (rGeometryShader.isEmpty())
    {
        nId = OpenGLHelper::LoadShaders(rVertexShader, rFragmentShader, rPreamble, OpenGLHelper::GetDigest( rVertexShader, rFragmentShader, rPreamble));
    }
    else
    {
        assert(rPreamble.empty());
        nId = OpenGLHelper::LoadShaders(rVertexShader, rFragmentShader, rGeometryShader);
    }
    if (!nId)
        return false;

    // We're interested in the error returned by glDeleteProgram().
    glGetError();

    glDeleteProgram(nId);
    return glGetError() == GL_NO_ERROR;
}

bool compiledShaderBinariesWork()
{
    static bool bBeenHere = false;
    static bool bResult;

    if (bBeenHere)
        return bResult;

    bBeenHere = true;

    bResult =
        (
#if 0 // Only look at shaders used by slideshow for now
         // canvas
         tryShaders("dummyVertexShader", "linearMultiColorGradientFragmentShader") &&
         tryShaders("dummyVertexShader", "linearTwoColorGradientFragmentShader") &&
         tryShaders("dummyVertexShader", "radialMultiColorGradientFragmentShader") &&
         tryShaders("dummyVertexShader", "radialTwoColorGradientFragmentShader") &&
         tryShaders("dummyVertexShader", "rectangularMultiColorGradientFragmentShader") &&
         tryShaders("dummyVertexShader", "rectangularTwoColorGradientFragmentShader") &&
#endif
         // slideshow
         tryShaders("reflectionVertexShader", "reflectionFragmentShader") &&
         tryShaders("basicVertexShader", "basicFragmentShader") &&
         tryShaders("vortexVertexShader", "vortexFragmentShader", "vortexGeometryShader") &&
         tryShaders("basicVertexShader", "rippleFragmentShader") &&
         tryShaders("glitterVertexShader", "glitterFragmentShader") &&
         tryShaders("honeycombVertexShader", "honeycombFragmentShader", "honeycombGeometryShader"));

    return bResult;
}

} // unnamed namespace

bool WinOpenGLContext::ImplInit()
{
    static bool bFirstCall = true;

    OpenGLZone aZone;

    VCL_GL_INFO("OpenGLContext::ImplInit----start");
    // PixelFormat tells Windows how we want things to be
    PIXELFORMATDESCRIPTOR PixelFormatFront =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                              // Version Number
        PFD_SUPPORT_OPENGL,
        PFD_TYPE_RGBA,                  // Request An RGBA Format
        BYTE(32),                       // Select Our Color Depth
        0, 0, 0, 0, 0, 0,               // Color Bits Ignored
        0,                              // No Alpha Buffer
        0,                              // Shift Bit Ignored
        0,                              // No Accumulation Buffer
        0, 0, 0, 0,                     // Accumulation Bits Ignored
        24,                             // 24 bit z-buffer
        8,                              // stencil buffer
        0,                              // No Auxiliary Buffer
        0,                              // now ignored
        0,                              // Reserved
        0, 0, 0                         // Layer Masks Ignored
    };

    PixelFormatFront.dwFlags |= PFD_DOUBLEBUFFER;
    PixelFormatFront.dwFlags |= PFD_DRAW_TO_WINDOW;

    //  we must check whether can set the MSAA
    int WindowPix = 0;
    bool bMultiSampleSupport = false;

    bMultiSampleSupport = InitMultisample(PixelFormatFront, WindowPix, /*bUseDoubleBufferedRendering*/true, false);

    if (bMultiSampleSupport && WindowPix != 0)
    {
        m_aGLWin.bMultiSampleSupported = true;
    }
    else
    {
        WindowPix = ChoosePixelFormat(m_aGLWin.hDC, &PixelFormatFront);
#if OSL_DEBUG_LEVEL > 0
        PIXELFORMATDESCRIPTOR pfd;
        DescribePixelFormat(m_aGLWin.hDC, WindowPix, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
        SAL_WARN("vcl.opengl", "Render Target: Window: " << static_cast<int>((pfd.dwFlags & PFD_DRAW_TO_WINDOW) != 0) << ", Bitmap: " << static_cast<int>((pfd.dwFlags & PFD_DRAW_TO_BITMAP) != 0));
        SAL_WARN("vcl.opengl", "Supports OpenGL: " << static_cast<int>((pfd.dwFlags & PFD_SUPPORT_OPENGL) != 0));
#endif
    }

    if (WindowPix == 0)
    {
        SAL_WARN("vcl.opengl", "Invalid pixelformat");
        return false;
    }

    if (!SetPixelFormat(m_aGLWin.hDC, WindowPix, &PixelFormatFront))
    {
        SAL_WARN("vcl.opengl", "SetPixelFormat failed: " << WindowsErrorString(GetLastError()));
        return false;
    }

    HGLRC hTempRC = wglCreateContext(m_aGLWin.hDC);
    if (hTempRC == nullptr)
    {
        SAL_WARN("vcl.opengl", "wglCreateContext failed: "<< WindowsErrorString(GetLastError()));
        return false;
    }

    if (!wglMakeCurrent(m_aGLWin.hDC, hTempRC))
    {
        g_bAnyCurrent = false;
        SAL_WARN("vcl.opengl", "wglMakeCurrent failed: "<< WindowsErrorString(GetLastError()));
        return false;
    }

    g_bAnyCurrent = true;

    if (!InitGL())
    {
        wglMakeCurrent(nullptr, nullptr);
        g_bAnyCurrent = false;
        wglDeleteContext(hTempRC);
        return false;
    }

    HGLRC hSharedCtx = nullptr;
    if (!g_vShareList.empty())
        hSharedCtx = g_vShareList.front();

    if (!wglCreateContextAttribsARB)
    {
        wglMakeCurrent(nullptr, nullptr);
        g_bAnyCurrent = false;
        wglDeleteContext(hTempRC);
        return false;
    }

    // now setup the shared context; this needs a temporary context already
    // set up in order to work
    int const attribs [] =
    {
#ifdef DBG_UTIL
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
        0
    };
    m_aGLWin.hRC = wglCreateContextAttribsARB(m_aGLWin.hDC, hSharedCtx, attribs);
    if (m_aGLWin.hRC == nullptr)
    {
        SAL_WARN("vcl.opengl", "wglCreateContextAttribsARB failed: "<< WindowsErrorString(GetLastError()));
        wglMakeCurrent(nullptr, nullptr);
        g_bAnyCurrent = false;
        wglDeleteContext(hTempRC);
        return false;
    }

    if (!compiledShaderBinariesWork())
    {
        wglMakeCurrent(nullptr, nullptr);
        g_bAnyCurrent = false;
        wglDeleteContext(hTempRC);
        return false;
    }

    wglMakeCurrent(nullptr, nullptr);
    g_bAnyCurrent = false;
    wglDeleteContext(hTempRC);

    if (!wglMakeCurrent(m_aGLWin.hDC, m_aGLWin.hRC))
    {
        g_bAnyCurrent = false;
        SAL_WARN("vcl.opengl", "wglMakeCurrent failed: " << WindowsErrorString(GetLastError()));
        return false;
    }

    g_bAnyCurrent = true;

    if (bFirstCall)
    {
        // Checking texture size
        GLint nMaxTextureSize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &nMaxTextureSize);
        if (nMaxTextureSize <= 4096)
            SAL_WARN("vcl.opengl", "Max texture size is " << nMaxTextureSize
                                    << ". This may not be enough for normal operation.");
        else
            VCL_GL_INFO("Max texture size: " << nMaxTextureSize);

        // Trying to make a texture and check its size
        for (GLint nWidthHeight = 1023; nWidthHeight < nMaxTextureSize; nWidthHeight += (nWidthHeight + 1))
        {
            glTexImage2D(GL_PROXY_TEXTURE_2D, 0, 4, nWidthHeight, nWidthHeight, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8, nullptr);
            CHECK_GL_ERROR();
            if (glGetError() == GL_NO_ERROR)
            {
                GLint nWidth = 0;
                GLint nHeight = 0;
                glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &nWidth);
                glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &nHeight);
                VCL_GL_INFO("Created texture " << nWidthHeight << "," << nWidthHeight << " reports size: " << nWidth << ", " << nHeight);
            }
            else
            {
                SAL_WARN("vcl.opengl", "Error when creating a " << nWidthHeight << ", " << nWidthHeight << " test texture.");
            }
        }
    }

    InitGLDebugging();

    g_vShareList.push_back(m_aGLWin.hRC);

    RECT clientRect;
    GetClientRect(WindowFromDC(m_aGLWin.hDC), &clientRect);
    m_aGLWin.Width = clientRect.right - clientRect.left;
    m_aGLWin.Height = clientRect.bottom - clientRect.top;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    registerAsCurrent();

    bFirstCall = false;

    return true;
}

OpenGLContext* WinSalInstance::CreateOpenGLContext()
{
    return new WinOpenGLContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
