/*************************************************************************
 *
 *  $RCSfile: ConnectionWrapper.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:51:13 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef _CONNECTIVITY_CONNECTIONWRAPPER_HXX_
#include "connectivity/ConnectionWrapper.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _RTL_DIGEST_H_
#include <rtl/digest.h>
#endif
#include <algorithm>

#include <algorithm>

using namespace connectivity;
//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
// --------------------------------------------------------------------------------
OConnectionWrapper::OConnectionWrapper()
{

}
// -----------------------------------------------------------------------------
void OConnectionWrapper::setDelegation(Reference< XAggregation >& _rxProxyConnection,oslInterlockedCount& _rRefCount)
{
    OSL_ENSURE(_rxProxyConnection.is(),"OConnectionWrapper: Connection must be valid!");
    osl_incrementInterlockedCount( &_rRefCount );
    if (_rxProxyConnection.is())
    {
        // transfer the (one and only) real ref to the aggregate to our member
        m_xProxyConnection = _rxProxyConnection;
        _rxProxyConnection = NULL;
        ::comphelper::query_aggregation(m_xProxyConnection,m_xConnection);
        m_xTypeProvider.set(m_xConnection,UNO_QUERY);
        m_xUnoTunnel.set(m_xConnection,UNO_QUERY);
        m_xServiceInfo.set(m_xConnection,UNO_QUERY);

        // set ourself as delegator
        Reference<XInterface> xIf = static_cast< XUnoTunnel* >( this );
        m_xProxyConnection->setDelegator( xIf );

    }
    osl_decrementInterlockedCount( &_rRefCount );
}
// -----------------------------------------------------------------------------
void OConnectionWrapper::disposing()
{
    m_xConnection = NULL;
}
//-----------------------------------------------------------------------------
OConnectionWrapper::~OConnectionWrapper()
{
    if (m_xProxyConnection.is())
        m_xProxyConnection->setDelegator(NULL);
}

// XServiceInfo
// --------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnectionWrapper::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdbc.drivers.OConnectionWrapper" ) );
}

// --------------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OConnectionWrapper::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    // first collect the services which are supported by our aggregate
    Sequence< ::rtl::OUString > aSupported;
    if ( m_xServiceInfo.is() )
        aSupported = m_xServiceInfo->getSupportedServiceNames();

    // append our own service, if necessary
    ::rtl::OUString sConnectionService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdbc.Connection" ) );
    if ( 0 == ::comphelper::findValue( aSupported, sConnectionService, sal_True ).getLength() )
    {
        sal_Int32 nLen = aSupported.getLength();
        aSupported.realloc( nLen + 1 );
        aSupported[ nLen ] = sConnectionService;
    }

    // outta here
    return aSupported;
}

// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OConnectionWrapper::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::comphelper::findValue( getSupportedServiceNames(), _rServiceName, sal_True ).getLength() != 0;
}

// --------------------------------------------------------------------------------
Any SAL_CALL OConnectionWrapper::queryInterface( const Type& _rType ) throw (RuntimeException)
{
    Any aReturn = OConnection_BASE::queryInterface(_rType);
    return aReturn.hasValue() ? aReturn : (m_xProxyConnection.is() ? m_xProxyConnection->queryAggregation(_rType) : aReturn);
}
// --------------------------------------------------------------------------------
Sequence< Type > SAL_CALL OConnectionWrapper::getTypes(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return ::comphelper::concatSequences(
        OConnection_BASE::getTypes(),
        m_xTypeProvider->getTypes()
    );
}
// -----------------------------------------------------------------------------
// com::sun::star::lang::XUnoTunnel
sal_Int64 SAL_CALL OConnectionWrapper::getSomething( const Sequence< sal_Int8 >& rId ) throw(RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    if(m_xUnoTunnel.is())
        return m_xUnoTunnel->getSomething(rId);
    return 0;
}

// -----------------------------------------------------------------------------
Sequence< sal_Int8 > OConnectionWrapper::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}
// -----------------------------------------------------------------------------
namespace
{
    class TPropertyValueLessFunctor : public ::std::binary_function< ::com::sun::star::beans::PropertyValue,::com::sun::star::beans::PropertyValue,bool>
    {
    public:
        TPropertyValueLessFunctor()
        {}
        bool operator() (const ::com::sun::star::beans::PropertyValue& lhs, const ::com::sun::star::beans::PropertyValue& rhs) const
        {
            return !!(lhs.Name.equalsIgnoreAsciiCase( rhs.Name ));
        }
    };

}

// -----------------------------------------------------------------------------
// creates a unique id out of the url and sequence of properties
void OConnectionWrapper::createUniqueId( const ::rtl::OUString& _rURL
                    ,Sequence< PropertyValue >& _rInfo
                    ,sal_uInt8* _pBuffer
                    ,const ::rtl::OUString& _rUserName
                    ,const ::rtl::OUString& _rPassword)
{
    // first we create the digest we want to have
    rtlDigest aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtlDigestError aError = rtl_digest_update(aDigest,_rURL.getStr(),_rURL.getLength()*sizeof(sal_Unicode));
    if ( _rUserName.getLength() )
        aError = rtl_digest_update(aDigest,_rUserName.getStr(),_rUserName.getLength()*sizeof(sal_Unicode));
    if ( _rPassword.getLength() )
        aError = rtl_digest_update(aDigest,_rPassword.getStr(),_rPassword.getLength()*sizeof(sal_Unicode));
    // now we need to sort the properties
    PropertyValue* pBegin = _rInfo.getArray();
    PropertyValue* pEnd   = pBegin + _rInfo.getLength();
    ::std::sort(pBegin,pEnd,TPropertyValueLessFunctor());

    pBegin = _rInfo.getArray();
    pEnd   = pBegin + _rInfo.getLength();
    for (; pBegin != pEnd; ++pBegin)
    {
        // we only include strings an integer values
        ::rtl::OUString sValue;
        if ( pBegin->Value >>= sValue )
            ;
        else
        {
            sal_Int32 nValue = 0;
            if ( pBegin->Value >>= nValue )
                sValue = ::rtl::OUString::valueOf(nValue);
            else
            {
                Sequence< ::rtl::OUString> aSeq;
                if ( pBegin->Value >>= aSeq )
                {
                    const ::rtl::OUString* pSBegin = aSeq.getConstArray();
                    const ::rtl::OUString* pSEnd   = pSBegin + aSeq.getLength();
                    for(;pSBegin != pSEnd;++pSBegin)
                        aError = rtl_digest_update(aDigest,pSBegin->getStr(),pSBegin->getLength()*sizeof(sal_Unicode));
                }
            }
        }
        if ( sValue.getLength() > 0 )
        {
            // we don't have to convert this into UTF8 because we don't store on a file system
            aError = rtl_digest_update(aDigest,sValue.getStr(),sValue.getLength()*sizeof(sal_Unicode));
        }
    }

    aError = rtl_digest_get(aDigest,_pBuffer,RTL_DIGEST_LENGTH_SHA1);
    // we have to destroy the digest
    rtl_digest_destroy(aDigest);
}
// -----------------------------------------------------------------------------


