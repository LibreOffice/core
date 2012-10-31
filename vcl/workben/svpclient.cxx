/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <sal/main.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/imgctrl.hxx>
#include <vcl/bitmapex.hxx>
#include <tools/extendapplicationenvironment.hxx>
#include <tools/stream.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <math.h>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


using namespace cppu;
using namespace comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringToOString;
using ::rtl::OStringToOUString;
using ::rtl::OUStringBuffer;
using ::rtl::OStringBuffer;
// -----------------------------------------------------------------------

// Forward declaration
void Main();

// -----------------------------------------------------------------------

SAL_IMPLEMENT_MAIN()
{
    tools::extendApplicationEnvironment();

    //-------------------------------------------------
    // create the global service-manager
    //-------------------------------------------------
    Reference< XMultiServiceFactory > xFactory;
    try
    {
        Reference< XComponentContext > xCtx = defaultBootstrap_InitialComponentContext();
        xFactory = Reference< XMultiServiceFactory >(  xCtx->getServiceManager(), UNO_QUERY );
        if( xFactory.is() )
            setProcessServiceFactory( xFactory );
    }
    catch(const com::sun::star::uno::Exception&)
    {
    }

    if( ! xFactory.is() )
    {
        fprintf( stderr, "Could not bootstrap UNO, installation must be in disorder. Exiting.\n" );
        exit( 1 );
    }

    InitVCL();
    ::Main();
    DeInitVCL();

    return 0;
}

// -----------------------------------------------------------------------

class MyWin : public WorkWindow
{
    PushButton      m_aListButton;
    ListBox         m_aSvpBitmaps;
    ImageControl    m_aImage;
    PushButton      m_aQuitButton;
public:
                MyWin( Window* pParent, WinBits nWinStyle );

    void        MouseMove( const MouseEvent& rMEvt );
    void        MouseButtonDown( const MouseEvent& rMEvt );
    void        MouseButtonUp( const MouseEvent& rMEvt );
    void        KeyInput( const KeyEvent& rKEvt );
    void        KeyUp( const KeyEvent& rKEvt );
    void        Paint( const Rectangle& rRect );
    void        Resize();

    sal_Bool        Close();

    void parseList( const rtl::OString& rList );
    rtl::OString processCommand( const rtl::OString& rCommand );

    DECL_LINK( ListHdl, Button* );
    DECL_LINK( SelectHdl, ListBox* );
    DECL_LINK( QuitHdl, Button* );
};

// -----------------------------------------------------------------------

void Main()
{
    MyWin aMainWin( NULL, WB_STDWORK );
    aMainWin.SetText( rtl::OUString( "SvpClient" ) );
    aMainWin.Show();

    Application::Execute();
}

// -----------------------------------------------------------------------

MyWin::MyWin( Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle ),
    m_aListButton( this, 0 ),
    m_aSvpBitmaps( this, WB_BORDER ),
    m_aImage( this, WB_BORDER ),
    m_aQuitButton( this, 0 )
{
    m_aListButton.SetPosSizePixel( Point( 10, 10 ), Size( 120, 25 ) );
    m_aListButton.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "List Elements" ) ) );
    m_aListButton.SetClickHdl( LINK( this, MyWin, ListHdl ) );
    m_aListButton.Show();

    m_aSvpBitmaps.SetPosSizePixel( Point( 10, 40 ), Size( 150, 150 ) );
    m_aSvpBitmaps.SetSelectHdl( LINK( this, MyWin, SelectHdl ) );
    m_aSvpBitmaps.Show();

    m_aImage.SetPosSizePixel( Point( 170, 10 ), Size( 400, 400 ) );
    m_aImage.SetScaleMode( com::sun::star::awt::ImageScaleMode::None );
    m_aImage.Show();

    m_aQuitButton.SetPosSizePixel( Point( 10, 300 ), Size( 120,25 ) );
    m_aQuitButton.SetText( String( RTL_CONSTASCII_USTRINGPARAM( "Quit SVP server" ) ) );
    m_aQuitButton.SetClickHdl( LINK( this, MyWin, QuitHdl ) );
    m_aQuitButton.Show();
}

sal_Bool MyWin::Close()
{
    sal_Bool bRet = WorkWindow::Close();
    if( bRet )
        Application::Quit();
    return bRet;
}

void MyWin::parseList( const rtl::OString& rList )
{
    sal_Int32 nTokenPos = 0;
    rtl::OUString aElementType;
    m_aSvpBitmaps.Clear();
    while( nTokenPos >= 0 )
    {
        rtl::OString aLine = rList.getToken( 0, '\n', nTokenPos );
        if( ! aLine.getLength() || *aLine.getStr() == '#' )
            continue;

        if( aLine.compareTo( "ElementType: ", 13 ) == 0 )
            aElementType = rtl::OStringToOUString( aLine.copy( 13 ), RTL_TEXTENCODING_ASCII_US );
        else
        {
            rtl::OUStringBuffer aNewElement( 64 );
            aNewElement.append( aElementType );
            aNewElement.appendAscii( ": " );
            aNewElement.append( rtl::OStringToOUString( aLine, RTL_TEXTENCODING_ASCII_US ) );
            m_aSvpBitmaps.InsertEntry( aNewElement.makeStringAndClear() );
        }
    }
}

rtl::OString MyWin::processCommand( const rtl::OString& rCommand )
{
    static const char* pEnv = getenv("SVP_LISTENER_PORT");
    rtl::OStringBuffer aAnswer;
    int nPort = (pEnv && *pEnv) ? atoi(pEnv) : 8000;
    int nSocket = socket( PF_INET, SOCK_STREAM, 0 );
    if( nSocket >= 0)
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(nPort);
        addr.sin_addr.s_addr = INADDR_ANY;
        if( connect( nSocket, (const sockaddr*)&addr, sizeof(addr) ) )
        {
            perror( "SvpElementContainer: connect() failed" );
            close(nSocket);
        }
        else
        {
            write( nSocket, rCommand.getStr(), rCommand.getLength() );
            write( nSocket, "\n", 1 );
            char buf[256];
            ssize_t nBytes = 0;
            do
            {
                nBytes = read( nSocket, buf, sizeof(buf) );
                aAnswer.append( buf, nBytes );
            } while( nBytes == sizeof( buf ) );
        }
    }
    else
        perror( "SvpElementContainer: socket() failed\n" );
    return aAnswer.makeStringAndClear();
}

IMPL_LINK( MyWin, ListHdl, Button*, )
{
    parseList( processCommand( "list" ) );
    return 0;
}

IMPL_LINK( MyWin, QuitHdl, Button*, )
{
    processCommand( "quit" );
    return 0;
}

IMPL_LINK( MyWin, SelectHdl, ListBox*, )
{
    String aEntry = m_aSvpBitmaps.GetSelectEntry();
    sal_uInt16 nPos = aEntry.SearchAscii( ": " );
    if( nPos != STRING_NOTFOUND )
    {
        OStringBuffer aCommand( 64 );
        aCommand.append( "get " );
        aCommand.append( rtl::OUStringToOString( aEntry.Copy( nPos+2 ), RTL_TEXTENCODING_ASCII_US ) );
        OString aAnswer( processCommand( aCommand.makeStringAndClear() ) );
        SvMemoryStream aStream( aAnswer.getLength() );
        aStream.Write( aAnswer.getStr(), aAnswer.getLength() );
        aStream.Seek( STREAM_SEEK_TO_BEGIN );
        Bitmap aBitmap;
        aStream >> aBitmap;
        fprintf( stderr, "got bitmap of size %ldx%ld\n",
                 sal::static_int_cast< long >(aBitmap.GetSizePixel().Width()),
                 sal::static_int_cast< long >(aBitmap.GetSizePixel().Height()));
        Size aFixedSize( aBitmap.GetSizePixel() );
        aFixedSize.Width() += 10;
        aFixedSize.Height() += 10;
        m_aImage.SetSizePixel( aFixedSize );
        m_aImage.SetImage( Image( BitmapEx( aBitmap ) ) );
    }
    return 0;
}

// -----------------------------------------------------------------------

void MyWin::MouseMove( const MouseEvent& rMEvt )
{
    WorkWindow::MouseMove( rMEvt );
}

// -----------------------------------------------------------------------

void MyWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void MyWin::MouseButtonUp( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonUp( rMEvt );
}

// -----------------------------------------------------------------------

void MyWin::KeyInput( const KeyEvent& rKEvt )
{
    WorkWindow::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void MyWin::KeyUp( const KeyEvent& rKEvt )
{
    WorkWindow::KeyUp( rKEvt );
}

// -----------------------------------------------------------------------

void MyWin::Paint( const Rectangle& rRect )
{
    WorkWindow::Paint( rRect );
}

// -----------------------------------------------------------------------

void MyWin::Resize()
{
    WorkWindow::Resize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
