/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: resultsetforquery.cxx,v $
 * $Revision: 1.18 $
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
#include "precompiled_xmlhelp.hxx"
#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/i18n/XExtendedTransliteration.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/script/XInvocation.hpp>

#include <qe/QueryProcessor.hxx>
#ifndef INCLUDED_STL_ALGORITHM
#include <algorithm>
#define INCLUDED_STL_ALGORITHM
#endif
#ifndef INCLUDED_STL_SET
#include <set>
#define INCLUDED_STL_SET
#endif

#include "resultsetforquery.hxx"
#include "databases.hxx"

using namespace std;
using namespace chelp;
using namespace xmlsearch::excep;
using namespace xmlsearch::qe;
using namespace com::sun::star;
using namespace com::sun::star::ucb;
using namespace com::sun::star::i18n;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

ResultSetForQuery::ResultSetForQuery( const uno::Reference< lang::XMultiServiceFactory >&  xMSF,
                                      const uno::Reference< XContentProvider >&  xProvider,
                                      sal_Int32 nOpenMode,
                                      const uno::Sequence< beans::Property >& seq,
                                      const uno::Sequence< NumberedSortingInfo >& seqSort,
                                      URLParameter& aURLParameter,
                                      Databases* pDatabases )
    : ResultSetBase( xMSF,xProvider,nOpenMode,seq,seqSort ),
      m_pDatabases( pDatabases ),
      m_aURLParameter( aURLParameter )
{
    Reference< XTransliteration > xTrans(
        xMSF->createInstance( rtl::OUString::createFromAscii( "com.sun.star.i18n.Transliteration" ) ),
        UNO_QUERY );
    Locale aLocale( aURLParameter.get_language(),
                    rtl::OUString(),
                    rtl::OUString() );
    if(xTrans.is())
        xTrans->loadModule(TransliterationModules_UPPERCASE_LOWERCASE,
                           aLocale );

    // Access Lucene via XInvocation
    Reference< script::XInvocation > xInvocation(
        xMSF->createInstance( rtl::OUString::createFromAscii( "com.sun.star.help.HelpSearch" ) ),
        UNO_QUERY );

    unsigned int i;
    vector< vector< rtl::OUString > > queryList;
    {
        sal_Int32 idx;
        rtl::OUString query = m_aURLParameter.get_query();
        while( query.getLength() )
        {
            idx = query.indexOf( sal_Unicode( ' ' ) );
            if( idx == -1 )
                idx = query.getLength();

            vector< rtl::OUString > currentQuery;
            rtl::OUString tmp(query.copy( 0,idx ));
            rtl:: OUString toliterate = tmp;
            if(xTrans.is()) {
                Sequence<sal_Int32> aSeq;
                toliterate = xTrans->transliterate(
                    tmp,0,tmp.getLength(),aSeq);
            }

            currentQuery.push_back( toliterate );
            queryList.push_back( currentQuery );

            int nCpy = 1 + idx;
            if( nCpy >= query.getLength() )
                query = rtl::OUString();
            else
                query = query.copy( 1 + idx );
        }
    }

    set< rtl::OUString > aSet,aCurrent,aResultSet;
    if( xInvocation.is() )
    {
        rtl::OUString scope = m_aURLParameter.get_scope();
        bool bCaptionsOnly = ( scope.compareToAscii( "Heading" ) == 0 );
        sal_Int32 hitCount = m_aURLParameter.get_hitCount();

        try
        {
            rtl::OUString idxDir;
            idxDir =
                m_pDatabases->getInstallPathAsURL()                    +
                m_pDatabases->lang( m_aURLParameter.get_language() )   +
                rtl::OUString::createFromAscii( "/" )                  +
                m_aURLParameter.get_module()                           +
                rtl::OUString::createFromAscii( ".idxl/" );

            if(queryList.size() > 1)
                hitCount = 2000;

            for( i = 0; i < queryList.size(); ++i )
            {
                if( xInvocation.is() )
                {
                    int nParamCount = bCaptionsOnly ? 7 : 6;
                    Sequence<uno::Any> aParamsSeq( nParamCount );

                    aParamsSeq[0] = uno::makeAny( rtl::OUString::createFromAscii( "-lang" ) );
                    aParamsSeq[1] = uno::makeAny( m_aURLParameter.get_language() );

                    aParamsSeq[2] = uno::makeAny( rtl::OUString::createFromAscii( "-index" ) );
                    rtl::OUString aSystemPath;
                    osl::FileBase::getSystemPathFromFileURL( idxDir, aSystemPath );
                    aParamsSeq[3] = uno::makeAny( aSystemPath );

                    aParamsSeq[4] = uno::makeAny( rtl::OUString::createFromAscii( "-query" ) );

                    const std::vector< rtl::OUString >& aListItem = queryList[i];
                    ::rtl::OUString aNewQueryStr = aListItem[0];
                    aParamsSeq[5] = uno::makeAny( aNewQueryStr );

                    if( bCaptionsOnly )
                        aParamsSeq[6] = uno::makeAny( rtl::OUString::createFromAscii( "-caption" ) );

                    Sequence< sal_Int16 > aOutParamIndex;
                    Sequence< uno::Any > aOutParam;
                    uno::Any aRet = xInvocation->invoke( rtl::OUString::createFromAscii( "search" ),
                        aParamsSeq, aOutParamIndex, aOutParam );

                    Sequence<rtl::OUString> aRetSeq;
                    if( aRet >>= aRetSeq )
                    {
                        aSet.clear();

                        const rtl::OUString* pRetSeq = aRetSeq.getConstArray();
                        int nCount = aRetSeq.getLength();
                        if( nCount > hitCount )
                            nCount = hitCount;
                        for( int j = 0 ; j < nCount ; ++j )
                            aSet.insert( pRetSeq[j] );
                    }
                }

                // intersect
                if( i == 0 )
                    aResultSet = aSet;
                else
                {
                    aCurrent = aResultSet;
                    aResultSet.clear();
                    set_intersection( aSet.begin(),aSet.end(),
                                      aCurrent.begin(),aCurrent.end(),
                                      inserter(aResultSet,aResultSet.begin()));
                }
            }
        }
        catch( const Exception& )
        {
        }
    }

    sal_Int32 replIdx = rtl::OUString::createFromAscii( "#HLP#" ).getLength();
    rtl::OUString replWith = rtl::OUString::createFromAscii( "vnd.sun.star.help://" );

    set< rtl::OUString >::const_iterator set_it = aResultSet.begin();
    while( set_it != aResultSet.end() )
    {
        rtl::OUString aResultStr = replWith + set_it->copy(replIdx);
          m_aPath.push_back( aResultStr );
        ++set_it;
    }

    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    Command aCommand;
    aCommand.Name = rtl::OUString::createFromAscii( "getPropertyValues" );
    aCommand.Argument <<= m_sProperty;

    for( m_nRow = 0; sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aPath.size(); ++m_nRow )
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
            cmd->execute( aCommand,0,uno::Reference< XCommandEnvironment >( 0 ) ) >>= m_aItems[m_nRow]; //TODO: check return value of operator >>=
        }
    }
    m_nRow = 0xffffffff;
}
