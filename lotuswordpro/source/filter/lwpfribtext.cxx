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
#include "lwpglobalmgr.hxx"
#include "lwpfribtext.hxx"
#include "lwpcharsetmgr.hxx"
#include "lwphyperlinkmgr.hxx"
#include "lwptools.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xftextspan.hxx"
#include "xfilter/xfbookmark.hxx"
#include "xfilter/xfentry.hxx"
#include "xfilter/xftextcontent.hxx"
#include "xfilter/xfcrossref.hxx"
#include "xfilter/xfpagenumber.hxx"
#include "xfilter/xfdocfield.hxx"
#include "xfilter/xfdatestyle.hxx"
#include "xfilter/xftimestyle.hxx"
#include "xfilter/xfdate.hxx"
#include "xfilter/xfannotation.hxx"

LwpFribText::LwpFribText( LwpPara *pPara, sal_Bool bNoUnicode )
    : LwpFrib(pPara), m_bNoUnicode(bNoUnicode)
{};

void LwpFribText::Read(LwpObjectStream* pObjStrm, sal_uInt16 len)
{
    if( len>=1 )
    {
        rtl_TextEncoding rEncode;
        if(m_bNoUnicode)
        {
            rEncode = RTL_TEXTENCODING_ISO_8859_1;
        }
        else
        {
            if (m_pModifiers && m_pModifiers->CodePage)
                rEncode = LwpCharSetMgr::GetInstance()->
                                              GetTextCharEncoding(m_pModifiers->CodePage);
            else
                rEncode = LwpCharSetMgr::GetInstance()->GetTextCharEncoding();
        }
        LwpTools::QuickReadUnicode(pObjStrm, m_Content, len, rEncode);
    }
}

void LwpFribText::XFConvert(XFContentContainer* pXFPara,LwpStory* pStory)
{
    if (!pStory)
        return;
    LwpHyperlinkMgr* pHyperlink = pStory->GetHyperlinkMgr();
    if (pHyperlink->GetHyperlinkFlag())
        LwpFrib::ConvertHyperLink(pXFPara,pHyperlink,GetText());
    else
        LwpFrib::ConvertChars(pXFPara,GetText());
}

/**
 * @short:   Reading mothed of paranumber frib.
 * @param:   pObjStrm wordpro object stream.
 * @param:   len length of the frib
 */
void LwpFribParaNumber::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    pObjStrm->QuickRead(&m_nStyleID, sizeof(m_nStyleID));
    pObjStrm->QuickRead(&m_nNumberChar, sizeof(m_nNumberChar));
    pObjStrm->QuickRead(&m_nLevel, sizeof(m_nLevel));
    pObjStrm->QuickRead(&m_nStart, sizeof(m_nStart));
}

/**
 * @short:   default constructor of LwpFribDocVar
 */
LwpFribDocVar::LwpFribDocVar(LwpPara* pPara)
    : LwpFrib(pPara), m_nType(0), m_pName(new LwpAtomHolder)
{
}
/**
 * @short:   destructor of LwpFribDocVar
 */
LwpFribDocVar::~LwpFribDocVar()
{
    if (m_pName)
    {
        delete m_pName;
        m_pName = NULL;
    }
}
/**
 * @short:   Reading mothed of document variable frib.
 * @param:   pObjStrm wordpro object stream.
 * @param:   len length of the frib
 */
void LwpFribDocVar::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    pObjStrm->QuickRead(&m_nType, sizeof(m_nType));
    m_pName->Read(pObjStrm);
}

/**
 * @short:   register style for doc field (text style,date style)
 * @param:
 * @param:
 */
void LwpFribDocVar::RegisterStyle(LwpFoundry* pFoundry)
{
    LwpFrib::RegisterStyle(pFoundry);

    switch(m_nType)
    {
    case DATECREATED:
    case DATELASTREVISION:
    {
//		m_TimeStyle = LwpTools::RegisterDefaultTimeStyle();
        RegisterDefaultTimeStyle();
    }
        break;
    case TOTALEDITTIME:
        RegisterTotalTimeStyle();
        break;
    default:
        break;
    }
}

/**
 * @short:   register default time style for doc field
 * @param:
 * @param:
 */
void LwpFribDocVar::RegisterDefaultTimeStyle()
{
    XFDateStyle* pDateStyle = new XFDateStyle;//use the default format

    pDateStyle->AddMonth();
    pDateStyle->AddText(A2OUSTR("/"));
    pDateStyle->AddMonthDay();
    pDateStyle->AddText(A2OUSTR("/"));
    pDateStyle->AddYear();
    pDateStyle->AddText(A2OUSTR(","));
    pDateStyle->AddHour();
    pDateStyle->AddText(A2OUSTR(":"));
    pDateStyle->AddMinute();
    pDateStyle->AddText(A2OUSTR(":"));
    pDateStyle->AddSecond(sal_True,0);

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_TimeStyle = pXFStyleManager->AddStyle(pDateStyle)->GetStyleName();
}
void LwpFribDocVar::RegisterTotalTimeStyle()
{
    XFTimeStyle* pTimeStyle = new XFTimeStyle;//use the default format
    pTimeStyle->SetTruncate(sal_False);
    pTimeStyle->AddMinute();
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_TimeStyle = pXFStyleManager->AddStyle(pTimeStyle)->GetStyleName();
}

/**
 * @short:   convert the doc info field
 * @param:
 * @param:
 */
void LwpFribDocVar::XFConvert(XFContentContainer* pXFPara)
{
    XFContent* pContent=NULL;
    switch(m_nType)
    {
    case FILENAME:
    {
        pContent = new XFFileName;
        static_cast<XFFileName*>(pContent)->SetType(A2OUSTR("FileName"));
        break;
    }
    case PATH:
    {
        pContent = new XFFileName;
        static_cast<XFFileName*>(pContent)->SetType(A2OUSTR("Path"));
        break;
    }
    case DESCRIPTION:
        pContent = new XFDescription;
        break;
    case DATECREATED:
    {
        pContent = new XFCreateTime;
        pContent->SetStyleName(m_TimeStyle);
        break;
    }
    case DATELASTREVISION:
    {
        pContent = new XFLastEditTime;
        pContent->SetStyleName(m_TimeStyle);
        break;
    }
    case NUMPAGES:
        pContent = new XFPageCount;
        break;
    case NUMWORDS:
        pContent = new XFWordCount;
        break;
    case NUMCHARS:
        pContent = new XFCharCount;
        break;
    case TOTALEDITTIME:
    {
        pContent = new XFTotalEditTime;
        pContent->SetStyleName(m_TimeStyle);
        break;
    }
    case KEYWORDS:
        pContent = new XFKeywords;
        break;
    case CREATEDBY:
        pContent = new XFInitialCreator;
        break;
    case DOCSIZE:
    {
/*		pContent = new XFAnnotation;
        XFTextContent* pSpan = new XFTextContent();
        pSpan->SetText(A2OUSTR("Document Size is Here"));
        XFParagraph* pPara = new XFParagraph;
        pPara->Add(pSpan);
        static_cast<XFAnnotation*>(pContent)->Add(pPara);
        break;
*/
        OUString text = A2OUSTR("<Document Size>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case SMARTMASTER:
    {
        OUString text = A2OUSTR("<Smart master>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case DIVISIONNAME:
    {
        OUString text = A2OUSTR("<Division name>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case SECTIONNAME:
    {
        OUString text = A2OUSTR("<Section name>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case VERSIONCREATEBY:
    {
        OUString text = A2OUSTR("<Version Creat by>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case VERSIONCREATEDATE:
    {
        OUString text = A2OUSTR("<Version Creat date>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case VERSIONOTHEREDITORS:
    {
        OUString text = A2OUSTR("<Version other Editors>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case VERSIONNAME:
    {
        OUString text = A2OUSTR("<Version Name>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case VERSIONNUMBER:
    {
        OUString text = A2OUSTR("<Version Numbers>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case ALLVERSIONNAME:
    {
        OUString text = A2OUSTR("<All Version Name>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case VERSIONREMARK:
    {
        OUString text = A2OUSTR("<Version Remark>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case DOCUMENTCATEGORY:
    {
        OUString text = A2OUSTR("<Document Category>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case VERSIONLASTDATE:
    {
        OUString text = A2OUSTR("<Version Last Modify Date>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case VERSIONLASTEDITOR:
    {
        OUString text = A2OUSTR("<Version Last Editor>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case LASTEDIT:
    {
        OUString text = A2OUSTR("<Last Editor>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case OTHEREDITORS:
    {
        OUString text = A2OUSTR("<Other Editors>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    case NUMOFREVISION:
    {
        OUString text = A2OUSTR("<Number of Revision>");
        LwpFrib::ConvertChars(pXFPara,text);
        return;
    }
    default:
        return;
    }
    if (m_ModFlag)//(m_pModifiers)
    {
        XFTextSpan *pSpan = new XFTextSpan;
        pSpan->SetStyleName(GetStyleName());
        pSpan->Add(pContent);
        pXFPara->Add(pSpan);
    }
    else
        pXFPara->Add(pContent);

}
/**
 * @short:   Read unicode
 * @param:
 * @param:
 */
void LwpFribUnicode::Read(LwpObjectStream* pObjStrm, sal_uInt16 len)
{
    if(len>1)
    {
        rtl_TextEncoding rEncode;

        if (m_pModifiers && m_pModifiers->CodePage)
                rEncode = LwpCharSetMgr::GetInstance()->
                                          GetTextCharEncoding(m_pModifiers->CodePage);
        else
                rEncode = LwpCharSetMgr::GetInstance()->GetTextCharEncoding();

        LwpTools::QuickReadUnicode(pObjStrm, m_Content, len, rEncode);

    }
    else
        pObjStrm->SeekRel(len);
}

void LwpFribUnicode::XFConvert(XFContentContainer* pXFPara,LwpStory* pStory)
{
    if (!pStory)
        return;
    LwpHyperlinkMgr* pHyperlink = pStory->GetHyperlinkMgr();
    if (pHyperlink->GetHyperlinkFlag())
        LwpFrib::ConvertHyperLink(pXFPara,pHyperlink,GetText());
    else
        LwpFrib::ConvertChars(pXFPara,GetText());
}

/**
 * @short:   Read page number
 * @param:
 * @param:
 */
void LwpFribPageNumber::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    m_nNumStyle = pObjStrm->QuickReaduInt16();
    m_aBefText.Read(pObjStrm);
    m_aAfterText.Read(pObjStrm);
    m_nStartNum = pObjStrm->QuickReaduInt16();
    m_nStartOnPage = pObjStrm->QuickReaduInt16();
    m_nFlag = pObjStrm->QuickReaduInt16();
}

void LwpFribPageNumber::XFConvert(XFContentContainer* pXFPara)
{
    if (m_nNumStyle == 0x0)
        return;
    XFPageNumber* pNum = new XFPageNumber;
//	pNum->SetSelect(A2OUSTR("current"));
    switch(m_nNumStyle)
    {
    case 0x01:
        pNum->SetNumFmt(A2OUSTR("1"));
        break;
    case 0x02:
        pNum->SetNumFmt(A2OUSTR("A"));
        break;
    case 0x03:
        pNum->SetNumFmt(A2OUSTR("a"));
        break;
    case 0x04:
        pNum->SetNumFmt(A2OUSTR("I"));
        break;
    case 0x05:
        pNum->SetNumFmt(A2OUSTR("i"));
        break;
    }
    OUString styleName = GetStyleName();
    if (m_aBefText.str().getLength()>0)
    {
        OUString textStr = m_aBefText.str();
        XFTextSpan *pBef = new XFTextSpan(textStr,styleName);
        pXFPara->Add(pBef);
    }
    if (m_ModFlag)//(m_pModifiers)
    {
        XFTextSpan *pSpan = new XFTextSpan;
        pSpan->SetStyleName(styleName);
        pSpan->Add(pNum);
        pXFPara->Add(pSpan);
    }
    else
        pXFPara->Add(pNum);
    if (m_aAfterText.str().getLength()>0)
    {
        OUString textStr = m_aAfterText.str();
        XFTextSpan *pAfter = new XFTextSpan(textStr,styleName);
        pXFPara->Add(pAfter);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
