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
#include "lineinfo.hxx"
#include "charfmt.hxx"
#include "poolfmt.hxx"
#include "rootfrm.hxx"
#include "viewsh.hxx"
#include <set>

void SwDoc::SetLineNumberInfo( const SwLineNumberInfo &rNew )
{
    SwRootFrm* pTmpRoot = GetCurrentLayout();//swmod 080219
    if (  pTmpRoot &&
         (rNew.IsCountBlankLines() != mpLineNumberInfo->IsCountBlankLines() ||
          rNew.IsRestartEachPage() != mpLineNumberInfo->IsRestartEachPage()) )
    {
        std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();//swmod 080225
        pTmpRoot->StartAllAction();
        // FME 2007-08-14 #i80120# Invalidate size, because ChgThisLines()
        // is only (onny may only be) called by the formatting routines
        //pTmpRoot->InvalidateAllCntnt( INV_LINENUM | INV_SIZE );
        std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::InvalidateAllCntnt), INV_LINENUM | INV_SIZE));//swmod 080226
         pTmpRoot->EndAllAction();
    }   //swmod 080219
    *mpLineNumberInfo = rNew;
    SetModified();
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
    bPaintLineNumbers( sal_False ),
    bCountBlankLines( sal_True ),
    bCountInFlys( sal_False ),
    bRestartEachPage( sal_False )
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
        ((SwModify*)rCpy.GetRegisteredIn())->Add( this );
}

SwLineNumberInfo& SwLineNumberInfo::operator=(const SwLineNumberInfo &rCpy)
{
    if ( rCpy.GetRegisteredIn() )
        ((SwModify*)rCpy.GetRegisteredIn())->Add( this );
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

SwCharFmt* SwLineNumberInfo::GetCharFmt( IDocumentStylePoolAccess& rIDSPA ) const
{
    if ( !GetRegisteredIn() )
    {
        SwCharFmt* pFmt = rIDSPA.GetCharFmtFromPool( RES_POOLCHR_LINENUM );
        pFmt->Add( (SwClient*)this );
    }
    return (SwCharFmt*)GetRegisteredIn();
}

void SwLineNumberInfo::SetCharFmt( SwCharFmt *pChFmt )
{
    OSL_ENSURE( pChFmt, "SetCharFmt, 0 is not a valid pointer" );
    pChFmt->Add( this );
}

void SwLineNumberInfo::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    CheckRegistration( pOld, pNew );
    SwDoc *pDoc = ((SwCharFmt*)GetRegisteredIn())->GetDoc();
    SwRootFrm* pRoot = pDoc->GetCurrentLayout();
    if( pRoot )
    {
        pRoot->StartAllAction();
        std::set<SwRootFrm*> aAllLayouts = pDoc->GetAllLayouts();
        std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllAddPaintRect));//swmod 080305
        //pRoot->GetCurrShell()->AddPaintRect( pRoot->Frm() );
        pRoot->EndAllAction();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
