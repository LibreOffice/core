/*************************************************************************
 *
 *  $RCSfile: resultsetforquery.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: abi $ $Date: 2001-07-16 15:20:25 $
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

#ifndef _COM_SUN_STAR_UCB_COMMAND_HPP_
#include <com/sun/star/ucb/Command.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _RESULTSETFORROOT_HXX
#include <provider/resultsetforquery.hxx>
#endif
#ifndef _DATABASES_HXX_
#include <provider/databases.hxx>
#endif
#ifndef _XMLSEARCH_QE_QUERYPROCESSOR_HXX_
#include <qe/QueryProcessor.hxx>
#endif
#ifndef INCLUDED_STL_SET
#include <set>
#define INCLUDED_STL_SET
#endif

using namespace chelp;
using namespace xmlsearch::excep;
using namespace xmlsearch::qe;
using namespace com::sun::star;
using namespace com::sun::star::ucb;



ResultSetForQuery::ResultSetForQuery( const uno::Reference< lang::XMultiServiceFactory >&  xMSF,
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
    std::vector< rtl::OUString > queryList;

    {
        sal_Int32 idx;
        rtl::OUString query = m_aURLParameter.get_query();
        while( query.getLength() )
        {
            idx = query.indexOf( sal_Unicode( ' ' ) );
            if( idx == -1 )
                idx = query.getLength();

            queryList.push_back( query.copy( 0,idx ) );
            query = query.copy( 1 + idx );
        }
    }

    rtl::OUString scope = m_aURLParameter.get_scope();
    StaticModuleInformation* inf =
        m_pDatabases->getStaticInformationForModule( m_aURLParameter.get_module(),
                                                     m_aURLParameter.get_language() );

    if( scope.compareToAscii( "Heading" ) == 0 )
        scope = inf->get_heading();
    else
        scope = inf->get_fulltext();

    sal_Int32 hitCount = m_aURLParameter.get_hitCount();

    QueryStatement queryStatement( hitCount,queryList,scope );
    QueryResults *queryResults = 0;

    try
    {
        QueryProcessor queryProcessor( m_pDatabases->getInstallPathAsURL()                    +
                                       m_pDatabases->lang( m_aURLParameter.get_language() )   +
                                       rtl::OUString::createFromAscii( "/" )                  +
                                       m_aURLParameter.get_module()                           +
                                       rtl::OUString::createFromAscii( ".idx/" ) );

        queryResults = queryProcessor.processQuery( queryStatement );
    }
    catch( IOException )
    {
    }

    sal_Int32 replIdx = rtl::OUString::createFromAscii( "#HLP#" ).getLength();
    rtl::OUString replWith = rtl::OUString::createFromAscii( "vnd.sun.star.help://" );

    QueryHitIterator* it = 0;
    if( queryResults )
        it = queryResults->makeQueryHitIterator();

    rtl::OUString aStr;
    std::set< rtl::OUString > aSet;

    while( it && it->next() )
    {
        QueryHitData* qhd = it->getHit( 0 );
        if( qhd &&
            aSet.insert(qhd->getDocument()).second )
        {
            m_aPath.push_back( replWith + (qhd->getDocument()).copy( replIdx ) );
        }
    }
    delete it;  // deletes also queryResults

    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    Command aCommand;
    aCommand.Name = rtl::OUString::createFromAscii( "getPropertyValues" );
    aCommand.Argument <<= m_sProperty;

    for( m_nRow = 0; m_nRow < m_aPath.size(); ++m_nRow )
    {
        m_aPath[m_nRow] =
            m_aPath[m_nRow]                                          +
            rtl::OUString::createFromAscii( "?Language=" )           +
            m_aURLParameter.get_language()                           +
            rtl::OUString::createFromAscii( "&System=" )             +
            m_aURLParameter.get_system();

        uno::Reference< XContent > content = queryContent();
        if( content.is() )
        {
            uno::Reference< XCommandProcessor > cmd( content,uno::UNO_QUERY );
            if( ! ( cmd->execute( aCommand,0,uno::Reference< XCommandEnvironment >( 0 ) ) >>= m_aItems[m_nRow] ) )
                ;
        }
    }
    m_nRow = -1;
}
