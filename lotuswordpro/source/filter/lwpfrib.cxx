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
#include <lwpfrib.hxx>
#include "lwphyperlinkmgr.hxx"
#include <xfilter/xfhyperlink.hxx>
#include <xfilter/xfstylemanager.hxx>
#include <xfilter/xftextspan.hxx>
#include <xfilter/xftextstyle.hxx>
#include <xfilter/xftextcontent.hxx>
#include "lwpfribheader.hxx"
#include "lwpfribtext.hxx"
#include "lwpfribtable.hxx"
#include "lwpfribbreaks.hxx"
#include "lwpfribframe.hxx"
#include "lwpfribsection.hxx"
#include "lwpcharacterstyle.hxx"
#include "lwpfootnote.hxx"
#include "lwpnotes.hxx"
#include "lwpfribmark.hxx"
#include <lwpglobalmgr.hxx>

#include <osl/diagnose.h>


LwpFrib::LwpFrib(LwpPara* pPara)
    : m_pFribMap(nullptr)
    , m_pPara(pPara)
    , m_pNext(nullptr)
    , m_nFribType(0)
    , m_ModFlag(false)
    , m_nRevisionType(0)
    , m_bRevisionFlag(false)
    , m_nEditor(0)
{
}

LwpFrib::~LwpFrib()
{
    Deregister();
}

LwpFrib* LwpFrib::CreateFrib(LwpPara* pPara, LwpObjectStream* pObjStrm, sal_uInt8 fribtag,sal_uInt8 editID)
{
    //Read Modifier
    std::unique_ptr<ModifierInfo> xModInfo;
    if(fribtag & FRIB_TAG_MODIFIER)
    {
        xModInfo.reset(new ModifierInfo);
        xModInfo->CodePage = 0;
        xModInfo->FontID = 0;
        xModInfo->RevisionType = 0;
        xModInfo->RevisionFlag = false;
        xModInfo->HasCharStyle = false;
        xModInfo->HasLangOverride = false;
        xModInfo->HasHighlight = false;
        ReadModifiers(pObjStrm, xModInfo.get());
    }

    //Read frib data
    std::unique_ptr<LwpFrib> newFrib;
    sal_uInt16 friblen = pObjStrm->QuickReaduInt16();
    sal_uInt8 fribtype = fribtag&~FRIB_TAG_TYPEMASK;
    switch(fribtype)
    {
        case FRIB_TAG_INVALID:  //fall through
        case FRIB_TAG_EOP:      //fall through
        default:
            newFrib.reset(new LwpFrib(pPara));
            break;
        case FRIB_TAG_TEXT:
        {
            newFrib.reset(new LwpFribText(pPara, (fribtag & FRIB_TAG_NOUNICODE) != 0));
            break;
        }
        case FRIB_TAG_TABLE:
            newFrib.reset(new LwpFribTable(pPara));
            break;
        case FRIB_TAG_TAB:
            newFrib.reset(new LwpFribTab(pPara));
            break;
        case FRIB_TAG_PAGEBREAK:
            newFrib.reset(new LwpFribPageBreak(pPara));
            break;
        case FRIB_TAG_FRAME:
            newFrib.reset(new LwpFribFrame(pPara));
            break;
        case FRIB_TAG_FOOTNOTE:
            newFrib.reset(new LwpFribFootnote(pPara));
            break;
        case FRIB_TAG_COLBREAK:
            newFrib.reset(new LwpFribColumnBreak(pPara));
            break;
        case FRIB_TAG_LINEBREAK:
            newFrib.reset(new LwpFribLineBreak(pPara));
            break;
        case FRIB_TAG_HARDSPACE:
            newFrib.reset(new LwpFribHardSpace(pPara));
            break;
        case FRIB_TAG_SOFTHYPHEN:
            newFrib.reset(new LwpFribSoftHyphen(pPara));
            break;
        case FRIB_TAG_PARANUMBER:
            newFrib.reset(new LwpFribParaNumber(pPara));
            break;
        case FRIB_TAG_UNICODE: //fall through
        case FRIB_TAG_UNICODE2: //fall through
        case FRIB_TAG_UNICODE3: //fall through
            newFrib.reset(new LwpFribUnicode(pPara));
            break;
        case FRIB_TAG_NOTE:
            newFrib.reset(new LwpFribNote(pPara));
            break;
        case FRIB_TAG_SECTION:
            newFrib.reset(new LwpFribSection(pPara));
            break;
        case FRIB_TAG_PAGENUMBER:
            newFrib.reset(new LwpFribPageNumber(pPara));
            break;
        case FRIB_TAG_DOCVAR:
            newFrib.reset(new LwpFribDocVar(pPara));
            break;
        case FRIB_TAG_BOOKMARK:
            newFrib.reset(new LwpFribBookMark(pPara));
            break;
        case FRIB_TAG_FIELD:
            newFrib.reset(new LwpFribField(pPara));
            break;
        case FRIB_TAG_CHBLOCK:
            newFrib.reset(new LwpFribCHBlock(pPara));
            break;
        case FRIB_TAG_RUBYMARKER:
            newFrib.reset(new LwpFribRubyMarker(pPara));
            break;
        case FRIB_TAG_RUBYFRAME:
            newFrib.reset(new LwpFribRubyFrame(pPara));
            break;
    }

    //Do not know why the fribTag judgment is necessary, to be checked with
    if (fribtag & FRIB_TAG_MODIFIER)
    {
        newFrib->SetModifiers(xModInfo.release());
    }

    newFrib->m_nFribType = fribtype;
    newFrib->m_nEditor = editID;
    newFrib->Read(pObjStrm, friblen);
    return newFrib.release();
}

void LwpFrib::Read(LwpObjectStream* pObjStrm, sal_uInt16 len)
{
    pObjStrm->SeekRel(len);
}

void LwpFrib::SetModifiers(ModifierInfo* pModifiers)
{
    if (pModifiers)
    {
        m_pModifiers.reset( pModifiers );
        m_ModFlag = true;
        if (pModifiers->RevisionFlag)
        {
            m_bRevisionFlag = true;
            m_nRevisionType = pModifiers->RevisionType;
        }
    }
}

void LwpFrib::RegisterStyle(LwpFoundry* pFoundry)
{
    if (!m_pModifiers)
        return;
    if (!m_pModifiers->FontID && !m_pModifiers->HasCharStyle && !m_pModifiers->HasHighlight)
    {
        m_ModFlag = false;
        return;
    }
    //we only read four modifiers, in these modifiers,CodePage and LangOverride are not styles,
    //so we can only handle fontid and charstyle, if others, we should not reg style
    //note by ,1-27
    rtl::Reference<XFFont> pFont;
    XFTextStyle* pStyle = nullptr;
    m_StyleName.clear();
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    XFTextStyle* pNamedStyle = nullptr;
    if (m_pModifiers->HasCharStyle && pFoundry)
    {
        pNamedStyle = dynamic_cast<XFTextStyle*>
                                (pFoundry->GetStyleManager()->GetStyle(m_pModifiers->CharStyleID));
    }
    if (pNamedStyle)
    {
        LwpCharacterStyle* pCharStyle = nullptr;
        if (m_pModifiers->FontID && pFoundry)
            pCharStyle = dynamic_cast<LwpCharacterStyle*>(m_pModifiers->CharStyleID.obj().get());
        if (pCharStyle)
        {
            std::unique_ptr<XFTextStyle> pNewStyle(new XFTextStyle());
            *pNewStyle = *pNamedStyle;

            pNewStyle->SetStyleName("");
            pFont = pFoundry->GetFontManager().CreateOverrideFont(pCharStyle->GetFinalFontID(),m_pModifiers->FontID);
            pNewStyle->SetFont(pFont);
            IXFStyleRet aNewStyle = pXFStyleManager->AddStyle(std::move(pNewStyle));
            m_StyleName = aNewStyle.m_pStyle->GetStyleName();
            pStyle = dynamic_cast<XFTextStyle*>(aNewStyle.m_pStyle);
            if (aNewStyle.m_bOrigDeleted)
                pStyle = nullptr;
        }
        else
            m_StyleName =  pNamedStyle->GetStyleName();
    }
    else
    {
        if (m_pModifiers->FontID && pFoundry)
        {
            std::unique_ptr<XFTextStyle> pNewStyle(new XFTextStyle());
            pFont = pFoundry->GetFontManager().CreateFont(m_pModifiers->FontID);
            pNewStyle->SetFont(pFont);
            IXFStyleRet aNewStyle = pXFStyleManager->AddStyle(std::move(pNewStyle));
            m_StyleName = aNewStyle.m_pStyle->GetStyleName();
            pStyle = dynamic_cast<XFTextStyle*>(aNewStyle.m_pStyle);
            if (aNewStyle.m_bOrigDeleted)
                pStyle = nullptr;
        }
    }

    if (!m_pModifiers->HasHighlight)
        return;

    XFColor  aColor = GetHighlightColor();//right yellow
    if (pStyle)//change the style directly
        pStyle->GetFont()->SetBackColor(aColor);
    else //register a new style
    {
        std::unique_ptr<XFTextStyle> pNewStyle(new XFTextStyle());

        if (!m_StyleName.isEmpty())
        {
            XFTextStyle* pOldStyle = pXFStyleManager->FindTextStyle(m_StyleName);
            *pNewStyle = *pOldStyle;
            pNewStyle->GetFont()->SetBackColor(aColor);
        }
        else
        {
            pFont = new XFFont;
            pFont->SetBackColor(aColor);
            pNewStyle->SetFont(pFont);
        }
        m_StyleName = pXFStyleManager->AddStyle(std::move(pNewStyle)).m_pStyle->GetStyleName();
    }
}

void LwpFrib::ReadModifiers(LwpObjectStream* pObjStrm,ModifierInfo* pModInfo)
{
    for(;;)
    {
        bool bFailure;

        // Get the modifier type
        sal_uInt8 Modifier = pObjStrm->QuickReaduInt8(&bFailure);
        if (bFailure)
            break;

        // Stop when we hit the last modifier
        if (Modifier == FRIB_MTAG_NONE)
            break;

        // Get the modifier length
        sal_uInt8 len = pObjStrm->QuickReaduInt8(&bFailure);
        if (bFailure)
            break;

        switch (Modifier)
        {
            case FRIB_MTAG_FONT:
                if (len != sizeof(pModInfo->FontID))
                {
                    OSL_FAIL("FRIB_MTAG_FONT entry wrong size");
                    pObjStrm->SeekRel(len);
                }
                else
                    pModInfo->FontID = pObjStrm->QuickReaduInt32();
                break;
            case FRIB_MTAG_CHARSTYLE:
                pModInfo->HasCharStyle = true;
                pModInfo->CharStyleID.ReadIndexed(pObjStrm);
                break;
            case FRIB_MTAG_LANGUAGE:
                pModInfo->HasLangOverride = true;
                pModInfo->Language.Read(pObjStrm);
                break;
            case FRIB_MTAG_CODEPAGE:
                if (len != sizeof(pModInfo->CodePage))
                {
                    OSL_FAIL("FRIB_MTAG_CODEPAGE entry wrong size");
                    pObjStrm->SeekRel(len);
                }
                else
                    pModInfo->CodePage = pObjStrm->QuickReaduInt16();
                break;
            case FRIB_MTAG_ATTRIBUTE:
                pModInfo->aTxtAttrOverride.Read(pObjStrm);
                if (pModInfo->aTxtAttrOverride.IsHighlight())
                    pModInfo->HasHighlight = true;
                break;
            case FRIB_MTAG_REVISION:
                pModInfo->RevisionType = pObjStrm->QuickReaduInt8();
                pModInfo->RevisionFlag = true;
                break;
            default:
                pObjStrm->SeekRel(len);
                break;
        }
        // TODO: read the modifier data
    }
}

/**
*  @descr:   Whether there are other fribs following current frib.
*  @return:  True if having following fribs, or false.
*/
bool LwpFrib::HasNextFrib()
{
    return GetNext() && GetNext()->GetType() != FRIB_TAG_EOP;
}

void LwpFrib::ConvertChars(XFContentContainer* pXFPara,const OUString& text)
{
    if (m_ModFlag)
    {
        OUString strStyleName = GetStyleName();
        XFTextSpan *pSpan = new XFTextSpan(text,strStyleName);
        pXFPara->Add(pSpan);
    }
    else
    {
        XFTextContent *pSpan = new XFTextContent();
        pSpan->SetText(text);
        pXFPara->Add(pSpan);
    }
}

void LwpFrib::ConvertHyperLink(XFContentContainer* pXFPara, const LwpHyperlinkMgr* pHyperlink,const OUString& text)
{
    XFHyperlink* pHyper = new XFHyperlink;
    pHyper->SetHRef(pHyperlink->GetHyperlink());
    pHyper->SetText(text);
    pHyper->SetStyleName(GetStyleName());
    pXFPara->Add(pHyper);
}

/**
*  @descr:   Get the current frib font style
*  @return:  XFFont pointer
*/
rtl::Reference<XFFont> LwpFrib::GetFont()
{
    rtl::Reference<XFFont> pFont;
    if(m_pModifiers&&m_pModifiers->FontID)
    {
        LwpFoundry* pFoundry = m_pPara->GetFoundry();
        if (pFoundry)
            pFont = pFoundry->GetFontManager().CreateFont(m_pModifiers->FontID);
    }
    else
    {
        XFParaStyle* pXFParaStyle = m_pPara->GetXFParaStyle();
        pFont = pXFParaStyle->GetFont();
    }
    return pFont;
}

OUString LwpFrib::GetEditor()
{
    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
    return pGlobal->GetEditorName(m_nEditor);
}

XFColor LwpFrib::GetHighlightColor()
{
    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
    return pGlobal->GetHighlightColor(m_nEditor);
}

void LwpFrib::Register(std::map<LwpFrib*,OUString>* pFribMap)
{
    if (m_pFribMap)
        throw std::runtime_error("registered already");
    m_pFribMap = pFribMap;
}

void LwpFrib::Deregister()
{
    if (m_pFribMap)
    {
        m_pFribMap->erase(this);
        m_pFribMap = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
