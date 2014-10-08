/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
private:
    bool mbPainted;
    OUString msKind;
    OpenGLWindow *mpOpenGLWindow;

public:
    MyWorkWindow( const char *kind, vcl::Window* pParent, WinBits nWinStyle );

    virtual void MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void Resize() SAL_OVERRIDE;
};

MyWorkWindow::MyWorkWindow( const char *kind, vcl::Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle )
{
    EnableInput();
    mbPainted = false;
    msKind = OUString( kind, strlen(kind), RTL_TEXTENCODING_UTF8 );
    if( msKind == "opengl" )
    {
        mpOpenGLWindow = new OpenGLWindow( this );
        mpOpenGLWindow->SetSizePixel( Size( 1000, 800 ) );
        mpOpenGLWindow->Show();
        mpOpenGLWindow->EnableInput();
   }
}

void MyWorkWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    SAL_INFO("vcl.icontest", "MouseButtonDown: " << rMEvt.GetPosPixel() );
    Size aSize( GetSizePixel() );
    Paint( Rectangle( Point( 0, 0 ), aSize ) );
    WorkWindow::MouseButtonDown( rMEvt );
}

void MyWorkWindow::Paint( const Rectangle& rRect )
{
    if( msKind == "opengl" )
    {
        SAL_INFO("vcl.icontest", "==> Paint! (OpenGL) " << GetSizePixel());
        OpenGLContext& aCtx = mpOpenGLWindow->getContext();
        aCtx.requestLegacyContext();
        aCtx.setWinSize( Size( 1000, 800 ) );

        CHECK_GL_ERROR();

        aCtx.makeCurrent();
        CHECK_GL_ERROR();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CHECK_GL_ERROR();
        Size aSize = GetSizePixel();
        glViewport( 0, 0, aSize.Width(), aSize.Height() );
        CHECK_GL_ERROR();
        glColor3f( 1, 0.7, 0.2 );
        CHECK_GL_ERROR();
        glRectf(-0.75, 0.75, 0.75, -0.75);
        CHECK_GL_ERROR();
        aCtx.swapBuffers();
        CHECK_GL_ERROR();
    }
    else
    {
        SAL_INFO("vcl.icontest", "==> Paint! (vcl)");
        WorkWindow::Paint( rRect );
    }
    mbPainted = true;
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
        MyWorkWindow *pWindow = new MyWorkWindow( "vcl", NULL, WB_APP | WB_STDWORK | WB_SIZEABLE | WB_CLOSEABLE | WB_CLIPCHILDREN );

        pWindow->SetText(OUString("VCL Image Test"));

        Point aPos(10, 10);

        for (std::vector<OUString>::const_iterator i = aImageFiles.cbegin(); i != aImageFiles.end(); ++i)
        {
            SvFileStream aFileStream( *i, STREAM_READ );
            GraphicFilter aGraphicFilter(false);
            Graphic aGraphic;
            if (aGraphicFilter.ImportGraphic(aGraphic, *i, aFileStream) != 0)
                continue;
            SAL_INFO("vcl.icontest", *i << ": size: " << aGraphic.GetSizeBytes() << "B, " << aGraphic.GetSizePixel());
            Size aSize( 100, 100 );
            GraphicConversionParameters aConv( aSize );
            Bitmap *pBitmap = new Bitmap( aGraphic.GetBitmap( aConv ) );

            FixedBitmap *pFixedBitmap = new FixedBitmap( pWindow );
            pFixedBitmap->SetBitmap( *pBitmap );
            pFixedBitmap->SetSizePixel( aSize );
            pFixedBitmap->SetPosPixel( aPos );
            pFixedBitmap->Show();

            aPos.Move( aSize.Width() + 10, 0);
            if ( aPos.X() > 1000 )
            {
                aPos.setX( 10 );
                aPos.setY( aPos.Y() + aSize.Height() + 10 );
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
        MyWorkWindow *pWindow = new MyWorkWindow( "opengl", NULL, WB_APP | WB_STDWORK | WB_SIZEABLE | WB_CLOSEABLE | WB_CLIPCHILDREN );

        pWindow->SetText(OUString("OpenGL Image Test"));

        Point aPos(10, 10);

        for (std::vector<OUString>::const_iterator i = aImageFiles.cbegin(); i != aImageFiles.end(); ++i)
        {
            SvFileStream aFileStream( *i, STREAM_READ );
            GraphicFilter aGraphicFilter(false);
            Graphic aGraphic;
            if (aGraphicFilter.ImportGraphic(aGraphic, *i, aFileStream) != 0)
                continue;
            SAL_INFO("vcl.icontest", *i << ": size: " << aGraphic.GetSizeBytes() << "B, " << aGraphic.GetSizePixel());
            Size aSize( 100, 100 );
            // Bitmap *pBitmap = new Bitmap( aGraphic.GetBitmap( ) );


            aPos.Move( aSize.Width() + 10, 0);
            if ( aPos.X() > 800 )
            {
                aPos.setX( 10 );
                aPos.setY( aPos.Y() + aSize.Height() + 10 );
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

void vclmain::createApplication()
{
    static IconTestApp aApp;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
