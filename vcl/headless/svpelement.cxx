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

#include "headless/svpelement.hxx"

#include <basebmp/scanlineformats.hxx>
#include <osl/diagnose.h>

#if defined WITH_SVP_LISTENING
#include <osl/thread.h>
#include <vcl/svapp.hxx>
#include <rtl/strbuf.hxx>
#include <vcl/bitmap.hxx>
#include <tools/stream.hxx>

#include "headless/svpvd.hxx"
#include "headless/svpbmp.hxx"
#include "headless/svpframe.hxx"

#include <list>
#include <boost/unordered_map.hpp>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <errno.h>

using namespace basegfx;

class SvpElementContainer
{
    std::list< SvpElement* >        m_aElements;
    int                             m_nSocket;
    oslThread                       m_aThread;

    SvpElementContainer();
    ~SvpElementContainer();
    public:
    void registerElement( SvpElement* pElement ) { m_aElements.push_back(pElement); }
    void deregisterElement( SvpElement* pElement ) { m_aElements.remove(pElement); }

    void run();
    DECL_LINK(processRequest,void*);

    static SvpElementContainer& get();
};

extern "C" void SAL_CALL SvpContainerThread( void* );

SvpElementContainer& SvpElementContainer::get()
{
    static SvpElementContainer* pInstance = new SvpElementContainer();
    return *pInstance;
}

SvpElementContainer::SvpElementContainer()
{
    static const char* pEnv = getenv("SVP_LISTENER_PORT");
    int nPort = (pEnv && *pEnv) ? atoi(pEnv) : 8000;
    m_nSocket = socket( PF_INET, SOCK_STREAM, 0 );
    if( m_nSocket >= 0)
    {
        int nOn = 1;
        if( setsockopt(m_nSocket, SOL_SOCKET, SO_REUSEADDR,
                       (char*)&nOn, sizeof(nOn)) )
        {
            perror( "SvpElementContainer: changing socket options failed" );
            close( m_nSocket );
        }
        else
        {
            struct sockaddr_in addr;
            memset(&addr, 0, sizeof(struct sockaddr_in));
            addr.sin_family = AF_INET;
            addr.sin_port = htons(nPort);
            addr.sin_addr.s_addr = INADDR_ANY;
            if( bind(m_nSocket,(struct sockaddr*)&addr,sizeof(addr)) )
            {
                perror( "SvpElementContainer: bind() failed" );
                close( m_nSocket );
            }
            else
            {
                if( listen( m_nSocket, 0 ) )
                {
                    perror( "SvpElementContainer: listen() failed" );
                    close(m_nSocket);
                }
                else
                {
                    m_aThread = osl_createThread( SvpContainerThread, this );
                }
            }
        }
    }
    else
        perror( "SvpElementContainer: socket() failed\n" );
}

SvpElementContainer::~SvpElementContainer()
{
}

void SAL_CALL SvpContainerThread(void* pSvpContainer)
{
    ((SvpElementContainer*)pSvpContainer)->run();
}

void SvpElementContainer::run()
{
    bool bRun = m_nSocket != 0;
    while( bRun )
    {
        int nLocalSocket = accept( m_nSocket, NULL, NULL );
        if( nLocalSocket < 0 )
        {
            bRun = false;
            perror( "accept() failed" );
        }
        else
        {
            Application::PostUserEvent( LINK( this, SvpElementContainer, processRequest ), (void*)nLocalSocket );
        }
    }
    if( m_nSocket )
        close( m_nSocket );
}

static const char* matchType( SvpElement* pEle )
{
    if( dynamic_cast<SvpSalBitmap*>(pEle) )
        return "Bitmap";
    else if( dynamic_cast<SvpSalFrame*>(pEle) )
        return "Frame";
    else if( dynamic_cast<SvpSalVirtualDevice*>(pEle) )
        return "VirtualDevice";
    return typeid(*pEle).name();
}

IMPL_LINK( SvpElementContainer, processRequest, void*, pSocket )
{
    int nFile = (int)pSocket;

    rtl::OStringBuffer aBuf( 256 ), aAnswer( 256 );
    char c;
    while( read( nFile, &c, 1 ) && c != '\n' )
        aBuf.append( sal_Char(c) );
    rtl::OString aCommand( aBuf.makeStringAndClear() );
    if( aCommand.startsWith( "list" ) )
    {
        boost::unordered_map< rtl::OString, std::list<SvpElement*>, rtl::OStringHash > aMap;
        for( std::list< SvpElement* >::const_iterator it = m_aElements.begin();
             it != m_aElements.end(); ++it )
        {
            std::list<SvpElement*>& rList = aMap[matchType(*it)];
            rList.push_back( *it );
        }
        for( boost::unordered_map< rtl::OString, std::list<SvpElement*>, rtl::OStringHash>::const_iterator hash_it = aMap.begin();
             hash_it != aMap.end(); ++hash_it )
        {
            aAnswer.append( "ElementType: " );
            aAnswer.append( hash_it->first );
            aAnswer.append( '\n' );
            for( std::list<SvpElement*>::const_iterator it = hash_it->second.begin();
                 it != hash_it->second.end(); ++it )
            {
                aAnswer.append( sal_Int64(reinterpret_cast<sal_uInt32>(*it)), 16 );
                aAnswer.append( '\n' );
            }
        }
    }
    else if( aCommand.startsWith( "get" ) )
    {
        sal_IntPtr aId = aCommand.copy( 3 ).toInt64( 16 );
        SvpElement* pElement = reinterpret_cast<SvpElement*>(aId);
        for( std::list< SvpElement* >::const_iterator it = m_aElements.begin();
             it != m_aElements.end(); ++it )
        {
            if( *it == pElement )
            {
                const basebmp::BitmapDeviceSharedPtr& rDevice = pElement->getDevice();
                if( rDevice.get() )
                {
                    SvpSalBitmap* pSalBitmap = new SvpSalBitmap();
                    pSalBitmap->setBitmap( rDevice );
                    Bitmap aBitmap( pSalBitmap );
                    SvMemoryStream aStream( 256, 256 );
                    aStream << aBitmap;
                    aStream.Seek( STREAM_SEEK_TO_END );
                    int nBytes = aStream.Tell();
                    aStream.Seek( STREAM_SEEK_TO_BEGIN );
                    aAnswer.append( (const sal_Char*)aStream.GetData(), nBytes );
                }
                break;
            }
        }
    }
    else if( aCommand.startsWith( "quit" ) )
    {
        Application::Quit();
        close( m_nSocket );
        m_nSocket = 0;
    }
    write( nFile, aAnswer.getStr(), aAnswer.getLength() );
    close( nFile );

    return 0;
}

#endif

using namespace basebmp;

SvpElement::SvpElement()
{
    #if defined WITH_SVP_LISTENING
    SvpElementContainer::get().registerElement( this );
    #endif
}

SvpElement::~SvpElement()
{
    #if defined WITH_SVP_LISTENING
    SvpElementContainer::get().deregisterElement( this );
    #endif
}

sal_uInt32 SvpElement::getBitCountFromScanlineFormat( sal_Int32 nFormat )
{
    sal_uInt32 nBitCount = 1;
    switch( nFormat )
    {
        case Format::ONE_BIT_MSB_GREY:
        case Format::ONE_BIT_LSB_GREY:
        case Format::ONE_BIT_MSB_PAL:
        case Format::ONE_BIT_LSB_PAL:
            nBitCount = 1;
            break;
        case Format::FOUR_BIT_MSB_GREY:
        case Format::FOUR_BIT_LSB_GREY:
        case Format::FOUR_BIT_MSB_PAL:
        case Format::FOUR_BIT_LSB_PAL:
            nBitCount = 4;
            break;
        case Format::EIGHT_BIT_PAL:
        case Format::EIGHT_BIT_GREY:
            nBitCount = 8;
            break;
        case Format::SIXTEEN_BIT_LSB_TC_MASK:
        case Format::SIXTEEN_BIT_MSB_TC_MASK:
            nBitCount = 16;
            break;
        case Format::TWENTYFOUR_BIT_TC_MASK:
            nBitCount = 24;
            break;
        case Format::THIRTYTWO_BIT_TC_MASK_BGRA:
        case Format::THIRTYTWO_BIT_TC_MASK_ARGB:
        case Format::THIRTYTWO_BIT_TC_MASK_ABGR:
        case Format::THIRTYTWO_BIT_TC_MASK_RGBA:
            nBitCount = 32;
            break;
        default:
        OSL_FAIL( "unsupported basebmp format" );
        break;
    }
    return nBitCount;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
