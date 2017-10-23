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

#include <boost/cast.hpp>

#include "lwppara.hxx"
#include <lwpglobalmgr.hxx>
#include <lwpfilehdr.hxx>
#include "lwpparaproperty.hxx"
#include <lwptools.hxx>
#include "lwpparastyle.hxx"
#include <xfilter/xffont.hxx>
#include <xfilter/xftextstyle.hxx>
#include <xfilter/xfstylemanager.hxx>
#include <xfilter/xfparagraph.hxx>
#include <xfilter/xftextcontent.hxx>
#include <xfilter/xftextspan.hxx>
#include <xfilter/xfmargins.hxx>
#include <xfilter/xftabstop.hxx>
#include <xfilter/xflinebreak.hxx>
#include <xfilter/xfsection.hxx>
#include <xfilter/xfsectionstyle.hxx>
#include <xfilter/xfcolor.hxx>
#include <xfilter/xfhyperlink.hxx>
#include "lwpcharsetmgr.hxx"
#include "lwpsection.hxx"
#include "lwplayout.hxx"
#include "lwpusewhen.hxx"

#include "lwpbulletstylemgr.hxx"
#include "lwpstory.hxx"
#include "lwpsilverbullet.hxx"
#include <xfilter/xflist.hxx>
#include <xfilter/xfframe.hxx>

#include "lwpdivinfo.hxx"
#include "lwpdoc.hxx"
#include "lwpholder.hxx"
#include "lwppagehint.hxx"

#include <lwpdropcapmgr.hxx>
#include "lwptable.hxx"
#include "lwpcelllayout.hxx"
#include "lwpframelayout.hxx"

// boost::polymorphic_downcast checks and reports (using assert), if the
// cast is incorrect (in debug builds).
using boost::polymorphic_downcast;

/**
 * @short   get text of paragraph
 */
OUString const & LwpPara::GetContentText(bool bAllText)
{
//  rFont = m_FontID;
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
void LwpPara::SetAllText(const OUString& sText)
{
    m_AllText+=sText;
}

/**
 * @short   set first frib content
 */
void LwpPara::SetFirstFrib(const OUString& Content,sal_uInt32 FontID)
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
void LwpPara::GatherDropcapInfo()
{
    m_nLines = m_pDropcapLayout->GetLines();
    m_nChars = m_pDropcapLayout->GetChars();
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
        pPara = dynamic_cast<LwpPara*>(GetPrevious().obj().get());
        while (pPara)
        {
            otherlevel = pPara->GetLevel();
            if ((otherlevel < level) || (otherlevel && (level == 0)))
                return pPara;
            pPara = dynamic_cast<LwpPara*>(pPara->GetPrevious().obj().get());
        }
    }
    return nullptr;
}

/**
 * @short:   Offer prefix, paranumber and suffix according to position.
 * @param:   nPosition index of wanted paranumbering in the style-list.
 * @param:   pParaNumbering a pointer to the structure which contains prefix, paranumber and
 *       suffix.
 */
void LwpPara::GetParaNumber(sal_uInt16 nPosition, ParaNumbering* pParaNumbering)
{
    if (nPosition > 9)
    {
        return;
    }
    sal_uInt16 nCurrentPos = 0;

    LwpFrib* pPreFrib = nullptr;
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
                            (pPreFrib->GetModifiers() && pPreFrib->GetModifiers()->aTxtAttrOverride.GetHideLevels() == nHideLevels))
                        {
                            pParaNumbering->pPrefix = static_cast<LwpFribText*>(pPreFrib);
                        }
                    }

                    //get para numbering
                    pParaNumbering->pParaNumber = static_cast<LwpFribParaNumber*>(pFrib);
                    pParaNumbering->nNumLevel = nHideLevels;

                    //get suffix text frib
                    pFrib = pFrib->GetNext();
                    if ( pFrib )
                    {
                        if( pFrib->GetType() == FRIB_TAG_TEXT )
                        {
                            if (
                                 (pFrib->GetNext() && pFrib->GetNext()->GetType() == FRIB_TAG_TEXT) ||
                                 (pFrib->GetModifiers() && pFrib->GetModifiers()->aTxtAttrOverride.GetHideLevels() == nHideLevels)
                               )
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
                    pFrib = pFrib->GetNext();
                    if ( pFrib )
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
        if (over)
            over->Override(base);
        LwpParaStyle::ApplySpacing(this,pOverStyle,base);
    }
    else
        LwpParaStyle::ApplySpacing(this,pOverStyle,over);
}

/**
 * @short:   Get parastyle object according to the objID.
 * @return:  pointer to the parastyle.
 */
LwpParaStyle* LwpPara::GetParaStyle()
{
    return dynamic_cast<LwpParaStyle*>(m_ParaStyle.obj(VO_PARASTYLE).get());
}

/**
 * @short:   Override paraborder style.
 * @param:   pProps pointer to the LwpParaProperty and we can get local breaks through it.
 * @param:   pOverStyle pointer to XFParaStyle which contains the parastyle for XFilter.
 */
void LwpPara::OverrideParaBorder(LwpParaProperty* pProps, XFParaStyle* pOverStyle)
{
    // get paraborder in parastyle
    LwpParaStyle* pParaStyle = GetParaStyle();
    if (!pParaStyle)
    {
        return;
    }

    LwpOverride* pBorder = pParaStyle->GetParaBorder();
    std::unique_ptr<LwpParaBorderOverride> pFinalBorder(
        pBorder
            ? polymorphic_downcast<LwpParaBorderOverride*>(pBorder->clone())
            : new LwpParaBorderOverride)
        ;

    // get local border
    pBorder = static_cast<LwpParaBorderProperty*>(pProps)->GetLocalParaBorder();
    if (pBorder)
    {
        std::unique_ptr<LwpParaBorderOverride> pLocalBorder(
                polymorphic_downcast<LwpParaBorderOverride*>(pBorder->clone()));
        pLocalBorder->Override(pFinalBorder.get());
    }

    LwpParaStyle::ApplyParaBorder(pOverStyle, pFinalBorder.get());
}
/**
 * @short:   Override parabreaks style.
 * @param:   pProps pointer to the LwpParaProperty and we can get local breaks through it.
 * @param:   pOverStyle pointer to XFParaStyle which contains the parastyle for XFilter.
 */
void LwpPara::OverrideParaBreaks(LwpParaProperty* pProps, XFParaStyle* pOverStyle)
{
    // get breaks in parastyle
    LwpParaStyle* pParaStyle = GetParaStyle();
    if (!pParaStyle)
    {
        return;
    }

    LwpOverride* pBreaks = pParaStyle->GetBreaks();
    std::unique_ptr<LwpBreaksOverride> pFinalBreaks(
        pBreaks
            ? polymorphic_downcast<LwpBreaksOverride*>(pBreaks->clone())
            : new LwpBreaksOverride)
        ;

    // get local breaks
    pBreaks = static_cast<LwpParaBreaksProperty*>(pProps)->GetLocalParaBreaks();
    if (pBreaks)
    {
        std::unique_ptr<LwpBreaksOverride> const pLocalBreaks(
                polymorphic_downcast<LwpBreaksOverride*>(pBreaks->clone()));
        pLocalBreaks->Override(pFinalBreaks.get());
    }

    // save the breaks
    delete m_pBreaks;
    m_pBreaks = pFinalBreaks.release();

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    if (m_pBreaks->IsKeepWithNext())
    {
        pOverStyle->SetBreaks(enumXFBreakKeepWithNext);
    }
    if (m_pBreaks->IsPageBreakBefore())
    {
        XFParaStyle* pStyle = new XFParaStyle();
        pStyle->SetBreaks(enumXFBreakAftPage);
        m_BefPageBreakName = pXFStyleManager->AddStyle(pStyle).m_pStyle->GetStyleName();
    }
    if (m_pBreaks->IsPageBreakAfter())
    {
        XFParaStyle* pStyle = new XFParaStyle();
        pStyle->SetBreaks(enumXFBreakAftPage);
        m_AftPageBreakName = pXFStyleManager->AddStyle(pStyle).m_pStyle->GetStyleName();
    }
    if (m_pBreaks->IsColumnBreakBefore())
    {
        XFParaStyle* pStyle = new XFParaStyle();
        pStyle->SetBreaks(enumXFBreakAftColumn);//tmp after, should change when layout read
        m_BefColumnBreakName = pXFStyleManager->AddStyle(pStyle).m_pStyle->GetStyleName();
    }
    if (m_pBreaks->IsColumnBreakAfter())
    {
        XFParaStyle* pStyle = new XFParaStyle();
        pStyle->SetBreaks(enumXFBreakAftColumn);
        m_AftColumnBreakName = pXFStyleManager->AddStyle(pStyle).m_pStyle->GetStyleName();
    }

//  pParaStyle->ApplyBreaks(pOverStyle, &aFinalBreaks);
}

/**
 * @short:   Override bullet styles.
 * @param:   pProps pointer to the LwpParaProperty and we can get local bullet through it.
 */
void LwpPara::OverrideParaBullet(LwpParaProperty* pProps)
{
    // get bulletoverride in parastyle
    LwpParaStyle* pParaStyle = GetParaStyle();
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
            m_bHasBullet = true;

            const LwpOverride* pBullet= pParaStyle->GetBulletOverride();
            std::unique_ptr<LwpBulletOverride> pFinalBullet(
                pBullet
                    ? polymorphic_downcast<LwpBulletOverride*>(pBullet->clone())
                    : new LwpBulletOverride)
                ;

            std::unique_ptr<LwpBulletOverride> const pLocalBullet2(pLocalBullet->clone());
            pLocalBullet2->Override(pFinalBullet.get());

            aSilverBulletID = pFinalBullet->GetSilverBullet();
            delete m_pBullOver;
            m_pBullOver = pFinalBullet.release();
            if (!aSilverBulletID.IsNull())
            {
                m_pSilverBullet = dynamic_cast<LwpSilverBullet*>(aSilverBulletID.obj(VO_SILVERBULLET).get());
                if (m_pSilverBullet)
                    m_pSilverBullet->SetFoundry(m_pFoundry);
            }

            m_aSilverBulletID = aSilverBulletID;
        }
    }
    else
    {
//      m_pBullOver = pParaStyle->GetBulletOverride();
        const LwpBulletOverride* pBullOver = pParaStyle->GetBulletOverride();
        if (pBullOver)
        {
            m_aSilverBulletID = pBullOver->GetSilverBullet();
            if (!m_aSilverBulletID.IsNull())
            {
                m_bHasBullet = true;

                m_pSilverBullet = dynamic_cast<LwpSilverBullet*>(m_aSilverBulletID.obj(VO_SILVERBULLET).get());
                if (m_pSilverBullet)
                    m_pSilverBullet->SetFoundry(m_pFoundry);
            }

            std::unique_ptr<LwpBulletOverride> pBulletOverride(pBullOver->clone());
            delete m_pBullOver;
            m_pBullOver = pBulletOverride.release();
        }
    }
}
/**
 * @short:   Override paranumbering properties.
 * @param:   pProps pointer to the LwpParaProperty and we can get local paranumbering through it.
 */
void LwpPara::OverrideParaNumbering(LwpParaProperty const * pProps)
{
    // get numbering override in parastyle
    LwpParaStyle* pParaStyle = GetParaStyle();
    if (!pParaStyle)
    {
        return;
    }

    LwpNumberingOverride* pParaNumbering = pParaStyle->GetNumberingOverride();
    std::unique_ptr<LwpNumberingOverride> pOver(new LwpNumberingOverride);
    //Override with the local numbering, if any
    if (pProps)
    {
        LwpNumberingOverride* pPropNumbering = static_cast<LwpParaNumberingProperty const *>(pProps)->GetLocalNumbering();
        if (pPropNumbering)
        {
            pOver.reset(pPropNumbering->clone());
        }
    }
    else
    {
        if (pParaNumbering)
        {
            pOver.reset(pParaNumbering->clone());
        }
    }

    if (m_nFlags & VALID_LEVEL)
    {
        pOver->OverrideLevel(m_nLevel);
    }

    m_pParaNumbering = std::move(pOver);
}

void LwpPara::FindLayouts()
{
    m_Fribs.SetPara(this);
    m_Fribs.FindLayouts();
    LwpPara* pNextPara = dynamic_cast<LwpPara*>(GetNext().obj().get());
    if(pNextPara)
    {
        pNextPara->FindLayouts();
    }
}

/**************************************************************************
 * @descr:  Get property according to the property type
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
    return nullptr;
}

/**************************************************************************
 * @descr:  Get local tab rack
**************************************************************************/
LwpTabOverride* LwpPara::GetLocalTabOverride()
{
    LwpParaProperty* pProp = GetProperty(PP_LOCAL_TABRACK);
    if(pProp)
    {
        return static_cast<LwpParaTabRackProperty*>(pProp)->GetTab();
    }
    return nullptr;
}

/**
* @descr:   Determined which para is earlier in position
*
*/
bool LwpPara::operator< (LwpPara const & Other)
{
    return m_nOrdinal < Other.m_nOrdinal;
}

/**
* @descr:  If the two layouts in the same para, compare which layout is earlied according to frib order
*
*/
bool LwpPara::ComparePagePosition(LwpVirtualLayout const * pPreLayout, LwpVirtualLayout const * pNextLayout)
{
    m_Fribs.SetPara(this);
    return m_Fribs.ComparePagePosition(pPreLayout, pNextLayout);
}

/**
 * @short   check paragraph in cell or not
 */
bool LwpPara::IsInCell()
{
    LwpStory *pStory = GetStory();
    if (!pStory)
        return false;
    rtl::Reference<LwpVirtualLayout> xLayout(pStory->GetLayout(nullptr));
    return xLayout.is() && xLayout->IsCell();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
