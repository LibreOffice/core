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
#include <sal/log.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/toolkit/button.hxx>
#include <vcl/toolkit/fixed.hxx>
#include <vcl/toolkit/lstbox.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/graph.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/extendapplicationenvironment.hxx>
#include <tools/stream.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <o3tl/string_view.hxx>

#include <math.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;

// Forward declaration
static void Main();

SAL_IMPLEMENT_MAIN()
{
    try
    {
        tools::extendApplicationEnvironment();

        // create the global service-manager
        Reference< XComponentContext > xContext = defaultBootstrap_InitialComponentContext();
        Reference< XMultiServiceFactory > xServiceManager( xContext->getServiceManager(), UNO_QUERY );

        if( !xServiceManager.is() )
            Application::Abort( u"Failed to bootstrap"_ustr );

        comphelper::setProcessServiceFactory( xServiceManager );

        InitVCL();
        ::Main();
        DeInitVCL();
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("vcl", "Fatal");
        return 1;
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl", "Fatal: " << e.what());
        return 1;
    }

    return 0;
}

namespace {

class MyWin : public WorkWindow
{
    VclPtr<PushButton>   m_aListButton;
    VclPtr<ListBox>      m_aSvpBitmaps;
    VclPtr<FixedImage> m_aImage;
    VclPtr<PushButton>   m_aQuitButton;
public:
                 MyWin( vcl::Window* pParent, WinBits nWinStyle );

    virtual bool Close() override;
    virtual ~MyWin() override { disposeOnce(); }
    virtual void dispose() override;

    void parseList( std::string_view rList );
    static OString processCommand( const OString& rCommand );

    DECL_LINK( ListHdl, Button*, void );
    DECL_LINK( SelectHdl, ListBox&, void );
    DECL_STATIC_LINK( MyWin, QuitHdl, Button*, void );
};

}

void Main()
{
    ScopedVclPtrInstance< MyWin > aMainWin( nullptr, WB_STDWORK );
    aMainWin->SetText( u"SvpClient"_ustr );
    aMainWin->Show();

    Application::Execute();
}

MyWin::MyWin( vcl::Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle ),
    m_aListButton(VclPtr<PushButton>::Create(this, 0)),
    m_aSvpBitmaps(VclPtr<ListBox>::Create(this, WB_BORDER)),
    m_aImage(VclPtr<FixedImage>::Create(this, WB_BORDER)),
    m_aQuitButton(VclPtr<PushButton>::Create(this, 0))
{
    m_aListButton->SetPosSizePixel( Point( 10, 10 ), Size( 120, 25 ) );
    m_aListButton->SetText( u"List Elements"_ustr );
    m_aListButton->SetClickHdl( LINK( this, MyWin, ListHdl ) );
    m_aListButton->Show();

    m_aSvpBitmaps->SetPosSizePixel( Point( 10, 40 ), Size( 150, 150 ) );
    m_aSvpBitmaps->SetSelectHdl( LINK( this, MyWin, SelectHdl ) );
    m_aSvpBitmaps->Show();

    m_aImage->SetPosSizePixel( Point( 170, 10 ), Size( 400, 400 ) );
    m_aImage->Show();

    m_aQuitButton->SetPosSizePixel( Point( 10, 300 ), Size( 120,25 ) );
    m_aQuitButton->SetText( u"Quit SVP server"_ustr );
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

void MyWin::parseList( std::string_view rList )
{
    sal_Int32 nTokenPos = 0;
    OUString aElementType;
    m_aSvpBitmaps->Clear();
    while( nTokenPos >= 0 )
    {
        std::string_view aLine = o3tl::getToken(rList, 0, '\n', nTokenPos );
        if( aLine.empty() || aLine[0] == '#' )
            continue;

        if( o3tl::starts_with(aLine, "ElementType: " ) )
            aElementType = OStringToOUString( aLine.substr( 13 ), RTL_TEXTENCODING_ASCII_US );
        else
        {
            OUString aNewElement =
                aElementType + ": " +
                OStringToOUString( aLine, RTL_TEXTENCODING_ASCII_US );
            m_aSvpBitmaps->InsertEntry( aNewElement );
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
            ssize_t fd = write( nSocket, rCommand.getStr(), rCommand.getLength() );

            if (fd == 0)
                SAL_WARN("vcl", "Connection closed on other end");
            else if (fd < 0)
                SAL_WARN("vcl", "Error writing to socket: " << strerror( errno ));

            fd = write( nSocket, "\n", 1 );

            if (fd == 0)
                SAL_WARN("vcl", "Connection closed on other end");
            else if (fd < 0)
                SAL_WARN("vcl", "Error writing to socket: " << strerror( errno ));


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

IMPL_LINK_NOARG( MyWin, ListHdl, Button*, void)
{
    parseList( processCommand( "list"_ostr ) );
}

IMPL_STATIC_LINK_NOARG( MyWin, QuitHdl, Button*, void)
{
    processCommand( "quit"_ostr );
}

IMPL_LINK_NOARG( MyWin, SelectHdl, ListBox&, void)
{
    OUString aEntry = m_aSvpBitmaps->GetSelectedEntry();
    sal_Int32 nPos = aEntry.indexOf( ": " );
    if( nPos == -1 )
        return;

    OString aCommand =
        "get " +
        OUStringToOString( aEntry.subView( nPos+2 ), RTL_TEXTENCODING_ASCII_US );
    OString aAnswer( processCommand( aCommand ) );
    SvMemoryStream aStream( aAnswer.getLength() );
    aStream.WriteBytes( aAnswer.getStr(), aAnswer.getLength() );
    aStream.Seek( STREAM_SEEK_TO_BEGIN );

    Graphic aGraphicResult;
    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
    rFilter.ImportGraphic( aGraphicResult, u"import", aStream );

    BitmapEx aBitmap = aGraphicResult.GetBitmapEx();

    SAL_INFO("vcl", "got bitmap of size " << aBitmap.GetSizePixel().Width() << "x" << aBitmap.GetSizePixel().Height());
    Size aFixedSize( aBitmap.GetSizePixel() );
    aFixedSize.AdjustWidth(10 );
    aFixedSize.AdjustHeight(10 );
    m_aImage->SetSizePixel( aFixedSize );
    m_aImage->SetImage( Image( aBitmap ) );

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
