/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 *  For LWP filter architecture prototype
 ************************************************************************/

#include <memory>

#include "lwpparastyle.hxx"
#include <lwpfilehdr.hxx>
#include <lwpoverride.hxx>
#include "lwpparaborderoverride.hxx"
#include "lwpbreaksoverride.hxx"
#include "lwpnumberingoverride.hxx"
#include "lwptaboverride.hxx"
#include "lwpbackgroundoverride.hxx"
#include <o3tl/safeint.hxx>
#include <xfilter/xfdefs.hxx>
#include <xfilter/xfparastyle.hxx>
#include <xfilter/xfborders.hxx>
#include <lwpfont.hxx>
#include <lwpfoundry.hxx>
#include "lwppiece.hxx"
#include "lwpshadow.hxx"
#include "lwpborderstuff.hxx"
#include "lwpmargins.hxx"
#include "lwptabrack.hxx"

#include "lwpsilverbullet.hxx"

LwpParaStyle::LwpParaStyle(LwpObjectHeader const & objHdr, LwpSvStream* pStrm) :
LwpTextStyle(objHdr, pStrm)
{
}

LwpParaStyle::~LwpParaStyle()
{
}

void LwpParaStyle::Read()
{
    LwpTextStyle::Read();

    if (LwpFileHeader::m_nFileRevision < 0x000B)
    {
        // read many overrides
        LwpAlignmentOverride    aAlignOverride;
        aAlignOverride.Read(m_pObjStrm.get());

        LwpSpacingOverride  aSpacingOverride;
        aSpacingOverride.Read(m_pObjStrm.get());

        LwpIndentOverride       aIndentOverride;
        aIndentOverride.Read(m_pObjStrm.get());

        LwpParaBorderOverride   aPBOverride;
        aPBOverride.Read(m_pObjStrm.get());

        LwpBreaksOverride   aBreaksOverride;
        aBreaksOverride.Read(m_pObjStrm.get());

        LwpNumberingOverride    aNumberingOverride;
        aNumberingOverride.Read(m_pObjStrm.get());

        LwpTabOverride      aTabOverride;
        aTabOverride.Read(m_pObjStrm.get());

    }
    else
    {
        m_AlignmentStyle.ReadIndexed(m_pObjStrm.get());
        m_SpacingStyle.ReadIndexed(m_pObjStrm.get());
        m_IndentStyle.ReadIndexed(m_pObjStrm.get());
        m_BorderStyle.ReadIndexed(m_pObjStrm.get());
        m_BreaksStyle.ReadIndexed(m_pObjStrm.get());
        m_NumberingStyle.ReadIndexed(m_pObjStrm.get());
        m_TabStyle.ReadIndexed(m_pObjStrm.get());

        m_KinsokuOptsOverride.Read(m_pObjStrm.get());
        m_BulletOverride.Read(m_pObjStrm.get());

        if (m_pObjStrm->CheckExtra())
        {
            m_BackgroundStyle.ReadIndexed(m_pObjStrm.get());
            m_pObjStrm->SkipExtra();
        }

    }
}

void LwpParaStyle::Apply(XFParaStyle *pParaStyle)
{
    assert(pParaStyle);

    LwpVirtualPiece *pPiece = nullptr;
    //alignment:
    pPiece = dynamic_cast<LwpVirtualPiece*>(m_AlignmentStyle.obj().get());
    if( pPiece )
    {
        LwpAlignmentOverride *pAlign = dynamic_cast<LwpAlignmentOverride*>(pPiece->GetOverride());
        if( pAlign )
                ApplyAlignment(pParaStyle,pAlign);
    }

    //don't known top and bottom indent now.
    pPiece = dynamic_cast<LwpVirtualPiece*>(m_IndentStyle.obj().get());
    if( pPiece )
    {
        LwpIndentOverride   *pIndent = dynamic_cast<LwpIndentOverride*>(pPiece->GetOverride());
        if( pIndent )
        {
            if (!m_BulletOverride.IsInValid())// for remove bullet indent in named bullet style
            {
                std::unique_ptr<LwpIndentOverride> pNewIndent(pIndent->clone());
                pNewIndent->SetMFirst(0);
                pNewIndent->SetMRest(0);
                ApplyIndent(nullptr, pParaStyle, pNewIndent.get());
            }
            else
                ApplyIndent(nullptr,pParaStyle,pIndent);
        }
    }
    //shadow & borders.
    pPiece = dynamic_cast<LwpVirtualPiece*>(m_BorderStyle.obj().get());
    if( pPiece )
    {
        LwpParaBorderOverride *pBorder = dynamic_cast<LwpParaBorderOverride*>(pPiece->GetOverride());
        if( pBorder )
        {
            ApplyParaBorder(pParaStyle, pBorder);
        }
    }

    pPiece = dynamic_cast<LwpVirtualPiece*>(m_SpacingStyle.obj().get());
    if (pPiece)
    {
        LwpSpacingOverride *pSpacing = dynamic_cast<LwpSpacingOverride*>(pPiece->GetOverride());
        if( pSpacing)
            ApplySpacing(nullptr,pParaStyle,pSpacing);
    }

    //paragraph background.
    pPiece = dynamic_cast<LwpVirtualPiece*>(m_BackgroundStyle.obj().get());
    if( pPiece )
    {
        LwpBackgroundOverride *pBack = dynamic_cast<LwpBackgroundOverride*>(pPiece->GetOverride());
        if( pBack )
        {
            LwpColor color = pBack->GetBackColor();
            XFColor aXFColor( color.To24Color() );
            pParaStyle->SetBackColor( aXFColor );
        }
    }

    //add tab style
    pPiece = dynamic_cast<LwpVirtualPiece*>(m_TabStyle.obj().get());
    if( pPiece  )
    {
        LwpTabOverride *pTab = dynamic_cast<LwpTabOverride*>(pPiece->GetOverride());
        if(pTab)
        {
            ApplyTab(pParaStyle,pTab);
        }
    }
    pPiece = dynamic_cast<LwpVirtualPiece*>(m_BreaksStyle.obj().get());
    if( pPiece  )
    {
        LwpBreaksOverride *pBreak = dynamic_cast<LwpBreaksOverride*>(pPiece->GetOverride());
        if(pBreak)
        {
            ApplyBreaks(pParaStyle,pBreak);
        }
    }

}

void LwpParaStyle::ApplySubBorder(LwpBorderStuff* pBorderStuff, LwpBorderStuff::BorderType eType, XFBorders* pXFBorders)
{
    enumXFBorder eXFBorderSide = enumXFBorderNone;
    switch (eType)
    {
    case LwpBorderStuff::LEFT:
        eXFBorderSide = enumXFBorderLeft;
        break;
    case LwpBorderStuff::RIGHT:
        eXFBorderSide = enumXFBorderRight;
        break;
    case LwpBorderStuff::TOP:
        eXFBorderSide = enumXFBorderTop;
        break;
    case LwpBorderStuff::BOTTOM:
        eXFBorderSide = enumXFBorderBottom;
        break;
    default:
        break;
    }

    LwpColor    aColor = pBorderStuff->GetSideColor(eType);
    float       fWidth = pBorderStuff->GetSideWidth(eType);
    sal_uInt16  nType = pBorderStuff->GetSideType(eType);

    switch (nType)
    {
    default://fall through!
    case 0x14: //single fall through!
    case 0x17: //treble
        pXFBorders->SetWidth(eXFBorderSide, fWidth);
        break;
    case 0x15: //double , fall through!
    case 0x16: //thick double
        pXFBorders->SetDoubleLine(eXFBorderSide);
        pXFBorders->SetWidthOuter(eXFBorderSide, static_cast<float>(fWidth*0.333));
        pXFBorders->SetWidthSpace(eXFBorderSide, static_cast<float>(fWidth*0.334));
        pXFBorders->SetWidthInner(eXFBorderSide, static_cast<float>(fWidth*0.333));
//      pXFBorders->SetWidth(eXFBorderSide, fWidth);
        break;
    case 0x18: //thick-thin
        pXFBorders->SetDoubleLine(eXFBorderSide);
        pXFBorders->SetWidthOuter(eXFBorderSide, static_cast<float>(fWidth*0.5));
        pXFBorders->SetWidthInner(eXFBorderSide, static_cast<float>(fWidth*0.25));
        pXFBorders->SetWidthSpace(eXFBorderSide, static_cast<float>(fWidth*0.25));
        break;
    case 0x19: //thin-thick
        pXFBorders->SetDoubleLine(eXFBorderSide);
        pXFBorders->SetWidthInner(eXFBorderSide, static_cast<float>(fWidth*0.7));
        pXFBorders->SetWidthOuter(eXFBorderSide, static_cast<float>(fWidth*0.15));
        pXFBorders->SetWidthSpace(eXFBorderSide, static_cast<float>(fWidth*0.15));
        break;
    }

    if (aColor.IsValidColor())
    {
        XFColor aXFColor(aColor.To24Color());
        pXFBorders->SetColor(eXFBorderSide, aXFColor );
    }
}

void LwpParaStyle::ApplyParaBorder(XFParaStyle* pParaStyle, LwpParaBorderOverride* pBorder)
{
    //convert LwpShadow:
    LwpShadow *pShadow = pBorder->GetShadow();
    if( pShadow )
    {
        LwpColor color = pShadow->GetColor();
        float   offsetX = pShadow->GetOffsetX();
        float   offsetY = pShadow->GetOffsetY();

        if( offsetX && offsetY && color.IsValidColor() )
        {
            XFColor aXFColor(color.To24Color());
            bool left = false;
            bool top = false;
            if( offsetX < 0 )
                left = true;
            if( offsetY < 0 )
                top = true;
            if( left )
            {
                if( top )
                    pParaStyle->SetShadow(enumXFShadowLeftTop,-offsetX,aXFColor);
                else
                    pParaStyle->SetShadow(enumXFShadowLeftBottom,-offsetX,aXFColor);
            }
            else
            {
                if( top )
                    pParaStyle->SetShadow(enumXFShadowRightTop,offsetX,aXFColor);
                else
                    pParaStyle->SetShadow(enumXFShadowRightBottom,offsetX,aXFColor);
            }
        }
    }

    //convert to XFBorders object:
    LwpBorderStuff  *pBorderStuff = pBorder->GetBorderStuff();
    if( pBorderStuff && pBorderStuff->GetSide() != 0 )
    {
        XFBorders   *pXFBorders = new XFBorders();
        pParaStyle->SetBorders(pXFBorders);

        LwpMargins* pMargins = pBorder->GetMargins();

        // apply 4 borders respectively
        LwpBorderStuff::BorderType pType[] = { LwpBorderStuff::LEFT, LwpBorderStuff::RIGHT,
            LwpBorderStuff::TOP, LwpBorderStuff::BOTTOM };
        float pMarginValue[4] = { 0.0, 0.0, 0.0, 0.0 };

        for (sal_uInt8 nC = 0; nC < 4; nC++)
        {
            if (pBorderStuff->HasSide(pType[nC]))
            {
                ApplySubBorder(pBorderStuff, pType[nC], pXFBorders);

                //get border spacing to text content
                if (pMargins)
                {
                    pMarginValue[nC] = static_cast<float>(pMargins->GetMarginsValue(nC));
                }
            }

        }

        //apply border spacing to text content
        pParaStyle->SetPadding(pMarginValue[0], pMarginValue[1], pMarginValue[2], pMarginValue[3]);

    }
}

void LwpParaStyle::ApplyBreaks(XFParaStyle* pParaStyle, LwpBreaksOverride* pBreaks)
{
    if (pBreaks->IsKeepWithNext())
    {
        pParaStyle->SetBreaks(enumXFBreakKeepWithNext);
    }
    if (pBreaks->IsPageBreakBefore())
    {
        pParaStyle->SetBreaks(enumXFBreakBefPage);
    }
    if (pBreaks->IsPageBreakAfter())
    {
        pParaStyle->SetBreaks(enumXFBreakAftPage);
    }
    if (pBreaks->IsColumnBreakBefore())
    {
        pParaStyle->SetBreaks(enumXFBreakBefColumn);
    }
    if (pBreaks->IsColumnBreakAfter())
    {
        pParaStyle->SetBreaks(enumXFBreakAftColumn);
    }
}

void LwpParaStyle::ApplyAlignment(XFParaStyle* pParaStyle, LwpAlignmentOverride* pAlign)
{
    enumXFAlignType alignType = enumXFAlignStart;
    LwpAlignmentOverride::AlignType type;

    type = pAlign->GetAlignType();
    pParaStyle->SetNumberRight(false);//to identify its align attribute
    switch(type)
    {
    case LwpAlignmentOverride::ALIGN_LEFT:
        alignType = enumXFAlignStart;
        break;
    case LwpAlignmentOverride::ALIGN_RIGHT:
        alignType = enumXFAlignEnd;
        break;
    case LwpAlignmentOverride::ALIGN_CENTER:
        alignType = enumXFAlignCenter;
        break;
    case LwpAlignmentOverride::ALIGN_NUMERICLEFT://if non-number in table,ALIGN_NUMERICLEFT/RIGHT are useless
        alignType = enumXFAlignStart;            //note by  1/28
        break;
    case LwpAlignmentOverride::ALIGN_JUSTIFY:
    case LwpAlignmentOverride::ALIGN_JUSTIFYALL:
        alignType = enumXFAlignJustify;
        break;
    case LwpAlignmentOverride::ALIGN_NUMERICRIGHT:
        pParaStyle->SetNumberRight(true);//to identify its align attribute
        alignType = enumXFAlignEnd;
        break;
    default:
        break;
    }
    pParaStyle->SetAlignType(alignType);
}

void LwpParaStyle::ApplyIndent(LwpPara* pPara, XFParaStyle* pParaStyle, LwpIndentOverride* pIndent)
{
    LwpPara* pParentPara;
    if (pPara)
        pParentPara = pPara->GetParent();
    else
        pParentPara = nullptr;

    std::unique_ptr<LwpIndentOverride> pTotalIndent(new LwpIndentOverride);
    if (pIndent->IsUseRelative() && pParentPara)
    {
        LwpIndentOverride* pParentIndent = pParentPara->GetIndent();
        if (!pParentIndent)
            return;
        pTotalIndent.reset(pIndent->clone());

        //for bullet only
        if (pPara && pPara->GetBulletFlag())
        {
            pTotalIndent->SetMAll(o3tl::saturating_add(pParentIndent->GetMAll(), pTotalIndent->GetMAll()));
            pTotalIndent->SetMRight(o3tl::saturating_add(pParentIndent->GetMRight(), pTotalIndent->GetMRight()));
            pParaStyle->SetMargins(LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(
                pTotalIndent->GetMAll())), pTotalIndent->GetRight());
            pPara->SetIndent(pTotalIndent.release());
            return;
        }
        sal_uInt16 relative = pParentIndent->GetRelative();

        sal_Int32 Amount = pParentIndent->GetMAll();

        if (relative == LwpIndentOverride::RELATIVE_FIRST)
            Amount = o3tl::saturating_add(Amount, pParentIndent->GetMFirst());
        else if (relative == LwpIndentOverride::RELATIVE_REST)
            Amount = o3tl::saturating_add(Amount, pParentIndent->GetMRest());
        pTotalIndent->SetMAll(o3tl::saturating_add(Amount, pTotalIndent->GetMAll()));
        pTotalIndent->SetMRight(o3tl::saturating_add(pParentIndent->GetMRight(), pTotalIndent->GetMRight()));

        pParaStyle->SetIndent(pTotalIndent->GetFirst());
        pParaStyle->SetMargins(pTotalIndent->GetLeft(), pTotalIndent->GetRight());
        pPara->SetIndent(pTotalIndent.release());

    }
    else
    {
        pTotalIndent.reset(pIndent->clone());
        if (pPara && pPara->GetBulletFlag())
        {
            pParaStyle->SetMargins(LwpTools::ConvertToMetric(
                LwpTools::ConvertFromUnits(pIndent->GetMAll())), pIndent->GetRight());
            pPara->SetIndent(pTotalIndent.release());
            return;
        }

        pParaStyle->SetIndent(pIndent->GetFirst());
        pParaStyle->SetMargins(pIndent->GetLeft(), pIndent->GetRight());
        if (pPara)
        {
            pPara->SetIndent(pTotalIndent.release());
        }
    }
}

void LwpParaStyle::ApplySpacing(LwpPara* pPara, XFParaStyle* pParaStyle, LwpSpacingOverride* pSpacing)
{
    LwpSpacingCommonOverride* spacing = pSpacing->GetSpacing();
    LwpSpacingCommonOverride* abovepara = pSpacing->GetAboveSpacing();
    LwpSpacingCommonOverride* belowpara = pSpacing->GetBelowSpacing();

    LwpSpacingCommonOverride::SpacingType type = spacing->GetType();
    sal_Int32 amount = spacing->GetAmount();
    sal_Int32 multiple = spacing->GetMultiple();
    enumLHType xftype;
    double height;

    switch(type)
    {
    case LwpSpacingCommonOverride::SPACING_DYNAMIC:
    {
    xftype = enumLHPercent;
    height = double(multiple)/65536L*100;
    pParaStyle->SetLineHeight(xftype,height);
    }
        break;
    case LwpSpacingCommonOverride::SPACING_LEADING:
    {
    xftype = enumLHSpace;
    height = LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(amount));
    pParaStyle->SetLineHeight(xftype,height);
    }
        break;
    case LwpSpacingCommonOverride::SPACING_CUSTOM:
    {
        xftype = enumLHHeight;
        height =  LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(sal_Int32(float(multiple)/65536L*amount)));
        pParaStyle->SetLineHeight(xftype,height);
    }
        break;
    case LwpSpacingCommonOverride::SPACING_NONE:
        break;
    }

//TO DO: Above Line need to be processed!!!!!!! what it means??????  1-26

    type = abovepara->GetType();
    amount = abovepara->GetAmount();
    multiple = abovepara->GetMultiple();
    double above_val =-1;
    switch(type)
    {
    case LwpSpacingCommonOverride::SPACING_DYNAMIC:
        break;
    case LwpSpacingCommonOverride::SPACING_LEADING:
        break;
    case LwpSpacingCommonOverride::SPACING_CUSTOM:
        above_val =  LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(sal_Int32(float(multiple)/65536L*amount)));
        break;
    case LwpSpacingCommonOverride::SPACING_NONE:
        break;
    }

    type = belowpara->GetType();
    amount = belowpara->GetAmount();
    multiple = belowpara->GetMultiple();
    double below_val=-1;
    switch(type)
    {
    case LwpSpacingCommonOverride::SPACING_DYNAMIC:
        break;
    case LwpSpacingCommonOverride::SPACING_LEADING:
        break;
    case LwpSpacingCommonOverride::SPACING_CUSTOM:
        below_val =  LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(sal_Int32(float(multiple)/65536L*amount)));
        break;
    case LwpSpacingCommonOverride::SPACING_NONE:
        break;
    }

    if (pPara)
    {
        if (below_val != -1)
            pPara->SetBelowSpacing(below_val);
        LwpPara* pPrePara = dynamic_cast<LwpPara*>(pPara->GetPrevious().obj().get());
        if (pPrePara && above_val != -1)
        {
            above_val += pPrePara->GetBelowSpacing();

        }

    }
    pParaStyle->SetMargins(-1,-1,above_val,below_val);
}

void LwpParaStyle::ApplyTab(XFParaStyle *pParaStyle, LwpTabOverride *pTabOverRide)
{
    LwpObjectID& rTabRackID = pTabOverRide->GetTabRackID();
    if(rTabRackID.IsNull())
    {
        return;
    }

    LwpTabRack* pTabRack = dynamic_cast<LwpTabRack*>(rTabRackID.obj().get());
    if(!pTabRack)
    {
        return;
    }

    pParaStyle->ClearTabStyles();
    //Get margin left value
    double dMarginLeft = pParaStyle->GetMargins().GetLeft();

    sal_uInt16 nNumTabs = pTabRack->GetNumTabs();
    for(sal_uInt16 nIndex=0; nIndex<nNumTabs; nIndex++)
    {
        //get tab type
        LwpTab* pTab = pTabRack->Lookup(nIndex);
        if(!pTab)
            return;

        enumXFTab eType = enumXFTabNone;
        LwpTab::TabType type = pTab->GetTabType();
        switch(type)
        {
        case LwpTab::TT_LEFT:
            eType = enumXFTabLeft;
            break;
        case LwpTab::TT_CENTER:
            eType = enumXFTabCenter;
            break;
        case LwpTab::TT_RIGHT:
            eType = enumXFTabRight;
            break;
        case LwpTab::TT_NUMERIC:
            eType = enumXFTabChar;
            break;
        }

        //get position
        sal_uInt32 nPos = pTab->GetPosition();
        //different feature between SODC and lwp, the tab length must minus the margin left of para.
        double fLen = LwpTools::ConvertFromUnitsToMetric(nPos) - dMarginLeft;

        //get leader type
        sal_Unicode cLeader = 0x00;
        LwpTab::LeaderType leader= pTab->GetLeaderType();
        switch(leader)
        {
        case LwpTab::TL_NONE:
            cLeader = 0x20;     //space
            break;
        case LwpTab::TL_HYPHEN: //'-'
            cLeader = 0xAD;
            break;
        case LwpTab::TL_DOT:    //'.'
            cLeader = 0x2E;
            break;
        case LwpTab::TL_LINE:   //'_'
            cLeader = 0x5F;
            break;
        }

        sal_Unicode cAlignChar = static_cast<sal_Unicode>(pTab->GetAlignChar());

        pParaStyle->AddTabStyle(eType,fLen,cLeader,cAlignChar);
    }

}

void LwpParaStyle::RegisterStyle()
{
    if (!m_pFoundry)
        throw std::runtime_error("missing Foundry");

    std::unique_ptr<XFParaStyle> xStyle(new XFParaStyle());

    //Set name
    OUString styleName = GetName().str();
    xStyle->SetStyleName(styleName);

    //Create font
    LwpFontManager& rFontMgr = m_pFoundry->GetFontManger();
    rtl::Reference<XFFont> pFont = rFontMgr.CreateFont(m_nFinalFontID);
    xStyle->SetFont(pFont);

    //Set other paragraph properties...

    Apply(xStyle.get());
    //Add style
    LwpStyleManager* pStyleMgr = m_pFoundry->GetStyleManager();
    pStyleMgr->AddStyle(GetObjectID(), std::move(xStyle));
}

LwpAlignmentOverride* LwpParaStyle::GetAlignment()
{
    if (m_AlignmentStyle.obj() == nullptr)
        return nullptr;

    LwpAlignmentPiece *pPiece = dynamic_cast<LwpAlignmentPiece*>(m_AlignmentStyle.obj().get());
    if (pPiece)
        return dynamic_cast<LwpAlignmentOverride*>(pPiece->GetOverride());
    return nullptr;
}

LwpIndentOverride* LwpParaStyle::GetIndent()
{
    if (m_IndentStyle.obj() == nullptr)
        return nullptr;

    LwpIndentPiece *pPiece = dynamic_cast<LwpIndentPiece*>(m_IndentStyle.obj().get());
    if (pPiece)
        return dynamic_cast<LwpIndentOverride*>(pPiece->GetOverride());
    return nullptr;
}

LwpSpacingOverride* LwpParaStyle::GetSpacing()
{
    if (m_SpacingStyle.obj() == nullptr)
        return nullptr;

    LwpSpacingPiece *pPiece = dynamic_cast<LwpSpacingPiece*>(m_SpacingStyle.obj().get());
    if (pPiece)
        return dynamic_cast<LwpSpacingOverride*>(pPiece->GetOverride());
    return nullptr;
}

LwpParaBorderOverride* LwpParaStyle::GetParaBorder() const
{
    if(m_BorderStyle.IsNull())
        return nullptr;

    LwpParaBorderPiece *pPiece = dynamic_cast<LwpParaBorderPiece*>(m_BorderStyle.obj(VO_PARABORDERPIECE).get());
    if (pPiece)
        return dynamic_cast<LwpParaBorderOverride*>(pPiece->GetOverride());
    return nullptr;
}

LwpBreaksOverride* LwpParaStyle::GetBreaks() const
{
    if(m_BreaksStyle.IsNull())
        return nullptr;

    LwpBreaksPiece *pPiece = dynamic_cast<LwpBreaksPiece*>(m_BreaksStyle.obj(VO_BREAKSPIECE).get());
    if (pPiece)
        return dynamic_cast<LwpBreaksOverride*>(pPiece->GetOverride());
    return nullptr;
}


LwpNumberingOverride* LwpParaStyle::GetNumberingOverride() const
{
    if(m_NumberingStyle.IsNull())
        return nullptr;

    LwpNumberingPiece *pPiece = dynamic_cast<LwpNumberingPiece*>(m_NumberingStyle.obj(VO_NUMBERINGPIECE).get());
    if (pPiece)
        return dynamic_cast<LwpNumberingOverride*>(pPiece->GetOverride());
    return nullptr;
}

LwpTabOverride* LwpParaStyle::GetTabOverride() const
{
    if(m_TabStyle.obj() == nullptr)
        return nullptr;
    LwpTabPiece *pPiece = dynamic_cast<LwpTabPiece*>(m_TabStyle.obj().get());
    if (pPiece)
        return dynamic_cast<LwpTabOverride*>(pPiece->GetOverride());
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
