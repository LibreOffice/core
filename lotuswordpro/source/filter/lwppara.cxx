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

#include "lwppara.hxx"
#include <lwpglobalmgr.hxx>
#include <lwpfilehdr.hxx>
#include "lwppagelayout.hxx"
#include "lwpparaproperty.hxx"
#include "lwpparastyle.hxx"
#include <xfilter/xfstylemanager.hxx>
#include <xfilter/xfparagraph.hxx>
#include <xfilter/xfmargins.hxx>
#include <xfilter/xfsection.hxx>
#include <xfilter/xfsectionstyle.hxx>
#include <xfilter/xfcolor.hxx>
#include <xfilter/xfliststyle.hxx>
#include "lwpfribheader.hxx"
#include "lwplayout.hxx"
#include "lwpbulletstylemgr.hxx"
#include "lwpstory.hxx"
#include "lwpsilverbullet.hxx"

#include <lwpdropcapmgr.hxx>
#include <memory>
#include <o3tl/sorted_vector.hxx>

LwpPara::LwpPara(LwpObjectHeader const & objHdr, LwpSvStream* pStrm)
    : LwpDLVList(objHdr, pStrm)
    , m_nOrdinal(0)
    , m_nFlags(0)
    , m_nLevel(0)
    , m_FontID(0)
    , m_AllText(u""_ustr)
    , m_bHasBullet(false)
    , m_pSilverBullet(nullptr)
    , m_bBullContinue(false)
    , m_SectionStyleName(u""_ustr)
    , m_bHasDropcap(false)
    , m_nLines(0)
    , m_nChars(0)
    , m_pDropcapLayout(nullptr)
    , m_BelowSpacing(0)
{
}

LwpPara::~LwpPara()
{
}

void LwpPara::Read()
{
    LwpDLVList::Read();

    bool Simple;
    bool Notify = false;
    if(LwpFileHeader::m_nFileRevision<0x0006)
        Simple = false;
    else if(LwpFileHeader::m_nFileRevision<0x000B)
        Simple = m_pObjStrm->QuickReaduInt8() != 0;
    else
    {
        sal_uInt8 Flag = m_pObjStrm->QuickReaduInt8();

        const int DISK_SIMPLE = 1;
        const int DISK_NOTIFY = 2;

        Simple = (Flag & DISK_SIMPLE) != 0;
        Notify = (Flag & DISK_NOTIFY) != 0;
    }

    if(!Simple)
    {
        m_nOrdinal = m_pObjStrm->QuickReaduInt32();
        if(LwpFileHeader::m_nFileRevision<0x000B)
        {
            assert(false);
        }
        else
        {
            if (Notify)
            {
                LwpForked3NotifyList aNotifyList;
                aNotifyList.GetExtraList().Read(m_pObjStrm.get());
                aNotifyList.Read(m_pObjStrm.get());
            }
        }
    }
    else
        m_nOrdinal = 0x0001;

    m_nFlags = m_pObjStrm->QuickReaduInt16();
    m_ParaStyle.ReadIndexed(m_pObjStrm.get());

    if(!Simple)
    {
        m_Hint.Read(m_pObjStrm.get());
    }

    m_Story.ReadIndexed(m_pObjStrm.get());
    if(!Simple)
    {
        if(LwpFileHeader::m_nFileRevision<0x000B)
        {
            // TODO: to process
            assert(false);
        }
        m_nLevel = m_pObjStrm->QuickReaduInt16();

        if (m_nLevel > 9)
        {
            m_nLevel = 9;
        }
    }
    else
        m_nLevel = 0x0001;

    m_Fribs.SetPara(this);// for silver bullet
    m_Fribs.ReadPara(m_pObjStrm.get());

    ReadPropertyList(m_pObjStrm.get());
}

void LwpPara::Parse(IXFStream* pOutputStream)
{
    m_xXFContainer.set(new XFContentContainer);
    XFConvert(m_xXFContainer.get());
    if (!m_xXFContainer)
        return;
    m_xXFContainer->ToXml(pOutputStream);
    m_xXFContainer->Reset();
    m_xXFContainer.clear();
}

void LwpPara::XFConvert(XFContentContainer* pCont)
{
    m_xXFContainer.set(pCont);

    LwpStory *pStory = dynamic_cast<LwpStory*>(m_Story.obj().get());

    if (pStory && pStory->GetDropcapFlag())
    {
        ParseDropcapContent();
        return;
    }

    //Add the break before para
    if (m_pBreaks && m_nOrdinal!=0)
        AddBreakBefore(pCont);

    //Create an XFPara for this VO_PARA
    rtl::Reference<XFParagraph> xPara(new XFParagraph);
    xPara->SetStyleName(m_StyleName);

    if(!m_SectionStyleName.isEmpty())
    {
        rtl::Reference<XFSection> xSection(CreateXFSection());
        if (pStory)
            pStory->AddXFContent(xSection.get());
        m_xXFContainer = xSection;
    }

    if (m_bHasBullet && m_pSilverBullet)
    {
        rtl::Reference<XFContentContainer> xListItem = AddBulletList(m_xXFContainer.get());
        if (xListItem)
        {
            xListItem->Add(xPara.get());
        }
    }
    else if (m_xXFContainer)
    {
        LwpBulletStyleMgr* pBulletStyleMgr = GetBulletStyleMgr();
        if (pBulletStyleMgr)
        {
            pBulletStyleMgr->SetCurrentSilverBullet(LwpObjectID());
            pBulletStyleMgr->SetContinueFlag(false);
        }
        m_xXFContainer->Add(xPara.get());
    }

    m_Fribs.SetXFPara(xPara.get());
    m_Fribs.XFConvert();

    if (m_pBreaks && m_xXFContainer)
        AddBreakAfter(m_xXFContainer.get());
}

void LwpPara::RegisterMasterPage(XFParaStyle const * pBaseStyle)
{
    //get story
    LwpStory* pStory = dynamic_cast<LwpStory*>(m_Story.obj().get());
    //if pagelayout is modified, register the pagelayout
    if(!(pStory && pStory->IsPMModified()))
        return;

    bool bNewSection = pStory->IsNeedSection();
    LwpPageLayout* pLayout = pStory->GetCurrentLayout();
    if(bNewSection)
    {
        RegisterNewSectionStyle(pLayout);
    }

    //register master page style
    std::unique_ptr<XFParaStyle> xOverStyle(new XFParaStyle);
    *xOverStyle = *pBaseStyle;
    xOverStyle->SetStyleName( u""_ustr);
    xOverStyle->SetMasterPage(pLayout->GetStyleName());
    if (!m_ParentStyleName.isEmpty())
        xOverStyle->SetParentStyleName(m_ParentStyleName);
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_StyleName = pXFStyleManager->AddStyle(std::move(xOverStyle)).m_pStyle->GetStyleName();
}
/**
 * @short   register paragraph style
 */
void LwpPara::RegisterStyle()
{ //1 reg autostyle
//  m_Fribs.SetPara(this);
//  m_Fribs.RegisterStyle();

  //2 reg para style
    if (!m_pFoundry)
        return;
    XFParaStyle* pBaseStyle = dynamic_cast<XFParaStyle*>(m_pFoundry->GetStyleManager()->GetStyle(m_ParaStyle));
    if (pBaseStyle == nullptr) return;
    m_StyleName = pBaseStyle->GetStyleName();//such intf to be added
    m_ParentStyleName = m_StyleName;
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();

    LwpParaStyle* pParaStyle = GetParaStyle();
    if (pParaStyle && pParaStyle->GetIndent())
    {
        std::unique_ptr<LwpIndentOverride> pIndentOverride(GetParaStyle()->GetIndent()->clone());
        m_pIndentOverride = std::move(pIndentOverride);
    }

    std::unique_ptr<XFParaStyle> xOverStyle;
    bool noSpacing = true;
    LwpParaProperty* pBulletProps = nullptr, *pNumberingProps = nullptr;

    if (!m_vProps.empty())
    {
        bool noIndent = true;
        xOverStyle.reset(new XFParaStyle);
        *xOverStyle = *pBaseStyle;
        xOverStyle->SetStyleName(u""_ustr);
        sal_uInt32 PropType;
        LwpParaStyle& rParaStyle = dynamic_cast<LwpParaStyle&>(*m_ParaStyle.obj());
        for (auto & pProps : m_vProps)
        {
            PropType = pProps->GetType();
            switch(PropType)
            {
            case PP_LOCAL_ALIGN:
            {
                LwpAlignmentOverride *pAlignment = static_cast<LwpParaAlignProperty*>(pProps.get())->GetAlignment();
                if (pAlignment)
                {
                    if (!rParaStyle.GetAlignment())
                        OverrideAlignment(nullptr, pAlignment, xOverStyle.get());
                    else
                    {
                        std::unique_ptr<LwpAlignmentOverride> const pAlign(
                                rParaStyle.GetAlignment()->clone());
                        OverrideAlignment(pAlign.get(),
                                pAlignment,
                                xOverStyle.get());
                    }
                }
                break;
            }
            case PP_LOCAL_INDENT:
            {
                noIndent = false;
                LwpIndentOverride *pIndent = static_cast<LwpParaIndentProperty*>(pProps.get())->GetIndent();
                if (pIndent)
                {
                    if (!rParaStyle.GetIndent())
                        OverrideIndent(nullptr, pIndent, xOverStyle.get());
                    else
                        OverrideIndent(m_pIndentOverride.get(), pIndent, xOverStyle.get());
                }
                break;
            }
            case PP_LOCAL_SPACING:
            {
                noSpacing = false;
                LwpSpacingOverride *pSpacing = static_cast<LwpParaSpacingProperty*>(pProps.get())->GetSpacing();
                if (pSpacing)
                {
                    if (!rParaStyle.GetSpacing())
                        OverrideSpacing(nullptr, pSpacing, xOverStyle.get());
                    else
                    {
                        std::unique_ptr<LwpSpacingOverride> const
                            pNewSpacing(rParaStyle.GetSpacing()->clone());
                        OverrideSpacing(pNewSpacing.get(), pSpacing, xOverStyle.get());
                    }
                }
                break;
            }
            case PP_LOCAL_BORDER:
            {
                OverrideParaBorder(pProps.get(), xOverStyle.get());
                break;
            }
            case PP_LOCAL_BREAKS:
            {
                OverrideParaBreaks(pProps.get(), xOverStyle.get());
                break;
            }
            case PP_LOCAL_BULLET:
            {
                pBulletProps = pProps.get();
                break;
            }
            case PP_LOCAL_NUMBERING:
            {
                pNumberingProps = pProps.get();
                break;
            }
            case PP_LOCAL_TABRACK:
            {
                break;
            }
            case PP_LOCAL_BACKGROUND:
            {
                LwpBackgroundOverride* pBGOver = static_cast<LwpParaBackGroundProperty*>(pProps.get())->GetBackground();
                if (pBGOver)
                {
                    LwpBackgroundStuff& rBGStuff = pBGOver->GetBGStuff();
                    if (!rBGStuff.IsTransparent() )
                    {
                        if (rBGStuff.IsPatternFill())
                        {
                            std::unique_ptr<XFBGImage> xXFBGImage(rBGStuff.GetFillPattern());
                            xOverStyle->SetBackImage(xXFBGImage);
                        }
                        else
                        {
                            LwpColor* pColor = rBGStuff.GetFillColor();
                            if (pColor && pColor->IsValidColor())
                            {
                                XFColor aXFColor( pColor->To24Color());
                                xOverStyle->SetBackColor( aXFColor );
                            }
                        }
                    }
                }
                break;
            }
            default:
                break;
            }
        }

        if (noIndent && m_pIndentOverride)
        {
            if (m_pIndentOverride->IsUseRelative() && GetParent())
            {
                OverrideIndent(nullptr,m_pIndentOverride.get(),xOverStyle.get());
            }
        }
        if (!m_ParentStyleName.isEmpty())
            xOverStyle->SetParentStyleName(m_ParentStyleName);
        m_StyleName = pXFStyleManager->AddStyle(std::move(xOverStyle)).m_pStyle->GetStyleName();

    }
    else //use named style
    {
        if (m_pIndentOverride)
        {
            if (m_pIndentOverride->IsUseRelative() && GetParent())
            {
                xOverStyle.reset(new XFParaStyle);
                *xOverStyle = *pBaseStyle;
                OverrideIndent(nullptr,m_pIndentOverride.get(),xOverStyle.get());
                if (!m_ParentStyleName.isEmpty())
                    xOverStyle->SetParentStyleName(m_ParentStyleName);
                m_StyleName = pXFStyleManager->AddStyle(std::move(xOverStyle)).m_pStyle->GetStyleName();
            }
        }
    }

    if (IsInCell())
    {
        XFParaStyle* pOldStyle = pXFStyleManager->FindParaStyle(m_StyleName);
        if (pOldStyle->GetNumberRight())
        {
            xOverStyle.reset(new XFParaStyle);
            *xOverStyle = *pOldStyle;
            xOverStyle->SetAlignType(enumXFAlignStart);
            if (!m_ParentStyleName.isEmpty())
                xOverStyle->SetParentStyleName(m_ParentStyleName);
            m_StyleName = pXFStyleManager->AddStyle(std::move(xOverStyle)).m_pStyle->GetStyleName();
        }
    }

    // override bullet and numbering
    OverrideParaBullet(pBulletProps);
    OverrideParaNumbering(pNumberingProps);

    //register bullet style
    LwpBulletStyleMgr* pBulletStyleMgr = GetBulletStyleMgr();
    if (pBulletStyleMgr)
    {
        // if has bullet or numbering
        if (m_bHasBullet)
        {
            //if it's normal bullet
            if (m_pSilverBullet)
            {
                if (m_pSilverBullet->HasName())
                {
                    m_aBulletStyleName = m_pSilverBullet->GetBulletStyleName();
                }
                else if (!m_xBullOver->IsEditable())
                {
                    m_aBulletStyleName = pBulletStyleMgr->RegisterBulletStyle(this, m_xBullOver.get(), m_pIndentOverride.get());
                }

                // test codes
                if (m_pSilverBullet->IsBulletOrdered())
                {
                    OUString aPreBullStyleName;
                    LwpNumberingOverride* pNumbering = GetParaNumbering();
                    sal_uInt16 nPosition = pNumbering->GetPosition();
                    bool bLesser = m_pSilverBullet->IsLesserLevel(nPosition);
                    LwpPara* pPara = this;
                    LwpPara* pPrePara = nullptr;
                    sal_uInt16 nNum = 0, nLevel = 0, nFoundLevel = 0xffff, nFoundBound = 0;

                    nFoundBound = nLevel = pNumbering->GetLevel();
                    if (nPosition == pNumbering->GetPosition())
                    {
                        nFoundBound++;
                    }
                    bool bHeading = pNumbering->IsHeading();

                    o3tl::sorted_vector<LwpPara*> aSeen;
                    while(true)
                    {
                        bool bAlreadySeen = !aSeen.insert(pPara).second;
                        if (bAlreadySeen)
                            throw std::runtime_error("loop in conversion");
                        LwpSilverBullet* pParaSilverBullet = pPara->GetSilverBullet();
                        pNumbering = pPara->GetParaNumbering();

                        if (pPara->GetObjectID() != GetObjectID())
                        {
                            if (!pParaSilverBullet)
                            {
                                break;
                            }

                            /* If lesser, stop when we hit an outline style whose level is
                                * higher than our current level.
                                */
                            // restart based on Outline level?
                            if (pNumbering && bLesser && (!bHeading || pNumbering->IsHeading()))
                            {
                                if (nFoundLevel != 0xffff)
                                {
                                    if (pNumbering->GetLevel() < nFoundLevel)
                                    {
                                        break;
                                    }
                                    if ((pNumbering->GetLevel() == nFoundLevel)
                                        && (pParaSilverBullet->GetObjectID() != m_pSilverBullet->GetObjectID()
                                            || pNumbering->GetPosition() != nPosition))
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    if (pNumbering->GetLevel() < nFoundBound
                                        &&  (pParaSilverBullet->GetObjectID() != m_pSilverBullet->GetObjectID()
                                            || pNumbering->GetPosition() != nPosition))
                                    {
                                        nFoundBound = pNumbering->GetLevel();
                                    }
                                }
                            }
                        }

                        // Don't bump the number if this bullet is skipped
                        if (m_xBullOver->IsSkip())
                            ;
                        else if ( pParaSilverBullet
                            && pParaSilverBullet->GetObjectID() == m_pSilverBullet->GetObjectID()
                            && pNumbering && nPosition == pNumbering->GetPosition())
                        {
                            if (bLesser)
                            {
                                if (nFoundLevel != 0xffff)
                                {
                                    if (nFoundLevel == pNumbering->GetLevel())
                                    {
                                        aPreBullStyleName = pPara->GetBulletStyleName();
                                        nNum++;
                                    }
                                }
                                else if (pNumbering->GetLevel() <= nLevel)
                                {
                                    if (pNumbering->GetLevel() >= nFoundBound)
                                    {
                                        break;
                                    }
                                    nFoundLevel = pNumbering->GetLevel();
                                    aPreBullStyleName = pPara->GetBulletStyleName();
                                    nNum++;
                                }
                            }
                            else
                            {
                                aPreBullStyleName = pPara->GetBulletStyleName();
                                nNum++;
                            }
                        }

                        pPrePara = dynamic_cast<LwpPara*>(pPara->GetPrevious().obj(VO_PARA).get());

                        if (!pPrePara)
                        {
                            LwpStory* pStory = pPara->GetStory();
                            pPrePara = pStory ? pStory->GetLastParaOfPreviousStory() : nullptr;

                            if (!pPrePara)
                            {
                                break;
                            }
                        }
                        pPara = pPrePara;
                    }
                    nNum = nNum ? nNum : 1;

                    if (nNum > 1)
                    {
                        m_aBulletStyleName = aPreBullStyleName;
                        m_bBullContinue = true;
                    }
                    else
                    {
                        m_bBullContinue = false;
                        if (IsInCell())
                        {
                            XFListStyle* pOldStyle = static_cast<XFListStyle*>(pXFStyleManager->FindStyle(m_aBulletStyleName));
                            if (pOldStyle)
                            {
                                std::unique_ptr<XFListStyle> xNewStyle(new XFListStyle(*pOldStyle));
                                m_aBulletStyleName = pXFStyleManager->AddStyle(std::move(xNewStyle)).m_pStyle->GetStyleName();
                            }
                        }
                    }

                    LwpStory* pMyStory = GetStory();
                    if (pMyStory)
                    {
                        if (pMyStory->IsBullStyleUsedBefore(m_aBulletStyleName, m_xParaNumbering->GetPosition()))
                        {
                            //m_bBullContinue = sal_True;
                        }
                        else
                        {
                            pMyStory->AddBullStyleName2List(m_aBulletStyleName, m_xParaNumbering->GetPosition());
                        }
                    }

                }
                // end of test codes
            }
        }
    }

    if (noSpacing)
    {
        LwpPara* pPrePara = dynamic_cast<LwpPara*>(GetPrevious().obj().get());
        if (pPrePara && pPrePara->GetBelowSpacing()!=0)
        {
            xOverStyle.reset(new XFParaStyle);
            *xOverStyle = *GetXFParaStyle();
            XFMargins* pMargin = &xOverStyle->GetMargins();
            pMargin->SetTop(pMargin->GetTop()+pPrePara->GetBelowSpacing());
            if (!m_ParentStyleName.isEmpty())
                    xOverStyle->SetParentStyleName(m_ParentStyleName);
            m_StyleName = pXFStyleManager->AddStyle(std::move(xOverStyle)).m_pStyle->GetStyleName();
        }
    }

    //register tab style
    if(m_Fribs.HasFrib(FRIB_TAG_TAB))
    {
        std::unique_ptr<XFParaStyle> xNewParaStyle(new XFParaStyle);
        *xNewParaStyle = *GetXFParaStyle();
        //xOverStyle->SetStyleName("");
        RegisterTabStyle(xNewParaStyle.get());
        if (!m_ParentStyleName.isEmpty())
            xNewParaStyle->SetParentStyleName(m_ParentStyleName);
        m_StyleName = pXFStyleManager->AddStyle(std::move(xNewParaStyle)).m_pStyle->GetStyleName();
    }

    //register master page;
    RegisterMasterPage(GetXFParaStyle());

    // reg auto style,lay here for pagebreak need overridden para style
    m_Fribs.SetPara(this);
    m_Fribs.RegisterStyle();

    if (m_bHasDropcap)
    {
        GatherDropcapInfo();
        std::unique_ptr<XFParaStyle> xStyle(new XFParaStyle);
        *xStyle = *GetXFParaStyle();
        xStyle->SetDropCap(m_nChars-1,m_nLines);
        if (!m_ParentStyleName.isEmpty())
            xStyle->SetParentStyleName(m_ParentStyleName);
        m_StyleName = pXFStyleManager->AddStyle(std::move(xStyle)).m_pStyle->GetStyleName();
    }
    // maybe useful for further version
    // deleted because Leader of Table is not supported in this version
    //AddTabStyleForTOC();
}

void LwpPara::RegisterNewSectionStyle(LwpPageLayout *pLayout)
{
    if( !pLayout )
        return;

    std::unique_ptr<XFSectionStyle> xSectStyle(new XFSectionStyle);
    XFColumns* pColumns = pLayout->GetXFColumns();
    if(pColumns)
    {
        xSectStyle->SetColumns(pColumns);
    }
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_SectionStyleName = pXFStyleManager->AddStyle(std::move(xSectStyle)).m_pStyle->GetStyleName();
}

rtl::Reference<XFSection> LwpPara::CreateXFSection()
{
    rtl::Reference<XFSection> xXFSection(new XFSection);
    xXFSection->SetStyleName(m_SectionStyleName);
    m_SectionStyleName.clear();
    return xXFSection;
}

/**************************************************************************
 * @descr:  register tab style
**************************************************************************/
void LwpPara::RegisterTabStyle(XFParaStyle* pXFParaStyle)
{
    LwpTabOverride aFinaOverride;
    LwpTabOverride* pBase = nullptr;
    //get the tabrack from the current layout
    LwpStory* pStory = dynamic_cast<LwpStory*>(m_Story.obj().get());
    LwpMiddleLayout* pLayout = pStory ? pStory->GetTabLayout() : nullptr;
    if(pLayout)
    {
        pBase = pLayout->GetTabOverride();
        if(pBase)
        {
            pBase->Override(&aFinaOverride);
        }
    }

    //get the tabrack from the base parastyle
    LwpParaStyle* pParaStyle =  GetParaStyle();
    pBase = pParaStyle->GetTabOverride();
    if(pBase)
    {
        pBase->Override(&aFinaOverride);
    }
    //get the tabrack from the local property
    pBase = GetLocalTabOverride();
    if(pBase)
    {
        pBase->Override(&aFinaOverride);
    }

    LwpParaStyle::ApplyTab(pXFParaStyle, &aFinaOverride);
}
/**
 * @short   parse dropcap text
 */
void LwpPara::ParseDropcapContent()
{
    if (!GetFoundry())
        return;
    XFParagraph* pDropcap = GetFoundry()->GetDropcapMgr().GetXFPara();
    if (pDropcap)
    {
        m_Fribs.SetXFPara(pDropcap);
        m_Fribs.XFConvert();
    }
}
/**
 * @short   add paragraph break attribute
 */
void LwpPara::AddBreakBefore(XFContentContainer* pCont)
{
    if (!m_pBreaks || !pCont)
        return;
    if (m_pBreaks->IsPageBreakBefore())
    {
        rtl::Reference<XFParagraph> xPara(new XFParagraph);
        xPara->SetStyleName(m_BefPageBreakName);
        pCont->Add(xPara.get());
    }
    else if (m_pBreaks->IsColumnBreakBefore())
    {
        rtl::Reference<XFParagraph> xPara(new XFParagraph);
        xPara->SetStyleName(m_BefColumnBreakName);
        pCont->Add(xPara.get());
    }
}

void LwpPara::AddBreakAfter(XFContentContainer* pCont)
{
    if (!m_pBreaks)
        return;
    if (m_pBreaks->IsPageBreakAfter())
    {
        rtl::Reference<XFParagraph> xPara(new XFParagraph);
        xPara->SetStyleName(m_AftPageBreakName);
        pCont->Add(xPara.get());
    }
    else if (m_pBreaks->IsColumnBreakAfter())
    {
        rtl::Reference<XFParagraph> xPara(new XFParagraph);
        xPara->SetStyleName(m_AftColumnBreakName);
        pCont->Add(xPara.get());
    }
}

LwpBulletStyleMgr* LwpPara::GetBulletStyleMgr()
{
    if (m_pFoundry)
    {
        return m_pFoundry->GetBulletStyleMgr();
    }

    return nullptr;
}

rtl::Reference<XFContentContainer> LwpPara::AddBulletList(XFContentContainer* pCont)
{
    LwpBulletStyleMgr* pBulletStyleMgr = GetBulletStyleMgr();
    if (!pBulletStyleMgr)
    {
        assert(false);
        return rtl::Reference<XFContentContainer>();
    }

    sal_uInt16 nLevel = m_nLevel;
    bool bOrdered = false;
    /*LwpStory* pMyStory =*/ GetStory();

    pBulletStyleMgr->SetContinueFlag(m_bBullContinue);

    if (m_pSilverBullet->IsBulletOrdered())
    {
        bOrdered = true;
    }
    if (m_pSilverBullet->HasName())
    {
        nLevel = m_xParaNumbering->GetPosition();
        m_nLevel = nLevel;//for get para level
    }

    return pBulletStyleMgr->AddBulletList(pCont, bOrdered, m_aBulletStyleName,
            nLevel, m_xBullOver->IsSkip());
}

LwpNumberingOverride* LwpPara::GetParaNumbering()
{
    return m_xParaNumbering.get();
}

void LwpForked3NotifyList::Read(LwpObjectStream* pObjStrm)
{
    m_PersistentList.Read(pObjStrm);
}

void LwpNotifyListPersistent::Read(LwpObjectStream* pObjStrm)
{
    m_Head.ReadIndexed(pObjStrm);
    pObjStrm->SkipExtra();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
