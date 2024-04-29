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

#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentState.hxx>
#include <lineinfo.hxx>
#include <charfmt.hxx>
#include <poolfmt.hxx>
#include <rootfrm.hxx>
#include <osl/diagnose.h>

void SwDoc::SetLineNumberInfo( const SwLineNumberInfo &rNew )
{
    SwRootFrame* pTmpRoot = getIDocumentLayoutAccess().GetCurrentLayout();
    if (  pTmpRoot &&
         (rNew.IsCountBlankLines() != mpLineNumberInfo->IsCountBlankLines() ||
          rNew.IsRestartEachPage() != mpLineNumberInfo->IsRestartEachPage()) )
    {
        pTmpRoot->StartAllAction();
        // FME 2007-08-14 #i80120# Invalidate size, because ChgThisLines()
        // is only (and may only be) called by the formatting routines
        //pTmpRoot->InvalidateAllContent( SwInvalidateFlags::LineNum | SwInvalidateFlags::Size );
        for( auto aLayout : GetAllLayouts() )
            aLayout->InvalidateAllContent( SwInvalidateFlags::LineNum | SwInvalidateFlags::Size );
        pTmpRoot->EndAllAction();
    }
    *mpLineNumberInfo = rNew;
    getIDocumentState().SetModified();
}

const SwLineNumberInfo& SwDoc::GetLineNumberInfo() const
{
    return *mpLineNumberInfo;
}

SwLineNumberInfo::SwLineNumberInfo() :
    m_nPosFromLeft(o3tl::toTwips(5, o3tl::Length::mm)),
    m_nCountBy( 5 ),
    m_nDividerCountBy( 3 ),
    m_ePos( LINENUMBER_POS_LEFT ),
    m_bPaintLineNumbers( false ),
    m_bCountBlankLines( true ),
    m_bCountInFlys( false ),
    m_bRestartEachPage( false )
{
}

SwLineNumberInfo::SwLineNumberInfo(const SwLineNumberInfo &rCpy ) : SwClient(),
    m_aType( rCpy.GetNumType() ),
    m_aDivider( rCpy.GetDivider() ),
    m_nPosFromLeft( rCpy.GetPosFromLeft() ),
    m_nCountBy( rCpy.GetCountBy() ),
    m_nDividerCountBy( rCpy.GetDividerCountBy() ),
    m_ePos( rCpy.GetPos() ),
    m_bPaintLineNumbers( rCpy.IsPaintLineNumbers() ),
    m_bCountBlankLines( rCpy.IsCountBlankLines() ),
    m_bCountInFlys( rCpy.IsCountInFlys() ),
    m_bRestartEachPage( rCpy.IsRestartEachPage() )
{
    StartListeningToSameModifyAs(rCpy);
}

SwLineNumberInfo& SwLineNumberInfo::operator=(const SwLineNumberInfo &rCpy)
{
    StartListeningToSameModifyAs(rCpy);

    m_aType = rCpy.GetNumType();
    m_aDivider = rCpy.GetDivider();
    m_nPosFromLeft = rCpy.GetPosFromLeft();
    m_nCountBy = rCpy.GetCountBy();
    m_nDividerCountBy = rCpy.GetDividerCountBy();
    m_ePos = rCpy.GetPos();
    m_bPaintLineNumbers = rCpy.IsPaintLineNumbers();
    m_bCountBlankLines = rCpy.IsCountBlankLines();
    m_bCountInFlys = rCpy.IsCountInFlys();
    m_bRestartEachPage = rCpy.IsRestartEachPage();

    return *this;
}

SwCharFormat* SwLineNumberInfo::GetCharFormat( IDocumentStylePoolAccess& rIDSPA ) const
{
    if ( !GetRegisteredIn() )
    {
        SwCharFormat* pFormat = rIDSPA.GetCharFormatFromPool( RES_POOLCHR_LINENUM );
        pFormat->Add(const_cast<SwLineNumberInfo&>(*this));
    }
    return const_cast<SwCharFormat*>(static_cast<const SwCharFormat*>(GetRegisteredIn()));
}

void SwLineNumberInfo::SetCharFormat( SwCharFormat *pChFormat )
{
    OSL_ENSURE( pChFormat, "SetCharFormat, 0 is not a valid pointer" );
    pChFormat->Add(*this);
}

void SwLineNumberInfo::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    if (rHint.GetId() != SfxHintId::SwLegacyModify)
        return;
    auto pLegacy = static_cast<const sw::LegacyModifyHint*>(&rHint);
    CheckRegistration( pLegacy->m_pOld );
    SwDoc *pDoc = static_cast<SwCharFormat*>(GetRegisteredIn())->GetDoc();
    SwRootFrame* pRoot = pDoc->getIDocumentLayoutAccess().GetCurrentLayout();
    if( pRoot )
    {
        pRoot->StartAllAction();
        for( auto aLayout : pDoc->GetAllLayouts() )
            aLayout->AllAddPaintRect();
        pRoot->EndAllAction();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
