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
 *      not inluding the numbering sequence.
 * @param   pPara pointer to the current paragraph which has a bullet/numbering.
 * @param   pBullOver pointer to the bulletoverride of current paragraph.
 * @param   pIndent pointer to the indentoverride of current paragraph.
 */
rtl::OUString LwpBulletStyleMgr::RegisterBulletStyle(LwpPara* pPara, LwpBulletOverride* pBullOver,
    LwpIndentOverride* pIndent)
{
    rtl::OUString aEmpty;

    if(!pPara || !pIndent || !pBullOver)
    {
        return aEmpty;
    }

    LwpSilverBullet* pSilverBullet = pPara->GetSilverBullet();
    if (!pSilverBullet)
    {
        assert(false);
        return aEmpty;
    }

    LwpPara* pBulletPara = pSilverBullet->GetBulletPara();
    if (!pBulletPara)
    {
        assert(false);
        return aEmpty;
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
    boost::shared_ptr<LwpBulletOverride> pBulletOver(pBullOver->clone());

    sal_uInt16 nNameIndex = 0;
    std::vector <OverridePair>::iterator iter;
    for(iter = m_vIDsPairList.begin(); iter != m_vIDsPairList.end(); ++iter)
    {
        if (iter->first->GetSilverBullet() == aBulletID && iter->second == aIndentID
            && iter->first->IsRightAligned() == pBullOver->IsRightAligned())
        {
            return m_vStyleNameList[nNameIndex];
        }
        else
        {
            nNameIndex++;
        }
    }

    m_vIDsPairList.push_back(std::make_pair(pBulletOver, aIndentID));
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
        ParaNumbering aParaNumbering;
        pBulletPara->GetParaNumber(1, &aParaNumbering);
        LwpFribParaNumber* pParaNumber = aParaNumbering.pParaNumber;
        if (pParaNumber)
        {
            for (sal_uInt8 nPos = 1; nPos < 10; nPos++)
            {
                if (pParaNumber->GetStyleID() != NUMCHAR_other)
                {
                    rtl::OUString aPrefix;
                    XFNumFmt aFmt;
                    if (aParaNumbering.pPrefix)
                    {
                        aPrefix += aParaNumbering.pPrefix->GetText();
                    }

                    rtl::OUString aNumber = LwpSilverBullet::GetNumCharByStyleID(pParaNumber);
                    if (pParaNumber->GetStyleID() == NUMCHAR_01 || pParaNumber->GetStyleID() == NUMCHAR_Chinese4)
                    {
                        aPrefix += rtl::OUString("0");
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
                        rtl::OUString("Times New Roman"), aPrefix, aSuffix);
                }

                pListStyle->SetListPosition(nPos, 0.0, 0.635, 0.0);
            }
            aStyleName = pXFStyleMgr->AddStyle(pListStyle)->GetStyleName();
        }
        else
            delete pListStyle;

    }

    m_vStyleNameList.push_back(aStyleName);
    return aStyleName;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
