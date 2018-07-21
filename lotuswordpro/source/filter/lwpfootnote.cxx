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

#include <memory>
#include "lwpfootnote.hxx"
#include "lwpstory.hxx"
#include <xfilter/xffootnote.hxx>
#include <xfilter/xfendnote.hxx>
#include <xfilter/xffootnoteconfig.hxx>
#include <xfilter/xfendnoteconfig.hxx>
#include <xfilter/xfstylemanager.hxx>
#include <xfilter/xftextspan.hxx>
#include "lwppara.hxx"
#include "lwpdoc.hxx"
#include "lwpfnlayout.hxx"
#include <lwpglobalmgr.hxx>

LwpFribFootnote::LwpFribFootnote(LwpPara* pPara ):LwpFrib(pPara)
{
}

/**
 * @descr  read footnote frib information
 */
void LwpFribFootnote::Read(LwpObjectStream * pObjStrm, sal_uInt16 /*len*/)
{
    m_Footnote.ReadIndexed(pObjStrm);
}

/**
 * @descr  Register footnote style by calling LwpFootnote::RegisterStyle()
 */
void LwpFribFootnote::RegisterNewStyle()
{
    LwpFootnote* pFootnote = GetFootnote();
    if(pFootnote)
    {
        //register footnote number font style
        LwpFrib::RegisterStyle(m_pPara->GetFoundry());
        //register footnote content style
        pFootnote->SetFoundry(m_pPara->GetFoundry());
        pFootnote->RegisterStyle();
    }
}

/**
 * @descr  Parse footnote  by calling LwpFootnote::XFConvert()
 */
void LwpFribFootnote::XFConvert(XFContentContainer* pCont)
{
    LwpFootnote* pFootnote = GetFootnote();
    if(pFootnote)
    {
        rtl::Reference<XFContentContainer> xContent;
        if(pFootnote->GetType() == FN_FOOTNOTE)
        {
            xContent.set(new XFFootNote);
        }
        else
        {
            xContent.set(new XFEndNote);
        }
        pFootnote->XFConvert(xContent.get());
        if (m_ModFlag)
        {
            //set footnote number font style
            rtl::Reference<XFTextSpan> xSpan(new XFTextSpan);
            xSpan->SetStyleName(GetStyleName());
            //add the xffootnote into the content container
            xSpan->Add(xContent.get());
            pCont->Add(xSpan.get());
        }
        else
        {
            pCont->Add(xContent.get());
        }
    }
}

/**
 * @descr  Get foonote object
 */
LwpFootnote* LwpFribFootnote::GetFootnote()
{
    return dynamic_cast<LwpFootnote*>(m_Footnote.obj().get());
}

LwpFootnote::LwpFootnote(LwpObjectHeader const &objHdr, LwpSvStream *pStrm)
    : LwpOrderedObject(objHdr, pStrm)
    , m_nType(0)
    , m_nRow(0)
{
}

LwpFootnote::~LwpFootnote()
{

}

/**
 * @descr  Read foonote object
 */
void LwpFootnote::Read()
{
    LwpOrderedObject::Read();
    m_nType = m_pObjStrm->QuickReaduInt16();
    m_nRow = m_pObjStrm->QuickReaduInt16();
    m_Content.ReadIndexed(m_pObjStrm.get());
    m_pObjStrm->SkipExtra();
}

/**
 * @descr  Register footnote style
 */
void LwpFootnote::RegisterStyle()
{
    //Only register footnote contents style,
    //Endnote contents style registers in LwpEnSuperTableLayout::RegisterStyle
    if(m_nType == FN_FOOTNOTE)
    {
        LwpContent* pContent = FindFootnoteContent();
        if(pContent)
        {
            pContent->SetFoundry(m_pFoundry);
            pContent->DoRegisterStyle();
        }
    }
}

/**
 * @descr  Parse footnote
 */
void LwpFootnote::XFConvert(XFContentContainer * pCont)
{
    LwpContent* pContent = FindFootnoteContent();
    if(pContent)
    {
        pContent->DoXFConvert(pCont);
    }
}

/**
 * @descr  Get endnote cell layout which contains current endnote content
 */
LwpCellLayout* LwpFootnote::GetCellLayout()
{
    LwpEnSuperTableLayout* pEnSuperLayout = FindFootnoteTableLayout();
    if(pEnSuperLayout)
    {
        LwpTableLayout* pTableLayout = dynamic_cast<LwpTableLayout*>(pEnSuperLayout->GetMainTableLayout());
        if(pTableLayout)
        {
            LwpRowLayout* pRowLayout = pTableLayout->GetRowLayout(m_nRow);
            if(pRowLayout)
            {
                return dynamic_cast<LwpCellLayout*>(pRowLayout->GetChildHead().obj().get());
            }
        }
    }
    return nullptr;
}

/**
 * @descr  Get division which footnote table contains current footnote content, copy from lwp source code
 */
LwpDocument* LwpFootnote::GetFootnoteTableDivision()
{
    if(!m_pFoundry)
        return nullptr;

    LwpDocument* pPrev =nullptr;
    LwpDocument* pDivision = nullptr;
    LwpDocument* pFootnoteDivision =nullptr;

    // Make sure the footnote does belong to some division
    // The division might not have a DivisionInfo if it's being Destruct()ed
    pPrev = m_pFoundry->GetDocument();
    pFootnoteDivision = pPrev;
    if (!pPrev || pPrev->GetDivInfoID().IsNull())
        return nullptr;

    switch (m_nType)
    {
        case FN_FOOTNOTE:
        {
            // Footnotes always use the source division
            return pFootnoteDivision;
        }
        case FN_DIVISION:
        {
            // Start with the footnote's division
            pDivision = pPrev;
            break;
        }
        case FN_DIVISION_SEPARATE:
        {
            // It had better be the next division
            pDivision = pPrev->GetNextDivision();
            break;
        }
        case FN_DIVISIONGROUP:
        case FN_DIVISIONGROUP_SEPARATE:
        {
            pDivision = pPrev->GetLastInGroupWithContents();
            break;
        }
        case FN_DOCUMENT:
        case FN_DOCUMENT_SEPARATE:
        {
            pDivision = pFootnoteDivision->GetRootDocument();
            if (pDivision)
                pDivision = pDivision->GetLastDivisionWithContents();
            break;
        }

    }

    // Make sure we're using the proper endnote division, if it's separate
    if (m_nType & FN_MASK_SEPARATE)
        pDivision = GetEndnoteDivision(pDivision);
    // Don't use a division that's specifically for endnotes
    else
    {
        while (pDivision)
        {
            if (pDivision->GetEndnoteType() == FN_DONTCARE)
                break;
            if (m_nType == FN_DIVISIONGROUP)
                pDivision = pDivision->GetPreviousInGroup();
            else
                pDivision = pDivision->GetPreviousDivisionWithContents();
        }
    }
    if (pDivision)
        return pDivision;
    return nullptr;
}

/**
 * @descr  Get division which endnote table contains current endnote content, copy from lwp source code
 */
LwpDocument* LwpFootnote::GetEndnoteDivision(LwpDocument* pPossible)
{
    LwpDocument*  pDivision = pPossible;
    sal_uInt16 nDivType;

    // In case we have multiple endnote divisions, walk backwards until
    // we find one.
    while (pDivision)
    {
        // Do we already have the right division?
        nDivType = pDivision->GetEndnoteType();
        if (nDivType == m_nType)
            return pDivision;
        // When we hit the first non-endnote division, stop looking.
        // -- SDC 10/8/96
        if (nDivType == FN_DONTCARE)
            break;
        pDivision = pDivision->GetPreviousDivision();
    }
    return nullptr;
}

/**
 * @descr  Get footnote table class name
 */
OUString LwpFootnote::GetTableClass()
{
    OUString strClassName;
    switch (GetType() & FN_MASK_BASE)
    {
        case FN_BASE_FOOTNOTE:
        {
            strClassName = STR_DivisionFootnote;
            break;
        }
        case FN_BASE_DOCUMENT:
        {
            strClassName = STR_DocumentEndnote;
            break;
        }
        case FN_BASE_DIVISION:
        {
            strClassName = STR_DivisionEndnote;
            break;
        }
        case FN_BASE_DIVISIONGROUP:
        {
            strClassName = STR_DivisionGroupEndnote;
            break;
        }
    }
    return strClassName;
}

/**
 * @descr  Find footnote tablelayout, copy from lwp source code
 */
LwpEnSuperTableLayout* LwpFootnote::FindFootnoteTableLayout()
{
    LwpDocument* pDivision = GetFootnoteTableDivision();
    if(!pDivision)
        return nullptr;

    LwpFoundry* pFoundry = pDivision->GetFoundry();
    OUString strClassName = GetTableClass();
    if(strClassName.isEmpty() )
        return nullptr;

    LwpContent* pContent = nullptr;

    while ((pContent = pFoundry->EnumContents(pContent)) != nullptr)
        if (pContent->IsTable() && (strClassName == pContent->GetClassName()) &&
            pContent->IsActive() && pContent->GetLayout(nullptr).is())
        {
            // Found it!
            return static_cast<LwpEnSuperTableLayout *>(
                static_cast<LwpTable*>(pContent)->GetSuperTableLayout());
        }

    return nullptr;
}

/**
 * @descr  Find footnote contents
 */
LwpContent* LwpFootnote::FindFootnoteContent()
{
    LwpContent* pContent = dynamic_cast<LwpContent*>(m_Content.obj().get());
    //if the content has layout, the content has footnote contents;
    //or looking for the celllayout and return the footnote contents.
    if (pContent && pContent->GetLayout(nullptr).is())
        return pContent;

    LwpCellLayout* pCellLayout = GetCellLayout();
    if(pCellLayout)
    {
        pContent = dynamic_cast<LwpContent*>(pCellLayout->GetContent().obj().get());
    }

    return pContent;
}

LwpFootnoteTable::LwpFootnoteTable(LwpObjectHeader const &objHdr, LwpSvStream *pStrm)
    : LwpTable(objHdr, pStrm)
{
}

void LwpFootnoteTable::Read()
{
    LwpTable::Read();
    m_pObjStrm->SkipExtra();
}

/**
 * @descr  Read footnote number options information
 */
void LwpFootnoteNumberOptions::Read(LwpObjectStream *pObjStrm)
{
    m_nFlag = pObjStrm->QuickReaduInt16();
    m_nStartingNumber = pObjStrm->QuickReaduInt16();
    m_LeadingText.Read(pObjStrm);
    m_TrailingText.Read(pObjStrm);
    pObjStrm->SkipExtra();
}

/**
 * @descr  Read footnote separator options information
 */
void LwpFootnoteSeparatorOptions::Read(LwpObjectStream *pObjStrm)
{
    m_nFlag = pObjStrm->QuickReaduInt16();
    m_nLength = pObjStrm->QuickReaduInt32();
    m_nIndent = pObjStrm->QuickReaduInt32();
    m_nAbove = pObjStrm->QuickReaduInt32();
    m_nBelow = pObjStrm->QuickReaduInt32();
    m_BorderStuff.Read(pObjStrm);
    pObjStrm->SkipExtra();
}

LwpFootnoteOptions::LwpFootnoteOptions(LwpObjectHeader const &objHdr, LwpSvStream *pStrm)
    : LwpObject(objHdr, pStrm)
    , m_nFlag(0)
{
}

LwpFootnoteOptions::~LwpFootnoteOptions()
{

}

/**
 * @descr  Register footnote options object
 */
void LwpFootnoteOptions::Read()
{
    m_nFlag = m_pObjStrm->QuickReaduInt16();
    m_FootnoteNumbering.Read(m_pObjStrm.get());
    m_EndnoteDivisionNumbering.Read(m_pObjStrm.get());
    m_EndnoteDivisionGroupNumbering.Read(m_pObjStrm.get());
    m_EndnoteDocNumbering.Read(m_pObjStrm.get());
    m_FootnoteSeparator.Read(m_pObjStrm.get());
    m_FootnoteContinuedSeparator.Read(m_pObjStrm.get());
    m_ContinuedOnMessage.Read(m_pObjStrm.get());
    m_ContinuedFromMessage.Read(m_pObjStrm.get());
    m_pObjStrm->SkipExtra();
}

/**
 * @descr  Register footnote options style
 */
void LwpFootnoteOptions::RegisterStyle()
{
    RegisterFootnoteStyle();
    RegisterEndnoteStyle();
}

/**
 * @descr  Register footnote configuration information
 */
void LwpFootnoteOptions::RegisterFootnoteStyle()
{
    std::unique_ptr<XFFootnoteConfig> xFootnoteConfig(new XFFootnoteConfig);
    xFootnoteConfig->SetStartValue(m_FootnoteNumbering.GetStartingNumber() -1);
    xFootnoteConfig->SetNumPrefix(m_FootnoteNumbering.GetLeadingText());
    xFootnoteConfig->SetNumSuffix(m_FootnoteNumbering.GetTrailingText());
    if(m_FootnoteNumbering.GetReset() == LwpFootnoteNumberOptions::RESET_PAGE)
    {
        xFootnoteConfig->SetRestartOnPage();
    }
    if(GetContinuedFrom())
    {
        xFootnoteConfig->SetMessageFrom(GetContinuedFromMessage());
    }
    if(GetContinuedOn())
    {
        xFootnoteConfig->SetMessageOn(GetContinuedOnMessage());
    }

    xFootnoteConfig->SetMasterPage( m_strMasterPage);
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    pXFStyleManager->SetFootnoteConfig(xFootnoteConfig.release());
}

/**
 * @descr  Register endnote configuration information
 */
void LwpFootnoteOptions::RegisterEndnoteStyle()
{
    std::unique_ptr<XFEndnoteConfig> xEndnoteConfig(new XFEndnoteConfig);
    xEndnoteConfig->SetStartValue(m_EndnoteDocNumbering.GetStartingNumber() -1);
    OUString message = m_EndnoteDocNumbering.GetLeadingText();
    if(message.isEmpty())
    {
        message = "[";//default prefix
    }
    xEndnoteConfig->SetNumPrefix(message);
    message = m_EndnoteDocNumbering.GetTrailingText();
    if(message.isEmpty())
    {
        message = "]";//default suffix
    }
    xEndnoteConfig->SetNumSuffix(message);
    if(m_EndnoteDocNumbering.GetReset() == LwpFootnoteNumberOptions::RESET_PAGE)
    {
        xEndnoteConfig->SetRestartOnPage();
    }

    xEndnoteConfig->SetMasterPage( m_strMasterPage);

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    pXFStyleManager->SetEndnoteConfig(xEndnoteConfig.release());
}

/**
 * @descr  Get continue on message
 */
OUString LwpFootnoteOptions::GetContinuedOnMessage()
{
    if(m_ContinuedOnMessage.HasValue())
    {
        return m_ContinuedOnMessage.str();
    }
    // else return default message
    return OUString(STRID_FOOTCONTINUEDON);
}

/**
 * @descr  Get continue from message
 */
OUString LwpFootnoteOptions::GetContinuedFromMessage()
{
    if(m_ContinuedFromMessage.HasValue())
    {
        return m_ContinuedFromMessage.str();
    }
    // else return default message
    return OUString(STRID_FOOTCONTINUEDFROM);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
