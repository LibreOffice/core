/*************************************************************************
 *
 *  $RCSfile: colmgr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:37 $
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
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"

#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif

#include "frmmgr.hxx"
#include "frmfmt.hxx"
#include "colmgr.hxx"


// PRIVATE METHODES ------------------------------------------------------
/*------------------------------------------------------------------------
 Beschreibung:  Spaltenbreite auf aktuelle Breite einstellen
------------------------------------------------------------------------*/



void FitToActualSize(SwFmtCol& rCol, USHORT nWidth)
{
    const USHORT nCount = rCol.GetColumns().Count();
    for(USHORT i = 0; i < nCount; ++i)
    {
        const USHORT nTmp = rCol.CalcColWidth(i, nWidth);
        rCol.GetColumns()[i]->SetWishWidth(nTmp);
    }
    rCol.SetWishWidth(nWidth);
}


// PUBLIC METHODES -------------------------------------------------------
/*------------------------------------------------------------------------
 Beschreibung:  Setzen Spaltenanzahl und Gutterwidth
------------------------------------------------------------------------*/



void SwColMgr::SetCount(USHORT nCount, USHORT  nGutterWidth)
{
    aFmtCol.Init(nCount, nGutterWidth, nWidth);
    aFmtCol.SetWishWidth(nWidth);
    aFmtCol.SetGutterWidth(nGutterWidth, nWidth);
}



USHORT SwColMgr::GetGutterWidth( USHORT nPos ) const
{
    USHORT nRet;
    if(nPos == USHRT_MAX )
        nRet = GetCount() > 1 ? aFmtCol.GetGutterWidth() : DEF_GUTTER_WIDTH;
    else
    {
        DBG_ASSERT(nPos < GetCount() - 1, "Spalte ueberindiziert" )
        const SwColumns& rCols = aFmtCol.GetColumns();
        nRet = rCols.GetObject(nPos)->GetRight() + rCols.GetObject(nPos + 1)->GetLeft();
    }
    return nRet;
}

/*-----------------22.10.96 14.28-------------------

--------------------------------------------------*/


void SwColMgr::SetGutterWidth(USHORT nGutterWidth, USHORT nPos )
{
    if(nPos == USHRT_MAX)
        aFmtCol.SetGutterWidth(nGutterWidth, nWidth);
    else
    {
        DBG_ASSERT(nPos < GetCount() - 1, "Spalte ueberindiziert" )
        SwColumns& rCols = aFmtCol.GetColumns();
        USHORT nGutterWidth2 = nGutterWidth / 2;
        rCols.GetObject(nPos)->SetRight(nGutterWidth2);
        rCols.GetObject(nPos + 1)->SetLeft(nGutterWidth2);
    }
}

/*------------------------------------------------------------------------
 Beschreibung:  Hoehe Trennlinie
------------------------------------------------------------------------*/



short SwColMgr::GetLineHeightPercent() const
{
    return (short)aFmtCol.GetLineHeight();
}



void SwColMgr::SetLineHeightPercent(short nPercent)
{
    ASSERT(nPercent <= 100, LineHeight darf nur bis 100 % gross  sein);
    aFmtCol.SetLineHeight((BYTE)nPercent);
}
/*------------------------------------------------------------------------
 Beschreibung:  Spaltenbreite
------------------------------------------------------------------------*/



USHORT SwColMgr::GetColWidth(USHORT nIdx) const
{
    ASSERT(nIdx < GetCount(), Spaltenarray ueberindiziert.);
    return aFmtCol.CalcPrtColWidth(nIdx, nWidth);
}



void SwColMgr::SetColWidth(USHORT nIdx, USHORT nWd)
{
    ASSERT(nIdx < GetCount(), Spaltenarray ueberindiziert.);
    aFmtCol.GetColumns()[nIdx]->SetWishWidth(nWd);

}

/*--------------------------------------------------------------------
    Beschreibung:   Groesse neu setzen
 --------------------------------------------------------------------*/



void SwColMgr::SetActualWidth(USHORT nW)
{
    nWidth = nW;
    ::FitToActualSize(aFmtCol, nW);
}

/*--------------------------------------------------------------------
    Beschreibung: ctor
 --------------------------------------------------------------------*/



SwColMgr::SwColMgr(const SfxItemSet& rSet, USHORT nActWidth) :
    aFmtCol((const SwFmtCol&)rSet.Get(RES_COL)),
    nWidth(nActWidth)
{
    if(nWidth == USHRT_MAX)
    {
        nWidth = (USHORT)((const SwFmtFrmSize&)rSet.Get(RES_FRM_SIZE)).GetWidth();
        if (nWidth < MINLAY)
            nWidth = USHRT_MAX;
        const SvxLRSpaceItem &rLR = (const SvxLRSpaceItem&)rSet.Get(RES_LR_SPACE);
        nWidth -= (USHORT)rLR.GetLeft();
        nWidth -= (USHORT)rLR.GetRight();
    }
    ::FitToActualSize(aFmtCol, nWidth);
}




SwColMgr::~SwColMgr() {}




/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.43  2000/09/18 16:05:32  willem.vandorp
    OpenOffice header added.

    Revision 1.42  1997/11/24 16:40:12  MA
    include


      Rev 1.41   24 Nov 1997 17:40:12   MA
   include

      Rev 1.40   03 Nov 1997 13:19:46   MA
   precomp entfernt

      Rev 1.39   12 Aug 1997 15:58:08   OS
   frmitems/textitem/paraitem aufgeteilt

      Rev 1.38   07 Aug 1997 14:59:18   OM
   Headerfile-Umstellung

      Rev 1.37   24 Oct 1996 17:59:50   OS
   unterschiedliche Spaltenabstaende einstellen

      Rev 1.36   28 Aug 1996 11:52:52   OS
   includes

      Rev 1.35   19 Apr 1996 16:59:00   MA
   chg: Vorbereitung fuer kleine Rahmen

      Rev 1.34   18 Apr 1996 11:47:06   OM
   Unbenutzte Fkt entfernt

      Rev 1.33   24 Nov 1995 16:58:10   OM
   PCH->PRECOMPILED

      Rev 1.32   13 Jul 1995 22:45:20   JP
   Spaltenbreiten richtig berechnen

      Rev 1.31   21 May 1995 18:55:28   OS
   Trennlinienhoehe gerichtet

      Rev 1.30   03 May 1995 14:09:24   OM
   DEF_GUTTER_WIDTH fuer Default-Spaltenabstand

      Rev 1.29   03 Apr 1995 17:13:08   OM
   Bug in Spaltenpage gefixt

      Rev 1.28   28 Mar 1995 15:38:56   OS
   Pre-Fix fuer leere Breite

      Rev 1.27   13 Mar 1995 16:19:44   MS
   neu: Spalten im Bsp

      Rev 1.26   08 Mar 1995 12:54:28   MS
   Aktuelle Breite einstellen

------------------------------------------------------------------------*/



