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


#include <math.h>

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
    const int WIDTH = 1000, HEIGHT = 800;

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
private:

protected:
    double mnStartTime;
    int mnPaintCount;

public:
    Graphic maGraphic;
    Bitmap *mpBitmap;
    FixedBitmap *mpFixedBitmap;

    MyWorkWindow( vcl::Window* pParent, WinBits nWinStyle );

    void LoadGraphic( const OUString& sImageFile );

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

    void LoadTexture();

    MyOpenGLWorkWindow( vcl::Window* pParent, WinBits nWinStyle );

    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;
};

MyWorkWindow::MyWorkWindow( vcl::Window* pParent, WinBits nWinStyle )
    : WorkWindow(pParent, nWinStyle)
    , mpBitmap(NULL)
    , mpFixedBitmap(NULL)
{
    mnPaintCount = 0;
    mnStartTime = getTimeNow();
    EnableInput();
}

void MyWorkWindow::LoadGraphic( const OUString& sImageFile )
{
    SvFileStream aFileStream( sImageFile, STREAM_READ );
    GraphicFilter aGraphicFilter(false);
    if (aGraphicFilter.ImportGraphic(maGraphic, sImageFile, aFileStream) != 0)
    {
        SAL_WARN("vcl.icontest", "Could not import image '" << sImageFile << "'");
        return;
    }
}

void MyWorkWindow::Paint( const Rectangle& rRect )
{
    std::cout << "==> Paint! " << mnPaintCount++ << " (vcl) " << GetSizePixel() << " " << getTimeNow() - mnStartTime << std::endl;

    Size aGraphicSize( maGraphic.GetSizePixel() );
    float aspect = ((float) aGraphicSize.Width()) / aGraphicSize.Height();
    Size aSize;
    if( aspect >= ((float) WIDTH) / HEIGHT )
        aSize = Size( WIDTH, HEIGHT/aspect );
    else
        aSize = Size( WIDTH * aspect, HEIGHT );
    aSize.setWidth( aSize.Width() * (1 + (0.1*sin(mnPaintCount/60.))) );
    aSize.setHeight( aSize.Height() * (1 + (0.1*sin(mnPaintCount/50.))) );

    Bitmap aEmpty;
    mpFixedBitmap->SetBitmap( aEmpty );
    GraphicConversionParameters aConv( aSize );
    mpBitmap = new Bitmap( maGraphic.GetBitmap( aConv ) );
    mpFixedBitmap->SetBitmap( *mpBitmap );
    mpFixedBitmap->SetSizePixel( aSize );

    WorkWindow::Paint( rRect );

    if (mnPaintCount == 100)
        Application::Quit();

    Invalidate( INVALIDATE_CHILDREN );
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

void MyOpenGLWorkWindow::Paint( const Rectangle& )
{
    std::cout << "==> Paint! "<< mnPaintCount++ << " (OpenGL) " << GetSizePixel() << " " << getTimeNow() - mnStartTime << std::endl;
    OpenGLContext& aCtx = mpOpenGLWindow->getContext();
    aCtx.requestLegacyContext();
    CHECK_GL_ERROR();

    if (!mbHaveTexture)
        LoadTexture();

    aCtx.setWinSize( Size( WIDTH+1, HEIGHT+1 ) );
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
    glVertex3f(0, 1 + (0.1*sin(mnPaintCount/50.)), 0);
    glTexCoord2f(1, 1);
    glVertex3f(1 + (0.1*sin(mnPaintCount/60.)), 1 + (0.1*sin(mnPaintCount/50.)), 0);
    glTexCoord2f(1, 0);
    glVertex3f(1 + (0.1*sin(mnPaintCount/60.)), 0, 0);
    glEnd();
    CHECK_GL_ERROR();

    glPopMatrix();
    CHECK_GL_ERROR();

    aCtx.swapBuffers();
    CHECK_GL_ERROR();

    if (mnPaintCount == 100)
        Application::Quit();

    Invalidate( INVALIDATE_CHILDREN );
}

void MyWorkWindow::Resize()
{
    SAL_INFO("vcl.icontest", "Resize " << GetSizePixel());
}

class IconTestApp : public Application
{
public:
    virtual void Init() SAL_OVERRIDE;
    virtual int Main() SAL_OVERRIDE;

    IconTestApp() : nRet(EXIT_SUCCESS) {};

private:
    int nRet;

    void DoItWithVcl(const OUString& sImageFile);
    void DoItWithOpenGL(const OUString& sImageFile);
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
    if (GetCommandLineParam(0) == "vcl")
        DoItWithVcl( sImageFile );
    else
        DoItWithOpenGL( sImageFile );

    return nRet;
}

void IconTestApp::DoItWithVcl( const OUString& sImageFile)
{
    try
    {
        MyWorkWindow *pWindow = new MyWorkWindow( NULL, WB_APP | WB_STDWORK | WB_SIZEABLE | WB_CLOSEABLE | WB_CLIPCHILDREN );

        pWindow->SetText(OUString("VCL Image Test"));

        pWindow->LoadGraphic( sImageFile );
        pWindow->mpFixedBitmap = new FixedBitmap( pWindow );
        pWindow->mpFixedBitmap->SetPosPixel( Point( 0, 0 ) );
        pWindow->mpFixedBitmap->Show();

        pWindow->Hide();
        pWindow->Show();

        Execute();
    }
    catch (const uno::Exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", OUStringToOString(e.Message, osl_getThreadTextEncoding()).getStr());
        nRet = EXIT_FAILURE;
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", e.what());
        nRet = EXIT_FAILURE;
    }
}

void IconTestApp::DoItWithOpenGL(const OUString& sImageFile)
{
    try
    {
        MyOpenGLWorkWindow *pWindow = new MyOpenGLWorkWindow( NULL, WB_APP | WB_STDWORK | WB_SIZEABLE | WB_CLOSEABLE | WB_CLIPCHILDREN );

        pWindow->SetText(OUString("OpenGL Image Test"));

        pWindow->LoadGraphic( sImageFile );

        Size aGraphicSize( pWindow->maGraphic.GetSizePixel() );
        float aspect = ((float) aGraphicSize.Width()) / aGraphicSize.Height();
        SAL_INFO("vcl.icontest", sImageFile << ": size: " << aGraphicSize << " aspect: " << aspect);

        pWindow->Hide();
        pWindow->Show();

        Execute();
    }
    catch (const uno::Exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", OUStringToOString(e.Message, osl_getThreadTextEncoding()).getStr());
        nRet = EXIT_FAILURE;
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", e.what());
        nRet = EXIT_FAILURE;
    }
}

void vclmain::createApplication()
{
    static IconTestApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
