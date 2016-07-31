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
#include <DocumentStatisticsManager.hxx>
#include <doc.hxx>
#include <editsh.hxx>
#include <fldbas.hxx>
#include <docsh.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <view.hxx>
#include <ndtxt.hxx>
#include <calbck.hxx>
#include <fmtfld.hxx>
#include <rootfrm.hxx>
#include <docufld.hxx>
#include <docstat.hxx>
#include <vector>
#include <viewsh.hxx>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <wrtsh.hxx>
#include <viewopt.hxx>

using namespace ::com::sun::star;

namespace
{
    class LockAllViews
    {
        std::vector<SwViewShell*> m_aViewWasUnLocked;
        SwViewShell* m_pViewShell;
    public:
        explicit LockAllViews(SwViewShell *pViewShell)
            : m_pViewShell(pViewShell)
        {
            if (!m_pViewShell)
                return;
            for (SwViewShell& rShell : m_pViewShell->GetRingContainer())
            {
                if (!rShell.IsViewLocked())
                {
                    m_aViewWasUnLocked.push_back(&rShell);
                    rShell.LockView(true);
                }
            }
        }
        ~LockAllViews()
        {
            for(SwViewShell* pShell : m_aViewWasUnLocked)
                pShell->LockView(false);
        }
    };
}

namespace sw
{

DocumentStatisticsManager::DocumentStatisticsManager( SwDoc& i_rSwdoc ) : m_rDoc( i_rSwdoc ),
                                                                          mpDocStat( new SwDocStat ),
                                                                          mbInitialized( false )
{
    maStatsUpdateTimer.SetTimeoutHdl( LINK( this, DocumentStatisticsManager, DoIdleStatsUpdate ) );
    maStatsUpdateTimer.SetPriority( SchedulerPriority::DEFAULT_IDLE );
    maStatsUpdateTimer.SetDebugName( "sw::DocumentStatisticsManager maStatsUpdateTimer" );
}

void DocumentStatisticsManager::DocInfoChgd(bool const isEnableSetModified)
{
    m_rDoc.getIDocumentFieldsAccess().GetSysFieldType( RES_DOCINFOFLD )->UpdateFields();
    m_rDoc.getIDocumentFieldsAccess().GetSysFieldType( RES_TEMPLNAMEFLD )->UpdateFields();
    if (isEnableSetModified)
    {
        m_rDoc.getIDocumentState().SetModified();
    }
}

const SwDocStat& DocumentStatisticsManager::GetDocStat() const
{
    return *mpDocStat;
}

void DocumentStatisticsManager::SetDocStatModified(bool bSet)
{
    mpDocStat->bModified = bSet;
}

const SwDocStat& DocumentStatisticsManager::GetUpdatedDocStat( bool bCompleteAsync, bool bFields )
{
    if( mpDocStat->bModified || !mbInitialized)
    {
        UpdateDocStat( bCompleteAsync, bFields );
    }
    return *mpDocStat;
}

void DocumentStatisticsManager::SetDocStat( const SwDocStat& rStat )
{
    *mpDocStat = rStat;
    mbInitialized = true;
}

void DocumentStatisticsManager::UpdateDocStat( bool bCompleteAsync, bool bFields )
{
    if( mpDocStat->bModified || !mbInitialized)
    {
        if (!bCompleteAsync)
        {
            maStatsUpdateTimer.Stop();
            while (IncrementalDocStatCalculate(
                        std::numeric_limits<long>::max(), bFields)) {}
        }
        else
        {
            if (!maStatsUpdateTimer.IsActive() && IncrementalDocStatCalculate(5000, bFields))
                maStatsUpdateTimer.Start();
        }
    }
}

// returns true while there is more to do
bool DocumentStatisticsManager::IncrementalDocStatCalculate(long nChars, bool bFields)
{
    mbInitialized = true;
    mpDocStat->Reset();
    mpDocStat->nPara = 0; // default is 1!

    // This is the inner loop - at least while the paras are dirty.
    for( sal_uLong i = m_rDoc.GetNodes().Count(); i > 0 && nChars > 0; )
    {
        SwNode* pNd;
        switch( ( pNd = m_rDoc.GetNodes()[ --i ])->GetNodeType() )
        {
        case ND_TEXTNODE:
        {
            long const nOldChars(mpDocStat->nChar);
            SwTextNode *pText = static_cast< SwTextNode * >( pNd );
            if (pText->CountWords(*mpDocStat, 0, pText->GetText().getLength()))
            {
                nChars -= (mpDocStat->nChar - nOldChars);
            }
            break;
        }
        case ND_TABLENODE:      ++mpDocStat->nTable;   break;
        case ND_GRFNODE:        ++mpDocStat->nGrf;   break;
        case ND_OLENODE:        ++mpDocStat->nOLE;   break;
        case ND_SECTIONNODE:    break;
        }
    }

    // #i93174#: notes contain paragraphs that are not nodes
    {
        SwFieldType * const pPostits( m_rDoc.getIDocumentFieldsAccess().GetSysFieldType(RES_POSTITFLD) );
        SwIterator<SwFormatField,SwFieldType> aIter( *pPostits );
        for( SwFormatField* pFormatField = aIter.First(); pFormatField;  pFormatField = aIter.Next() )
        {
            if (pFormatField->IsFieldInDoc())
            {
                SwPostItField const * const pField(
                    static_cast<SwPostItField const*>(pFormatField->GetField()));
                mpDocStat->nAllPara += pField->GetNumberOfParagraphs();
            }
        }
    }

    mpDocStat->nPage     = m_rDoc.getIDocumentLayoutAccess().GetCurrentLayout() ? m_rDoc.getIDocumentLayoutAccess().GetCurrentLayout()->GetPageNum() : 0;
    mpDocStat->bModified = false;

    css::uno::Sequence < css::beans::NamedValue > aStat( mpDocStat->nPage ? 8 : 7);
    sal_Int32 n=0;
    aStat[n].Name = "TableCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nTable;
    aStat[n].Name = "ImageCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nGrf;
    aStat[n].Name = "ObjectCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nOLE;
    if ( mpDocStat->nPage )
    {
        aStat[n].Name = "PageCount";
        aStat[n++].Value <<= (sal_Int32)mpDocStat->nPage;
    }
    aStat[n].Name = "ParagraphCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nPara;
    aStat[n].Name = "WordCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nWord;
    aStat[n].Name = "CharacterCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nChar;
    aStat[n].Name = "NonWhitespaceCharacterCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nCharExcludingSpaces;

    // For e.g. autotext documents there is no pSwgInfo (#i79945)
    SwDocShell* pObjShell(m_rDoc.GetDocShell());
    if (pObjShell)
    {
        const uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                pObjShell->GetModel(), uno::UNO_QUERY_THROW);
        const uno::Reference<document::XDocumentProperties> xDocProps(
                xDPS->getDocumentProperties());
        // #i96786#: do not set modified flag when updating statistics
        const bool bDocWasModified( m_rDoc.getIDocumentState().IsModified() );
        const ModifyBlocker_Impl b(pObjShell);
        // rhbz#1081176: don't jump to cursor pos because of (temporary)
        // activation of modified flag triggering move to input position
        LockAllViews aViewGuard(pObjShell->GetEditShell());
        xDocProps->setDocumentStatistics(aStat);
        if (!bDocWasModified)
        {
            m_rDoc.getIDocumentState().ResetModified();
        }
    }

    // optionally update stat. fields
    if (bFields)
    {
        SwFieldType *pType = m_rDoc.getIDocumentFieldsAccess().GetSysFieldType(RES_DOCSTATFLD);
        pType->UpdateFields();
    }

    return nChars < 0;
}

IMPL_LINK_TYPED( DocumentStatisticsManager, DoIdleStatsUpdate, Timer *, pTimer, void )
{
    (void) pTimer;
    SwView* pView = m_rDoc.GetDocShell() ? m_rDoc.GetDocShell()->GetView() : nullptr;
    if( pView )
    {
        SwWrtShell& rWrtShell = pView->GetWrtShell();
        if (!rWrtShell.GetViewOptions()->IsIdle())
        {
            maStatsUpdateTimer.SetTimeout( 1000 );
            return;
        }
    }
    maStatsUpdateTimer.SetTimeout( 0 );

    if (!IncrementalDocStatCalculate(32000))
        maStatsUpdateTimer.Stop();

    if( pView )
        pView->UpdateDocStats();
}

DocumentStatisticsManager::~DocumentStatisticsManager()
{
    maStatsUpdateTimer.Stop();
    delete mpDocStat;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
