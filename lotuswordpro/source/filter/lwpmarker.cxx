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

#include "lwpfoundry.hxx"
#include "lwpfilehdr.hxx"
#include "lwpstory.hxx"
#include "lwpmarker.hxx"
#include "lwpproplist.hxx"
#include "lwpglobalmgr.hxx"
#include "xfilter/xfplaceholder.hxx"
#include "xfilter/xfinputlist.hxx"

LwpMarker::LwpMarker(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    : LwpDLNFPVList(objHdr,pStrm)
    , m_nFlag(0)
    , m_nPageNumber(0)
    , m_nNeedUpdate(0)
{
}

void LwpMarker::Read()
{
    LwpDLNFPVList::Read();
    m_objContent.ReadIndexed(m_pObjStrm.get());
    m_objLayout.ReadIndexed(m_pObjStrm.get());
    m_objMarkerList.ReadIndexed(m_pObjStrm.get());
    m_nNeedUpdate = m_pObjStrm->QuickReaduInt16();
    m_nFlag = m_pObjStrm->QuickReaduInt16();
    m_nPageNumber = m_pObjStrm->QuickReaduInt16();

    m_pObjStrm->SkipExtra();
}

OUString LwpMarker::GetNamedProperty(const OUString& name)
{
    LwpPropList* pProp = GetPropList();
    if (pProp)
        return pProp->GetNamedProperty(name);
    else
        return OUString();
}

LwpStoryMarker::LwpStoryMarker(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    : LwpMarker(objHdr,pStrm)
    , m_nFlag(0)
{
}

void LwpStoryMarker::Read()
{
    LwpMarker::Read();
    m_nFlag = m_pObjStrm->QuickReaduInt16();
    m_Range.Read(m_pObjStrm.get());
    m_pObjStrm->SkipExtra();
}

void LwpFribRange::Read(LwpObjectStream* pObjStrm)
{
    m_StartPara.ReadIndexed(pObjStrm);
    m_EndPara.ReadIndexed(pObjStrm);
}

LwpCHBlkMarker::LwpCHBlkMarker(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    : LwpStoryMarker(objHdr, pStrm)
    , m_nTab(0)
    , m_nFlag(0)
    , m_nAction(0)
{
}

void LwpCHBlkMarker::Read()
{
    LwpStoryMarker::Read();
    m_objPromptStory.ReadIndexed(m_pObjStrm.get());
    m_Help.Read(m_pObjStrm.get());
    m_nAction = m_pObjStrm->QuickReaduInt16();
    m_nTab = m_pObjStrm->QuickReaduInt32();
    m_nFlag = m_pObjStrm->QuickReaduInt16();
    if(m_pObjStrm->CheckExtra())
    {
        m_Mirror.Read(m_pObjStrm.get());
        m_pObjStrm->SkipExtra();
    }
}

OUString LwpCHBlkMarker::GetPromptText()
{
    LwpStory* pStory = nullptr;
    if (m_objPromptStory.obj().is())
        pStory = dynamic_cast<LwpStory*>(m_objPromptStory.obj().get());
    if (pStory)
        return pStory->GetContentText();
    return OUString();
}

void LwpCHBlkMarker::ConvertCHBlock(XFContentContainer* pXFPara, sal_uInt8 nType)
{
    sal_uInt16 nAction = GetAction();

    switch(nAction)
    {
    case CLICKHERE_CHBEHAVIORTEXT:
    case CLICKHERE_CHBEHAVIORTABLE:
    case CLICKHERE_CHBEHAVIORPICTURE:
    case CLICKHERE_CHBEHAVIOROLEOBJECT:
        ProcessPlaceHolder(pXFPara,nAction,nType);
        break;
    case CLICKHERE_CHBEHAVIORCHART:
    case CLICKHERE_CHBEHAVIORDRAWING:
    case CLICKHERE_CHBEHAVIORGLOSSARY:
    case CLICKHERE_CHBEHAVIOREQUATION:
    case CLICKHERE_CHBEHAVIORSYMBOL:
    case CLICKHERE_CHBEHAVIORPAGENUM:
    case CLICKHERE_CHBEHAVIORDOCFIELD:
    case CLICKHERE_CHBEHAVIORDATETIME:
        ProcessOtherCHB(pXFPara,nType);
        break;
    case CLICKHERE_CHBEHAVIORSTRINGLIST:
        ProcessKeylist(pXFPara,nType);
        break;
    default:
        break;
    }
}

void LwpCHBlkMarker::ProcessPlaceHolder(XFContentContainer* pXFPara,sal_uInt16 nAction,
                sal_uInt8 nType)
{
    bool bFillFlag = IsHasFilled();
    bool bHelpFlag = IsBubbleHelp();

    if ( bFillFlag )
        return;
    if (nType == MARKER_START)
    {
        XFHolderStart* pHolder= new XFHolderStart;
        switch(nAction)
        {
        case CLICKHERE_CHBEHAVIORTEXT:
            pHolder->SetType("text");
            break;
        case CLICKHERE_CHBEHAVIORTABLE:
            pHolder->SetType("table");
            break;
        case CLICKHERE_CHBEHAVIORPICTURE:
            pHolder->SetType("image");
            break;
        case CLICKHERE_CHBEHAVIOROLEOBJECT:
            pHolder->SetType("object");
            break;
        default:
            break;
        }

        if (bHelpFlag)
            pHolder->SetDesc(m_Help.str());
        pHolder->SetPrompt(GetPromptText());
        pXFPara->Add(pHolder);
    }
    else if (nType == MARKER_END)
    {
        XFHolderEnd* pHolder = new XFHolderEnd;
        pXFPara->Add(pHolder);
    }
}

void LwpCHBlkMarker::ProcessOtherCHB(XFContentContainer* pXFPara,sal_uInt8 nType)
{
    bool bFillFlag = IsHasFilled();
    bool bHelpFlag = IsBubbleHelp();

    if ( bFillFlag )
        return;
    if (nType == MARKER_START)
    {
        XFHolderStart* pHolder= new XFHolderStart;
        pHolder->SetType("text");
        if (bHelpFlag)
            pHolder->SetDesc(m_Help.str());
        pHolder->SetPrompt(GetPromptText());
        pXFPara->Add(pHolder);
    }
    else if (nType == MARKER_END)
    {
        XFHolderEnd* pHolder = new XFHolderEnd;
        pXFPara->Add(pHolder);
    }

}

//note: there will be a blank to mark the list
//all input content of key list processed as normal text
void LwpCHBlkMarker::ProcessKeylist(XFContentContainer* pXFPara,sal_uInt8 nType)
{
    bool bFillFlag = IsHasFilled();

    if ( bFillFlag )
    {
        if (nType == MARKER_START)
        {
            EnumAllKeywords();//traverse the proplist to find all keywords
            XFInputList* pList = new XFInputList;
            pList->SetName(LwpDLNFPVList::m_Name.str());
            pList->SetLabels(m_Keylist);
            pXFPara->Add(pList);
        }
        else if (nType == MARKER_END)//skip
        {
        }
    }
    else
    {
        if (nType == MARKER_START)
        {
            EnumAllKeywords();
            XFInputList* pList = new XFInputList;
            pList->SetName(LwpDLNFPVList::m_Name.str());
            pList->SetLabels(m_Keylist);
            pXFPara->Add(pList);

            XFHolderStart* pHolder= new XFHolderStart;
            pHolder->SetType("text");
            pHolder->SetPrompt(GetPromptText());
            pXFPara->Add(pHolder);
        }
        else if (nType == MARKER_END)
        {
            XFHolderEnd* pHolder = new XFHolderEnd;
            pXFPara->Add(pHolder);
        }
    }
}

bool LwpCHBlkMarker::IsHasFilled()
{
    return (CHB_PROMPT & m_nFlag) == 0;
}

bool LwpCHBlkMarker::IsBubbleHelp()
{
    return (CHB_HELP & m_nFlag) != 0;
}

void LwpCHBlkMarker::EnumAllKeywords()
{
    OUString name1("");
    OUString value1("");
    OUString name2("start");
    LwpPropList* pProp = GetPropList();
    if (!pProp)
        return;
    while(!name2.isEmpty())
    {
        name2 = pProp->EnumNamedProperty(name1,value1);
        if ( name1.match("LIST",0) )
        {
            m_Keylist.push_back(value1);
        }
        name1 = name2;
    }
}

LwpBookMark::LwpBookMark(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    : LwpDLNFVList(objHdr,pStrm)
    , m_nFlag(0)
{
}

void LwpBookMark::Read()
{
    LwpDLNFVList::Read();
    m_objMarker.ReadIndexed(m_pObjStrm.get());
    if (LwpFileHeader::m_nFileRevision < 0x0008)
    {
        if (m_pObjStrm->QuickReadBool())
            m_nFlag |= BKMK_NOTESFX;
    }
    else
        m_nFlag = m_pObjStrm->QuickReaduInt16();
    m_pObjStrm->SkipExtra();
}

bool LwpBookMark::IsRightMarker(LwpObjectID objMarker)
{
    return objMarker == m_objMarker;
}

OUString LwpBookMark::GetName()
{
    return LwpDLNFVList::GetName().str();
}

LwpFieldMark::LwpFieldMark(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    : LwpStoryMarker(objHdr,pStrm)
    , m_nFlag(0)
    , m_nFieldType(0)
    , m_bHasStyle(false)
    , m_bHasStart(false)
    , m_pFrib(nullptr)
    , m_bRevisionFlag(false)
{
}

void LwpFieldMark::Read()
{
    LwpStoryMarker::Read();
    m_Formula.Read(m_pObjStrm.get());
    m_objFormulaStory.ReadIndexed(m_pObjStrm.get());
    if (LwpFileHeader::m_nFileRevision < 0x000B)
        return;
    m_objResultContent.ReadIndexed(m_pObjStrm.get());
    m_nFlag = m_pObjStrm->QuickReaduInt16();
    m_nFieldType = m_pObjStrm->QuickReaduInt16();
    m_pObjStrm->SkipExtra();
}

void LwpFieldMark::ParseIndex(OUString& sKey1,OUString& sKey2)
{
    OUString sFormula = m_Formula.str();
    sal_Int32 index[4];
    sal_Unicode ch(0x0022);//"
    index[0] = sFormula.indexOf(ch);
    index[1] = sFormula.indexOf(ch,index[0]+1);

    index[2] = sFormula.indexOf(ch,index[1]+1);
    index[3] = sFormula.indexOf(ch,index[2]+1);
    if (index[0]>=0 && index[1]>=0)
        sKey1 = sFormula.copy(index[0]+1,index[1]-index[0]-1);
    else
        sKey1.clear();
    if (index[2]>=0 && index[3]>=0)
        sKey2 = sFormula.copy(index[2]+1,index[3]-index[2]-1);
    else
        sKey2.clear();
}

void LwpFieldMark::ParseTOC(OUString& sLevel,OUString& sText)
{
    OUString sFormula = m_Formula.str();
    sal_Int32 index[4];
    sal_Unicode ch1(0x0020);//space
    sal_Unicode ch2(0x0022);//"

    index[0] = sFormula.indexOf(ch1);
    index[1] = sFormula.indexOf(ch1,index[0]+1);

    index[2] = sFormula.indexOf(ch2,index[1]+1);
    index[3] = sFormula.indexOf(ch2,index[2]+1);
    if (index[0]>=0 && index[1]>=0)
        sLevel = sFormula.copy(index[0]+1,index[1]-index[0]-1);
    else
        sLevel.clear();
    if (index[2]>=0 && index[3]>=0)
        sText = sFormula.copy(index[2]+1,index[3]-index[2]-1);
    else
        sText.clear();
}

bool LwpFieldMark::IsFormulaInsert()
{
    return (m_nFlag & FF_FORMULAINSERTED) != 0;
}

bool LwpFieldMark::IsDateTimeField(sal_uInt8& type,OUString& formula)
{
    OUString sFormula = m_Formula.str();
    sal_Int32 index = sFormula.indexOf(0x20); // space
    if (index < 0)
    {
        if (sFormula == "TotalEditingTime")
        {
            type = DATETIME_TOTALTIME;
            return true;
        }
        return false;
    }

    OUString tag = sFormula.copy(0,index);
    if (tag == "Now()")
    {
        type = DATETIME_NOW;
        formula = sFormula.copy(index+1);
        return true;
    }
    else if (tag == "CreateDate")
    {
        type = DATETIME_CREATE;
        formula = sFormula.copy(index+1);
        return true;
    }
    else if (tag == "EditDate")
    {
        type = DATETIME_LASTEDIT;
        formula = sFormula.copy(index+1);
        return true;
    }
    else if (tag == "YesterdaysDate" || tag == "TomorrowsDate"
            || tag == "TodaysDate")
    {
        type = DATETIME_SKIP;
        return true;
    }
    else
        return false;
}

bool LwpFieldMark::IsCrossRefField(sal_uInt8& nType, OUString& sMarkName)
{
    OUString sFormula = m_Formula.str();
    sal_Int32 index = sFormula.indexOf(0x20); // space
    if (index < 0)
    {
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpBookmarkMgr* pMarkMgr = pGlobal->GetLwpBookmarkMgr();
        if (pMarkMgr->FindBookmark(sFormula))
        {
            sMarkName = sFormula;
            nType = CROSSREF_TEXT;
            return true;
        }
        else
            return false;
    }

    OUString tag = sFormula.copy(0,index);
    if (tag == "PageRef")
    {
        sMarkName = sFormula.copy(index+1);
        nType = CROSSREF_PAGE;
        return true;
    }
    else if (tag == "ParaRef")
    {
        sMarkName = sFormula.copy(index+1);
        nType = CROSSREF_PARANUMBER;
        return true;
    }
    else
        return false;
}

bool LwpFieldMark::IsDocPowerField(sal_uInt8& nType,OUString& sFormula)
{
    sFormula = m_Formula.str();

    if (sFormula == "Description")
    {
        nType = DOC_DESCRIPTION;
        return true;
    }
    else if (sFormula == "NumPages")
    {
        nType = DOC_NUMPAGES;
        return true;
    }
    else if (sFormula == "NumChars")
    {
        nType = DOC_NUMCHARS;
        return true;
    }
    else if (sFormula == "NumWords")
    {
        nType = DOC_NUMWORDS;
        return true;
    }
    else
    {
        return false;
    }
}

LwpRubyMarker::LwpRubyMarker(LwpObjectHeader &objHdr, LwpSvStream *pStrm):LwpStoryMarker(objHdr,pStrm)
{
}

void LwpRubyMarker::Read()
{
    LwpStoryMarker::Read();
    m_objLayout.ReadIndexed(m_pObjStrm.get());
    m_pObjStrm->SkipExtra();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
