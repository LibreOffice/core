/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_OPENGL_OPENGLCONTEXT_HXX
#define INCLUDED_VCL_OPENGL_OPENGLCONTEXT_HXX

#include <string.h>

#include <epoxy/gl.h>

#include <vcl/dllapi.h>
#include <vcl/window.hxx>
#include <tools/gen.hxx>
#include <vcl/syschild.hxx>
#include <rtl/crc.h>
#include <rtl/ref.hxx>

#include <map>
#include <memory>
#include <set>
#include <unordered_map>

class OpenGLFramebuffer;
class OpenGLProgram;
class OpenGLTexture;
class SalGraphicsImpl;
class OpenGLTests;
class RenderState;

/// Holds the information of our new child window
struct VCL_DLLPUBLIC GLWindow
{
    unsigned int            Width;
    unsigned int            Height;
    bool bMultiSampleSupported;

    GLWindow()
        : Width(0)
        , Height(0)
        , bMultiSampleSupported(false)
    {
    }

    virtual bool Synchronize(bool bOnoff) const;

    virtual ~GLWindow();
};

struct VCL_DLLPUBLIC OpenGLCapabilitySwitch
{
    bool mbLimitedShaderRegisters;

    OpenGLCapabilitySwitch()
        : mbLimitedShaderRegisters(false)
    {}
};

class VCL_DLLPUBLIC OpenGLContext
{
    friend class OpenGLTests;
protected:
    OpenGLContext();
public:
    static rtl::Reference<OpenGLContext> Create();
    virtual ~OpenGLContext();
    void acquire() { mnRefCount++; }
    void release() { if ( --mnRefCount == 0 ) delete this; }
    void dispose();

    void requestLegacyContext();

    bool init(vcl::Window* pParent);
    bool init(SystemChildWindow* pChildWindow);

    void reset();

    // use these methods right after setting a context to make sure drawing happens
    // in the right FBO (default one is for onscreen painting)
    bool               BindFramebuffer( OpenGLFramebuffer* pFramebuffer );
    void               AcquireDefaultFramebuffer();
    OpenGLFramebuffer* AcquireFramebuffer( const OpenGLTexture& rTexture );
    static void        ReleaseFramebuffer( OpenGLFramebuffer* pFramebuffer );
    void UnbindTextureFromFramebuffers( GLuint nTexture );
    static bool        IsTextureAttachedAnywhere( GLuint nTexture );

    void               ReleaseFramebuffer( const OpenGLTexture& rTexture );
    void               ReleaseFramebuffers();

    // retrieve a program from the cache or compile/link it
    OpenGLProgram*      GetProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const OString& preamble = "" );
    OpenGLProgram*      UseProgram( const OUString& rVertexShader, const OUString& rFragmentShader, const OString& preamble = "" );
    void                UseNoProgram();

    RenderState& state()
    {
        return *mpRenderState;
    }

    OpenGLCapabilitySwitch& getOpenGLCapabilitySwitch()
    {
        return maOpenGLCapabilitySwitch;
    }

    /// Is this GL context the current context ?
    virtual bool isCurrent();
    /// Is any GL context the current context ?
    virtual bool isAnyCurrent();
    /// release bound resources from the current context
    static void clearCurrent();
    /// release contexts etc. before (potentially) allowing another thread run.
    static void prepareForYield();
    /// Is there a current GL context ?
    static bool hasCurrent();

    /// make a VCL context (any context) current, create it if necessary.
    static void makeVCLCurrent();
    /// fetch any VCL context, creating one if bMakeIfNecessary is set.
    static rtl::Reference<OpenGLContext> getVCLContext(bool bMakeIfNecessary = true);
    /// make this GL context current - so it is implicit in subsequent GL calls
    virtual void makeCurrent();
    /// Put this GL context to the end of the context list.
    void registerAsCurrent();
    /// reset the GL context so this context is not implicit in subsequent GL calls.
    virtual void resetCurrent();
    /// unbind the GL_FRAMEBUFFER to its default state, needed for gtk3
    virtual void restoreDefaultFramebuffer();
    virtual void swapBuffers();
    virtual void sync();
    void show();

    void setWinPosAndSize(const Point &rPos, const Size& rSize);
    virtual const GLWindow& getOpenGLWindow() const = 0;

    SystemChildWindow* getChildWindow();
    const SystemChildWindow* getChildWindow() const;

    bool isInitialized()
    {
        return mbInitialized;
    }

    /// VCL promiscuously re-uses its own contexts:
    void setVCLOnly() { mbVCLOnly = true; }
    bool isVCLOnly() { return mbVCLOnly; }

    virtual SystemWindowData generateWinData(vcl::Window* pParent, bool bRequestLegacyContext);

private:
    virtual void initWindow();
    virtual void destroyCurrentContext();
    virtual void adjustToNewSize();

protected:
    bool InitGL();
    static void InitGLDebugging();
    static void InitChildWindow(SystemChildWindow *pChildWindow);
    static void BuffersSwapped();
    virtual GLWindow& getModifiableOpenGLWindow() = 0;
    virtual bool ImplInit();

    VclPtr<vcl::Window> m_xWindow;
    VclPtr<vcl::Window> mpWindow; //points to m_pWindow or the parent window, don't delete it
    VclPtr<SystemChildWindow> m_pChildWindow;
    bool mbInitialized;
    int  mnRefCount;
    bool mbRequestLegacyContext;
    bool mbUseDoubleBufferedRendering;
    bool mbVCLOnly;

    int mnFramebufferCount;
    OpenGLFramebuffer* mpCurrentFramebuffer;
    OpenGLFramebuffer* mpFirstFramebuffer;
    OpenGLFramebuffer* mpLastFramebuffer;

    OpenGLCapabilitySwitch maOpenGLCapabilitySwitch;

private:
    struct ProgramHash
    {
        size_t operator()( const rtl::OString& aDigest ) const
        {
            return static_cast<size_t>( rtl_crc32( 0, aDigest.getStr(), aDigest.getLength() ) );
        }
    };

    typedef std::unordered_map< rtl::OString, std::shared_ptr<OpenGLProgram>, ProgramHash > ProgramCollection;
    ProgramCollection maPrograms;
    OpenGLProgram* mpCurrentProgram;

    std::unique_ptr<RenderState> mpRenderState;

public:
    vcl::Region maClipRegion;

    // Don't hold references to ourselves:
    OpenGLContext *mpPrevContext;
    OpenGLContext *mpNextContext;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
