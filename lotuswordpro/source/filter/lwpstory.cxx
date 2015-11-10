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

#include "lwpglobalmgr.hxx"
#include "lwpstory.hxx"
#include "xfilter/xfstylemanager.hxx"

LwpStory::LwpStory(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpContent(objHdr, pStrm)
    , m_bPMModified(false)
    , m_pCurrentLayout(nullptr)
    , m_pTabLayout(nullptr)
    , m_bDropcap(false)
    , m_pHyperlinkMgr(new LwpHyperlinkMgr)
    , m_pXFContainer(nullptr)
{
}

LwpStory::~LwpStory()
{
    delete m_pHyperlinkMgr;
}

void LwpStory::Read()
{
    LwpContent::Read();
    m_ParaList.Read(m_pObjStrm);
    m_FirstParaStyle.ReadIndexed(m_pObjStrm);
    m_pObjStrm->SkipExtra();
}

#include "lwppara.hxx"
#include "lwpobjfactory.hxx"

/**************************************************************************
 * @descr:   Convert all the contents in current story
**************************************************************************/
void LwpStory::XFConvert(XFContentContainer* pCont)
{
    //process frame which anchor frame
    XFConvertFrameInFrame(pCont);
    //process para list
    XFContentContainer* pParaCont = pCont;
    LwpPara* pPara = dynamic_cast<LwpPara*> ( GetFirstPara().obj().get() );
    while(pPara)
    {
        pPara->SetFoundry(m_pFoundry);
        pPara->XFConvert(pParaCont);

        //Get the xfcontainer for the next para
        pParaCont = pPara->GetXFContainer();
        pPara = dynamic_cast<LwpPara*> ( pPara->GetNext().obj().get() );
    }

    //process frame which anchor is to cell after converter all the para
    XFConvertFrameInCell(pCont);
    XFConvertFrameInHeaderFooter(pCont);

    //Release Lwp Objects
    LwpPara* pCur = dynamic_cast<LwpPara*> (GetFirstPara().obj().get());
    LwpPara* pNext;
    while(pCur)
    {
        pNext = dynamic_cast<LwpPara*> ( pCur->GetNext().obj().get() );
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpObjectFactory* pObjMgr = pGlobal->GetLwpObjFactory();
        pObjMgr->ReleaseObject(pCur->GetObjectID());
        pCur = pNext;
    }
}

void LwpStory::RegisterStyle()
{
    LwpPara* pPara = dynamic_cast<LwpPara*>( GetFirstPara().obj().get() );
    while(pPara)
    {
        pPara->SetFoundry(m_pFoundry);
        pPara->RegisterStyle();
        pPara = dynamic_cast<LwpPara*>(pPara->GetNext().obj().get());
    }
}

void LwpStory::Parse(IXFStream* pOutputStream)
{
    m_pXFContainer = new XFContentContainer;
    XFConvert(m_pXFContainer);
    m_pXFContainer->ToXml(pOutputStream);
    delete m_pXFContainer;
    m_pXFContainer = nullptr;

    //It seems that, currently, we do not need to process the child story
    /*LwpObject* pChildStory = GetFirstStory()->obj();
    if(pChildStory)
    {
        pChildStory->SetFoundry(m_pFoundry);
        pChildStory->Parse(pOutputStream);
    }*/

    //Don't process the next story
/*  LwpObject* pNextStory = GetNextStory()->obj();
    if(pNextStory)
    {
        pNextStory->SetFoundry(m_pFoundry);
        pNextStory->Parse(pOutputStream);
    }*/

}

#include "lwppagelayout.hxx"
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

void LwpStory::AddPageLayout(LwpPageLayout * pObject)
{
    m_LayoutList.push_back(pObject);
}
/**************************************************************************
 * @descr:   Get the next page layout relative to m_pCurrentLayout
**************************************************************************/
LwpPageLayout* LwpStory::GetNextPageLayout()
{
    std::vector<LwpPageLayout*>::iterator it;
    for( it = m_LayoutList.begin(); it != m_LayoutList.end(); ++it )
    {
        if(m_pCurrentLayout == *it)
        {
            if((it+1) !=m_LayoutList.end())
            {
                return *(it+1);
            }
        }
    }
    return nullptr;
}
/**************************************************************************
 * @descr:   Sort the pagelayout according to their position
**************************************************************************/
void LwpStory::SortPageLayout()
{
    //Get all the pagelayout and store in list
    std::vector<LwpPageLayout*>  aLayoutList;
    LwpVirtualLayout* pLayout = GetLayout(nullptr);
    while(pLayout)
    {
        if(pLayout->IsPage())
        {
            LwpLayout::UseWhenType eSectionType = static_cast<LwpPageLayout*>(pLayout)->GetUseWhenType();
            //for mirror page, the child is pagelayout
            LwpVirtualLayout* pParent = pLayout->GetParentLayout();
            if(eSectionType != LwpLayout::StartWithinColume && pParent && !pParent->IsPage())
            {
                aLayoutList.push_back(static_cast<LwpPageLayout*>(pLayout));
            }
        }
        pLayout = GetLayout(pLayout);
    }
    // sort the pagelayout according to their position
    std::vector<LwpPageLayout*>::iterator aIt;
    if (!aLayoutList.empty())
    {
        for( aIt = aLayoutList.begin(); aIt != aLayoutList.end() -1; ++aIt)
        {
            for( std::vector<LwpPageLayout*>::iterator bIt = aIt +1; bIt != aLayoutList.end(); ++bIt )
            {
                if(**aIt < **bIt)
                {
                    continue;
                }
                else
                {
                    LwpPageLayout* pTemp = *aIt;
                    *aIt = *bIt;
                    *bIt = pTemp;
                }
            }
        }
    }

    //put all the sorted  layouts into list
    m_LayoutList.clear();

    for( aIt = aLayoutList.begin(); aIt != aLayoutList.end(); ++aIt)
    {
        m_LayoutList.push_back(*aIt);
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
    LwpVirtualLayout* pLayout = GetLayout(nullptr);
    while(pLayout)
    {
        LwpVirtualLayout* pFrameLayout = dynamic_cast<LwpVirtualLayout*>(pLayout->GetChildHead().obj().get());
        while(pFrameLayout)
        {

            //if(pFrameLayout->IsAnchorCell())
            if(pFrameLayout->IsAnchorCell() && pFrameLayout->HasContent())
            {
                //get the first xfpara
                rtl::Reference<XFContent> first(
                    pCont->FindFirstContent(enumXFContentPara));
                XFContentContainer* pXFFirtPara = static_cast<XFContentContainer*>(first.get());
                if(pXFFirtPara)
                    pFrameLayout->XFConvert(pXFFirtPara);
            }
            pFrameLayout = dynamic_cast<LwpVirtualLayout*>(pFrameLayout->GetNext().obj().get());
        }
        pLayout = GetLayout(pLayout);
    }
}

/**************************************************************************
 * @descr:  process frame which anchor type is to page
**************************************************************************/
void LwpStory::XFConvertFrameInPage(XFContentContainer* pCont)
{
    LwpVirtualLayout* pLayout = GetLayout(nullptr);
    while(pLayout)
    {
        LwpVirtualLayout* pFrameLayout = dynamic_cast<LwpVirtualLayout*>(pLayout->GetChildHead().obj().get());
        while(pFrameLayout)
        {
            if((pFrameLayout->IsAnchorPage()
                &&(pFrameLayout->IsFrame()
                      ||pFrameLayout->IsSuperTable()
                      ||pFrameLayout->IsGroupHead())))
            {
                pFrameLayout->XFConvert(pCont);
            }
            pFrameLayout = dynamic_cast<LwpVirtualLayout*>(pFrameLayout->GetNext().obj().get());
        }
        pLayout = GetLayout(pLayout);
    }
}
/**************************************************************************
 * @descr:  process frame which anchor type is to frame
**************************************************************************/
void LwpStory::XFConvertFrameInFrame(XFContentContainer* pCont)
{
    LwpVirtualLayout* pLayout = GetLayout(nullptr);
    while(pLayout)
    {
        LwpVirtualLayout* pFrameLayout = dynamic_cast<LwpVirtualLayout*>(pLayout->GetChildHead().obj().get());
        while(pFrameLayout)
        {
            if(pFrameLayout->IsAnchorFrame())
            {
                pFrameLayout->XFConvert(pCont);
            }
            pFrameLayout = dynamic_cast<LwpVirtualLayout*>(pFrameLayout->GetNext().obj().get());
        }
        pLayout = GetLayout(pLayout);
    }
}
/**************************************************************************
 * @descr:  process frame which anchor type is to page and the frame is contained by header or footer
**************************************************************************/
void LwpStory::XFConvertFrameInHeaderFooter(XFContentContainer* pCont)
{
    LwpVirtualLayout* pLayout = GetLayout(nullptr);
    while(pLayout)
    {
        LwpVirtualLayout* pFrameLayout = dynamic_cast<LwpVirtualLayout*>(pLayout->GetChildHead().obj().get());
        while(pFrameLayout)
        {
            if(pFrameLayout->IsAnchorPage() && (pLayout->IsHeader() || pLayout->IsFooter()))
            {
                //The frame must be included by <text:p>
                rtl::Reference<XFContent> first(
                    pCont->FindFirstContent(enumXFContentPara));
                XFContentContainer* pXFFirtPara = static_cast<XFContentContainer*>(first.get());
                if(pXFFirtPara)
                    pFrameLayout->XFConvert(pXFFirtPara);
            }
            pFrameLayout = dynamic_cast<LwpVirtualLayout*>(pFrameLayout->GetNext().obj().get());
        }
        pLayout = GetLayout(pLayout);
    }
}

void LwpStory::AddXFContent(XFContent* pContent)
{
    if(m_pXFContainer)
        m_pXFContainer->Add(pContent);
}

XFContentContainer* LwpStory::GetXFContent()
{
    if(m_pXFContainer)
        return m_pXFContainer;
    else
        return nullptr;
}

LwpPara* LwpStory::GetLastParaOfPreviousStory()
{
    LwpVirtualLayout* pVLayout = this->GetLayout(nullptr);
    if (pVLayout)
    {
        return pVLayout->GetLastParaOfPreviousStory();
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
        OUString sText("");
        //process para list
        LwpPara* pPara = dynamic_cast<LwpPara*>(GetFirstPara().obj().get());
        while (pPara)
        {
            pPara->SetFoundry(m_pFoundry);
            sText += pPara->GetContentText(true);
            pPara = dynamic_cast<LwpPara*>(pPara->GetNext().obj().get());
        }
        return sText;
    }
    else //only the first text frib
    {
        rtl::Reference<LwpObject> pObj = GetFirstPara().obj();
        if(pObj.is())
        {
            LwpPara* pPara = dynamic_cast<LwpPara*>(pObj.get());
            if (!pPara || pPara->GetNext().obj() != nullptr)
                return OUString("");
            pPara->SetFoundry(m_pFoundry);
            return pPara->GetContentText();
        }
        return  OUString("");
    }

}
OUString LwpStory::RegisterFirstFribStyle()
{
    LwpPara* pPara = dynamic_cast<LwpPara*>(GetFirstPara().obj().get());
    if (!pPara)
        return OUString("");
    pPara->SetFoundry(m_pFoundry);
    LwpFribPtr& rFribs = pPara->GetFribs();
    LwpFrib* pFirstFrib = rFribs.GetFribs();
    pFirstFrib->RegisterStyle(m_pFoundry);
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    XFTextStyle* pBaseStyle = pXFStyleManager->FindTextStyle(pFirstFrib->GetStyleName());
    if (pBaseStyle == nullptr)
        return OUString("");
    XFTextStyle* pStyle = new XFTextStyle;
    *pStyle = *pBaseStyle;
    OUString sName = "Ruby" + pFirstFrib->GetStyleName();
    pStyle->SetStyleName(sName);
    pXFStyleManager->AddStyle(pStyle);
    return sName;
}

bool LwpStory::IsBullStyleUsedBefore(const OUString& rStyleName, const sal_uInt8& nPos)
{
    std::vector <NamePosPair>::reverse_iterator rIter;
    for (rIter = m_vBulletStyleNameList.rbegin(); rIter != m_vBulletStyleNameList.rend(); ++rIter)
    {
        OUString aName = (*rIter).first;
        sal_uInt8 nPosition = (*rIter).second;
        if (aName == rStyleName && nPosition == nPos)
        {
            return true;
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
