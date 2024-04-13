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

#include "lwpbulletstylemgr.hxx"
#include "lwpfribheader.hxx"
#include "lwppara.hxx"
#include "lwpsilverbullet.hxx"
#include <lwptools.hxx>
#include "lwpparaproperty.hxx"
#include <xfilter/xfliststyle.hxx>
#include <xfilter/xfstylemanager.hxx>
#include <xfilter/xflist.hxx>
#include <lwpglobalmgr.hxx>
#include <xfilter/xflistitem.hxx>
#include <sal/log.hxx>

LwpBulletStyleMgr::LwpBulletStyleMgr()
    : m_pFoundry(nullptr)
    , m_bContinue(true)
    , m_bIsBulletSkipped(false)
{
}

LwpBulletStyleMgr::~LwpBulletStyleMgr()
{
    m_vIDsPairList.clear();
    m_vStyleNameList.clear();
}

/**
 * @short   Register bullet style to style-list. The function only register the bullet and single customized numbering
 *      not including the numbering sequence.
 * @param   pPara pointer to the current paragraph which has a bullet/numbering.
 * @param   pBullOver pointer to the bulletoverride of current paragraph.
 * @param   pIndent pointer to the indentoverride of current paragraph.
 */
OUString LwpBulletStyleMgr::RegisterBulletStyle(LwpPara* pPara, const LwpBulletOverride* pBullOver,
    LwpIndentOverride const * pIndent)
{
    if(!pPara || !pIndent || !pBullOver)
    {
        return OUString();
    }

    LwpSilverBullet* pSilverBullet = pPara->GetSilverBullet();
    if (!pSilverBullet)
    {
        assert(false);
        return OUString();
    }

    LwpPara* pBulletPara = pSilverBullet->GetBulletPara();
    if (!pBulletPara)
    {
        SAL_WARN("lwp", "missing bullet para");
        return OUString();
    }

    LwpParaProperty* pProp = pPara->GetProperty(PP_LOCAL_INDENT);
    LwpObjectID aIndentID;
    if (pProp)
    {
        LwpParaIndentProperty* pIndentProp = static_cast<LwpParaIndentProperty*>(pProp);
        aIndentID = pIndentProp->GetIndentID();
    }

    LwpObjectID aBulletID = pBullOver->GetSilverBullet();
    std::unique_ptr<LwpBulletOverride> pBulletOver(pBullOver->clone());

    sal_uInt16 nNameIndex = 0;
    for (auto const& vIDsPair : m_vIDsPairList)
    {
        if (vIDsPair.first->GetSilverBullet() == aBulletID && vIDsPair.second == aIndentID
            && vIDsPair.first->IsRightAligned() == pBullOver->IsRightAligned())
        {
            return m_vStyleNameList[nNameIndex];
        }
        else
        {
            nNameIndex++;
        }
    }

    m_vIDsPairList.emplace_back(std::move(pBulletOver), aIndentID);
    OUString aStyleName;

    LwpFribPtr& rBulletParaFribs = pBulletPara->GetFribs();
    bool bIsNumbering = (rBulletParaFribs.HasFrib(FRIB_TAG_PARANUMBER) != nullptr);

    enumXFAlignType eAlign = enumXFAlignStart;
    if (pBullOver->IsRightAligned())
    {
        eAlign = enumXFAlignEnd;
    }

    std::unique_ptr<XFListStyle> pListStyle(new XFListStyle());
    XFStyleManager* pXFStyleMgr = LwpGlobalMgr::GetInstance()->GetXFStyleManager();

    if (!bIsNumbering)
    {
        for (sal_uInt8 nC = 1; nC < 11; nC++)
        {
            pListStyle->SetListBullet(nC, pSilverBullet->GetBulletChar(), pSilverBullet->GetBulletFontName(),
                LwpSilverBullet::GetPrefix(), LwpSilverBullet::GetSuffix());

            if (pIndent->GetMRest() > 0) /* note: used to be 0.001, no idea why */
            {
                pListStyle->SetListPosition(nC, 0.0,
                    LwpTools::ConvertFromUnits(pIndent->GetMRest()), 0.0, eAlign);
            }
            else
            {
                pListStyle->SetListPosition(nC, 0.0,
                    0.0, LwpTools::ConvertFromUnits(pIndent->GetMFirst()), eAlign);
            }
        }

        aStyleName = pXFStyleMgr->AddStyle(std::move(pListStyle)).m_pStyle->GetStyleName();
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
                    OUString aPrefix;
                    XFNumFmt aFmt;
                    if (aParaNumbering.pPrefix)
                    {
                        aPrefix += aParaNumbering.pPrefix->GetText();
                    }

                    OUString aNumber = LwpSilverBullet::GetNumCharByStyleID(pParaNumber);
                    if (pParaNumber->GetStyleID() == NUMCHAR_01 || pParaNumber->GetStyleID() == NUMCHAR_Chinese4)
                    {
                        aPrefix += "0";
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
                    OUString aPrefix, aSuffix;
                    if (aParaNumbering.pPrefix)
                    {
                        aPrefix = aParaNumbering.pPrefix->GetText();
                    }
                    if (aParaNumbering.pSuffix)
                    {
                        aSuffix = aParaNumbering.pSuffix->GetText();
                    }

                    pListStyle->SetListBullet(nPos, LwpSilverBullet::GetNumCharByStyleID(pParaNumber),
                        "Times New Roman", aPrefix, aSuffix);
                }

                pListStyle->SetListPosition(nPos, 0.0, 0.635, 0.0);
            }
            aStyleName = pXFStyleMgr->AddStyle(std::move(pListStyle)).m_pStyle->GetStyleName();
        }
    }

    m_vStyleNameList.push_back(aStyleName);
    return aStyleName;

}

//Create nested XFList and XFItems and then add it to XFContentContainer(pCont)
//Return the inner XFItem created.
rtl::Reference<XFContentContainer> LwpBulletStyleMgr::AddBulletList(
        XFContentContainer* pCont, bool bIsOrdered,
        const OUString& rStyleName, sal_Int16 nLevel, bool bIsBulletSkiped)
{
    assert(nLevel > 0);

    m_bIsBulletSkipped = bIsBulletSkiped;

    //todo: need judge here.
    bool bContinue = m_bContinue;

    rtl::Reference<XFList> prevList;
    XFListItem* theItem;
    XFListItem* InnerItem = nullptr;
    for (sal_Int8 nC = nLevel - 1; nC >= 0; nC--)
    {
        rtl::Reference<XFList> theList(new XFList);
        theItem = new XFListItem();
        theList->Add(theItem);

        if (bIsOrdered)
        {
            theList->SetOrdered(true);
        }
        else
        {
            bContinue = false;
            theList->SetOrdered(false);
        }

        if (nC == nLevel - 1)
        {
            theList->SetContinueNumber(bContinue);
        }
        //Add the outer list to pCont
        if (nC == 0 && pCont)
        {
            theList->SetStyleName(rStyleName);
            theList->SetContinueNumber(bContinue);
            pCont->Add(theList.get());
        }

        if ((nC == 0) && bIsBulletSkiped)
        {
            theList->SetContinueNumber(true);
        }

        if ((nC == nLevel - 1) && bIsBulletSkiped)
        {
            theItem->SetIsHeader();
        }

        if (nC == nLevel - 1)
        {
            InnerItem = theItem;
        }

        if (prevList)
        {
            theItem->Add(prevList.get());
        }
        prevList = theList;
    }
    return InnerItem;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
