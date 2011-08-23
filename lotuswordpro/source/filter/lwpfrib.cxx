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
 Jan 2005			Created
 ************************************************************************/
#include "lwpfrib.hxx"
#include "lwpcharsetmgr.hxx"
#include "lwpsection.hxx"
#include "lwphyperlinkmgr.hxx"
#include "xfilter/xfhyperlink.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xfsection.hxx"
#include "xfilter/xfsectionstyle.hxx"
#include "xfilter/xftextspan.hxx"
#include "xfilter/xftextcontent.hxx"
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
#include "lwpchangemgr.hxx"
#include "lwpdocdata.hxx"
#include "lwpglobalmgr.hxx"

LwpFrib::LwpFrib(LwpPara* pPara)
    : m_pPara(pPara), m_pNext(NULL)
{
    m_ModFlag = sal_False;
    m_pModifiers = NULL;
    m_bRevisionFlag = sal_False;
}

LwpFrib::~LwpFrib()
{
    if(m_pModifiers)
        delete m_pModifiers;
}

LwpFrib* LwpFrib::CreateFrib(LwpPara* pPara, LwpObjectStream* pObjStrm, sal_uInt8 fribtag,sal_uInt8 editID)
{
    //Read Modifier
    ModifierInfo* pModInfo = NULL;
    if(fribtag & FRIB_TAG_MODIFIER)
    {
        pModInfo  = new ModifierInfo();
        pModInfo->CodePage = 0;
        pModInfo->FontID = 0;
        pModInfo->RevisionFlag = sal_False;
        pModInfo->HasCharStyle = sal_False;
        pModInfo->HasLangOverride = sal_False;
        pModInfo->HasHighLight = sal_False;
        ReadModifiers( pObjStrm, pModInfo );
    }

    //Read frib data
    LwpFrib* newFrib = NULL;
    sal_uInt16 friblen = pObjStrm->QuickReaduInt16();
    sal_uInt8 fribtype = fribtag&~FRIB_TAG_TYPEMASK;
    switch(fribtype)
    {
        case FRIB_TAG_INVALID:	//fall through
        case FRIB_TAG_EOP:		//fall through
        default:
            newFrib = new LwpFrib(pPara);
            break;
        case FRIB_TAG_TEXT:
        {
            newFrib = new LwpFribText (pPara, fribtag & FRIB_TAG_NOUNICODE);
            break;
        }
        case FRIB_TAG_TABLE:
            newFrib = new LwpFribTable(pPara);
            break;
        case FRIB_TAG_TAB:
            newFrib = new LwpFribTab(pPara);
            break;
        case FRIB_TAG_PAGEBREAK:
            newFrib = new LwpFribPageBreak(pPara);
            break;
        case FRIB_TAG_FRAME:
            newFrib = new LwpFribFrame(pPara);
            break;
        case FRIB_TAG_FOOTNOTE:
            newFrib = new LwpFribFootnote(pPara);
            break;
        case FRIB_TAG_COLBREAK:
            newFrib = new LwpFribColumnBreak(pPara);
            break;
        case FRIB_TAG_LINEBREAK:
            newFrib = new LwpFribLineBreak(pPara);
            break;
        case FRIB_TAG_HARDSPACE:
            newFrib = new LwpFribHardSpace(pPara);
            break;
        case FRIB_TAG_SOFTHYPHEN:
            newFrib = new LwpFribSoftHyphen(pPara);
            break;
        case FRIB_TAG_PARANUMBER:
            newFrib = new LwpFribParaNumber(pPara);
            break;
        case FRIB_TAG_UNICODE: //fall through
        case FRIB_TAG_UNICODE2: //fall through
        case FRIB_TAG_UNICODE3: //fall through
            newFrib = new LwpFribUnicode(pPara);
            break;
        case FRIB_TAG_NOTE:
            newFrib = new  LwpFribNote(pPara);
            break;
/*		case FRIB_TAG_KANJI:
            #ifdef KANJI
            newFrib = new CFribHelperKanji;
            #else
            newFrib = new CFribHelperUnicode;
            #endif
            break;
        case FRIB_TAG_HKATAKANA:
            #ifdef KANJI
            newFrib = new CFribHelperHKatakana;
            #else
            newFrib = new CFribHelperUnicode;
            #endif
            break;
        case FRIB_TAG_SEPARATOR:
            newFrib = new CFribHelperSeparator;
            break;*/
        case FRIB_TAG_SECTION:
            newFrib = new LwpFribSection(pPara);
            break;
/*		case FRIB_TAG_TOMBSTONE:
            newFrib = new CFribHelperTombstone;
            break;
        case FRIB_TAG_SPECIALTAB:
            newFrib = new CFribHelperSpecialTab;
            break;
        case FRIB_TAG_PAGENUMBER:
            newFrib = new CFribHelperPageNumber;
            break;
*/		case FRIB_TAG_PAGENUMBER:
            newFrib = new LwpFribPageNumber(pPara);
            break;
        case FRIB_TAG_DOCVAR:
            newFrib = new LwpFribDocVar(pPara);
            break;
        case FRIB_TAG_BOOKMARK:
            newFrib = new LwpFribBookMark(pPara);
            break;
/*		case FRIB_TAG_DOCVAR:
            newFrib = new CFribHelperBookmark;
            break;
        case FRIB_TAG_DDE:
            newFrib = new CFribHelperDDE;
            break;
*/		case FRIB_TAG_FIELD:
            newFrib = new LwpFribField(pPara);
            break;
        case FRIB_TAG_CHBLOCK:
            newFrib = new LwpFribCHBlock(pPara);
            break;
/*		case FRIB_TAG_FLOWBREAK:
            newFrib = new CFribHelperFlowBreak;
            break;
#ifdef RUBY*/
        case FRIB_TAG_RUBYMARKER:
            newFrib = new LwpFribRubyMarker(pPara);
            break;
        case FRIB_TAG_RUBYFRAME:
            newFrib = new LwpFribRubyFrame(pPara);
            break;
/*#endif
*/
    }

    //Do not know why the fribTag judgement is necessary, to be checked with
    if ( fribtag & FRIB_TAG_MODIFIER )
    {
        newFrib->SetModifiers(pModInfo);
    }

    newFrib->SetType(fribtype);
    newFrib->SetEditor(editID);
    newFrib->Read(pObjStrm, friblen);
    return newFrib;
}

void LwpFrib::Read(LwpObjectStream* pObjStrm, sal_uInt16 len)
{
    pObjStrm->SeekRel(len);
}

void LwpFrib::SetModifiers(ModifierInfo* pModifiers)
{
    if (pModifiers)
    {
        m_pModifiers = pModifiers;
        m_ModFlag = sal_True;
        if (pModifiers->RevisionFlag)
        {
            m_bRevisionFlag = sal_True;
            m_nRevisionType = pModifiers->RevisionType;
        }
    }
}

void LwpFrib::RegisterStyle(LwpFoundry* pFoundry)
{
    if (!m_pModifiers)
        return;
    if (!m_pModifiers->FontID && !m_pModifiers->HasCharStyle && !m_pModifiers->HasHighLight)
    {
        m_ModFlag = sal_False;
        return;
    }
    //we only read four modifiers, in these modifiers,CodePage and LangOverride are not styles,
    //so we can only handle fontid and characstyle, if others ,we should not reg style
    //note by ,1-27
    XFFont* pFont;
    XFTextStyle* pStyle = NULL;
    m_StyleName = A2OUSTR("");
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    if (m_pModifiers->HasCharStyle)
    {
        XFTextStyle* pNamedStyle = static_cast<XFTextStyle*>
                                (pFoundry->GetStyleManager()->GetStyle(m_pModifiers->CharStyleID));
        if (m_pModifiers->FontID)
        {
            pStyle = new XFTextStyle();
            *pStyle = *pNamedStyle;
            LwpCharacterStyle* pCharStyle = static_cast<LwpCharacterStyle*>(m_pModifiers->CharStyleID.obj());

            pStyle->SetStyleName(A2OUSTR(""));
            pFont = pFoundry->GetFontManger()->CreateOverrideFont(pCharStyle->GetFinalFontID(),m_pModifiers->FontID);
            pStyle->SetFont(pFont);
            m_StyleName = pXFStyleManager->AddStyle(pStyle)->GetStyleName();
            }
        else
            m_StyleName =  pNamedStyle->GetStyleName();
    }
    else
    {
        if (m_pModifiers->FontID)
        {
            pStyle = new XFTextStyle();
            pFont = pFoundry->GetFontManger()->CreateFont(m_pModifiers->FontID);
            pStyle->SetFont(pFont);
            m_StyleName = pXFStyleManager->AddStyle(pStyle)->GetStyleName();
        }
    }

    if (m_pModifiers->HasHighLight)
    {
        XFColor  aColor = GetHighLightColor();//right yellow
        if (pStyle)//change the style directly
            pStyle->GetFont()->SetBackColor(aColor);
        else //register a new style
        {
            pStyle = new XFTextStyle();

            if (m_StyleName.getLength()>0)
            {
                XFTextStyle* pOldStyle = pXFStyleManager->FindTextStyle(m_StyleName);
                *pStyle = *pOldStyle;
                pStyle->GetFont()->SetBackColor(aColor);
            }
            else
            {
                pFont = new XFFont;
                pFont->SetBackColor(aColor);
                pStyle->SetFont(pFont);
            }
            m_StyleName = pXFStyleManager->AddStyle(pStyle)->GetStyleName();
        }
    }
}

void LwpFrib::ReadModifiers(LwpObjectStream* pObjStrm,ModifierInfo* pModInfo)
{
    sal_uInt8 Modifier;
    sal_uInt8 len;
//	sal_Bool ModifyFlag = sal_False;

    for(;;)
    {
        // Get the modifier type
        pObjStrm->QuickRead(&Modifier, sizeof(Modifier));

        // Stop when we hit the last modifier
        if (Modifier == FRIB_MTAG_NONE)
            break;
//        ModifyFlag = sal_True;
        // Get the modifier length
        pObjStrm->QuickRead(&len, sizeof(len));

        switch (Modifier)
        {
            case FRIB_MTAG_FONT:
                pObjStrm->QuickRead(&pModInfo->FontID,len);
                break;
            case FRIB_MTAG_CHARSTYLE:
                pModInfo->HasCharStyle = sal_True;
                pModInfo->CharStyleID.ReadIndexed(pObjStrm);
                break;
            case FRIB_MTAG_LANGUAGE:
                pModInfo->HasLangOverride = sal_True;
                pModInfo->Language.Read(pObjStrm);
                break;
            case FRIB_MTAG_CODEPAGE:
                pObjStrm->QuickRead(&pModInfo->CodePage,len);
                break;
            //add by , 02/22/2005
            case FRIB_MTAG_ATTRIBUTE:
                pModInfo->aTxtAttrOverride.Read(pObjStrm);
                if (pModInfo->aTxtAttrOverride.IsHighLight())
                    pModInfo->HasHighLight = sal_True;
                break;
            //end add
            case FRIB_MTAG_REVISION:
                pModInfo->RevisionType = pObjStrm->QuickReaduInt8();
                pModInfo->RevisionFlag = sal_True;
                break;
            default:
                pObjStrm->SeekRel(len);
                break;
        }
        // TODO: read the modifier data
    //	pObjStrm->SeekRel(len);

    }

}

//do nothing
//void LwpFrib::Parse(IXFStream* pOutputStream)
//{}

/**
*  @descr:   Whether there are other fribs following current frib.
*  @return:  Ture if having following fribs, or false.
*/
sal_Bool LwpFrib::HasNextFrib()
{
    if (!GetNext() || GetNext()->GetType()==FRIB_TAG_EOP)
        return sal_False;
    return sal_True;
}

void LwpFrib::ConvertChars(XFContentContainer* pXFPara,OUString text)
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

void LwpFrib::ConvertHyperLink(XFContentContainer* pXFPara,LwpHyperlinkMgr* pHyperlink,OUString text)
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
XFFont* LwpFrib::GetFont()
{
    XFFont* pFont = NULL;
    if(m_pModifiers&&m_pModifiers->FontID)
    {
        LwpFoundry* pFoundry = m_pPara->GetFoundry();
        pFont = pFoundry->GetFontManger()->CreateFont(m_pModifiers->FontID);
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

XFColor LwpFrib::GetHighLightColor()
{
    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
    return pGlobal->GetHighLightColor(m_nEditor);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
