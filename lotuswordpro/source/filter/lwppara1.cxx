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
 Jan 2005			Created
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
#include "lwpcelllayout.hxx"
/**
 * @short   get text of paragraph
 */
OUString LwpPara::GetContentText(sal_Bool bAllText)
{
//	rFont = m_FontID;
    if (bAllText)
    {
        m_Fribs.SetPara(this);
        m_Fribs.GatherAllText();
        return m_AllText;
    }
    else
        return m_Content;
}

/**
 * @short   set text of paragraph
 */
void LwpPara::SetAllText(OUString sText)
{
    m_AllText+=sText;
}

/**
 * @short   set first frib content
 */
void LwpPara::SetFirstFrib(rtl::OUString Content,sal_uInt32 FontID)
{
    m_FontID= FontID;
    m_Content=Content;
}
/**
 * @short   get paragraph xfstyle
 */
XFParaStyle* LwpPara::GetXFParaStyle()
{
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    return pXFStyleManager->FindParaStyle(m_StyleName);
}
/**
 * @short   get drop cap info
 */
#include "lwpframelayout.hxx"
void LwpPara::GatherDropcapInfo()
{
    SetDropcapLines(m_pDropcapLayout->GetLines());
    SetDropcapChars(m_pDropcapLayout->GetChars());
}
/**
 * @short   get parent paragraph
 */
LwpPara* LwpPara::GetParent()
{
    LwpPara* pPara;
    sal_uInt16 otherlevel;
    sal_uInt16 level = GetLevel();

    if (level != 1)
    {
        pPara = static_cast<LwpPara*>(GetPrevious()->obj());
        while (pPara)
        {
            otherlevel = pPara->GetLevel();
            if ((otherlevel < level) || (otherlevel && (level == 0)))
                return pPara;
            pPara = static_cast<LwpPara*>(pPara->GetPrevious()->obj());
        }
    }
    return NULL;
}

/**
 * @short:   Offer prefix, paranumber and suffix according to position.
 * @param:   nPosition index of wanted paranumbering in the style-list.
 * @param:   pParaNumbering a pointer to the structure which contains prefix, paranumber and
 *		 suffix.
 */
void LwpPara::GetParaNumber(sal_uInt16 nPosition, ParaNumbering* pParaNumbering)
{
    if (nPosition > 9)
    {
        return;
    }
    sal_uInt16 nCurrentPos = 0;

    LwpFrib* pPreFrib = NULL;
    LwpFrib* pFrib = m_Fribs.GetFribs();
    if (!pFrib)
    {
        return;
    }

    while (pFrib)
    {
        sal_uInt8 nFribType = pFrib->GetType();
        if (nFribType == FRIB_TAG_PARANUMBER)
        {
            nCurrentPos++;
            ModifierInfo* pModInfo = pFrib->GetModifiers();
            if (pModInfo)
            {
                sal_uInt16 nHideLevels = pModInfo->aTxtAttrOverride.GetHideLevels();
                if (nCurrentPos == nPosition)
                {
                    //get prefix text frib
                    if (pPreFrib)
                    {
                        if ((pPreFrib->GetType() == FRIB_TAG_TEXT) &&
                            (pPreFrib->GetModifiers()->aTxtAttrOverride.GetHideLevels() == nHideLevels))
                        {
                            pParaNumbering->pPrefix = static_cast<LwpFribText*>(pPreFrib);
                        }
                    }

                    //get para numbering
                    pParaNumbering->pParaNumber = static_cast<LwpFribParaNumber*>(pFrib);
                    pParaNumbering->nNumLevel = nHideLevels;

                    //get suffix text frib
                    if ( (pFrib = pFrib->GetNext()) )
                    {
//						if((pFrib->GetType() == FRIB_TAG_TEXT) &&
//							(pFrib->GetModifiers()->aTxtAttrOverride.GetHideLevels() == nHideLevels))
                        if( pFrib->GetType() == FRIB_TAG_TEXT )
                        {
                            if ((pFrib->GetNext()->GetType() == FRIB_TAG_TEXT ) ||
                                (pFrib->GetModifiers()->aTxtAttrOverride.GetHideLevels() == nHideLevels))
                            {
                                pParaNumbering->pSuffix = static_cast<LwpFribText*>(pFrib);
                            }
                        }
                    }

                    break;
                }
            }
            else
            {
                if (nCurrentPos == nPosition)
                {
                    //get prefix text frib
                    if (pPreFrib)
                    {
                        if (pPreFrib->GetType() == FRIB_TAG_TEXT)
                        {
                            pParaNumbering->pPrefix = static_cast<LwpFribText*>(pPreFrib);
                        }
                    }

                    //get para numbering
                    pParaNumbering->pParaNumber = static_cast<LwpFribParaNumber*>(pFrib);

                    //get suffix text frib
                    if ( (pFrib = pFrib->GetNext()) )
                    {
                        if (pFrib->GetType() == FRIB_TAG_TEXT)
                        {
                            pParaNumbering->pSuffix = static_cast<LwpFribText*>(pFrib);
                        }
                    }

                }
            }
        }
        pPreFrib = pFrib;
        if (pFrib)
        {
            pFrib = pFrib->GetNext();
        }
    }
}
/**
 * @short   override alignment
 */
//override style, add by  1-24
void LwpPara::OverrideAlignment(LwpAlignmentOverride* base,LwpAlignmentOverride* over,XFParaStyle* pOverStyle)
{
    if (base)//the latter two parameter never be null
    {
        over->Override(base);
        LwpParaStyle::ApplyAlignment(pOverStyle,base);
    }
    else
        LwpParaStyle::ApplyAlignment(pOverStyle,over);
}
/**
 * @short   override indent attribute
 */
void LwpPara::OverrideIndent(LwpIndentOverride* base,LwpIndentOverride* over,XFParaStyle* pOverStyle)
{
    if (base)//the latter two parameter never be null
    {
            over->Override(base);
            LwpParaStyle::ApplyIndent(this,pOverStyle,base);
    }
    else
    {
            LwpParaStyle::ApplyIndent(this,pOverStyle,over);
        }
}
/**
 * @short   override spacing
 */
void LwpPara::OverrideSpacing(LwpSpacingOverride* base,LwpSpacingOverride* over,XFParaStyle* pOverStyle)
{
    if (base)//the latter two parameter never be null
    {
        over->Override(base);
        LwpParaStyle::ApplySpacing(this,pOverStyle,base);
    }
    else
        LwpParaStyle::ApplySpacing(this,pOverStyle,over);
}

//add by , 01/25/2005
/**
 * @short:   Get parastyle object according to the objID.
 * @return:  pointer to the parastyle.
 */
LwpParaStyle* LwpPara::GetParaStyle()
{
    return static_cast<LwpParaStyle*>(m_ParaStyle.obj(VO_PARASTYLE));
}

/**
 * @short:   Override paraborder style.
 * @param:   pProps pointer to the LwpParaProperty and we can get local breaks through it.
 * @param:   pOverStyle pointer to XFParaStyle which contains the parastyle for XFilter.
 */
void LwpPara::OverrideParaBorder(LwpParaProperty* pProps, XFParaStyle* pOverStyle)
{
    // get paraborder in parastyle
    LwpParaStyle* pParaStyle = this->GetParaStyle();
    if (!pParaStyle)
    {
        return;
    }

    LwpOverride* pBorder = pParaStyle->GetParaBorder();
    LwpParaBorderOverride aFinalBorder;
    if (pBorder)
    {
        aFinalBorder = *pBorder;
    }

    // get local border
    pBorder = static_cast<LwpParaBorderProperty*>(pProps)->GetLocalParaBorder();
    if (pBorder)
    {
        LwpParaBorderOverride aLocalBorder;
        aLocalBorder =  *pBorder;
        aLocalBorder.Override(&aFinalBorder);
    }

    pParaStyle->ApplyParaBorder(pOverStyle, &aFinalBorder);
}
/**
 * @short:   Override parabreaks style.
 * @param:   pProps pointer to the LwpParaProperty and we can get local breaks through it.
 * @param:   pOverStyle pointer to XFParaStyle which contains the parastyle for XFilter.
 */
void LwpPara::OverrideParaBreaks(LwpParaProperty* pProps, XFParaStyle* pOverStyle)
{
    // get breaks in parastyle
    LwpParaStyle* pParaStyle = this->GetParaStyle();
    if (!pParaStyle)
    {
        return;
    }

    LwpOverride* pBreaks = pParaStyle->GetBreaks();
    LwpBreaksOverride* pFinalBreaks = new LwpBreaksOverride();
    if (pBreaks)
    {
        *pFinalBreaks = *pBreaks;
    }

    // get local breaks
    pBreaks = static_cast<LwpParaBreaksProperty*>(pProps)->GetLocalParaBreaks();
    if (pBreaks)
    {
        LwpBreaksOverride aLocalBreaks;
        aLocalBreaks = *pBreaks;
        aLocalBreaks.Override(pFinalBreaks);
    }

    // save the breaks
    m_pBreaks = pFinalBreaks;

//add by  1/31
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    if (m_pBreaks->IsKeepWithNext())
    {
        pOverStyle->SetBreaks(enumXFBreakKeepWithNext);
    }
    if (m_pBreaks->IsPageBreakBefore())
    {
        XFParaStyle* pStyle = new XFParaStyle();
        pStyle->SetBreaks(enumXFBreakAftPage);
        m_BefPageBreakName = pXFStyleManager->AddStyle(pStyle)->GetStyleName();
    }
    if (m_pBreaks->IsPageBreakAfter())
    {
        XFParaStyle* pStyle = new XFParaStyle();
        pStyle->SetBreaks(enumXFBreakAftPage);
        m_AftPageBreakName = pXFStyleManager->AddStyle(pStyle)->GetStyleName();
    }
    if (m_pBreaks->IsColumnBreakBefore())
    {
        XFParaStyle* pStyle = new XFParaStyle();
        pStyle->SetBreaks(enumXFBreakAftColumn);//tmp after, should change when layout read,note by  1/31
        m_BefColumnBreakName = pXFStyleManager->AddStyle(pStyle)->GetStyleName();
    }
    if (m_pBreaks->IsColumnBreakAfter())
    {
        XFParaStyle* pStyle = new XFParaStyle();
        pStyle->SetBreaks(enumXFBreakAftColumn);
        m_AftColumnBreakName = pXFStyleManager->AddStyle(pStyle)->GetStyleName();
    }
//add end

//	pParaStyle->ApplyBreaks(pOverStyle, &aFinalBreaks);
}

/**
 * @short:   Override bullet styles.
 * @param:   pProps pointer to the LwpParaProperty and we can get local bullet through it.
 */
void LwpPara::OverrideParaBullet(LwpParaProperty* pProps)
{
    // get bulletoverride in parastyle
    LwpParaStyle* pParaStyle = this->GetParaStyle();
    if (!pParaStyle)
    {
        return;
    }

    if (pProps)
    {
        m_pBullOver = new LwpBulletOverride();
        // get local bulletoverride
        LwpBulletOverride* pLocalBullet  = static_cast<LwpParaBulletProperty*>(pProps)->GetLocalParaBullet();
        if (!pLocalBullet)
        {
            return;
        }

        LwpObjectID aSilverBulletID = pLocalBullet->GetSilverBullet();
        if (aSilverBulletID.IsNull())
        {
            return;
        }
        else
        {
            m_bHasBullet = sal_True;

            LwpOverride* pBullet= pParaStyle->GetBulletOverride();
            LwpBulletOverride aFinalBullet;
            if (pBullet)
            {
                aFinalBullet = *pBullet;
            }

            LwpBulletOverride aLocalBullet;
            aLocalBullet = *pLocalBullet;
            aLocalBullet.Override(&aFinalBullet);

            *m_pBullOver = aFinalBullet;
            aSilverBulletID = aFinalBullet.GetSilverBullet();
            if (!aSilverBulletID.IsNull())
            {
                m_pSilverBullet = static_cast<LwpSilverBullet*>(aSilverBulletID.obj(VO_SILVERBULLET));
                m_pSilverBullet->SetFoundry(m_pFoundry);
            }

            m_aSilverBulletID = aSilverBulletID;
        }
    }
    else
    {
//		m_pBullOver = pParaStyle->GetBulletOverride();
        LwpBulletOverride* pBullOver = pParaStyle->GetBulletOverride();
        if (pBullOver)
        {
            m_aSilverBulletID = pBullOver->GetSilverBullet();
            if (!m_aSilverBulletID.IsNull())
            {
                m_bHasBullet = sal_True;

                m_pSilverBullet = static_cast<LwpSilverBullet*>(m_aSilverBulletID.obj(VO_SILVERBULLET));
                m_pSilverBullet->SetFoundry(m_pFoundry);
            }

            m_pBullOver = new LwpBulletOverride();
            *m_pBullOver = *pBullOver;
        }
    }
}
/**
 * @short:   Override paranumbering properties.
 * @param:   pProps pointer to the LwpParaProperty and we can get local paranumbering through it.
 */
void LwpPara::OverrideParaNumbering(LwpParaProperty* pProps)
{
    // get numbering override in parastyle
    LwpParaStyle* pParaStyle = this->GetParaStyle();
    if (!pParaStyle)
    {
        return;
    }

    LwpNumberingOverride* pParaNumbering = pParaStyle->GetNumberingOverride();
    LwpNumberingOverride aOver;
    //Override with the local numbering, if any
    if (pProps)
    {
        LwpNumberingOverride* pPropNumbering = static_cast<LwpParaNumberingProperty*>(pProps)->GetLocalNumbering();
        if (pPropNumbering)
        {
            aOver = *pPropNumbering;
        }
    }
    else
    {
        if (pParaNumbering)
        {
            aOver = *pParaNumbering;
        }
    }

    if (m_nFlags & VALID_LEVEL)
    {
        aOver.OverrideLevel(m_nLevel);
    }

    m_aParaNumbering = aOver;

}

/**************************************************************************
 * @short:
 * @descr:
 * @param:
 * @param:
 * @return:
**************************************************************************/
void LwpPara::OverrideTab(LwpTabOverride *base, LwpTabOverride *over, XFParaStyle *pOverStyle)
{
    if (base)
    {
        over->Override(base);
        LwpParaStyle::ApplyTab(pOverStyle,base);
    }
    else
        LwpParaStyle::ApplyTab(pOverStyle,over);
}

void LwpPara::FindLayouts()
{
    m_Fribs.SetPara(this);
    m_Fribs.FindLayouts();
    LwpPara* pNextPara = static_cast<LwpPara*>(GetNext()->obj());
    if(pNextPara)
    {
        pNextPara->FindLayouts();
    }
}

/**************************************************************************
 * @descr:  Get property according to the property type
 * @param:
 * @param:
 * @return:
**************************************************************************/
LwpParaProperty* LwpPara::GetProperty(sal_uInt32 nPropType)
{
    LwpParaProperty* pProps = m_pProps;
    while(pProps)
    {
        if(pProps->GetType() == nPropType)
        {
            return pProps;
        }
        pProps = pProps->GetNext();

    }
    return NULL;
}

/**************************************************************************
 * @descr:  Get local tab rack
 * @param:
 * @param:
 * @return:
**************************************************************************/
LwpTabOverride* LwpPara::GetLocalTabOverride()
{
    LwpParaProperty* pProp = GetProperty(PP_LOCAL_TABRACK);
    if(pProp)
    {
        return static_cast<LwpParaTabRackProperty*>(pProp)->GetTab();
    }
    return NULL;
}

/**
* @descr:   Determined which para is earlier in position
*
*/
sal_Bool LwpPara::operator< (LwpPara& Other)
{
    return m_nOrdinal < Other.GetOrdinal();
}

/**
* @descr:  If the two layouts in the same para, compare which layout is earlied according to frib order
*
*/
sal_Bool LwpPara::ComparePagePosition(LwpVirtualLayout * pPreLayout, LwpVirtualLayout * pNextLayout)
{
    m_Fribs.SetPara(this);
    return m_Fribs.ComparePagePosition(pPreLayout, pNextLayout);
}
/*
sal_Bool LwpPara::IsNeedTabForTOC()
{
    LwpStory* pStory = GetStory();
    if(pStory)
    {
        if(GetNext()->IsNull())
        {
            LwpVirtualLayout* pLayout = pStory->GetLayout(NULL);
            if(pLayout && pLayout->IsCell())
            {
                LwpCellLayout * pCell = static_cast<LwpCellLayout *>(pStory->GetLayout(NULL));
                if (pCell->GetLeaderChar() == 0)
                {
                    return sal_False;
                }
                return sal_True;
            }
        }
    }
    return sal_False;
}

void LwpPara::AddTabStyleForTOC( )
{
    if(IsNeedTabForTOC())
    {
        XFParaStyle* pParaStyle = new XFParaStyle;
        *pParaStyle = *GetXFParaStyle();
        pParaStyle->ClearTabStyles();
        //Add Tab Style;
        enumXFTab eType = enumXFTabRight;

        LwpCellLayout * pCell = static_cast<LwpCellLayout *>(GetStory()->GetLayout(NULL));
        double dLen = pCell->GetActualWidth(); /// todo: get from table
        sal_Unicode cLeader = static_cast<sal_Unicode>(pCell->GetLeaderChar());
        pParaStyle->AddTabStyle(eType, dLen, cLeader );
        m_StyleName = XFStyleManager::AddStyle(pParaStyle)->GetStyleName();

        // Get font info of default text style and set into tab style
        XFParaStyle* pBaseStyle = static_cast<XFParaStyle*>(m_pFoundry->GetStyleManager()->GetStyle(*m_pFoundry->GetDefaultTextStyle()));
        XFTextStyle*pTextStyle = new XFTextStyle;
        pTextStyle->SetFont(pBaseStyle->GetFont()); // who delete this font?????
        m_TabStyleName = XFStyleManager::AddStyle(pTextStyle)->GetStyleName();
    }
}

void LwpPara::AddTabStopForTOC()
{
    if(IsNeedTabForTOC())
    {
        XFParagraph* pXFPara = GetFribs()->GetXFPara();
        pXFPara->SetStyleName(m_StyleName);

        XFParaStyle* pParaStyle = static_cast<XFParaStyle*>(XFStyleManager::FindStyle(m_StyleName));
        if(pParaStyle)
        {
            XFTextSpan *pSpan = new XFTextSpan;
            XFTabStop *pTab = new XFTabStop;
            pSpan->Add(pTab);
            pSpan->SetStyleName(m_TabStyleName);
            enumXFAlignType eType = pParaStyle->GetAlighType();

            if(eType == enumXFAlignStart || eType == enumXFAlignJustify || eType == enumXFAlignNone)
            {
                pXFPara->Add(pSpan);
            }
            else if(eType == enumXFAlignEnd)
            {
                pXFPara->InsertAtBegin(pSpan);
            }
            else
            {
                delete pSpan; // pTabl will be delete inside XFTextSpan
            }

        }

    }
}
*/
/**
 * @short   check paragraph alignment
 */
sal_Bool LwpPara::IsNumberRight()
{
    LwpObject* pObj = m_ParaStyle.obj();
    if (pObj)
    {
        LwpParaStyle* pStyle = static_cast<LwpParaStyle*>(pObj);
        return pStyle->IsNumberRight();
    }
    else
        return sal_False;
}
/**
 * @short   check paragraph in cell or not
 */
sal_Bool LwpPara::IsInCell()
{
    LwpVirtualLayout* pLayout = GetStory()->GetLayout(NULL);
    if(pLayout && pLayout->IsCell())
        return sal_True;
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
