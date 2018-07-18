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

#include <vcl/bitmapaccess.hxx>
#include <vcl/graph.hxx>

#include <osl/thread.hxx>

#include <svdata.hxx>
#include <salgdi.hxx>
#include <salinst.hxx>

#include <opengl/framebuffer.hxx>
#include <opengl/program.hxx>
#include <opengl/texture.hxx>
#include <opengl/zone.hxx>

#include <opengl/RenderState.hxx>

#include <config_features.h>

using namespace com::sun::star;

#define MAX_FRAMEBUFFER_COUNT 30

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
    mbUseDoubleBufferedRendering(true),
    mbVCLOnly(false),
    mnFramebufferCount(0),
    mpCurrentFramebuffer(nullptr),
    mpFirstFramebuffer(nullptr),
    mpLastFramebuffer(nullptr),
    mpCurrentProgram(nullptr),
    mpRenderState(new RenderState),
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

bool OpenGLContext::init(SystemChildWindow* pChildWindow)
{
    if(mbInitialized)
        return true;

    if( !pChildWindow )
        return false;

    OpenGLZone aZone;

    mpWindow = pChildWindow->GetParent();
    m_pChildWindow = pChildWindow;
    initWindow();
    return ImplInit();
}

bool OpenGLContext::ImplInit()
{
    VCL_GL_INFO("OpenGLContext not implemented for this platform");
    return false;
}

OUString getGLString(GLenum eGlEnum)
{
    OUString sString;
    const GLubyte* pString = glGetString(eGlEnum);
    if (pString)
    {
        sString = OUString::createFromAscii(reinterpret_cast<const sal_Char*>(pString));
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

    // reset the clip region
    maClipRegion.SetEmpty();
    mpRenderState.reset(new RenderState);

    // destroy all framebuffers
    if( mpLastFramebuffer )
    {
        OpenGLFramebuffer* pFramebuffer = mpLastFramebuffer;

        makeCurrent();
        while( pFramebuffer )
        {
            OpenGLFramebuffer* pPrevFramebuffer = pFramebuffer->mpPrevFramebuffer;
            delete pFramebuffer;
            pFramebuffer = pPrevFramebuffer;
        }
        mnFramebufferCount = 0;
        mpFirstFramebuffer = nullptr;
        mpLastFramebuffer = nullptr;
    }

    // destroy all programs
    if( !maPrograms.empty() )
    {
        makeCurrent();
        maPrograms.clear();
    }

    if( isCurrent() )
        resetCurrent();

    mbInitialized = false;

    // destroy the context itself
    destroyCurrentContext();
}

SystemWindowData OpenGLContext::generateWinData(vcl::Window* /*pParent*/, bool /*bRequestLegacyContext*/)
{
    SystemWindowData aWinData;
    memset(&aWinData, 0, sizeof(aWinData));
    return aWinData;
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
    ImplSVData* pSVData = ImplGetSVData();

    // release all framebuffers from the old context so we can re-attach the
    // texture in the new context
    rtl::Reference<OpenGLContext> pCurrentCtx = pSVData->maGDIData.mpLastContext;
    if( pCurrentCtx.is() && pCurrentCtx->isCurrent() )
        pCurrentCtx->ReleaseFramebuffers();
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

rtl::Reference<OpenGLContext> OpenGLContext::getVCLContext(bool bMakeIfNecessary)
{
    ImplSVData* pSVData = ImplGetSVData();
    OpenGLContext *pContext = pSVData->maGDIData.mpLastContext;
    while( pContext )
    {
        // check if this context is usable
        if( pContext->isInitialized() && pContext->isVCLOnly() )
            break;
        pContext = pContext->mpPrevContext;
    }
    rtl::Reference<OpenGLContext> xContext;
    vcl::Window* pDefWindow = !pContext && bMakeIfNecessary ? ImplGetDefaultWindow() : nullptr;
    if (pDefWindow)
    {
        // create our magic fallback window context.
#if HAVE_FEATURE_OPENGL
        xContext = pDefWindow->GetGraphics()->GetOpenGLContext();
        assert(xContext.is());
#endif
    }
    else
        xContext = pContext;

    if( xContext.is() )
        xContext->makeCurrent();

    return xContext;
}

/*
 * We don't care what context we have, but we want one that is live,
 * ie. not reset underneath us, and is setup for VCL usage - ideally
 * not swapping context at all.
 */
void OpenGLContext::makeVCLCurrent()
{
    getVCLContext();
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

    // sync the render state with the current context
    mpRenderState->sync();
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

bool OpenGLContext::BindFramebuffer( OpenGLFramebuffer* pFramebuffer )
{
    OpenGLZone aZone;

    if( pFramebuffer != mpCurrentFramebuffer )
    {
        if( pFramebuffer )
            pFramebuffer->Bind();
        else
            OpenGLFramebuffer::Unbind();
        mpCurrentFramebuffer = pFramebuffer;
    }

    return true;
}

void OpenGLContext::AcquireDefaultFramebuffer()
{
    BindFramebuffer( nullptr );
}

OpenGLFramebuffer* OpenGLContext::AcquireFramebuffer( const OpenGLTexture& rTexture )
{
    OpenGLZone aZone;

    OpenGLFramebuffer* pFramebuffer = nullptr;
    OpenGLFramebuffer* pFreeFbo = nullptr;
    OpenGLFramebuffer* pSameSizeFbo = nullptr;

    // check if there is already a framebuffer attached to that texture
    pFramebuffer = mpLastFramebuffer;
    while( pFramebuffer )
    {
        if( pFramebuffer->IsAttached( rTexture ) )
            break;
        if( !pFreeFbo && pFramebuffer->IsFree() )
            pFreeFbo = pFramebuffer;
        if( !pSameSizeFbo &&
            pFramebuffer->GetWidth() == rTexture.GetWidth() &&
            pFramebuffer->GetHeight() == rTexture.GetHeight() )
            pSameSizeFbo = pFramebuffer;
        pFramebuffer = pFramebuffer->mpPrevFramebuffer;
    }

    // else use any framebuffer having the same size
    if( !pFramebuffer && pSameSizeFbo )
        pFramebuffer = pSameSizeFbo;

    // else use the first free framebuffer
    if( !pFramebuffer && pFreeFbo )
        pFramebuffer = pFreeFbo;

    // if there isn't any free one, create a new one if the limit isn't reached
    if( !pFramebuffer && mnFramebufferCount < MAX_FRAMEBUFFER_COUNT )
    {
        mnFramebufferCount++;
        pFramebuffer = new OpenGLFramebuffer();
        if( mpLastFramebuffer )
        {
            pFramebuffer->mpPrevFramebuffer = mpLastFramebuffer;
            mpLastFramebuffer = pFramebuffer;
        }
        else
        {
            mpFirstFramebuffer = pFramebuffer;
            mpLastFramebuffer = pFramebuffer;
        }
    }

    // last try, use any framebuffer
    // TODO order the list of framebuffers as a LRU
    if( !pFramebuffer )
        pFramebuffer = mpFirstFramebuffer;

    assert( pFramebuffer );
    BindFramebuffer( pFramebuffer );
    pFramebuffer->AttachTexture( rTexture );

    state().viewport(tools::Rectangle(Point(), Size(rTexture.GetWidth(), rTexture.GetHeight())));

    return pFramebuffer;
}

// FIXME: this method is rather grim from a perf. perspective.
// We should instead (eventually) use pointers to associate the
// framebuffer and texture cleanly.
void OpenGLContext::UnbindTextureFromFramebuffers( GLuint nTexture )
{
    OpenGLFramebuffer* pFramebuffer;

    // see if there is a framebuffer attached to that texture
    pFramebuffer = mpLastFramebuffer;
    while( pFramebuffer )
    {
        if (pFramebuffer->IsAttached(nTexture))
        {
            BindFramebuffer(pFramebuffer);
            pFramebuffer->DetachTexture();
        }
        pFramebuffer = pFramebuffer->mpPrevFramebuffer;
    }

    // Lets just check that no other context has a framebuffer
    // with this texture - that would be bad ...
    assert( !IsTextureAttachedAnywhere( nTexture ) );
}

/// Method for debugging; check texture is not already attached.
bool OpenGLContext::IsTextureAttachedAnywhere( GLuint nTexture )
{
    ImplSVData* pSVData = ImplGetSVData();
    for( auto *pCheck = pSVData->maGDIData.mpLastContext; pCheck;
               pCheck = pCheck->mpPrevContext )
    {
        for( auto pBuffer = pCheck->mpLastFramebuffer; pBuffer;
                  pBuffer = pBuffer->mpPrevFramebuffer )
        {
            if( pBuffer->IsAttached( nTexture ) )
                return true;
        }
    }
    return false;
}

void OpenGLContext::ReleaseFramebuffer( OpenGLFramebuffer* pFramebuffer )
{
    if( pFramebuffer )
        pFramebuffer->DetachTexture();
}

void OpenGLContext::ReleaseFramebuffer( const OpenGLTexture& rTexture )
{
    OpenGLZone aZone;

    if (!rTexture) // no texture to release.
        return;

    OpenGLFramebuffer* pFramebuffer = mpLastFramebuffer;

    while( pFramebuffer )
    {
        if( pFramebuffer->IsAttached( rTexture ) )
        {
            BindFramebuffer( pFramebuffer );
            pFramebuffer->DetachTexture();
            if (mpCurrentFramebuffer == pFramebuffer)
                BindFramebuffer( nullptr );
        }
        pFramebuffer = pFramebuffer->mpPrevFramebuffer;
    }
}

void OpenGLContext::ReleaseFramebuffers()
{
    OpenGLZone aZone;

    OpenGLFramebuffer* pFramebuffer = mpLastFramebuffer;
    while( pFramebuffer )
    {
        if (!pFramebuffer->IsFree())
        {
            BindFramebuffer( pFramebuffer );
            pFramebuffer->DetachTexture();
        }
        pFramebuffer = pFramebuffer->mpPrevFramebuffer;
    }
    BindFramebuffer( nullptr );
}

OpenGLProgram* OpenGLContext::GetProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const rtl::OString& preamble )
{
    OpenGLZone aZone;

    // We cache the shader programs in a per-process run-time cache
    // based on only the names and the preamble. We don't expect
    // shader source files to change during the lifetime of a
    // LibreOffice process.
    rtl::OString aNameBasedKey = OUStringToOString(rVertexShader + "+" + rFragmentShader, RTL_TEXTENCODING_UTF8) + "+" + preamble;
    if( !aNameBasedKey.isEmpty() )
    {
        ProgramCollection::iterator it = maPrograms.find( aNameBasedKey );
        if( it != maPrograms.end() )
            return it->second.get();
    }

    // Binary shader programs are cached persistently (between
    // LibreOffice process instances) based on a hash of their source
    // code, as the source code can and will change between
    // LibreOffice versions even if the shader names don't change.
    rtl::OString aPersistentKey = OpenGLHelper::GetDigest( rVertexShader, rFragmentShader, preamble );
    std::shared_ptr<OpenGLProgram> pProgram = std::make_shared<OpenGLProgram>();
    if( !pProgram->Load( rVertexShader, rFragmentShader, preamble, aPersistentKey ) )
        return nullptr;

    maPrograms.insert(std::make_pair(aNameBasedKey, pProgram));
    return pProgram.get();
}

OpenGLProgram* OpenGLContext::UseProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const OString& preamble )
{
    OpenGLZone aZone;

    OpenGLProgram* pProgram = GetProgram( rVertexShader, rFragmentShader, preamble );

    if (pProgram && pProgram == mpCurrentProgram)
    {
        VCL_GL_INFO("Context::UseProgram: Reusing existing program " << pProgram->Id());
        pProgram->Reuse();
        return pProgram;
    }

    mpCurrentProgram = pProgram;

    if (!mpCurrentProgram)
    {
        SAL_WARN("vcl.opengl", "OpenGLContext::UseProgram: mpCurrentProgram is 0");
        return nullptr;
    }

    mpCurrentProgram->Use();

    return mpCurrentProgram;
}

void OpenGLContext::UseNoProgram()
{
    if( mpCurrentProgram == nullptr )
        return;

    mpCurrentProgram = nullptr;
    glUseProgram( 0 );
    CHECK_GL_ERROR();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
