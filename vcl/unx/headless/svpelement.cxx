/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svpelement.cxx,v $
 * $Revision: 1.3 $
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

#include "svpelement.hxx"

#include <basebmp/scanlineformats.hxx>
#include <tools/debug.hxx>

#if defined WITH_SVP_LISTENING
#include <osl/thread.h>
#include <vcl/svapp.hxx>
#include <rtl/strbuf.hxx>
#include <vcl/bitmap.hxx>
#include <tools/stream.hxx>

#include "svpvd.hxx"
#include "svpbmp.hxx"
#include "svpframe.hxx"

#include <list>
#include <hash_map>

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
    if( aCommand.compareTo( "list", 4 ) == 0 )
    {
        std::hash_map< rtl::OString, std::list<SvpElement*>, rtl::OStringHash > aMap;
        for( std::list< SvpElement* >::const_iterator it = m_aElements.begin();
             it != m_aElements.end(); ++it )
        {
            std::list<SvpElement*>& rList = aMap[matchType(*it)];
            rList.push_back( *it );
        }
        for( std::hash_map< rtl::OString, std::list<SvpElement*>, rtl::OStringHash>::const_iterator hash_it = aMap.begin();
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
    else if( aCommand.compareTo( "get", 3 ) == 0 )
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
    else if( aCommand.compareTo( "quit", 4 ) == 0 )
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
        case Format::THIRTYTWO_BIT_TC_MASK:
            nBitCount = 32;
            break;
        default:
        DBG_ERROR( "unsupported basebmp format" );
        break;
    }
    return nBitCount;
}


