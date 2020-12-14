/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <chrono>

#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/opengl/OpenGLWrapper.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#include <osl/thread.hxx>
#include <sal/log.hxx>

#include <svdata.hxx>
#include <salgdi.hxx>
#include <salinst.hxx>

#include <opengl/zone.hxx>

#include <config_features.h>

using namespace com::sun::star;

static sal_Int64 nBufferSwapCounter = 0;

GLWindow::~GLWindow()
{
}

bool GLWindow::Synchronize(bool /*bOnoff*/) const
{
    return false;
}

OpenGLContext::OpenGLContext():
    mpWindow(nullptr),
    m_pChildWindow(nullptr),
    mbInitialized(false),
    mnRefCount(0),
    mbRequestLegacyContext(false),
    mpPrevContext(nullptr),
    mpNextContext(nullptr)
{
    VCL_GL_INFO("new context: " << this);

    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maGDIData.mpLastContext )
    {
        pSVData->maGDIData.mpLastContext->mpNextContext = this;
        mpPrevContext = pSVData->maGDIData.mpLastContext;
    }
    pSVData->maGDIData.mpLastContext = this;

    // FIXME: better hope we call 'makeCurrent' soon to preserve
    // the invariant that the last item is the current context.
}

OpenGLContext::~OpenGLContext()
{
    assert (mnRefCount == 0);

    mnRefCount = 1; // guard the shutdown paths.
    VCL_GL_INFO("delete context: " << this);

    reset();

    ImplSVData* pSVData = ImplGetSVData();
    if( mpPrevContext )
        mpPrevContext->mpNextContext = mpNextContext;
    if( mpNextContext )
        mpNextContext->mpPrevContext = mpPrevContext;
    else
        pSVData->maGDIData.mpLastContext = mpPrevContext;

    m_pChildWindow.disposeAndClear();
    assert (mnRefCount == 1);
}

// release associated child-window if we have one
void OpenGLContext::dispose()
{
    reset();
    m_pChildWindow.disposeAndClear();
}

rtl::Reference<OpenGLContext> OpenGLContext::Create()
{
    return rtl::Reference<OpenGLContext>(ImplGetSVData()->mpDefInst->CreateOpenGLContext());
}

void OpenGLContext::requestLegacyContext()
{
    mbRequestLegacyContext = true;
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

    return "unknown";
}

extern "C" void
#if defined _WIN32
APIENTRY
#endif
debug_callback(GLenum source, GLenum type, GLuint id,
               GLenum severity, GLsizei , const GLchar* message,
               const GLvoid*)
{
    // ignore Nvidia's 131218: "Program/shader state performance warning: Fragment Shader is going to be recompiled because the shader key based on GL state mismatches."
    // the GLSL compiler is a bit too aggressive in optimizing the state based on the current OpenGL state

    // ignore 131185: "Buffer detailed info: Buffer object x (bound to GL_ARRAY_BUFFER_ARB,
    // usage hint is GL_STATIC_DRAW) will use VIDEO memory as the source for buffer object operations."
    if (id == 131218 || id == 131185)
        return;

    SAL_WARN("vcl.opengl", "OpenGL debug message: source: " << getSourceString(source) << ", type: "
            << getTypeString(type) << ", id: " << id << ", severity: " << getSeverityString(severity) << ", with message: " << message);
}

}

#endif

bool OpenGLContext::init( vcl::Window* pParent )
{
    if(mbInitialized)
        return true;

    OpenGLZone aZone;

    m_xWindow.reset(pParent ? nullptr : VclPtr<vcl::Window>::Create(nullptr, WB_NOBORDER|WB_NODIALOGCONTROL));
    mpWindow = pParent ? pParent : m_xWindow.get();
    if(m_xWindow)
        m_xWindow->setPosSizePixel(0,0,0,0);
    //tdf#108069 we may be initted twice, so dispose earlier effort
    m_pChildWindow.disposeAndClear();
    initWindow();
    return ImplInit();
}

bool OpenGLContext::ImplInit()
{
    VCL_GL_INFO("OpenGLContext not implemented for this platform");
    return false;
}

static OUString getGLString(GLenum eGlEnum)
{
    OUString sString;
    const GLubyte* pString = glGetString(eGlEnum);
    if (pString)
    {
        sString = OUString::createFromAscii(reinterpret_cast<const char*>(pString));
    }

    CHECK_GL_ERROR();
    return sString;
}

bool OpenGLContext::InitGL()
{
    VCL_GL_INFO("OpenGLContext::ImplInit----end");
    VCL_GL_INFO("Vendor: " << getGLString(GL_VENDOR) << " Renderer: " << getGLString(GL_RENDERER) << " GL version: " << OpenGLHelper::getGLVersion());
    mbInitialized = true;

    // I think we need at least GL 3.0
    if (epoxy_gl_version() < 30)
    {
        SAL_WARN("vcl.opengl", "We don't have at least OpenGL 3.0");
        return false;
    }

    // Check that some "optional" APIs that we use unconditionally are present
    if (!glBindFramebuffer)
    {
        SAL_WARN("vcl.opengl", "We don't have glBindFramebuffer");
        return false;
    }

    return true;
}

void OpenGLContext::InitGLDebugging()
{
#ifdef DBG_UTIL
    // only enable debug output in dbgutil build
    if (epoxy_has_gl_extension("GL_ARB_debug_output"))
    {
        OpenGLZone aZone;

        if (glDebugMessageCallbackARB)
        {
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
            glDebugMessageCallbackARB(&debug_callback, nullptr);

#ifdef GL_DEBUG_SEVERITY_NOTIFICATION_ARB
            // Ignore i965’s shader compiler notification flood.
            glDebugMessageControlARB(GL_DEBUG_SOURCE_SHADER_COMPILER_ARB, GL_DEBUG_TYPE_OTHER_ARB, GL_DEBUG_SEVERITY_NOTIFICATION_ARB, 0, nullptr, true);
#endif
        }
        else if ( glDebugMessageCallback )
        {
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(&debug_callback, nullptr);

            // Ignore i965’s shader compiler notification flood.
            glDebugMessageControl(GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DEBUG_TYPE_OTHER, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, true);
        }
    }

    // Test hooks for inserting tracing messages into the stream
    VCL_GL_INFO("LibreOffice GLContext initialized");
#endif
}

void OpenGLContext::restoreDefaultFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLContext::setWinPosAndSize(const Point &rPos, const Size& rSize)
{
    if (m_xWindow)
        m_xWindow->SetPosSizePixel(rPos, rSize);
    if (m_pChildWindow)
        m_pChildWindow->SetPosSizePixel(rPos, rSize);

    GLWindow& rGLWin = getModifiableOpenGLWindow();
    rGLWin.Width = rSize.Width();
    rGLWin.Height = rSize.Height();
    adjustToNewSize();
}

void OpenGLContext::adjustToNewSize()
{
    const GLWindow& rGLWin = getOpenGLWindow();
    glViewport(0, 0, rGLWin.Width, rGLWin.Height);
}

void OpenGLContext::InitChildWindow(SystemChildWindow *pChildWindow)
{
    pChildWindow->SetMouseTransparent(true);
    pChildWindow->SetParentClipMode(ParentClipMode::Clip);
    pChildWindow->EnableEraseBackground(false);
    pChildWindow->SetControlForeground();
    pChildWindow->SetControlBackground();
}

void OpenGLContext::initWindow()
{
}

void OpenGLContext::destroyCurrentContext()
{
    //nothing by default
}

void OpenGLContext::reset()
{
    if( !mbInitialized )
        return;

    OpenGLZone aZone;

    if( isCurrent() )
        resetCurrent();

    mbInitialized = false;

    // destroy the context itself
    destroyCurrentContext();
}

SystemWindowData OpenGLContext::generateWinData(vcl::Window* /*pParent*/, bool /*bRequestLegacyContext*/)
{
    return {};
}

bool OpenGLContext::isCurrent()
{
    (void) this; // loplugin:staticmethods
    return false;
}

void OpenGLContext::makeCurrent()
{
    if (isCurrent())
        return;

    OpenGLZone aZone;

    clearCurrent();

    // by default nothing else to do

    registerAsCurrent();
}

bool OpenGLContext::isAnyCurrent()
{
    return false;
}

bool OpenGLContext::hasCurrent()
{
    ImplSVData* pSVData = ImplGetSVData();
    rtl::Reference<OpenGLContext> pCurrentCtx = pSVData->maGDIData.mpLastContext;
    return pCurrentCtx.is() && pCurrentCtx->isAnyCurrent();
}

void OpenGLContext::clearCurrent()
{
}

void OpenGLContext::prepareForYield()
{
    ImplSVData* pSVData = ImplGetSVData();

    // release all framebuffers from the old context so we can re-attach the
    // texture in the new context
    rtl::Reference<OpenGLContext> pCurrentCtx = pSVData->maGDIData.mpLastContext;

    if ( !pCurrentCtx.is() )
        return;                 // Not using OpenGL

    SAL_INFO("vcl.opengl", "Unbinding contexts in preparation for yield");

    // Find the first context that is current and reset it.
    // Usually the last context is the current, but not in case a new
    // OpenGLContext is created already but not yet initialized.
    while (pCurrentCtx.is())
    {
        if (pCurrentCtx->isCurrent())
        {
            pCurrentCtx->resetCurrent();
            break;
        }

        pCurrentCtx = pCurrentCtx->mpPrevContext;
    }

    assert (!hasCurrent());
}

void OpenGLContext::registerAsCurrent()
{
    ImplSVData* pSVData = ImplGetSVData();

    // move the context to the end of the contexts list
    static int nSwitch = 0;
    VCL_GL_INFO("******* CONTEXT SWITCH " << ++nSwitch << " *********");
    if( mpNextContext )
    {
        if( mpPrevContext )
            mpPrevContext->mpNextContext = mpNextContext;
        mpNextContext->mpPrevContext = mpPrevContext;

        mpPrevContext = pSVData->maGDIData.mpLastContext;
        mpNextContext = nullptr;
        pSVData->maGDIData.mpLastContext->mpNextContext = this;
        pSVData->maGDIData.mpLastContext = this;
    }
}

void OpenGLContext::resetCurrent()
{
    clearCurrent();
    // by default nothing else to do
}

void OpenGLContext::swapBuffers()
{
    // by default nothing else to do
    BuffersSwapped();
}

void OpenGLContext::BuffersSwapped()
{
    nBufferSwapCounter++;

    static bool bSleep = getenv("SAL_GL_SLEEP_ON_SWAP");
    if (bSleep)
    {
        // half a second.
        osl::Thread::wait( std::chrono::milliseconds(500) );
    }
}


sal_Int64 OpenGLWrapper::getBufferSwapCounter()
{
    return nBufferSwapCounter;
}

void OpenGLContext::sync()
{
    // default is nothing
    (void) this; // loplugin:staticmethods
}

void OpenGLContext::show()
{
    if (m_pChildWindow)
        m_pChildWindow->Show();
    else if (m_xWindow)
        m_xWindow->Show();
}

SystemChildWindow* OpenGLContext::getChildWindow()
{
    return m_pChildWindow;
}

const SystemChildWindow* OpenGLContext::getChildWindow() const
{
    return m_pChildWindow;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
