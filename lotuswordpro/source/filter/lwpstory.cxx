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
 *  LwpStory:
        1. Word Pro object for paragraph list;
        2. the content of layout object
 ************************************************************************/

#include <lwpglobalmgr.hxx>
#include "lwpstory.hxx"
#include <xfilter/xfstylemanager.hxx>
#include "lwppara.hxx"
#include <lwpobjfactory.hxx>
#include "lwppagelayout.hxx"
#include <rtl/ustrbuf.hxx>

#include <algorithm>
#include <set>


LwpStory::LwpStory(LwpObjectHeader const &objHdr, LwpSvStream* pStrm)
    : LwpContent(objHdr, pStrm)
    , m_bPMModified(false)
    , m_pCurrentLayout(nullptr)
    , m_pTabLayout(nullptr)
    , m_bDropcap(false)
    , m_pHyperlinkMgr(new LwpHyperlinkMgr)
{
}

LwpStory::~LwpStory()
{
}

void LwpStory::Read()
{
    LwpContent::Read();
    m_ParaList.Read(m_pObjStrm.get());
    m_FirstParaStyle.ReadIndexed(m_pObjStrm.get());
    m_pObjStrm->SkipExtra();
}

/**************************************************************************
 * @descr:   Convert all the contents in current story
**************************************************************************/
void LwpStory::XFConvert(XFContentContainer* pCont)
{
    //process frame which anchor frame
    XFConvertFrameInFrame(pCont);
    //process para list
    XFContentContainer* pParaCont = pCont;
    rtl::Reference<LwpPara> xPara(dynamic_cast<LwpPara*>(GetFirstPara().obj().get()));
    std::set<LwpPara*> aConverted;
    while (xPara.is())
    {
        xPara->SetFoundry(m_pFoundry);
        xPara->XFConvert(pParaCont);
        aConverted.insert(xPara.get());

        //Get the xfcontainer for the next para
        pParaCont = xPara->GetXFContainer();
        rtl::Reference<LwpPara> xNext(dynamic_cast<LwpPara*>(xPara->GetNext().obj().get()));
        if (aConverted.find(xNext.get()) != aConverted.end())
            throw std::runtime_error("loop in conversion");
        xPara = xNext;
    }

    //process frame which anchor is to cell after converter all the para
    XFConvertFrameInCell(pCont);
    XFConvertFrameInHeaderFooter(pCont);

    //Release Lwp Objects
    rtl::Reference<LwpPara> xCur(dynamic_cast<LwpPara*>(GetFirstPara().obj().get()));
    rtl::Reference<LwpPara> xNext;
    while (xCur.is())
    {
        xNext.set(dynamic_cast<LwpPara*>(xCur->GetNext().obj().get()));
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpObjectFactory* pObjMgr = pGlobal->GetLwpObjFactory();
        pObjMgr->ReleaseObject(xCur->GetObjectID());
        xCur = xNext;
    }
}

void LwpStory::RegisterStyle()
{
    rtl::Reference<LwpPara> xPara(dynamic_cast<LwpPara*>(GetFirstPara().obj().get()));
    std::set<LwpPara*> aSeen;
    while (xPara.is())
    {
        aSeen.insert(xPara.get());
        xPara->SetFoundry(m_pFoundry);
        xPara->DoRegisterStyle();
        xPara.set(dynamic_cast<LwpPara*>(xPara->GetNext().obj().get()));
        if (aSeen.find(xPara.get()) != aSeen.end())
            throw std::runtime_error("loop in register style");
    }
}

void LwpStory::Parse(IXFStream* pOutputStream)
{
    m_xXFContainer.set(new XFContentContainer);
    XFConvert(m_xXFContainer.get());
    m_xXFContainer->ToXml(pOutputStream);
    m_xXFContainer.clear();
}

/**************************************************************************
 * @descr:   Set current page layout. If pPageLayout is a mirror page layout,
             use odd child page layout as current page layout.
**************************************************************************/
void LwpStory::SetCurrentLayout(LwpPageLayout *pPageLayout)
{
    LwpPageLayout* pLayout = pPageLayout->GetOddChildLayout();
    if(pLayout)
    {
        m_pCurrentLayout = pLayout;
        m_pTabLayout = pLayout;
    }
    else
    {
        m_pCurrentLayout = pPageLayout;
        m_pTabLayout = pPageLayout;
    }
    m_bPMModified = true;
}

/**************************************************************************
 * @descr:   Get the next page layout relative to m_pCurrentLayout
**************************************************************************/
LwpPageLayout* LwpStory::GetNextPageLayout()
{
    std::vector<LwpPageLayout*>::iterator it = std::find(m_LayoutList.begin(), m_LayoutList.end(), m_pCurrentLayout);
    if (it != m_LayoutList.end() && (it+1) != m_LayoutList.end())
        return *(it+1);
    return nullptr;
}
/**************************************************************************
 * @descr:   Sort the pagelayout according to their position
**************************************************************************/
void LwpStory::SortPageLayout()
{
    //Get all the pagelayout and store in list
    std::vector<LwpPageLayout*>  aLayoutList;
    rtl::Reference<LwpVirtualLayout> xLayout(GetLayout(nullptr));
    while (xLayout.get())
    {
        LwpPageLayout *pLayout = xLayout->IsPage()
            ? dynamic_cast<LwpPageLayout*>(xLayout.get())
            : nullptr;
        if (pLayout)
        {
            LwpLayout::UseWhenType eSectionType = pLayout->GetUseWhenType();
            //for mirror page, the child is pagelayout
            rtl::Reference<LwpVirtualLayout> xParent = xLayout->GetParentLayout();
            if(eSectionType != LwpLayout::StartWithinColume && xParent.is() && !xParent->IsPage())
            {
                aLayoutList.push_back(pLayout);
            }
        }
        xLayout = GetLayout(xLayout.get());
    }
    // sort the pagelayout according to their position
    if (!aLayoutList.empty())
    {
        for( std::vector<LwpPageLayout*>::iterator aIt = aLayoutList.begin(); aIt != aLayoutList.end() -1; ++aIt)
        {
            for( std::vector<LwpPageLayout*>::iterator bIt = aIt +1; bIt != aLayoutList.end(); ++bIt )
            {
                if(!(**aIt < **bIt))
                {
                    std::swap(*aIt, *bIt);
                }
            }
        }
    }

    //put all the sorted  layouts into list
    m_LayoutList.clear();

    for (auto const& layout : aLayoutList)
    {
        m_LayoutList.push_back(layout);
    }
}

/**************************************************************************
 * @descr:  whether need create a new section and reset column in current page layout
**************************************************************************/
bool LwpStory::IsNeedSection()
{
    bool bNewSection = false;
    if(m_pCurrentLayout)
    {
        if(m_pCurrentLayout->HasColumns())
        {
            //get the following pagelayout and its type
            bool bColumns = true;
            LwpPageLayout* pNextLayout = GetNextPageLayout();
            if(pNextLayout)
            {
                //get layout type
                LwpLayout::UseWhenType eWhenType = pNextLayout->GetUseWhenType();
                switch(eWhenType)
                {
                case LwpLayout::StartWithinColume://not support now
                case LwpLayout::StartWithinPage:
                {
                    bColumns =false;
                    bNewSection = true;
                    break;
                }
                case LwpLayout::StartOnNextPage:
                case LwpLayout::StartOnOddPage://not support now
                case LwpLayout::StartOnEvenPage://not support now
                {
                    bColumns =true;
                    bNewSection = false;
                    break;
                }
                default:
                    break;
                }
            }

            //if bColumns is true, the page layout doesn't need columns, set the xfcolumns to NULL in page master
            if(!bColumns)
            {
                m_pCurrentLayout->ResetXFColumns();
            }
        }
        SetPMModified(false);
    }
    return bNewSection;
}
/**************************************************************************
 * @descr:  process frame which anchor type is to cell
**************************************************************************/
void LwpStory::XFConvertFrameInCell(XFContentContainer* pCont)
{
    rtl::Reference<LwpVirtualLayout> xLayout(GetLayout(nullptr));
    while (xLayout.is())
    {
        rtl::Reference<LwpVirtualLayout> xFrameLayout(dynamic_cast<LwpVirtualLayout*>(xLayout->GetChildHead().obj().get()));
        while (xFrameLayout.is())
        {
            if (xFrameLayout->IsAnchorCell() && xFrameLayout->HasContent())
            {
                //get the first xfpara
                rtl::Reference<XFContent> first(
                    pCont->FindFirstContent(enumXFContentPara));
                XFContentContainer* pXFFirtPara = static_cast<XFContentContainer*>(first.get());
                if(pXFFirtPara)
                    xFrameLayout->DoXFConvert(pXFFirtPara);
            }
            xFrameLayout.set(dynamic_cast<LwpVirtualLayout*>(xFrameLayout->GetNext().obj().get()));
        }
        xLayout = GetLayout(xLayout.get());
    }
}

/**************************************************************************
 * @descr:  process frame which anchor type is to page
**************************************************************************/
void LwpStory::XFConvertFrameInPage(XFContentContainer* pCont)
{
    rtl::Reference<LwpVirtualLayout> xLayout(GetLayout(nullptr));
    while (xLayout.is())
    {
        rtl::Reference<LwpVirtualLayout> xFrameLayout(dynamic_cast<LwpVirtualLayout*>(xLayout->GetChildHead().obj().get()));
        while (xFrameLayout.is())
        {
            if( xFrameLayout->IsAnchorPage()
                && (xFrameLayout->IsFrame()
                    || xFrameLayout->IsSuperTable()
                    || xFrameLayout->IsGroupHead()) )
            {
                xFrameLayout->DoXFConvert(pCont);
            }
            xFrameLayout.set(dynamic_cast<LwpVirtualLayout*>(xFrameLayout->GetNext().obj().get()));
        }
        xLayout = GetLayout(xLayout.get());
    }
}
/**************************************************************************
 * @descr:  process frame which anchor type is to frame
**************************************************************************/
void LwpStory::XFConvertFrameInFrame(XFContentContainer* pCont)
{
    rtl::Reference<LwpVirtualLayout> xLayout(GetLayout(nullptr));
    while (xLayout.get())
    {
        rtl::Reference<LwpVirtualLayout> xFrameLayout(dynamic_cast<LwpVirtualLayout*>(xLayout->GetChildHead().obj().get()));
        std::set<LwpVirtualLayout*> aSeen;
        while (xFrameLayout.is())
        {
            aSeen.insert(xFrameLayout.get());
            if (xFrameLayout->IsAnchorFrame())
            {
                xFrameLayout->DoXFConvert(pCont);
            }
            xFrameLayout.set(dynamic_cast<LwpVirtualLayout*>(xFrameLayout->GetNext().obj().get()));
            if (aSeen.find(xFrameLayout.get()) != aSeen.end())
                throw std::runtime_error("loop in register style");
        }
        xLayout = GetLayout(xLayout.get());
    }
}
/**************************************************************************
 * @descr:  process frame which anchor type is to page and the frame is contained by header or footer
**************************************************************************/
void LwpStory::XFConvertFrameInHeaderFooter(XFContentContainer* pCont)
{
    rtl::Reference<LwpVirtualLayout> xLayout(GetLayout(nullptr));
    while (xLayout.is())
    {
        rtl::Reference<LwpVirtualLayout> xFrameLayout(dynamic_cast<LwpVirtualLayout*>(xLayout->GetChildHead().obj().get()));
        while (xFrameLayout.is())
        {
            if (xFrameLayout->IsAnchorPage() && (xLayout->IsHeader() || xLayout->IsFooter()))
            {
                //The frame must be included by <text:p>
                rtl::Reference<XFContent> first(
                    pCont->FindFirstContent(enumXFContentPara));
                XFContentContainer* pXFFirtPara = static_cast<XFContentContainer*>(first.get());
                if(pXFFirtPara)
                    xFrameLayout->DoXFConvert(pXFFirtPara);
            }
            xFrameLayout.set(dynamic_cast<LwpVirtualLayout*>(xFrameLayout->GetNext().obj().get()));
        }
        xLayout = GetLayout(xLayout.get());
    }
}

void LwpStory::AddXFContent(XFContent* pContent)
{
    if (m_xXFContainer)
        m_xXFContainer->Add(pContent);
}

XFContentContainer* LwpStory::GetXFContent()
{
    return m_xXFContainer.get();
}

LwpPara* LwpStory::GetLastParaOfPreviousStory()
{
    rtl::Reference<LwpVirtualLayout> xVLayout(GetLayout(nullptr));
    if (xVLayout.is())
    {
        return xVLayout->GetLastParaOfPreviousStory();
    }

    return nullptr;
}
/**************************************************************************
 * @descr:  get text from story
**************************************************************************/
OUString LwpStory::GetContentText(bool bAllText)
{
    if (bAllText)//convert all text fribs
    {
        OUStringBuffer sText;
        //process para list
        LwpPara* pPara = dynamic_cast<LwpPara*>(GetFirstPara().obj().get());
        while (pPara)
        {
            pPara->SetFoundry(m_pFoundry);
            sText.append(pPara->GetContentText(true));
            pPara = dynamic_cast<LwpPara*>(pPara->GetNext().obj().get());
        }
        return sText.makeStringAndClear();
    }
    else //only the first text frib
    {
        rtl::Reference<LwpObject> pObj = GetFirstPara().obj();
        if(pObj.is())
        {
            LwpPara* pPara = dynamic_cast<LwpPara*>(pObj.get());
            if (!pPara || pPara->GetNext().obj() != nullptr)
                return OUString();
            pPara->SetFoundry(m_pFoundry);
            return pPara->GetContentText();
        }
        return  OUString();
    }

}
OUString LwpStory::RegisterFirstFribStyle()
{
    LwpPara* pPara = dynamic_cast<LwpPara*>(GetFirstPara().obj().get());
    if (!pPara)
        return OUString();
    pPara->SetFoundry(m_pFoundry);
    LwpFribPtr& rFribs = pPara->GetFribs();
    LwpFrib* pFirstFrib = rFribs.GetFribs();
    pFirstFrib->RegisterStyle(m_pFoundry);
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    XFTextStyle* pBaseStyle = pXFStyleManager->FindTextStyle(pFirstFrib->GetStyleName());
    if (pBaseStyle == nullptr)
        return OUString();
    std::unique_ptr<XFTextStyle> pStyle(new XFTextStyle);
    *pStyle = *pBaseStyle;
    OUString sName = "Ruby" + pFirstFrib->GetStyleName();
    pStyle->SetStyleName(sName);
    pXFStyleManager->AddStyle(std::move(pStyle));
    return sName;
}

bool LwpStory::IsBullStyleUsedBefore(const OUString& rStyleName, sal_uInt8 nPos)
{
    return std::any_of(m_vBulletStyleNameList.rbegin(), m_vBulletStyleNameList.rend(),
        [&rStyleName, &nPos](const NamePosPair& rPair) { return rPair.first == rStyleName && rPair.second == nPos; });
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
