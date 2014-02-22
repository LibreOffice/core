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
 *  License at http:
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
#include <rtl/ustrbuf.hxx>
#include "lwpfilehdr.hxx"
#include "lwpglobalmgr.hxx"

LwpDocData::LwpDocData(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    :LwpObject(objHdr, pStrm)
{}

LwpDocData::~LwpDocData()
{
}

void LwpDocData::Read()
{
    
    
    
    
    m_DocOptions.nOptionFlag = m_pObjStrm->QuickReaduInt16();
    
    m_DocOptions.encrypt1password.Read(m_pObjStrm);
    
    m_DocOptions.encrypt2password.Read(m_pObjStrm);
    
    m_DocOptions.characterSet.Read(m_pObjStrm);
    
    m_DocOptions.grammerSet.Read(m_pObjStrm);
    
    m_DocOptions.nMarginMarks = m_pObjStrm->QuickReaduInt16();
    
    m_DocOptions.nMarginMarksLocation = m_pObjStrm->QuickReaduInt16();
    
    m_DocOptions.nMarginMarksChar = m_pObjStrm->QuickReaduInt16();
    m_pObjStrm->SkipExtra();
    
    
    
    m_DocInfo.description.Read(m_pObjStrm);
    
    m_DocInfo.keywords.Read(m_pObjStrm);
    
    m_DocInfo.createdBy.Read(m_pObjStrm);
    
    m_DocInfo.nCreationTime = m_pObjStrm->QuickReadInt32();
    LtgLocalTime(m_DocInfo.nCreationTime,m_nCreationTime);
    
    m_DocInfo.nLastRevisionTime = m_pObjStrm->QuickReadInt32();
    LtgLocalTime(m_DocInfo.nLastRevisionTime,m_nLastRevisionTime);
    
    m_DocInfo.nTotalEditTime = m_pObjStrm->QuickReadInt32();

    m_nTotalEditTime.tm_hour = m_DocInfo.nTotalEditTime / 60;
    m_nTotalEditTime.tm_min = m_DocInfo.nTotalEditTime % 60;
    m_nTotalEditTime.tm_sec = 0;
    
    m_DocInfo.cpVerDocInfo.ReadIndexed(m_pObjStrm);

    
    m_DocInfo.nNumEditedBy = m_pObjStrm->QuickReaduInt16();
    LwpAtomHolder* pCDLNList = new LwpAtomHolder[m_DocInfo.nNumEditedBy];
    LwpAtomHolder* pEditorList = new LwpAtomHolder[m_DocInfo.nNumEditedBy];
    sal_uInt16 i = 0;
    for ( i = 0; i < m_DocInfo.nNumEditedBy; i++)
    {
        
        pCDLNList[i].Read(m_pObjStrm);  
        m_pObjStrm->SkipExtra();
        
        pEditorList[i].Read(m_pObjStrm);    
        m_pObjStrm->SkipExtra();
    }

    m_pObjStrm->SkipExtra();

    delete [] pCDLNList;
    delete [] pEditorList;
    
    
    
    m_DocControl.cGreeting.Read(m_pObjStrm);
    
    m_DocControl.nFlags = m_pObjStrm->QuickReaduInt16();

    
    m_DocControl.nDocControlProtection = m_pObjStrm->QuickReaduInt16();
    
    m_DocControl.nLen1 = m_pObjStrm->QuickReaduInt16();
    
    m_pObjStrm->SeekRel(m_DocControl.nLen1);

    
    m_DocControl.nFileProtection = m_pObjStrm->QuickReaduInt16();
    
    m_DocControl.nLen2 = m_pObjStrm->QuickReaduInt16();
    
    m_pObjStrm->SeekRel(m_DocControl.nLen2);

    
    m_DocControl.nAutoVersioning = m_pObjStrm->QuickReaduInt16();
    
    m_DocControl.cDocControlOnlyEditor.Read(m_pObjStrm);
    
    m_DocControl.nEditorVerification = m_pObjStrm->QuickReaduInt16();

    m_pObjStrm->SkipExtra();
    
    
    
    sal_uInt16 numeditors = m_pObjStrm->QuickReaduInt16();
    LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();

    for (i = 0;i<numeditors;i++)
    {
        LwpEditorAttr* pEditorAttr = new LwpEditorAttr;
        
        pEditorAttr->cName.Read(m_pObjStrm);
        
        pEditorAttr->cInitials.Read(m_pObjStrm);
        
        pEditorAttr->cHiLiteColor.Read(m_pObjStrm);

        
        pEditorAttr->nID = m_pObjStrm->QuickReaduInt16();

        
        
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cAttrBits = m_pObjStrm->QuickReaduInt16();
        
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cAttrOverrideBits = m_pObjStrm->QuickReaduInt16();
        
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cAttrApplyBits = m_pObjStrm->QuickReaduInt16();
        
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cAttrOverrideBits2 = m_pObjStrm->QuickReaduInt8();
        
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cAttrApplyBits2 = m_pObjStrm->QuickReaduInt8();
        
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cCase = m_pObjStrm->QuickReaduInt8();
        
        pEditorAttr->cInsFontOver.cFontAttributeOverride.cUnder = m_pObjStrm->QuickReaduInt8();
        m_pObjStrm->SkipExtra();
        
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cOverrideBits = m_pObjStrm->QuickReaduInt8();
        
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cApplyBits = m_pObjStrm->QuickReaduInt8();
        
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cPointSize = m_pObjStrm->QuickReadInt32();
        
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cOverstrike = m_pObjStrm->QuickReaduInt8();
        
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cTightness = m_pObjStrm->QuickReaduInt16();
        
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cColor.Read(m_pObjStrm);

        
        pEditorAttr->cInsFontOver.cFontDescriptionOverrideBase.cBackgroundColor.Read(m_pObjStrm);

        m_pObjStrm->SkipExtra();
        
        pEditorAttr->cInsFontOver.cFaceName.Read(m_pObjStrm);
        
        pEditorAttr->cInsFontOver.cAltFaceName.Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();

        
        
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cAttrBits = m_pObjStrm->QuickReaduInt16();
        
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cAttrOverrideBits = m_pObjStrm->QuickReaduInt16();
        
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cAttrApplyBits = m_pObjStrm->QuickReaduInt16();
        
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cAttrOverrideBits2 = m_pObjStrm->QuickReaduInt8();
        
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cAttrApplyBits2 = m_pObjStrm->QuickReaduInt8();
        
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cCase = m_pObjStrm->QuickReaduInt8();
        
        pEditorAttr->cDelFontOver.cFontAttributeOverride.cUnder = m_pObjStrm->QuickReaduInt8();
        m_pObjStrm->SkipExtra();
        
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cOverrideBits = m_pObjStrm->QuickReaduInt8();
        
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cApplyBits = m_pObjStrm->QuickReaduInt8();
        
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cPointSize = m_pObjStrm->QuickReadInt32();
        
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cOverstrike = m_pObjStrm->QuickReaduInt8();
        
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cTightness = m_pObjStrm->QuickReaduInt16();
        
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cColor.Read(m_pObjStrm);

        
        pEditorAttr->cDelFontOver.cFontDescriptionOverrideBase.cBackgroundColor.Read(m_pObjStrm);

        m_pObjStrm->SkipExtra();
        
        pEditorAttr->cDelFontOver.cFaceName.Read(m_pObjStrm);
        
        pEditorAttr->cDelFontOver.cAltFaceName.Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();

        
        pEditorAttr->nAbilities = m_pObjStrm->QuickReaduInt16();
        
        pEditorAttr->nLocks = m_pObjStrm->QuickReaduInt16();
        
        pEditorAttr->nSuggestions = m_pObjStrm->QuickReaduInt16();

        
        
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
OUString   LwpDocData::DateTimeToOUString(LtTm& dt)
{
    OUStringBuffer buf;
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
OUString   LwpDocData::TimeToOUString(LtTm& dt)
{
    
    OUStringBuffer buf;
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
    
    
    
    XFOfficeMeta xfMeta;
    xfMeta.SetCreator(m_DocInfo.createdBy.str());
    xfMeta.SetDescription(m_DocInfo.description.str());
    xfMeta.SetKeywords(m_DocInfo.keywords.str());
    xfMeta.SetCreationTime(DateTimeToOUString(m_nCreationTime));
    xfMeta.SetLastTime(DateTimeToOUString(m_nLastRevisionTime));
    xfMeta.SetEditTime(TimeToOUString(m_nTotalEditTime));
    xfMeta.ToXml(pOutputStream);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
