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

#include <iterator>

#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/i18n/Transliteration.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include <helpcompiler/HelpSearch.hxx>

#if defined(__GNUC__)
#  pragma GCC visibility push (default)
#endif
#include <CLucene.h>
#if defined(__GNUC__)
#  pragma GCC visibility pop
#endif

#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#include <algorithm>
#include <set>
#include <qe/DocGenerator.hxx>
#include "resultsetforquery.hxx"
#include "databases.hxx"

using namespace std;
using namespace chelp;
using namespace xmlsearch::qe;
using namespace com::sun::star;
using namespace com::sun::star::ucb;
using namespace com::sun::star::i18n;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

struct HitItem
{
    OUString   m_aURL;
    float      m_fScore;

    HitItem(const OUString& aURL, float fScore)
        : m_aURL(aURL)
        , m_fScore(fScore)
    {}
    bool operator < ( const HitItem& rHitItem ) const
    {
        return rHitItem.m_fScore < m_fScore;
    }
};

ResultSetForQuery::ResultSetForQuery( const uno::Reference< uno::XComponentContext >& rxContext,
                                      const uno::Reference< XContentProvider >&  xProvider,
                                      const uno::Sequence< beans::Property >& seq,
                                      const URLParameter& aURLParameter,
                                      Databases* pDatabases )
    : ResultSetBase( rxContext,xProvider,seq ),
      m_aURLParameter( aURLParameter )
{
    Reference< XExtendedTransliteration > xTrans = Transliteration::create( rxContext );
    Locale aLocale( aURLParameter.get_language(),
                    OUString(),
                    OUString() );
    xTrans->loadModule(TransliterationModules_UPPERCASE_LOWERCASE,
                       aLocale );

    vector< vector< OUString > > queryList;
    {
        sal_Int32 idx;
        OUString query = m_aURLParameter.get_query();
        while( !query.isEmpty() )
        {
            idx = query.indexOf( ' ' );
            if( idx == -1 )
                idx = query.getLength();

            vector< OUString > currentQuery;
            OUString tmp(query.copy( 0,idx ));
            Sequence<sal_Int32> aSeq;
            OUString toliterate = xTrans->transliterate(
                tmp,0,tmp.getLength(),aSeq);

            currentQuery.push_back( toliterate );
            queryList.push_back( currentQuery );

            int nCpy = 1 + idx;
            if( nCpy >= query.getLength() )
                query.clear();
            else
                query = query.copy( 1 + idx );
        }
    }

    vector< OUString > aCompleteResultVector;
    OUString scope = m_aURLParameter.get_scope();
    bool bCaptionsOnly = scope == "Heading";
    sal_Int32 hitCount = m_aURLParameter.get_hitCount();

    IndexFolderIterator aIndexFolderIt( *pDatabases, m_aURLParameter.get_module(), m_aURLParameter.get_language() );
    OUString idxDir;
    bool bExtension = false;
    vector< vector<HitItem> > aIndexFolderResultVectorVector;

    bool bTemporary;
    while( !(idxDir = aIndexFolderIt.nextIndexFolder( bExtension, bTemporary )).isEmpty() )
    {
        vector<HitItem> aIndexFolderResultVector;

        try
        {
            vector< vector<HitItem> > aQueryListResultVectorVector;
            set< OUString > aSet,aCurrent,aResultSet;

            int nQueryListSize = queryList.size();
            if( nQueryListSize > 1 )
                hitCount = 2000;

            for( int i = 0; i < nQueryListSize; ++i )
            {
                vector<HitItem>* pQueryResultVector;
                if( nQueryListSize > 1 )
                {
                    aQueryListResultVectorVector.emplace_back();
                    pQueryResultVector = &aQueryListResultVectorVector.back();
                }
                else
                {
                    pQueryResultVector = &aIndexFolderResultVector;
                }
                pQueryResultVector->reserve( hitCount );

                const std::vector< OUString >& aListItem = queryList[i];
                OUString aNewQueryStr = aListItem[0];

                vector<float> aScoreVector;
                vector<OUString> aPathVector;

                try
                {
                    HelpSearch searcher(idxDir);
                    searcher.query(aNewQueryStr, bCaptionsOnly, aPathVector, aScoreVector);
                }
                catch (CLuceneError &e)
                {
                    SAL_WARN("xmlhelp", "CLuceneError: " << e.what());
                }

                if( nQueryListSize > 1 )
                    aSet.clear();

                for (size_t j = 0; j < aPathVector.size(); ++j) {
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
                    vector<HitItem>& rQueryResultVector = aQueryListResultVectorVector[n];

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
                                    if( rFindItem.m_aURL == aItemCopy.m_aURL )
                                    {
                                        rFindItem.m_fScore += aItemCopy.m_fScore;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                sort( aIndexFolderResultVector.begin(), aIndexFolderResultVector.end() );
            }

            aIndexFolderResultVectorVector.push_back( std::move(aIndexFolderResultVector) );
        }
        catch (const Exception &e)
        {
            SAL_WARN("xmlhelp", e);
        }

        if( bTemporary )
            aIndexFolderIt.deleteTempIndexFolder( idxDir );

    }   // Iterator


    int nVectorCount = aIndexFolderResultVectorVector.size();
    std::unique_ptr<std::vector<HitItem>::size_type[]> pCurrentVectorIndex(new vector<HitItem>::size_type[nVectorCount]);
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
            vector<HitItem>& rIndexFolderVector = aIndexFolderResultVectorVector[k];
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

        vector<HitItem>& rIndexFolderVector = aIndexFolderResultVectorVector[iVectorWithBestScore];
        const HitItem& rItem = rIndexFolderVector[ pCurrentVectorIndex[iVectorWithBestScore] ];

        pCurrentVectorIndex[iVectorWithBestScore]++;

        aCompleteResultVector.push_back( rItem.m_aURL );
        ++nHitCount;
    }

    pCurrentVectorIndex.reset();
    aIndexFolderResultVectorVector.clear();

    sal_Int32 replIdx = OUString( "#HLP#" ).getLength();
    OUString replWith = "vnd.sun.star.help://";

    int nResultCount = aCompleteResultVector.size();
    for( int r = 0 ; r < nResultCount ; ++r )
    {
        OUString aURL = aCompleteResultVector[r];
        OUString aResultStr = replWith + aURL.copy(replIdx);
        m_aPath.push_back( aResultStr );
    }

    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    Command aCommand;
    aCommand.Name = "getPropertyValues";
    aCommand.Argument <<= m_sProperty;

    for( m_nRow = 0; sal::static_int_cast<sal_uInt32>( m_nRow ) < m_aPath.size(); ++m_nRow )
    {
        m_aPath[m_nRow] =
            m_aPath[m_nRow] +
            "?Language=" +
            m_aURLParameter.get_language() +
            "&System=" +
            m_aURLParameter.get_system();

        uno::Reference< XContent > content = queryContent();
        if( content.is() )
        {
            uno::Reference< XCommandProcessor > cmd( content,uno::UNO_QUERY );
            cmd->execute( aCommand,0,uno::Reference< XCommandEnvironment >( nullptr ) ) >>= m_aItems[m_nRow]; //TODO: check return value of operator >>=
        }
    }
    m_nRow = 0xffffffff;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
