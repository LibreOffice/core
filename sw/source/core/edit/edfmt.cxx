/*************************************************************************
 *
 *  $RCSfile: edfmt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:18 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "doc.hxx"
#include "editsh.hxx"
#include "swtable.hxx"
#include "pam.hxx"
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif

#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#include "ndtxt.hxx"    // Fuer GetXXXFmt
#include "hints.hxx"


/*************************************
 * Formate
 *************************************/
// Char
// OPT: inline


USHORT SwEditShell::GetCharFmtCount() const
{
    return GetDoc()->GetCharFmts()->Count();
}


SwCharFmt& SwEditShell::GetCharFmt(USHORT nFmt) const
{
    return *((*(GetDoc()->GetCharFmts()))[nFmt]);
}


SwCharFmt* SwEditShell::GetCurCharFmt() const
{
    SwCharFmt *pFmt = 0;
    SfxItemSet aSet( GetDoc()->GetAttrPool(), RES_TXTATR_CHARFMT,
                                                RES_TXTATR_CHARFMT );
    const SfxPoolItem* pItem;
    if( GetAttr( aSet ) && SFX_ITEM_SET ==
        aSet.GetItemState( RES_TXTATR_CHARFMT, FALSE, &pItem ) )
        pFmt = ((SwFmtCharFmt*)pItem)->GetCharFmt();

    return pFmt;
}


void SwEditShell::FillByEx(SwCharFmt* pCharFmt, BOOL bReset)
{
    if ( bReset )
        pCharFmt->ResetAllAttr();

    SwPaM* pPam = GetCrsr();
    const SwCntntNode* pCNd = pPam->GetCntntNode();
    if( pCNd->IsTxtNode() )
    {
        xub_StrLen nStt, nEnd;
        if( pPam->HasMark() )
        {
            const SwPosition* pPtPos = pPam->GetPoint();
            const SwPosition* pMkPos = pPam->GetMark();
            if( pPtPos->nNode == pMkPos->nNode )        // im selben Node ?
            {
                nStt = pPtPos->nContent.GetIndex();
                if( nStt < pMkPos->nContent.GetIndex() )
                    nEnd = pMkPos->nContent.GetIndex();
                else
                {
                    nEnd = nStt;
                    nStt = pMkPos->nContent.GetIndex();
                }
            }
            else
            {
                nStt = pMkPos->nContent.GetIndex();
                if( pPtPos->nNode < pMkPos->nNode )
                {
                    nEnd = nStt;
                    nStt = 0;
                }
                else
                    nEnd = ((SwTxtNode*)pCNd)->GetTxt().Len();
            }
        }
        else
            nStt = nEnd = pPam->GetPoint()->nContent.GetIndex();

        SfxItemSet aSet( pDoc->GetAttrPool(),
                            pCharFmt->GetAttrSet().GetRanges() );
        ((SwTxtNode*)pCNd)->GetAttr( aSet, nStt, nEnd );
        pCharFmt->SetAttr( aSet );
    }
    else if( pCNd->GetpSwAttrSet() )
        pCharFmt->SetAttr( *pCNd->GetpSwAttrSet() );
}

// Frm
USHORT SwEditShell::GetTblFrmFmtCount(BOOL bUsed) const
{
    return GetDoc()->GetTblFrmFmtCount(bUsed);
}

SwFrmFmt& SwEditShell::GetTblFrmFmt(USHORT nFmt, BOOL bUsed ) const
{
    return GetDoc()->GetTblFrmFmt(nFmt, bUsed );
}

String SwEditShell::GetUniqueTblName() const
{
    return GetDoc()->GetUniqueTblName();
}


SwCharFmt* SwEditShell::MakeCharFmt( const String& rName,
                                    SwCharFmt* pDerivedFrom )
{
    if( !pDerivedFrom )
        pDerivedFrom = GetDoc()->GetDfltCharFmt();

    return GetDoc()->MakeCharFmt( rName, pDerivedFrom );
}

//----------------------------------
// inlines im Product


SwTxtFmtColl* SwEditShell::GetTxtCollFromPool( USHORT nId )
{
    return GetDoc()->GetTxtCollFromPool( nId );
}


    // return das geforderte automatische  Format - Basis-Klasse !
SwFmt* SwEditShell::GetFmtFromPool( USHORT nId )
{
    return GetDoc()->GetFmtFromPool( nId );
}


SwPageDesc* SwEditShell::GetPageDescFromPool( USHORT nId )
{
    return GetDoc()->GetPageDescFromPool( nId );
}


BOOL SwEditShell::IsUsed( const SwModify& rModify ) const
{
    return pDoc->IsUsed( rModify );
}


USHORT SwEditShell::GetPoolId( const String& rName,
                                SwGetPoolIdFromName eFlags ) const
{
    return pDoc->GetPoolId( rName, eFlags );
}


const SvStringsDtor& SwEditShell::GetChrFmtNmArray() const
{
    return pDoc->GetChrFmtNmArray();
}


const SvStringsDtor& SwEditShell::GetHTMLChrFmtNmArray() const
{
    return pDoc->GetHTMLChrFmtNmArray();
}


const SwFlyFrmFmt* SwEditShell::FindFlyByName( const String& rName, BYTE nNdTyp ) const
{
    return pDoc->FindFlyByName(rName, nNdTyp);
}


SwCharFmt* SwEditShell::FindCharFmtByName( const String& rName ) const
{
    return pDoc->FindCharFmtByName( rName );
}

SwFrmFmt* SwEditShell::FindFrmFmtByName( const String& rName ) const
{
    return pDoc->FindFrmFmtByName( rName );
}

SwTxtFmtColl* SwEditShell::FindTxtFmtCollByName( const String& rName ) const
{
    return pDoc->FindTxtFmtCollByName( rName );
}

#ifdef USED

SwGrfFmtColl* SwEditShell::FindGrfFmtCollByName( const String& rName ) const
{
    return pDoc->FindGrfFmtCollByName( rName );
}
#endif





