/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#include <boost/scoped_array.hpp>
#include <vcl/pngwrite.hxx>
#include <vcl/bmpacc.hxx>
#include <vcl/graph.hxx>

#include "win/WinOpenGLContext.hxx"

using namespace com::sun::star;

WinOpenGLContext::WinOpenGLContext():
    mpWindow(NULL),
    m_pChildWindow(NULL),
    mbInitialized(false)
{
}

WinOpenGLContext::~WinOpenGLContext()
{
    if (m_aGLWin.hRC)
    {
        wglMakeCurrent( m_aGLWin.hDC, 0 );
        wglDeleteContext( m_aGLWin.hRC );
        ReleaseDC( m_aGLWin.hWnd, m_aGLWin.hDC );
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
    case WM_KEYDOWN:
        switch(wParam)
        {
        case VK_ESCAPE:
            PostQuitMessage(0);
            return 0;

        case VK_SPACE:
            break;
        }
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
}

int InitTempWindow(HWND *hwnd, int width, int height, PIXELFORMATDESCRIPTOR inPfd, GLWindow glWin)
{
    PIXELFORMATDESCRIPTOR  pfd = inPfd;
    int  pfmt;
    int ret;
    WNDCLASS wc;
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = wc.cbWndExtra = 0;
    wc.hInstance = NULL;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = (LPCSTR)"GLRenderer";
    RegisterClass(&wc);
    *hwnd = CreateWindow(wc.lpszClassName, NULL, WS_DISABLED, 0, 0, width, height, NULL, NULL, wc.hInstance, NULL);
    glWin.hDC = GetDC(*hwnd);
    pfmt = ChoosePixelFormat(glWin.hDC, &pfd);
    if (!pfmt)
    {
        return -1;
    }
    ret = SetPixelFormat(glWin.hDC, pfmt, &pfd);
    if(!ret)
    {
        return -1;
    }
    glWin.hRC = wglCreateContext(glWin.hDC);
    if(!(glWin.hRC))
    {
        return -1;
    }
    ret = wglMakeCurrent(glWin.hDC, glWin.hRC);
    if(!ret)
    {
        return -1;
    }
    return 0;
}

bool WGLisExtensionSupported(const char *extension)
{
    const size_t extlen = strlen(extension);
    const char *supported = NULL;

    // Try To Use wglGetExtensionStringARB On Current DC, If Possible
    PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");

    if (wglGetExtString)
        supported = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());
    // If That Failed, Try Standard Opengl Extensions String
    if (supported == NULL)
        supported = (char*)glGetString(GL_EXTENSIONS);
    // If That Failed Too, Must Be No Extensions Supported
    if (supported == NULL)
        return 0;

    // Begin Examination At Start Of String, Increment By 1 On False Match
    for (const char* p = supported; ; p++)
    {
        // Advance p Up To The Next Possible Match
        p = strstr(p, extension);

        if (p == NULL)
            return 0; // No Match

        // Make Sure That Match Is At The Start Of The String Or That
        // The Previous Char Is A Space, Or Else We Could Accidentally
        // Match "wglFunkywglExtension" With "wglExtension"

        // Also, Make Sure That The Following Character Is Space Or NULL
        // Or Else "wglExtensionTwo" Might Match "wglExtension"
        if ((p==supported || p[-1]==' ') && (p[extlen]=='\0' || p[extlen]==' '))
            return 1; // Match
    }
}

bool InitMultisample(PIXELFORMATDESCRIPTOR pfd, int& rPixelFormat)
{
    HWND hWnd = NULL;
    GLWindow glWin;
    //create a temp windwo to check whether support multi-sample, if support, get the format
    if (InitTempWindow(&hWnd, 1, 1, pfd, glWin) < 0)
    {
        SAL_WARN("vcl.opengl", "Can't create temp window to test");
        return false;
    }

    // See If The String Exists In WGL!
    if (!WGLisExtensionSupported("WGL_ARB_multisample"))
    {
        SAL_WARN("vcl.opengl", "Device doesn't support multi sample");
        return false;
    }
    // Get Our Pixel Format
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    if (!wglChoosePixelFormatARB)
    {
        return false;
    }
    // Get Our Current Device Context
    HDC hDC = GetDC(hWnd);

    int pixelFormat;
    int valid;
    UINT    numFormats;
    float   fAttributes[] = {0,0};
    // These Attributes Are The Bits We Want To Test For In Our Sample
    // Everything Is Pretty Standard, The Only One We Want To
    // Really Focus On Is The SAMPLE BUFFERS ARB And WGL SAMPLES
    // These Two Are Going To Do The Main Testing For Whether Or Not
    // We Support Multisampling On This Hardware.
    int iAttributes[] =
    {
        WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
        WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
        WGL_COLOR_BITS_ARB,24,
        WGL_ALPHA_BITS_ARB,8,
        WGL_DEPTH_BITS_ARB,16,
        WGL_STENCIL_BITS_ARB,0,
        WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
        WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
        WGL_SAMPLES_ARB,8,
        0,0
    };

    bool bArbMultisampleSupported = true;

    // First We Check To See If We Can Get A Pixel Format For 4 Samples
    valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
    // If We Returned True, And Our Format Count Is Greater Than 1
    if (valid && numFormats >= 1)
    {
        bArbMultisampleSupported = true;
        rPixelFormat = pixelFormat;
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(glWin.hRC);
        ReleaseDC(hWnd, glWin.hDC);
        DestroyWindow(hWnd);
        return bArbMultisampleSupported;
    }
    // Our Pixel Format With 4 Samples Failed, Test For 2 Samples
    iAttributes[19] = 2;
    valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
    if (valid && numFormats >= 1)
    {
        bArbMultisampleSupported = true;
        rPixelFormat = pixelFormat;
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(glWin.hRC);
        ReleaseDC(hWnd, glWin.hDC);
        DestroyWindow(hWnd);
        return bArbMultisampleSupported;
    }
    // Return The Valid Format
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(glWin.hRC);
    ReleaseDC(hWnd, glWin.hDC);
    DestroyWindow(hWnd);

    return  bArbMultisampleSupported;
}

#ifdef DBG_UTIL

namespace {

const char* getSeverityString(GLenum severity)
{
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_LOW:
            return "low";
        case GL_DEBUG_SEVERITY_MEDIUM:
            return "medium";
        case GL_DEBUG_SEVERITY_HIGH:
            return "high";
        default:
            ;
    }

    return "unknown";
}

const char* getSourceString(GLenum source)
{
    switch(source)
    {
        case GL_DEBUG_SOURCE_API:
            return "API";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "shader compiler";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "window system";
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return "third party";
        case GL_DEBUG_SOURCE_APPLICATION:
            return "Libreoffice";
        case GL_DEBUG_SOURCE_OTHER:
            return "unknown";
        default:
            ;
    }

    return "unknown";
}

const char* getTypeString(GLenum type)
{
    switch(type)
    {
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return "deprecated behavior";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return "undefined behavior";
        case GL_DEBUG_TYPE_PERFORMANCE:
            return "performance";
        case GL_DEBUG_TYPE_PORTABILITY:
            return "portability";
        case GL_DEBUG_TYPE_MARKER:
            return "marker";
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return "push group";
        case GL_DEBUG_TYPE_POP_GROUP:
            return "pop group";
        case GL_DEBUG_TYPE_OTHER:
            return "other";
        case GL_DEBUG_TYPE_ERROR:
            return "error";
        default:
            ;
    }

    return "unkown";
}

extern "C" void APIENTRY
debug_callback(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei , const GLchar* message, GLvoid* )
{
    SAL_WARN("vcl.opengl", "OpenGL debug message: source: " << getSourceString(source) << ", type: "
            << getTypeString(type) << ", id: " << id << ", severity: " << getSeverityString(severity) << " with message: " << message);
}

}

bool WinOpenGLContext::init( Window* pParent )
{
    if(mbInitialized)
        return true;

    m_pWindow.reset(pParent ? NULL : new Window(0, WB_NOBORDER|WB_NODIALOGCONTROL));
    mpWindow = pParent ? pParent : m_pWindow.get();
    m_pChildWindow = 0;
    initWindow();
    return ImplInit();
}

bool WinOpenGLContext::init(SystemChildWindow* pChildWindow)
{
    if(mbInitialized)
        return true;

    if( !pChildWindow )
        return false;

    mpWindow = pChildWindow->GetParent();
    m_pChildWindow = pChildWindow;
    initWindow();
    return ImplInit();
}

bool WinOpenGLContext::ImplInit()
{
    SAL_INFO("vcl.opengl", "WinOpenGLContext::ImplInit----start");
    if(m_pWindow)
        m_pWindow->setPosSizePixel(0,0,0,0);
    m_aGLWin.Width = 0;
    m_aGLWin.Height = 0;

    m_aGLWin.hDC = GetDC(m_aGLWin.hWnd);

    PIXELFORMATDESCRIPTOR PixelFormatFront =                    // PixelFormat Tells Windows How We Want Things To Be
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                              // Version Number
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                  // Request An RGBA Format
        (BYTE)32,                       // Select Our Color Depth
        0, 0, 0, 0, 0, 0,               // Color Bits Ignored
        0,                              // No Alpha Buffer
        0,                              // Shift Bit Ignored
        0,                              // No Accumulation Buffer
        0, 0, 0, 0,                     // Accumulation Bits Ignored
        64,                             // 32 bit Z-BUFFER
        0,                              // 0 bit stencil buffer
        0,                              // No Auxiliary Buffer
        0,                              // now ignored
        0,                              // Reserved
        0, 0, 0                         // Layer Masks Ignored
    };

    //  we must check whether can set the MSAA
    int WindowPix = 0;
    bool bMultiSampleSupport = InitMultisample(PixelFormatFront, WindowPix);
    if (bMultiSampleSupport && WindowPix != 0)
    {
        m_aGLWin.bMultiSampleSupported = true;
    }
    else
    {
        WindowPix = ChoosePixelFormat(m_aGLWin.hDC, &PixelFormatFront);
    }

    if (WindowPix == 0)
    {
        SAL_WARN("vcl.opengl", "Invalid pixelformat");
        return false;
    }

    SetPixelFormat(m_aGLWin.hDC, WindowPix, &PixelFormatFront);
    m_aGLWin.hRC = wglCreateContext(m_aGLWin.hDC);
    if (m_aGLWin.hRC == NULL)
    {
        SAL_WARN("vcl.opengl", "wglCreateContext failed");
        return false;
    }

    if (!wglMakeCurrent(m_aGLWin.hDC, m_aGLWin.hRC))
    {
        SAL_WARN("vcl.opengl", "wglMakeCurrent failed: " << GetLastError());
        return false;
    }

    //rGLRender.InitOpenGL(m_aGLWin);

    static bool bGlewInit = false;
    if(!bGlewInit)
    {
        glewExperimental = GL_TRUE;
        GLenum err = glewInit();
        if (err != GLEW_OK)
        {
            SAL_WARN("vcl.opengl", "Failed to initialize GLEW: " << glewGetErrorString(err));
            return false;
        }
        else
            bGlewInit = true;
    }

#ifdef DBG_UTIL
    // only enable debug output in dbgutil build
    // somehow there are implementations where the feature is present and the function
    // pointer is still NULL
    if( GLEW_ARB_debug_output && glDebugMessageCallback )
    {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(&debug_callback, NULL);
    }

#endif

    SAL_INFO("vcl.opengl", "WinOpenGLContext::ImplInit----end");
    mbInitialized = true;
    return true;
}

void WinOpenGLContext::setWinPosAndSize(const Point &rPos, const Size& rSize)
{
    if(m_pWindow)
        m_pWindow->SetPosSizePixel(rPos, rSize);
    if( m_pChildWindow )
        m_pChildWindow->SetPosSizePixel(rPos, rSize);

    m_aGLWin.Width = rSize.Width();
    m_aGLWin.Height = rSize.Height();
}

void WinOpenGLContext::setWinSize(const Size& rSize)
{
    if(m_pWindow)
        m_pWindow->SetSizePixel(rSize);
    if( m_pChildWindow )
        m_pChildWindow->SetSizePixel(rSize);

    m_aGLWin.Width = rSize.Width();
    m_aGLWin.Height = rSize.Height();
}

void WinOpenGLContext::renderToFile()
{
    int iWidth = m_aGLWin.Width;
    int iHeight = m_aGLWin.Height;
    static int nIdx = 0;
    OUString aName = OUString( "file:///home/moggi/Documents/work/output" ) + OUString::number( nIdx++ ) + ".png";
    OpenGLHelper::renderToFile(iWidth, iHeight, aName);
}

bool WinOpenGLContext::isInitialized() const
{
    return mbInitialized;
}

bool WinOpenGLContext::initWindow()
{
    if( !m_pChildWindow )
    {
        SystemWindowData winData = vcl::generateSystemWindowData(mpWindow);
        m_pChildWindow = new SystemChildWindow(mpWindow, 0, &winData, false);
        m_pChildWindowGC.reset(m_pChildWindow);
    }

    if( m_pChildWindow )
    {
        m_pChildWindow->SetMouseTransparent( true );
        m_pChildWindow->SetParentClipMode(PARENTCLIPMODE_CLIP);
        m_pChildWindow->EnableEraseBackground( false );
        m_pChildWindow->SetControlForeground();
        m_pChildWindow->SetControlBackground();
        //m_pChildWindow->EnablePaint(false);

        const SystemEnvData* sysData(m_pChildWindow->GetSystemData());
        m_aGLWin.hWnd = sysData->hWnd;
    }

    return true;
}

void WinOpenGLContext::makeCurrent()
{
    if (!wglMakeCurrent(m_aGLWin.hDC, m_aGLWin.hRC))
    {
        SAL_WARN("vcl.opengl", "WinOpenGLContext::makeCurrent(): wglMakeCurrent failed: " << GetLastError());
    }
}

void WinOpenGLContext::swapBuffers()
{
    SwapBuffers(m_aGLWin.hDC);
}

void WinOpenGLContext::sync()
{
    // nothing
}

void WinOpenGLContext::show()
{
    if (m_pChildWindow)
        m_pChildWindow->Show();
    else if (m_pWindow)
        m_pWindow->Show();
}

bool WinOpenGLContext::hasGLExtension(const char *pName) const
{
    return m_aGLWin.HasGLExtension(pName);
}

unsigned WinOpenGLContext::getWidth() const
{
    return m_aGLWin.Width;
}

unsigned WinOpenGLContext::getHeight() const
{
    return m_aGLWin.Height;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
