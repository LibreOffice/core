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
#include <lwpglobalmgr.hxx>
#include "lwpsilverbullet.hxx"
#include "lwpdoc.hxx"
#include "lwpdivinfo.hxx"
#include "lwpfribheader.hxx"
#include <lwpfoundry.hxx>
#include "lwpstory.hxx"
#include "lwppara.hxx"
#include <xfilter/xfliststyle.hxx>
#include <xfilter/xfstylemanager.hxx>

LwpSilverBullet::LwpSilverBullet(LwpObjectHeader const & objHdr, LwpSvStream* pStrm)
    : LwpDLNFVList(objHdr, pStrm)
    , m_nFlags(0)
    , m_nUseCount(0)
    , m_pAtomHolder(new LwpAtomHolder)
{
}

LwpSilverBullet::~LwpSilverBullet()
{
}

void LwpSilverBullet::Read()
{
    LwpDLNFVList::Read();

    m_nFlags = m_pObjStrm->QuickReaduInt16();
    m_aStory.ReadIndexed(m_pObjStrm.get());

    sal_uInt16 nNumPos = m_pObjStrm->QuickReaduInt16();

    if (nNumPos > SAL_N_ELEMENTS(m_pResetPositionFlags))
        throw std::range_error("corrupt SilverBullet");

    for (sal_uInt16 nC = 0; nC < nNumPos; nC++)
        m_pResetPositionFlags[nC] = m_pObjStrm->QuickReaduInt8();

    m_nUseCount = m_pObjStrm->QuickReaduInt32();

    m_pAtomHolder->Read(m_pObjStrm.get());
}

/**
 * @short:   Register bullet or numbering style-list and store the returned
 *          name from XFStyleManager.
 * @descr:
 */
void LwpSilverBullet::RegisterStyle()
{
    std::unique_ptr<XFListStyle> xListStyle(new XFListStyle());
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();

    GetBulletPara();

    if (IsBulletOrdered() && HasName())
    {
        //todo: find the flag in the file
        bool bCumulative = false;

        for (sal_uInt8 nPos = 1; nPos < 10; nPos++)
        {
            ParaNumbering aParaNumbering;
            //get numbering format according to the position.
            m_xBulletPara->GetParaNumber(nPos, &aParaNumbering);
            LwpFribParaNumber* pParaNumber = aParaNumbering.pParaNumber;
            if (pParaNumber)
            {
                if (pParaNumber->GetStyleID() != NUMCHAR_other)
                {
                    m_pHideLevels[nPos] = aParaNumbering.nNumLevel;
                    sal_uInt16 nDisplayLevel = GetDisplayLevel(nPos);
                    bCumulative = (nDisplayLevel > 1);
                    OUString aPrefix = GetAdditionalName(nPos);

                    XFNumFmt aFmt;
                    if (!bCumulative && aParaNumbering.pPrefix)
                    {
                        aFmt.SetPrefix(aPrefix + aParaNumbering.pPrefix->GetText());
                    }

                    aFmt.SetFormat(GetNumCharByStyleID(pParaNumber));

                    if (aParaNumbering.pSuffix)
                    {
                        aFmt.SetSuffix(aParaNumbering.pSuffix->GetText());
                    }

                    //set numbering format into the style-list.
                    xListStyle->SetListNumber(nPos, aFmt, pParaNumber->GetStart()+1);

                    if (bCumulative && nPos > 1)
                    {
                        xListStyle->SetDisplayLevel(nPos, nDisplayLevel);
                    }

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

                    xListStyle->SetListBullet(nPos, GetNumCharByStyleID(pParaNumber),
                        "Times New Roman", aPrefix, aSuffix);
                }

                xListStyle->SetListPosition(nPos, 0.0, 0.635, 0.0);
                aParaNumbering.clear();
            }
        }
    }

    //add style-list to style manager.
    m_strStyleName = pXFStyleManager->AddStyle(std::move(xListStyle)).m_pStyle->GetStyleName();
}

/**
 * @short:   Get the font name of the bullet.
 * @descr:
 * @return:  Font name of the bullet.
 */
OUString LwpSilverBullet::GetBulletFontName()
{
    //foundry has been set?
    if (!m_pFoundry)
    {
        return OUString();
    }

    LwpFontManager& rFontMgr = m_pFoundry->GetFontManager();

    sal_uInt32 nBulletFontID = m_xBulletPara->GetBulletFontID();
    sal_uInt16 nFinalFont = static_cast<sal_uInt16>((nBulletFontID >> 16) & 0xFFFF);

    //final fontid is valid?
    if (nFinalFont > 255 || nFinalFont == 0)
    {
        return OUString();
    }

    //get font name from font manager.
    OUString aFontName = rFontMgr.GetNameByID(nBulletFontID);

    return aFontName;
}
/**
 * @short:   Get bullet character of the bullet vo_para.
 * @descr:
 * @return:  An UChar32 bullet character.
 */
OUString const & LwpSilverBullet::GetBulletChar() const
{
    return m_xBulletPara->GetBulletChar();
}

/**
 * @short:   Get the LwpPara object through story id.
 */
LwpPara* LwpSilverBullet::GetBulletPara()
{
    if (!m_xBulletPara.is())
    {
        LwpStory* pStory = dynamic_cast<LwpStory*>(m_aStory.obj(VO_STORY).get());
        if (!pStory)
        {
            return nullptr;
        }

        m_xBulletPara.set(dynamic_cast<LwpPara*>(pStory->GetFirstPara().obj(VO_PARA).get()));
    }

    return m_xBulletPara.get();
}

/**
 * @short:   Get numbering character of the bullet vo_para.
 * @descr:
 * @param:   pParaNumber a pointer to the structure LwpFribParaNumber which
 *          includes numbering prefix, format and suffix.
 * @return:  An OUString object which store the numbering character.
 */
OUString LwpSilverBullet::GetNumCharByStyleID(LwpFribParaNumber const * pParaNumber)
{
    if (!pParaNumber)
    {
        assert(false);
        return OUString();
    }

    OUString strNumChar("1");
    sal_uInt16 nStyleID = pParaNumber->GetStyleID();

    switch (nStyleID)
    {
    case NUMCHAR_1:
    case NUMCHAR_01:
    case NUMCHAR_Chinese4:
        strNumChar = "1";
        break;
    case NUMCHAR_A :
        strNumChar = "A";
        break;
    case NUMCHAR_a:
        strNumChar = "a";
        break;
    case NUMCHAR_I:
        strNumChar = "I";
        break;
    case NUMCHAR_i:
        strNumChar = "i";
        break;
    case NUMCHAR_other:
        strNumChar = OUString(sal_Unicode(pParaNumber->GetNumberChar()));
        break;
    case NUMCHAR_Chinese1:
        {
        sal_Unicode const sBuf[13] = {0x58f9,0x002c,0x0020,0x8d30,0x002c,0x0020,0x53c1,0x002c,0x0020,0x002e,0x002e,0x002e,0x0};
        strNumChar = OUString(sBuf);
        }
        break;
    case NUMCHAR_Chinese2:
        {
        sal_Unicode const sBuf[13] = {0x4e00,0x002c,0x0020,0x4e8c,0x002c,0x0020,0x4e09,0x002c,0x0020,0x002e,0x002e,0x002e,0x0};
        strNumChar = OUString(sBuf);
        }
        break;
    case NUMCHAR_Chinese3:
        {
        sal_Unicode const sBuf[13] = {0x7532,0x002c,0x0020,0x4e59,0x002c,0x0020,0x4e19,0x002c,0x0020,0x002e,0x002e,0x002e,0x0};
        strNumChar = OUString(sBuf);
        }
        break;
    case NUMCHAR_none:
        strNumChar.clear();
        break;
    default:
        break;
    }//mod end
    return strNumChar;
}

/**
 * @short:   To judge the silverbullet list is ordered or not.
 * @descr:
 * @return:  sal_True if list is ordered, sal_False if list is unordered.
 */
bool LwpSilverBullet::IsBulletOrdered()
{
    if (!m_xBulletPara.is())
        return false;

    LwpFribPtr& rFribs = m_xBulletPara->GetFribs();

    return (rFribs.HasFrib(FRIB_TAG_PARANUMBER) != nullptr);
}

/**
 * @short:   Calculate the displaylevel according to the position and hidelevels.
 * @descr:
 * @param:   nPos position of the numbering.
 * @return:  displaylevel of the position passed in.
 */
sal_uInt16 LwpSilverBullet::GetDisplayLevel(sal_uInt8 nPos)
{
    if (nPos > 1)
    {
        sal_uInt16 nHideBit = (1 << nPos);
        for (sal_uInt8 nC = nPos-1; nC > 0; nC--)
        {
            sal_uInt16 nAttrMask = ~m_pHideLevels[nC];
            if (!(nAttrMask & nHideBit))
            {
                return static_cast<sal_uInt16>(nPos - nC);
            }
        }
    }

    return static_cast<sal_uInt16>(nPos);
}

/**
 * @descr:   Get the additional information, "Division name" or "Section Name" from document
 *      variable frib according to numbering position. Whether we should add a
 *      Division/Section name or not is determined by the hidelevels of the frib.
 * @param:   nPos position of the numbering.
 * @return:  Division or Section name.
 */
OUString LwpSilverBullet::GetAdditionalName(sal_uInt8 nPos)
{
    OUString aRet;
    sal_uInt16 nHideBit = (1 << nPos);
    bool bDivisionName = false;
    bool bSectionName = false;

    LwpFrib* pParaFrib = m_xBulletPara->GetFribs().GetFribs();
    if (!pParaFrib)
    {
        return OUString();
    }

    while (pParaFrib)
    {
        if (pParaFrib->GetType() == FRIB_TAG_DOCVAR)
        {
            ModifierInfo* pMoInfo = pParaFrib->GetModifiers();
            if (!pMoInfo)
            {
                return OUString();
            }
            sal_uInt16 nHideLevels = pMoInfo->aTxtAttrOverride.GetHideLevels();
            sal_uInt16 nType = static_cast<LwpFribDocVar*>(pParaFrib)->GetType();

            if (~nHideLevels & nHideBit)
            {
                if (nType == 0x000D)
                {
                    bDivisionName = true;
                }
                else if (nType == 0x000E)
                {
                    bSectionName= true;
                }
            }
        }
        pParaFrib = pParaFrib->GetNext();
    }

    if (bDivisionName)
    {
        aRet += GetDivisionName();
    }
    if (bSectionName)
    {
        aRet += GetSectionName();
    }

    return aRet;
}

OUString LwpSilverBullet::GetDivisionName()
{
    OUString aRet;

    if (!m_pFoundry)
    {
        return aRet;
    }

    LwpDocument* pDoc = m_pFoundry->GetDocument();
    if (pDoc)
    {
        LwpObjectID& rID = pDoc->GetDivInfoID();
        if (!rID.IsNull())
        {
            LwpDivInfo *pInfo = dynamic_cast<LwpDivInfo*>(rID.obj(VO_DIVISIONINFO).get());
            if (pInfo)
                aRet = pInfo->GetDivName();
        }
    }

    return aRet;
}

OUString LwpSilverBullet::GetSectionName() const
{
    LwpStory* pStory = dynamic_cast<LwpStory*>(m_aStory.obj(VO_STORY).get());
    if (!pStory)
    {
        return OUString();
    }

    return pStory->GetSectionName();
}

bool LwpSilverBullet::HasName()
{
    LwpAtomHolder& rName = GetName();
    return (!rName.str().isEmpty());
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
