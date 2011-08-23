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
#include "lwpbulletstylemgr.hxx"
#include "lwpdoc.hxx"
#include "lwpstory.hxx"
#include "lwpdivinfo.hxx"
#include "lwppara.hxx"
#include "lwpsilverbullet.hxx"
#include "lwptools.hxx"
#include "lwpparaproperty.hxx"
#include "xfilter/xfliststyle.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xflist.hxx"
#include "lwpglobalmgr.hxx"

LwpBulletStyleMgr::LwpBulletStyleMgr() :
m_pBulletList(NULL), m_bContinue(sal_True), m_bIsBulletSkipped(sal_False), m_nCurrentPos(0xFF)
{
}

LwpBulletStyleMgr::~LwpBulletStyleMgr()
{
    if (m_pBulletList)
    {
        delete m_pBulletList;
    }

    m_vIDsPairList.clear();
    m_vStyleNameList.clear();
}

/**
 * @short   Register bullet style to style-list. The function only register the bullet and single customized numbering
 *		not inluding the numbering sequence.
 * @param   pPara pointer to the current paragraph which has a bullet/numbering.
 * @param   pBullOver pointer to the bulletoverride of current paragraph.
 * @param   pIndent pointer to the indentoverride of current paragraph.
 */
rtl::OUString LwpBulletStyleMgr::RegisterBulletStyle(LwpPara* pPara, LwpBulletOverride* pBullOver,
    LwpIndentOverride* pIndent)
{
    if(!pPara || !pIndent || !pBullOver)
    {
//		assert(false);
        return rtl::OUString::createFromAscii("");
    }

    LwpSilverBullet* pSilverBullet = pPara->GetSilverBullet();
    if (!pSilverBullet)
    {
        assert(false);
        return rtl::OUString::createFromAscii("");
    }

    LwpPara* pBulletPara = pSilverBullet->GetBulletPara();
    if (!pBulletPara)
    {
        assert(false);
        return rtl::OUString::createFromAscii("");
    }

    LwpParaProperty* pProp = pPara->GetProperty(PP_LOCAL_INDENT);
    LwpParaIndentProperty* pIndentProp = NULL;
    LwpObjectID aIndentID;
    if (pProp)
    {
        pIndentProp = static_cast<LwpParaIndentProperty*>(pProp);
        aIndentID = pIndentProp->GetIndentID();
    }

    LwpObjectID aBulletID = pBullOver->GetSilverBullet();
    LwpBulletOverride aBulletOver = *pBullOver;

    sal_uInt16 nNameIndex = 0;
    std::vector <OverridePair>::iterator iter;
    for(iter = m_vIDsPairList.begin(); iter != m_vIDsPairList.end(); iter++)
    {
        if (iter->first.GetSilverBullet() == aBulletID && iter->second == aIndentID
            && iter->first.IsRightAligned() == pBullOver->IsRightAligned())
        {
            return m_vStyleNameList[nNameIndex];
        }
        else
        {
            nNameIndex++;
        }
    }

    m_vIDsPairList.push_back(std::make_pair(aBulletOver, aIndentID));
    rtl::OUString aStyleName;

    LwpFribPtr* pBulletParaFribs = pBulletPara->GetFribs();
    sal_Bool bIsNumbering = (sal_Bool)(pBulletParaFribs->HasFrib(FRIB_TAG_PARANUMBER) != 0);

    enumXFAlignType eAlign = enumXFAlignStart;
    if (pBullOver->IsRightAligned())
    {
        eAlign = enumXFAlignEnd;
    }

    XFListStyle* pListStyle = new XFListStyle();
    XFStyleManager* pXFStyleMgr = LwpGlobalMgr::GetInstance()->GetXFStyleManager();

    if (!bIsNumbering)
    {
        for (sal_uInt8 nC = 1; nC < 11; nC++)
        {
            pListStyle->SetListBullet(nC, pSilverBullet->GetBulletChar(), pSilverBullet->GetBulletFontName(),
                pSilverBullet->GetPrefix(), pSilverBullet->GetSuffix());

            if (pIndent->GetMRest() > 0.001)
            {
                pListStyle->SetListPosition(nC, 0.0,
                    LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(pIndent->GetMRest())), 0.0, eAlign);
            }
            else
            {
                pListStyle->SetListPosition(nC, 0.0,
                    0.0, LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(pIndent->GetMFirst())), eAlign);
            }
        }

        aStyleName = pXFStyleMgr->AddStyle(pListStyle)->GetStyleName();
    }
    else
    {
        rtl::OUString aPrefix = rtl::OUString::createFromAscii("");

        LwpFrib* pFrib = pBulletParaFribs->HasFrib(FRIB_TAG_DOCVAR);
        LwpFribDocVar* pDocVarFrib = NULL;
        if (pFrib)
        {
            pDocVarFrib = static_cast<LwpFribDocVar*>(pFrib);
//				ModifierInfo* pInfo = pDocVarFrib->GetModifiers();
            switch (pDocVarFrib->GetType())
            {
            case 0x000D: // division name
                aPrefix = this->GetDivisionName();
                break;
            case 0x000E: // section name
                aPrefix = this->GetSectionName(pPara);
                break;
            }
        }
        ParaNumbering aParaNumbering;
        pBulletPara->GetParaNumber(1, &aParaNumbering);
        LwpFribParaNumber* pParaNumber = aParaNumbering.pParaNumber;
        if (pParaNumber)
        {
            for (sal_uInt8 nPos = 1; nPos < 10; nPos++)
            {
                aPrefix = rtl::OUString::createFromAscii("");
                if (pParaNumber->GetStyleID() != NUMCHAR_other)
                {
                    XFNumFmt aFmt;
                    if (aParaNumbering.pPrefix)
                    {
                        aPrefix += aParaNumbering.pPrefix->GetText();
//							aFmt.SetPrefix(aParaNumbering.pPrefix->GetText() + aAdditionalInfoName);
                    }

                    rtl::OUString aNumber = LwpSilverBullet::GetNumCharByStyleID(pParaNumber);
                    if (pParaNumber->GetStyleID() == NUMCHAR_01 || pParaNumber->GetStyleID() == NUMCHAR_Chinese4)
                    {
                        aPrefix += rtl::OUString::createFromAscii("0");
                    }
                    aFmt.SetPrefix(aPrefix);

                    aFmt.SetFormat(aNumber);

                    if (aParaNumbering.pSuffix)
                    {
                        aFmt.SetSuffix(aParaNumbering.pSuffix->GetText());
                    }

                    //set numbering format into the style-list.
                    pListStyle->SetListNumber(nPos, aFmt, pParaNumber->GetStart()+1);

                }
                else
                {
                    rtl::OUString aPrefix, aSuffix;
                    if (aParaNumbering.pPrefix)
                    {
                        aPrefix = aParaNumbering.pPrefix->GetText();
                    }
                    if (aParaNumbering.pSuffix)
                    {
                        aSuffix = aParaNumbering.pSuffix->GetText();
                    }

                    pListStyle->SetListBullet(nPos, LwpSilverBullet::GetNumCharByStyleID(pParaNumber).toChar(),
                        rtl::OUString::createFromAscii("Times New Roman"), aPrefix, aSuffix);
                }

                pListStyle->SetListPosition(nPos, 0.0, 0.635, 0.0);
            }
            aStyleName = pXFStyleMgr->AddStyle(pListStyle)->GetStyleName();
            }

    }

    m_vStyleNameList.push_back(aStyleName);
    return aStyleName;

}

/**
 * @short   No use now.
 * @param   rContent
 * @param   nFontID
 * @param   pIndent
 */
/*rtl::OUString LwpBulletStyleMgr::RegisterBulletStyle(const rtl::OUString& rContent, sal_uInt32 nFontID, LwpIndentOverride* pIndent)
{
    if (!pIndent)
    {
        assert(false);
    }

    if (!m_pFoundry)
    {
        return rtl::OUString::createFromAscii("");
    }
    LwpFontManager* pFontMgr = m_pFoundry->GetFontManger();

    rtl::OUString aFontName = pFontMgr->GetNameByID(nFontID);
    UChar32 cBulletChar = rContent.toChar();
    rtl::OUString aSuffix = rContent.copy(1);
    XFListStyle* pListStyle = new XFListStyle();
    XFStyleManager* pXFStyleMgr = XFStyleManager::Instance();

    for (sal_uInt8 nC = 1; nC < 11; nC++)
    {
        pListStyle->SetListBullet(nC, cBulletChar, aFontName, rtl::OUString::createFromAscii(""), aSuffix);

        if (pIndent->GetMRest() > 0.001)
        {
            pListStyle->SetListPosition(nC, 0.0,
                LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(pIndent->GetMRest())), 0.0);
        }
        else
        {
            pListStyle->SetListPosition(nC, 0.0,
                0.0, LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(pIndent->GetMFirst())));
        }
    }
    return pXFStyleMgr->AddStyle(pListStyle);
}*/

/**
 * @short   No use now.
 * @param   pListStyle
 * @param   pXFStyleMgr
 */
/*void LwpBulletStyleMgr::CreateNewListStyle(XFListStyle*& pListStyle, XFStyleManager* pXFStyleMgr)
{
    pListStyle = new XFListStyle();
    m_aBulletStyleList.push_back(pListStyle);
    m_aCurrentStyleName = pXFStyleMgr->AddStyle(pListStyle);
}*/

/**
 * @short   No use now.
 * @param   pSilverBullet
 * @param   nLevel
 */
/*rtl::OUString LwpBulletStyleMgr::AddStyleToList(LwpSilverBullet* pSilverBullet, sal_uInt16 nLevel)
{
    XFListStyle* pListStyle = NULL;
    XFStyleManager* pXFStyleMgr = XFStyleManager::Instance();

    if (m_aBulletStyleList.empty())
    {
    //	pListStyle = new XFListStyle();
    //	m_aBulletStyleList.push_back(pListStyle);
    //	m_aCurrentStyleName = pXFStyleMgr->AddStyle(pListStyle);
    //	this->CreateNewListStyle(pListStyle, pXFStyleMgr);
        if (pSilverBullet->IsBulletOrdered())
        {
            m_strCurrentNumberingName = pSilverBullet->GetNumberingName();
        }
        else
        {
            m_nCurrentChar = pSilverBullet->GetBulletChar();
            m_strCurrentFontName = pSilverBullet->GetBulletFontName();
        }
    }
    else if (nLevel == 1)
    {
        if (pSilverBullet->IsBulletOrdered())
        {
            if (m_strCurrentNumberingName == pSilverBullet->GetNumberingName())
            {
                pListStyle = m_aBulletStyleList.back();
            }
            else
            {
                this->CreateNewListStyle(pListStyle, pXFStyleMgr);
            }
        }
        else
        {
            if ((m_nCurrentChar == pSilverBullet->GetBulletChar()) &&
                (m_strCurrentFontName == pSilverBullet->GetBulletFontName()))
            {
                pListStyle = m_aBulletStyleList.back();
            }
            else
            {
                this->CreateNewListStyle(pListStyle, pXFStyleMgr);
            }
        }
    }
    else
    {
        pListStyle = m_aBulletStyleList.back();
    }

    if (pSilverBullet->IsBulletOrdered())
    {
//		pListStyle->SetListNumber(int level, XFNumFmt & fmt, sal_Int16 start);
    }
    else
    {
        pListStyle->SetListBullet(nLevel, pSilverBullet->GetBulletChar(), pSilverBullet->GetBulletFontName(),
            pSilverBullet->GetPrefix(), pSilverBullet->GetSuffix());
    }

    return m_aCurrentStyleName;
}*/

/**
 * @short   Output bullet list header, such as <text:ordered-list> and <text:list-item>
 * @param   pOutputStream pointer of XFstream to be written in.
 * @param   bIsOrdered if the list if ordered or not.
 * @param   rStyleName style name of the list
 * @param   nLevel level of the paragraph
 */
void LwpBulletStyleMgr::OutputBulletListHeader(IXFStream* pOutputStream, sal_Bool bIsOrdered,
        const rtl::OUString& rStyleName, sal_Int16 nLevel, sal_Bool bIsBulletSkiped)
{
    if (nLevel == 0)
    {
        return;
    }

    m_bIsBulletSkipped = bIsBulletSkiped;

    if (m_pBulletList)
    {
        delete m_pBulletList;
    }

    m_pBulletList = new XFList();

    //todo: need judge here.
    sal_Bool bContinue = m_bContinue;

    if (bIsOrdered)
    {
        m_pBulletList->SetOrdered(sal_True);
    }
    else
    {
        bContinue = sal_False;
        m_pBulletList->SetOrdered(sal_False);
    }
    m_pBulletList->SetStyleName(rStyleName);
//	if (nLevels < 0)
//	{
//		m_pBulletList->StartList(pOutputStream);
//	}
//	else
//	{
        for (sal_uInt8 nC = 0; nC < nLevel; nC++)
        {
            //continue numbering
            if (nC == nLevel-1)
            {
                m_pBulletList->StartList(pOutputStream, bContinue);
            }
            else
            {
                m_pBulletList->StartList(pOutputStream);
            }
            if ((nC == nLevel-1) && bIsBulletSkiped)
            {
                XFList::StartListHeader(pOutputStream);
            }
            else
            {
                XFList::StartListItem(pOutputStream);
            }
        }
//	}
}

#include "xfilter/xflistitem.hxx"
//Create nested XFList and XFItems and then add it to XFContentContainer(pCont)
//Return the inner XFItem created.
XFContentContainer* LwpBulletStyleMgr::AddBulletList(
        XFContentContainer* pCont, sal_Bool bIsOrdered,
        const rtl::OUString& rStyleName, sal_Int16 nLevel, sal_Bool bIsBulletSkiped)
{
    assert(nLevel>0);

    m_bIsBulletSkipped = bIsBulletSkiped;

    //todo: need judge here.
    sal_Bool bContinue = m_bContinue;

    XFList* theList;
    XFList* prevList = NULL;
    XFListItem* theItem;
    XFListItem* InnerItem = NULL;
    for (sal_Int8 nC = nLevel-1; nC >= 0; nC--)
    {
        theList = new XFList();
        theItem = new XFListItem();
        theList->Add(theItem);

        if (bIsOrdered)
        {
            theList->SetOrdered(sal_True);
        }
        else
        {
            bContinue = sal_False;
            theList->SetOrdered(sal_False);
        }

        if (nC == nLevel-1)
        {
            theList->SetContinueNumber(bContinue);
        }
        //Add the outer list to pCont
        if (nC == 0)
        {
            theList->SetStyleName(rStyleName);
            pCont->Add(theList);
        }

        if ((nC == nLevel-1) && bIsBulletSkiped)
        {
            theItem->SetIsHeader(sal_True);

            theList->SetContinueNumber(sal_True);
            // end of add
        }

        if(nC == nLevel-1)
        {
            InnerItem = theItem;
        }

        if(prevList)
        {
            theItem->Add(prevList);
        }
        prevList = theList;
    }
    return InnerItem;
}

/**
 * @short   Output bullet list header, such as </text:ordered-list> and </text:list-item>
 * @param   pOutputStream pointer of XFstream to be written in.
 * @param   nLevel level of the paragraph
 */
void LwpBulletStyleMgr::OutputBulletListTail(IXFStream* pOutputStream, sal_uInt16 nLevel)
{
    if ( !m_pBulletList )
    {
        return;
    }

    for (sal_uInt8 nC = 0; nC < nLevel; nC++)
    {
        if (m_bIsBulletSkipped)
        {
            XFList::EndListHeader(pOutputStream);
            m_bIsBulletSkipped = sal_False;
        }
        else
        {
            XFList::EndListItem(pOutputStream);
        }
        m_pBulletList->EndList(pOutputStream);
    }
}

rtl::OUString LwpBulletStyleMgr::GetDivisionName()
{
    if (!m_pFoundry)
    {
        return rtl::OUString::createFromAscii("");
    }

    rtl::OUString aRet = rtl::OUString::createFromAscii("");

    LwpDocument* pDoc = m_pFoundry->GetDocument();
    if (pDoc)
    {
        LwpObjectID* pID = pDoc->GetDivInfoID();
        if (!pID->IsNull())
        {
            aRet = static_cast<LwpDivInfo*>(pID->obj(VO_DIVISIONINFO))->GetDivName();
        }
    }

    return aRet;
}

rtl::OUString LwpBulletStyleMgr::GetSectionName(LwpPara* pPara)
{
    LwpObjectID* pStoryID = pPara->GetStoryID();
    if (pStoryID->IsNull())
    {
        return rtl::OUString::createFromAscii("");
    }

    LwpStory* pStory = static_cast<LwpStory*>(pStoryID->obj(VO_STORY));
    if (!pStory)
    {
        return rtl::OUString::createFromAscii("");
    }

    return pStory->GetSectionName();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
