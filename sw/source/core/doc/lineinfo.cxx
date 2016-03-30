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

#include "doc.hxx"
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentState.hxx>
#include "lineinfo.hxx"
#include "charfmt.hxx"
#include "poolfmt.hxx"
#include "rootfrm.hxx"
#include "viewsh.hxx"
#include <set>

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
    nPosFromLeft( MM50 ),
    nCountBy( 5 ),
    nDividerCountBy( 3 ),
    ePos( LINENUMBER_POS_LEFT ),
    bPaintLineNumbers( false ),
    bCountBlankLines( true ),
    bCountInFlys( false ),
    bRestartEachPage( false )
{
}

SwLineNumberInfo::SwLineNumberInfo(const SwLineNumberInfo &rCpy ) : SwClient(),
    aType( rCpy.GetNumType() ),
    aDivider( rCpy.GetDivider() ),
    nPosFromLeft( rCpy.GetPosFromLeft() ),
    nCountBy( rCpy.GetCountBy() ),
    nDividerCountBy( rCpy.GetDividerCountBy() ),
    ePos( rCpy.GetPos() ),
    bPaintLineNumbers( rCpy.IsPaintLineNumbers() ),
    bCountBlankLines( rCpy.IsCountBlankLines() ),
    bCountInFlys( rCpy.IsCountInFlys() ),
    bRestartEachPage( rCpy.IsRestartEachPage() )
{
    if ( rCpy.GetRegisteredIn() )
        const_cast<SwModify*>(rCpy.GetRegisteredIn())->Add( this );
}

SwLineNumberInfo& SwLineNumberInfo::operator=(const SwLineNumberInfo &rCpy)
{
    if ( rCpy.GetRegisteredIn() )
        const_cast<SwModify*>(rCpy.GetRegisteredIn())->Add( this );
    else if ( GetRegisteredIn() )
        GetRegisteredInNonConst()->Remove( this );

    aType = rCpy.GetNumType();
    aDivider = rCpy.GetDivider();
    nPosFromLeft = rCpy.GetPosFromLeft();
    nCountBy = rCpy.GetCountBy();
    nDividerCountBy = rCpy.GetDividerCountBy();
    ePos = rCpy.GetPos();
    bPaintLineNumbers = rCpy.IsPaintLineNumbers();
    bCountBlankLines = rCpy.IsCountBlankLines();
    bCountInFlys = rCpy.IsCountInFlys();
    bRestartEachPage = rCpy.IsRestartEachPage();

    return *this;
}

bool SwLineNumberInfo::operator==( const SwLineNumberInfo& rInf ) const
{
    return  GetRegisteredIn() == rInf.GetRegisteredIn() &&
            aType.GetNumberingType() == rInf.GetNumType().GetNumberingType() &&
            aDivider == rInf.GetDivider() &&
            nPosFromLeft == rInf.GetPosFromLeft() &&
            nCountBy == rInf.GetCountBy() &&
            nDividerCountBy == rInf.GetDividerCountBy() &&
            ePos == rInf.GetPos() &&
            bPaintLineNumbers == rInf.IsPaintLineNumbers() &&
            bCountBlankLines == rInf.IsCountBlankLines() &&
            bCountInFlys == rInf.IsCountInFlys() &&
            bRestartEachPage == rInf.IsRestartEachPage();
}

SwCharFormat* SwLineNumberInfo::GetCharFormat( IDocumentStylePoolAccess& rIDSPA ) const
{
    if ( !GetRegisteredIn() )
    {
        SwCharFormat* pFormat = rIDSPA.GetCharFormatFromPool( RES_POOLCHR_LINENUM );
        pFormat->Add( const_cast<SwClient*>(static_cast<SwClient const *>(this)) );
    }
    return const_cast<SwCharFormat*>(static_cast<const SwCharFormat*>(GetRegisteredIn()));
}

void SwLineNumberInfo::SetCharFormat( SwCharFormat *pChFormat )
{
    OSL_ENSURE( pChFormat, "SetCharFormat, 0 is not a valid pointer" );
    pChFormat->Add( this );
}

void SwLineNumberInfo::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    CheckRegistration( pOld, pNew );
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
