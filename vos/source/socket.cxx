/*************************************************************************
 *
 *  $RCSfile: socket.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mfe $ $Date: 2000-10-13 12:04:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdio.h>

#include <rtl/memory.h>

#include <vos/socket.hxx>
#include <vos/diagnose.hxx>
//#include <osl/tools.h>

#ifdef _USE_NAMESPACE
using namespace vos;
#endif


VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OSocketAddr, vos),
                        VOS_NAMESPACE(OSocketAddr, vos),
                        VOS_NAMESPACE(OObject, vos), 0);


/*****************************************************************************/
// OSocketAddr()
/*****************************************************************************/
OSocketAddr::OSocketAddr()
{
    m_SockAddr= 0;
}

/*****************************************************************************/
// OSocketAddr()
/*****************************************************************************/
OSocketAddr::OSocketAddr(oslSocketAddr Addr)
{
    m_SockAddr= Addr;
}

/*****************************************************************************/
// OSocketAddr()
/*****************************************************************************/
OSocketAddr::OSocketAddr(const OSocketAddr& Addr)
{
    m_SockAddr= osl_copySocketAddr((oslSocketAddr)Addr);
}

/*****************************************************************************/
// ~OSocketAddr()
/*****************************************************************************/
OSocketAddr::~OSocketAddr()
{
    osl_destroySocketAddr(m_SockAddr);
}


/*****************************************************************************/
// getFamily()
/*****************************************************************************/
OSocketAddr::TAddrFamily OSocketAddr::getFamily() const
{
    return (TAddrFamily)osl_getFamilyOfSocketAddr(m_SockAddr);
}

/*****************************************************************************/
// operator oslSocketAddr ()
/*****************************************************************************/
OSocketAddr::operator oslSocketAddr() const
{
    return m_SockAddr;
}

/*****************************************************************************/
// getHostname()
/*****************************************************************************/
OSocketAddr::TResult OSocketAddr::getHostname(rtl::OUString& rBuffer ) const
{
    return (TResult)osl_getHostnameOfSocketAddr(m_SockAddr, &rBuffer.pData );
}

/*****************************************************************************/
// getLocalHostname()
/*****************************************************************************/
OSocketAddr::TResult OSocketAddr::getLocalHostname( rtl::OUString& pBuffer )
{
    return (TResult)osl_getLocalHostname( &pBuffer.pData );
}

/*****************************************************************************/
// resolveHostname()
/*****************************************************************************/
oslSocketAddr OSocketAddr::resolveHostname(const rtl::OUString& ustrHostname)
{
    return osl_resolveHostname( ustrHostname.pData );
}

/*****************************************************************************/
// operator= (oslSocketAddr Addr)
/*****************************************************************************/
void OSocketAddr::operator= (oslSocketAddr Addr)
{
    if(m_SockAddr) {
        osl_destroySocketAddr(m_SockAddr);
    }

    m_SockAddr= Addr;
}

/*****************************************************************************/
// operator== (oslSocketAddr Addr)
/*****************************************************************************/
sal_Bool OSocketAddr::operator== (oslSocketAddr Addr)
{
    return (osl_isEqualSocketAddr(m_SockAddr, Addr));
}

/*****************************************************************************/
// operator=(const OSocketAddr& Addr)
/*****************************************************************************/
OSocketAddr& OSocketAddr::operator=(const OSocketAddr& Addr)
{
    if(m_SockAddr) {
        osl_destroySocketAddr(m_SockAddr);
    }

    m_SockAddr= osl_copySocketAddr((oslSocketAddr)Addr);

    return *this;
}


VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OInetSocketAddr, vos),
                        VOS_NAMESPACE(OInetSocketAddr, vos),
                        VOS_NAMESPACE(OSocketAddr, vos), 0);



/*****************************************************************************/
// OInetSocketAddr
// creates arbitrary inet-address (INADDR_ANY)
/*****************************************************************************/
OInetSocketAddr::OInetSocketAddr()
{
    m_SockAddr= osl_createEmptySocketAddr(osl_Socket_FamilyInet);
}

/*****************************************************************************/
// OInetSocketAddr
// uses the given oslSocketAddr as its own
/*****************************************************************************/
OInetSocketAddr::OInetSocketAddr(oslSocketAddr Addr) :
    OSocketAddr(Addr)
{
    VOS_ASSERT(osl_getFamilyOfSocketAddr(Addr) == osl_Socket_FamilyInet);
}

/*****************************************************************************/
// OInetSocketAddr
// Create a socket address either from a dotted decimal address
//(e.g. 141.99.128.50) or a hostname (e.g. www.stardiv.de).
/*****************************************************************************/
OInetSocketAddr::OInetSocketAddr(const rtl::OUString& ustrAddrOrHostname, sal_Int32 Port)
{
    // first try as dotted address.
    m_SockAddr= osl_createInetSocketAddr(ustrAddrOrHostname.pData, Port);

    // create failed, maybe it's an hostname
    if(m_SockAddr == 0)
    {
        m_SockAddr= osl_resolveHostname(ustrAddrOrHostname.pData);

        // host found?
        if(m_SockAddr)
        {
            // set port will fail if addrtype is not osl_Socket_FamilyInet
            VOS_VERIFY(osl_setInetPortOfSocketAddr(m_SockAddr, Port));
        }
    }
}

/*****************************************************************************/
// OInetSocketAddr(const OInetSocketAddr&)
/*****************************************************************************/
OInetSocketAddr::OInetSocketAddr(const OInetSocketAddr& sa) :
    OSocketAddr(sa)
{
    VOS_ASSERT(getFamily() == TFamily_Inet);
}

/*****************************************************************************/
// OInetSocketAddr(const OSocketAddr&)
/*****************************************************************************/
OInetSocketAddr::OInetSocketAddr(const OSocketAddr& sa) :
    OSocketAddr(sa)
{
    VOS_ASSERT(sa.getFamily() == TFamily_Inet);
}

/*****************************************************************************/
// ~OInetSocketAddr
/*****************************************************************************/
OInetSocketAddr::~OInetSocketAddr()
{
}

/*****************************************************************************/
// operator= (oslSocketAddr Addr)
/*****************************************************************************/
void OInetSocketAddr::operator= (oslSocketAddr Addr)
{
    VOS_PRECOND(osl_getFamilyOfSocketAddr(Addr) == osl_Socket_FamilyInet,
                "oslSocketAddr not of type osl_Socket_FamilyInet!");

    OSocketAddr::operator=(Addr);
}

/*****************************************************************************/
// operator== (oslSocketAddr Addr)
/*****************************************************************************/
sal_Bool OInetSocketAddr::operator== (oslSocketAddr Addr)
{
    return (osl_isEqualSocketAddr(m_SockAddr, Addr));
}

/*****************************************************************************/
// operator=(const OInetSocketAddr& Addr)
/*****************************************************************************/
OInetSocketAddr& OInetSocketAddr::operator=(const OInetSocketAddr& Addr)
{
    VOS_ASSERT(Addr.getFamily() == TFamily_Inet);

    OSocketAddr::operator=(Addr);

    return *this;
}

/*****************************************************************************/
// operator=(const OSocketAddr& Addr)
/*****************************************************************************/
OInetSocketAddr& OInetSocketAddr::operator=(const OSocketAddr& Addr)
{
    VOS_ASSERT(Addr.getFamily() == TFamily_Inet);

    OSocketAddr::operator=(Addr);

    return *this;
}

/*****************************************************************************/
// getServicePort()
/*****************************************************************************/
sal_Int32 OInetSocketAddr::getServicePort(const rtl::OUString& ustrServiceName,
                                    const rtl::OUString& ustrProtocolName)
{
    return osl_getServicePort(ustrServiceName.pData, ustrProtocolName.pData);
}


/*****************************************************************************/
// getPort()
/*****************************************************************************/
sal_Int32 OInetSocketAddr::getPort () const
{
    return osl_getInetPortOfSocketAddr(m_SockAddr);
}


/*****************************************************************************/
// setPort()
/*****************************************************************************/
sal_Bool OInetSocketAddr::setPort (sal_Int32 Port)
{
    return osl_setInetPortOfSocketAddr(m_SockAddr, Port);
}


/*****************************************************************************/
// getDottedAddr()
/*****************************************************************************/
OSocketAddr::TResult OInetSocketAddr::getDottedAddr( rtl::OUString& pBuffer ) const
{
    return (TResult)osl_getDottedInetAddrOfSocketAddr(m_SockAddr, &pBuffer.pData );
}

/*****************************************************************************/
// setAddr()
/*****************************************************************************/
sal_Bool OInetSocketAddr::setAddr(const rtl::OUString& ustrAddrOrHostname)
{
    sal_Int32 Port = 0;

    if(m_SockAddr) {

        // retrieve old port
        Port= getPort();

        // free old address
        osl_destroySocketAddr(m_SockAddr);
        m_SockAddr= 0;
    }

    // first try as dotted address.
    m_SockAddr= osl_createInetSocketAddr(ustrAddrOrHostname.pData, Port);

    // create failed, maybe it's an hostname
    if(m_SockAddr == 0)
    {

        m_SockAddr= osl_resolveHostname( ustrAddrOrHostname.pData );

        // host found?
        if(m_SockAddr==0)
        {
            return sal_False;
        }

        // set port will fail if addrtype is not osl_Socket_FamilyInet
        VOS_VERIFY(osl_setInetPortOfSocketAddr(m_SockAddr, Port));

    }

    return sal_True;
}

///////////////////////////////////////////////////////////////////////////////
// OIpxSocketAddr

VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OIpxSocketAddr, vos),
                        VOS_NAMESPACE(OIpxSocketAddr, vos),
                        VOS_NAMESPACE(OSocketAddr, vos), 0);


/*****************************************************************************/
// OIpxSocketAddr()
/*****************************************************************************/
OIpxSocketAddr::OIpxSocketAddr()
{
    m_SockAddr= osl_createEmptySocketAddr(osl_Socket_FamilyIpx);
}

/*****************************************************************************/
// OIpxSocketAddr(oslSocketAddr)
/*****************************************************************************/
OIpxSocketAddr::OIpxSocketAddr(oslSocketAddr Addr) :
    OSocketAddr(Addr)
{
    VOS_ASSERT(osl_getFamilyOfSocketAddr(Addr) == osl_Socket_FamilyIpx);
}

/*****************************************************************************/
// OIpxSocketAddr()
/*****************************************************************************/
OIpxSocketAddr::OIpxSocketAddr(const rtl::OUString& NetNumber,
                               const rtl::OUString& NodeNumber,
                               sal_uInt32 SocketNumber)
{
    m_SockAddr= osl_createIpxSocketAddr(NetNumber.pData,
                                        NodeNumber.pData,
                                        SocketNumber);

}

/*****************************************************************************/
// OIpxSocketAddr(OIpxSocketAddr&)
/*****************************************************************************/
OIpxSocketAddr::OIpxSocketAddr(const OIpxSocketAddr& sa) :
    OSocketAddr(sa)
{
    VOS_ASSERT(sa.getFamily() == TFamily_Ipx);
}


/*****************************************************************************/
// OIpxSocketAddr(OSocketAddr&)
/*****************************************************************************/
OIpxSocketAddr::OIpxSocketAddr(const OSocketAddr& sa) :
    OSocketAddr(sa)
{
    VOS_ASSERT(sa.getFamily() == TFamily_Ipx);
}

/*****************************************************************************/
// ~OIpxSocketAddr()
/*****************************************************************************/
OIpxSocketAddr::~OIpxSocketAddr()
{
}


/*****************************************************************************/
// operator=()
/*****************************************************************************/
void OIpxSocketAddr::operator= (oslSocketAddr Addr)
{
    VOS_PRECOND(osl_getFamilyOfSocketAddr(Addr) == osl_Socket_FamilyIpx,
            "oslSocketAddr not of type osl_Socket_FamilyIpx!");

    OSocketAddr::operator=(Addr);
}

/*****************************************************************************/
// operator== (oslSocketAddr Addr)
/*****************************************************************************/
sal_Bool OIpxSocketAddr::operator== (oslSocketAddr Addr)
{
    return (osl_isEqualSocketAddr(m_SockAddr, Addr));
}

/*****************************************************************************/
// operator=(const OIpxSocketAddr& Addr)
/*****************************************************************************/
OIpxSocketAddr& OIpxSocketAddr::operator=(const OIpxSocketAddr& Addr)
{
    VOS_ASSERT(Addr.getFamily() == TFamily_Ipx);

    OSocketAddr::operator=(Addr);

    return *this;
}

/*****************************************************************************/
// operator=(const OSocketAddr& Addr)
/*****************************************************************************/
OIpxSocketAddr& OIpxSocketAddr::operator=(const OSocketAddr& Addr)
{
    VOS_ASSERT(Addr.getFamily() == TFamily_Ipx);

    OSocketAddr::operator=(Addr);

    return *this;
}

/*****************************************************************************/
// getNetNumber()
/*****************************************************************************/
OSocketAddr::TResult OIpxSocketAddr::getNetNumber(TIpxNetNumber& NetNumber) const
{
    return (TResult)osl_getIpxNetNumber(m_SockAddr, NetNumber);
}

/*****************************************************************************/
// getNodeNumber()
/*****************************************************************************/
OSocketAddr::TResult OIpxSocketAddr::getNodeNumber(TIpxNodeNumber& NodeNumber) const
{
    return (TResult)osl_getIpxNodeNumber(m_SockAddr, NodeNumber);
}

/*****************************************************************************/
// getSocketNumber()
/*****************************************************************************/
sal_uInt32 OIpxSocketAddr::getSocketNumber() const
{
    return osl_getIpxSocketNumber(m_SockAddr);
}


/*****************************************************************************/
// getAddressString()
/*****************************************************************************/
//void OIpxSocketAddr::getAddressString(sal_Char* Buffer, sal_uInt32 Len) const
void OIpxSocketAddr::getAddressString( rtl::OUString& Buffer ) const
{
    TIpxNetNumber  NetNumber;
    TIpxNodeNumber NodeNumber;

    sal_Char charBuffer[32] = "";

    sal_Int32 i= 0, j= 0;

    osl_getIpxNetNumber(m_SockAddr, NetNumber);
    osl_getIpxNodeNumber(m_SockAddr, NodeNumber);

    sprintf(charBuffer,
            "%02X%02X%02X%02X.%02X%02X%02X%02X%02X%02X:%04X",
            NetNumber[0],
            NetNumber[1],
            NetNumber[2],
            NetNumber[3],
            NodeNumber[0],
            NodeNumber[1],
            NodeNumber[2],
            NodeNumber[3],
            NodeNumber[4],
            NodeNumber[5],
            osl_getIpxSocketNumber(m_SockAddr));
    Buffer = rtl::OUString::createFromAscii( charBuffer );
}


///////////////////////////////////////////////////////////////////////////////
// Socket


VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OSocket, vos),
                        VOS_NAMESPACE(OSocket, vos),
                        VOS_NAMESPACE(OObject, vos), 0);

/*****************************************************************************/
// OSocket()
/*****************************************************************************/
OSocket::OSocket()
{
    m_pRecvTimeout = 0;
    m_pSendTimeout = 0;

    m_pSockRef= 0;
}


/*****************************************************************************/
// OSocket()
/*****************************************************************************/
OSocket::OSocket(TSocketType Type,
                 TAddrFamily Family,
                 TProtocol   Protocol)
{
    m_pRecvTimeout = 0;
    m_pSendTimeout = 0;

    m_pSockRef=
        new SockRef(osl_createSocket((oslAddrFamily)Family,
                                     (oslSocketType)Type,
                                     (oslProtocol)Protocol));

    VOS_POSTCOND(m_pSockRef != 0, "OSocket(): new failed.\n");
    VOS_POSTCOND((*m_pSockRef)(), "OSocket(): creation of socket failed!\n");
}

/*****************************************************************************/
// OSocket()
/*****************************************************************************/
OSocket::OSocket(const OSocket& sock)
{
    m_pRecvTimeout = 0;
    m_pSendTimeout = 0;
    m_pSockRef=0;

    VOS_ASSERT(sock.m_pSockRef != 0);

    if ( sock.m_pSockRef !=0 )
    {
        m_pSockRef= sock.m_pSockRef;

        setRecvTimeout(sock.m_pRecvTimeout);
        setSendTimeout(sock.m_pSendTimeout);

        m_pSockRef->acquire();
    }
}

/*****************************************************************************/
// OSocket()
/*****************************************************************************/
OSocket::OSocket(oslSocket Socket)
{
    m_pRecvTimeout = 0;
    m_pSendTimeout = 0;

    m_pSockRef = new SockRef(Socket);
}


/*****************************************************************************/
// ~OSocket()
/*****************************************************************************/
OSocket::~OSocket()
{
    close();

    delete m_pRecvTimeout;
    delete m_pSendTimeout;
}


/*****************************************************************************/
// create
/*****************************************************************************/
sal_Bool OSocket::create(TSocketType Type,
                        TAddrFamily Family,
                        TProtocol   Protocol)
{
    // if this was a valid socket, decrease reference
    if ((m_pSockRef) && (m_pSockRef->release() == 0))
    {
        osl_destroySocket((*m_pSockRef)());
        delete m_pSockRef;
        m_pSockRef= 0;
    }

    m_pSockRef=
        new SockRef(osl_createSocket((oslAddrFamily)Family,
                                     (oslSocketType)Type,
                                     (oslProtocol)Protocol));

    VOS_POSTCOND(m_pSockRef != 0, "OSocket(): new failed.\n");

    return (*m_pSockRef)() != 0;
}

/*****************************************************************************/
// operator=
/*****************************************************************************/
OSocket& OSocket::operator= (const OSocket& sock)
{
    VOS_PRECOND(sock.m_pSockRef != 0, "OSocket::operator=: tried to assign an empty/invalid socket\n");

    if (m_pSockRef == sock.m_pSockRef)
        return *this;

    // if this was a valid socket, decrease reference
    if ((m_pSockRef) && (m_pSockRef->release() == 0))
    {
        osl_destroySocket((*m_pSockRef)());
        delete m_pSockRef;
        m_pSockRef= 0;
    }

    m_pSockRef= sock.m_pSockRef;

    setRecvTimeout(sock.m_pRecvTimeout);
    setSendTimeout(sock.m_pSendTimeout);

    m_pSockRef->acquire();

    return *this;
}

/*****************************************************************************/
// operator oslSocket()
/*****************************************************************************/
OSocket::operator oslSocket() const
{
    VOS_ASSERT(m_pSockRef);
    return (*m_pSockRef)();
}

/*****************************************************************************/
// isValid()
/*****************************************************************************/
sal_Bool OSocket::isValid() const
{
    return m_pSockRef != 0 && (*m_pSockRef)() != 0;
}


/*****************************************************************************/
// close
/*****************************************************************************/
void OSocket::close()
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if (m_pSockRef && (m_pSockRef->release() == 0))
    {
        osl_destroySocket((*m_pSockRef)());
        delete m_pSockRef;
    }

    m_pSockRef= 0;
}

/*****************************************************************************/
// getLocalAddr
/*****************************************************************************/
void OSocket::getLocalAddr(OSocketAddr& sa) const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        sa= osl_getLocalAddrOfSocket((*m_pSockRef)());
    }
}

/*****************************************************************************/
// getLocalPort
/*****************************************************************************/
sal_Int32 OSocket::getLocalPort() const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    sal_Int32 Port= OSL_INVALID_PORT;

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        oslSocketAddr Addr= osl_getLocalAddrOfSocket((*m_pSockRef)());

        if(Addr)
        {
            Port= osl_getInetPortOfSocketAddr(Addr);
            osl_destroySocketAddr(Addr);
        }
    }

    return Port;
}

/*****************************************************************************/
// getLocalHost
/*****************************************************************************/
OSocket::TResult OSocket::getLocalHost( rtl::OUString& pBuffer) const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        oslSocketAddr Addr= osl_getLocalAddrOfSocket((*m_pSockRef)());

        if(Addr)
        {
//          TResult Result= (TResult)osl_getHostnameOfSocketAddr(Addr,
//                                                      pBuffer, BufferSize);
            TResult Result= (TResult)osl_getHostnameOfSocketAddr(Addr,
                                                             &pBuffer.pData );

            osl_destroySocketAddr(Addr);

            return Result;
        }
    }

    return TResult_Error;
}

/*****************************************************************************/
// getPeerAddr
/*****************************************************************************/
void OSocket::getPeerAddr(OSocketAddr& sa) const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        sa= osl_getPeerAddrOfSocket((*m_pSockRef)());
    }
}

/*****************************************************************************/
// getPeerPort
/*****************************************************************************/
sal_Int32 OSocket::getPeerPort() const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    sal_Int32 Port= OSL_INVALID_PORT;

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        oslSocketAddr Addr= osl_getPeerAddrOfSocket((*m_pSockRef)());

        if(Addr)
        {
            Port= osl_getInetPortOfSocketAddr(Addr);
            osl_destroySocketAddr(Addr);
        }
    }

    return Port;
}

/*****************************************************************************/
// getPeerHost
/*****************************************************************************/
OSocket::TResult OSocket::getPeerHost( rtl::OUString& pBuffer ) const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        oslSocketAddr Addr= osl_getPeerAddrOfSocket((*m_pSockRef)());

        if(Addr)
        {
//          TResult Result= (TResult)osl_getHostnameOfSocketAddr(Addr,
//                                                      pBuffer, BufferSize);
            TResult Result= (TResult)osl_getHostnameOfSocketAddr(Addr,
                                                             &pBuffer.pData );

            osl_destroySocketAddr(Addr);

            return Result;
        }
    }

    return TResult_Error;
}

/*****************************************************************************/
// bind
/*****************************************************************************/
sal_Bool OSocket::bind(const OSocketAddr& Addr)
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        return osl_bindAddrToSocket((*m_pSockRef)(), (oslSocketAddr)Addr);
    }

    return sal_False;
}


/*****************************************************************************/
// setSendTimeout
/*****************************************************************************/
void OSocket::setSendTimeout(const TimeValue* pTimeout)
{
    delete m_pSendTimeout;

    if (pTimeout)
        m_pSendTimeout = new TimeValue(*pTimeout);
    else
        m_pSendTimeout = 0;
}

/*****************************************************************************/
// setRecvTimeout
/*****************************************************************************/
void OSocket::setRecvTimeout(const TimeValue* pTimeout)
{
    delete m_pRecvTimeout;

    if (pTimeout)
        m_pRecvTimeout = new TimeValue(*pTimeout);
    else
        m_pRecvTimeout = 0;
}

/*****************************************************************************/
// isRecvReady
/*****************************************************************************/
sal_Bool OSocket::isRecvReady(const TimeValue* pTimeout) const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        return osl_isReceiveReady((*m_pSockRef)(), pTimeout);
    }

    return sal_False;
}

/*****************************************************************************/
// isSendReady
/*****************************************************************************/
sal_Bool OSocket::isSendReady(const TimeValue* pTimeout) const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        return osl_isSendReady((*m_pSockRef)(), pTimeout);
    }

    return sal_False;
}

/*****************************************************************************/
// isExceptionPending
/*****************************************************************************/
sal_Bool OSocket::isExceptionPending(const TimeValue* pTimeout) const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        return osl_isExceptionPending((*m_pSockRef)(), pTimeout);
    }

    return sal_False;
}


/*****************************************************************************/
// getOption
/*****************************************************************************/
sal_Int32 OSocket::getOption(TSocketOption Option,
                              void* pBuffer,
                           sal_uInt32 BufferLen,
                           TSocketOptionLevel Level) const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        return osl_getSocketOption((*m_pSockRef)(),
                                   (oslSocketOptionLevel)Level,
                                   (oslSocketOption)Option,
                                   pBuffer,
                                   BufferLen);
    }

    return sal_False;
}

/*****************************************************************************/
// setOption
/*****************************************************************************/
sal_Bool OSocket::setOption(TSocketOption Option,
                           void* pBuffer,
                           sal_uInt32 BufferLen,
                           TSocketOptionLevel Level) const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        return osl_setSocketOption((*m_pSockRef)(),
                                   (oslSocketOptionLevel)Level,
                                   (oslSocketOption)Option,
                                   pBuffer,
                                   BufferLen);
    }

    return sal_False;
}


/*****************************************************************************/
// enableNonBlockingMode
/*****************************************************************************/
sal_Bool OSocket::enableNonBlockingMode(sal_Bool On)
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        return osl_enableNonBlockingMode((*m_pSockRef)(), On);
    }

    return sal_False;
}

/*****************************************************************************/
// isNonBlockingMode
/*****************************************************************************/
sal_Bool OSocket::isNonBlockingMode() const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        return osl_isNonBlockingMode((*m_pSockRef)());
    }

    return sal_False;
}

/*****************************************************************************/
// getType
/*****************************************************************************/
OSocket::TSocketType OSocket::getType() const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        return (TSocketType)osl_getSocketType((*m_pSockRef)());
    }

    return TType_Invalid;
}

/*****************************************************************************/
// clearError
/*****************************************************************************/
sal_Int32 OSocket::clearError() const
{
    sal_Int32 err = 0;

    getOption(TOption_Error, &err, sizeof(err));

    return err;
}

/*****************************************************************************/
// setDebug
/*****************************************************************************/
sal_Int32 OSocket::setDebug(sal_Int32 opt) const
{
    sal_Int32 old= 0;

    getOption(TOption_Debug, &old, sizeof(old));

    if (opt != -1)
        setOption(TOption_Debug, &opt, sizeof(opt));

    return old;
}

/*****************************************************************************/
// setReuseAddr
/*****************************************************************************/
sal_Int32 OSocket::setReuseAddr(sal_Int32 opt) const
{
    sal_Int32 old = 0;

    getOption(TOption_ReuseAddr, &old, sizeof(old));

    if (opt != -1)
        setOption(TOption_ReuseAddr, &opt, sizeof(opt));

    return (old);
}

/*****************************************************************************/
// setKeepAlive
/*****************************************************************************/
sal_Int32 OSocket::setKeepAlive(sal_Int32 opt) const
{
    sal_Int32 old = 0;

    getOption(TOption_KeepAlive, &old, sizeof(old));

    if (opt != -1)
        setOption(TOption_KeepAlive, &opt, sizeof(opt));

    return (old);
}

/*****************************************************************************/
// setDontRoute
/*****************************************************************************/
sal_Int32 OSocket::setDontRoute(sal_Int32 opt) const
{
    sal_Int32 old = 0;

    getOption(TOption_DontRoute, &old, sizeof(old));

    if (opt != -1)
        setOption(TOption_DontRoute, &opt, sizeof(opt));

    return (old);
}

/*****************************************************************************/
// setBroadcast
/*****************************************************************************/
sal_Int32 OSocket::setBroadcast(sal_Int32 opt) const
{
    sal_Int32 old = 0;

    getOption(TOption_Broadcast, &old, sizeof(old));

    if (opt != -1)
        setOption(TOption_Broadcast, &opt, sizeof(opt));

    return (old);
}

/*****************************************************************************/
// setOobinline
/*****************************************************************************/
sal_Int32 OSocket::setOobinline(sal_Int32 opt) const
{
    sal_Int32 old = 0;

    getOption(TOption_OOBinLine, &old, sizeof(old));

    if (opt != -1)
        setOption(TOption_OOBinLine, &opt, sizeof(opt));

    return (old);
}

/*****************************************************************************/
// setLinger
/*****************************************************************************/
sal_Int32 OSocket::setLinger(sal_Int32 time) const
{
    /* local decl. of linger-struct */
    struct SockLinger
    {
        sal_Int32 m_onoff;    // option on/off
        sal_Int32 m_linger;   // linger time
    };


    SockLinger  old = { 0, 0 };

    getOption(TOption_Linger, &old, sizeof(old));

    if (time > 0) // enable linger with wait-times > 0
    {
        SockLinger nw = { 1, time };
        setOption(TOption_Linger, &nw, sizeof(nw));
    }
    else if (time == 0) // disable linger with wait-time == 0
    {
        SockLinger nw = { 0, old.m_linger };
        setOption(TOption_Linger, &nw, sizeof(nw));
    }

    // returns 0 if linger was off, else the linger-time
    return (old.m_onoff ? old.m_linger : 0);
}

/*****************************************************************************/
// setSendBufSize
/*****************************************************************************/
sal_Int32 OSocket::setSendBufSize(sal_Int32 sz) const
{
    sal_Int32 old = 0;

    getOption(TOption_SndBuf, &old, sizeof(old));

    if (sz >= 0)
        setOption(TOption_SndBuf, &sz, sizeof(sz));

    return (old);
}

/*****************************************************************************/
// setRecvBufSize
/*****************************************************************************/
sal_Int32 OSocket::setRecvBufSize(sal_Int32 sz) const
{
    sal_Int32 old = 0;

    getOption(TOption_RcvBuf, &old, sizeof(old));

    if (sz >= 0)
        setOption(TOption_RcvBuf, &sz, sizeof(sz));

    return (old);
}

/*****************************************************************************/
// setTcpNoDelay
/*****************************************************************************/
sal_Int32 OSocket::setTcpNoDelay(sal_Int32 sz) const
{
    sal_Int32 old = 0;

    getOption(TOption_TcpNoDelay, &old, sizeof(old), TLevel_Tcp);

    if (sz >= 0)
        setOption(TOption_TcpNoDelay, &sz, sizeof(sz), TLevel_Tcp);

    return (old);
}

/*****************************************************************************/
// getError
/*****************************************************************************/
//void OSocket::getError(sal_Char* pBuffer, sal_uInt32 nSize) const
void OSocket::getError( rtl::OUString& pBuffer ) const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if (m_pSockRef && (*m_pSockRef)())
        osl_getLastSocketErrorDescription((*m_pSockRef)(), &pBuffer.pData );
    else
        osl_getLastSocketErrorDescription(NULL, &pBuffer.pData );
}

/*****************************************************************************/
// getError
/*****************************************************************************/
OSocket::TSocketError OSocket::getError() const
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if (m_pSockRef && (*m_pSockRef)())
        return (TSocketError)osl_getLastSocketError((*m_pSockRef)());
    else
        return (TSocketError)osl_getLastSocketError(NULL);
}



VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OAcceptorSocket, vos),
                        VOS_NAMESPACE(OAcceptorSocket, vos),
                        VOS_NAMESPACE(OSocket, vos), 0);


/*****************************************************************************/
// OAcceptorSocket
/*****************************************************************************/
OAcceptorSocket::OAcceptorSocket(TAddrFamily Family,
                                 TProtocol   Protocol,
                                 TSocketType Type) :
    OSocket(Type, Family, Protocol)
{
}

/*****************************************************************************/
// OAcceptorSocket
/*****************************************************************************/
OAcceptorSocket::OAcceptorSocket(const OAcceptorSocket& sock) :
    OSocket(sock)
{
}

/*****************************************************************************/
// ~OAcceptorSocket
/*****************************************************************************/
OAcceptorSocket::~OAcceptorSocket()
{
}

/*****************************************************************************/
// close
/*****************************************************************************/
void OAcceptorSocket::close()
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if (m_pSockRef && (m_pSockRef->release() == 0))
    {
        // shutdown() needed only on some systems to unblock accept
        osl_shutdownSocket((*m_pSockRef)(), osl_Socket_DirReadWrite);
        osl_destroySocket((*m_pSockRef)());
        delete m_pSockRef;
    }
    m_pSockRef= 0;
}

/*****************************************************************************/
// listen
/*****************************************************************************/
sal_Bool OAcceptorSocket::listen(sal_Int32 MaxPendingConnections)
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        return osl_listenOnSocket((*m_pSockRef)(), MaxPendingConnections);
    }

    return sal_False;
}


/*****************************************************************************/
// acceptConnection
/*****************************************************************************/
OSocket::TResult OAcceptorSocket::acceptConnection(OStreamSocket& connection)
{
    if (m_pRecvTimeout && ! isRecvReady(m_pRecvTimeout))
        return TResult_TimedOut;

    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        connection= osl_acceptConnectionOnSocket((*m_pSockRef)(), 0);
    }

    if(connection.isValid())
        return TResult_Ok;
    else
        return TResult_Error;
}

/*****************************************************************************/
// acceptConnection
/*****************************************************************************/
OSocket::TResult OAcceptorSocket::acceptConnection(OStreamSocket& connection,
                                                   OSocketAddr& sa)
{
    oslSocketAddr PeerAddr;
    oslSocket     Socket = 0;

    if (m_pRecvTimeout && ! isRecvReady(m_pRecvTimeout))
        return TResult_TimedOut;

    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        Socket= osl_acceptConnectionOnSocket((*m_pSockRef)(), &PeerAddr);
    }

    if (Socket)
    {
        sa= PeerAddr;
        connection= Socket;
        return TResult_Ok;
    }
    else
    {
        return TResult_Error;
    }
}


VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OStreamSocket, vos),
                        VOS_NAMESPACE(OStreamSocket, vos),
                        VOS_NAMESPACE(OSocket, vos), 0);



/*****************************************************************************/
// OStreamSocket
/*****************************************************************************/
OStreamSocket::OStreamSocket()
{
}

/*****************************************************************************/
// OStreamSocket
/*****************************************************************************/
OStreamSocket::OStreamSocket(TAddrFamily Family,
                             TProtocol   Protocol,
                             TSocketType Type) :
    OSocket(Type, Family, Protocol)
{
}


/*****************************************************************************/
// OStreamSocket
/*****************************************************************************/
OStreamSocket::OStreamSocket(oslSocket Socket) :
    OSocket(Socket)
{
}

/*****************************************************************************/
// OStreamSocket
// copy constructor
/*****************************************************************************/
OStreamSocket::OStreamSocket(const OStreamSocket& sock) :
    OSocket(sock)
{
}

/*****************************************************************************/
// OStreamSocket
// copy constructor
/*****************************************************************************/
OStreamSocket::OStreamSocket(const OSocket& sock) :
    OSocket(sock)
{
}

/*****************************************************************************/
// ~OStreamSocket
/*****************************************************************************/
OStreamSocket::~OStreamSocket()
{
}

/*****************************************************************************/
// close
/*****************************************************************************/
void OStreamSocket::close()
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if (m_pSockRef && (m_pSockRef->release() == 0))
    {
        shutdown();
        osl_destroySocket((*m_pSockRef)());
        delete m_pSockRef;
    }

    m_pSockRef= 0;
}


/*****************************************************************************/
// operator=(oslSocket)
/*****************************************************************************/
OStreamSocket& OStreamSocket::operator=(oslSocket Socket)
{
    OSocket::operator=(Socket);

    return *this;
}

/*****************************************************************************/
// operator=
/*****************************************************************************/
OStreamSocket& OStreamSocket::operator= (const OSocket& sock)
{
    OSocket::operator=(sock);

    return *this;
}

/*****************************************************************************/
// operator=
/*****************************************************************************/
OStreamSocket& OStreamSocket::operator= (const OStreamSocket& sock)
{
    OSocket::operator=(sock);

    return *this;
}

/*****************************************************************************/
// read
/*****************************************************************************/
sal_Int32 OStreamSocket::read(void* pBuffer, sal_uInt32 n) const
{
    sal_uInt8 SAL_HUGE *Ptr = (sal_uInt8 SAL_HUGE *)pBuffer;

    if (m_pRecvTimeout && ! isRecvReady(m_pRecvTimeout))
        return 0;

    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( ! ( m_pSockRef && (*m_pSockRef)() ) )
    {
        return -1;
    }

    /* loop until all desired bytes were read or an error occured */
    sal_uInt32 BytesRead= 0;
    sal_uInt32 BytesToRead= n;
    while (BytesToRead > 0)
    {
        sal_Int32 RetVal;
        RetVal= osl_receiveSocket((*m_pSockRef)(),
                                   Ptr,
                                   BytesToRead,
                                   osl_Socket_MsgNormal);

        /* error occured? */
        if(RetVal <= 0)
        {
            break;
        }

        BytesToRead -= RetVal;
        BytesRead += RetVal;
        Ptr += RetVal;

        /* wait for next available data or timeout */
        if (m_pRecvTimeout && ! isRecvReady(m_pRecvTimeout))
            break;

    }

    return BytesRead;
}

/*****************************************************************************/
// write
/*****************************************************************************/
sal_Int32 OStreamSocket::write(const void* pBuffer, sal_uInt32 n)
{
    sal_uInt8 SAL_HUGE *Ptr = (sal_uInt8 SAL_HUGE *)pBuffer;

    if (m_pSendTimeout && ! isSendReady(m_pSendTimeout))
        return 0;

    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( ! ( m_pSockRef && (*m_pSockRef)() ) )
    {
        return -1;
    }

    /* loop until all desired bytes were send or an error occured */
    sal_uInt32 BytesSend= 0;
    sal_uInt32 BytesToSend= n;
    while (BytesToSend > 0)
    {
        sal_Int32 RetVal;

        RetVal= osl_sendSocket((*m_pSockRef)(),
                                Ptr,
                                BytesToSend,
                                osl_Socket_MsgNormal);

        /* error occured? */
        if(RetVal <= 0)
        {
            break;
        }

        BytesToSend -= RetVal;
        BytesSend += RetVal;
        Ptr += RetVal;

        /* wait till new data is available or timeout occures */
        if (m_pSendTimeout && ! isSendReady(m_pSendTimeout))
            break;
    }

    return BytesSend;
}

sal_Bool OStreamSocket::isEof() const
{
    return isValid();
    // BHO not enough
}

/*****************************************************************************/
// recv
/*****************************************************************************/
sal_Int32 OStreamSocket::recv(void* pBuffer,
                            sal_uInt32 BytesToRead,
                            TSocketMsgFlag Flag)
{
    if (m_pRecvTimeout && ! isRecvReady(m_pRecvTimeout))
        return 0;

    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( ! ( m_pSockRef && (*m_pSockRef)() ) )
    {
        return -1;
    }

    return osl_receiveSocket((*m_pSockRef)(),
                             pBuffer,
                             BytesToRead,
                             (oslSocketMsgFlag)Flag);
}

/*****************************************************************************/
// send
/*****************************************************************************/
sal_Int32 OStreamSocket::send(const void* pBuffer,
                            sal_uInt32 BytesToSend,
                            TSocketMsgFlag Flag)
{
    if (m_pSendTimeout && ! isSendReady(m_pSendTimeout))
        return 0;

    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( ! ( m_pSockRef && (*m_pSockRef)() ) )
    {
        return -1;
    }

    return osl_sendSocket((*m_pSockRef)(),
                          pBuffer,
                          BytesToSend,
                          (oslSocketMsgFlag)Flag);
}

/*****************************************************************************/
// shutdown
/*****************************************************************************/
sal_Bool OStreamSocket::shutdown(TSocketDirection Direction)
{
    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        return osl_shutdownSocket((*m_pSockRef)(), (oslSocketDirection)Direction);
    }

    return sal_False;
}



VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OConnectorSocket, vos),
                        VOS_NAMESPACE(OConnectorSocket, vos),
                        VOS_NAMESPACE(OStreamSocket, vos), 0);



/*****************************************************************************/
// OConnectorSocket
/*****************************************************************************/
OConnectorSocket::OConnectorSocket(TAddrFamily Family,
                                   TProtocol   Protocol,
                                   TSocketType Type) :
    OStreamSocket(Family, Protocol, Type)
{
}

/*****************************************************************************/
// OConnectorSocket
/*****************************************************************************/
OConnectorSocket::OConnectorSocket(const OConnectorSocket& sock) :
    OStreamSocket(sock)
{
}

/*****************************************************************************/
// ~OConnectorSocket
/*****************************************************************************/
OConnectorSocket::~OConnectorSocket()
{
}

/*****************************************************************************/
// connect
/*****************************************************************************/
OSocket::TResult OConnectorSocket::connect(const OSocketAddr& Addr,
                                           const TimeValue* pTimeout)
{

    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( m_pSockRef && (*m_pSockRef)() )
    {
        return (TResult)osl_connectSocketTo((*m_pSockRef)(),
                                            (oslSocketAddr)Addr,
                                            pTimeout);
    }

    return TResult_Error;
}


VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(ODatagramSocket, vos),
                        VOS_NAMESPACE(ODatagramSocket, vos),
                        VOS_NAMESPACE(OSocket, vos), 0);


/*****************************************************************************/
// ODatagramSocket
/*****************************************************************************/
ODatagramSocket::ODatagramSocket(TAddrFamily Family,
                                 TProtocol   Protocol,
                                 TSocketType Type) :
    OSocket(Type, Family, Protocol)
{
}

/*****************************************************************************/
// ODatagramSocket
/*****************************************************************************/
ODatagramSocket::ODatagramSocket(const ODatagramSocket& sock) :
    OSocket(sock)
{
}

/*****************************************************************************/
// ~ODatagramSocket
/*****************************************************************************/
ODatagramSocket::~ODatagramSocket()
{
}


/*****************************************************************************/
// recvFrom
/*****************************************************************************/
sal_Int32 ODatagramSocket::recvFrom(void* pBuffer,
                                  sal_uInt32 BufferSize,
                                    OSocketAddr* pSenderAddr,
                                    TSocketMsgFlag Flag)
{

    if (m_pRecvTimeout && ! isRecvReady(m_pRecvTimeout))
        return 0;

    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( ! ( m_pSockRef && (*m_pSockRef)() ) )
    {
        return -1;
    }

    sal_Int32 BytesRead;

    if(pSenderAddr)
    {
        // we are interested in the senders address
        oslSocketAddr SenderAddr= osl_createEmptySocketAddr(osl_Socket_FamilyInet);

        BytesRead=
            osl_receiveFromSocket((*m_pSockRef)(),
                                   SenderAddr,
                                   pBuffer,
                                   BufferSize,
                                   (oslSocketMsgFlag)Flag);

        *pSenderAddr= SenderAddr;
    }
    else
    {
        // we don't want to know the senders address

        BytesRead=
            osl_receiveFromSocket((*m_pSockRef)(),
                                   0,
                                   pBuffer,
                                   BufferSize,
                                   (oslSocketMsgFlag)Flag);
    }

    return BytesRead;
}


/*****************************************************************************/
// sendTo
/*****************************************************************************/
sal_Int32 ODatagramSocket::sendTo(const OSocketAddr& ReceiverAddr,
                                const void* pBuffer,
                                sal_uInt32 BufferSize,
                                TSocketMsgFlag Flag)
{
    if (m_pSendTimeout && ! isSendReady(m_pSendTimeout))
        return 0;

    VOS_ASSERT(m_pSockRef && (*m_pSockRef)());

    if ( ( m_pSockRef && (*m_pSockRef)() ) )
    {

        return osl_sendToSocket((*m_pSockRef)(),
                                (oslSocketAddr)ReceiverAddr,
                                pBuffer,
                                BufferSize,
                                (oslSocketMsgFlag)Flag);
    }

    return -1;
}

