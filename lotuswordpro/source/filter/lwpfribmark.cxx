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

#include "lwpfribmark.hxx"
#include <lwptools.hxx>
#include <lwpglobalmgr.hxx>
#include "lwpdoc.hxx"
#include "lwpdivinfo.hxx"
#include <xfilter/xfstylemanager.hxx>
#include <xfilter/xftextspan.hxx>
#include <xfilter/xfbookmark.hxx>
#include <xfilter/xfentry.hxx>
#include <xfilter/xftextcontent.hxx>
#include <xfilter/xfcrossref.hxx>
#include <xfilter/xfdocfield.hxx>
#include <xfilter/xfdatestyle.hxx>
#include <xfilter/xftimestyle.hxx>
#include <xfilter/xfdate.hxx>
#include <xfilter/xfruby.hxx>
#include <lwpchangemgr.hxx>
#include <xfilter/xfchange.hxx>

/**
 * @short:   Read hyperlink frib and Click here block
 */
void LwpFribCHBlock::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    m_objMarker.ReadIndexed(pObjStrm);
    m_nType = pObjStrm->QuickReaduInt8();
}

void LwpFribCHBlock::XFConvert(XFContentContainer* pXFPara,LwpStory* pStory)
{
    sal_uInt8 type = GetType();
    if (!pStory)
        return;
    LwpCHBlkMarker* pMarker = dynamic_cast<LwpCHBlkMarker*>(m_objMarker.obj().get());

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
                pHyperlink->SetHyperlinkFlag(true);
                pHyperlink->SetHyperlink(pMarker->GetNamedProperty("URL"));
            }
        }
        else if (type == MARKER_END)//or none
        {
            pHyperlink->SetHyperlinkFlag(false);
        }
    }
    else//click here block
    {
        pMarker->ConvertCHBlock(pXFPara,type);
    }
}
/**
 * @short:   register bookmark frib
 */
void  LwpFribBookMark::RegisterStyle(LwpFoundry* pFoundry)
{
    OUString name;
    LwpBookMark* pBook = pFoundry ? pFoundry->GetBookMark(GetMarkerID()) : nullptr;
    if (pBook)
        name = pBook->GetName();

    OUString sDivision;
    LwpDocument* pDoc = pFoundry ? pFoundry->GetDocument() : nullptr;
    if (pDoc)
    {
        LwpObjectID& rID = pDoc->GetDivInfoID();
        if (!rID.IsNull())
        {
            LwpDivInfo *pDivInvo = dynamic_cast<LwpDivInfo*>(rID.obj(VO_DIVISIONINFO).get());
            if (pDivInvo)
                sDivision = pDivInvo->GetDivName();
        }
    }

    sal_uInt8 type = GetType();

    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
    LwpBookmarkMgr* pMarkMgr = pGlobal->GetLwpBookmarkMgr();
    if (type == MARKER_START)
    {
        rtl::Reference<XFBookmarkStart> xMarkStart(new XFBookmarkStart);
        xMarkStart->SetDivision(sDivision);
        xMarkStart->SetName(name);
        pMarkMgr->AddXFBookmarkStart(name, xMarkStart.get());//add to map
        m_xStart = xMarkStart;
    }
    else if(type == MARKER_END)
    {
        rtl::Reference<XFBookmarkEnd> xMarkEnd(new XFBookmarkEnd);
        xMarkEnd->SetDivision(sDivision);
        xMarkEnd->SetName(name);
        pMarkMgr->AddXFBookmarkEnd(name, xMarkEnd.get());  //add to map
        m_xEnd = xMarkEnd;
    }
}

LwpFribBookMark::LwpFribBookMark(LwpPara* pPara )
    : LwpFrib(pPara)
    , m_nType(0)
{
}

/**
 * @short:   Read bookmark frib
 */
void LwpFribBookMark::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    m_objMarker.ReadIndexed(pObjStrm);
    m_nType = pObjStrm->QuickReaduInt8();
}
/**
 * @short:   convert bookmark frib
 */
void LwpFribBookMark::XFConvert(XFContentContainer* pXFPara)
{
    sal_uInt8 type = GetType();

    if (type == MARKER_START && m_xStart)
    {
        pXFPara->Add(m_xStart.get());
    }
    else if(type == MARKER_END && m_xEnd)
    {
        pXFPara->Add(m_xEnd.get());
    }
}

/**
 * @short:   Read index entry frib
 */
LwpFribField::LwpFribField( LwpPara* pPara )
    : LwpFrib(pPara)
    , m_nType(0)
    , m_TimeStyle("")
    , m_nSubType(0)
    , m_sFormula("")
    , m_nCrossRefType(0)
    , m_nDateTimeType(0)
    , m_nDocPowerType(0)
{
}

void LwpFribField::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    m_objMarker.ReadIndexed(pObjStrm);
    m_nType = pObjStrm->QuickReaduInt8();
}

LwpFieldMark* LwpFribField::GetMarker()
{
    return dynamic_cast<LwpFieldMark*>(m_objMarker.obj().get());
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
        if (!pFieldMark->GetStart())
            return;
        if (pFieldMark->IsFormulaInsert())
        {
            XFTextContent *pSpan = new XFTextContent();
            pSpan->SetText(">");
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
        if(pFieldMark->GetRevisionFlag() && !sChangeID.isEmpty())
        {
            XFChangeEnd* pChangeEnd = new XFChangeEnd;
            pChangeEnd->SetChangeID(sChangeID);
            pXFPara->Add(pChangeEnd);
        }

        return;
    }

    //start marker
    if(pFieldMark->GetRevisionFlag() && !sChangeID.isEmpty())
    {
        XFChangeStart* pChangeStart = new XFChangeStart;
        pChangeStart->SetChangeID(sChangeID);
        pXFPara->Add(pChangeStart);
    }

    if (fieldType == LwpFieldMark::FLD_INDEX)
    {
        OUString sKey1,sKey2;
        pFieldMark->ParseIndex(sKey1,sKey2);
        if (!sKey1.isEmpty())
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
        if (!sLevel.isEmpty() && !sText.isEmpty())
        {
            XFEntry* pEntry = new XFEntry;
            pEntry->SetEntryType(enumXFEntryTOC);
            pEntry->SetStringValue(sText);
            pEntry->SetOutlineLevel(sLevel.toInt32());
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
        pSpan->SetText("<");
        pXFPara->Add(pSpan);
    }

    pFieldMark->SetStart(true);
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
        pFieldMark->SetRevisionFlag(true);
    }
}

void LwpFribField::RegisterTimeField(LwpFieldMark* pFieldMark)
{
    OUString sFormula = pFieldMark->GetFormula();//now bookmark maybe not all register to bookmarkmgr,
    if (sFormula == "TotalEditingTime")//so check field type now is not correct.
        RegisterTotalTimeStyle();
    else
    {
        sal_Int32 index = sFormula.indexOf(0x20); //space
        if (index < 0)
            return;

        OUString tag = sFormula.copy(0,index);
        if (tag == "Now()" || tag == "CreateDate" ||  tag == "EditDate")
            RegisterDateTimeStyle(sFormula.copy(index+1));
    }
}

void LwpFribField::RegisterTotalTimeStyle()
{
    std::unique_ptr<XFTimeStyle> pTimeStyle(new XFTimeStyle);//use the default format
    pTimeStyle->SetTruncate(false);
    pTimeStyle->AddMinute();
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_TimeStyle = pXFStyleManager->AddStyle(std::move(pTimeStyle)).m_pStyle->GetStyleName();
}

void LwpFribField::RegisterDateTimeStyle(const OUString& sFormula)
{
    std::unique_ptr<XFDateStyle> pDateStyle;
    std::unique_ptr<XFTimeStyle> pTimeStyle;
//DATE
    if (sFormula.getLength()<2)
        return;
    if (sFormula[1] == 'F')
    {
        if (sFormula == "%FLSystemShortDate")
        {
            pDateStyle = LwpTools::GetSystemDateStyle(false);
        }
        else if (sFormula == "%FLSystemLongDate")
        {
            pDateStyle = LwpTools::GetSystemDateStyle(true);
        }
        else if (sFormula == "%FLISODate1" || sFormula == "%FLYYYY/MM/DD" )
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddYear();
            pDateStyle->AddText("/");
            pDateStyle->AddMonth();
            pDateStyle->AddText("/");
            pDateStyle->AddMonthDay();
        }
        else if (sFormula == "%FLISODate2" || sFormula == "%FLYYYY/MM/DD HH:mm:SS" )
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddYear();
            pDateStyle->AddText("/");
            pDateStyle->AddMonth();
            pDateStyle->AddText("/");
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(" ");
            pDateStyle->AddHour();
            pDateStyle->AddText(":");
            pDateStyle->AddMinute();
            pDateStyle->AddText(":");
            pDateStyle->AddSecond();
        }
        else if (sFormula == "%FLM/D/YY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(false);
            pDateStyle->AddText("/");
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText("/");
            pDateStyle->AddYear(false);
        }
        else if (sFormula == "%FLMonth D, YYYY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(", ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLWeekday, Month D, YYYY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(", ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(", ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLMn D, YYYY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(false,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(", ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLWday, Mn D, YYYY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay(false);
            pDateStyle->AddText(", ");
            pDateStyle->AddMonth(false,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(", ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLMn D")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(false,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
        }
        else if (sFormula == "%FLWeekday, Mn D")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(", ");
            pDateStyle->AddMonth(false,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
        }
        else if (sFormula == "%FLMn D, YY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(false,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(", ");
            pDateStyle->AddYear(false);
        }
        else if (sFormula == "%FLM/D")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(false);
            pDateStyle->AddText("/");
            pDateStyle->AddMonthDay(false);
        }
        else if (sFormula == "%FLM/YY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(false);
            pDateStyle->AddText("/");
            pDateStyle->AddYear(false);
        }
        else if (sFormula == "%FLMn YY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(false,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddYear(false);
        }
        else if (sFormula == "%FLWeekday")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay();
        }
        else if (sFormula == "%FLWday")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay(false);
        }
        else if (sFormula == "%FLMonth")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(true,true);
        }
        else if (sFormula == "%FLMn")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(false,true);
        }
        else if (sFormula == "%FLD")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay(false);
        }
        else if (sFormula == "%FLM")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(false);
        }
        else if (sFormula == "%FLYYYY" )
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLYY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddYear(false);
        }
        //chinese version begin
        else if (sFormula == "%FLYY/M/D")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddYear(false);
            pDateStyle->AddText("-");
            pDateStyle->AddMonth(false);
            pDateStyle->AddText("-");
            pDateStyle->AddMonthDay(false);
        }
        else if(sFormula == "%FLYYYY Month D")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddYear();
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
        }
        else if(sFormula == "%FLWeekday, YYYY Month D")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(",");
            pDateStyle->AddYear();
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
        }
        else if(sFormula == "%FLYYYY Mn D")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddYear();
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(false,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
        }
        else if(sFormula == "%FLWday, YYYY Mn D")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay(false);
            pDateStyle->AddText(",");
            pDateStyle->AddYear();
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
        }
        //next 2 are the same with english version
        else if (sFormula == "%FLYY Mn D")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddYear(false);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(false,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
        }
        else if (sFormula == "%FLYY/M")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(false);
            pDateStyle->AddText("-");
            pDateStyle->AddMonth(false);
        }
        else if (sFormula == "%FLYY Mn")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(false);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(false,true);
        }
        else if (sFormula == "%FLeeeeoa" || sFormula == "%FLffffooaa" || sFormula == "%FLEEEEOA")
        {
            pDateStyle.reset(new XFDateStyle);
            OUString sText;
            pDateStyle->AddYear();
            sText = OUString(u'\x5e74');
            pDateStyle->AddText(sText);
            pDateStyle->AddMonth(false);
            sText = OUString(u'\x6708');
            pDateStyle->AddText(sText);
            pDateStyle->AddMonthDay(false);
            sText = OUString(u'\x65e5');
            pDateStyle->AddText(sText);
        }
        else if (sFormula == "%FLoa" || sFormula == "%FLooaa" || sFormula == "%FLOA" )
        {
            pDateStyle.reset(new XFDateStyle);
            OUString sText;
            pDateStyle->AddMonth(false);
            sText = OUString(u'\x6708');
            pDateStyle->AddText(sText);
            pDateStyle->AddMonthDay(false);
            sText = OUString(u'\x65e5');
            pDateStyle->AddText(sText);
        }
        else if (sFormula == "%FLYYYY/M/D" || sFormula == "%FLGGGG/od/ad")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddYear();
            pDateStyle->AddText("-");
            pDateStyle->AddMonth(false);
            pDateStyle->AddText("-");
            pDateStyle->AddMonthDay(false);
        }
        else if (sFormula == "%FLYY.M.D" )
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddYear(false);
            pDateStyle->AddText(".");
            pDateStyle->AddMonth(false);
            pDateStyle->AddText(".");
            pDateStyle->AddMonthDay(false);
        }
        //chinese version end
        //other version begin
        else if (sFormula == "%FLWeekday, Month DD, YYYY" )
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(", ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(", ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLYYYY/MM/DD" )
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(", ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(", ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLD/M/YY" )
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText("/");
            pDateStyle->AddMonth(false);
            pDateStyle->AddText("/");
            pDateStyle->AddYear(false);
        }
        else if (sFormula == "%FLD Month YYYY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLDD Month YYYY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLWeekday, D Month YYYY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(", ");
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLWeekday, DD Month YYYY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(", ");
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLD Mn YYYY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(false,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLWday, D Mn YYYY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay(false);
            pDateStyle->AddText(", ");
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(false,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLWday, DD Mn YYYY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay(false);
            pDateStyle->AddText(", ");
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(false,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%FLD Mn")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(false,true);
        }
        else if (sFormula == "%FLDD Mn")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(false,true);
        }
        else if (sFormula == "%FLWeekday, D Mn")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(", ");
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(false,true);
        }
        else if (sFormula == "%FLWeekday, DD Mn")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(", ");
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(false,true);
        }
        else if (sFormula == "%FLD Mn YY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(false,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddYear(false);
        }
        else if (sFormula == "%FLDD Mn YY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay();
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(false,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddYear(false);
        }
        else if (sFormula == "%FLD/M")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText("/");
            pDateStyle->AddMonth(false);
        }
        else if (sFormula == "%FLDD/MM")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay();
            pDateStyle->AddText("/");
            pDateStyle->AddMonth();
        }
        else if(sFormula == "%FLDD/MM/YY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay();
            pDateStyle->AddText("/");
            pDateStyle->AddMonth();
            pDateStyle->AddText("/");
            pDateStyle->AddYear(false);
        }
        else if (sFormula == "%FLMM/YY")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth();
            pDateStyle->AddText("/");
            pDateStyle->AddYear(false);
        }
        //other version end
    //TIME
        else if (sFormula == "%FLSystemTime")
        {
            pTimeStyle = LwpTools::GetSystemTimeStyle();
        }
        else if (sFormula == "%FLISOTime1" || sFormula == "%FLH:mm:SS")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour();
            pTimeStyle->AddText(":");
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(":");
            pTimeStyle->AddSecond();
        }
        else if (sFormula == "%FLH:mm ampm")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour(false);
            pTimeStyle->AddText(":");
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(" ");
            pTimeStyle->SetAmPm(true);
        }
        else if (sFormula == "%FLH:mm")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour(false);
            pTimeStyle->AddText(":");
            pTimeStyle->AddMinute();
        }
        else if (sFormula == "%FLH:mm:SS ampm")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour(false);
            pTimeStyle->AddText(":");
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(":");
            pTimeStyle->AddSecond();
            pTimeStyle->AddText(" ");
            pTimeStyle->SetAmPm(true);
        }
        else if (sFormula == "%FLH:mm:SS")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour(false);
            pTimeStyle->AddText(":");
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(":");
            pTimeStyle->AddSecond();
        }
        else if (sFormula == "%FLH ampm")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour(false);
            pTimeStyle->AddText(" ");
            pTimeStyle->SetAmPm(true);
        }
        else if (sFormula == "%FLH")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour(false);
        }
        else if (sFormula == "%FLmm")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddMinute();
        }
        else if (sFormula == "%FLSS")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddSecond();
        }
        else if (sFormula == "%FLampm")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->SetAmPm(true);
        }
        //chinese version begin
        else     if (sFormula == "%FLjF" || sFormula == "%FLJFF"
            || sFormula == "%FLjjF" || sFormula == "%FLJJFF " )
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour(false);
            OUString sText;
            sText = OUString(u'\x70b9');
            pTimeStyle->AddText(sText);
            pTimeStyle->AddMinute(false);
            sText = OUString(u'\x5206');
            pTimeStyle->AddText(sText);
        }
        else if (sFormula == "%FLjjjF" || sFormula == "%FLJJJFF" )
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->SetAmPm(true);
            pTimeStyle->AddHour(false);
            OUString sText;
            sText = OUString(u'\x70b9');
            pTimeStyle->AddText(sText);
            pTimeStyle->AddMinute(false);
            sText = OUString(u'\x5206');
            pTimeStyle->AddText(sText);
        }
        //chinese version end
        //other version begin
        else if (sFormula == "%FLHH:mm ampm")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour();
            pTimeStyle->AddText(":");
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(" ");
            pTimeStyle->SetAmPm(true);
        }
        else if (sFormula == "%FLHH ampm")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour();
            pTimeStyle->AddText(" ");
            pTimeStyle->SetAmPm(true);
        }
        else if (sFormula == "%FLHH:mm:SS ampm")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour();
            pTimeStyle->AddText(":");
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(":");
            pTimeStyle->AddSecond();
            pTimeStyle->AddText(" ");
            pTimeStyle->SetAmPm(true);
        }
        else if (sFormula == "%FLHH:mm")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour();
            pTimeStyle->AddText(":");
            pTimeStyle->AddMinute();
        }
        else if (sFormula == "%FLHH:mm:SS")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour();
            pTimeStyle->AddText(":");
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(":");
            pTimeStyle->AddSecond();
        }
        //other version end
    }
    else if(sFormula[1] == 'D')
    {
        if (sFormula == "%Da")
        {
            pDateStyle = LwpTools::GetSystemDateStyle(false);
        }
        else if (sFormula == "%DB" || sFormula == "%Db")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(", ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%Dc" || sFormula == "%DC")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%Dd" || sFormula == "%DD")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(", ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(", ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%DE" || sFormula == "%De")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
        }
        else if (sFormula == "%Df" || sFormula == "%DF")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddWeekDay();
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
        }
        else if (sFormula == "%Dg")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(false);
            pDateStyle->AddText("/");
            pDateStyle->AddMonthDay(false);
        }
        else if (sFormula == "%Dh")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(false);
            pDateStyle->AddText("/");
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText("/");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%Di" || sFormula == "%DI")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(".");
            pDateStyle->AddMonth(true,true);
        }
        else if (sFormula == "%Dj" || sFormula == "%DJ")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay(false);
            pDateStyle->AddText(".");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%Dk" || sFormula == "%DK")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddYear();
            pDateStyle->AddText(" ");
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(" ");
            pDateStyle->AddMonthDay(false);
        }
        else if (sFormula == "%DL" || sFormula == "%Dl")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonth(true,true);
            pDateStyle->AddText(",");
            pDateStyle->AddYear();
        }
        else if (sFormula == "%Dm")
        {
            pDateStyle.reset(new XFDateStyle);
            pDateStyle->AddMonthDay();
            pDateStyle->AddText("/");
            pDateStyle->AddMonth();
            pDateStyle->AddText("/");
            pDateStyle->AddYear();
        }
    }
    else if (sFormula[1] == 'T')
    {
        if (sFormula == "%T1")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour();
            pTimeStyle->AddText(":");
            pTimeStyle->AddMinute();
        }
        else if (sFormula == "%T2" || sFormula == "%T6"
         || sFormula == "%T4" || sFormula == "%T8" )
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour(false);
            pTimeStyle->AddText(":");
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(":");
            pTimeStyle->SetAmPm(true);
        }
        else if (sFormula == "%T3" || sFormula == "%T7"
         || sFormula == "%T5" || sFormula == "%T9")
        {
            pTimeStyle.reset(new XFTimeStyle);
            pTimeStyle->AddHour();
            pTimeStyle->AddText(":");
            pTimeStyle->AddMinute();
            pTimeStyle->AddText(":");
            pTimeStyle->SetAmPm(true);
        }
    }
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    if (pDateStyle)
        m_TimeStyle = pXFStyleManager->AddStyle(std::move(pDateStyle)).m_pStyle->GetStyleName();
    if (pTimeStyle)
        m_TimeStyle = pXFStyleManager->AddStyle(std::move(pTimeStyle)).m_pStyle->GetStyleName();
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
    XFContent* pContent = nullptr;
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
    }
    if (pContent)
    {
        if (m_ModFlag)//(m_pModifiers)
        {
            XFTextSpanStart* pSpan = new XFTextSpanStart;
            pSpan->SetStyleName(GetStyleName());
            pSpan->Add(pContent);
            pXFPara->Add(pSpan);
            pFieldMark->SetStyleFlag(true);
        }
        else
            pXFPara->Add(pContent);
    }
}

void LwpFribField::ConvertDocFieldEnd(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark)
{
    XFContent* pContent = nullptr;
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
    XFContent* pContent = nullptr;
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
        if (m_ModFlag)
        {
            XFTextSpanStart* pSpan = new XFTextSpanStart;
            pSpan->SetStyleName(GetStyleName());
            pSpan->Add(pContent);
            pXFPara->Add(pSpan);
            pFieldMark->SetStyleFlag(true);
        }
        else
            pXFPara->Add(pContent);
    }

}

void LwpFribField::ConvertDateTimeEnd(XFContentContainer* pXFPara,LwpFieldMark* pFieldMark)
{
    XFContent* pContent = nullptr;
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
//  pFieldMark->SetStart(sal_True);//for some unusual cases
    if (m_ModFlag)
    {
        XFTextSpanStart* pSpan = new XFTextSpanStart;
        pSpan->SetStyleName(GetStyleName());
        pSpan->Add(pRef);
        pXFPara->Add(pSpan);
        pFieldMark->SetStyleFlag(true);
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

LwpFribRubyMarker::LwpFribRubyMarker( LwpPara* pPara )
  : LwpFrib(pPara)
  , m_nType(0)
{
}

void LwpFribRubyMarker::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    m_objMarker.ReadIndexed(pObjStrm);
    m_nType = pObjStrm->QuickReaduInt8();
}

void LwpFribRubyMarker::XFConvert(XFContentContainer* pXFPara)
{
    sal_uInt8 type = GetType();
    LwpRubyMarker* pMarker = dynamic_cast<LwpRubyMarker*>(m_objMarker.obj(VO_RUBYMARKER).get());

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
