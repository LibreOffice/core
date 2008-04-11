/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: urp_bridgeimpl.cxx,v $
 * $Revision: 1.13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"
#include <osl/thread.h>
#include <bridges/remote/helper.hxx>

#include <algorithm>

#include "urp_bridgeimpl.hxx"

using namespace ::rtl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
namespace bridges_urp
{

template < class t >
inline t mymin( const t & val1, const t & val2 )
{
    return val1 < val2 ? val1 : val2;
}

/***********
 * urp_BridgeImpl
 ***********/
urp_BridgeImpl::urp_BridgeImpl( sal_Int32 nCacheSize , sal_uInt32 nInitialMarshalerSize ) :
    m_blockMarshaler( this , nInitialMarshalerSize , ::bridges_remote::remote_retrieveOidFromProxy),
    m_nMarshaledMessages( 0 ),
    m_oidCacheOut( (sal_uInt16)nCacheSize ),
    m_tidCacheOut( (sal_uInt16)nCacheSize ),
    m_typeCacheOut( (sal_uInt16)nCacheSize )
{
    m_pOidIn = new OUString[ nCacheSize ];
    m_pTidIn = new ByteSequence[ nCacheSize ];
    m_pTypeIn = new Type[ nCacheSize ];
    m_nRemoteThreads = 0;
}

urp_BridgeImpl::~urp_BridgeImpl()
{
    delete [] m_pOidIn;
    delete [] m_pTidIn;
    delete [] m_pTypeIn;
}


void urp_BridgeImpl::applyProtocolChanges( const Properties &props )
{
    if( m_properties.nTypeCacheSize != props.nTypeCacheSize )
    {
        if( props.nTypeCacheSize == 0 )
        {
            delete [] m_pTypeIn;
            m_pTypeIn = 0;
        }
        else
        {
            Type *pNew = new Type[props.nTypeCacheSize];
            sal_Int32 i;
            sal_Int32 iMin = mymin( m_properties.nTypeCacheSize , props.nTypeCacheSize );
            for( i = 0; i < iMin ; i ++ )
            {
                pNew[i] = m_pTypeIn[i];
            }
            delete [] m_pTypeIn;
            m_pTypeIn = pNew;
        }
        OSL_ASSERT( props.nTypeCacheSize <= 0xffff );
        m_properties.nTypeCacheSize = props.nTypeCacheSize;
        m_typeCacheOut.resize( (sal_uInt16)props.nTypeCacheSize );
    }

     if( m_properties.nOidCacheSize != props.nOidCacheSize )
     {
         if( 0 == props.nOidCacheSize )
         {
             delete [] m_pOidIn;
             m_pOidIn = 0;
         }
         else
         {
             OUString *pNew = new OUString[props.nOidCacheSize];
             sal_Int32 i;
             sal_Int32 iMin = mymin( m_properties.nOidCacheSize , props.nOidCacheSize );
             for( i = 0; i < iMin ; i ++ )
             {
                 pNew[i] = m_pOidIn[i];
             }
             delete [] m_pOidIn;
             m_pOidIn = pNew;
         }
        OSL_ASSERT( props.nOidCacheSize <= 0xffff );
         m_oidCacheOut.resize( (sal_uInt16)props.nOidCacheSize );
         m_properties.nOidCacheSize = props.nOidCacheSize;
     }

     if( m_properties.nTidCacheSize != props.nTidCacheSize )
     {
         if( 0 == props.nTidCacheSize )
         {
             delete [] m_pTidIn;
            m_pTidIn = 0;
         }
         else
         {
             ByteSequence *pNew = new ByteSequence[props.nTidCacheSize];
             sal_Int32 i;
             sal_Int32 iMin = mymin( m_properties.nTidCacheSize , props.nTidCacheSize );
             for( i = 0; i < iMin ; i ++ )
             {
                 pNew[i] = m_pTidIn[i];
             }
             delete [] m_pTidIn;
             m_pTidIn = pNew;
         }
        OSL_ASSERT( props.nTidCacheSize <= 0xffff );
         m_tidCacheOut.resize( (sal_uInt16)props.nTidCacheSize );
         m_properties.nTidCacheSize = props.nTidCacheSize;
     }

     if( m_properties.sVersion != props.sVersion )
     {
        m_properties.sVersion = props.sVersion;
     }

    if( m_properties.nFlushBlockSize != props.nFlushBlockSize )
    {
        m_properties.nFlushBlockSize = props.nFlushBlockSize;
    }

    if( m_properties.nOnewayTimeoutMUSEC != props.nOnewayTimeoutMUSEC )
    {
        m_properties.nOnewayTimeoutMUSEC = props.nOnewayTimeoutMUSEC;
    }

    if( props.bClearCache )
    {
        if( m_properties.nTypeCacheSize )
        {
            delete [] m_pTypeIn;
            m_pTypeIn = new Type[m_properties.nTypeCacheSize];
            m_typeCacheOut.clear();
        }
        m_lastInType = Type();
        m_lastOutType = Type();

        if( m_properties.nOidCacheSize )
        {
            delete [] m_pOidIn;
            m_pOidIn = new OUString[ m_properties.nOidCacheSize];
            m_oidCacheOut.clear();
        }
        m_lastOutOid = OUString();
        m_lastInOid = OUString();

        if( m_properties.nTidCacheSize )
        {
            delete [] m_pTidIn;
            m_pTidIn = new ByteSequence[m_properties.nTidCacheSize];
            m_tidCacheOut.clear();
        }
        m_lastInTid = ByteSequence();
        m_lastOutTid = ByteSequence();
    }

    if( m_properties.bNegotiate != props.bNegotiate )
    {
        m_properties.bNegotiate = props.bNegotiate;
    }

    if( m_properties.bForceSynchronous != props.bForceSynchronous )
    {
        m_properties.bForceSynchronous = props.bForceSynchronous;
    }

    m_properties.bCurrentContext = props.bCurrentContext;
}

void urp_BridgeImpl::addError( char const *pError )
{
    OUString message = OUString( RTL_CONSTASCII_USTRINGPARAM( "(tid=" ) );
    message += OUString::valueOf( (sal_Int32 ) osl_getThreadIdentifier( 0 ) );
    message += OUString::createFromAscii( ") " );
    message += OUString::createFromAscii( pError );
    MutexGuard guard( m_errorListMutex );
    m_lstErrors.push_back( message );
}

void urp_BridgeImpl::addError( const OUString & error )
{
    OUString message = OUString( RTL_CONSTASCII_USTRINGPARAM( "(tid=" ) );
    message += OUString::valueOf( (sal_Int32 ) osl_getThreadIdentifier( 0 ) );
    message += OUString::createFromAscii( ") " );
    message += error;
    MutexGuard guard( m_errorListMutex );
    m_lstErrors.push_back( message );
}

void urp_BridgeImpl::dumpErrors( FILE * f)
{
    MutexGuard guard( m_errorListMutex );
    for( ::std::list< OUString >::iterator ii = m_lstErrors.begin() ;
         ii != m_lstErrors.end() ;
         ++ii )
    {
        OString o = OUStringToOString( *ii , RTL_TEXTENCODING_UTF8 );
        fprintf( f,  "%s\n" , o.getStr() );
    }
}

OUString urp_BridgeImpl::getErrorsAsString( )
{
    MutexGuard guard( m_errorListMutex );
    OUString ret;
    for( ::std::list< OUString >::iterator ii = m_lstErrors.begin() ;
         ii != m_lstErrors.end() ;
         ++ii )
    {
        ret += *ii;
    }
    return ret;
}

}
