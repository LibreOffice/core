/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlhelp.hxx"
#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>

#include "resultsetforroot.hxx"
#include "databases.hxx"

using namespace chelp;
using namespace com::sun::star;
using namespace com::sun::star::ucb;



ResultSetForRoot::ResultSetForRoot( const uno::Reference< lang::XMultiServiceFactory >&  xMSF,
                                    const uno::Reference< XContentProvider >&  xProvider,
                                    sal_Int32 nOpenMode,
                                    const uno::Sequence< beans::Property >& seq,
                                    const uno::Sequence< NumberedSortingInfo >& seqSort,
                                    URLParameter& aURLParameter,
                                    Databases* pDatabases )
    : ResultSetBase( xMSF,xProvider,nOpenMode,seq,seqSort ),
      m_aURLParameter( aURLParameter ),
      m_pDatabases( pDatabases )
{
    m_aPath = m_pDatabases->getModuleList( m_aURLParameter.get_language() );
    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    Command aCommand;
    aCommand.Name = rtl::OUString::createFromAscii( "getPropertyValues" );
    aCommand.Argument <<= m_sProperty;

    for( sal_uInt32 i = 0; i < m_aPath.size(); ++i )
    {
        m_aPath[i] =
            rtl::OUString::createFromAscii( "vnd.sun.star.help://" ) +
            m_aPath[i]                                               +
            rtl::OUString::createFromAscii( "?Language=" )           +
            m_aURLParameter.get_language()                           +
            rtl::OUString::createFromAscii( "&System=" )             +
            m_aURLParameter.get_system();

        m_nRow = sal_Int32( i );

        uno::Reference< XContent > content = queryContent();
        if( content.is() )
        {
            uno::Reference< XCommandProcessor > cmd( content,uno::UNO_QUERY );
            cmd->execute( aCommand,0,uno::Reference< XCommandEnvironment >( 0 ) ) >>= m_aItems[i]; //TODO: check return value of operator >>=
        }
        m_nRow = 0xffffffff;
    }
}
