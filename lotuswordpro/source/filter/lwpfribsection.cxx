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
/*************************************************************************
 * Change History
Jan 2005		Created
 ************************************************************************/
#include "lwpfribsection.hxx"
#include "lwpfribbreaks.hxx"
#include "lwpstory.hxx"
#include "lwpsection.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xfsectionstyle.hxx"
#include "xfilter/xfsection.hxx"
#include "xfilter/xfindex.hxx"
#include "lwpfribptr.hxx"
#include "lwpglobalmgr.hxx"

LwpFribSection::LwpFribSection(LwpPara *pPara)
     : LwpFrib(pPara),m_pMasterPage(NULL)
{

}

LwpFribSection::~LwpFribSection()
{
    if(m_pMasterPage)
    {
        delete m_pMasterPage;
        m_pMasterPage = NULL;
    }
}

/**
 * @descr:	Read section frib information
 *
 */
void LwpFribSection::Read(LwpObjectStream *pObjStrm, sal_uInt16 /*len*/)
{
    m_Section.ReadIndexed(pObjStrm);
}

/**
 * @descr:	Get section pointer
 *
 */
LwpSection* LwpFribSection::GetSection()
{
    return static_cast<LwpSection*>(m_Section.obj());
}

/**
 * @descr:	Register section style
 *
 */
void LwpFribSection::RegisterSectionStyle()
{
    LwpPageLayout* pLayout = GetPageLayout();
    if(pLayout)
    {
        m_pMasterPage = new LwpMasterPage(m_pPara, pLayout);
        m_pMasterPage->RegisterMasterPage(this);
    }
}

/**
 * @descr:	Register section style
 *
 */
void LwpFribSection::SetSectionName()
{
    LwpSection* pSection = GetSection();
    if(pSection)
    {
        LwpStory* pStory = static_cast<LwpStory*>(m_pPara->GetStoryID()->obj());
        pStory->SetSectionName(pSection->GetSectionName());
    }
}

/**
 * @descr:	Get page layout that current section points
 *
 */
LwpPageLayout* LwpFribSection::GetPageLayout()
{
    if(GetSection())
        return GetSection()->GetPageLayout();
    return NULL;
}

/**
 * @descr:	XFConvert section
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
    else
    {
        LwpStory* pStory = static_cast<LwpStory*> ( m_pPara->GetStoryID()->obj() );
        if (m_Section.obj()->GetTag() == VO_INDEXSECTION)
        {
            //create a new section and add it to container
            XFIndex* pIndex = new XFIndex;
            pIndex->SetIndexType(enumXFIndexAlphabetical);
            /*
            sal_Bool bRunin = sal_False;
            sal_Bool bSeparator = sal_False;
            LwpIndexSection* pIndexSection = static_cast<LwpIndexSection*>(m_Section.obj());
            if (pIndexSection->IsFormatRunin())
                bRunin = sal_True;
            if (pIndexSection->IsFormatSeparator())
                bSeparator = sal_True;
            pIndex->SetDefaultAlphaIndex(A2OUSTR(""),bRunin,bSeparator);
            */
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
    OUString styleName = pFoundry->FindActuralStyleName(A2OUSTR("Separator"));

    LwpIndexSection* pIndexSection = static_cast<LwpIndexSection*>(m_Section.obj());
    XFIndexTemplate * pTemplateSep = new XFIndexTemplate();
    if (pIndexSection->IsFormatSeparator())
    {
        pXFIndex->SetSeparator(sal_True);
        pTemplateSep->AddEntry(enumXFIndexTemplateText,A2OUSTR(""));
    }
    //pXFIndex->AddTemplate(A2OUSTR("separator"),A2OUSTR("Separator"),pTemplateSep);
    pXFIndex->AddTemplate(A2OUSTR("separator"),styleName,pTemplateSep);


    styleName = pFoundry->FindActuralStyleName(A2OUSTR("Primary"));

    XFIndexTemplate * pTemplate1 = new XFIndexTemplate();
    pTemplate1->AddEntry(enumXFIndexTemplateText,A2OUSTR(""));
    pTemplate1->AddEntry(enumXFIndexTemplateTab,A2OUSTR(""));
    pTemplate1->AddEntry(enumXFIndexTemplatePage,A2OUSTR(""));
    //pXFIndex->AddTemplate(Int32ToOUString(1),A2OUSTR("Primary"),pTemplate1);
    pXFIndex->AddTemplate(Int32ToOUString(1),styleName,pTemplate1);

    XFIndexTemplate * pTemplate2 = new XFIndexTemplate();
    pTemplate2->AddEntry(enumXFIndexTemplateText,A2OUSTR(""));
    pTemplate2->AddEntry(enumXFIndexTemplateTab,A2OUSTR(""));
    pTemplate2->AddEntry(enumXFIndexTemplatePage,A2OUSTR(""));
    XFIndexTemplate * pTemplate3 = new XFIndexTemplate();
    pTemplate3->AddEntry(enumXFIndexTemplateText,A2OUSTR(""));
    pTemplate3->AddEntry(enumXFIndexTemplateTab,A2OUSTR(""));
    pTemplate3->AddEntry(enumXFIndexTemplatePage,A2OUSTR(""));

    if (pIndexSection->IsFormatRunin())
    {
        //pXFIndex->AddTemplate(Int32ToOUString(2),A2OUSTR("Primary"),pTemplate2);
        //pXFIndex->AddTemplate(Int32ToOUString(3),A2OUSTR("Primary"),pTemplate3);
        pXFIndex->AddTemplate(Int32ToOUString(2),styleName,pTemplate2);
        pXFIndex->AddTemplate(Int32ToOUString(3),styleName,pTemplate3);
    }
    else
    {
        //pXFIndex->AddTemplate(Int32ToOUString(2),A2OUSTR("Secondary"),pTemplate2);
        //pXFIndex->AddTemplate(Int32ToOUString(3),A2OUSTR("Secondary"),pTemplate3);
        styleName = pFoundry->FindActuralStyleName(A2OUSTR("Secondary"));
        pXFIndex->AddTemplate(Int32ToOUString(2),styleName,pTemplate2);
        pXFIndex->AddTemplate(Int32ToOUString(3),styleName,pTemplate3);
    }
}


LwpMasterPage::LwpMasterPage(LwpPara* pPara, LwpPageLayout* pLayout)
    :m_bNewSection(sal_False),m_pPara(pPara),m_pLayout(pLayout)
{
}

/**
 * @descr:	Register master page style for para style and register section style if necessary
 *
 */
sal_Bool LwpMasterPage::RegisterMasterPage(LwpFrib* pFrib)
{
    //if there is no other frib after current firb, register master page in starting para of next page
    if(IsNextPageType()&&(!pFrib->HasNextFrib()))
    {
        LwpStory* pStory = static_cast<LwpStory*>(m_pPara->GetStoryID()->obj());
        pStory->SetCurrentLayout(m_pLayout);
        RegisterFillerPageStyle();
        return sal_False;
    }

    m_bNewSection = sal_False;
    //sal_Bool bSectionColumns = sal_False;

    XFParaStyle* pOverStyle = new XFParaStyle;
    *pOverStyle = *(m_pPara->GetXFParaStyle());
    pOverStyle->SetStyleName(A2OUSTR(""));

    LwpLayout::UseWhenType eUserType = m_pLayout->GetUseWhenType();
    switch(eUserType)
    {
        case LwpLayout::StartWithinColume://not support now
        {
            m_bNewSection = sal_False;
            break;
        }
        case LwpLayout::StartWithinPage:
        {
            m_bNewSection = sal_True;
            //bSectionColumns = sal_True;
            break;
        }
        case LwpLayout::StartOnNextPage:	//fall throught
        case LwpLayout::StartOnOddPage:	//fall throught
        case LwpLayout::StartOnEvenPage:
        {
            LwpStory* pStory = static_cast<LwpStory*>(m_pPara->GetStoryID()->obj());
            pStory->SetCurrentLayout(m_pLayout);
            //get odd page layout when the current pagelayout is mirror
            m_pLayout = pStory->GetCurrentLayout();
            m_bNewSection = IsNeedSection();
            //bSectionColumns = m_bNewSection;
            pOverStyle->SetMasterPage( m_pLayout->GetStyleName());
            RegisterFillerPageStyle();
            break;
        }
        default:
            break;
    }

    //register tab style;
    LwpStory* pStory = static_cast<LwpStory*>(m_pPara->GetStoryID()->obj());
    pStory->SetTabLayout(m_pLayout);
    m_pPara->RegisterTabStyle(pOverStyle);

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_StyleName = pXFStyleManager->AddStyle(pOverStyle)->GetStyleName();
    //register section style here
    if(m_bNewSection)
    {
        XFSectionStyle* pSectStyle= new XFSectionStyle();
        //set margin
        pStory = static_cast<LwpStory*>(m_pPara->GetStoryID()->obj());
        if(pStory)
        {
            LwpPageLayout* pCurrentLayout = pStory->GetCurrentLayout();
            double fLeft = m_pLayout->GetMarginsValue(MARGIN_LEFT)- pCurrentLayout->GetMarginsValue(MARGIN_LEFT);
            double fRight = m_pLayout->GetMarginsValue(MARGIN_RIGHT)- pCurrentLayout->GetMarginsValue(MARGIN_RIGHT);
            pSectStyle->SetMarginLeft(fLeft);
            pSectStyle->SetMarginRight(fRight);
        }

        //if(bSectionColumns)
        //{
            //set columns
            XFColumns* pColumns = m_pLayout->GetXFColumns();
            if(pColumns)
            {
                pSectStyle->SetColumns(pColumns);
            }
        //}
        m_SectionStyleName = pXFStyleManager->AddStyle(pSectStyle)->GetStyleName();
    }
    return sal_False;
}

/**
 * @descr:	Whether it need create a new section
 *
 */
sal_Bool LwpMasterPage::IsNeedSection()
{
    sal_Bool bNewSection = sal_False;
    //get story
    LwpStory* pStory = static_cast<LwpStory*>(m_pPara->GetStoryID()->obj());
    //if pagelayout is modified, register the pagelayout
    if(pStory->IsPMModified())
    {
        bNewSection = pStory->IsNeedSection();
    }
    return bNewSection;
}

/**
 * @descr:	Create XFSection if necessary
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
    return NULL;
}

/**
 * @descr:	Parse section
 *
 */
void LwpMasterPage::ParseSection(LwpFrib* pFrib)
{
    LwpFribPtr* pFribPtr = m_pPara->GetFribs();
    //XFParagraph * pXFPara = pFribPtr->GetXFPara();

    //parse fillerpage text
    if(m_pLayout->HasFillerPageText(m_pPara->GetFoundry()))
    {
        XFParagraph *pPara = new XFParagraph();
        pPara->SetStyleName(GetFillerPageStyleName());
        m_pPara->AddXFContent(pPara);
        pFribPtr->SetXFPara(pPara);

        m_pLayout->ConvertFillerPageText(m_pPara->GetXFContainer());
    }
    //create a new section and add it to container
    XFContentContainer* pContent = CreateXFSection();
    if(pContent)
    {
        LwpStory* pStory = static_cast<LwpStory*> ( m_pPara->GetStoryID()->obj() );
        //delete the additional blank para, 06/28/2005
        XFParagraph* pCurrPara = pFribPtr->GetXFPara();
        if(!pCurrPara->HasContents())
        {
            XFContentContainer* pCurrContainer = m_pPara->GetXFContainer();
            if(pFrib->HasNextFrib() && (pCurrContainer->GetLastContent() == pCurrPara))
            {
                pCurrContainer->RemoveLastContent();
            }
        }
        //end,06/28/2005
        pStory->AddXFContent( pContent );
    }
    else
    {
        LwpStory* pStory = static_cast<LwpStory*> ( m_pPara->GetStoryID()->obj() );
        pContent = pStory->GetXFContent();
    }
    if(pContent)
    {
        m_pPara->SetXFContainer( pContent );
    }
    //out put the contents after the section frib in the same para.
    if(pFrib->HasNextFrib())
    {
        XFParagraph *pNextPara = new XFParagraph();
        pNextPara->SetStyleName(GetStyleName());
        m_pPara->AddXFContent(pNextPara);
        pFribPtr->SetXFPara(pNextPara);
    }

}

/**
 * @descr:	Register filler page text style
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
            XFParaStyle* pPagebreakStyle = new XFParaStyle;
            *pPagebreakStyle = *(m_pPara->GetXFParaStyle());
            pPagebreakStyle->SetStyleName(A2OUSTR(""));
            pPagebreakStyle->SetBreaks(enumXFBreakAftPage);
            XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
            m_FillerPageStyleName= pXFStyleManager->AddStyle(pPagebreakStyle)->GetStyleName();
        }
    }
}

/**
 * @descr:	Whether the layout is next page type
 *
 */
sal_Bool LwpMasterPage::IsNextPageType()
{
    LwpLayout::UseWhenType eUserType = m_pLayout->GetUseWhenType();
    if(eUserType == LwpLayout::StartOnNextPage
        || eUserType == LwpLayout::StartOnOddPage
        || eUserType == LwpLayout::StartOnEvenPage )
    {
        return sal_True;
    }
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
