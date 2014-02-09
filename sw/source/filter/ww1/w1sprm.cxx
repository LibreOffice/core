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


#include <hintids.hxx>
#include <tools/solar.h>
#include <editeng/paperinf.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/tstpitem.hxx>
#include <fmtclds.hxx>
#include <fmtfsize.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <charatr.hxx>
#include <frmatr.hxx>
#include <doc.hxx>
#include <fltini.hxx>
#include <docufld.hxx>
#include <pagedesc.hxx>
#include <flddat.hxx>
#include <reffld.hxx>
#include <expfld.hxx>
#include <w1par.hxx>
#include <mdiexp.hxx>
#include <swerror.h>
#include <statstr.hrc>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;

void Ww1Sprm::Stop( Ww1Shell& rOut, Ww1Manager& rMan)
{
    if(IsUsed())
        for(short i=Count()-1;i>=0;i--){    // backwards
            sal_uInt8 nId;
            sal_uInt16 nSize;
            sal_uInt8* pSprm;
            Fill(i, nId, nSize, pSprm);
            GetTab(nId).Stop(rOut, nId, pSprm, nSize, rMan);
        }
}

void Ww1Sprm::Start(
    Ww1Shell& rOut, Ww1Manager& rMan, sal_uInt16 i)
{
    sal_uInt8 nId;
    sal_uInt16 nSize;
    sal_uInt8* pSprm;
    Fill(i, nId, nSize, pSprm);
    GetTab(nId).Start(rOut, nId, pSprm, nSize, rMan);
}

void Ww1Sprm::Start(
    Ww1Shell& rOut, Ww1Manager& rMan)
{
    if(IsUsed())
        for(sal_uInt16 i=0;i<Count();i++)
            Start(rOut, rMan, i);
}

void Ww1SingleSprm::Start(
    Ww1Shell&, sal_uInt8 /*nId*/, sal_uInt8*, sal_uInt16, Ww1Manager&)
{
}

void Ww1SingleSprm::Stop(
    Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&)
{
// OSL_ENSURE(FALSE, "Unknown Sprm");
}

// STOP
// The following defines are used for implementing the SingleSprm
// classes' Stop() members, because they normally merely put
// EndItem(s) into the shell.
#define STOP1(Class, Code) \
    void Class::Stop( \
     Ww1Shell& rOut, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) { \
        rOut.EndItem(Code); }
#define STOP2(Class, Code1, Code2) \
    void Class::Stop( \
     Ww1Shell& rOut, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) { \
        rOut.EndItem(Code1).EndItem(Code2); }

// SingleSprmXxxxx
// This starts the evaluation of the actual sprms. Each sprmtyp has
// its own class implementing the virtual methods start and stop. The
// classes are referenced from the sprm table, a static member of
// Ww1Sprm. When an sprm is encountered in the document, the virtual
// methods will be called at the formatting boundaries.
void Ww1SingleSprmPDxaLeft::Start(
    Ww1Shell& rOut, sal_uInt8, sal_uInt8* pSprm, sal_uInt16, Ww1Manager&)
{
    SvxLRSpaceItem aLR((SvxLRSpaceItem&)rOut.GetAttr(RES_LR_SPACE));
    short nPara = SVBT16ToShort(pSprm);
    if(nPara < 0)
        nPara = 0;
    if(aLR.GetTxtFirstLineOfst() < -nPara)
        aLR.SetTxtFirstLineOfst(-nPara); // otherwise SetTxtLeft() refuses to work
    aLR.SetTxtLeft(nPara);
    rOut << aLR;
}

void Ww1SingleSprmPDxaRight::Start(
    Ww1Shell& rOut, sal_uInt8, sal_uInt8* pSprm, sal_uInt16, Ww1Manager&)
{
    SvxLRSpaceItem aLR((SvxLRSpaceItem&)rOut.GetAttr(RES_LR_SPACE));
    short nPara = SVBT16ToShort(pSprm);
    if(nPara < 0)
        nPara = 0;
    aLR.SetRight(nPara);
    rOut << aLR;
}

void Ww1SingleSprmPDxaLeft1::Start(
    Ww1Shell& rOut, sal_uInt8, sal_uInt8* pSprm, sal_uInt16, Ww1Manager&)
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
    Ww1Shell& rOut, sal_uInt8, sal_uInt8* pSprm, sal_uInt16, Ww1Manager&)
{
    static SvxAdjust const aAdj[] = {
        SVX_ADJUST_LEFT,
        SVX_ADJUST_CENTER,
        SVX_ADJUST_RIGHT,
        SVX_ADJUST_BLOCK };
    sal_uInt8 nPara = *pSprm;
    nPara %=SAL_N_ELEMENTS(aAdj);
    rOut << SvxAdjustItem(aAdj[nPara], RES_PARATR_ADJUST);
}

STOP1(Ww1SingleSprmPJc, RES_PARATR_ADJUST)

void Ww1SingleSprmPFKeep::Start(
    Ww1Shell& rOut, sal_uInt8, sal_uInt8* pSprm, sal_uInt16, Ww1Manager&)
{
    rOut << SvxFmtSplitItem((*pSprm & 1) == 0, RES_PARATR_SPLIT);
}

STOP1(Ww1SingleSprmPFKeep, RES_PARATR_SPLIT)

void Ww1SingleSprmPFKeepFollow::Start(
    Ww1Shell& rOut, sal_uInt8, sal_uInt8* pSprm, sal_uInt16, Ww1Manager&)
{
    rOut << SvxFmtKeepItem((*pSprm & 1) != 0, RES_KEEP);
}

STOP1(Ww1SingleSprmPFKeepFollow, RES_KEEP)

void Ww1SingleSprmPPageBreakBefore::Start(
    Ww1Shell& rOut, sal_uInt8, sal_uInt8* pSprm, sal_uInt16, Ww1Manager&)
{
    rOut << SvxFmtBreakItem(*pSprm & 1?
     SVX_BREAK_PAGE_BEFORE:SVX_BREAK_NONE, RES_BREAK );
}

STOP1(Ww1SingleSprmPPageBreakBefore, RES_BREAK)

SvxBorderLine* Ww1SingleSprmPBrc::SetBorder(SvxBorderLine* pLine, W1_BRC10* pBrc)
{
    sal_uInt16 nCode;
    ::editeng::SvxBorderStyle eStyle = table::BorderLineStyle::SOLID;
    if(pBrc->dxpLine2WidthGet() == 0)
    {
        switch(pBrc->dxpLine1WidthGet())
        {
        default: OSL_FAIL("unknown linewidth");
        case 0: return 0;                           // no line
        case 1: nCode = DEF_LINE_WIDTH_0; break;
        case 2: nCode = DEF_LINE_WIDTH_1; break;
        case 3: nCode = DEF_LINE_WIDTH_2; break;
        case 4: nCode = DEF_LINE_WIDTH_3; break;
        case 5: nCode = DEF_LINE_WIDTH_4; break;
        case 6:
                nCode = DEF_LINE_WIDTH_5;
                eStyle = table::BorderLineStyle::DOTTED;
                break;
        case 7:
                nCode = DEF_LINE_WIDTH_5;
                eStyle = table::BorderLineStyle::DASHED;
                break;
        }
        pLine->SetWidth( nCode );
        pLine->SetBorderLineStyle( eStyle );
    }
    else
    {
        if ( pBrc->dxpLine1WidthGet() == 1 && pBrc->dxpLine2WidthGet() == 1 )
        {
            pLine->SetBorderLineStyle( table::BorderLineStyle::DOUBLE );
            pLine->SetWidth( DEF_LINE_WIDTH_0 );
        }
        else
            OSL_ENSURE(false, "unknown linewidth");
    }
    return pLine;
}

void Ww1SingleSprmPBrc::Start(
    Ww1Shell& rOut, sal_uInt8,
    W1_BRC10* pBrc,
    sal_uInt16
#if OSL_DEBUG_LEVEL > 0
    nSize
#endif
    ,
    Ww1Manager& /*rMan*/,
    SvxBoxItem& aBox)
{
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE(sizeof(W1_BRC10) == nSize, "sizemissmatch");
#endif
    if(pBrc->dxpSpaceGet())
        aBox.SetDistance(10 + 20 * pBrc->dxpSpaceGet());
            //??? Why 10+... ????

    if( rOut.IsInFly() )
        rOut.SetFlyFrmAttr( aBox );
    else
        rOut << aBox;

    if(pBrc->fShadowGet())
    {
        Color aBlack(COL_BLACK); // black...
        SvxShadowItem aS(RES_SHADOW,(const Color*)&aBlack, 32,
                         SVX_SHADOW_BOTTOMRIGHT); // 1.6 tw wide
        if( rOut.IsInFly() )
            rOut.SetFlyFrmAttr( aS );
        else
            rOut << aS;
    }
}

STOP2(Ww1SingleSprmPBrc, RES_BOX, RES_SHADOW)

static sal_uInt16 nBrcTrans[BRC_ANZ] =
     { BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT };

void Ww1SingleSprmPBrc10::Start(
    Ww1Shell& rOut, sal_uInt8 nId, sal_uInt8* pSprm, sal_uInt16 nSize, Ww1Manager& rMan)
{

    W1_BRC10* pBrc =(W1_BRC10*)pSprm;
    const SfxPoolItem &rItem = ( ( rOut.IsInFly() ) ?
                     rOut.GetFlyFrmAttr(RES_BOX) :rOut.GetAttr(RES_BOX));
    const SvxBoxItem &rBoxItem = (const SvxBoxItem&)rItem;
    SvxBoxItem aBox( rBoxItem );
    SvxBorderLine aLine;
    aBox.SetLine(SetBorder(&aLine, pBrc), nBrcTrans[nLine] );
    Ww1SingleSprmPBrc::Start(rOut, nId, pBrc, nSize, rMan, aBox);
}

STOP1(Ww1SingleSprmParaSpace, RES_UL_SPACE)

void Ww1SingleSprmPDyaBefore::Start(
    Ww1Shell& rOut, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 /*nSize*/, Ww1Manager& /*rMan*/)
{
    short nPara = SVBT16ToShort(pSprm);
    if(nPara < 0)
        nPara = -nPara;
    SvxULSpaceItem aUL((SvxULSpaceItem&)rOut.GetAttr(RES_UL_SPACE));
    aUL.SetUpper(nPara);
    rOut << aUL;
}

void Ww1SingleSprmPDyaAfter::Start(
    Ww1Shell& rOut, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 /*nSize*/, Ww1Manager& /*rMan*/)
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
    Ww1Shell& rOut, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 /*nSize*/, Ww1Manager& /*rMan*/)
{
    short nSpace = SVBT16ToShort(pSprm);
    if(nSpace < 0)
        nSpace = -nSpace;
    SvxLineSpacingItem aLSpc( LINE_SPACE_DEFAULT_HEIGHT, RES_PARATR_LINESPACING );
    {// MultilineSpace(proportional)
        long n = nSpace * 100 / 240;    // W1: 240 = 100%, SW: 100 = 100%
        if(n>200)
            n = 200; // SW_UI-Maximum
        aLSpc.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
        aLSpc.SetPropLineSpace((sal_uInt8)n);
    }
    rOut << aLSpc;
}

void Ww1SingleSprmPChgTabsPapx::Start(
    Ww1Shell& rOut, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 /*nSize*/, Ww1Manager& /*rMan*/)
{

    short nLeftPMgn = 0;    // Coordinates approximately equal ??

    short i;
    sal_uInt8 nDel = pSprm[1];
    sal_uInt8* pDel = pSprm + 2;                        // Del - Array
    sal_uInt8 nIns = pSprm[nDel*2+2];
    sal_uInt8* pIns = pSprm + 2*nDel + 3;           // Ins - Array
    W1_TBD* pTyp = (W1_TBD*)(pSprm + 2*nDel + 2*nIns + 3);// Typ - Array

    SvxTabStopItem aAttr( (SvxTabStopItem&)rOut.GetNodeOrStyAttr( RES_PARATR_TABSTOP ));

    SvxTabStop aTabStop;

    for( i=0; i<nDel; i++ ){
        sal_uInt16 nPos = aAttr.GetPos( SVBT16ToShort( pDel + i*2 ) - nLeftPMgn );
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
        case 4: continue;                   // ignore Bar
        }

        switch( pTyp[i].tlcGet() ){
        case 0: aTabStop.GetFill() = ' '; break;
        case 1: aTabStop.GetFill() = '.'; break;
        case 2: aTabStop.GetFill() = '-'; break;
        case 3:
        case 4: aTabStop.GetFill() = '_'; break;
        }

        sal_uInt16 nPos2 = aAttr.GetPos( nPos );
        if( nPos2 != SVX_TAB_NOTFOUND )
            aAttr.Remove( nPos2, 1 );       // otherwise Insert() refuses to work

        aAttr.Insert( aTabStop );
    }
    rOut << aAttr;
}

STOP1(Ww1SingleSprmPChgTabsPapx, RES_PARATR_TABSTOP)

void Ww1SingleSprmSGprfIhdt::Start(
    Ww1Shell& /*rOut*/, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 /*nSize*/, Ww1Manager& rMan)
{
    rMan.GetSep().SetGrpfIhdt(*pSprm);
}

void Ww1SingleSprmSColumns::Start(
    Ww1Shell& rOut, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 /*nSize*/, Ww1Manager& /*rMan*/)
{
    short nColSpace = 708; // default 1,25 cm
    short nNettoWidth = -1; //~ mdt: dxaLeft/Right missing...
    short nCols = SVBT16ToShort(pSprm);
    nCols++;
    if (nCols < 2)
        return;
    SwFmtCol aCol;
    aCol.Init(nCols, nColSpace, nNettoWidth);
    rOut.GetPageDesc().GetMaster().SetFmtAttr(aCol);
}

void Ww1SingleSprmPTtp::Start(
    Ww1Shell& /*rOut*/, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager& rMan)
{
    rMan.SetInTtp( true );      // Special case: will be reset at
                                // InTable::Stop instead of InTtp::Stop,
                                // because evaluation happens in InTable
}

void Ww1SingleSprmPTtp::Stop(
    Ww1Shell& rOut, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager& rMan)
{
    if (rOut.IsInTable() && rMan.HasInTable())
        rOut.NextTableRow();
}

void Ww1SingleSprmPFInTable::Start(
    Ww1Shell& /*rOut*/, sal_uInt8, sal_uInt8*, sal_uInt16,
    Ww1Manager& /*rMan*/)
{
}

void Ww1SingleSprmPFInTable::Stop(
    Ww1Shell&
#if OSL_DEBUG_LEVEL > 0
    rOut
#endif
    ,
    sal_uInt8, sal_uInt8*, sal_uInt16,
    Ww1Manager& /*rMan*/)
{
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE(rOut.IsInTable(), "");
#endif
}

void Ww1SingleSprmTDxaGapHalf::Start(
    Ww1Shell& rOut, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 /*nSize*/, Ww1Manager& /*rMan*/)
{
    short nSpace = SVBT16ToShort(pSprm);
    rOut.SetCellSpace(nSpace);
}

void Ww1SingleSprmTDefTable10::Start(
    Ww1Shell& rOut, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 nSize, Ww1Manager& /*rMan*/)
{
    sal_uInt16 i;
    sal_uInt8 *p = pSprm + 2;   // skip length word
    sal_uInt8 nCount = *p;
    p++;
    nSize -= 3;
// Still missing:
// - GapHalf
// - potential thinning of cell borders

    if( nCount < 1 || nCount > 32 || nSize < ( nCount + 1 ) * 2  )
        return;

// Let's read the cell positions for starters
    short nPos = SVBT16ToShort( p );    // signed, can be negative !!!

    {
        short nWholeWidth = SVBT16ToShort( p + 2 * nCount ) - nPos;
        rOut.SetTableWidth( (sal_uInt16)nWholeWidth );  // set table width

// Set table position
        long nMidTab = nPos + nWholeWidth / 2;      // table center
        const SwFrmFmt &rFmt = rOut.GetPageDesc().GetMaster();
        const SvxLRSpaceItem& rLR = rFmt.GetLRSpace();
        long nRight = rFmt.GetFrmSize().GetWidth()
                      - rLR.GetLeft() - rLR.GetRight();

        sal_Int16 eOri = text::HoriOrientation::LEFT;
        if( nPos > MINLAY ){                        // positioned by twiddling
            if ( nMidTab <= nRight / 3 )            // 1/3rd of page
                eOri = text::HoriOrientation::LEFT;
            else if ( nMidTab <= 2 * nRight / 3 )   // 2/3rd of page
                eOri = text::HoriOrientation::CENTER;
            else
                eOri = text::HoriOrientation::RIGHT;
        }
        rOut.SetTableOrient( eOri );
    }

    sal_uInt8* pEndPos = p+2;
    sal_uInt8* pTc0 = ( nSize >= nCount * 10 ) ? pEndPos + 2 * nCount : 0;
    sal_uInt16 nCellsDeleted = 0;       // for merged cells

    for( i = 0; i < nCount; i++ ){
// Collect info
        W1_TC* pTc = (W1_TC*)pTc0;
        sal_Bool bMerged = (pTc) ? pTc->fMergedGet() : sal_False;

// set cell widths
        sal_uInt16 nPos1 = SVBT16ToShort( pEndPos );
        if( !bMerged )
            rOut.SetCellWidth( nPos1 - nPos, i - nCellsDeleted );
                                        // set cell width
                                        // Interaction with GapHalf still missing
                                        // ( GapHalf is being ignored for now )
        pEndPos+=2;
        nPos = nPos1;

        if( pTc0 ){                     // are there actually TCs ?
            W1_TC* pTc2 = (W1_TC*)pTc0;
            sal_Bool bMerged2 = pTc2->fMergedGet();
            if( !bMerged2 ){
// and now the borders
                SvxBoxItem aBox( (SvxBoxItem&)rOut.GetCellAttr( RES_BOX ));
                SvxBorderLine aLine;
                W1_BRC10* pBrc = pTc2->rgbrcGet();
                for( sal_uInt16 j=0; j<4; j++ ){
                    aBox.SetLine(Ww1SingleSprmPBrc::SetBorder(&aLine, pBrc),
                                 nBrcTrans[j]);
                    pBrc++;
                }
                rOut.SetCellBorder( aBox, i - nCellsDeleted );
            }else{
// remove merged cells
                rOut.DeleteCell( i - nCellsDeleted );
                nCellsDeleted++;
            }
            pTc0+=sizeof(W1_TC);    // 10
        }
    }
}


void Ww1SingleSprmTDyaRowHeight::Start(
    Ww1Shell& rOut, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 /*nSize*/, Ww1Manager& /*rMan*/)
{
    short nSpace = SVBT16ToShort(pSprm);
    rOut.SetCellHeight(nSpace);
}

// I don't need Fastsave attributes as dummies

void Ww1SingleSprmPpc::Start(
    Ww1Shell& rOut, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 /*nSize*/, Ww1Manager& rMan)
{
    sal_uInt8 nPpc = *pSprm;

    if (rOut.IsInTable())                       // PMW does not know Flys in tables
        return;

    RndStdIds eAnchor;          // binding

    switch ( ( nPpc & 0x30 ) >> 4 )     // Y binding determines Sw binding
    {
        case 0:
            eAnchor = FLY_AT_PARA;      // Vert Margin
            break;
        default:
            eAnchor = FLY_AT_PAGE;      // Vert Page or unknown
            break;                          // 2=Vert. Paragraph, 3=Use Default
    }

    if( !rOut.IsInFly() && rMan.IsInStyle() ){
        rOut.BeginFly( eAnchor );           // start StyleFly
    }else{
        rOut.SetFlyAnchor( eAnchor );       // set anchor
    }
}

void Ww1SingleSprmPDxaAbs::Start(
    Ww1Shell& rOut, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 /*nSize*/, Ww1Manager& rMan)
{
    short nXPos = SVBT16ToShort(pSprm);

    if( rMan.IsInStyle() && !rOut.IsInFly() ){
        rOut.BeginFly();                    // Fly ohne PPc-Attribut
    }

    sal_Int16 eHRel = text::RelOrientation::FRAME;
    sal_Int16 eHAlign = text::HoriOrientation::NONE;

    switch( nXPos ){                        // Special x positions ?
    case 0:
    case -12: eHAlign = text::HoriOrientation::NONE; nXPos = 0; break;   // Cheat: inside -> left
                            // actually text::HoriOrientation::LEFT, but then
                            // an outer margin moves the Fly
    case -4:  eHAlign = text::HoriOrientation::CENTER; nXPos = 0; break;
    case -8:                                           // right
    case -16: eHAlign = text::HoriOrientation::RIGHT; nXPos = 0; break;  // Cheat: outside -> right

    }
    rOut.SetFlyXPos( nXPos, eHRel, eHAlign );
}

void Ww1SingleSprmPDyaAbs::Start(
    Ww1Shell& rOut, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 /*nSize*/, Ww1Manager& /*rMan*/)
{
    short nYPos = SVBT16ToShort(pSprm);
    sal_Int16 eVRel = text::RelOrientation::FRAME;
    sal_Int16 eVAlign = text::VertOrientation::NONE;

    switch( nYPos ){                            // special y positions ?
    case -4:  eVAlign = text::VertOrientation::TOP; nYPos = 0; break;
    case -8:  eVAlign = text::VertOrientation::CENTER; nYPos = 0; break;
    case -12: eVAlign = text::VertOrientation::BOTTOM; nYPos = 0; break;

    }
    rOut.SetFlyYPos( nYPos, eVRel, eVAlign );
}

void Ww1SingleSprmPDxaWidth::Start(
    Ww1Shell& rOut, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 /*nSize*/, Ww1Manager& /*rMan*/)
{
    short nDxaWidth = SVBT16ToShort(pSprm);
    rOut.SetFlyFrmAttr( SwFmtFrmSize( ATT_VAR_SIZE, nDxaWidth, MINFLY ) );
}

void Ww1SingleSprmPFromText::Start(
    Ww1Shell& rOut, sal_uInt8 /*nId*/, sal_uInt8* pSprm, sal_uInt16 /*nSize*/, Ww1Manager& /*rMan*/)
{
    if( rOut.IsInFly() ){   // Appears outside of Fly as well, but then it
                            // obviously has no meaning. It definitely does
                            // not implicitly start a new Fly
        short nFromText = SVBT16ToShort(pSprm);

        SvxLRSpaceItem aLR( RES_LR_SPACE );
        aLR.SetTxtLeft( nFromText );
        aLR.SetRight( nFromText );
        rOut.SetFlyFrmAttr( aLR );

        rOut.SetFlyFrmAttr( SvxULSpaceItem( nFromText, nFromText, RES_UL_SPACE ) );
    }
}

#undef STOP1
#undef STOP2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
