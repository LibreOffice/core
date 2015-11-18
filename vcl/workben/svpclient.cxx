/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/main.h>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/imgctrl.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/graph.hxx>
#include <tools/extendapplicationenvironment.hxx>
#include <tools/stream.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <math.h>

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;

// Forward declaration
void Main();

SAL_IMPLEMENT_MAIN()
{
    try
    {
        tools::extendApplicationEnvironment();

        // create the global service-manager
        Reference< XComponentContext > xContext = defaultBootstrap_InitialComponentContext();
        Reference< XMultiServiceFactory > xServiceManager( xContext->getServiceManager(), UNO_QUERY );

        if( !xServiceManager.is() )
            Application::Abort( "Failed to bootstrap" );

        comphelper::setProcessServiceFactory( xServiceManager );

        InitVCL();
        ::Main();
        DeInitVCL();
    }
    catch (const Exception& e)
    {
        SAL_WARN("vcl.app", "Fatal exception: " << e.Message);
        return 1;
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl.app", "Fatal exception: " << e.what());
        return 1;
    }

    return 0;
}

class MyWin : public WorkWindow
{
    VclPtr<PushButton>   m_aListButton;
    VclPtr<ListBox>      m_aSvpBitmaps;
    VclPtr<ImageControl> m_aImage;
    VclPtr<PushButton>   m_aQuitButton;
public:
                 MyWin( vcl::Window* pParent, WinBits nWinStyle );

    virtual void MouseMove( const MouseEvent& rMEvt ) override;
    virtual void MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void KeyInput( const KeyEvent& rKEvt ) override;
    virtual void KeyUp( const KeyEvent& rKEvt ) override;
    virtual void Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& rRect ) override;
    virtual void Resize() override;

    virtual bool Close() override;
    virtual ~MyWin() { disposeOnce(); }
    virtual void dispose() override;

    void parseList( const OString& rList );
    static OString processCommand( const OString& rCommand );

    DECL_LINK_TYPED( ListHdl, Button*, void );
    DECL_LINK_TYPED( SelectHdl, ListBox&, void );
    DECL_STATIC_LINK_TYPED( MyWin, QuitHdl, Button*, void );
};

void Main()
{
    ScopedVclPtrInstance< MyWin > aMainWin( nullptr, WB_STDWORK );
    aMainWin->SetText( "SvpClient" );
    aMainWin->Show();

    Application::Execute();
}

MyWin::MyWin( vcl::Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle ),
    m_aListButton(VclPtr<PushButton>::Create(this, 0)),
    m_aSvpBitmaps(VclPtr<ListBox>::Create(this, WB_BORDER)),
    m_aImage(VclPtr<ImageControl>::Create(this, WB_BORDER)),
    m_aQuitButton(VclPtr<PushButton>::Create(this, 0))
{
    m_aListButton->SetPosSizePixel( Point( 10, 10 ), Size( 120, 25 ) );
    m_aListButton->SetText( "List Elements" );
    m_aListButton->SetClickHdl( LINK( this, MyWin, ListHdl ) );
    m_aListButton->Show();

    m_aSvpBitmaps->SetPosSizePixel( Point( 10, 40 ), Size( 150, 150 ) );
    m_aSvpBitmaps->SetSelectHdl( LINK( this, MyWin, SelectHdl ) );
    m_aSvpBitmaps->Show();

    m_aImage->SetPosSizePixel( Point( 170, 10 ), Size( 400, 400 ) );
    m_aImage->SetScaleMode( css::awt::ImageScaleMode::NONE );
    m_aImage->Show();

    m_aQuitButton->SetPosSizePixel( Point( 10, 300 ), Size( 120,25 ) );
    m_aQuitButton->SetText( "Quit SVP server" );
    m_aQuitButton->SetClickHdl( LINK( this, MyWin, QuitHdl ) );
    m_aQuitButton->Show();
}

bool MyWin::Close()
{
    bool bRet = WorkWindow::Close();
    if( bRet )
        Application::Quit();
    return bRet;
}

void MyWin::dispose()
{
    m_aListButton.disposeAndClear();
    m_aSvpBitmaps.disposeAndClear();
    m_aImage.disposeAndClear();
    m_aQuitButton.disposeAndClear();
    WorkWindow::dispose();
}

void MyWin::parseList( const OString& rList )
{
    sal_Int32 nTokenPos = 0;
    OUString aElementType;
    m_aSvpBitmaps->Clear();
    while( nTokenPos >= 0 )
    {
        OString aLine = rList.getToken( 0, '\n', nTokenPos );
        if( ! aLine.getLength() || *aLine.getStr() == '#' )
            continue;

        if( aLine.startsWith( "ElementType: " ) )
            aElementType = OStringToOUString( aLine.copy( 13 ), RTL_TEXTENCODING_ASCII_US );
        else
        {
            OUStringBuffer aNewElement( 64 );
            aNewElement.append( aElementType );
            aNewElement.append( ": " );
            aNewElement.append( OStringToOUString( aLine, RTL_TEXTENCODING_ASCII_US ) );
            m_aSvpBitmaps->InsertEntry( aNewElement.makeStringAndClear() );
        }
    }
}

OString MyWin::processCommand( const OString& rCommand )
{
    static const char* pEnv = getenv("SVP_LISTENER_PORT");
    OStringBuffer aAnswer;
    int nPort = (pEnv && *pEnv) ? atoi(pEnv) : 8000;
    int nSocket = socket( PF_INET, SOCK_STREAM, 0 );
    if( nSocket >= 0)
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(nPort);
        addr.sin_addr.s_addr = INADDR_ANY;
        if( connect( nSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr) ) )
        {
            perror( "SvpElementContainer: connect() failed" );
        }
        else
        {
            ssize_t nBytes = 0;
            write( nSocket, rCommand.getStr(), rCommand.getLength() );
            write( nSocket, "\n", 1 );
            char buf[256];
            do
            {
                nBytes = read( nSocket, buf, sizeof(buf) );
                aAnswer.append( buf, nBytes );
            } while( nBytes == sizeof( buf ) );
        }
        close(nSocket);
    }
    else
        perror( "SvpElementContainer: socket() failed\n" );
    return aAnswer.makeStringAndClear();
}

IMPL_LINK_NOARG_TYPED( MyWin, ListHdl, Button*, void)
{
    parseList( processCommand( "list" ) );
}

IMPL_STATIC_LINK_NOARG_TYPED( MyWin, QuitHdl, Button*, void)
{
    processCommand( "quit" );
}

IMPL_LINK_NOARG_TYPED( MyWin, SelectHdl, ListBox&, void)
{
    OUString aEntry = m_aSvpBitmaps->GetSelectEntry();
    sal_Int32 nPos = aEntry.indexOf( ": " );
    if( nPos != -1 )
    {
        OStringBuffer aCommand( 64 );
        aCommand.append( "get " );
        aCommand.append( OUStringToOString( aEntry.copy( nPos+2 ), RTL_TEXTENCODING_ASCII_US ) );
        OString aAnswer( processCommand( aCommand.makeStringAndClear() ) );
        SvMemoryStream aStream( aAnswer.getLength() );
        aStream.Write( aAnswer.getStr(), aAnswer.getLength() );
        aStream.Seek( STREAM_SEEK_TO_BEGIN );

        Graphic aGraphicResult;
        GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.ImportGraphic( aGraphicResult, OUString("import"), aStream );

        Bitmap aBitmap = aGraphicResult.GetBitmap();

        SAL_INFO("vcl", "got bitmap of size " << aBitmap.GetSizePixel().Width() << "x" << aBitmap.GetSizePixel().Height() << "\n");
        Size aFixedSize( aBitmap.GetSizePixel() );
        aFixedSize.Width() += 10;
        aFixedSize.Height() += 10;
        m_aImage->SetSizePixel( aFixedSize );
        m_aImage->SetImage( Image( BitmapEx( aBitmap ) ) );
    }
}

void MyWin::MouseMove( const MouseEvent& rMEvt )
{
    WorkWindow::MouseMove( rMEvt );
}

void MyWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonDown( rMEvt );
}

void MyWin::MouseButtonUp( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonUp( rMEvt );
}

void MyWin::KeyInput( const KeyEvent& rKEvt )
{
    WorkWindow::KeyInput( rKEvt );
}

void MyWin::KeyUp( const KeyEvent& rKEvt )
{
    WorkWindow::KeyUp( rKEvt );
}

void MyWin::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    WorkWindow::Paint(rRenderContext, rRect);
}

void MyWin::Resize()
{
    WorkWindow::Resize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
