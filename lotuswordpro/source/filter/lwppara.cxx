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
/*************************************************************************
 * Change History
 Jan 2005           Created
 ************************************************************************/

#include "lwppara.hxx"
#include "lwpglobalmgr.hxx"
#include "lwpfilehdr.hxx"
#include "lwpparaproperty.hxx"
#include "lwptools.hxx"
#include "lwpparastyle.hxx"
#include "xfilter/xffont.hxx"
#include "xfilter/xftextstyle.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xfparagraph.hxx"
#include "xfilter/xftextcontent.hxx"
#include "xfilter/xftextspan.hxx"
#include "xfilter/xfmargins.hxx"
#include "xfilter/xftabstop.hxx"
#include "xfilter/xflinebreak.hxx"
#include "xfilter/xfsection.hxx"
#include "xfilter/xfsectionstyle.hxx"
#include "xfilter/xfcolor.hxx"
#include "xfilter/xfhyperlink.hxx"
#include "xfilter/xfliststyle.hxx"
#include "lwpcharsetmgr.hxx"
#include "lwpsection.hxx"
#include "lwplayout.hxx"
#include "lwpusewhen.hxx"
#include "lwpbulletstylemgr.hxx"
#include "lwpstory.hxx"
#include "lwpsilverbullet.hxx"
#include "xfilter/xflist.hxx"
#include "xfilter/xfframe.hxx"

#include "lwpdivinfo.hxx"
#include "lwpdoc.hxx"
#include "lwpholder.hxx"
#include "lwppagehint.hxx"

#include "lwpdropcapmgr.hxx"
#include "lwptable.hxx"

LwpPara::LwpPara(LwpObjectHeader& objHdr, LwpSvStream* pStrm)
    : LwpDLVList(objHdr, pStrm), m_pBreaks(NULL), m_pIndentOverride(NULL)
    , m_bHasBullet(sal_False), m_pSilverBullet(NULL)
    , /*m_pParaNumbering(NULL),*/ m_pBullOver(NULL)
    , m_bBullContinue(sal_False)
    , m_bHasDropcap(sal_False), m_nLines(0), m_nChars(0)
    , m_pDropcapLayout(NULL), m_BelowSpacing(0), m_pXFContainer(NULL)
{
    m_pProps = NULL;
    m_SectionStyleName = A2OUSTR("");
    m_AllText = A2OUSTR("");
}

LwpPara::~LwpPara()
{

    if (m_pBreaks)
    {
        delete m_pBreaks;
        m_pBreaks = NULL;
    }
/*  if (m_pParaNumbering)
    {
        delete m_pParaNumbering;
        m_pParaNumbering = NULL;
    }*/

    if (m_pBullOver)
    {
        delete m_pBullOver;
        m_pBullOver = NULL;
    }
    if(m_pIndentOverride)
    {
        delete m_pIndentOverride;
    }

    LwpParaProperty* pNextProp;
    while(m_pProps)
    {
        pNextProp = m_pProps->GetNext();
        delete m_pProps;
        m_pProps = pNextProp;
    }
}

void LwpPara::Read()
{
    LwpDLVList::Read();

    sal_Bool Simple;
    sal_Bool Notify = sal_False;
    if(LwpFileHeader::m_nFileRevision<0x0006)
        Simple = sal_False;
    else if(LwpFileHeader::m_nFileRevision<0x000B)
        Simple = m_pObjStrm->QuickReaduInt8();
    else
    {
        sal_uInt8 Flag = m_pObjStrm->QuickReaduInt8();

        const int DISK_SIMPLE = 1;
        const int DISK_NOTIFY = 2;

        Simple = (Flag & DISK_SIMPLE) ? sal_True : sal_False;
        Notify = (Flag & DISK_NOTIFY) ? sal_True : sal_False;
    }

    if(!Simple)
    {
        m_nOrdinal = m_pObjStrm->QuickReaduInt32();
        if(LwpFileHeader::m_nFileRevision<0x000B)
        {
            // TODO: to process
            assert(false);
            /*pCForked3NotifyList Notify = new CForked3NotifyList(this);
            Notify->GetExtraList()->QuickRead(pFile);
            Notify->QuickRead(pFile);
            if(Notify->GetExtraList()->IsEmpty() &&
                Notify->IsEmpty())
                delete Notify;
            else
                cNotifyList = Notify;*/
        }
        else
        {
            if (Notify)
            {
                LwpForked3NotifyList* pNotifyList = new LwpForked3NotifyList();
                pNotifyList->GetExtraList()->Read(m_pObjStrm);
                pNotifyList->Read(m_pObjStrm);
                delete pNotifyList;
            }
        }
    }
    else
        m_nOrdinal = 0x0001;

    m_nFlags = m_pObjStrm->QuickReaduInt16();
    m_ParaStyle.ReadIndexed(m_pObjStrm);

    if(!Simple)
    {
        m_Hint.Read(m_pObjStrm);
        if (IsBadHintsInFile())
        {
            // This hint is no good, so throw it away -- SDC 9/28/95
            m_Hint.Reset();
        }
    }

    m_Story.ReadIndexed(m_pObjStrm);
    if(!Simple)
    {
        if(LwpFileHeader::m_nFileRevision<0x000B)
        {
            // TODO: to process
            assert(false);
            /*PropList = new CParaPropListProperty(this);
            PropList->GetList()->QuickRead(pFile);
            if(PropList->GetList()->IsEmpty())
            {
                delete PropList;
                PropList = LNULL;
            }*/
        }
        m_nLevel = m_pObjStrm->QuickReaduInt16();

        // test
        if (m_nLevel > 9)
        {
            m_nLevel = 9;
        }
        // test ends
    }
    else
        m_nLevel = 0x0001;

    m_Fribs.SetPara(this);//add by  2/1, for silver bullet
    m_Fribs.ReadPara(m_pObjStrm);

    m_pProps = LwpParaProperty::ReadPropertyList(m_pObjStrm,this);
}

void LwpPara::Parse(IXFStream* pOutputStream)
{
    m_pXFContainer = new XFContentContainer;
    XFConvert(m_pXFContainer);
    m_pXFContainer->ToXml(pOutputStream);
    m_pXFContainer->Reset();
    delete m_pXFContainer;
    m_pXFContainer = NULL;
}

void LwpPara::XFConvert(XFContentContainer* pCont)
{
    m_pXFContainer = pCont;

    LwpStory *pStory = dynamic_cast<LwpStory*>(m_Story.obj());

    if (pStory && pStory->GetDropcapFlag() == sal_True)
    {
        ParseDropcapContent();
        return;
    }

    //Add the break before para
    if (m_pBreaks && m_nOrdinal!=0)
        AddBreakBefore(pCont);

    //Create an XFPara for this VO_PARA
    XFParagraph *pPara = new XFParagraph;
    pPara->SetStyleName(m_StyleName);

    if(!m_SectionStyleName.isEmpty())
    {
        XFSection* pSection = CreateXFSection();
        if (pStory)
            pStory->AddXFContent(pSection);
        //pSection->Add(pPara);
        m_pXFContainer = pSection;
    }

    if (m_bHasBullet &&  m_pSilverBullet)
    {
        XFContentContainer* pListItem = AddBulletList(m_pXFContainer);
        if (pListItem)
        {
            pListItem->Add(pPara);
        }
    }
    else
    {
        LwpBulletStyleMgr* pBulletStyleMgr = this->GetBulletStyleMgr();
        if (pBulletStyleMgr)
        {
            pBulletStyleMgr->SetCurrentSilverBullet(LwpObjectID());
            pBulletStyleMgr->SetContinueFlag(sal_False);
        }
        m_pXFContainer->Add(pPara);
    }

    m_Fribs.SetXFPara(pPara);
    m_Fribs.XFConvert();

    if (m_pBreaks)
        AddBreakAfter(m_pXFContainer);
}

sal_Bool LwpPara::RegisterMasterPage(XFParaStyle* pBaseStyle)
{
    sal_Bool bSuccess = sal_False;
    //get story
    LwpStory* pStory = dynamic_cast<LwpStory*>(m_Story.obj());
    //if pagelayout is modified, register the pagelayout
    if(pStory && pStory->IsPMModified())
    {
        sal_Bool bNewSection = pStory->IsNeedSection();
        LwpPageLayout* pLayout = pStory->GetCurrentLayout();
        if(bNewSection)
        {
            RegisterNewSectionStyle(pLayout);
        }

        bSuccess = sal_True;
        //register master page style
        XFParaStyle* pOverStyle = new XFParaStyle();
        *pOverStyle = *pBaseStyle;
        pOverStyle->SetStyleName( A2OUSTR(""));
        pOverStyle->SetMasterPage(pLayout->GetStyleName());
        if (!m_ParentStyleName.isEmpty())
                    pOverStyle->SetParentStyleName(m_ParentStyleName);
        XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
        m_StyleName = pXFStyleManager->AddStyle(pOverStyle)->GetStyleName();
    }
    return bSuccess;
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
    XFParaStyle* pBaseStyle = static_cast<XFParaStyle*>(m_pFoundry->GetStyleManager()->GetStyle(m_ParaStyle));
    if (pBaseStyle == NULL) return;
    m_StyleName = pBaseStyle->GetStyleName();//such intf to be added
    m_ParentStyleName = m_StyleName;
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();

    if (GetParaStyle()->GetIndent())
    {
        std::auto_ptr<LwpIndentOverride> pIndentOverride(GetParaStyle()->GetIndent()->clone());
        delete m_pIndentOverride;
        m_pIndentOverride = pIndentOverride.release();
    }

    XFParaStyle* pOverStyle = NULL;
    sal_Bool noSpacing = sal_True;
    sal_Bool noIndent = sal_True;
    LwpParaProperty* pBulletProps = NULL, *pNumberingProps = NULL;

    if (m_pProps != NULL)
    {
        pOverStyle = new XFParaStyle;
        *pOverStyle = *pBaseStyle;
        pOverStyle->SetStyleName(A2OUSTR(""));
        LwpParaProperty* pProps = m_pProps;
        sal_uInt32 PropType;
        LwpParaStyle* pParaStyle = dynamic_cast<LwpParaStyle*>(m_ParaStyle.obj());
        while(pProps)
        {
            PropType = pProps->GetType();
            switch(PropType)
            {
            case PP_LOCAL_ALIGN:
            {
                if (!pParaStyle->GetAlignment())
                    OverrideAlignment(NULL,static_cast<LwpParaAlignProperty*>(pProps)->GetAlignment(),pOverStyle);
                else
                {
                    boost::scoped_ptr<LwpAlignmentOverride> const pAlign(
                            pParaStyle->GetAlignment()->clone());
                    OverrideAlignment(pAlign.get(),
                            static_cast<LwpParaAlignProperty*>(pProps)->GetAlignment(),
                            pOverStyle);
                }
            }
                break;
            case PP_LOCAL_INDENT:
            {
                noIndent = sal_False;
                if (!pParaStyle->GetIndent())
                    OverrideIndent(NULL,static_cast<LwpParaIndentProperty*>(pProps)->GetIndent(),pOverStyle);

                else
                {
                    OverrideIndent(m_pIndentOverride,static_cast<LwpParaIndentProperty*>(pProps)->GetIndent(),pOverStyle);
                }
            }
                break;
            case PP_LOCAL_SPACING:
            {
                noSpacing = sal_False;
                if (!pParaStyle->GetSpacing())
                    OverrideSpacing(NULL,static_cast<LwpParaSpacingProperty*>(pProps)->GetSpacing(),pOverStyle);
                else
                {
                    boost::scoped_ptr<LwpSpacingOverride> const
                        pSpacing(pParaStyle->GetSpacing()->clone());
                    OverrideSpacing(pSpacing.get(),
                            static_cast<LwpParaSpacingProperty*>(pProps)->GetSpacing(),
                            pOverStyle);
                }
            }
                break;
        // 01/25/2005
        case PP_LOCAL_BORDER:
        {
            OverrideParaBorder(pProps, pOverStyle);
            break;
        }
        case PP_LOCAL_BREAKS:
        {
            OverrideParaBreaks(pProps, pOverStyle);
            break;
        }
        case PP_LOCAL_BULLET:
        {
            pBulletProps = pProps;
//          OverrideParaBullet(pProps);
            break;
        }
        case PP_LOCAL_NUMBERING:
        {
            pNumberingProps = pProps;
//          OverrideParaNumbering(pProps);
            break;
        }
        //end
        case PP_LOCAL_TABRACK:
        {
            //, 01/28/05
            /*LwpTabOverride* pTabOverride=pParaStyle->GetTabOverride();
            if(!pTabOverride)
            {
                OverrideTab(NULL,static_cast<LwpParaTabRackProperty*>(pProps)->GetTab(),pOverStyle);
            }
            else
            {
                OverrideTab(pTabOverride,static_cast<LwpParaTabRackProperty*>(pProps)->GetTab(),pOverStyle);
            }*/
            break;
        }
        case PP_LOCAL_BACKGROUND:
        {
/*          LwpBackgroundOverride aBackground;
            if (!pParaStyle->GetBackground())
                OverrideBackground(NULL,static_cast<LwpParaBackGroundProperty*>(pProps)->GetBackground(),pOverStyle);
            else
            {
                aBackground = *(pParaStyle->GetaBackground());
                OverrideBackground(&aBackground,static_cast<LwpParaBackGroundProperty*>(pProps)->GetBackground(),pOverStyle);
            }
*/
            // modified by , 06/03/2005
            LwpBackgroundOverride* pBGOver = static_cast<LwpParaBackGroundProperty*>(pProps)->GetBackground();
            if (pBGOver)
            {
                LwpBackgroundStuff* pBGStuff = pBGOver->GetBGStuff();
                if (pBGStuff && !pBGStuff->IsTransparent() )
                {
                    if (pBGStuff->IsPatternFill())
                    {
                        XFBGImage* pXFBGImage = pBGStuff->GetFillPattern();
                        pOverStyle->SetBackImage(pXFBGImage);
                    }
                    else
                    {
                        LwpColor* pColor = pBGStuff->GetFillColor();
                        if (pColor && pColor->IsValidColor())
                        {
                            XFColor aXFColor( pColor->To24Color());
                            pOverStyle->SetBackColor( aXFColor );
                        }
                    }
                }
            }
            // end modified
                    break;
        }
                default:
                    break;
                }
            pProps = pProps->GetNext();
        }

        if (noIndent && m_pIndentOverride)
            {
                if (m_pIndentOverride->IsUseRelative() && GetParent())
                {
                OverrideIndent(NULL,m_pIndentOverride,pOverStyle);
                }
            }
            if (!m_ParentStyleName.isEmpty())
                pOverStyle->SetParentStyleName(m_ParentStyleName);
            m_StyleName = pXFStyleManager->AddStyle(pOverStyle)->GetStyleName();

    }
    else //use named style
    {
        if (m_pIndentOverride)
            {
                if (m_pIndentOverride->IsUseRelative() && GetParent())
                {
                    pOverStyle = new XFParaStyle;
                    *pOverStyle = *pBaseStyle;
                    OverrideIndent(NULL,m_pIndentOverride,pOverStyle);
                    if (!m_ParentStyleName.isEmpty())
                        pOverStyle->SetParentStyleName(m_ParentStyleName);
                    m_StyleName = pXFStyleManager->AddStyle(pOverStyle)->GetStyleName();
                }
            }
    }

    if (IsInCell())
    {
        XFParaStyle* pOldStyle = pXFStyleManager->FindParaStyle(m_StyleName);
        if (pOldStyle->GetNumberRight())
        {
            pOverStyle = new XFParaStyle;
            *pOverStyle = *pOldStyle;
            pOverStyle->SetAlignType(enumXFAlignStart);
            if (!m_ParentStyleName.isEmpty())
                pOverStyle->SetParentStyleName(m_ParentStyleName);
            m_StyleName = pXFStyleManager->AddStyle(pOverStyle)->GetStyleName();
        }
    }

    // override bullet and numbering
    OverrideParaBullet(pBulletProps);
    OverrideParaNumbering(pNumberingProps);

    //add by
    //register bullet style
    LwpBulletStyleMgr* pBulletStyleMgr = this->GetBulletStyleMgr();
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
                else if (!m_pBullOver->IsEditable())
                {
                    m_aBulletStyleName = pBulletStyleMgr->RegisterBulletStyle(this, m_pBullOver, m_pIndentOverride);
                }

                // test codes
                if (m_pSilverBullet->IsBulletOrdered())
                {
                    OUString aPreBullStyleName;
                    LwpNumberingOverride* pNumbering = this->GetParaNumbering();
                    sal_uInt16 nPosition = pNumbering->GetPosition();
                    sal_Bool bLesser = m_pSilverBullet->IsLesserLevel(nPosition);
                    /*sal_Bool bResetSection =*/ m_pSilverBullet->IsNewSection(nPosition);
                    sal_Bool bHeading;
                    LwpPara* pPara = this;
                    LwpPara* pPrePara = NULL;
                    LwpSilverBullet* pParaSilverBullet = NULL;
                    sal_uInt16 nNum = 0, nLevel = 0, nFoundLevel = 0xffff, nFoundBound = 0;

                    nFoundBound = nLevel = pNumbering->GetLevel();
                    if (nPosition == pNumbering->GetPosition())
                    {
                        nFoundBound++;
                    }
                    bHeading = pNumbering->IsHeading();

                    while(true)
                    {
                        /*// When we hit the hint paragraph, we can stop and check the hint.
                        if (qNumberHint && (qPara == qNumberHint->GetPara()) &&
                            qNumberHint->Lookup(qSilverBullet, Level, Position, &Offset))
                        {
                            Num += Offset;
                            break;
                        }*/

                        pParaSilverBullet = pPara->GetSilverBullet();
                        pNumbering = pPara->GetParaNumbering();

                        if (*(pPara->GetObjectID()) != *(this->GetObjectID()))
                        {
                            if (!pParaSilverBullet)
                            {
                                break;
                            }

                            /* If lesser, stop when we hit an outline style whose level is
                                * higher than our current level.
                                */
                            // restart based on Outline level?
                            if (pNumbering && bLesser && (bHeading ? pNumbering->IsHeading() : sal_True))
                            {
                                if (nFoundLevel != 0xffff)
                                {
                                    if (pNumbering->GetLevel() < nFoundLevel)
                                    {
                                        break;
                                    }
                                    if ((pNumbering->GetLevel() == nFoundLevel)
                                        && (*(pParaSilverBullet->GetObjectID()) != *(m_pSilverBullet->GetObjectID())
                                            || pNumbering->GetPosition() != nPosition))
                                    {
                                        break;
                                    }
                                }
                                else
                                {
                                    if (pNumbering && pNumbering->GetLevel() < nFoundBound && pParaSilverBullet
                                        &&  (*(pParaSilverBullet->GetObjectID()) != *(m_pSilverBullet->GetObjectID())
                                            || pNumbering->GetPosition() != nPosition))
                                    {
                                        nFoundBound = pNumbering->GetLevel();
                                    }
                                }
                            }

                            /*if (qSpecificStyle
                            && qSpecificStyle == qPara->GetParaStyle(LTRUE))
                                break;

                            // See if we crossed a section boundary
                            if (ResetSection)
                            {
                                CurrPos.SetPara(qPara);
                                if (CurrPos <= SectionPos)
                                    break;
                            }*/
                        }

                        // Don't bump the number if this bullet is skipped
                        if (m_pBullOver->IsSkip())
                            ;
                        else if ( pParaSilverBullet
                            && *(pParaSilverBullet->GetObjectID()) == *(m_pSilverBullet->GetObjectID())
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

                        if (pPara && pPara->GetPrevious())
                            pPrePara = dynamic_cast<LwpPara*>(pPara->GetPrevious()->obj(VO_PARA));
                        else
                            pPrePara=NULL;

                        if (!pPrePara)
                        {
                            LwpStory* pStory = pPara->GetStory();
                            pPrePara = pStory->GetLastParaOfPreviousStory();

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
                        m_bBullContinue = sal_True;
                    }
                    else
                    {
                        m_bBullContinue = sal_False;
                        if (this->IsInCell())
                        {
                            XFListStyle* pOldStyle = static_cast<XFListStyle*>(pXFStyleManager->FindStyle(m_aBulletStyleName));
                            if (pOldStyle)
                            {
                                XFListStyle* pNewStyle = new XFListStyle(*pOldStyle);
                                m_aBulletStyleName = pXFStyleManager->AddStyle(pNewStyle)->GetStyleName();
                            }
                        }
                    }

                    LwpStory* pMyStory = this->GetStory();
                    if (pMyStory)
                    {
                        if (pMyStory->IsBullStyleUsedBefore(m_aBulletStyleName, m_pParaNumbering->GetPosition()))
                        {
                            //m_bBullContinue = sal_True;
                        }
                        else
                        {
                            pMyStory->AddBullStyleName2List(m_aBulletStyleName, m_pParaNumbering->GetPosition());
                        }
                    }

                }
                // end of test codes
            }
        }
    }
    //end add

    if (noSpacing && GetPrevious())
    {
        LwpPara* pPrePara = dynamic_cast<LwpPara*>(GetPrevious()->obj());
        if (pPrePara && pPrePara->GetBelowSpacing()!=0)
        {
            pOverStyle = new XFParaStyle;
            *pOverStyle = *GetXFParaStyle();
            XFMargins* pMargin = &pOverStyle->GetMargins();
            pMargin->SetTop(pMargin->GetTop()+pPrePara->GetBelowSpacing());
            if (!m_ParentStyleName.isEmpty())
                    pOverStyle->SetParentStyleName(m_ParentStyleName);
            m_StyleName = pXFStyleManager->AddStyle(pOverStyle)->GetStyleName();
        }
    }

    //register tab style
    if(m_Fribs.HasFrib(FRIB_TAG_TAB))
    {
        XFParaStyle* pParaStyle = new XFParaStyle;
        *pParaStyle = *GetXFParaStyle();
        //pOverStyle->SetStyleName(A2OUSTR(""));
        this->RegisterTabStyle(pParaStyle);
        if (!m_ParentStyleName.isEmpty())
                    pParaStyle->SetParentStyleName(m_ParentStyleName);
        m_StyleName = pXFStyleManager->AddStyle(pParaStyle)->GetStyleName();
    }

    //register master page;
    RegisterMasterPage(GetXFParaStyle());

    // reg auto style,lay here for pagebreak need overrided para style
    m_Fribs.SetPara(this);
    m_Fribs.RegisterStyle();

    if (m_bHasDropcap == sal_True)
    {
        GatherDropcapInfo();
        XFParaStyle* pStyle = new XFParaStyle;
        *pStyle = *GetXFParaStyle();
        pStyle->SetDropCap(m_nChars-1,m_nLines);
        if (!m_ParentStyleName.isEmpty())
                    pStyle->SetParentStyleName(m_ParentStyleName);
        m_StyleName = pXFStyleManager->AddStyle(pStyle)->GetStyleName();
    }
    // maybe useful for futer version
    // deleted because Leader of Table is not supported in this version
    //AddTabStyleForTOC();
}

void LwpPara::RegisterNewSectionStyle(LwpPageLayout *pLayout)
{
    if( !pLayout )
        return;

    XFSectionStyle* pSectStyle= new XFSectionStyle();
    XFColumns* pColumns = pLayout->GetXFColumns();
    if(pColumns)
    {
        pSectStyle->SetColumns(pColumns);
    }
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_SectionStyleName = pXFStyleManager->AddStyle(pSectStyle)->GetStyleName();
}

XFSection* LwpPara::CreateXFSection()
{
    XFSection* pXFSection = new XFSection();
    pXFSection->SetStyleName(m_SectionStyleName);
    m_SectionStyleName = A2OUSTR("");
    return pXFSection;
}

/**************************************************************************
 * @descr:  register tab style
 * @param:
 * @param:
 * @return:
**************************************************************************/
void LwpPara::RegisterTabStyle(XFParaStyle* pXFParaStyle)
{
    LwpTabOverride aFinaOverride;
    LwpTabOverride* pBase = NULL;
    //get the tabrack from the current layout
    LwpStory* pStory = dynamic_cast<LwpStory*>(m_Story.obj());
    LwpMiddleLayout* pLayout = pStory ? pStory->GetTabLayout() : NULL;
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
    XFParagraph* pDropcap = GetFoundry()->GetDropcapMgr()->GetXFPara();
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
    if (!m_pBreaks)
        return;
    if (m_pBreaks->IsPageBreakBefore())
    {
        XFParagraph *pPara = new XFParagraph();
        pPara->SetStyleName(m_BefPageBreakName);
        pCont->Add(pPara);
    }
    else if (m_pBreaks->IsColumnBreakBefore())
    {
        XFParagraph *pPara = new XFParagraph();
        pPara->SetStyleName(m_BefColumnBreakName);
        pCont->Add(pPara);
    }
}

void LwpPara::AddBreakAfter(XFContentContainer* pCont)
{
    if (!m_pBreaks)
        return;
    if (m_pBreaks->IsPageBreakAfter())
    {
        XFParagraph *pPara = new XFParagraph();
        pPara->SetStyleName(m_AftPageBreakName);
        pCont->Add(pPara);
    }
    else if (m_pBreaks->IsColumnBreakAfter())
    {
        XFParagraph *pPara = new XFParagraph();
        pPara->SetStyleName(m_AftColumnBreakName);
        pCont->Add(pPara);
    }
}

LwpBulletStyleMgr* LwpPara::GetBulletStyleMgr()
{
    if (m_pFoundry)
    {
        return m_pFoundry->GetBulletStyleMgr();
    }

    return NULL;
}

XFContentContainer* LwpPara::AddBulletList(XFContentContainer* pCont)
{
    LwpBulletStyleMgr* pBulletStyleMgr = GetBulletStyleMgr();
    if (!pBulletStyleMgr)
    {
        assert(false);
        return NULL;
    }

    sal_uInt16 nLevel = m_nLevel;
    sal_Bool bOrdered = sal_False;
    /*LwpStory* pMyStory =*/ GetStory();

    pBulletStyleMgr->SetContinueFlag(m_bBullContinue);

    if (m_pSilverBullet->IsBulletOrdered())
    {
        bOrdered = sal_True;
    }
    if (m_pSilverBullet->HasName())
    {
        nLevel = m_pParaNumbering->GetPosition();
        m_nLevel = nLevel;//add by ,for get para level
//      m_aBulletStyleName = m_pSilverBullet->GetBulletStyleName();
    }

    return ( pBulletStyleMgr->AddBulletList(pCont, bOrdered, m_aBulletStyleName,
            nLevel, m_pBullOver->IsSkip()) );
}

LwpNumberingOverride* LwpPara::GetParaNumbering()
{
    return m_pParaNumbering.get();
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

void LwpPara::Release()
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
