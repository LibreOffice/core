/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/i18n/XExtendedTransliteration.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/script/XInvocation.hpp>

#include <helpcompiler/HelpSearch.hxx>

#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4068 4263 4264 4266)
#endif

#if defined(__GNUC__) && defined(HAVE_GCC_VISIBILITY_FEATURE)
#  pragma GCC visibility push (default)
#endif
#include <CLucene.h>
#if defined(__GNUC__) && defined(HAVE_GCC_VISIBILITY_FEATURE)
#  pragma GCC visibility pop
#endif

#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <rtl/oustringostreaminserter.hxx>

#include <algorithm>
#include <set>
#include <qe/Query.hxx>
#include <qe/DocGenerator.hxx>
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

struct HitItem
{
    rtl::OUString   m_aURL;
    float           m_fScore;

    HitItem( void ) {}
    HitItem( const rtl::OUString& aURL, float fScore )
        : m_aURL( aURL )
        , m_fScore( fScore )
    {}
    bool operator < ( const HitItem& rHitItem ) const
    {
        return rHitItem.m_fScore < m_fScore;
    }
};

ResultSetForQuery::ResultSetForQuery( const uno::Reference< lang::XMultiServiceFactory >&  xMSF,
                                      const uno::Reference< XContentProvider >&  xProvider,
                                      sal_Int32 nOpenMode,
                                      const uno::Sequence< beans::Property >& seq,
                                      const uno::Sequence< NumberedSortingInfo >& seqSort,
                                      URLParameter& aURLParameter,
                                      Databases* pDatabases )
    : ResultSetBase( xMSF,xProvider,nOpenMode,seq,seqSort ),
      m_aURLParameter( aURLParameter )
{
    Reference< XTransliteration > xTrans(
        xMSF->createInstance( rtl::OUString( "com.sun.star.i18n.Transliteration" ) ),
        UNO_QUERY );
    Locale aLocale( aURLParameter.get_language(),
                    rtl::OUString(),
                    rtl::OUString() );
    if(xTrans.is())
        xTrans->loadModule(TransliterationModules_UPPERCASE_LOWERCASE,
                           aLocale );

    vector< vector< rtl::OUString > > queryList;
    {
        sal_Int32 idx;
        rtl::OUString query = m_aURLParameter.get_query();
        while( !query.isEmpty() )
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

    vector< rtl::OUString > aCompleteResultVector;
    rtl::OUString scope = m_aURLParameter.get_scope();
    bool bCaptionsOnly = ( scope.compareToAscii( "Heading" ) == 0 );
    sal_Int32 hitCount = m_aURLParameter.get_hitCount();

    IndexFolderIterator aIndexFolderIt( *pDatabases, m_aURLParameter.get_module(), m_aURLParameter.get_language() );
    rtl::OUString idxDir;
    bool bExtension = false;
    int iDir = 0;
    vector< vector<HitItem>* > aIndexFolderResultVectorVector;

    bool bTemporary;
    while( !(idxDir = aIndexFolderIt.nextIndexFolder( bExtension, bTemporary )).isEmpty() )
    {
        vector<HitItem> aIndexFolderResultVector;

        try
        {
            vector< vector<HitItem>* > aQueryListResultVectorVector;
            set< rtl::OUString > aSet,aCurrent,aResultSet;

            int nQueryListSize = queryList.size();
            if( nQueryListSize > 1 )
                hitCount = 2000;

            for( int i = 0; i < nQueryListSize; ++i )
            {
                vector<HitItem>* pQueryResultVector;
                if( nQueryListSize > 1 )
                {
                    pQueryResultVector = new vector<HitItem>();
                    aQueryListResultVectorVector.push_back( pQueryResultVector );
                }
                else
                {
                    pQueryResultVector = &aIndexFolderResultVector;
                }
                pQueryResultVector->reserve( hitCount );

                rtl::OUString aLang = m_aURLParameter.get_language();
                const std::vector< rtl::OUString >& aListItem = queryList[i];
                ::rtl::OUString aNewQueryStr = aListItem[0];

                vector<float> aScoreVector;
                vector<rtl::OUString> aPathVector;

                try
                {
                    HelpSearch searcher(aLang, idxDir);
                    searcher.query(aNewQueryStr, bCaptionsOnly, aPathVector, aScoreVector);
                }
                catch (CLuceneError &e)
                {
                    SAL_WARN("xmlhelp", "CLuceneError: " << e.what());
                }

                if( nQueryListSize > 1 )
                    aSet.clear();

                for (unsigned j = 0; j < aPathVector.size(); ++j) {
                    pQueryResultVector->push_back(HitItem(aPathVector[j], aScoreVector[j]));
                    if (nQueryListSize > 1)
                        aSet.insert(aPathVector[j]);
                }

                // intersect
                if( nQueryListSize > 1 )
                {
                    if( i == 0 )
                    {
                        aResultSet = aSet;
                    }
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

            // Combine results in aIndexFolderResultVector
            if( nQueryListSize > 1 )
            {
                for( int n = 0 ; n < nQueryListSize ; ++n )
                {
                    vector<HitItem>* pQueryResultVector = aQueryListResultVectorVector[n];
                    vector<HitItem>& rQueryResultVector = *pQueryResultVector;

                    int nItemCount = rQueryResultVector.size();
                    for( int i = 0 ; i < nItemCount ; ++i )
                    {
                        const HitItem& rItem = rQueryResultVector[ i ];
                        if( (aResultSet.find( rItem.m_aURL )) != aResultSet.end() )
                        {
                            HitItem aItemCopy( rItem );
                            aItemCopy.m_fScore /= nQueryListSize;   // To get average score
                            if( n == 0 )
                            {
                                // Use first pass to create entry
                                aIndexFolderResultVector.push_back( aItemCopy );
                            }
                            else
                            {
                                // Find entry in vector
                                int nCount = aIndexFolderResultVector.size();
                                for( int j = 0 ; j < nCount ; ++j )
                                {
                                    HitItem& rFindItem = aIndexFolderResultVector[ j ];
                                    if( rFindItem.m_aURL.equals( aItemCopy.m_aURL ) )
                                    {
                                        rFindItem.m_fScore += aItemCopy.m_fScore;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    delete pQueryResultVector;
                }

                sort( aIndexFolderResultVector.begin(), aIndexFolderResultVector.end() );
            }

            vector<HitItem>* pIndexFolderHitItemVector = new vector<HitItem>( aIndexFolderResultVector );
            aIndexFolderResultVectorVector.push_back( pIndexFolderHitItemVector );
            aIndexFolderResultVector.clear();
        }
        catch (const Exception &e)
        {
            SAL_WARN("xmlhelp", "Exception: " << e.Message);
        }

        ++iDir;

        if( bTemporary )
            aIndexFolderIt.deleteTempIndexFolder( idxDir );

    }   // Iterator


    int nVectorCount = aIndexFolderResultVectorVector.size();
    vector<HitItem>::size_type* pCurrentVectorIndex = new vector<HitItem>::size_type[nVectorCount];
    for( int j = 0 ; j < nVectorCount ; ++j )
        pCurrentVectorIndex[j] = 0;

    sal_Int32 nTotalHitCount = m_aURLParameter.get_hitCount();
    sal_Int32 nHitCount = 0;
    while( nHitCount < nTotalHitCount )
    {
        int iVectorWithBestScore = -1;
        float fBestScore = 0.0;
        for( int k = 0 ; k < nVectorCount ; ++k )
        {
            vector<HitItem>& rIndexFolderVector = *aIndexFolderResultVectorVector[k];
            if( pCurrentVectorIndex[k] < rIndexFolderVector.size() )
            {
                const HitItem& rItem = rIndexFolderVector[ pCurrentVectorIndex[k] ];

                if( fBestScore < rItem.m_fScore )
                {
                    fBestScore = rItem.m_fScore;
                    iVectorWithBestScore = k;
                }
            }
        }

        if( iVectorWithBestScore == -1 )    // No item left at all
            break;

        vector<HitItem>& rIndexFolderVector = *aIndexFolderResultVectorVector[iVectorWithBestScore];
        const HitItem& rItem = rIndexFolderVector[ pCurrentVectorIndex[iVectorWithBestScore] ];

        pCurrentVectorIndex[iVectorWithBestScore]++;

        aCompleteResultVector.push_back( rItem.m_aURL );
        ++nHitCount;
    }

    delete[] pCurrentVectorIndex;
    for( int n = 0 ; n < nVectorCount ; ++n )
    {
        vector<HitItem>* pIndexFolderVector = aIndexFolderResultVectorVector[n];
        delete pIndexFolderVector;
    }

    sal_Int32 replIdx = rtl::OUString( "#HLP#" ).getLength();
    rtl::OUString replWith = rtl::OUString( "vnd.sun.star.help://" );

    int nResultCount = aCompleteResultVector.size();
    for( int r = 0 ; r < nResultCount ; ++r )
    {
        rtl::OUString aURL = aCompleteResultVector[r];
        rtl::OUString aResultStr = replWith + aURL.copy(replIdx);
          m_aPath.push_back( aResultStr );
    }

    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    Command aCommand;
    aCommand.Name = rtl::OUString( "getPropertyValues" );
    aCommand.Argument <<= m_sProperty;

    for( m_nRow = 0; sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aPath.size(); ++m_nRow )
    {
        m_aPath[m_nRow] =
            m_aPath[m_nRow]                         +
            rtl::OUString( "?Language=" )           +
            m_aURLParameter.get_language()          +
            rtl::OUString( "&System=" )             +
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
