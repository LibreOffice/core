/*************************************************************************
 *
 *  $RCSfile: w1sprm.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:58 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
// std
#include <stdio.h> // sprintf

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#include <tools/solar.h>
#ifndef _SVX_PAPERINF_HXX
#include <svx/paperinf.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif


#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _W1PAR_HXX
#include <w1par.hxx>
#endif

#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _STATSTR_HRC
#include <statstr.hrc>
#endif

/////////////////////////////////////////////////////////////// Ww1Sprm
void Ww1Sprm::Stop( Ww1Shell& rOut, Ww1Manager& rMan)
{
    if(IsUsed())
//      for(USHORT i=0;i<Count();i++)
        for(short i=Count()-1;i>=0;i--){    // rueckwaerts
            BYTE nId;
            USHORT nSize;
            BYTE* pSprm;
            Fill(i, nId, nSize, pSprm);
            GetTab(nId).Stop(rOut, nId, pSprm, nSize, rMan);
        }
}

void Ww1Sprm::Start(
    Ww1Shell& rOut, Ww1Manager& rMan, USHORT i)
{
    BYTE nId;
    USHORT nSize;
    BYTE* pSprm;
    Fill(i, nId, nSize, pSprm);
    GetTab(nId).Start(rOut, nId, pSprm, nSize, rMan);
}

void Ww1Sprm::Start(
    Ww1Shell& rOut, Ww1Manager& rMan)
{
    if(IsUsed())
        for(USHORT i=0;i<Count();i++)
            Start(rOut, rMan, i);
}

//////////////////////////////////////////////////////////// SingleSprm
void Ww1SingleSprm::Start(
    Ww1Shell&, BYTE nId, BYTE*, USHORT, Ww1Manager&)
{
#ifdef DEBUG
//  ASSERT(FALSE, "Unknown Sprm");
    if (nId)
    {
        char s[200];
        sprintf(s, "%s(%d) Sprm id %d started.",
         __FILE__, __LINE__,(int)nId);
        DBG_ERROR(s);
    }
#endif
}

void Ww1SingleSprm::Stop(
    Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&)
{
//  ASSERT(FALSE, "Unknown Sprm");
}

////////////////////////////////////////////////////////////////// STOP
//
// folgende defines werden genutzt zur implementierung der
// Stop()-Member der SingleSprm-klassen, da diese im normalfalle
// lediglich EndItem(s) in die shell stellen.
//
#define STOP1(Class, Code) \
    void Class::Stop( \
     Ww1Shell& rOut, BYTE, BYTE*, USHORT, Ww1Manager&) { \
        rOut.EndItem(Code); }
#define STOP2(Class, Code1, Code2) \
    void Class::Stop( \
     Ww1Shell& rOut, BYTE, BYTE*, USHORT, Ww1Manager&) { \
        rOut.EndItem(Code1).EndItem(Code2); }

/////////////////////////////////////////////////////// SingleSprmXxxxx
//
// hier beginnt die auswertung der eigentlichen sprms. jeder sprmtyp
// hat eine eigene klasse, die die virtualen methoden start und stop
// implementiert. die klassen stehen in der sprm-tab, die statischer
// member von Ww1Sprm ist. taucht ein sprm im dokument auf, werden die
// virtualen methoden bei beginn und ende der formatierung gerufen.
//
void Ww1SingleSprmPDxaLeft::Start(
    Ww1Shell& rOut, BYTE, BYTE* pSprm, USHORT, Ww1Manager&)
{
    SvxLRSpaceItem aLR((SvxLRSpaceItem&)rOut.GetAttr(RES_LR_SPACE));
    short nPara = SVBT16ToShort(pSprm);
    if(nPara < 0)
        nPara = 0;
    if(aLR.GetTxtFirstLineOfst() < -nPara)
        aLR.SetTxtFirstLineOfst(-nPara); // sonst weigert sich SetTxtLeft()
    aLR.SetTxtLeft(nPara);
    rOut << aLR;
}

void Ww1SingleSprmPDxaRight::Start(
    Ww1Shell& rOut, BYTE, BYTE* pSprm, USHORT, Ww1Manager&)
{
    SvxLRSpaceItem aLR((SvxLRSpaceItem&)rOut.GetAttr(RES_LR_SPACE));
    short nPara = SVBT16ToShort(pSprm);
    if(nPara < 0)
        nPara = 0;
    aLR.SetRight(nPara);
    rOut << aLR;
}

void Ww1SingleSprmPDxaLeft1::Start(
    Ww1Shell& rOut, BYTE, BYTE* pSprm, USHORT, Ww1Manager&)
{
    SvxLRSpaceItem aLR((SvxLRSpaceItem&)rOut.GetAttr(RES_LR_SPACE));
    short nPara = SVBT16ToShort(pSprm);
    if(-nPara >(short)aLR.GetTxtLeft())
        nPara = -(short)aLR.GetTxtLeft();
    aLR.SetTxtFirstLineOfst(nPara);
    rOut << aLR;
}

STOP1(Ww1SingleSprmPDxa, RES_LR_SPACE)

void Ww1SingleSprmPJc::Start(
    Ww1Shell& rOut, BYTE, BYTE* pSprm, USHORT, Ww1Manager&)
{
    static SvxAdjust __READONLY_DATA aAdj[] = {
        SVX_ADJUST_LEFT,
        SVX_ADJUST_CENTER,
        SVX_ADJUST_RIGHT,
        SVX_ADJUST_BLOCK };
    BYTE nPara = SVBT8ToByte(pSprm);
    nPara %=(sizeof(aAdj)/sizeof(*aAdj));
    rOut << SvxAdjustItem(aAdj[nPara]);
}

STOP1(Ww1SingleSprmPJc, RES_PARATR_ADJUST)

void Ww1SingleSprmPFKeep::Start(
    Ww1Shell& rOut, BYTE, BYTE* pSprm, USHORT, Ww1Manager&)
{
    rOut << SvxFmtSplitItem((SVBT8ToByte(pSprm) & 1) == 0);
}

STOP1(Ww1SingleSprmPFKeep, RES_PARATR_SPLIT)

void Ww1SingleSprmPFKeepFollow::Start(
    Ww1Shell& rOut, BYTE, BYTE* pSprm, USHORT, Ww1Manager&)
{
    rOut << SvxFmtKeepItem((SVBT8ToByte(pSprm) & 1) != 0);
}

STOP1(Ww1SingleSprmPFKeepFollow, RES_KEEP)

void Ww1SingleSprmPPageBreakBefore::Start(
    Ww1Shell& rOut, BYTE, BYTE* pSprm, USHORT, Ww1Manager&)
{
    rOut << SvxFmtBreakItem(SVBT8ToByte(pSprm) & 1?
     SVX_BREAK_PAGE_BEFORE:SVX_BREAK_NONE );
}

STOP1(Ww1SingleSprmPPageBreakBefore, RES_BREAK)

SvxBorderLine* Ww1SingleSprmPBrc::SetBorder(SvxBorderLine* pLine, W1_BRC10* pBrc)
{
    USHORT nCode;
    if(pBrc->dxpLine2WidthGet() == 0)
    {
        switch(pBrc->dxpLine1WidthGet())
        {
        default: ASSERT(FALSE, "unknown linewidth");
        case 0: return 0;                           // keine Linie
        case 1: nCode = DEF_LINE_WIDTH_0; break;
        case 2: nCode = DEF_LINE_WIDTH_1; break;
        case 3: nCode = DEF_LINE_WIDTH_2; break;
        case 4: nCode = DEF_LINE_WIDTH_3; break;
        case 5: nCode = DEF_LINE_WIDTH_4; break;
        }
        pLine->SetOutWidth(nCode);
        pLine->SetInWidth(0);
    }
    else
    {
        switch(pBrc->dxpLine1WidthGet())
        {
        default: ASSERT(FALSE, "unknown linewidth");
        case 1: nCode = DEF_DOUBLE_LINE0_IN; break;
        }
        pLine->SetOutWidth(nCode);
        switch(pBrc->dxpLine2WidthGet())
        {
        default: ASSERT(FALSE, "unknown linewidth");
        case 1: nCode = DEF_DOUBLE_LINE0_OUT; break;
        }
        pLine->SetInWidth(nCode);
    }
    switch(pBrc->dxpLine1WidthGet())
    {
    default: ASSERT(FALSE, "unknown space");
    case 0: nCode = DEF_DOUBLE_LINE0_DIST; break;
    case 1: nCode = DEF_DOUBLE_LINE1_DIST; break;
    case 2: nCode = DEF_DOUBLE_LINE2_DIST; break;
    case 3: nCode = DEF_DOUBLE_LINE3_DIST; break;
    }
    pLine->SetDistance(nCode);
    return pLine;
}
#if 0
SvxBorderLine* Ww1SingleSprmPBrc::SetBorder(SvxBorderLine* pLine, W1_BRC* pBrc)
{
    static USHORT __READONLY_DATA nOutTab[] = { // Aussenlinie
        DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_0,     // No Line -> hair line
        DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_0,
        DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_0,
        DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_0,     // Single Thin Dotted / Dashed
        DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_0,     // Single Line thin(Idx * 15tw)
        DEF_LINE_WIDTH_1, DEF_LINE_WIDTH_2,
        DEF_LINE_WIDTH_2, DEF_LINE_WIDTH_3,
        DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_0,     // Single Thin Dotted / Dashed
        DEF_LINE_WIDTH_0, DEF_LINE_WIDTH_1,     // Single Line thick(Idx * 30tw)
        DEF_LINE_WIDTH_2, DEF_LINE_WIDTH_3,
        DEF_LINE_WIDTH_4, DEF_LINE_WIDTH_4,
        DEF_LINE_WIDTH_1, DEF_LINE_WIDTH_1,     // Single Thick Dotted / Dashed
        DEF_DOUBLE_LINE0_OUT, DEF_DOUBLE_LINE0_OUT, // Double Line(Idx * 15tw)
        DEF_DOUBLE_LINE1_OUT, DEF_DOUBLE_LINE2_OUT,
        DEF_DOUBLE_LINE2_OUT, DEF_DOUBLE_LINE3_OUT,
        DEF_DOUBLE_LINE0_OUT, DEF_DOUBLE_LINE0_OUT }; // Double Dotted / Dashed
    static USHORT __READONLY_DATA nInTab[] = {  // Innenlinie
        0, 0, 0, 0, 0, 0, 0, 0,                 // No line / dotted / dashed
        0, 0, 0, 0, 0, 0, 0, 0,                 // Single Line thin(Idx * 15tw)
        0, 0, 0, 0, 0, 0, 0, 0,                 // Single Line thick(Idx * 30tw)
        DEF_DOUBLE_LINE0_IN, DEF_DOUBLE_LINE0_IN,   // Double Line(Idx * 15tw)
        DEF_DOUBLE_LINE1_IN, DEF_DOUBLE_LINE2_IN,
        DEF_DOUBLE_LINE2_IN, DEF_DOUBLE_LINE3_IN,
        DEF_DOUBLE_LINE0_IN, DEF_DOUBLE_LINE0_IN }; // Double Dotted / Dashed
    static USHORT __READONLY_DATA nDistTab[] = { // Abstand der Linien
        0, 0, 0, 0, 0, 0, 0, 0,                 // No line / dotted / dashed
        0, 0, 0, 0, 0, 0, 0, 0,                 // Single Line thin(Idx * 15tw)
        0, 0, 0, 0, 0, 0, 0, 0,                 // Single Line thick(Idx * 30tw)
        DEF_DOUBLE_LINE0_DIST, DEF_DOUBLE_LINE0_DIST, // Double Line(Idx * 15tw)
        DEF_DOUBLE_LINE1_DIST, DEF_DOUBLE_LINE2_DIST,
        DEF_DOUBLE_LINE2_DIST, DEF_DOUBLE_LINE3_DIST,
        DEF_DOUBLE_LINE0_DIST, DEF_DOUBLE_LINE0_DIST }; // Double Dotted / Dashed
    short nIdx = pBrc->dxpLineWidthGet() + 8 * pBrc->brcTypeGet();
    pLine->SetOutWidth(nOutTab[nIdx]);
    pLine->SetInWidth(nInTab[nIdx]);
    pLine->SetDistance(nDistTab[nIdx]);
    if(pBrc->icoGet())
    {
        ColorName nIdx;
        switch(pBrc->icoGet())
        {
        default: ASSERT(FALSE, "unknown color code");
        case 0:
        case 1: nIdx = COL_BLACK; break;
        case 2: nIdx = COL_LIGHTBLUE; break;
        case 3: nIdx = COL_LIGHTCYAN; break;
        case 4: nIdx = COL_LIGHTGREEN; break;
        case 5: nIdx = COL_LIGHTMAGENTA; break;
        case 6: nIdx = COL_LIGHTRED; break;
        case 7: nIdx = COL_YELLOW; break;
        case 8: nIdx = COL_WHITE; break;
        }
        Color aCol(nIdx);
        pLine->SetColor(aCol);
    }
    return pLine;
}
#endif

void Ww1SingleSprmPBrc::Start(
    Ww1Shell& rOut, BYTE, W1_BRC10* pBrc, USHORT nSize, Ww1Manager& rMan, SvxBoxItem& aBox)
{
    ASSERT(sizeof(W1_BRC10) == nSize, "sizemissmatch");
    if(pBrc->dxpSpaceGet())
        aBox.SetDistance(10 + 20 * pBrc->dxpSpaceGet());
            //??? Warum 10+... ????

    if( rOut.IsInFly() )
        rOut.SetFlyFrmAttr( aBox );
    else
        rOut << aBox;

    if(pBrc->fShadowGet())
    {
        Color aBlack(COL_BLACK); // schwarzer...
        SvxShadowItem aS(ITEMID_SHADOW,(const Color*)&aBlack, 32,
                         SVX_SHADOW_BOTTOMRIGHT); // 1.6 tw breit
        if( rOut.IsInFly() )
            rOut.SetFlyFrmAttr( aS );
        else
            rOut << aS;
    }
}

STOP2(Ww1SingleSprmPBrc, RES_BOX, RES_SHADOW)

static USHORT __READONLY_DATA nBrcTrans[BRC_ANZ] =
     { BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT };

void Ww1SingleSprmPBrc10::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{

    W1_BRC10* pBrc =(W1_BRC10*)pSprm;
    const SfxPoolItem &rItem = ( ( rOut.IsInFly() ) ?
                     rOut.GetFlyFrmAttr(RES_BOX) :rOut.GetAttr(RES_BOX));
    const SvxBoxItem &rBoxItem = (const SvxBoxItem&)rItem;
    SvxBoxItem aBox( rBoxItem );
//  rOut >> aBox;
    SvxBorderLine aLine;
    aBox.SetLine(SetBorder(&aLine, pBrc), nBrcTrans[nLine] );
    Ww1SingleSprmPBrc::Start(rOut, nId, pBrc, nSize, rMan, aBox);
}

STOP1(Ww1SingleSprmParaSpace, RES_UL_SPACE)

void Ww1SingleSprmPDyaBefore::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nPara = SVBT16ToShort(pSprm);
    if(nPara < 0)
        nPara = -nPara;
    SvxULSpaceItem aUL((SvxULSpaceItem&)rOut.GetAttr(RES_UL_SPACE));
    aUL.SetUpper(nPara);
    rOut << aUL;
}

void Ww1SingleSprmPDyaAfter::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nPara = SVBT16ToShort(pSprm);
    if(nPara < 0)
        nPara = -nPara;
    SvxULSpaceItem aUL((SvxULSpaceItem&)rOut.GetAttr(RES_UL_SPACE));
    aUL.SetLower(nPara);
    rOut << aUL;
}

STOP1(Ww1SingleSprmPDyaLine, RES_PARATR_LINESPACING)

void Ww1SingleSprmPDyaLine::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nSpace = SVBT16ToShort(pSprm);
    if(nSpace < 0)
        nSpace = -nSpace;
    SvxLineSpacingItem aLSpc;
    if(TRUE)
    {// MultilineSpace(proportional)
        long n = nSpace * 100 / 240;    // W1: 240 = 100%, SW: 100 = 100%
        if(n>200)
            n = 200; // SW_UI-Maximum
        aLSpc.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
        aLSpc.SetPropLineSpace((BYTE)n);
    }
    else
    {// Fixed / Minimum
        aLSpc.SetLineHeight((USHORT)nSpace);
        aLSpc.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
    }
    rOut << aLSpc;
}

void Ww1SingleSprmPChgTabsPapx::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
#ifdef DEBUG
//  rOut << 'T';
#endif
    short nLeftPMgn = 0;    // Koordinaten etwa gleich ??
//  ( pAktColl ) ? pCollA[nAktColl].nLeftParaMgn
//                                 : nLeftParaMgn;      // Absatz L-Space

    short i;
    BYTE nDel = pSprm[1];
    BYTE* pDel = pSprm + 2;                     // Del - Array
    BYTE nIns = pSprm[nDel*2+2];
    BYTE* pIns = pSprm + 2*nDel + 3;            // Ins - Array
    W1_TBD* pTyp = (W1_TBD*)(pSprm + 2*nDel + 2*nIns + 3);// Typ - Array

    SvxTabStopItem aAttr( (SvxTabStopItem&)rOut.GetNodeOrStyAttr( RES_PARATR_TABSTOP ));

    SvxTabStop aTabStop;

    for( i=0; i<nDel; i++ ){
        USHORT nPos = aAttr.GetPos( SVBT16ToShort( pDel + i*2 ) - nLeftPMgn );
        if( nPos != SVX_TAB_NOTFOUND )
            aAttr.Remove( nPos, 1 );
    }

    for( i=0; i<nIns; i++ ){
        short nPos = SVBT16ToShort( pIns + i*2 ) - nLeftPMgn;
        if( nPos < 0 )
            continue;
        aTabStop.GetTabPos() = nPos;
        switch( pTyp[i].jcGet() ){
        case 0: aTabStop.GetAdjustment() = SVX_TAB_ADJUST_LEFT; break;
        case 1: aTabStop.GetAdjustment() = SVX_TAB_ADJUST_CENTER; break;
        case 2: aTabStop.GetAdjustment() = SVX_TAB_ADJUST_RIGHT; break;
        case 3: aTabStop.GetAdjustment() = SVX_TAB_ADJUST_DECIMAL; break;
        case 4: continue;                   // ignoriere Bar
        }

        switch( pTyp[i].tlcGet() ){
        case 0: aTabStop.GetFill() = ' '; break;
        case 1: aTabStop.GetFill() = '.'; break;
        case 2: aTabStop.GetFill() = '-'; break;
        case 3:
        case 4: aTabStop.GetFill() = '_'; break;
        }

        USHORT nPos2 = aAttr.GetPos( nPos );
        if( nPos2 != SVX_TAB_NOTFOUND )
            aAttr.Remove( nPos2, 1 );       // sonst weigert sich das Insert()

        aAttr.Insert( aTabStop );
    }
    rOut << aAttr;
}

STOP1(Ww1SingleSprmPChgTabsPapx, RES_PARATR_TABSTOP)

void Ww1SingleSprmSGprfIhdt::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    rMan.GetSep().SetGrpfIhdt(SVBT8ToByte(pSprm));
}

void Ww1SingleSprmSColumns::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nColSpace = 708; // default 1,25 cm
    short nNettoWidth = -1; //~ mdt: dxaLeft/Right missing...
    short nCols = SVBT16ToShort(pSprm);
    nCols++;
    if (nCols < 2)
        return;
    SwFmtCol aCol;
    aCol.Init(nCols, nColSpace, nNettoWidth);
    rOut.GetPageDesc().GetMaster().SetAttr(aCol);
}

void Ww1SingleSprmPTtp::Start(
    Ww1Shell& rOut, BYTE, BYTE*, USHORT, Ww1Manager& rMan)
{
    rMan.SetInTtp( TRUE );      // Besonderheit: wird bei InTable::Stop und
                                // nicht bei InTtp::Stop zurueckgesetzt,
                                // da Auswertung in InTable
}

void Ww1SingleSprmPTtp::Stop(
    Ww1Shell& rOut, BYTE, BYTE*, USHORT, Ww1Manager& rMan)
{
    if (rOut.IsInTable() && rMan.HasInTable())
        rOut.NextTableRow();
}

void Ww1SingleSprmPFInTable::Start(
    Ww1Shell& rOut, BYTE, BYTE*, USHORT,
    Ww1Manager& rMan)
{
}

void Ww1SingleSprmPFInTable::Stop(
    Ww1Shell& rOut, BYTE, BYTE*, USHORT,
    Ww1Manager& rMan)
{
    ASSERT(rOut.IsInTable(), "");
//  if (rOut.IsInTable() && rMan.HasInTable() && !rMan.IsInTtp() && !rMan.HasTtp())
//      rOut.NextTableCell();
    rMan.SetInTtp( FALSE );
}

void Ww1SingleSprmTJc::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nSpace = SVBT16ToShort(pSprm);
}

#if 0
void Ww1SingleSprmTDxaLeft::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nSpace = SVBT16ToShort(pSprm);
    rOut.SetCellWidth(nSpace - rMan.GetX());
    rMan.SetX(nSpace);
}
#endif

void Ww1SingleSprmTDxaGapHalf::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nSpace = SVBT16ToShort(pSprm);
    rOut.SetCellSpace(nSpace);
}

void Ww1SingleSprmTDefTable10::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    int i;
    BYTE *p = pSprm + 2;    // LaengenWord ueberlesen
    BYTE nCount = *p;
    p++;
    nSize -= 3;
// Es fehlt noch:
// - GapHalf
// - eventuelle Ausduennung der Zellenumrandungen

    if( nCount < 1 || nCount > 32 || nSize < ( nCount + 1 ) * 2  )
        return;

// Erstmal die Zellenpositionen einlesen
    short nPos = SVBT16ToShort( p );    // signed, kann auch neg. sein !!!

//  if( !rOut.IsTableWidthSet() ){      // Muss Tabellenbreite und -Ausrichtung
                                        // noch gesetzt werden ?
    {
        short nWholeWidth = SVBT16ToShort( p + 2 * nCount ) - nPos;
        rOut.SetTableWidth( (USHORT)nWholeWidth );  // Tabellenbreite setzen

// Pos der Tabelle setzen
        long nMidTab = nPos + nWholeWidth / 2;      // TabellenMitte
        const SwFrmFmt &rFmt = rOut.GetPageDesc().GetMaster();
        const SvxLRSpaceItem& rLR = rFmt.GetLRSpace();
        long nRight = rFmt.GetFrmSize().GetWidth()
                      - rLR.GetLeft() - rLR.GetRight();

        SwHoriOrient eOri = HORI_LEFT;
        if( nPos > MINLAY ){                        // per Zuppeln positioniert
            if ( nMidTab <= nRight / 3 )            // 1/3 der Seite
                eOri = HORI_LEFT;
            else if ( nMidTab <= 2 * nRight / 3 )   // 2/3 der Seite
                eOri = HORI_CENTER;
            else
                eOri = HORI_RIGHT;
        }
        rOut.SetTableOrient( eOri );
    }

    BYTE* pEndPos = p+2;
    BYTE* pTc0 = ( nSize >= nCount * 10 ) ? pEndPos + 2 * nCount : 0;
    USHORT nCellsDeleted = 0;       // fuer gemergte Zellen

    for( i = 0; i < nCount; i++ ){
// Info sammeln
        W1_TC* pTc = (W1_TC*)pTc0;
        BOOL bFirstMerged = (pTc) ? pTc->fFirstMergedGet() : FALSE;
        BOOL bMerged = (pTc) ? pTc->fMergedGet() : FALSE;

// Zellenbreiten setzen
        USHORT nPos1 = SVBT16ToShort( pEndPos );
        if( !bMerged )
            rOut.SetCellWidth( nPos1 - nPos, i - nCellsDeleted );
                                        // Zellenbreite setzen
                                        // Wechselwirkung mit GapHalf fehlt noch
                                        // ( GapHalf wird noch ignoriert )
        pEndPos+=2;
        nPos = nPos1;

        if( pTc0 ){                     // gibts TCs ueberhaupt ?
            W1_TC* pTc = (W1_TC*)pTc0;
            BOOL bFirstMerged = pTc->fFirstMergedGet();
            BOOL bMerged = pTc->fMergedGet();
//          ASSERT( !bMerged, "Gemergte Tabellenzellen noch nicht vollstaendig implementiert" );
            if( !bMerged ){
// und nun die Umrandungen
                SvxBoxItem aBox( (SvxBoxItem&)rOut.GetCellAttr( RES_BOX ));
                SvxBorderLine aLine;
                W1_BRC10* pBrc = pTc->rgbrcGet();
                for( USHORT j=0; j<4; j++ ){
                    aBox.SetLine(Ww1SingleSprmPBrc::SetBorder(&aLine, pBrc),
                                 nBrcTrans[j]);
                    pBrc++;
                }
                rOut.SetCellBorder( aBox, i - nCellsDeleted );
            }else{
// gemergte Zellen entfernen
                rOut.DeleteCell( i - nCellsDeleted );
                nCellsDeleted++;
            }
            pTc0+=sizeof(W1_TC);    // 10
        }
    }
}


void Ww1SingleSprmTDyaRowHeight::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nSpace = SVBT16ToShort(pSprm);
    rOut.SetCellHeight(nSpace);
}

// Fastsave-Attribute brauche ich als Dymmys nicht

#if 0

void Ww1SingleSprmTInsert::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
}

void Ww1SingleSprmTDelete::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nSpace = SVBT16ToShort(pSprm);
}

void Ww1SingleSprmTDxaCol::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
}

void Ww1SingleSprmTMerge::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nSpace = SVBT16ToShort(pSprm);
}

void Ww1SingleSprmTSplit::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nSpace = SVBT16ToShort(pSprm);
}

void Ww1SingleSprmTSetBrc10::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
}
#endif // 0

void Ww1SingleSprmPpc::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    BYTE nPpc = SVBT8ToByte(pSprm);

    if (rOut.IsInTable())                       // Flys in Tabellen kann PMW
        return;                                 // nicht

    RndStdIds eAnchor;          // Bindung
    SwRelationOrient eHRel;     // Seite oder Seitenrand
    SwRelationOrient eVRel;     // Seite oder Seitenrand

    switch ( ( nPpc & 0x30 ) >> 4 ){        // Y - Bindung bestimmt Sw-Bindung
    case 0: eAnchor = FLY_AT_CNTNT;         // Vert Margin
            eVRel = PRTAREA;
//          if( nYPos < 0 )
//              nYPos = 0;                  // koennen wir nicht
            break;
/*  case 1:*/                               // Vert. Seite
    default:eAnchor = FLY_PAGE;             // Vert Page oder unknown
            eVRel = FRAME;
            break;                          // 2=Vert. Paragraph, 3=Use Default
    }

    switch ( ( nPpc & 0xc0 ) >> 6 ){        // X - Bindung -> Koordinatentransformation
    case 0:                                 // Hor. Spalte
    case 1:                                 // Hor. Absatz
            eHRel = PRTAREA;
//          nXPos += nPgLeft;               // in Seiten-Koordinaten umrechnen
            break;
/*  case 2:*/                               // Hor. Seite
    default:
            eHRel = FRAME;
            break;
    }

    if( !rOut.IsInFly() && rMan.IsInStyle() ){
        rOut.BeginFly( eAnchor );           // Starte StyleFly
    }else{
        rOut.SetFlyAnchor( eAnchor );       // Setze Anker
    }
}

void Ww1SingleSprmPDxaAbs::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nXPos = SVBT16ToShort(pSprm);

    if( rMan.IsInStyle() && !rOut.IsInFly() ){
        rOut.BeginFly();                    // Fly ohne PPc-Attribut
    }

    SwRelationOrient eHRel = FRAME;
    SwHoriOrient eHAlign = HORI_NONE;

    switch( nXPos ){                        // besondere X-Positionen ?
    case 0:
    case -12: eHAlign = HORI_NONE; nXPos = 0; break;   // Mogel: innen -> links
                            // eigentich HORI_LEFT, aber dann verrueckt
                            // ein Abstand nach aussen den Fly
    case -4:  eHAlign = HORI_CENTER; nXPos = 0; break; // zentriert
    case -8:                                           // rechts
    case -16: eHAlign = HORI_RIGHT; nXPos = 0; break;  // Mogel: aussen -> rechts
//  default:  nXPos += (short)nIniFlyDx; break; // Korrekturen per Ini-Datei
    }
    rOut.SetFlyXPos( nXPos, eHRel, eHAlign );
}

void Ww1SingleSprmPDyaAbs::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nYPos = SVBT16ToShort(pSprm);
    SwRelationOrient eVRel = FRAME;
    SwVertOrient eVAlign = VERT_NONE;

    switch( nYPos ){                            // besondere Y-Positionen ?
    case -4:  eVAlign = VERT_TOP; nYPos = 0; break; // oben
    case -8:  eVAlign = VERT_CENTER; nYPos = 0; break;  // zentriert
    case -12: eVAlign = VERT_BOTTOM; nYPos = 0; break;  // unten
//  default:  nYPos += (short)nIniFlyDy; break; // Korrekturen per Ini-Datei
    }
    rOut.SetFlyYPos( nYPos, eVRel, eVAlign );
}

void Ww1SingleSprmPDxaWidth::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    short nDxaWidth = SVBT16ToShort(pSprm);
    rOut.SetFlyFrmAttr( SwFmtFrmSize( ATT_VAR_SIZE, nDxaWidth, MINFLY ) );
}

void Ww1SingleSprmPFromText::Start(
    Ww1Shell& rOut, BYTE nId, BYTE* pSprm, USHORT nSize, Ww1Manager& rMan)
{
    if( rOut.IsInFly() ){   // Kommt auch ausserhalb eines Flys vor, hat
                            // dann aber offensichtlich nichts zu bedeuten.
                            // Einen impliziten Fly-Anfang bedeutet es
                            // definitiv nicht
        short nFromText = SVBT16ToShort(pSprm);

        SvxLRSpaceItem aLR;
        aLR.SetTxtLeft( nFromText );
        aLR.SetRight( nFromText );
        rOut.SetFlyFrmAttr( aLR );

        rOut.SetFlyFrmAttr( SvxULSpaceItem( nFromText, nFromText ) );
    }
}

#undef STOP1
#undef STOP2

/**********************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww1/w1sprm.cxx,v 1.1.1.1 2000-09-18 17:14:58 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.19  2000/09/18 16:04:57  willem.vandorp
      OpenOffice header added.

      Revision 1.18  2000/05/11 16:06:59  jp
      Changes for Unicode

      Revision 1.17  2000/03/03 15:22:12  os
      StarView remainders removed

      Revision 1.16  1998/06/29 11:01:32  JP
      SvxFmtBreakItem ohne Flag!


      Rev 1.15   29 Jun 1998 13:01:32   JP
   SvxFmtBreakItem ohne Flag!

      Rev 1.14   19 Feb 1998 17:01:14   HJS
   fuer sh eingechecked

      Rev 1.12   29 Nov 1997 17:37:08   MA
   includes

      Rev 1.11   09 Oct 1997 14:13:48   JP
   Aenderungen von SH

      Rev 1.6   11 Aug 1997 12:52:20   SH
   Flys und einfache Tabellen und Tabulatoren

      Rev 1.5   18 Jul 1997 09:31:38   MA
   Stand von SH, #41738# GPF und Tabellen etwas besser

      Rev 1.4   16 Jun 1997 13:08:56   MA
   Stand Micheal Dietrich

      Rev 1.3   12 Mar 1997 19:12:54   SH
   MDT: Progressbar, Pagedesc-Bug, Spalten, Anfang Tabellen u.a.

      Rev 1.2   13 Jan 1997 16:42:02   NF
   __READONLY_DATA fÅr Win16 korrigiert: Typ __READONLY_DATA identifier ...

      Rev 1.1   10 Jan 1997 18:47:34   SH
   Stabiler und mehr von MDT

      Rev 1.0   28 Nov 1996 18:09:30   SH
   Schneller Schoener Weiter von MDT

      Rev 1.0   14 Aug 1996 19:32:28   SH
   Initial revision.


**********************************************************************/
//]})
