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
 May 2005			Created
 ************************************************************************/

#include "lwpfribmark.hxx"
#include "lwptools.hxx"
#include "lwpglobalmgr.hxx"
#include "lwpdoc.hxx"
#include "lwpdivinfo.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xftextspan.hxx"
#include "xfilter/xfbookmark.hxx"
#include "xfilter/xfentry.hxx"
#include "xfilter/xftextcontent.hxx"
#include "xfilter/xfcrossref.hxx"
#include "xfilter/xfdocfield.hxx"
#include "xfilter/xfdatestyle.hxx"
#include "xfilter/xftimestyle.hxx"
#include "xfilter/xfdate.hxx"
#include "xfilter/xfruby.hxx"
#include "lwpchangemgr.hxx"
#include "xfilter/xfchange.hxx"

/**
 * @short:   Read hyperlink frib and Click here block
 * @param:
 * @param:
 */
void LwpFribCHBlock::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    m_objMarker.ReadIndexed(pObjStrm);
    m_nType = pObjStrm->QuickReaduInt8();
}

LwpCHBlkMarker* LwpFribCHBlock::GetMarker()
{
    return static_cast<LwpCHBlkMarker*>(m_objMarker.obj());
}

void LwpFribCHBlock::XFConvert(XFContentContainer* pXFPara,LwpStory* pStory)
{
    sal_uInt8 type = GetType();
    if (!pStory)
        return;
    LwpCHBlkMarker* pMarker = GetMarker();

    if (!pMarker)
        return;
    sal_uInt16 nAction = pMarker->GetAction();
    if (nAction == LwpCHBlkMarker::CLICKHERE_CHBEHAVIORINTERNETLINK)//hyperlink
    {
        LwpHyperlinkMgr* pHyperlink = pStory->GetHyperlinkMgr();
        if (type == MARKER_START)
        {
            if (pHyperlink)
            {
                pHyperlink->SetHyperlinkFlag(sal_True);
                pHyperlink->SetHyperlink(pMarker->GetNamedProperty(A2OUSTR("URL")));
            }
        }
        else if (type == MARKER_END)//or none
        {
            pHyperlink->SetHyperlinkFlag(sal_False);
        }
    }
    else//click here block
    {
        pMarker->ConvertCHBlock(pXFPara,type);
    }
}
/**
 * @short:   register bookmark frib
 * @param:
 * @param:
 */
void  LwpFribBookMark::RegisterStyle(LwpFoundry* pFoundry)
{
    OUString name;
    LwpBookMark* pBook = pFoundry->GetBookMark(GetMarkerID());
    if (pBook)
        name = pBook->GetName();

    OUString sDivision;
    LwpDocument* pDoc = pFoundry->GetDocument();
    if (pDoc)
    {
        LwpObjectID* pID = pDoc->GetDivInfoID();
        if (!pID->IsNull())
        {
            sDivision = static_cast<LwpDivInfo*>(pID->obj(VO_DIVISIONINFO))->GetDivName();
        }
    }

    sal_uInt8 type = GetType();

    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
    LwpBookmarkMgr* pMarkMgr = pGlobal->GetLwpBookmarkMgr();
    if (type == MARKER_START)
    {
        XFBookmarkStart* pMarkStart = new XFBookmarkStart;
        pMarkStart->SetDivision(sDivision);
        pMarkStart->SetName(name);
        pMarkMgr->AddXFBookmarkStart(name,pMarkStart);//add to map
        m_pStart = pMarkStart;
    }
    else if(type == MARKER_END)
    {
        XFBookmarkEnd* pMarkEnd = new XFBookmarkEnd;
        pMarkEnd->SetDivision(sDivision);
        pMarkEnd->SetName(name);
        pMarkMgr->AddXFBookmarkEnd(name,pMarkEnd);	//add to map
        m_pEnd = pMarkEnd;
    }
}

LwpFribBookMark::LwpFribBookMark(LwpPara* pPara ) : LwpFrib(pPara),m_pStart(NULL),m_pEnd(NULL)
{
}

/**
 * @short:   Read bookmark frib
 * @param:
 * @param:
 */
void LwpFribBookMark::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    m_objMarker.ReadIndexed(pObjStrm);
    m_nType = pObjStrm->QuickReaduInt8();
}
/**
 * @short:   convert bookmark frib
 * @param:
 * @param:
 */
void LwpFribBookMark::XFConvert(XFContentContainer* pXFPara)
{
    sal_uInt8 type = GetType();

    if (type == MARKER_START && m_pStart)
    {
        pXFPara->Add(m_pStart);
    }
    else if(type == MARKER_END && m_pEnd)
    {
        pXFPara->Add(m_pEnd);
    }
}

/**
 * @short:   Read index entry frib
 * @param:
 * @param:
 */
LwpFribField::LwpFribField( LwpPara* pPara ) : LwpFrib(pPara)
{
    m_TimeStyle = A2OUSTR("");

    m_nSubType = 0;//datetime/crossref/other
    m_sFormula = A2OUSTR("");

    m_nCrossRefType = 0;
    m_nDateTimeType = 0;
}

void LwpFribField::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    m_objMarker.ReadIndexed(pObjStrm);
    m_nType = pObjStrm->QuickReaduInt8();
}

LwpFieldMark* LwpFribField::GetMarker()
{
    return static_cast<LwpFieldMark*>(m_objMarker.obj());
}

void LwpFribField::XFConvert(XFContentContainer* pXFPara)
{
    LwpFieldMark* pFieldMark = GetMarker();
    if (!pFieldMark)
        return;

    sal_uInt8 type = GetType();
    sal_uInt16 fieldType = pFieldMark->GetFieldType();

    OUString sChangeID;
    if(pFieldMark->GetRevisionFlag())
    {
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpChangeMgr* pChangeMgr = pGlobal->GetLwpChangeMgr();
        sChangeID = pChangeMgr->GetChangeID(pFieldMark->GetStartFrib());
    }

    CheckFieldType(pFieldMark);
    //end marker
    if (type == MARKER_END)
    {
        if (pFieldMark->GetStart() == sal_False)
            return;
        if (pFieldMark->IsFormulaInsert())
        {
            XFTextContent *pSpan = new XFTextContent();
            pSpan->SetText(A2OUSTR(">"));
            pXFPara->Add(pSpan);
        }
        if (fieldType == LwpFieldMark::FLD_FIELD)
        {
            if (m_nSubType == SUBFIELD_DATETIME)
            {
                ConvertDateTimeEnd(pXFPara,pFieldMark);
            }
            else if (m_nSubType == SUBFIELD_CROSSREF)
            {
                ConvertCrossRefEnd(pXFPara,pFieldMark);
            }
            else if (m_nSubType == SUBFIELD_DOCPOWER)
            {
                ConvertDocFieldEnd(pXFPara,pFieldMark);
            }
        }
        if(pFieldMark->GetRevisionFlag() && sChangeID.getLength())
        {
            XFChangeEnd* pChangeEnd = new XFChangeEnd;
            pChangeEnd->SetChangeID(sChangeID);
            pXFPara->Add(pChangeEnd);
        }

        return;
    }

    //start marker
    if(pFieldMark->GetRevisionFlag() &&  sChangeID.getLength())
    {
        XFChangeStart* pChangeStart = new XFChangeStart;
        pChangeStart->SetChangeID(sChangeID);
        pXFPara->Add(pChangeStart);
    }

    if (fieldType == LwpFieldMark::FLD_INDEX)
    {
        OUString sKey1,sKey2;
        pFieldMark->ParseIndex(sKey1,sKey2);
        if (sKey1.getLength()>0)
        {
            XFEntry* pEntry = new XFEntry;
            pEntry->SetEntryType(enumXFEntryAlphabetical);
            pEntry->SetKey(sKey1,sKey2);
            pXFPara->Add(pEntry);
        }
    }
    else if (fieldType == LwpFieldMark::FLD_TOC)
    {
        OUString sLevel,sText;
        pFieldMark->ParseTOC(sLevel,sText);
        if (sLevel.getLength()>0 && sText.getLength()>0)
        {
            XFEntry* pEntry = new XFEntry;
            pEntry->SetEntryType(enumXFEntryTOC);
            pEntry->SetStringValue(sText);
            pEntry->SetOutlineLevel(sLevel.toInt32(10));
            pXFPara->Add(pEntry);
        }
    }
    else if (fieldType == LwpFieldMark::FLD_FIELD)
    {
        sal_uInt8 nDatetype;
        OUString sFormula;
        /*sal_Bool bIsDateTime =*/ pFieldMark->IsDateTimeField(nDatetype,sFormula);

        if (m_nSubType == SUBFIELD_DATETIME)//date time
        {
            ConvertDateTimeStart(pXFPara,pFieldMark);
        }
        else if (m_nSubType == SUBFIELD_CROSSREF)//cross ref
        {
            ConvertCrossRefStart(pXFPara,pFieldMark);
        }
        else if (m_nSubType == SUBFIELD_DOCPOWER)
        {
            ConvertDocFieldStart(pXFPara,pFieldMark);
        }
    }

    if (pFieldMark->IsFormulaInsert())
    {
        XFTextContent *pSpan = new XFTextContent();
        pSpan->SetText(A2OUSTR("<"));
        pXFPara->Add(pSpan);
    }

    pFieldMark->SetStart(sal_True);
}

void LwpFribField::RegisterStyle(LwpFoundry* pFoundry)
{
    LwpFrib::RegisterStyle(pFoundry);
    LwpFieldMark* pFieldMark = GetMarker();
    if (!pFieldMark)
        return;

    sal_uInt8 type = GetType();
    sal_uInt16 fieldType = pFieldMark->GetFieldType();

    if (type == MARKER_START && fieldType == LwpFieldMark::FLD_FIELD)
    {
        RegisterTimeField(pFieldMark);
    }

    if (type == MARKER_START && m_bRevisionFlag)
    {
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpChangeMgr* pChangeMgr = pGlobal->GetLwpChangeMgr();
        pChangeMgr->AddChangeFrib(this);
        pFieldMark->SetStartFrib(this);
        pFieldMark->SetRevisionFlag(sal_True);
    }
}

void LwpFribField::RegisterTimeField(LwpFieldMark* pFieldMark)
{
    OUString sFormula = pFieldMark->GetFormula();//now bookmark maybe not all register to bookmarkmgr,
    if (sFormula == A2OUSTR("TotalEditingTime"))//so check field type now is not correct.
        RegisterTotalTimeStyle();
    else
    {
        sal_Int32 index;
        sal_Unicode ch1(0x0020);//space
        OUString tag;
        index = sFormula.indexOf(ch1,0);
        if (index < 0)
            return;

        tag = sFormula.copy(0,index);
        if (tag == A2OUSTR("Now()") || tag == A2OUSTR("CreateDate") ||  tag == A2OUSTR("EditDate"))
            RegisterDateTimeStyle(sFormula.copy(index+1,sFormula.getLength()-index-1));
    }
}

void LwpFribField::RegisterTotalTimeStyle()
{
    XFTimeStyle* pTimeStyle = new XFTimeStyle;//use the default format
    pTimeStyle->SetTruncate(sal_False);
    pTimeStyle->AddMinute();
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_TimeStyle = pXFStyleManager->AddStyle(pTimeStyle)->GetStyleName();
}

void LwpFribField::RegisterDateTimeStyle(OUString sFormula)
{
    XFDateStyle* pDateStyle = NULL;
    XFTimeStyle* pTimeStyle = NULL;
//DATE
    if (sFormula.getLength()<2)
        return;
    if (sFormula[1] == 'F')
    {
        if (sFormula == A2OUSTR("%FLSystemShortDate"))
        {
            pDateStyle = LwpTools::GetSystemDateStyle(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLSystemLongDate"))
        {
            pDateStyle = LwpTools::GetSystemDateStyle(sal_True);
        }
        else if (sFormula == A2OUSTR("%FLISODate1") || sFormula == A2OUSTR("%FLYYYY/MM/DD") )
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddYear();
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddMonth();
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddMonthDay();
        }
        else if (sFormula == A2OUSTR("%FLISODate2") || sFormula == A2OUSTR("%FLYYYY/MM/DD HH:mm:SS") )
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddYear();
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddMonth();
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddHour();
            pDateStyle->AddText(A2OUSTR(":"));
            pDateStyle->AddMinute();
            pDateStyle->AddText(A2OUSTR(":"));
            pDateStyle->AddSecond(sal_True,0);
        }
        else if (sFormula == A2OUSTR("%FLM/D/YY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_False);
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddYear(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLMonth D, YYYY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLWeekday, Month D, YYYY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLMn D, YYYY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_False,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLWday, Mn D, YYYY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay(sal_False);
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddMonth(sal_False,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLMn D"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_False,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLWeekday, Mn D"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddMonth(sal_False,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLMn D, YY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_False,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddYear(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLM/D"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_False);
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddMonthDay(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLM/YY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_False);
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddYear(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLMn YY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_False,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddYear(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLWeekday"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay();
        }
        else if (sFormula == A2OUSTR("%FLWday"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLMonth"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_True,sal_True);
        }
        else if (sFormula == A2OUSTR("%FLMn"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_False,sal_True);
        }
        else if (sFormula == A2OUSTR("%FLD"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLM"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLYYYY")	)
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLYY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddYear(sal_False);
        }
        //chinese version begin
        else if (sFormula == A2OUSTR("%FLYY/M/D"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddYear(sal_False);
            pDateStyle->AddText(A2OUSTR("-"));
            pDateStyle->AddMonth(sal_False);
            pDateStyle->AddText(A2OUSTR("-"));
            pDateStyle->AddMonthDay(sal_False);
        }
        else if(sFormula == A2OUSTR("%FLYYYY Month D"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddYear();
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
        }
        else if(sFormula == A2OUSTR("%FLWeekday, YYYY Month D"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(A2OUSTR(","));
            pDateStyle->AddYear();
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
        }
        else if(sFormula == A2OUSTR("%FLYYYY Mn D"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddYear();
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_False,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
        }
        else if(sFormula == A2OUSTR("%FLWday, YYYY Mn D"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay(sal_False);
            pDateStyle->AddText(A2OUSTR(","));
            pDateStyle->AddYear();
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
        }
        //next 2 are the same with english version
        else if (sFormula == A2OUSTR("%FLYY Mn D"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddYear(sal_False);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_False,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLYY/M"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_False);
            pDateStyle->AddText(A2OUSTR("-"));
            pDateStyle->AddMonth(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLYY Mn"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_False);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_False,sal_True);
        }
        else if (sFormula == A2OUSTR("%FLeeeeoa") || sFormula == A2OUSTR("%FLffffooaa") || sFormula == A2OUSTR("%FLEEEEOA"))
        {
            pDateStyle = new XFDateStyle;
    //		OUString lang = OUString::createFromAscii("zh-CN");
    //		pDateStyle->SetLanguage( lang);
            rtl::OUString sText;
            pDateStyle->AddYear();
            sText = rtl::OUString(0x5e74);
            pDateStyle->AddText(sText);
            pDateStyle->AddMonth(sal_False);
            sText = rtl::OUString(0x6708);
            pDateStyle->AddText(sText);
            pDateStyle->AddMonthDay(sal_False);
            sText = rtl::OUString(0x65e5);
            pDateStyle->AddText(sText);
        }
        else if (sFormula == A2OUSTR("%FLoa") || sFormula == A2OUSTR("%FLooaa") || sFormula == A2OUSTR("%FLOA") )
        {
            pDateStyle = new XFDateStyle;
            OUString sText;
            pDateStyle->AddMonth(sal_False);
            sText = rtl::OUString(0x6708);
            pDateStyle->AddText(sText);
            pDateStyle->AddMonthDay(sal_False);
            sText = rtl::OUString(0x65e5);
            pDateStyle->AddText(sText);
        }
        else if (sFormula == A2OUSTR("%FLYYYY/M/D") || sFormula == A2OUSTR("%FLGGGG/od/ad"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddYear();
            pDateStyle->AddText(A2OUSTR("-"));
            pDateStyle->AddMonth(sal_False);
            pDateStyle->AddText(A2OUSTR("-"));
            pDateStyle->AddMonthDay(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLYY.M.D") )
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddYear(sal_False);
            pDateStyle->AddText(A2OUSTR("."));
            pDateStyle->AddMonth(sal_False);
            pDateStyle->AddText(A2OUSTR("."));
            pDateStyle->AddMonthDay(sal_False);
        }
        //chinese version end
        //other version begin
        else if (sFormula == A2OUSTR("%FLWeekday, Month DD, YYYY") )
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLYYYY/MM/DD") )
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLD/M/YY") )
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddMonth(sal_False);
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddYear(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLD Month YYYY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLDD Month YYYY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLWeekday, D Month YYYY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLWeekday, DD Month YYYY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLD Mn YYYY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_False,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLWday, D Mn YYYY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay(sal_False);
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_False,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLWday, DD Mn YYYY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay(sal_False);
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_False,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%FLD Mn"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_False,sal_True);
        }
        else if (sFormula == A2OUSTR("%FLDD Mn"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_False,sal_True);
        }
        else if (sFormula == A2OUSTR("%FLWeekday, D Mn"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_False,sal_True);
        }
        else if (sFormula == A2OUSTR("%FLWeekday, DD Mn"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_False,sal_True);
        }
        else if (sFormula == A2OUSTR("%FLD Mn YY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_False,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddYear(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLDD Mn YY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_False,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddYear(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLD/M"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddMonth(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLDD/MM"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddMonth();
        }
        else if(sFormula == A2OUSTR("%FLDD/MM/YY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddMonth();
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddYear(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLMM/YY"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth();
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddYear(sal_False);
        }
        //other version end
    //TIME
        else if (sFormula == A2OUSTR("%FLSystemTime"))
        {
            pTimeStyle = LwpTools::GetSystemTimeStyle();
        }
        else if (sFormula == A2OUSTR("%FLISOTime1") || sFormula == A2OUSTR("%FLH:mm:SS"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddSecond(sal_True,0);
        }
        else if (sFormula == A2OUSTR("%FLH:mm ampm"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour(sal_False);
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(A2OUSTR(" "));
            pTimeStyle->SetAmPm(sal_True);
        }
        else if (sFormula == A2OUSTR("%FLH:mm"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour(sal_False);
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddMinute();
        }
        else if (sFormula == A2OUSTR("%FLH:mm:SS ampm"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour(sal_False);
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddSecond(sal_True,0);
            pTimeStyle->AddText(A2OUSTR(" "));
            pTimeStyle->SetAmPm(sal_True);
        }
        else if (sFormula == A2OUSTR("%FLH:mm:SS"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour(sal_False);
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddSecond(sal_True,0);
        }
        else if (sFormula == A2OUSTR("%FLH ampm"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour(sal_False);
            pTimeStyle->AddText(A2OUSTR(" "));
            pTimeStyle->SetAmPm(sal_True);
        }
        else if (sFormula == A2OUSTR("%FLH"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour(sal_False);
        }
        else if (sFormula == A2OUSTR("%FLmm"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddMinute();
        }
        else if (sFormula == A2OUSTR("%FLSS"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddSecond(sal_True,0);
        }
        else if (sFormula == A2OUSTR("%FLampm"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->SetAmPm(sal_True);
        }
        //chinese version begin
        else	 if (sFormula == A2OUSTR("%FLjF") || sFormula == A2OUSTR("%FLJFF")
            || sFormula == A2OUSTR("%FLjjF") || sFormula == A2OUSTR("%FLJJFF ") )
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour(sal_False);
            rtl::OUString sText;
            sText = rtl::OUString(0x70b9);
            pTimeStyle->AddText(sText);
            pTimeStyle->AddMinute(sal_False);
            sText = rtl::OUString(0x5206);
            pTimeStyle->AddText(sText);
        }
        else if (sFormula == A2OUSTR("%FLjjjF") || sFormula == A2OUSTR("%FLJJJFF") )
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->SetAmPm(sal_True);
            pTimeStyle->AddHour(sal_False);
            rtl::OUString sText;
            sText = rtl::OUString(0x70b9);
            pTimeStyle->AddText(sText);
            pTimeStyle->AddMinute(sal_False);
            sText = rtl::OUString(0x5206);
            pTimeStyle->AddText(sText);
        }
        //chinese version end
        //other version begin
        else if (sFormula == A2OUSTR("%FLHH:mm ampm"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(A2OUSTR(" "));
            pTimeStyle->SetAmPm(sal_True);
        }
        else if (sFormula == A2OUSTR("%FLHH ampm"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour();
            pTimeStyle->AddText(A2OUSTR(" "));
            pTimeStyle->SetAmPm(sal_True);
        }
        else if (sFormula == A2OUSTR("%FLHH:mm:SS ampm"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddSecond(sal_True,0);
            pTimeStyle->AddText(A2OUSTR(" "));
            pTimeStyle->SetAmPm(sal_True);
        }
        else if (sFormula == A2OUSTR("%FLHH:mm"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddMinute();
        }
        else if (sFormula == A2OUSTR("%FLHH:mm:SS"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddSecond(sal_True,0);
        }
        //other version end
    }
    else if(sFormula[1] == 'D')
    {
        if (sFormula == A2OUSTR("%Da"))
        {
            pDateStyle = LwpTools::GetSystemDateStyle(sal_False);
        }
        else if (sFormula == A2OUSTR("%DB") || sFormula == A2OUSTR("%Db"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%Dc") || sFormula == A2OUSTR("%DC"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%Dd") || sFormula == A2OUSTR("%DD"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR(", "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%DE") || sFormula == A2OUSTR("%De"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
        }
        else if (sFormula == A2OUSTR("%Df") || sFormula == A2OUSTR("%DF"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
        }
        else if (sFormula == A2OUSTR("%Dg"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_False);
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddMonthDay(sal_False);
        }
        else if (sFormula == A2OUSTR("%Dh"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_False);
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%Di") || sFormula == A2OUSTR("%DI"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR("."));
            pDateStyle->AddMonth(sal_True,sal_True);
        }
        else if (sFormula == A2OUSTR("%Dj") || sFormula == A2OUSTR("%DJ"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay(sal_False);
            pDateStyle->AddText(A2OUSTR("."));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%Dk") || sFormula == A2OUSTR("%DK"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddYear();
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(" "));
            pDateStyle->AddMonthDay(sal_False);
        }
        else if (sFormula == A2OUSTR("%DL") || sFormula == A2OUSTR("%Dl"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonth(sal_True,sal_True);
            pDateStyle->AddText(A2OUSTR(","));
            pDateStyle->AddYear();
        }
        else if (sFormula == A2OUSTR("%Dm"))
        {
            pDateStyle = new XFDateStyle;
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddMonth();
            pDateStyle->AddText(A2OUSTR("/"));
            pDateStyle->AddYear();
        }
    }
    else if (sFormula[1] == 'T')
    {
        if (sFormula == A2OUSTR("%T1"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddMinute();
        }
        else if (sFormula == A2OUSTR("%T2") || sFormula == A2OUSTR("%T6")
         || sFormula == A2OUSTR("%T4") || sFormula == A2OUSTR("%T8") )
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour(sal_False);
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->SetAmPm(sal_True);
        }
        else if (sFormula == A2OUSTR("%T3") || sFormula == A2OUSTR("%T7")
         || sFormula == A2OUSTR("%T5") || sFormula == A2OUSTR("%T9"))
        {
            pTimeStyle = new XFTimeStyle;
            pTimeStyle->AddHour();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(A2OUSTR(":"));
            pTimeStyle->SetAmPm(sal_True);
        }
    }
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    if (pDateStyle)
        m_TimeStyle = pXFStyleManager->AddStyle(pDateStyle)->GetStyleName();
    if (pTimeStyle)
        m_TimeStyle = pXFStyleManager->AddStyle(pTimeStyle)->GetStyleName();
}

void LwpFribField::CheckFieldType(LwpFieldMark* pFieldMark)
{
    sal_uInt8 nType;
    OUString sFormula;
    if (pFieldMark->IsDateTimeField(nType,sFormula))
    {
        m_nDateTimeType = nType;
        m_sFormula = sFormula;
        m_nSubType = SUBFIELD_DATETIME;
    }
    else if (pFieldMark->IsCrossRefField(nType,sFormula))
    {
        m_nCrossRefType = nType;
        m_sFormula = sFormula;
        m_nSubType = SUBFIELD_CROSSREF;
    }
    else if (pFieldMark->IsDocPowerField(nType,sFormula))
    {
        m_nDocPowerType = nType;
        m_nSubType = SUBFIELD_DOCPOWER;
    }
    else
    {
        m_nSubType = SUBFIELD_INVALID;
    }
}

void LwpFribField::ConvertDocFieldStart(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark)
{
    XFContent* pContent = NULL;
    switch(m_nDocPowerType)
    {
        case LwpFieldMark::DOC_DESCRIPTION:
        {
            pContent = new XFDescriptionStart;
            break;
        }
        case LwpFieldMark::DOC_NUMPAGES:
        {
            pContent = new XFPageCountStart;
            break;
        }
        case LwpFieldMark::DOC_NUMWORDS:
        {
            pContent = new XFWordCountStart;
            break;
        }
        case LwpFieldMark::DOC_NUMCHARS:
        {
            pContent = new XFCharCountStart;
            break;
        }
//		case LwpFieldMark::DOC_TOTALTIME:
//		{
//			pContent = new XFTotalEditTimeStart;
//			pContent->SetStyleName(m_TimeStyle);
//			break;
//		}
    }
    if (pContent)
    {
        if (m_ModFlag)//(m_pModifiers)
        {
            XFTextSpanStart* pSpan = new XFTextSpanStart;
            pSpan->SetStyleName(GetStyleName());
            pSpan->Add(pContent);
            pXFPara->Add(pSpan);
            pFieldMark->SetStyleFlag(sal_True);
        }
        else
            pXFPara->Add(pContent);
    }
}

void LwpFribField::ConvertDocFieldEnd(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark)
{
    XFContent* pContent = NULL;
    switch(m_nDocPowerType)
    {
        case LwpFieldMark::DOC_DESCRIPTION:
        {
            pContent = new XFDescriptionEnd;
            break;
        }
        case LwpFieldMark::DOC_NUMPAGES:
        {
            pContent = new XFPageCountEnd;
            break;
        }
        case LwpFieldMark::DOC_NUMWORDS:
        {
            pContent = new XFWordCountEnd;
            break;
        }
        case LwpFieldMark::DOC_NUMCHARS:
        {
            pContent = new XFCharCountEnd;
            break;
        }
//		case LwpFieldMark::DOC_TOTALTIME:
//		{
//			pContent = new XFTotalEditTimeEnd;
//			break;
//		}
    }
    if (pContent)
    {
        if (pFieldMark->GetStyleFlag())
        {
            XFTextSpanEnd* pSpan = new XFTextSpanEnd;
            pSpan->Add(pContent);
            pXFPara->Add(pSpan);
        }
        else
            pXFPara->Add(pContent);
    }
}

void LwpFribField::ConvertDateTimeStart(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark)
{
    XFContent* pContent = NULL;
    switch(m_nDateTimeType)
    {
    case LwpFieldMark::DATETIME_NOW:
    {
        pContent = new XFDateStart;
        pContent->SetStyleName(m_TimeStyle);
        break;
    }
    case LwpFieldMark::DATETIME_CREATE:
    {
        pContent = new XFCreateTimeStart;
        pContent->SetStyleName(m_TimeStyle);
        break;
    }
    case LwpFieldMark::DATETIME_LASTEDIT:
    {
        pContent = new XFLastEditTimeStart;
        pContent->SetStyleName(m_TimeStyle);
        break;
    }
    case LwpFieldMark::DATETIME_TOTALTIME:
    {
        pContent = new XFTotalEditTimeStart;
        pContent->SetStyleName(m_TimeStyle);
        break;
    }
    default:
        break;
    }
    if (pContent)
    {
        if (m_ModFlag)//(m_pModifiers)
        {
            XFTextSpanStart* pSpan = new XFTextSpanStart;
            pSpan->SetStyleName(GetStyleName());
            pSpan->Add(pContent);
            pXFPara->Add(pSpan);
            pFieldMark->SetStyleFlag(sal_True);
        }
        else
            pXFPara->Add(pContent);
    }

}

void LwpFribField::ConvertDateTimeEnd(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark)
{
    XFContent* pContent = NULL;
    switch(m_nDateTimeType)
    {
        case LwpFieldMark::DATETIME_NOW:
        {
            pContent = new XFDateEnd;
            break;
        }
        case LwpFieldMark::DATETIME_CREATE:
        {
            pContent = new XFCreateTimeEnd;
            break;
        }
        case LwpFieldMark::DATETIME_LASTEDIT:
        {
            pContent = new XFLastEditTimeEnd;
            break;
        }
        case LwpFieldMark::DATETIME_TOTALTIME:
        {
            pContent = new XFTotalEditTimeEnd;
            break;
        }
    }
    if (pContent)
    {
        if (pFieldMark->GetStyleFlag())
        {
            XFTextSpanEnd* pSpan = new XFTextSpanEnd;
            pSpan->Add(pContent);
            pXFPara->Add(pSpan);
        }
        else
            pXFPara->Add(pContent);
    }
}

void LwpFribField::ConvertCrossRefStart(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark)
{
    XFCrossRefStart* pRef = new XFCrossRefStart;
    pRef->SetRefType(m_nCrossRefType);
    pRef->SetMarkName(m_sFormula);
//	pFieldMark->SetStart(sal_True);//for some disnormal cases
    if (m_ModFlag)
    {
        XFTextSpanStart* pSpan = new XFTextSpanStart;
        pSpan->SetStyleName(GetStyleName());
        pSpan->Add(pRef);
        pXFPara->Add(pSpan);
        pFieldMark->SetStyleFlag(sal_True);
    }
    else
        pXFPara->Add(pRef);
}

void LwpFribField::ConvertCrossRefEnd(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark)
{
    XFCrossRefEnd* pRef = new XFCrossRefEnd;
    if (pFieldMark->GetStyleFlag())
    {
        XFTextSpanEnd* pSpan = new XFTextSpanEnd;
        pSpan->Add(pRef);
        pXFPara->Add(pSpan);
    }
    else
        pXFPara->Add(pRef);
}

LwpFribRubyMarker::LwpFribRubyMarker( LwpPara* pPara ): LwpFrib(pPara)
{
}

LwpRubyMarker* LwpFribRubyMarker::GetMarker()
{
    return static_cast<LwpRubyMarker*>(m_objMarker.obj(VO_RUBYMARKER));
}

void LwpFribRubyMarker::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    m_objMarker.ReadIndexed(pObjStrm);
    m_nType = pObjStrm->QuickReaduInt8();
}

void LwpFribRubyMarker::XFConvert(XFContentContainer* pXFPara)
{
    sal_uInt8 type = GetType();
    LwpRubyMarker* pMarker = GetMarker();

    if (type == MARKER_START)
    {
        XFRubyStart* pRubyStart = new XFRubyStart;
        if (pMarker)
        {
            pRubyStart->SetStyleName(pMarker->GetRubyStyleName());
        }
        pXFPara->Add(pRubyStart);
    }
    else if(type == MARKER_END)
    {
        XFRubyEnd* pRubyEnd = new XFRubyEnd;
        if (pMarker)
        {
            pRubyEnd->SetText(pMarker->GetRubyText());
            pRubyEnd->SetStyleName(pMarker->GetTextStyleName());
        }
        pXFPara->Add(pRubyEnd);
    }
}

void LwpFribRubyMarker::RegisterStyle(LwpFoundry* /*pFoundry*/)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
