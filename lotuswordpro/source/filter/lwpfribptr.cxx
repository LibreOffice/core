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

#include "lwpfribptr.hxx"

#include "lwpfribheader.hxx"
#include "lwpfribtext.hxx"
#include "lwppara.hxx"
#include "lwpstory.hxx"
#include <xfilter/xftextspan.hxx>
#include <xfilter/xftextcontent.hxx>
#include <xfilter/xftabstop.hxx>
#include <xfilter/xflinebreak.hxx>
#include "lwpfribsection.hxx"
#include "lwpsection.hxx"
#include "lwpfribbreaks.hxx"
#include "lwpfribframe.hxx"
#include "lwpfribtable.hxx"
#include <xfilter/xfstylemanager.hxx>
#include "lwphyperlinkmgr.hxx"
#include <xfilter/xfhyperlink.hxx>
#include "lwpfootnote.hxx"
#include "lwpnotes.hxx"
#include "lwpfribmark.hxx"
#include <xfilter/xfchange.hxx>
#include <lwpchangemgr.hxx>
#include <lwpglobalmgr.hxx>
#include <lwpdropcapmgr.hxx>

LwpFribPtr::LwpFribPtr()
    : m_pFribs(nullptr),m_pXFPara(nullptr),m_pPara(nullptr)
{
}

LwpFribPtr::~LwpFribPtr()
{
    for (LwpFrib* pCurFrib = m_pFribs; pCurFrib;)
    {
        LwpFrib* pNextFrib = pCurFrib -> GetNext();
        delete pCurFrib;
        pCurFrib = pNextFrib;
    }
}

void LwpFribPtr::ReadPara(LwpObjectStream* pObjStrm)
{
    LwpFrib* pCurFrib = m_pFribs = nullptr;
    for(;;)
    {
        // Get the frib type
        sal_uInt8 FribTag = pObjStrm->QuickReaduInt8();

        sal_uInt8 FribType = FribTag & ~FRIB_TAG_TYPEMASK;

        // Stop when we hit Elvis
        if (FribType == FRIB_TAG_ELVIS)
            break;

        // skip the editor ID
        //pObjStrm->SeekRel(sizeof(sal_uInt8));
        sal_uInt8 FribEditor = pObjStrm->QuickReaduInt8();
        if( FribType != FT_MAXIMUM )
        {
            LwpFrib* pFrib = LwpFrib::CreateFrib( m_pPara, pObjStrm, FribTag,FribEditor);
            if(!m_pFribs)
            {
                m_pFribs = pFrib;
            }
            if(pCurFrib)
            {
                pCurFrib->SetNext(pFrib);
            }
            else//first frib in the para
            {
                if (pFrib->GetType() == FRIB_TAG_TEXT)
                {
                    LwpFribText* pText = static_cast<LwpFribText*>(pFrib);
                    if (pFrib->GetModifiers())
                        m_pPara->SetFirstFrib(pText->GetText(),pText->GetModifiers()->FontID);
                    else
                        m_pPara->SetFirstFrib(pText->GetText(),0);
                }
            }
            pCurFrib = pFrib;
        }
    }
}

void LwpFribPtr::XFConvert()
{
    LwpFrib* pFrib = m_pFribs;
    while(pFrib)
    {
        sal_uInt8 nFribType = pFrib->GetType();
        bool bRevisionFlag = pFrib->GetRevisionFlag();
        OUString sChangeID;
        if (bRevisionFlag)
        {
            if ( nFribType!= FRIB_TAG_TABLE && nFribType != FRIB_TAG_FIELD
                    && nFribType != FRIB_TAG_FRAME)
            {
                //sal_uInt8 nRevisionType = pFrib->GetRevisionType();
                LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
                LwpChangeMgr* pChangeMgr = pGlobal->GetLwpChangeMgr();
                sChangeID = pChangeMgr->GetChangeID(pFrib);
                if (!sChangeID.isEmpty())
                {
                    rtl::Reference<XFChangeStart> xChangeStart(new XFChangeStart);
                    xChangeStart->SetChangeID(sChangeID);
                    m_pXFPara->Add(xChangeStart.get());
                }
            }
        }

        switch(nFribType)
        {
        case FRIB_TAG_TEXT:
        {
            LwpFribText* textFrib= static_cast<LwpFribText*>(pFrib);
            textFrib->XFConvert(m_pXFPara,m_pPara->GetStory());
        }
            break;
        case FRIB_TAG_TAB:
        {
            LwpFribTab* tabFrib = static_cast<LwpFribTab*>(pFrib);
            if (pFrib->m_ModFlag)
            {
                rtl::Reference<XFTextSpan> xSpan(new XFTextSpan);
                xSpan->SetStyleName(tabFrib->GetStyleName());
                rtl::Reference<XFTabStop> xTab(new XFTabStop);
                xSpan->Add(xTab.get());
                m_pXFPara->Add(xSpan.get());
            }
            else
            {
                rtl::Reference<XFTabStop> xTab(new XFTabStop);
                m_pXFPara->Add(xTab.get());
            }
            break;
        }
        case FRIB_TAG_SECTION:
        {
            LwpFribSection* pSectionFrib = static_cast<LwpFribSection*>(pFrib);
            pSectionFrib->ParseSection();
            break;
        }

        case FRIB_TAG_PAGEBREAK:
        {
            LwpFribPageBreak* pPageBreak = static_cast<LwpFribPageBreak*>(pFrib);
            LwpPageLayout* pLayout = dynamic_cast<LwpPageLayout*>(pPageBreak->GetLayout().obj().get());
            if(pLayout)
            {
                pPageBreak->ParseLayout();
            }
            else
            {
                if (pPageBreak->IsLastFrib())
                {
                    m_pXFPara->SetStyleName( pPageBreak->GetStyleName() );
                }
                else
                {
                    //parse pagebreak
                    rtl::Reference<XFParagraph> xPara(new XFParagraph);
                    xPara->SetStyleName(pFrib->GetStyleName());
                    SetXFPara(xPara.get());
                    m_pPara->AddXFContent(xPara.get());
                }
            }
            break;
        }
        case FRIB_TAG_COLBREAK:
        {
            rtl::Reference<XFParagraph> xPara(new XFParagraph);
            xPara->SetStyleName(pFrib->GetStyleName());
            SetXFPara(xPara.get());
            m_pPara->AddXFContent(xPara.get());
            break;
        }
        case FRIB_TAG_LINEBREAK:
        {
            rtl::Reference<XFLineBreak> xLineBreak(new XFLineBreak);
            m_pXFPara->Add(xLineBreak.get());
            break;
        }
        case FRIB_TAG_UNICODE: //fall through
        case FRIB_TAG_UNICODE2: //fall through
        case FRIB_TAG_UNICODE3: //fall through
        {
            LwpFribUnicode* unicodeFrib= static_cast<LwpFribUnicode*>(pFrib);
            unicodeFrib->XFConvert(m_pXFPara,m_pPara->GetStory());
        }
            break;
        case FRIB_TAG_HARDSPACE:
        {
            OUString sHardSpace(u'\x00a0');
            LwpStory *pStory = m_pPara->GetStory();
            LwpHyperlinkMgr* pHyperlink = pStory ? pStory->GetHyperlinkMgr() : nullptr;
            if (pHyperlink && pHyperlink->GetHyperlinkFlag())
                pFrib->ConvertHyperLink(m_pXFPara,pHyperlink,sHardSpace);
            else
                pFrib->ConvertChars(m_pXFPara,sHardSpace);
        }
            break;
        case FRIB_TAG_SOFTHYPHEN:
        {
            OUString sSoftHyphen(u'\x00ad');
            pFrib->ConvertChars(m_pXFPara,sSoftHyphen);
        }
            break;
        case FRIB_TAG_FRAME:
        {
            LwpFribFrame* frameFrib= static_cast<LwpFribFrame*>(pFrib);
            rtl::Reference<LwpObject> pLayout = frameFrib->GetLayout();
            if (pLayout.is() && pLayout->GetTag() == VO_DROPCAPLAYOUT)
            {
                LwpFoundry* pFoundry = m_pPara->GetFoundry();
                LwpDropcapMgr* pMgr = pFoundry ? pFoundry->GetDropcapMgr() : nullptr;
                if (pMgr)
                    pMgr->SetXFPara(m_pXFPara);
            }
            frameFrib->XFConvert(m_pXFPara);
            break;
        }
        case FRIB_TAG_CHBLOCK:
        {
            LwpFribCHBlock* chbFrib = static_cast<LwpFribCHBlock*>(pFrib);
            chbFrib->XFConvert(m_pXFPara,m_pPara->GetStory());
        }
            break;
        case FRIB_TAG_TABLE:
        {
            LwpFribTable* tableFrib = static_cast<LwpFribTable*>(pFrib);
            //tableFrib->XFConvert(m_pPara->GetXFContainer());
            tableFrib->XFConvert(m_pXFPara);
        }
            break;
        case FRIB_TAG_BOOKMARK:
        {
            LwpFribBookMark* bookmarkFrib = static_cast<LwpFribBookMark*>(pFrib);
            bookmarkFrib->XFConvert(m_pXFPara);
        }
        break;
        case FRIB_TAG_FOOTNOTE:
        {
            LwpFribFootnote* pFootnoteFrib = static_cast<LwpFribFootnote*>(pFrib);
            pFootnoteFrib->XFConvert(m_pXFPara);
            break;
        }
        case FRIB_TAG_FIELD:
        {
            LwpFribField* fieldFrib = static_cast<LwpFribField*>(pFrib);
            fieldFrib->XFConvert(m_pXFPara);
            break;
        }
        case FRIB_TAG_NOTE:
        {
            LwpFribNote* pNoteFrib = static_cast<LwpFribNote*>(pFrib);
            pNoteFrib->XFConvert(m_pXFPara);
            break;
        }
        case FRIB_TAG_PAGENUMBER:
        {
            LwpFribPageNumber* pagenumFrib = static_cast<LwpFribPageNumber*>(pFrib);
            pagenumFrib->XFConvert(m_pXFPara);
            break;
        }
        case FRIB_TAG_DOCVAR:
        {
            LwpFribDocVar* docFrib = static_cast<LwpFribDocVar*>(pFrib);
            docFrib->XFConvert(m_pXFPara);
            break;
        }
        case FRIB_TAG_RUBYMARKER:
        {
            LwpFribRubyMarker* rubyFrib = static_cast<LwpFribRubyMarker*>(pFrib);
            rubyFrib->XFConvert(m_pXFPara);
            break;
        }
        case FRIB_TAG_RUBYFRAME:
        {
            LwpFribRubyFrame* rubyfrmeFrib = static_cast<LwpFribRubyFrame*>(pFrib);
            rubyfrmeFrib->XFConvert();
            break;
        }
        default :
            break;
        }

        if (bRevisionFlag )//&& pFrib->GetRevisionType() == LwpFrib::REV_INSERT)
        {
            if (nFribType!= FRIB_TAG_TABLE && nFribType != FRIB_TAG_FIELD
                    && nFribType != FRIB_TAG_FRAME)
            {
                if (!sChangeID.isEmpty())
                {
                    rtl::Reference<XFChangeEnd> xChangeEnd(new XFChangeEnd);
                    xChangeEnd->SetChangeID(sChangeID);
                    m_pXFPara->Add(xChangeEnd.get());
                }
            }
        }
        pFrib = pFrib->GetNext();
    }

}

/**************************************************************************
 * @descr:  Whether has a frib which type is nType
**************************************************************************/
LwpFrib* LwpFribPtr::HasFrib(sal_uInt8 nType)
{
    LwpFrib* pFrib = m_pFribs;
    while(pFrib)
    {
        if(pFrib->GetType()==nType)
        {
            return pFrib;
        }
        pFrib = pFrib->GetNext();
    }
    return nullptr;
}

void LwpFribPtr::GatherAllText()
{
    LwpFrib* pFrib = m_pFribs;
    while(pFrib)
    {
        switch(pFrib->GetType())
        {
        case FRIB_TAG_TEXT:
        {
            OUString sText = static_cast<LwpFribText*>(pFrib)->GetText();
            m_pPara->SetAllText(sText);
            break;
        }
        case FRIB_TAG_UNICODE: //fall through
        case FRIB_TAG_UNICODE2: //fall through
        case FRIB_TAG_UNICODE3: //fall through  :
        {
            OUString sText = static_cast<LwpFribText*>(pFrib)->GetText();
            m_pPara->SetAllText(sText);
            break;
        }
        default :
            break;
    }
        pFrib = pFrib->GetNext();
    }
}

void LwpFribPtr::RegisterStyle()
{
    LwpFrib* pFrib = m_pFribs;
    while(pFrib)
    {
        switch(pFrib->GetType())
        {
        case FRIB_TAG_TEXT:
        {
            pFrib->RegisterStyle(m_pPara->GetFoundry());
            OUString sText = static_cast<LwpFribText*>(pFrib)->GetText();
            ProcessDropcap(m_pPara->GetStory(), pFrib,sText.getLength());
            break;
        }
        case FRIB_TAG_TAB:
            pFrib->RegisterStyle(m_pPara->GetFoundry());
            break;
        case FRIB_TAG_SECTION:
            {
                //section registerstyle here;
                LwpFribSection* pSectionFrib = static_cast<LwpFribSection*>(pFrib);
                pSectionFrib->RegisterSectionStyle();
                //for bullet
                pSectionFrib->SetSectionName();
            }
            break;
        case FRIB_TAG_PAGEBREAK:
        {
            LwpFribPageBreak* breakFrib = static_cast<LwpFribPageBreak*>(pFrib);
            breakFrib->RegisterBreakStyle(m_pPara);
        }
            break;
        case FRIB_TAG_COLBREAK:
        {
            LwpFribColumnBreak* breakFrib = static_cast<LwpFribColumnBreak*>(pFrib);
            breakFrib->RegisterBreakStyle(m_pPara);
        }
            break;
        case FRIB_TAG_UNICODE: //fall through
        case FRIB_TAG_UNICODE2: //fall through
        case FRIB_TAG_UNICODE3: //fall through  :
        {
            pFrib->RegisterStyle(m_pPara->GetFoundry());
            OUString sText = static_cast<LwpFribUnicode*>(pFrib)->GetText();
            ProcessDropcap(m_pPara->GetStory(), pFrib,sText.getLength());
            break;
        }
        case FRIB_TAG_HARDSPACE:
            pFrib->RegisterStyle(m_pPara->GetFoundry());
            ProcessDropcap(m_pPara->GetStory(),pFrib,1);
            break;
        case FRIB_TAG_SOFTHYPHEN:
            pFrib->RegisterStyle(m_pPara->GetFoundry());
            break;
        case FRIB_TAG_FRAME:
        {
            LwpFribFrame* frameFrib = static_cast<LwpFribFrame*>(pFrib);
            frameFrib->SetParaDropcap(m_pPara);
            frameFrib->RegisterStyle(m_pPara->GetFoundry());
            //register framelayout style, ,03/09/2005
            //frameFrib->GetLayout()->RegisterStyle();
                }
                    break;
                case FRIB_TAG_CHBLOCK:
                    break;
                case FRIB_TAG_TABLE:
                {
                    LwpFribTable* tableFrib = static_cast<LwpFribTable*>(pFrib);
                    tableFrib->RegisterNewStyle();
                }
                    break;
        case FRIB_TAG_FOOTNOTE:
        {
            LwpFribFootnote* pFribFootnote = static_cast<LwpFribFootnote*>(pFrib);
            pFribFootnote->RegisterNewStyle();
        }
            break;
        case FRIB_TAG_NOTE:
        {
            LwpFribNote* pNoteFrib = static_cast<LwpFribNote*>(pFrib);
            pNoteFrib->RegisterNewStyle();
            break;
        }
        case FRIB_TAG_PAGENUMBER:
        {
            pFrib->RegisterStyle(m_pPara->GetFoundry());
            break;
        }
        case FRIB_TAG_DOCVAR:
        {
            LwpFribDocVar* docFrib = static_cast<LwpFribDocVar*>(pFrib);
            docFrib->RegisterStyle(m_pPara->GetFoundry());
            break;
        }
        case FRIB_TAG_FIELD:
        {
            LwpFribField* fieldFrib = static_cast<LwpFribField*>(pFrib);
            fieldFrib->RegisterStyle(m_pPara->GetFoundry());
            break;
        }
        case FRIB_TAG_BOOKMARK:
        {
            LwpFribBookMark* bookmarkFrib = static_cast<LwpFribBookMark*>(pFrib);
            bookmarkFrib->RegisterStyle(m_pPara->GetFoundry());
            break;
        }
        case FRIB_TAG_RUBYFRAME:
        {
            LwpFribRubyFrame* rubyfrmeFrib = static_cast<LwpFribRubyFrame*>(pFrib);
            rubyfrmeFrib->RegisterStyle(m_pPara->GetFoundry());
            break;
        }
        default :
            break;
        }
        if (pFrib->GetRevisionFlag() && pFrib->GetType()!= FRIB_TAG_FIELD)
        {
            LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
            LwpChangeMgr* pChangeMgr = pGlobal->GetLwpChangeMgr();
            pChangeMgr->AddChangeFrib(pFrib);
        }

        pFrib = pFrib->GetNext();
    }
}

void LwpFribPtr::ProcessDropcap(LwpStory* pStory,LwpFrib* pFrib,sal_uInt32 nLen)
{
    if (pStory && pStory->GetDropcapFlag())
    {
        XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
        XFTextStyle* pFribStyle = pXFStyleManager->FindTextStyle(pFrib->GetStyleName());
        pFribStyle->GetFont()->SetFontSize(0);

        LwpDropcapLayout* pObj = dynamic_cast<LwpDropcapLayout*>(pStory->GetLayoutsWithMe().GetOnlyLayout().obj().get());
        if (pObj)
            pObj->SetChars(nLen);
    }
}
/**
 * @descr:  If the position of pPreLayout is earlier than pNextLayout, return true, or return false, default return true
 *
*/
bool LwpFribPtr::ComparePagePosition(LwpVirtualLayout const * pPreLayout, LwpVirtualLayout const * pNextLayout)
{
    if(!pPreLayout || !pNextLayout)
        return true;

    LwpFrib* pFrib = m_pFribs;
    LwpVirtualLayout* pLayout = nullptr;
    while(pFrib)
    {
        switch(pFrib->GetType())
            {
            case FRIB_TAG_SECTION:
            {
                LwpFribSection* pSectionFrib = static_cast<LwpFribSection*>(pFrib);
                LwpSection* pSection = pSectionFrib->GetSection();
                if(pSection)
                {
                    pLayout = pSection->GetPageLayout();
                }

                break;
            }
            case FRIB_TAG_PAGEBREAK:
            {
                LwpFribPageBreak* pPageBreak = static_cast<LwpFribPageBreak*>(pFrib);
                pLayout = dynamic_cast<LwpVirtualLayout*>(pPageBreak->GetLayout().obj().get());
                break;
            }
            default:
                break;
        }

        if(pLayout)
        {
            if(pPreLayout == pLayout)
                return true;
            if(pNextLayout == pLayout)
                return false;
        }
        pFrib = pFrib->GetNext();
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
