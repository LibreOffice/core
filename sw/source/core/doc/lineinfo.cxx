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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



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
         (rNew.IsCountBlankLines() != pLineNumberInfo->IsCountBlankLines() ||
          rNew.IsRestartEachPage() != pLineNumberInfo->IsRestartEachPage()) )
    {
        std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();//swmod 080225
        pTmpRoot->StartAllAction();
        // FME 2007-08-14 #i80120# Invalidate size, because ChgThisLines()
        // is only (onny may only be) called by the formatting routines
        //pTmpRoot->InvalidateAllCntnt( INV_LINENUM | INV_SIZE );
        std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::InvalidateAllCntnt), INV_LINENUM | INV_SIZE));//swmod 080226
         pTmpRoot->EndAllAction();
    }   //swmod 080219
    *pLineNumberInfo = rNew;
    SetModified();
}

const SwLineNumberInfo& SwDoc::GetLineNumberInfo() const
{
    return *pLineNumberInfo;
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

sal_Bool SwLineNumberInfo::operator==( const SwLineNumberInfo& rInf ) const
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
    ASSERT( pChFmt, "SetCharFmt, 0 is not a valid pointer" );
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

