/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * =======================================================================
 *
 * This is a quick hack to test some stuff. Work in progress. Don't touch
 * and don't bother inspecting too closely.
 *
 * =======================================================================
 */


#include <GL/glew.h>

#include <glm/gtx/bit.hpp>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <osl/file.hxx>
#include <vcl/builder.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/image.hxx>
#include <vcl/openglwin.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/vclmain.hxx>
#include <vcl/wrkwin.hxx>

using namespace com::sun::star;

namespace {
    const int WIDTH = 1024, HEIGHT = 768;

    double getTimeNow()
    {
        TimeValue aValue;
        osl_getSystemTime(&aValue);
        return (double)aValue.Seconds +
            (double)aValue.Nanosec / (1000*1000*1000);
    }

}

class MyWorkWindow : public WorkWindow
{
protected:
    double  nStartTime;
    int     nPaintCount;
    Graphic maGraphic;
    Size    maDestinationSize;

public:
    MyWorkWindow( vcl::Window* pParent, WinBits nWinStyle );

    virtual void LoadGraphic( const OUString &sImageFile );
    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void Resize() SAL_OVERRIDE;
};

class MyOpenGLWorkWindow : public MyWorkWindow
{
public:
    bool mbHaveTexture;
    OpenGLWindow *mpOpenGLWindow;
    GLuint mnTextureName;
    float mnTextureAspect;

    void LoadTexture(); // deferred
    virtual void LoadGraphic( const OUString &sImageFile ) SAL_OVERRIDE;

    MyOpenGLWorkWindow( vcl::Window* pParent, WinBits nWinStyle );

    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;
};


// ------------------ existing DrawingLayer behavior ------------------

void MyWorkWindow::Paint( const Rectangle& /* rRect */ )
{
    OutputDevice &rDev = *this;

    // yes indeed drawinglayer re-scales the image per render etc.
    BitmapEx aScaledBitamp( maGraphic.GetBitmapEx() );
    aScaledBitamp.Scale( maDestinationSize, BMP_SCALE_SUPER);

    std::cerr << "==> Paint! " << nPaintCount++ << " (vcl) " << GetSizePixel() << " " << getTimeNow() - nStartTime << " image of size " << maGraphic.GetBitmapEx().GetSizePixel() << " scale to size " << maDestinationSize << std::endl;

    rDev.DrawBitmapEx( Point( 0, 0 ), aScaledBitamp );

    Invalidate( INVALIDATE_CHILDREN ); // trigger re-render
}

// ------------------ quick hack of an openGL equivalent ------------------

void MyOpenGLWorkWindow::Paint( const Rectangle& )
{
    std::cerr << "==> Paint! "<< nPaintCount++ << " (OpenGL) " << GetSizePixel() << " " << getTimeNow() - nStartTime << std::endl;
    OpenGLContext& aCtx = mpOpenGLWindow->getContext();
    aCtx.requestLegacyContext();
    CHECK_GL_ERROR();

    if (!mbHaveTexture)
        LoadTexture();

    aCtx.setWinSize( Size( WIDTH, HEIGHT ) );
    CHECK_GL_ERROR();

    aCtx.makeCurrent();
    CHECK_GL_ERROR();

    glViewport( 0, 0, WIDTH, HEIGHT );
    CHECK_GL_ERROR();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERROR();

    glBindTexture(GL_TEXTURE_2D, mnTextureName);
    CHECK_GL_ERROR();

    glPushMatrix();
    CHECK_GL_ERROR();

    glTranslatef(-1, -1, 0);
    glScalef(2, 2, 2);

    if (mnTextureAspect >= ((float) WIDTH) / HEIGHT)
        glScalef(1, 1/mnTextureAspect, 1);
    else
        glScalef(1*mnTextureAspect, 1, 1);
    CHECK_GL_ERROR();

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(0, 0, 0);
    glTexCoord2f(0, 1);
    glVertex3f(0, 1, 0);
    glTexCoord2f(1, 1);
    glVertex3f(1, 1, 0);
    glTexCoord2f(1, 0);
    glVertex3f(1, 0, 0);
    glEnd();
    CHECK_GL_ERROR();

    glPopMatrix();
    CHECK_GL_ERROR();

    aCtx.swapBuffers();
    CHECK_GL_ERROR();

    Invalidate( INVALIDATE_CHILDREN ); // trigger re-render
}

// ------------------ bootstrapping foo ------------------

MyWorkWindow::MyWorkWindow( vcl::Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle )
{
    nPaintCount = 0;
    nStartTime = getTimeNow();
    EnableInput();
}


MyOpenGLWorkWindow::MyOpenGLWorkWindow( vcl::Window* pParent, WinBits nWinStyle ) :
    MyWorkWindow( pParent, nWinStyle )
{
    mbHaveTexture = false;
    mpOpenGLWindow = new OpenGLWindow( this );
    mpOpenGLWindow->SetSizePixel( Size( WIDTH, HEIGHT ) );
    mpOpenGLWindow->Show();
    mpOpenGLWindow->EnableInput();
}

void MyOpenGLWorkWindow::LoadGraphic(const OUString &sImageFile )
{
    MyWorkWindow::LoadGraphic( sImageFile );
    // do more one-off work here ? but needs an OpenGL context ...
}

void MyOpenGLWorkWindow::LoadTexture()
{
    mbHaveTexture = true;

    glEnable(GL_TEXTURE_2D);
    CHECK_GL_ERROR();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    CHECK_GL_ERROR();

    glGenTextures( 1, &mnTextureName );
    CHECK_GL_ERROR();

    glBindTexture(GL_TEXTURE_2D, mnTextureName);
    CHECK_GL_ERROR();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    CHECK_GL_ERROR();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CHECK_GL_ERROR();

    BitmapEx aBitmap( maGraphic.GetBitmapEx( ) );
    Size aBitmapSize( aBitmap.GetSizePixel() );

    GLint maxTexSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    CHECK_GL_ERROR();

    SAL_INFO("vcl.icontest", "GL_MAX_TEXTURE_SIZE: " << maxTexSize);

    if (aBitmapSize.Width() > maxTexSize || aBitmapSize.Height() > maxTexSize)
    {
        Size aNewSize(aBitmapSize);
        if (aNewSize.Width() > maxTexSize)
        {
            aNewSize.setHeight(aNewSize.Height() * (((float) maxTexSize) / aNewSize.Width()));
            aNewSize.setWidth(maxTexSize);
        }
        if (aNewSize.Height() > maxTexSize)
        {
            aNewSize.setWidth(aNewSize.Width() * (((float) maxTexSize) / aNewSize.Height()));
            aNewSize.setHeight(maxTexSize);
        }
        SAL_INFO("vcl.icontest", "Scaling to " << aNewSize);
        aBitmap.Scale(aNewSize, BMP_SCALE_SUPER);
        aBitmapSize = aNewSize;
    }

    SAL_INFO("vcl.icontest", "GLEW_ARB_texture_non_power_of_two: " << (GLEW_ARB_texture_non_power_of_two ? "YES" : "NO"));

    GLsizei texWidth(aBitmapSize.Width()), texHeight(aBitmapSize.Height());

    mnTextureAspect = ((float) aBitmapSize.Width()) / aBitmapSize.Height();

    if (!GLEW_ARB_texture_non_power_of_two)
    {
        texWidth = texHeight = std::max(aBitmapSize.Width(), aBitmapSize.Height());
        if (!glm::isPowerOfTwo(texWidth))
        {
            texWidth = glm::powerOfTwoAbove(texWidth);
            texHeight = texWidth;
        }

        aBitmap.Expand(texWidth - aBitmapSize.Width(), texHeight - aBitmapSize.Height());

        mnTextureAspect = 1;
    }

    SAL_INFO("vcl.icontest", "Texture size: " << texWidth << "x" << texHeight);

    GLubyte *buffer = new GLubyte[texWidth * texHeight * 4];
    OpenGLHelper::ConvertBitmapExToRGBATextureBuffer( aBitmap, buffer, true );

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 texWidth, texHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 buffer);
    CHECK_GL_ERROR();

    delete[] buffer;
}

void MyWorkWindow::Resize()
{
    SAL_INFO("vcl.icontest", "Resize " << GetSizePixel());
}

class IconTestApp : public Application
{
public:
    virtual void Init() SAL_OVERRIDE;
    virtual int  Main() SAL_OVERRIDE;
    IconTestApp() : nRet(EXIT_SUCCESS) {};

private:
    int nRet;

    MyWorkWindow * CreateWithVcl();
    MyWorkWindow * CreateWithOpenGL();
};

void IconTestApp::Init()
{
    nRet = EXIT_SUCCESS;

    uno::Reference<uno::XComponentContext> xContext =
        cppu::defaultBootstrap_InitialComponentContext();
    uno::Reference<lang::XMultiComponentFactory> xFactory =
        xContext->getServiceManager();
    uno::Reference<lang::XMultiServiceFactory> xSFactory =
        uno::Reference<lang::XMultiServiceFactory> (xFactory, uno::UNO_QUERY_THROW);
    comphelper::setProcessServiceFactory(xSFactory);

    // Create UCB (for backwards compatibility, in case some code still uses
    // plain createInstance w/o args directly to obtain an instance):
    ::ucb::UniversalContentBroker::create(
        comphelper::getProcessComponentContext() );
}

int IconTestApp::Main()
{
    if (GetCommandLineParamCount() != 2 ||
        (GetCommandLineParam(0) != "vcl" &&
         GetCommandLineParam(0) != "opengl"))
    {
        fprintf(stderr, "Usage: imagetest [vcl|opengl] image\n");
        return EXIT_FAILURE;
    }
    OUString sImageFile( GetCommandLineParam( 1 ) );
    MyWorkWindow *pWindow;

    if (GetCommandLineParam(0) == "vcl")
        pWindow = CreateWithVcl();
    else
        pWindow = CreateWithOpenGL();

    if (!pWindow)
        return EXIT_FAILURE;

    pWindow->LoadGraphic( sImageFile );

    pWindow->Hide();
    pWindow->Show();

    Execute();

    return nRet;
}

void MyWorkWindow::LoadGraphic( const OUString &sImageFile )
{
    try
    {
        SvFileStream aFileStream( sImageFile, STREAM_READ );
        GraphicFilter aGraphicFilter(false);
        if (aGraphicFilter.ImportGraphic(maGraphic, sImageFile, aFileStream) != 0)
        {
            SAL_WARN("vcl.icontest", "Could not import image '" << sImageFile << "'");
            return;
        }
        // destination size
        Size aGraphicSize( maGraphic.GetSizePixel() );
        float aspect = ((float) aGraphicSize.Width()) / aGraphicSize.Height();
        std::cerr << "icontest" << sImageFile << ": size: " << aGraphicSize << " aspect: " << aspect;
        if( aspect >= ((float) WIDTH) / HEIGHT )
            maDestinationSize = Size( WIDTH, HEIGHT/aspect );
        else
            maDestinationSize = Size( WIDTH * aspect, HEIGHT );
    }
    catch (const uno::Exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", OUStringToOString(e.Message, osl_getThreadTextEncoding()).getStr());
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", e.what());
    }
}

#define WINDOW_MASK ( WB_APP | WB_STDWORK | WB_SIZEABLE | WB_CLOSEABLE | WB_CLIPCHILDREN )

MyWorkWindow *IconTestApp::CreateWithVcl()
{
    MyWorkWindow *pWindow;

    pWindow = new MyWorkWindow( NULL, WINDOW_MASK );
    pWindow->SetText(OUString("VCL Image Test"));

    pWindow->EnableChildTransparentMode( false );
    pWindow->SetParentClipMode( 0 );
    pWindow->SetPaintTransparent( false );

    return pWindow;
}

MyWorkWindow * IconTestApp::CreateWithOpenGL()
{
    MyOpenGLWorkWindow *pWindow;

    pWindow = new MyOpenGLWorkWindow( NULL, WINDOW_MASK );
    pWindow->SetText(OUString("OpenGL Image Test"));

    return pWindow;
}

void vclmain::createApplication()
{
    static IconTestApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
