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
 * This is a quick hack to test some stuff. Work in progress. Don't touch.
 *
 * =======================================================================
 */


#include <GL/glew.h>

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

class MyWorkWindow : public WorkWindow
{
public:
    MyWorkWindow( vcl::Window* pParent, WinBits nWinStyle );

    virtual void Resize() SAL_OVERRIDE;
};

class MyOpenGLWorkWindow : public MyWorkWindow
{
private:
    OpenGLWindow *mpOpenGLWindow;

public:
    MyOpenGLWorkWindow( vcl::Window* pParent, WinBits nWinStyle );

    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;

    std::vector<GLuint>maTextureName;
    std::vector<float>maTextureAspect;
};

MyWorkWindow::MyWorkWindow( vcl::Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle )
{
    EnableInput();
}

MyOpenGLWorkWindow::MyOpenGLWorkWindow( vcl::Window* pParent, WinBits nWinStyle ) :
    MyWorkWindow( pParent, nWinStyle )
{
    mpOpenGLWindow = new OpenGLWindow( this );
    mpOpenGLWindow->SetSizePixel( Size( 1000, 800 ) );
    mpOpenGLWindow->Show();
    mpOpenGLWindow->EnableInput();
}

void MyOpenGLWorkWindow::Paint( const Rectangle& )
{
    const int WIDTH = 1000, HEIGHT = 800;

    SAL_INFO("vcl.icontest", "==> Paint! (OpenGL) " << GetSizePixel());
    OpenGLContext& aCtx = mpOpenGLWindow->getContext();
    aCtx.requestLegacyContext();
    aCtx.setWinSize( Size( WIDTH, HEIGHT ) );

    CHECK_GL_ERROR();

    aCtx.makeCurrent();
    CHECK_GL_ERROR();

    Size aSize(WIDTH, HEIGHT);
    glViewport( 0, 0, aSize.Width(), aSize.Height() );

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERROR();

    float nThumbWidth = 2.0 / (WIDTH / 100.0);
    float nThumbHeight = 2.0 / (HEIGHT / 100.0);

    float nStepX = 2.0 / (WIDTH / (100.0 + 10));
    float nStepY = 2.0 / (HEIGHT / (100.0 + 10));

    float nX = -1, nY = -1;

    for (size_t i = 0; i < maTextureName.size(); ++i)
    {
        glBindTexture(GL_TEXTURE_2D, maTextureName[i]);
        CHECK_GL_ERROR();

        glPushMatrix();
        CHECK_GL_ERROR();

        glTranslatef(nX, nY, 0);

        if (maTextureAspect[i] >= 1)
            glScalef(1, 1/maTextureAspect[i], 1);
        else
            glScalef(1*maTextureAspect[i], 1, 1);
        CHECK_GL_ERROR();

        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex3f(0, 0, 0);
        glTexCoord2f(0, 1);
        glVertex3f(0, nThumbHeight, 0);
        glTexCoord2f(1, 1);
        glVertex3f(nThumbWidth, nThumbHeight, 0);
        glTexCoord2f(1, 0);
        glVertex3f(nThumbWidth, 0, 0);
        glEnd();
        CHECK_GL_ERROR();

        glPopMatrix();
        CHECK_GL_ERROR();

        nX += nStepX;
        if (nX + nThumbWidth >= 1)
        {
            nX = -1;
            nY += nStepY;
        }
    }

    aCtx.swapBuffers();
    CHECK_GL_ERROR();
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

    void DoItWithVcl(std::vector<OUString>& aImageFiles);
    void DoItWithOpenGL(std::vector<OUString>& aImageFiles);
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
    if (GetCommandLineParamCount() < 2 ||
        (GetCommandLineParam(0) != "vcl" &&
         GetCommandLineParam(0) != "opengl"))
    {
        fprintf(stderr, "Usage: imagetest [vcl|opengl] directory ...\n");
        return EXIT_FAILURE;
    }
    std::vector<OUString> aImageFiles;
    for (int i = 1; i < GetCommandLineParamCount(); ++i)
    {
        OUString aDirURL;
        osl::File::getFileURLFromSystemPath(GetCommandLineParam(i), aDirURL);

        osl::Directory aDirectory(aDirURL);
        if (aDirectory.open() != osl::FileBase::E_None)
            continue;

        while (true)
        {
            osl::DirectoryItem aDirItem;
            if (aDirectory.getNextItem(aDirItem, SAL_MAX_UINT32) != osl::FileBase::E_None)
                break;
            osl::FileStatus aFileStatus( osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_FileURL);
            if (aDirItem.getFileStatus(aFileStatus) != osl::FileBase::E_None ||
                aFileStatus.getFileType() != osl::FileStatus::Regular)
                continue;
            OUString aFileURL(aFileStatus.getFileURL());
            aImageFiles.push_back(aFileURL);
        }
        aDirectory.close();
    }

    if (aImageFiles.empty())
    {
        fprintf(stderr, "No images found\n");
        return EXIT_FAILURE;
    }

    if (GetCommandLineParam(0) == "vcl")
        DoItWithVcl(aImageFiles);
    else
        DoItWithOpenGL(aImageFiles);

    return nRet;
}

void IconTestApp::DoItWithVcl(std::vector<OUString>& aImageFiles)
{
    try
    {
        MyWorkWindow *pWindow = new MyWorkWindow( NULL, WB_APP | WB_STDWORK | WB_SIZEABLE | WB_CLOSEABLE | WB_CLIPCHILDREN );

        pWindow->SetText(OUString("VCL Image Test"));

        Point aPos(10, 10);

        for (auto i = aImageFiles.cbegin(); i != aImageFiles.end(); ++i)
        {
            SvFileStream aFileStream( *i, STREAM_READ );
            GraphicFilter aGraphicFilter(false);
            Graphic aGraphic;
            if (aGraphicFilter.ImportGraphic(aGraphic, *i, aFileStream) != 0)
                continue;
            Size aGraphicSize( aGraphic.GetSizePixel() );
            float aspect = ((float) aGraphicSize.Width()) / aGraphicSize.Height();
            SAL_INFO("vcl.icontest", *i << ": size: " << aGraphic.GetSizeBytes() << "B, " << aGraphicSize << " (" << aspect << ")");
            Size aSize;
            if( aspect >= 1 )
                aSize = Size( 100, 100/aspect );
            else
                aSize = Size( 100 * aspect, 100 );
            GraphicConversionParameters aConv( aSize );
            Bitmap *pBitmap = new Bitmap( aGraphic.GetBitmap( aConv ) );

            FixedBitmap *pFixedBitmap = new FixedBitmap( pWindow );
            pFixedBitmap->SetBitmap( *pBitmap );
            pFixedBitmap->SetSizePixel( aSize );
            Point aShiftedPos( aPos );
            aShiftedPos.Move( (100 - aSize.Width()) / 2, (100 - aSize.Height()) / 2 );
            pFixedBitmap->SetPosPixel( aShiftedPos );
            pFixedBitmap->Show();

            aPos.Move( 100 + 10, 0);
            if ( aPos.X() > 1000 )
            {
                aPos.setX( 10 );
                aPos.setY( aPos.Y() + 100 + 10 );
            }
        }

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

void IconTestApp::DoItWithOpenGL(std::vector<OUString>& aImageFiles)
{
    try
    {
        MyOpenGLWorkWindow *pWindow = new MyOpenGLWorkWindow( NULL, WB_APP | WB_STDWORK | WB_SIZEABLE | WB_CLOSEABLE | WB_CLIPCHILDREN );

        pWindow->SetText(OUString("OpenGL Image Test"));

        glEnable(GL_TEXTURE_2D);
        CHECK_GL_ERROR();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        CHECK_GL_ERROR();

        pWindow->maTextureName.resize( aImageFiles.size() );
        pWindow->maTextureAspect.resize( aImageFiles.size() );

        glGenTextures( aImageFiles.size(), pWindow->maTextureName.data() );
        CHECK_GL_ERROR();

        int n = 0;
        for (auto i = aImageFiles.cbegin(); i != aImageFiles.end(); ++i)
        {
            SvFileStream aFileStream( *i, STREAM_READ );
            GraphicFilter aGraphicFilter(false);
            Graphic aGraphic;
            if (aGraphicFilter.ImportGraphic(aGraphic, *i, aFileStream) != 0)
                continue;
            SAL_INFO("vcl.icontest", *i << ": size: " << aGraphic.GetSizeBytes() << "B, " << aGraphic.GetSizePixel());

            glBindTexture(GL_TEXTURE_2D, pWindow->maTextureName[n]);
            CHECK_GL_ERROR();

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            CHECK_GL_ERROR();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            CHECK_GL_ERROR();

            BitmapEx aBitmap( aGraphic.GetBitmapEx( ) );
            Size aBitmapSize( aBitmap.GetSizePixel() );

            pWindow->maTextureAspect[n] = ((float) aBitmapSize.Width()) / aBitmapSize.Height();

            GLubyte *buffer = new GLubyte[aBitmapSize.Width() * aBitmapSize.Height() * 4];
            OpenGLHelper::ConvertBitmapExToRGBATextureBuffer( aBitmap, buffer, true );

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                         aBitmapSize.Width(), aBitmapSize.Height(),
                         0, GL_RGBA, GL_UNSIGNED_BYTE,
                         buffer);
            CHECK_GL_ERROR();

            delete[] buffer;
            n++;
        }
        pWindow->maTextureName.resize( n );

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
