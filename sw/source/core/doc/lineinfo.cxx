/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lineinfo.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-07 12:17:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

void SwDoc::SetLineNumberInfo( const SwLineNumberInfo &rNew )
{
    if ( GetRootFrm() &&
         (rNew.IsCountBlankLines() != pLineNumberInfo->IsCountBlankLines() ||
          rNew.IsRestartEachPage() != pLineNumberInfo->IsRestartEachPage()) )
    {
        GetRootFrm()->StartAllAction();
        // FME 2007-08-14 #i80120# Invalidate size, because ChgThisLines()
        // is only (onny may only be) called by the formatting routines
        GetRootFrm()->InvalidateAllCntnt( INV_LINENUM | INV_SIZE );
        GetRootFrm()->EndAllAction();
    }
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
    bPaintLineNumbers( FALSE ),
    bCountBlankLines( TRUE ),
    bCountInFlys( FALSE ),
    bRestartEachPage( FALSE )
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
        pRegisteredIn->Remove( this );

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

BOOL SwLineNumberInfo::operator==( const SwLineNumberInfo& rInf ) const
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

void SwLineNumberInfo::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    SwClient::Modify( pOld, pNew );
    SwDoc *pDoc = ((SwCharFmt*)GetRegisteredIn())->GetDoc();
    SwRootFrm* pRoot = pDoc->GetRootFrm();
    if( pRoot && pRoot->GetCurrShell() )
    {
        pRoot->StartAllAction();
        pRoot->GetCurrShell()->AddPaintRect( pRoot->Frm() );
        pRoot->EndAllAction();
    }
}

