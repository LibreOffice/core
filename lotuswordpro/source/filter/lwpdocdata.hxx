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
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPDOCDATA_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPDOCDATA_HXX
#include "lwpobj.hxx"
#include "lwpatomholder.hxx"
#include "localtime.hxx"
#include "lwpcolor.hxx"
#include "xfilter/xfcolor.hxx"
#include <map>

struct LwpDocOptions
{
    sal_uInt16 nOptionFlag;
    LwpAtomHolder encrypt1password;
    LwpAtomHolder encrypt2password;
    LwpAtomHolder characterSet;
    LwpAtomHolder grammerSet;
    sal_uInt16 nMarginMarks;
    sal_uInt16 nMarginMarksLocation;
    sal_uInt16 nMarginMarksChar;
};
struct LwpDocInfo
{
    LwpAtomHolder description;
    LwpAtomHolder keywords;
    LwpAtomHolder createdBy;
    sal_Int32 nCreationTime;
    sal_Int32 nLastRevisionTime;
    sal_Int32 nTotalEditTime;
    LwpObjectID cpVerDocInfo;
    sal_uInt16 nNumEditedBy;
    //skip editor list
};
struct LwpDocControl
{
    LwpAtomHolder cGreeting;
    sal_uInt16 nFlags;
    sal_uInt16 nDocControlProtection;
    sal_uInt16 nLen1;
    //skip doc control password string
    sal_uInt16 nFileProtection;
    sal_uInt16 nLen2;
    //skip file password string
    sal_uInt16 nAutoVersioning;
    LwpAtomHolder cDocControlOnlyEditor;
    sal_uInt16 nEditorVerification;
};
struct LwpFontDescriptionOverrideBase
{
    sal_uInt8 cOverrideBits;
    sal_uInt8 cApplyBits;
    sal_Int32 cPointSize;
    sal_uInt8 cOverstrike;
    sal_uInt16 cTightness;
    //lcolor cColor;
    //lcolor cBackgroundColor;
    LwpColor cColor;
    LwpColor cBackgroundColor;
};
struct LwpFontAttributeOverride
{
    sal_uInt16 cAttrBits;
    sal_uInt16 cAttrOverrideBits;
    sal_uInt16 cAttrApplyBits;
    sal_uInt8 cAttrOverrideBits2;
    sal_uInt8 cAttrApplyBits2;
    sal_uInt8 cCase;
    sal_uInt8 cUnder;
};
struct LwpFontDescriptionOverride
{
    LwpFontAttributeOverride cFontAttributeOverride;
    LwpFontDescriptionOverrideBase cFontDescriptionOverrideBase;
    LwpAtomHolder cFaceName;
    LwpAtomHolder cAltFaceName;
};
struct LwpOverrideOg
{
    sal_uInt16 cValues;
    sal_uInt16 cOverride;
    sal_uInt16 cApply;
};
struct LwpTextAttributeOverrideOg
{
    LwpOverrideOg cOverride;
    sal_uInt16 cHideLevels;
    sal_Int32 cBaselineOffset;
};
struct LwpEditorAttr
{
    LwpAtomHolder cName;
    LwpAtomHolder cInitials;
    LwpColor cHiLiteColor;
    sal_uInt16 nID;
    LwpFontDescriptionOverride cInsFontOver;
    LwpFontDescriptionOverride cDelFontOver;
    sal_uInt16 nAbilities;
    sal_uInt16 nLocks;
    sal_uInt16 nSuggestions;
    LwpTextAttributeOverrideOg cDelTextAttr;
};
class LwpDocData : public LwpObject
{
public:
    LwpDocData(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
private:
    virtual ~LwpDocData();

    LwpDocOptions m_DocOptions;
    LwpDocInfo m_DocInfo;
    LwpDocControl m_DocControl;
//  static std::map<sal_uInt16,LwpEditorAttr*> m_EditorAttrMap;
private:
    LtTm m_nCreationTime;
    LtTm m_nLastRevisionTime;
    LtTm m_nTotalEditTime;
private:
    static OUString DateTimeToOUString(LtTm& dt);
    static OUString TimeToOUString(LtTm& dt);

public:
    void Read() override;
    void Parse(IXFStream *pOutputStream) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
