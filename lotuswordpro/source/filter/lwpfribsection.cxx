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
 *  the class for section frib and process section and page layout
 ************************************************************************/

#include "lwpfribsection.hxx"
#include "lwpfribbreaks.hxx"
#include "lwpstory.hxx"
#include "lwpsection.hxx"
#include <xfilter/xfstylemanager.hxx>
#include <xfilter/xfsectionstyle.hxx>
#include <xfilter/xfsection.hxx>
#include <xfilter/xfindex.hxx>
#include "lwpfribptr.hxx"
#include <lwpglobalmgr.hxx>

LwpFribSection::LwpFribSection(LwpPara *pPara)
     : LwpFrib(pPara)
{

}

LwpFribSection::~LwpFribSection()
{
}

/**
 * @descr:  Read section frib information
 *
 */
void LwpFribSection::Read(LwpObjectStream *pObjStrm, sal_uInt16 /*len*/)
{
    m_Section.ReadIndexed(pObjStrm);
}

/**
 * @descr:  Get section pointer
 *
 */
LwpSection* LwpFribSection::GetSection()
{
    return dynamic_cast<LwpSection*>(m_Section.obj().get());
}

/**
 * @descr:  Register section style
 *
 */
void LwpFribSection::RegisterSectionStyle()
{
    LwpPageLayout* pLayout = GetPageLayout();
    if(pLayout)
    {
        m_pMasterPage.reset( new LwpMasterPage(m_pPara, pLayout) );
        m_pMasterPage->RegisterMasterPage(this);
    }
}

/**
 * @descr:  Register section style
 *
 */
void LwpFribSection::SetSectionName()
{
    LwpSection* pSection = GetSection();
    if (!pSection)
        return;
    LwpStory* pStory = dynamic_cast<LwpStory*>(m_pPara->GetStoryID().obj().get());
    if (!pStory)
        return;
    pStory->SetSectionName(pSection->GetSectionName());
}

/**
 * @descr:  Get page layout that current section points
 *
 */
LwpPageLayout* LwpFribSection::GetPageLayout()
{
    if(GetSection())
        return GetSection()->GetPageLayout();
    return nullptr;
}

/**
 * @descr:  XFConvert section
 *
 */
void LwpFribSection::ParseSection()
{
    LwpPageLayout* pLayout = GetPageLayout();
    if(pLayout)
    {
        // StartWithinColume not support now
        LwpLayout::UseWhenType eSectionType = pLayout->GetUseWhenType();
        if(eSectionType==LwpLayout::StartWithinColume)
        {
            return;
        }
        if(m_pMasterPage)
        {
            m_pMasterPage->ParseSection(this);
        }
    }
    else if (LwpStory* pStory = dynamic_cast<LwpStory*>(m_pPara->GetStoryID().obj().get()))
    {
        rtl::Reference<LwpObject> xObj(m_Section.obj());
        if (xObj.is() && xObj->GetTag() == VO_INDEXSECTION)
        {
            //create a new section and add it to container
            XFIndex* pIndex = new XFIndex;
            pIndex->SetIndexType(enumXFIndexAlphabetical);
            SetDefaultAlphaIndex(pIndex);

            pStory->AddXFContent( pIndex );
            m_pPara->SetXFContainer( pIndex );
        }
        else
        {
            XFContentContainer* pContent = pStory->GetXFContent();
            m_pPara->SetXFContainer( pContent );
        }
    }
}

void LwpFribSection::SetDefaultAlphaIndex(XFIndex * pXFIndex)
{
    LwpFoundry* pFoundry = m_pPara->GetFoundry();
    OUString styleName = pFoundry->FindActuralStyleName("Separator");

    LwpIndexSection* pIndexSection = dynamic_cast<LwpIndexSection*>(m_Section.obj().get());
    XFIndexTemplate * pTemplateSep = new XFIndexTemplate();
    if (pIndexSection && pIndexSection->IsFormatSeparator())
    {
        pXFIndex->SetSeparator(true);
        pTemplateSep->AddEntry(enumXFIndexTemplateText,"");
    }
    //pXFIndex->AddTemplate("separator","Separator",pTemplateSep);
    pXFIndex->AddTemplate("separator",styleName,pTemplateSep);

    styleName = pFoundry->FindActuralStyleName("Primary");

    XFIndexTemplate * pTemplate1 = new XFIndexTemplate();
    pTemplate1->AddEntry(enumXFIndexTemplateText,"");
    pTemplate1->AddEntry(enumXFIndexTemplateTab,"");
    pTemplate1->AddEntry(enumXFIndexTemplatePage,"");
    //pXFIndex->AddTemplate(OUString::number(1),"Primary",pTemplate1);
    pXFIndex->AddTemplate(OUString::number(1),styleName,pTemplate1);

    XFIndexTemplate * pTemplate2 = new XFIndexTemplate();
    pTemplate2->AddEntry(enumXFIndexTemplateText,"");
    pTemplate2->AddEntry(enumXFIndexTemplateTab,"");
    pTemplate2->AddEntry(enumXFIndexTemplatePage,"");
    XFIndexTemplate * pTemplate3 = new XFIndexTemplate();
    pTemplate3->AddEntry(enumXFIndexTemplateText,"");
    pTemplate3->AddEntry(enumXFIndexTemplateTab,"");
    pTemplate3->AddEntry(enumXFIndexTemplatePage,"");

    if (pIndexSection && pIndexSection->IsFormatRunin())
    {
        //pXFIndex->AddTemplate(OUString::number(2),"Primary",pTemplate2);
        //pXFIndex->AddTemplate(OUString::number(3),"Primary",pTemplate3);
        pXFIndex->AddTemplate(OUString::number(2),styleName,pTemplate2);
        pXFIndex->AddTemplate(OUString::number(3),styleName,pTemplate3);
    }
    else
    {
        //pXFIndex->AddTemplate(OUString::number(2),"Secondary",pTemplate2);
        //pXFIndex->AddTemplate(OUString::number(3),"Secondary",pTemplate3);
        styleName = pFoundry->FindActuralStyleName("Secondary");
        pXFIndex->AddTemplate(OUString::number(2),styleName,pTemplate2);
        pXFIndex->AddTemplate(OUString::number(3),styleName,pTemplate3);
    }
}

LwpMasterPage::LwpMasterPage(LwpPara* pPara, LwpPageLayout* pLayout)
    :m_bNewSection(false),m_pPara(pPara),m_pLayout(pLayout)
{
}

/**
 * @descr:  Register master page style for para style and register section style if necessary
 *
 */
void LwpMasterPage::RegisterMasterPage(LwpFrib* pFrib)
{
    //if there is no other frib after current frib, register master page in starting para of next page
    LwpStory* pStory = nullptr;
    if (IsNextPageType()&&(!pFrib->HasNextFrib()))
        pStory = dynamic_cast<LwpStory*>(m_pPara->GetStoryID().obj().get());

    if (pStory)
    {
        pStory->SetCurrentLayout(m_pLayout);
        RegisterFillerPageStyle();
        return;
    }

    m_bNewSection = false;
    //sal_Bool bSectionColumns = sal_False;

    std::unique_ptr<XFParaStyle> xOverStyle(new XFParaStyle);
    *xOverStyle = *(m_pPara->GetXFParaStyle());
    xOverStyle->SetStyleName("");

    LwpLayout::UseWhenType eUserType = m_pLayout->GetUseWhenType();
    switch(eUserType)
    {
        case LwpLayout::StartWithinColume://not support now
        {
            m_bNewSection = false;
            break;
        }
        case LwpLayout::StartWithinPage:
        {
            m_bNewSection = true;
            //bSectionColumns = sal_True;
            break;
        }
        case LwpLayout::StartOnNextPage://fall through
        case LwpLayout::StartOnOddPage: //fall through
        case LwpLayout::StartOnEvenPage:
        {
            pStory = dynamic_cast<LwpStory*>(m_pPara->GetStoryID().obj().get());
            if (pStory)
            {
                pStory->SetCurrentLayout(m_pLayout);
                //get odd page layout when the current pagelayout is mirror
                m_pLayout = pStory->GetCurrentLayout();
                m_bNewSection = IsNeedSection();
                //bSectionColumns = m_bNewSection;
                xOverStyle->SetMasterPage(m_pLayout->GetStyleName());
                RegisterFillerPageStyle();
            }
            break;
        }
        default:
            break;
    }

    //register tab style;
    pStory = dynamic_cast<LwpStory*>(m_pPara->GetStoryID().obj().get());
    if (!pStory)
        return;

    pStory->SetTabLayout(m_pLayout);
    m_pPara->RegisterTabStyle(xOverStyle.get());

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_StyleName = pXFStyleManager->AddStyle(std::move(xOverStyle)).m_pStyle->GetStyleName();
    //register section style here
    if(m_bNewSection)
    {
        std::unique_ptr<XFSectionStyle> pSectStyle(new XFSectionStyle());
        //set margin
        pStory = dynamic_cast<LwpStory*>(m_pPara->GetStoryID().obj().get());
        LwpPageLayout* pCurrentLayout = pStory ? pStory->GetCurrentLayout() : nullptr;
        if (pCurrentLayout)
        {
            double fLeft = m_pLayout->GetMarginsValue(MARGIN_LEFT) - pCurrentLayout->GetMarginsValue(MARGIN_LEFT);
            double fRight = m_pLayout->GetMarginsValue(MARGIN_RIGHT) - pCurrentLayout->GetMarginsValue(MARGIN_RIGHT);
            pSectStyle->SetMarginLeft(fLeft);
            pSectStyle->SetMarginRight(fRight);
        }

        XFColumns* pColumns = m_pLayout->GetXFColumns();
        if(pColumns)
        {
            pSectStyle->SetColumns(pColumns);
        }
        m_SectionStyleName = pXFStyleManager->AddStyle(std::move(pSectStyle)).m_pStyle->GetStyleName();
    }
}

/**
 * @descr:  Whether it need create a new section
 *
 */
bool LwpMasterPage::IsNeedSection()
{
    bool bNewSection = false;
    //get story
    LwpStory* pStory = dynamic_cast<LwpStory*>(m_pPara->GetStoryID().obj().get());
    //if pagelayout is modified, register the pagelayout
    if (pStory && pStory->IsPMModified())
    {
        bNewSection = pStory->IsNeedSection();
    }
    return bNewSection;
}

/**
 * @descr:  Create XFSection if necessary
 *
 */
XFSection* LwpMasterPage::CreateXFSection()
{
    if(m_bNewSection)
    {
        //new a section
        XFSection* pXFSection = new XFSection();
        pXFSection->SetStyleName(m_SectionStyleName);
        return pXFSection;
    }
    return nullptr;
}

/**
 * @descr:  Parse section
 *
 */
void LwpMasterPage::ParseSection(LwpFrib* pFrib)
{
    LwpFribPtr& rFribPtr = m_pPara->GetFribs();
    //XFParagraph * pXFPara = rFribPtr.GetXFPara();

    //parse fillerpage text
    if(m_pLayout->HasFillerPageText(m_pPara->GetFoundry()))
    {
        XFParagraph *pPara = new XFParagraph();
        pPara->SetStyleName(m_FillerPageStyleName);
        m_pPara->AddXFContent(pPara);
        rFribPtr.SetXFPara(pPara);

        m_pLayout->ConvertFillerPageText(m_pPara->GetXFContainer());
    }
    //create a new section and add it to container
    XFContentContainer* pContent = CreateXFSection();
    if(pContent)
    {
        LwpStory* pStory = dynamic_cast<LwpStory*> ( m_pPara->GetStoryID().obj().get() );
        //delete the additional blank para
        XFParagraph* pCurrPara = rFribPtr.GetXFPara();
        if(!pCurrPara->HasContents())
        {
            XFContentContainer* pCurrContainer = m_pPara->GetXFContainer();
            if(pFrib->HasNextFrib() && (pCurrContainer->GetLastContent() == pCurrPara))
            {
                pCurrContainer->RemoveLastContent();
            }
        }
        if (pStory)
            pStory->AddXFContent( pContent );
    }
    else
    {
        LwpStory* pStory = dynamic_cast<LwpStory*> ( m_pPara->GetStoryID().obj().get() );
        pContent = pStory ? pStory->GetXFContent() : nullptr;
    }
    if(pContent)
    {
        m_pPara->SetXFContainer( pContent );
    }
    //out put the contents after the section frib in the same para.
    if(pFrib->HasNextFrib())
    {
        XFParagraph *pNextPara = new XFParagraph();
        pNextPara->SetStyleName(m_StyleName);
        m_pPara->AddXFContent(pNextPara);
        rFribPtr.SetXFPara(pNextPara);
    }

}

/**
 * @descr:  Register filler page text style
 *
 */
void LwpMasterPage::RegisterFillerPageStyle()
{
    LwpLayout::UseWhenType eUserType = m_pLayout->GetUseWhenType();
    if(eUserType==LwpLayout::StartOnOddPage
            ||eUserType==LwpLayout::StartOnEvenPage)
    {
        if(m_pLayout->HasFillerPageText(m_pPara->GetFoundry()))
        {
            std::unique_ptr<XFParaStyle> pPagebreakStyle(new XFParaStyle);
            *pPagebreakStyle = *(m_pPara->GetXFParaStyle());
            pPagebreakStyle->SetStyleName("");
            pPagebreakStyle->SetBreaks(enumXFBreakAftPage);
            XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
            m_FillerPageStyleName= pXFStyleManager->AddStyle(std::move(pPagebreakStyle)).m_pStyle->GetStyleName();
        }
    }
}

/**
 * @descr:  Whether the layout is next page type
 *
 */
bool LwpMasterPage::IsNextPageType()
{
    LwpLayout::UseWhenType eUserType = m_pLayout->GetUseWhenType();
    return eUserType == LwpLayout::StartOnNextPage
        || eUserType == LwpLayout::StartOnOddPage
        || eUserType == LwpLayout::StartOnEvenPage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
