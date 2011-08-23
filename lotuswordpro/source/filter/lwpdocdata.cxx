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
#include "lwpdocdata.hxx"
#include "xfilter/xfofficemeta.hxx"
#include "localtime.hxx"
#include	<rtl/ustrbuf.hxx>
#include "lwpfilehdr.hxx"
#include "lwpglobalmgr.hxx"

//std::map<sal_uInt16,LwpEditorAttr*> LwpDocData::m_EditorAttrMap;

LwpDocData::LwpDocData(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    :LwpObject(objHdr, pStrm)//m_pEditorAttrList(0)
{}

LwpDocData::~LwpDocData()
{
}
void LwpDocData::SkipExtra(LwpSvStream* pStream)
{
    sal_uInt16 extra;
    pStream->Read(&extra,2);
    while(extra != 0)
    {
        pStream->SeekRel(extra);
        pStream->Read(&extra,2);
    }
}
void LwpDocData::SkipAtomHandler(LwpSvStream* pStream)
{
    sal_uInt16 disksize;
    sal_uInt16 len;
    pStream->Read(&disksize,2);
    pStream->Read(&len,2);
    if(len)pStream->SeekRel(disksize-2);
}
void LwpDocData::Read()
{
    //XFDateTime
    //////////////////////////////////////////////////////////////////////////
    //doc options
    //cOptionFlag
    m_DocOptions.nOptionFlag = m_pObjStrm->QuickReaduInt16();
    //cEncrypt1Password
    m_DocOptions.encrypt1password.Read(m_pObjStrm);
    //cEncrypt2Password
    m_DocOptions.encrypt2password.Read(m_pObjStrm);
    //cCharecterSet
    m_DocOptions.characterSet.Read(m_pObjStrm);
    //cGrammerSet
    m_DocOptions.grammerSet.Read(m_pObjStrm);
    //cShowMarginMarks
    m_DocOptions.nMarginMarks = m_pObjStrm->QuickReaduInt16();
    //cMarginMarksLocation
    m_DocOptions.nMarginMarksLocation = m_pObjStrm->QuickReaduInt16();
    //cMarginMarksChar
    m_DocOptions.nMarginMarksChar = m_pObjStrm->QuickReaduInt16();
    m_pObjStrm->SkipExtra();
    //////////////////////////////////////////////////////////////////////////
    //doc info
    //cDescription
    m_DocInfo.description.Read(m_pObjStrm);
    //cKeyWord
    m_DocInfo.keywords.Read(m_pObjStrm);
    //cCreatedBy
    m_DocInfo.createdBy.Read(m_pObjStrm);
    //cCreationTime
    m_DocInfo.nCreationTime = m_pObjStrm->QuickReadInt32();
    LtgLocalTime(m_DocInfo.nCreationTime,m_nCreationTime);
    //cLastRevisionTime
    m_DocInfo.nLastRevisionTime = m_pObjStrm->QuickReadInt32();
    LtgLocalTime(m_DocInfo.nLastRevisionTime,m_nLastRevisionTime);
    //cTotalEditTime
    m_DocInfo.nTotalEditTime = m_pObjStrm->QuickReadInt32();

    m_nTotalEditTime.tm_hour = m_DocInfo.nTotalEditTime / 60;
    m_nTotalEditTime.tm_min = m_DocInfo.nTotalEditTime % 60;
    m_nTotalEditTime.tm_sec = 0;
    //cpVerDocInfo
    m_DocInfo.cpVerDocInfo.ReadIndexed(m_pObjStrm);

    //EditorList
    m_DocInfo.nNumEditedBy = m_pObjStrm->QuickReaduInt16();
    LwpAtomHolder* pCDLNList = new LwpAtomHolder[m_DocInfo.nNumEditedBy];
    LwpAtomHolder* pEditorList = new LwpAtomHolder[m_DocInfo.nNumEditedBy];
    sal_uInt16 i = 0;
    for ( i = 0; i < m_DocInfo.nNumEditedBy; i++)
    {
        //CDLNList
        pCDLNList[i].Read(m_pObjStrm);	//cName
        m_pObjStrm->SkipExtra();
        //CEditedBy
        pEditorList[i].Read(m_pObjStrm);	//cName
        m_pObjStrm->SkipExtra();
    }

    m_pObjStrm->SkipExtra();

    delete [] pCDLNList;
    delete [] pEditorList;
    //////////////////////////////////////////////////////////////////////////
    //doc control
    //cGreeting
    m_DocControl.cGreeting.Read(m_pObjStrm);
    //cFlags
    m_DocControl.nFlags = m_pObjStrm->QuickReaduInt16();

    //cDocControlProtection
    m_DocControl.nDocControlProtection = m_pObjStrm->QuickReaduInt16();
    //Len
    m_DocControl.nLen1 = m_pObjStrm->QuickReaduInt16();
    //skip doc control password string
    m_pObjStrm->SeekRel(m_DocControl.nLen1);

    //cFileProtection
    m_DocControl.nFileProtection = m_pObjStrm->QuickReaduInt16();
    //Len
    m_DocControl.nLen2 = m_pObjStrm->QuickReaduInt16();
    //skip file password string
    m_pObjStrm->SeekRel(m_DocControl.nLen2);

    //cAutoVersioning
    m_DocControl.nAutoVersioning = m_pObjStrm->QuickReaduInt16();
    //cDocControlOnlyEditor
    m_DocControl.cDocControlOnlyEditor.Read(m_pObjStrm);
    //cEditorVerification
    m_DocControl.nEditorVerification = m_pObjStrm->QuickReaduInt16();

    m_pObjStrm->SkipExtra();
    //////////////////////////////////////////////////////////////////////////
    //editor list
    //numeditors
    sal_uInt16 numeditors = m_pObjStrm->QuickReaduInt16();
    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();

    for (i = 0;i<numeditors;i++)
    {
        LwpEditorAttr* pEditorAttr = new LwpEditorAttr;
        //cName
        pEditorAttr->cName.Read(m_pObjStrm);
        //cInitials
        pEditorAttr->cInitials.Read(m_pObjStrm);
        //cHiLite
        pEditorAttr->cHiLiteColor.Read(m_pObjStrm);

        //cID
        pEditorAttr->nID = m_pObjStrm->QuickReaduInt16();

        //cInsFontOver
        //CFontAttributeOverride --cAttrBits
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cAttrBits = m_pObjStrm->QuickReaduInt16();
        //CFontAttributeOverride --cAttrOverrideBits
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cAttrOverrideBits = m_pObjStrm->QuickReaduInt16();
        //CFontAttributeOverride --cAttrApplyBits
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cAttrApplyBits = m_pObjStrm->QuickReaduInt16();
        //CFontAttributeOverride --cAttrOverrideBits2
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cAttrOverrideBits2 = m_pObjStrm->QuickReaduInt8();
        //CFontAttributeOverride --cAttrApplyBits2
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cAttrApplyBits2 = m_pObjStrm->QuickReaduInt8();
        //CFontAttributeOverride --cCase
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cCase = m_pObjStrm->QuickReaduInt8();
        //CFontAttributeOverride --cUnder
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cUnder = m_pObjStrm->QuickReaduInt8();
        m_pObjStrm->SkipExtra();
        //CFontDescriptionOverrideBase--cOverrideBits
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cOverrideBits = m_pObjStrm->QuickReaduInt8();
        //CFontDescriptionOverrideBase--cApplyBits
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cApplyBits = m_pObjStrm->QuickReaduInt8();
        //CFontDescriptionOverrideBase--cPointSize
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cPointSize = m_pObjStrm->QuickReadInt32();
        //CFontDescriptionOverrideBase--cOverstrike
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cOverstrike = m_pObjStrm->QuickReaduInt8();
        //CFontDescriptionOverrideBase--cTightness
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cTightness = m_pObjStrm->QuickReaduInt16();
        //CFontDescriptionOverrideBase--cColor
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cColor.Read(m_pObjStrm);

        //CFontDescriptionOverrideBase--cBackgroundColor
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cBackgroundColor.Read(m_pObjStrm);

        m_pObjStrm->SkipExtra();
        //cFaceName
        pEditorAttr->cInsFontOver.cFaceName.Read(m_pObjStrm);
        //cAltFaceName
        pEditorAttr->cInsFontOver.cAltFaceName.Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();

        //cDelFontOver
        //CFontAttributeOverride --cAttrBits
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cAttrBits = m_pObjStrm->QuickReaduInt16();
        //CFontAttributeOverride --cAttrOverrideBits
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cAttrOverrideBits = m_pObjStrm->QuickReaduInt16();
        //CFontAttributeOverride --cAttrApplyBits
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cAttrApplyBits = m_pObjStrm->QuickReaduInt16();
        //CFontAttributeOverride --cAttrOverrideBits2
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cAttrOverrideBits2 = m_pObjStrm->QuickReaduInt8();
        //CFontAttributeOverride --cAttrApplyBits2
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cAttrApplyBits2 = m_pObjStrm->QuickReaduInt8();
        //CFontAttributeOverride --cCase
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cCase = m_pObjStrm->QuickReaduInt8();
        //CFontAttributeOverride --cUnder
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cUnder = m_pObjStrm->QuickReaduInt8();
        m_pObjStrm->SkipExtra();
        //CFontDescriptionOverrideBase--cOverrideBits
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cOverrideBits = m_pObjStrm->QuickReaduInt8();
        //CFontDescriptionOverrideBase--cApplyBits
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cApplyBits = m_pObjStrm->QuickReaduInt8();
        //CFontDescriptionOverrideBase--cPointSize
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cPointSize = m_pObjStrm->QuickReadInt32();
        //CFontDescriptionOverrideBase--cOverstrike
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cOverstrike = m_pObjStrm->QuickReaduInt8();
        //CFontDescriptionOverrideBase--cTightness
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cTightness = m_pObjStrm->QuickReaduInt16();
        //CFontDescriptionOverrideBase--cColor
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cColor.Read(m_pObjStrm);

        //CFontDescriptionOverrideBase--cBackgroundColor
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cBackgroundColor.Read(m_pObjStrm);

        m_pObjStrm->SkipExtra();
        //cFaceName
        pEditorAttr->cDelFontOver.cFaceName.Read(m_pObjStrm);
        //cAltFaceName
        pEditorAttr->cDelFontOver.cAltFaceName.Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();

        //cAbilities
        pEditorAttr->nAbilities = m_pObjStrm->QuickReaduInt16();
        //cLocks
        pEditorAttr->nLocks = m_pObjStrm->QuickReaduInt16();
        //cSuggestions
        pEditorAttr->nSuggestions = m_pObjStrm->QuickReaduInt16();

        //cDelTextAttr
        //m_pEditorAttrList[i].cDelTextAttr.Read(m_pObjStrm);
        if (m_pObjStrm->QuickReadBool())
        {
            pEditorAttr->cDelTextAttr.cOverride.cValues = m_pObjStrm->QuickReaduInt16();
            pEditorAttr->cDelTextAttr.cOverride.cOverride = m_pObjStrm->QuickReaduInt16();
            pEditorAttr->cDelTextAttr.cOverride.cApply = m_pObjStrm->QuickReaduInt16();
            m_pObjStrm->SkipExtra();
            pEditorAttr->cDelTextAttr.cHideLevels = m_pObjStrm->QuickReaduInt16();

            if (LwpFileHeader::m_nFileRevision>= 0x000b)
            {
                pEditorAttr->cDelTextAttr.cBaselineOffset = m_pObjStrm->QuickReadInt32();
            }
            else
            {
                pEditorAttr->cDelTextAttr.cBaselineOffset = 0;
            }
        }
        m_pObjStrm->SkipExtra();

        m_pObjStrm->SkipExtra();

        pGlobal->SetEditorAttrMap(pEditorAttr->nID, pEditorAttr);
    }
}
rtl::OUString	LwpDocData::DateTimeToOUString(LtTm& dt)
{
    rtl::OUStringBuffer	buf;
    buf.append((sal_Int32)dt.tm_year);
    buf.append( A2OUSTR("-") );
    buf.append((sal_Int32)dt.tm_mon);
    buf.append( A2OUSTR("-") );
    buf.append((sal_Int32)dt.tm_mday);
    buf.append( A2OUSTR("T") );
    buf.append((sal_Int32)dt.tm_hour);
    buf.append( A2OUSTR(":") );
    buf.append((sal_Int32)dt.tm_min);
    buf.append( A2OUSTR(":") );
    buf.append((sal_Int32)dt.tm_sec);
    buf.append( A2OUSTR(".") );
    buf.append((sal_Int32)0);

    return buf.makeStringAndClear();
}
rtl::OUString	LwpDocData::TimeToOUString(LtTm& dt)
{
    //PT3H43M44S
    rtl::OUStringBuffer	buf;
    buf.append( A2OUSTR("PT") );
    buf.append((sal_Int32)dt.tm_hour);
    buf.append( A2OUSTR("H") );
    buf.append((sal_Int32)dt.tm_min);
    buf.append( A2OUSTR("M") );
    buf.append((sal_Int32)dt.tm_sec);
    buf.append( A2OUSTR("S") );

    return buf.makeStringAndClear();
}
void LwpDocData::Parse(IXFStream *pOutputStream)
{
    //<!DOCTYPE office:document-meta PUBLIC "-//OpenOffice.org//DTD OfficeDocument 1.0//EN" "office.dtd">
    //<office:document-meta xmlns:office="http://openoffice.org/2000/office" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:meta="http://openoffice.org/2000/meta" office:version="1.0">
    //</office:document-meta>
    XFOfficeMeta xfMeta;
    xfMeta.SetCreator(m_DocInfo.createdBy.str());
    xfMeta.SetDescription(m_DocInfo.description.str());
    xfMeta.SetKeywords(m_DocInfo.keywords.str());
    xfMeta.SetCreationTime(DateTimeToOUString(m_nCreationTime));
    xfMeta.SetLastTime(DateTimeToOUString(m_nLastRevisionTime));
    xfMeta.SetEditTime(TimeToOUString(m_nTotalEditTime));
    xfMeta.ToXml(pOutputStream);
}
/*
OUString LwpDocData::GetEditorName(sal_uInt8 nID)
{
    std::map<sal_uInt16,LwpEditorAttr*>::iterator iter;
    iter = m_EditorAttrMap.find(nID);
    if (iter != m_EditorAttrMap.end())
        return iter->second->cName.str();
    return A2OUSTR("");
}

XFColor LwpDocData::GetHighLightColor(sal_uInt8 nID)
{
    std::map<sal_uInt16,LwpEditorAttr*>::iterator iter;

    iter = m_EditorAttrMap.find(nID);
    if (iter != m_EditorAttrMap.end())
    {
        LwpColor aLwpColor = iter->second->cHiLiteColor;
        XFColor aColor(aLwpColor.GetRed(),aLwpColor.GetGreen(),aLwpColor.GetBlue());
        return aColor;
    }
    else
    {
        XFColor aColor(255,255,0);//yellow
        return aColor;
    }
}
void LwpDocData::Reset()
{
    std::map<sal_uInt16,LwpEditorAttr*>::iterator iter;
    for (iter =m_EditorAttrMap.begin();iter != m_EditorAttrMap.end(); iter++)
    {
        delete iter->second;
        iter->second = NULL;
    }
    m_EditorAttrMap.clear();
}*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
