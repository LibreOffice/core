/*************************************************************************
 *
 *  $RCSfile: urp_bridgeimpl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jbu $ $Date: 2000-11-28 14:45:18 $
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
#include <bridges/remote/helper.hxx>

#include "urp_bridgeimpl.hxx"

using namespace ::rtl;
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
    m_oidCacheOut( nCacheSize ),
    m_tidCacheOut( nCacheSize ),
    m_typeCacheOut( nCacheSize ),
    m_blockMarshaler( this , nInitialMarshalerSize , ::bridges_remote::remote_retrieveOidFromProxy),
    m_nMarshaledMessages( 0 )
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
        m_properties.nTypeCacheSize = props.nTypeCacheSize;
        m_typeCacheOut.resize( props.nTypeCacheSize );
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
         m_oidCacheOut.resize( props.nOidCacheSize );
         m_properties.nOidCacheSize = props.nOidCacheSize;
     }

     if( m_properties.nTidCacheSize != props.nTidCacheSize )
     {
         if( 0 == props.nTidCacheSize )
         {
             delete [] m_pTidIn;
             m_pOidIn = 0;
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
         m_tidCacheOut.resize( props.nTidCacheSize );
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

}

}
