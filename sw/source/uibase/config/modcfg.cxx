/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <memory>
#include <hintids.hxx>
#include <comphelper/classids.hxx>
#include <o3tl/any.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <svx/svxids.hrc>
#include <editeng/svxenum.hxx>
#include <osl/diagnose.h>

#include <tools/globname.hxx>
#include <swtypes.hxx>
#include <itabenum.hxx>
#include <modcfg.hxx>
#include <fldupde.hxx>
#include <caption.hxx>

#include <unomid.h>

using namespace com::sun::star::uno;

#define GLOB_NAME_CALC      0
#define GLOB_NAME_IMPRESS   1
#define GLOB_NAME_DRAW      2
#define GLOB_NAME_MATH      3
#define GLOB_NAME_CHART     4

InsCaptionOpt* InsCaptionOptArr::Find(const SwCapObjType eType, const SvGlobalName *pOleId)
{
    for (auto const& it : m_InsCapOptArr)
    {
        InsCaptionOpt &rObj = *it;
        if (rObj.GetObjType() == eType && (eType != OLE_CAP || (pOleId && rObj.GetOleId() == *pOleId)))
            return &rObj;
    }

    return nullptr;
}

void InsCaptionOptArr::Insert(InsCaptionOpt* pObj)
{
    m_InsCapOptArr.push_back(std::unique_ptr<InsCaptionOpt>(pObj)); //takes ownership
}

const InsCaptionOpt* SwModuleOptions::GetCapOption(
    bool bHTML, const SwCapObjType eType, const SvGlobalName *pOleId)
{
    if(bHTML)
    {
        OSL_FAIL("no caption option in sw/web!");
        return nullptr;
    }
    else
    {
        if(eType == OLE_CAP && pOleId)
        {
            bool bFound = false;
            for( sal_uInt16 nId = 0; nId <= GLOB_NAME_CHART && !bFound; nId++)
                bFound = *pOleId == aInsertConfig.aGlobalNames[nId  ];
            if(!bFound)
                return aInsertConfig.pOLEMiscOpt.get();
        }
        return aInsertConfig.pCapOptions->Find(eType, pOleId);
    }
}

bool SwModuleOptions::SetCapOption(bool bHTML, const InsCaptionOpt* pOpt)
{
    bool bRet = false;

    if(bHTML)
    {
        OSL_FAIL("no caption option in sw/web!");
    }
    else if (pOpt)
    {
        if(pOpt->GetObjType() == OLE_CAP)
        {
            bool bFound = false;
            for( sal_uInt16 nId = 0; nId <= GLOB_NAME_CHART; nId++)
                bFound = pOpt->GetOleId() == aInsertConfig.aGlobalNames[nId  ];
            if(!bFound)
            {
                if(aInsertConfig.pOLEMiscOpt)
                    *aInsertConfig.pOLEMiscOpt = *pOpt;
                else
                    aInsertConfig.pOLEMiscOpt.reset(new InsCaptionOpt(*pOpt));
            }
        }

        InsCaptionOptArr& rArr = *aInsertConfig.pCapOptions;
        InsCaptionOpt *pObj = rArr.Find(pOpt->GetObjType(), &pOpt->GetOleId());

        if (pObj)
        {
            *pObj = *pOpt;
        }
        else
            rArr.Insert(new InsCaptionOpt(*pOpt));

        aInsertConfig.SetModified();
        bRet = true;
    }

    return bRet;
}

SwModuleOptions::SwModuleOptions() :
    aInsertConfig(false),
    aWebInsertConfig(true),
    aTableConfig(false),
    aWebTableConfig(true),
    bHideFieldTips(false)
{
}

OUString SwModuleOptions::ConvertWordDelimiter(const OUString& rDelim, bool bFromUI)
{
    OUString sReturn;
    const sal_Int32 nDelimLen = rDelim.getLength();
    if(bFromUI)
    {
        for (sal_Int32 i = 0; i < nDelimLen; )
        {
            const sal_Unicode c = rDelim[i++];

            if (c == '\\' && i < nDelimLen )
            {
                switch (rDelim[i++])
                {
                    case 'n':   sReturn += "\n";    break;
                    case 't':   sReturn += "\t";    break;
                    case '\\':  sReturn += "\\";    break;

                    case 'x':
                    {
                        sal_Unicode nChar = 0;
                        bool bValidData = true;
                        for( sal_Int32 n = 0; n < 2 && i < nDelimLen; ++n, ++i )
                        {
                            sal_Unicode nVal = rDelim[i];
                            if( (nVal >= '0') && ( nVal <= '9') )
                                nVal -= '0';
                            else if( (nVal >= 'A') && (nVal <= 'F') )
                                nVal -= 'A' - 10;
                            else if( (nVal >= 'a') && (nVal <= 'f') )
                                nVal -= 'a' - 10;
                            else
                            {
                                OSL_FAIL("wrong hex value" );
                                bValidData = false;
                                break;
                            }

                            nChar <<= 4;
                            nChar += nVal;
                        }
                        if( bValidData )
                            sReturn += OUStringLiteral1(nChar);
                        break;
                    }

                    default:    // Unknown, so insert backslash
                        sReturn += "\\";
                        i--;
                        break;
                }
            }
            else
                sReturn += OUStringLiteral1(c);
        }
    }
    else
    {
        for (sal_Int32 i = 0; i < nDelimLen; ++i)
        {
            const sal_Unicode c = rDelim[i];

            switch (c)
            {
                case '\n':  sReturn += "\\n"; break;
                case '\t':  sReturn += "\\t"; break;
                case '\\':  sReturn += "\\\\"; break;

                default:
                    if( c <= 0x1f || c >= 0x7f )
                    {
                        sReturn += "\\x" + OUString::number( c, 16 );
                    }
                    else
                    {
                        sReturn += OUStringLiteral1(c);
                    }
            }
        }
    }
    return sReturn;
}

const Sequence<OUString>& SwRevisionConfig::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        const int nCount = 8;
        aNames.realloc(nCount);
        static const char* aPropNames[] =
        {
            "TextDisplay/Insert/Attribute",             // 0
            "TextDisplay/Insert/Color",                 // 1
            "TextDisplay/Delete/Attribute",             // 2
            "TextDisplay/Delete/Color",                 // 3
            "TextDisplay/ChangedAttribute/Attribute",   // 4
            "TextDisplay/ChangedAttribute/Color",       // 5
            "LinesChanged/Mark",                        // 6
            "LinesChanged/Color"                        // 7
        };
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

SwRevisionConfig::SwRevisionConfig() :
    ConfigItem("Office.Writer/Revision",
        ConfigItemMode::DelayedUpdate|ConfigItemMode::ReleaseTree)
{
    aInsertAttr.m_nItemId = SID_ATTR_CHAR_UNDERLINE;
    aInsertAttr.m_nAttr = LINESTYLE_SINGLE;
    aInsertAttr.m_nColor = COL_TRANSPARENT;
    aDeletedAttr.m_nItemId = SID_ATTR_CHAR_STRIKEOUT;
    aDeletedAttr.m_nAttr = STRIKEOUT_SINGLE;
    aDeletedAttr.m_nColor = COL_TRANSPARENT;
    aFormatAttr.m_nItemId = SID_ATTR_CHAR_WEIGHT;
    aFormatAttr.m_nAttr = WEIGHT_BOLD;
    aFormatAttr.m_nColor = COL_BLACK;
    Load();
}

SwRevisionConfig::~SwRevisionConfig()
{
}

static sal_Int32 lcl_ConvertAttrToCfg(const AuthorCharAttr& rAttr)
{
    sal_Int32 nRet = 0;
    switch(rAttr.m_nItemId)
    {
        case  SID_ATTR_CHAR_WEIGHT: nRet = 1; break;
        case  SID_ATTR_CHAR_POSTURE: nRet = 2; break;
        case  SID_ATTR_CHAR_UNDERLINE: nRet = LINESTYLE_SINGLE == rAttr.m_nAttr ? 3 : 4; break;
        case  SID_ATTR_CHAR_STRIKEOUT: nRet = 3; break;
        case  SID_ATTR_CHAR_CASEMAP:
        {
            switch(static_cast<SvxCaseMap>(rAttr.m_nAttr))
            {
                case  SvxCaseMap::Uppercase : nRet = 5;break;
                case  SvxCaseMap::Lowercase : nRet = 6;break;
                case  SvxCaseMap::SmallCaps : nRet = 7;break;
                case  SvxCaseMap::Capitalize: nRet = 8;break;
                default: break;
            }
        }
        break;
        case SID_ATTR_BRUSH : nRet = 9; break;
    }
    return nRet;
}

void SwRevisionConfig::Notify( const css::uno::Sequence< OUString >& ) {}

void SwRevisionConfig::ImplCommit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0 : pValues[nProp] <<= lcl_ConvertAttrToCfg(aInsertAttr); break;
            case 1 : pValues[nProp] <<= aInsertAttr.m_nColor;   break;
            case 2 : pValues[nProp] <<= lcl_ConvertAttrToCfg(aDeletedAttr); break;
            case 3 : pValues[nProp] <<= aDeletedAttr.m_nColor;  break;
            case 4 : pValues[nProp] <<= lcl_ConvertAttrToCfg(aFormatAttr); break;
            case 5 : pValues[nProp] <<= aFormatAttr.m_nColor;   break;
            case 6 : pValues[nProp] <<= nMarkAlign;             break;
            case 7 : pValues[nProp] <<= aMarkColor;             break;
        }
    }
    PutProperties(aNames, aValues);
}

static void lcl_ConvertCfgToAttr(sal_Int32 nVal, AuthorCharAttr& rAttr, bool bDelete = false)
{
    rAttr.m_nItemId = rAttr.m_nAttr = 0;
    switch(nVal)
    {
        case 1: rAttr.m_nItemId = SID_ATTR_CHAR_WEIGHT;   rAttr.m_nAttr = WEIGHT_BOLD              ; break;
        case 2: rAttr.m_nItemId = SID_ATTR_CHAR_POSTURE;  rAttr.m_nAttr = ITALIC_NORMAL            ; break;
        case 3: if(bDelete)
                {
                    rAttr.m_nItemId = SID_ATTR_CHAR_STRIKEOUT;
                    rAttr.m_nAttr = STRIKEOUT_SINGLE;
                }
                else
                {
                    rAttr.m_nItemId = SID_ATTR_CHAR_UNDERLINE;
                    rAttr.m_nAttr = LINESTYLE_SINGLE;
                }
        break;
        case 4: rAttr.m_nItemId = SID_ATTR_CHAR_UNDERLINE;rAttr.m_nAttr = LINESTYLE_DOUBLE         ; break;
        case 5: rAttr.m_nItemId = SID_ATTR_CHAR_CASEMAP;  rAttr.m_nAttr = sal_uInt16(SvxCaseMap::Uppercase); break;
        case 6: rAttr.m_nItemId = SID_ATTR_CHAR_CASEMAP;  rAttr.m_nAttr = sal_uInt16(SvxCaseMap::Lowercase); break;
        case 7: rAttr.m_nItemId = SID_ATTR_CHAR_CASEMAP;  rAttr.m_nAttr = sal_uInt16(SvxCaseMap::SmallCaps); break;
        case 8: rAttr.m_nItemId = SID_ATTR_CHAR_CASEMAP;  rAttr.m_nAttr = sal_uInt16(SvxCaseMap::Capitalize); break;
        case 9: rAttr.m_nItemId = SID_ATTR_BRUSH; break;
    }
}
void SwRevisionConfig::Load()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    assert(aValues.getLength() == aNames.getLength());
    for (sal_Int32 nProp = 0; nProp < aNames.getLength(); ++nProp)
    {
        if (pValues[nProp].hasValue())
        {
            sal_Int32 nVal = 0;
            pValues[nProp] >>= nVal;
            switch (nProp)
            {
                case 0 : lcl_ConvertCfgToAttr(nVal, aInsertAttr); break;
                case 1 : aInsertAttr.m_nColor     = Color(nVal); break;
                case 2 : lcl_ConvertCfgToAttr(nVal, aDeletedAttr, true); break;
                case 3 : aDeletedAttr.m_nColor    = Color(nVal); break;
                case 4 : lcl_ConvertCfgToAttr(nVal, aFormatAttr); break;
                case 5 : aFormatAttr.m_nColor     = Color(nVal); break;
                case 6 : nMarkAlign = sal::static_int_cast< sal_uInt16, sal_Int32>(nVal); break;
                case 7 : aMarkColor = Color(nVal); break;
            }
        }
    }
}

enum InsertConfigProp
{
    INS_PROP_TABLE_HEADER = 0,
    INS_PROP_TABLE_REPEATHEADER,                        // 1
    INS_PROP_TABLE_BORDER,                              // 2
    INS_PROP_TABLE_SPLIT,                               // 3 from here not in writer/web
    INS_PROP_CAP_AUTOMATIC,                             // 4
    INS_PROP_CAP_CAPTIONORDERNUMBERINGFIRST,            // 5
    INS_PROP_CAP_OBJECT_TABLE_ENABLE,                   // 6
    INS_PROP_CAP_OBJECT_TABLE_CATEGORY,                 // 7
    INS_PROP_CAP_OBJECT_TABLE_NUMBERING,                // 8
    INS_PROP_CAP_OBJECT_TABLE_NUMBERINGSEPARATOR,       // 9
    INS_PROP_CAP_OBJECT_TABLE_CAPTIONTEXT,              //10
    INS_PROP_CAP_OBJECT_TABLE_DELIMITER,                //11
    INS_PROP_CAP_OBJECT_TABLE_LEVEL,                    //12
    INS_PROP_CAP_OBJECT_TABLE_POSITION,                 //13
    INS_PROP_CAP_OBJECT_TABLE_CHARACTERSTYLE,           //14
    INS_PROP_CAP_OBJECT_FRAME_ENABLE,                   //15
    INS_PROP_CAP_OBJECT_FRAME_CATEGORY,                 //16
    INS_PROP_CAP_OBJECT_FRAME_NUMBERING,                //17
    INS_PROP_CAP_OBJECT_FRAME_NUMBERINGSEPARATOR,       //18
    INS_PROP_CAP_OBJECT_FRAME_CAPTIONTEXT,              //19
    INS_PROP_CAP_OBJECT_FRAME_DELIMITER,                //20
    INS_PROP_CAP_OBJECT_FRAME_LEVEL,                    //21
    INS_PROP_CAP_OBJECT_FRAME_POSITION,                 //22
    INS_PROP_CAP_OBJECT_FRAME_CHARACTERSTYLE,           //23
    INS_PROP_CAP_OBJECT_GRAPHIC_ENABLE,                          //24
    INS_PROP_CAP_OBJECT_GRAPHIC_CATEGORY,               //25
    INS_PROP_CAP_OBJECT_GRAPHIC_NUMBERING,              //26
    INS_PROP_CAP_OBJECT_GRAPHIC_NUMBERINGSEPARATOR,     //27
    INS_PROP_CAP_OBJECT_GRAPHIC_CAPTIONTEXT,            //28
    INS_PROP_CAP_OBJECT_GRAPHIC_DELIMITER,              //29
    INS_PROP_CAP_OBJECT_GRAPHIC_LEVEL,                  //30
    INS_PROP_CAP_OBJECT_GRAPHIC_POSITION,               //31
    INS_PROP_CAP_OBJECT_GRAPHIC_CHARACTERSTYLE,         //32
    INS_PROP_CAP_OBJECT_GRAPHIC_APPLYATTRIBUTES,        //33
    INS_PROP_CAP_OBJECT_CALC_ENABLE,                             //34
    INS_PROP_CAP_OBJECT_CALC_CATEGORY,                  //35
    INS_PROP_CAP_OBJECT_CALC_NUMBERING,                 //36
    INS_PROP_CAP_OBJECT_CALC_NUMBERINGSEPARATOR,        //37
    INS_PROP_CAP_OBJECT_CALC_CAPTIONTEXT,               //38
    INS_PROP_CAP_OBJECT_CALC_DELIMITER,                 //39
    INS_PROP_CAP_OBJECT_CALC_LEVEL,                     //40
    INS_PROP_CAP_OBJECT_CALC_POSITION,                  //41
    INS_PROP_CAP_OBJECT_CALC_CHARACTERSTYLE,            //42
    INS_PROP_CAP_OBJECT_CALC_APPLYATTRIBUTES,           //43
    INS_PROP_CAP_OBJECT_IMPRESS_ENABLE,                          //44
    INS_PROP_CAP_OBJECT_IMPRESS_CATEGORY,               //45
    INS_PROP_CAP_OBJECT_IMPRESS_NUMBERING,              //46
    INS_PROP_CAP_OBJECT_IMPRESS_NUMBERINGSEPARATOR,     //47
    INS_PROP_CAP_OBJECT_IMPRESS_CAPTIONTEXT,            //48
    INS_PROP_CAP_OBJECT_IMPRESS_DELIMITER,              //49
    INS_PROP_CAP_OBJECT_IMPRESS_LEVEL,                  //50
    INS_PROP_CAP_OBJECT_IMPRESS_POSITION,               //51
    INS_PROP_CAP_OBJECT_IMPRESS_CHARACTERSTYLE,         //52
    INS_PROP_CAP_OBJECT_IMPRESS_APPLYATTRIBUTES,        //53
    INS_PROP_CAP_OBJECT_CHART_ENABLE,                            //54
    INS_PROP_CAP_OBJECT_CHART_CATEGORY,                 //55
    INS_PROP_CAP_OBJECT_CHART_NUMBERING,                //56
    INS_PROP_CAP_OBJECT_CHART_NUMBERINGSEPARATOR,       //57
    INS_PROP_CAP_OBJECT_CHART_CAPTIONTEXT,              //58
    INS_PROP_CAP_OBJECT_CHART_DELIMITER,                //59
    INS_PROP_CAP_OBJECT_CHART_LEVEL,                    //60
    INS_PROP_CAP_OBJECT_CHART_POSITION,                 //61
    INS_PROP_CAP_OBJECT_CHART_CHARACTERSTYLE,           //62
    INS_PROP_CAP_OBJECT_CHART_APPLYATTRIBUTES,          //63
    INS_PROP_CAP_OBJECT_FORMULA_ENABLE,                          //64
    INS_PROP_CAP_OBJECT_FORMULA_CATEGORY,               //65
    INS_PROP_CAP_OBJECT_FORMULA_NUMBERING,              //66
    INS_PROP_CAP_OBJECT_FORMULA_NUMBERINGSEPARATOR,     //67
    INS_PROP_CAP_OBJECT_FORMULA_CAPTIONTEXT,            //68
    INS_PROP_CAP_OBJECT_FORMULA_DELIMITER,              //69
    INS_PROP_CAP_OBJECT_FORMULA_LEVEL,                  //70
    INS_PROP_CAP_OBJECT_FORMULA_POSITION,               //71
    INS_PROP_CAP_OBJECT_FORMULA_CHARACTERSTYLE,         //72
    INS_PROP_CAP_OBJECT_FORMULA_APPLYATTRIBUTES,        //73
    INS_PROP_CAP_OBJECT_DRAW_ENABLE,                             //74
    INS_PROP_CAP_OBJECT_DRAW_CATEGORY,                  //75
    INS_PROP_CAP_OBJECT_DRAW_NUMBERING,                 //76
    INS_PROP_CAP_OBJECT_DRAW_NUMBERINGSEPARATOR,        //77
    INS_PROP_CAP_OBJECT_DRAW_CAPTIONTEXT,               //78
    INS_PROP_CAP_OBJECT_DRAW_DELIMITER,                 //79
    INS_PROP_CAP_OBJECT_DRAW_LEVEL,                     //80
    INS_PROP_CAP_OBJECT_DRAW_POSITION,                  //81
    INS_PROP_CAP_OBJECT_DRAW_CHARACTERSTYLE,            //82
    INS_PROP_CAP_OBJECT_DRAW_APPLYATTRIBUTES,           //83
    INS_PROP_CAP_OBJECT_OLEMISC_ENABLE,                          //84
    INS_PROP_CAP_OBJECT_OLEMISC_CATEGORY,               //85
    INS_PROP_CAP_OBJECT_OLEMISC_NUMBERING,              //86
    INS_PROP_CAP_OBJECT_OLEMISC_NUMBERINGSEPARATOR,     //87
    INS_PROP_CAP_OBJECT_OLEMISC_CAPTIONTEXT,            //88
    INS_PROP_CAP_OBJECT_OLEMISC_DELIMITER,              //89
    INS_PROP_CAP_OBJECT_OLEMISC_LEVEL,                  //90
    INS_PROP_CAP_OBJECT_OLEMISC_POSITION,               //91
    INS_PROP_CAP_OBJECT_OLEMISC_CHARACTERSTYLE,         //92
    INS_PROP_CAP_OBJECT_OLEMISC_APPLYATTRIBUTES        //93
};
const Sequence<OUString>& SwInsertConfig::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    static Sequence<OUString> aWebNames;
    if(!aNames.getLength())
    {
        static const char* aPropNames[] =
        {
            "Table/Header",                                                 // 0
            "Table/RepeatHeader",                                           // 1
            "Table/Border",                                                 // 2
            "Table/Split",                                                  // 3 from here not in writer/web
            "Caption/Automatic",                                            // 4
            "Caption/CaptionOrderNumberingFirst",                           // 5
            "Caption/WriterObject/Table/Enable",                            // 6
            "Caption/WriterObject/Table/Settings/Category",                 // 7
            "Caption/WriterObject/Table/Settings/Numbering",                // 8
            "Caption/WriterObject/Table/Settings/NumberingSeparator",       // 9
            "Caption/WriterObject/Table/Settings/CaptionText",              //10
            "Caption/WriterObject/Table/Settings/Delimiter",                //11
            "Caption/WriterObject/Table/Settings/Level",                    //12
            "Caption/WriterObject/Table/Settings/Position",                 //13
            "Caption/WriterObject/Table/Settings/CharacterStyle",           //14
            "Caption/WriterObject/Frame/Enable",                            //15
            "Caption/WriterObject/Frame/Settings/Category",                 //16
            "Caption/WriterObject/Frame/Settings/Numbering",                //17
            "Caption/WriterObject/Frame/Settings/NumberingSeparator",       //18
            "Caption/WriterObject/Frame/Settings/CaptionText",              //19
            "Caption/WriterObject/Frame/Settings/Delimiter",                //20
            "Caption/WriterObject/Frame/Settings/Level",                    //21
            "Caption/WriterObject/Frame/Settings/Position",                 //22
            "Caption/WriterObject/Frame/Settings/CharacterStyle",           //23
            "Caption/WriterObject/Graphic/Enable",                          //24
            "Caption/WriterObject/Graphic/Settings/Category",               //25
            "Caption/WriterObject/Graphic/Settings/Numbering",              //26
            "Caption/WriterObject/Graphic/Settings/NumberingSeparator",     //27
            "Caption/WriterObject/Graphic/Settings/CaptionText",            //28
            "Caption/WriterObject/Graphic/Settings/Delimiter",              //29
            "Caption/WriterObject/Graphic/Settings/Level",                  //30
            "Caption/WriterObject/Graphic/Settings/Position",               //31
            "Caption/WriterObject/Graphic/Settings/CharacterStyle",         //32
            "Caption/WriterObject/Graphic/Settings/ApplyAttributes",        //33
            "Caption/OfficeObject/Calc/Enable",                             //34
            "Caption/OfficeObject/Calc/Settings/Category",                  //35
            "Caption/OfficeObject/Calc/Settings/Numbering",                 //36
            "Caption/OfficeObject/Calc/Settings/NumberingSeparator",        //37
            "Caption/OfficeObject/Calc/Settings/CaptionText",               //38
            "Caption/OfficeObject/Calc/Settings/Delimiter",                 //39
            "Caption/OfficeObject/Calc/Settings/Level",                     //40
            "Caption/OfficeObject/Calc/Settings/Position",                  //41
            "Caption/OfficeObject/Calc/Settings/CharacterStyle",            //42
            "Caption/OfficeObject/Calc/Settings/ApplyAttributes",           //43
            "Caption/OfficeObject/Impress/Enable",                          //44
            "Caption/OfficeObject/Impress/Settings/Category",               //45
            "Caption/OfficeObject/Impress/Settings/Numbering",              //46
            "Caption/OfficeObject/Impress/Settings/NumberingSeparator",     //47
            "Caption/OfficeObject/Impress/Settings/CaptionText",            //48
            "Caption/OfficeObject/Impress/Settings/Delimiter",              //49
            "Caption/OfficeObject/Impress/Settings/Level",                  //50
            "Caption/OfficeObject/Impress/Settings/Position",               //51
            "Caption/OfficeObject/Impress/Settings/CharacterStyle",         //52
            "Caption/OfficeObject/Impress/Settings/ApplyAttributes",        //53
            "Caption/OfficeObject/Chart/Enable",                            //54
            "Caption/OfficeObject/Chart/Settings/Category",                 //55
            "Caption/OfficeObject/Chart/Settings/Numbering",                //56
            "Caption/OfficeObject/Chart/Settings/NumberingSeparator",       //57
            "Caption/OfficeObject/Chart/Settings/CaptionText",              //58
            "Caption/OfficeObject/Chart/Settings/Delimiter",                //59
            "Caption/OfficeObject/Chart/Settings/Level",                    //60
            "Caption/OfficeObject/Chart/Settings/Position",                 //61
            "Caption/OfficeObject/Chart/Settings/CharacterStyle",           //62
            "Caption/OfficeObject/Chart/Settings/ApplyAttributes",          //63
            "Caption/OfficeObject/Formula/Enable",                          //64
            "Caption/OfficeObject/Formula/Settings/Category",               //65
            "Caption/OfficeObject/Formula/Settings/Numbering",              //66
            "Caption/OfficeObject/Formula/Settings/NumberingSeparator",     //67
            "Caption/OfficeObject/Formula/Settings/CaptionText",            //68
            "Caption/OfficeObject/Formula/Settings/Delimiter",              //69
            "Caption/OfficeObject/Formula/Settings/Level",                  //70
            "Caption/OfficeObject/Formula/Settings/Position",               //71
            "Caption/OfficeObject/Formula/Settings/CharacterStyle",         //72
            "Caption/OfficeObject/Formula/Settings/ApplyAttributes",        //73
            "Caption/OfficeObject/Draw/Enable",                             //74
            "Caption/OfficeObject/Draw/Settings/Category",                  //75
            "Caption/OfficeObject/Draw/Settings/Numbering",                 //76
            "Caption/OfficeObject/Draw/Settings/NumberingSeparator",        //77
            "Caption/OfficeObject/Draw/Settings/CaptionText",               //78
            "Caption/OfficeObject/Draw/Settings/Delimiter",                 //79
            "Caption/OfficeObject/Draw/Settings/Level",                     //80
            "Caption/OfficeObject/Draw/Settings/Position",                  //81
            "Caption/OfficeObject/Draw/Settings/CharacterStyle",            //82
            "Caption/OfficeObject/Draw/Settings/ApplyAttributes",           //83
            "Caption/OfficeObject/OLEMisc/Enable",                          //84
            "Caption/OfficeObject/OLEMisc/Settings/Category",               //85
            "Caption/OfficeObject/OLEMisc/Settings/Numbering",              //86
            "Caption/OfficeObject/OLEMisc/Settings/NumberingSeparator",     //87
            "Caption/OfficeObject/OLEMisc/Settings/CaptionText",            //88
            "Caption/OfficeObject/OLEMisc/Settings/Delimiter",              //89
            "Caption/OfficeObject/OLEMisc/Settings/Level",                  //90
            "Caption/OfficeObject/OLEMisc/Settings/Position",               //91
            "Caption/OfficeObject/OLEMisc/Settings/CharacterStyle",         //92
            "Caption/OfficeObject/OLEMisc/Settings/ApplyAttributes"         //93
        };
        const int nCount = INS_PROP_CAP_OBJECT_OLEMISC_APPLYATTRIBUTES + 1;
        const int nWebCount = INS_PROP_TABLE_BORDER + 1;
        aNames.realloc(nCount);
        aWebNames.realloc(nWebCount);
        OUString* pNames = aNames.getArray();
        OUString* pWebNames = aWebNames.getArray();
        int i;
        for(i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
        for(i = 0; i < nWebCount; i++)
            pWebNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return bIsWeb ? aWebNames : aNames;
}

SwInsertConfig::SwInsertConfig(bool bWeb) :
    ConfigItem(bWeb ? OUString("Office.WriterWeb/Insert") : OUString("Office.Writer/Insert"),
        ConfigItemMode::DelayedUpdate|ConfigItemMode::ReleaseTree),
    pCapOptions(nullptr),
    pOLEMiscOpt(nullptr),
    bInsWithCaption( false ),
    bCaptionOrderNumberingFirst( false ),
    aInsTableOpts(SwInsertTableFlags::NONE,0),
    bIsWeb(bWeb)
{
    aGlobalNames[GLOB_NAME_CALC   ] = SvGlobalName(SO3_SC_CLASSID);
    aGlobalNames[GLOB_NAME_IMPRESS] = SvGlobalName(SO3_SIMPRESS_CLASSID);
    aGlobalNames[GLOB_NAME_DRAW   ] = SvGlobalName(SO3_SDRAW_CLASSID);
    aGlobalNames[GLOB_NAME_MATH   ] = SvGlobalName(SO3_SM_CLASSID);
    aGlobalNames[GLOB_NAME_CHART  ] = SvGlobalName(SO3_SCH_CLASSID);
    if(!bIsWeb)
        pCapOptions.reset(new InsCaptionOptArr);

    Load();
}

SwInsertConfig::~SwInsertConfig()
{
    pCapOptions.reset();
    pOLEMiscOpt.reset();
}

static void lcl_WriteOpt(const InsCaptionOpt& rOpt, Any* pValues, sal_Int32 nProp, sal_Int32 nOffset)
{
    switch(nOffset)
    {
        case 0: pValues[nProp] <<= rOpt.UseCaption(); break;//Enable
        case 1: pValues[nProp] <<= rOpt.GetCategory(); break;//Category
        case 2: pValues[nProp] <<= static_cast<sal_Int32>(rOpt.GetNumType()); break;//Numbering",
        case 3: pValues[nProp] <<= rOpt.GetNumSeparator(); break;//NumberingSeparator",
        case 4: pValues[nProp] <<= rOpt.GetCaption();  break;//CaptionText",
        case 5: pValues[nProp] <<= rOpt.GetSeparator();break;//Delimiter",
        case 6: pValues[nProp] <<= static_cast<sal_Int32>(rOpt.GetLevel());   break;//Level",
        case 7: pValues[nProp] <<= static_cast<sal_Int32>(rOpt.GetPos());     break;//Position",
        case 8: pValues[nProp] <<= rOpt.GetCharacterStyle(); break; //CharacterStyle
        case 9: pValues[nProp] <<= rOpt.CopyAttributes(); break; //ApplyAttributes
    }
}

void SwInsertConfig::Notify( const css::uno::Sequence< OUString >& ) {}

void SwInsertConfig::ImplCommit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        const InsCaptionOpt* pWriterTableOpt = nullptr;
        const InsCaptionOpt* pWriterFrameOpt = nullptr;
        const InsCaptionOpt* pWriterGraphicOpt = nullptr;
        const InsCaptionOpt* pOLECalcOpt = nullptr;
        const InsCaptionOpt* pOLEImpressOpt = nullptr;
        const InsCaptionOpt* pOLEChartOpt = nullptr;
        const InsCaptionOpt* pOLEFormulaOpt = nullptr;
        const InsCaptionOpt* pOLEDrawOpt = nullptr;
        if(pCapOptions)
        {
            pWriterTableOpt = pCapOptions->Find(TABLE_CAP);
            pWriterFrameOpt = pCapOptions->Find(FRAME_CAP);
            pWriterGraphicOpt = pCapOptions->Find(GRAPHIC_CAP);
            pOLECalcOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_CALC]);
            pOLEImpressOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_IMPRESS]);
            pOLEDrawOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_DRAW   ]);
            pOLEFormulaOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_MATH   ]);
            pOLEChartOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_CHART  ]);
        }
        switch(nProp)
        {
            case INS_PROP_TABLE_HEADER:
                pValues[nProp] <<= bool(aInsTableOpts.mnInsMode & SwInsertTableFlags::Headline);
            break;//"Table/Header",
            case INS_PROP_TABLE_REPEATHEADER:
                pValues[nProp] <<= aInsTableOpts.mnRowsToRepeat > 0;
            break;//"Table/RepeatHeader",
            case INS_PROP_TABLE_BORDER:
                pValues[nProp] <<= bool(aInsTableOpts.mnInsMode & SwInsertTableFlags::DefaultBorder);
            break;//"Table/Border",
            case INS_PROP_TABLE_SPLIT:
                pValues[nProp] <<= bool(aInsTableOpts.mnInsMode & SwInsertTableFlags::SplitLayout);
            break;//"Table/Split",
            case INS_PROP_CAP_AUTOMATIC:
                pValues[nProp] <<= bInsWithCaption;
            break;//"Caption/Automatic",
            case INS_PROP_CAP_CAPTIONORDERNUMBERINGFIRST:
                pValues[nProp] <<= bCaptionOrderNumberingFirst;
            break;//"Caption/CaptionOrderNumberingFirst"

            case INS_PROP_CAP_OBJECT_TABLE_ENABLE:
            case INS_PROP_CAP_OBJECT_TABLE_CATEGORY:
            case INS_PROP_CAP_OBJECT_TABLE_NUMBERING:
            case INS_PROP_CAP_OBJECT_TABLE_NUMBERINGSEPARATOR:
            case INS_PROP_CAP_OBJECT_TABLE_CAPTIONTEXT:
            case INS_PROP_CAP_OBJECT_TABLE_DELIMITER:
            case INS_PROP_CAP_OBJECT_TABLE_LEVEL:
            case INS_PROP_CAP_OBJECT_TABLE_POSITION:
            case INS_PROP_CAP_OBJECT_TABLE_CHARACTERSTYLE:
                    if(pWriterTableOpt)
                        lcl_WriteOpt(*pWriterTableOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_TABLE_ENABLE);
            break;
            case INS_PROP_CAP_OBJECT_FRAME_ENABLE:
            case INS_PROP_CAP_OBJECT_FRAME_CATEGORY:
            case INS_PROP_CAP_OBJECT_FRAME_NUMBERING:
            case INS_PROP_CAP_OBJECT_FRAME_NUMBERINGSEPARATOR:
            case INS_PROP_CAP_OBJECT_FRAME_CAPTIONTEXT:
            case INS_PROP_CAP_OBJECT_FRAME_DELIMITER:
            case INS_PROP_CAP_OBJECT_FRAME_LEVEL:
            case INS_PROP_CAP_OBJECT_FRAME_POSITION:
            case INS_PROP_CAP_OBJECT_FRAME_CHARACTERSTYLE:
                    if(pWriterFrameOpt)
                        lcl_WriteOpt(*pWriterFrameOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_FRAME_ENABLE);
            break;
            case INS_PROP_CAP_OBJECT_GRAPHIC_ENABLE:
            case INS_PROP_CAP_OBJECT_GRAPHIC_CATEGORY:
            case INS_PROP_CAP_OBJECT_GRAPHIC_NUMBERING:
            case INS_PROP_CAP_OBJECT_GRAPHIC_NUMBERINGSEPARATOR:
            case INS_PROP_CAP_OBJECT_GRAPHIC_CAPTIONTEXT:
            case INS_PROP_CAP_OBJECT_GRAPHIC_DELIMITER:
            case INS_PROP_CAP_OBJECT_GRAPHIC_LEVEL:
            case INS_PROP_CAP_OBJECT_GRAPHIC_POSITION:
            case INS_PROP_CAP_OBJECT_GRAPHIC_CHARACTERSTYLE:
            case INS_PROP_CAP_OBJECT_GRAPHIC_APPLYATTRIBUTES:
                    if(pWriterGraphicOpt)
                        lcl_WriteOpt(*pWriterGraphicOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_GRAPHIC_ENABLE);
            break;
            case INS_PROP_CAP_OBJECT_CALC_ENABLE:
            case INS_PROP_CAP_OBJECT_CALC_CATEGORY:
            case INS_PROP_CAP_OBJECT_CALC_NUMBERING:
            case INS_PROP_CAP_OBJECT_CALC_NUMBERINGSEPARATOR:
            case INS_PROP_CAP_OBJECT_CALC_CAPTIONTEXT:
            case INS_PROP_CAP_OBJECT_CALC_DELIMITER:
            case INS_PROP_CAP_OBJECT_CALC_LEVEL:
            case INS_PROP_CAP_OBJECT_CALC_POSITION:
            case INS_PROP_CAP_OBJECT_CALC_CHARACTERSTYLE:
            case INS_PROP_CAP_OBJECT_CALC_APPLYATTRIBUTES:
                    if(pOLECalcOpt)
                        lcl_WriteOpt(*pOLECalcOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_CALC_ENABLE);
            break;
            case INS_PROP_CAP_OBJECT_IMPRESS_ENABLE:
            case INS_PROP_CAP_OBJECT_IMPRESS_CATEGORY:
            case INS_PROP_CAP_OBJECT_IMPRESS_NUMBERING:
            case INS_PROP_CAP_OBJECT_IMPRESS_NUMBERINGSEPARATOR:
            case INS_PROP_CAP_OBJECT_IMPRESS_CAPTIONTEXT:
            case INS_PROP_CAP_OBJECT_IMPRESS_DELIMITER:
            case INS_PROP_CAP_OBJECT_IMPRESS_LEVEL:
            case INS_PROP_CAP_OBJECT_IMPRESS_POSITION:
            case INS_PROP_CAP_OBJECT_IMPRESS_CHARACTERSTYLE:
            case INS_PROP_CAP_OBJECT_IMPRESS_APPLYATTRIBUTES:
                    if(pOLEImpressOpt)
                        lcl_WriteOpt(*pOLEImpressOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_IMPRESS_ENABLE);
            break;
            case INS_PROP_CAP_OBJECT_CHART_ENABLE:
            case INS_PROP_CAP_OBJECT_CHART_CATEGORY:
            case INS_PROP_CAP_OBJECT_CHART_NUMBERING:
            case INS_PROP_CAP_OBJECT_CHART_NUMBERINGSEPARATOR:
            case INS_PROP_CAP_OBJECT_CHART_CAPTIONTEXT:
            case INS_PROP_CAP_OBJECT_CHART_DELIMITER:
            case INS_PROP_CAP_OBJECT_CHART_LEVEL:
            case INS_PROP_CAP_OBJECT_CHART_POSITION:
            case INS_PROP_CAP_OBJECT_CHART_CHARACTERSTYLE:
            case INS_PROP_CAP_OBJECT_CHART_APPLYATTRIBUTES:
                    if(pOLEChartOpt)
                        lcl_WriteOpt(*pOLEChartOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_CHART_ENABLE);
            break;
            case INS_PROP_CAP_OBJECT_FORMULA_ENABLE:
            case INS_PROP_CAP_OBJECT_FORMULA_CATEGORY:
            case INS_PROP_CAP_OBJECT_FORMULA_NUMBERING:
            case INS_PROP_CAP_OBJECT_FORMULA_NUMBERINGSEPARATOR:
            case INS_PROP_CAP_OBJECT_FORMULA_CAPTIONTEXT:
            case INS_PROP_CAP_OBJECT_FORMULA_DELIMITER:
            case INS_PROP_CAP_OBJECT_FORMULA_LEVEL:
            case INS_PROP_CAP_OBJECT_FORMULA_POSITION:
            case INS_PROP_CAP_OBJECT_FORMULA_CHARACTERSTYLE:
            case INS_PROP_CAP_OBJECT_FORMULA_APPLYATTRIBUTES:
                    if(pOLEFormulaOpt)
                        lcl_WriteOpt(*pOLEFormulaOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_FORMULA_ENABLE);
            break;
            case INS_PROP_CAP_OBJECT_DRAW_ENABLE:
            case INS_PROP_CAP_OBJECT_DRAW_CATEGORY:
            case INS_PROP_CAP_OBJECT_DRAW_NUMBERING:
            case INS_PROP_CAP_OBJECT_DRAW_NUMBERINGSEPARATOR:
            case INS_PROP_CAP_OBJECT_DRAW_CAPTIONTEXT:
            case INS_PROP_CAP_OBJECT_DRAW_DELIMITER:
            case INS_PROP_CAP_OBJECT_DRAW_LEVEL:
            case INS_PROP_CAP_OBJECT_DRAW_POSITION:
            case INS_PROP_CAP_OBJECT_DRAW_CHARACTERSTYLE:
            case INS_PROP_CAP_OBJECT_DRAW_APPLYATTRIBUTES:
                    if(pOLEDrawOpt)
                        lcl_WriteOpt(*pOLEDrawOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_DRAW_ENABLE);
            break;
            case INS_PROP_CAP_OBJECT_OLEMISC_ENABLE:
            case INS_PROP_CAP_OBJECT_OLEMISC_CATEGORY:
            case INS_PROP_CAP_OBJECT_OLEMISC_NUMBERING:
            case INS_PROP_CAP_OBJECT_OLEMISC_NUMBERINGSEPARATOR:
            case INS_PROP_CAP_OBJECT_OLEMISC_CAPTIONTEXT:
            case INS_PROP_CAP_OBJECT_OLEMISC_DELIMITER:
            case INS_PROP_CAP_OBJECT_OLEMISC_LEVEL:
            case INS_PROP_CAP_OBJECT_OLEMISC_POSITION:
            case INS_PROP_CAP_OBJECT_OLEMISC_CHARACTERSTYLE:
            case INS_PROP_CAP_OBJECT_OLEMISC_APPLYATTRIBUTES:
                    if(pOLEMiscOpt)
                        lcl_WriteOpt(*pOLEMiscOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_OLEMISC_ENABLE);
            break;

        }
    }
    PutProperties(aNames, aValues);
}

static void lcl_ReadOpt(InsCaptionOpt& rOpt, const Any* pValues, sal_Int32 nProp, sal_Int32 nOffset)
{
    switch(nOffset)
    {
        case 0:
            rOpt.UseCaption() = *o3tl::doAccess<bool>(pValues[nProp]);
        break;//Enable
        case 1:
        {
            OUString sTemp; pValues[nProp] >>= sTemp;
            rOpt.SetCategory(sTemp);
        }
        break;//Category
        case 2:
        {
            sal_Int32 nTemp = 0;
            pValues[nProp] >>= nTemp;
            rOpt.SetNumType(sal::static_int_cast< sal_uInt16, sal_Int32>(nTemp));
        }
        break;//Numbering",
        case 3:
        {
            OUString sTemp; pValues[nProp] >>= sTemp;
            rOpt.SetNumSeparator(sTemp);
        }
        break;//NumberingSeparator",
        case 4:
        {
            OUString sTemp; pValues[nProp] >>= sTemp;
            rOpt.SetCaption(sTemp);
        }
        break;//CaptionText",
        case 5:
        {
            OUString sTemp;
            if(pValues[nProp] >>= sTemp)
                rOpt.SetSeparator(sTemp);
        }
        break;//Delimiter",
        case 6:
        {
            sal_Int32 nTemp = 0;
            pValues[nProp] >>= nTemp;
            rOpt.SetLevel(sal::static_int_cast< sal_uInt16, sal_Int32>(nTemp));
        }
        break;//Level",
        case 7:
        {
            sal_Int32 nTemp = 0;
            pValues[nProp] >>= nTemp;
            rOpt.SetPos(sal::static_int_cast< sal_uInt16, sal_Int32>(nTemp));
        }
        break;//Position",
        case 8 : //CharacterStyle
        {
            OUString sTemp; pValues[nProp] >>= sTemp;
            rOpt.SetCharacterStyle( sTemp );
        }
        break;
        case 9 : //ApplyAttributes
        {
            pValues[nProp] >>= rOpt.CopyAttributes();
        }
        break;
    }
}

void SwInsertConfig::Load()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    assert(aValues.getLength() == aNames.getLength());
    InsCaptionOpt* pWriterTableOpt = nullptr;
    InsCaptionOpt* pWriterFrameOpt = nullptr;
    InsCaptionOpt* pWriterGraphicOpt = nullptr;
    InsCaptionOpt* pOLECalcOpt = nullptr;
    InsCaptionOpt* pOLEImpressOpt = nullptr;
    InsCaptionOpt* pOLEChartOpt = nullptr;
    InsCaptionOpt* pOLEFormulaOpt = nullptr;
    InsCaptionOpt* pOLEDrawOpt = nullptr;
    if (pCapOptions)
    {
        pWriterTableOpt = pCapOptions->Find(TABLE_CAP);
        pWriterFrameOpt = pCapOptions->Find(FRAME_CAP);
        pWriterGraphicOpt = pCapOptions->Find(GRAPHIC_CAP);
        pOLECalcOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_CALC]);
        pOLEImpressOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_IMPRESS]);
        pOLEDrawOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_DRAW   ]);
        pOLEFormulaOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_MATH   ]);
        pOLEChartOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_CHART  ]);
    }
    else if (!bIsWeb)
        return;

    SwInsertTableFlags nInsTableFlags = SwInsertTableFlags::NONE;
    for (sal_Int32 nProp = 0; nProp < aNames.getLength(); ++nProp)
    {
        if (pValues[nProp].hasValue())
        {
            bool bBool = nProp < INS_PROP_CAP_OBJECT_TABLE_ENABLE && *o3tl::doAccess<bool>(pValues[nProp]);
            switch (nProp)
            {
                case INS_PROP_TABLE_HEADER:
                {
                    if(bBool)
                        nInsTableFlags |= SwInsertTableFlags::Headline;
                }
                break;//"Table/Header",
                case INS_PROP_TABLE_REPEATHEADER:
                {
                    aInsTableOpts.mnRowsToRepeat = bBool? 1 : 0;

                }
                break;//"Table/RepeatHeader",
                case INS_PROP_TABLE_BORDER:
                {
                    if(bBool)
                        nInsTableFlags |= SwInsertTableFlags::DefaultBorder;
                }
                break;//"Table/Border",
                case INS_PROP_TABLE_SPLIT:
                {
                    if(bBool)
                        nInsTableFlags |= SwInsertTableFlags::SplitLayout;
                }
                break;//"Table/Split",
                case INS_PROP_CAP_AUTOMATIC:
                    bInsWithCaption = bBool;
                break;
                case INS_PROP_CAP_CAPTIONORDERNUMBERINGFIRST: bCaptionOrderNumberingFirst = bBool; break;
                case INS_PROP_CAP_OBJECT_TABLE_ENABLE:
                case INS_PROP_CAP_OBJECT_TABLE_CATEGORY:
                case INS_PROP_CAP_OBJECT_TABLE_NUMBERING:
                case INS_PROP_CAP_OBJECT_TABLE_NUMBERINGSEPARATOR:
                case INS_PROP_CAP_OBJECT_TABLE_CAPTIONTEXT:
                case INS_PROP_CAP_OBJECT_TABLE_DELIMITER:
                case INS_PROP_CAP_OBJECT_TABLE_LEVEL:
                case INS_PROP_CAP_OBJECT_TABLE_POSITION:
                case INS_PROP_CAP_OBJECT_TABLE_CHARACTERSTYLE:
                    if(!pWriterTableOpt)
                    {
                        pWriterTableOpt = new InsCaptionOpt(TABLE_CAP);
                        pCapOptions->Insert(pWriterTableOpt);
                    }
                    lcl_ReadOpt(*pWriterTableOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_TABLE_ENABLE);
                break;
                case INS_PROP_CAP_OBJECT_FRAME_ENABLE:
                case INS_PROP_CAP_OBJECT_FRAME_CATEGORY:
                case INS_PROP_CAP_OBJECT_FRAME_NUMBERING:
                case INS_PROP_CAP_OBJECT_FRAME_NUMBERINGSEPARATOR:
                case INS_PROP_CAP_OBJECT_FRAME_CAPTIONTEXT:
                case INS_PROP_CAP_OBJECT_FRAME_DELIMITER:
                case INS_PROP_CAP_OBJECT_FRAME_LEVEL:
                case INS_PROP_CAP_OBJECT_FRAME_POSITION:
                case INS_PROP_CAP_OBJECT_FRAME_CHARACTERSTYLE:
                    if(!pWriterFrameOpt)
                    {
                        pWriterFrameOpt = new InsCaptionOpt(FRAME_CAP);
                        pCapOptions->Insert(pWriterFrameOpt);
                    }
                    lcl_ReadOpt(*pWriterFrameOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_FRAME_ENABLE);
                break;
                case INS_PROP_CAP_OBJECT_GRAPHIC_ENABLE:
                case INS_PROP_CAP_OBJECT_GRAPHIC_CATEGORY:
                case INS_PROP_CAP_OBJECT_GRAPHIC_NUMBERING:
                case INS_PROP_CAP_OBJECT_GRAPHIC_NUMBERINGSEPARATOR:
                case INS_PROP_CAP_OBJECT_GRAPHIC_CAPTIONTEXT:
                case INS_PROP_CAP_OBJECT_GRAPHIC_DELIMITER:
                case INS_PROP_CAP_OBJECT_GRAPHIC_LEVEL:
                case INS_PROP_CAP_OBJECT_GRAPHIC_POSITION:
                case INS_PROP_CAP_OBJECT_GRAPHIC_CHARACTERSTYLE:
                case INS_PROP_CAP_OBJECT_GRAPHIC_APPLYATTRIBUTES:
                    if(!pWriterGraphicOpt)
                    {
                        pWriterGraphicOpt = new InsCaptionOpt(GRAPHIC_CAP);
                        pCapOptions->Insert(pWriterGraphicOpt);
                    }
                    lcl_ReadOpt(*pWriterGraphicOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_GRAPHIC_ENABLE);
                break;
                case INS_PROP_CAP_OBJECT_CALC_ENABLE:
                case INS_PROP_CAP_OBJECT_CALC_CATEGORY:
                case INS_PROP_CAP_OBJECT_CALC_NUMBERING:
                case INS_PROP_CAP_OBJECT_CALC_NUMBERINGSEPARATOR:
                case INS_PROP_CAP_OBJECT_CALC_CAPTIONTEXT:
                case INS_PROP_CAP_OBJECT_CALC_DELIMITER:
                case INS_PROP_CAP_OBJECT_CALC_LEVEL:
                case INS_PROP_CAP_OBJECT_CALC_POSITION:
                case INS_PROP_CAP_OBJECT_CALC_CHARACTERSTYLE:
                case INS_PROP_CAP_OBJECT_CALC_APPLYATTRIBUTES:
                    if(!pOLECalcOpt)
                    {
                        pOLECalcOpt = new InsCaptionOpt(OLE_CAP, &aGlobalNames[GLOB_NAME_CALC]);
                        pCapOptions->Insert(pOLECalcOpt);
                    }
                    lcl_ReadOpt(*pOLECalcOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_CALC_ENABLE);
                break;
                case INS_PROP_CAP_OBJECT_IMPRESS_ENABLE:
                case INS_PROP_CAP_OBJECT_IMPRESS_CATEGORY:
                case INS_PROP_CAP_OBJECT_IMPRESS_NUMBERING:
                case INS_PROP_CAP_OBJECT_IMPRESS_NUMBERINGSEPARATOR:
                case INS_PROP_CAP_OBJECT_IMPRESS_CAPTIONTEXT:
                case INS_PROP_CAP_OBJECT_IMPRESS_DELIMITER:
                case INS_PROP_CAP_OBJECT_IMPRESS_LEVEL:
                case INS_PROP_CAP_OBJECT_IMPRESS_POSITION:
                case INS_PROP_CAP_OBJECT_IMPRESS_CHARACTERSTYLE:
                case INS_PROP_CAP_OBJECT_IMPRESS_APPLYATTRIBUTES:
                    if(!pOLEImpressOpt)
                    {
                        pOLEImpressOpt = new InsCaptionOpt(OLE_CAP, &aGlobalNames[GLOB_NAME_IMPRESS]);
                        pCapOptions->Insert(pOLEImpressOpt);
                    }
                    lcl_ReadOpt(*pOLEImpressOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_IMPRESS_ENABLE);
                break;
                case INS_PROP_CAP_OBJECT_CHART_ENABLE:
                case INS_PROP_CAP_OBJECT_CHART_CATEGORY:
                case INS_PROP_CAP_OBJECT_CHART_NUMBERING:
                case INS_PROP_CAP_OBJECT_CHART_NUMBERINGSEPARATOR:
                case INS_PROP_CAP_OBJECT_CHART_CAPTIONTEXT:
                case INS_PROP_CAP_OBJECT_CHART_DELIMITER:
                case INS_PROP_CAP_OBJECT_CHART_LEVEL:
                case INS_PROP_CAP_OBJECT_CHART_POSITION:
                case INS_PROP_CAP_OBJECT_CHART_CHARACTERSTYLE:
                case INS_PROP_CAP_OBJECT_CHART_APPLYATTRIBUTES:
                    if(!pOLEChartOpt)
                    {
                        pOLEChartOpt = new InsCaptionOpt(OLE_CAP, &aGlobalNames[GLOB_NAME_CHART]);
                        pCapOptions->Insert(pOLEChartOpt);
                    }
                    lcl_ReadOpt(*pOLEChartOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_CHART_ENABLE);
                break;
                case INS_PROP_CAP_OBJECT_FORMULA_ENABLE:
                case INS_PROP_CAP_OBJECT_FORMULA_CATEGORY:
                case INS_PROP_CAP_OBJECT_FORMULA_NUMBERING:
                case INS_PROP_CAP_OBJECT_FORMULA_NUMBERINGSEPARATOR:
                case INS_PROP_CAP_OBJECT_FORMULA_CAPTIONTEXT:
                case INS_PROP_CAP_OBJECT_FORMULA_DELIMITER:
                case INS_PROP_CAP_OBJECT_FORMULA_LEVEL:
                case INS_PROP_CAP_OBJECT_FORMULA_POSITION:
                case INS_PROP_CAP_OBJECT_FORMULA_CHARACTERSTYLE:
                case INS_PROP_CAP_OBJECT_FORMULA_APPLYATTRIBUTES:
                    if(!pOLEFormulaOpt)
                    {
                        pOLEFormulaOpt = new InsCaptionOpt(OLE_CAP, &aGlobalNames[GLOB_NAME_MATH]);
                        pCapOptions->Insert(pOLEFormulaOpt);
                    }
                    lcl_ReadOpt(*pOLEFormulaOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_FORMULA_ENABLE);
                break;
                case INS_PROP_CAP_OBJECT_DRAW_ENABLE:
                case INS_PROP_CAP_OBJECT_DRAW_CATEGORY:
                case INS_PROP_CAP_OBJECT_DRAW_NUMBERING:
                case INS_PROP_CAP_OBJECT_DRAW_NUMBERINGSEPARATOR:
                case INS_PROP_CAP_OBJECT_DRAW_CAPTIONTEXT:
                case INS_PROP_CAP_OBJECT_DRAW_DELIMITER:
                case INS_PROP_CAP_OBJECT_DRAW_LEVEL:
                case INS_PROP_CAP_OBJECT_DRAW_POSITION:
                case INS_PROP_CAP_OBJECT_DRAW_CHARACTERSTYLE:
                case INS_PROP_CAP_OBJECT_DRAW_APPLYATTRIBUTES:
                    if(!pOLEDrawOpt)
                    {
                        pOLEDrawOpt = new InsCaptionOpt(OLE_CAP, &aGlobalNames[GLOB_NAME_DRAW]);
                        pCapOptions->Insert(pOLEDrawOpt);
                    }
                    lcl_ReadOpt(*pOLEDrawOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_DRAW_ENABLE);
                break;
                case INS_PROP_CAP_OBJECT_OLEMISC_ENABLE:
                case INS_PROP_CAP_OBJECT_OLEMISC_CATEGORY:
                case INS_PROP_CAP_OBJECT_OLEMISC_NUMBERING:
                case INS_PROP_CAP_OBJECT_OLEMISC_NUMBERINGSEPARATOR:
                case INS_PROP_CAP_OBJECT_OLEMISC_CAPTIONTEXT:
                case INS_PROP_CAP_OBJECT_OLEMISC_DELIMITER:
                case INS_PROP_CAP_OBJECT_OLEMISC_LEVEL:
                case INS_PROP_CAP_OBJECT_OLEMISC_POSITION:
                case INS_PROP_CAP_OBJECT_OLEMISC_CHARACTERSTYLE:
                case INS_PROP_CAP_OBJECT_OLEMISC_APPLYATTRIBUTES:
                    if(!pOLEMiscOpt)
                    {
                        pOLEMiscOpt.reset(new InsCaptionOpt(OLE_CAP));
                    }
                    lcl_ReadOpt(*pOLEMiscOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_OLEMISC_ENABLE);
                break;
            }
        }
        else if (nProp == INS_PROP_CAP_CAPTIONORDERNUMBERINGFIRST)
        {
            bCaptionOrderNumberingFirst = false;
        }

    }
    aInsTableOpts.mnInsMode = nInsTableFlags;
}

const Sequence<OUString>& SwTableConfig::GetPropertyNames()
{
    const int nCount = 8;
    static Sequence<OUString> aNames(nCount);
    static const char* aPropNames[] =
    {
        "Shift/Row",                    //  0
        "Shift/Column",                 //  1
        "Insert/Row",                   //  2
        "Insert/Column",                //  3
        "Change/Effect",                //  4
        "Input/NumberRecognition",      //  5
        "Input/NumberFormatRecognition",//  6
        "Input/Alignment"               //  7
    };
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}

SwTableConfig::SwTableConfig(bool bWeb)
    : ConfigItem(bWeb ? OUString("Office.WriterWeb/Table") : OUString("Office.Writer/Table"),
        ConfigItemMode::DelayedUpdate|ConfigItemMode::ReleaseTree)
    , nTableHMove(0)
    , nTableVMove(0)
    , nTableHInsert(0)
    , nTableVInsert(0)
    , eTableChgMode(TableChgMode::FixedWidthChangeAbs)
    , bInsTableFormatNum(false)
    , bInsTableChangeNumFormat(false)
    , bInsTableAlignNum(false)
{
    Load();
}

SwTableConfig::~SwTableConfig()
{
}

void SwTableConfig::Notify( const css::uno::Sequence< OUString >& ) {}

void SwTableConfig::ImplCommit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0 : pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(nTableHMove)); break;   //"Shift/Row",
            case 1 : pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(nTableVMove)); break;     //"Shift/Column",
            case 2 : pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(nTableHInsert)); break;   //"Insert/Row",
            case 3 : pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(nTableVInsert)); break;   //"Insert/Column",
            case 4 : pValues[nProp] <<= static_cast<sal_Int32>(eTableChgMode); break;   //"Change/Effect",
            case 5 : pValues[nProp] <<= bInsTableFormatNum; break;  //"Input/NumberRecognition",
            case 6 : pValues[nProp] <<= bInsTableChangeNumFormat; break;  //"Input/NumberFormatRecognition",
            case 7 : pValues[nProp] <<= bInsTableAlignNum; break;  //"Input/Alignment"
        }
    }
    PutProperties(aNames, aValues);
}

void SwTableConfig::Load()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    assert(aValues.getLength() == aNames.getLength());
    for (sal_Int32 nProp = 0; nProp < aNames.getLength(); ++nProp)
    {
        if (pValues[nProp].hasValue())
        {
            sal_Int32 nTemp = 0;
            switch (nProp)
            {
                case 0 : pValues[nProp] >>= nTemp; nTableHMove = static_cast<sal_uInt16>(convertMm100ToTwip(nTemp)); break;  //"Shift/Row",
                case 1 : pValues[nProp] >>= nTemp; nTableVMove = static_cast<sal_uInt16>(convertMm100ToTwip(nTemp)); break;     //"Shift/Column",
                case 2 : pValues[nProp] >>= nTemp; nTableHInsert = static_cast<sal_uInt16>(convertMm100ToTwip(nTemp)); break;   //"Insert/Row",
                case 3 : pValues[nProp] >>= nTemp; nTableVInsert = static_cast<sal_uInt16>(convertMm100ToTwip(nTemp)); break;   //"Insert/Column",
                case 4 : pValues[nProp] >>= nTemp; eTableChgMode = static_cast<TableChgMode>(nTemp); break;   //"Change/Effect",
                case 5 : bInsTableFormatNum = *o3tl::doAccess<bool>(pValues[nProp]);  break;  //"Input/NumberRecognition",
                case 6 : bInsTableChangeNumFormat = *o3tl::doAccess<bool>(pValues[nProp]); break;  //"Input/NumberFormatRecognition",
                case 7 : bInsTableAlignNum = *o3tl::doAccess<bool>(pValues[nProp]); break;  //"Input/Alignment"
            }
        }
    }
}

SwMiscConfig::SwMiscConfig() :
    ConfigItem("Office.Writer",
        ConfigItemMode::DelayedUpdate|ConfigItemMode::ReleaseTree),
    bDefaultFontsInCurrDocOnly(false),
    bShowIndexPreview(false),
    bGrfToGalleryAsLnk(true),
    bNumAlignSize(true),
    bSinglePrintJob(false),
    bIsNameFromColumn(true),
    bAskForMailMergeInPrint(true),
    nMailingFormats(MailTextFormats::NONE)
{
    Load();
}

SwMiscConfig::~SwMiscConfig()
{
}

const Sequence<OUString>& SwMiscConfig::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        const int nCount = 12;
        aNames.realloc(nCount);
        static const char* aPropNames[] =
        {
            "Statistics/WordNumber/Delimiter",          // 0
            "DefaultFont/Document",                     // 1
            "Index/ShowPreview",                        // 2
            "Misc/GraphicToGalleryAsLink",              // 3
            "Numbering/Graphic/KeepRatio",              // 4
            "FormLetter/PrintOutput/SinglePrintJobs",   // 5
            "FormLetter/MailingOutput/Format",          // 6
            "FormLetter/FileOutput/FileName/FromDatabaseField",  // 7
            "FormLetter/FileOutput/Path",               // 8
            "FormLetter/FileOutput/FileName/FromManualSetting",   // 9
            "FormLetter/FileOutput/FileName/Generation",//10
            "FormLetter/PrintOutput/AskForMerge"        //11
        };
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

void SwMiscConfig::Notify( const css::uno::Sequence< OUString >& ) {}

void SwMiscConfig::ImplCommit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0 :
                pValues[nProp] <<=
                    SwModuleOptions::ConvertWordDelimiter(sWordDelimiter, false);
            break;
            case 1 : pValues[nProp] <<= bDefaultFontsInCurrDocOnly; break;
            case 2 : pValues[nProp] <<= bShowIndexPreview; break;
            case 3 : pValues[nProp] <<= bGrfToGalleryAsLnk; break;
            case 4 : pValues[nProp] <<= bNumAlignSize; break;
            case 5 : pValues[nProp] <<= bSinglePrintJob; break;
            case 6 : pValues[nProp] <<= static_cast<sal_Int32>(nMailingFormats); break;
            case 7 : pValues[nProp] <<= sNameFromColumn;  break;
            case 8 : pValues[nProp] <<= sMailingPath;     break;
            case 9 : pValues[nProp] <<= sMailName;        break;
            case 10: pValues[nProp] <<= bIsNameFromColumn; break;
            case 11: pValues[nProp] <<= bAskForMailMergeInPrint; break;
        }
    }
    PutProperties(aNames, aValues);
}

void SwMiscConfig::Load()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    assert(aValues.getLength() == aNames.getLength());
    OUString sTmp;
    for (sal_Int32 nProp = 0; nProp < aNames.getLength(); ++nProp)
    {
        if (pValues[nProp].hasValue())
        {
            switch (nProp)
            {
                case 0 : pValues[nProp] >>= sTmp;
                    sWordDelimiter = SwModuleOptions::ConvertWordDelimiter(sTmp, true);
                break;
                case 1 : bDefaultFontsInCurrDocOnly = *o3tl::doAccess<bool>(pValues[nProp]); break;
                case 2 : bShowIndexPreview = *o3tl::doAccess<bool>(pValues[nProp]); break;
                case 3 : bGrfToGalleryAsLnk = *o3tl::doAccess<bool>(pValues[nProp]); break;
                case 4 : bNumAlignSize = *o3tl::doAccess<bool>(pValues[nProp]); break;
                case 5 : bSinglePrintJob = *o3tl::doAccess<bool>(pValues[nProp]); break;
                case 6 : nMailingFormats = static_cast<MailTextFormats>(*o3tl::doAccess<sal_Int32>(pValues[nProp])); break;
                case 7 : pValues[nProp] >>= sTmp; sNameFromColumn = sTmp; break;
                case 8 : pValues[nProp] >>= sTmp; sMailingPath = sTmp;  break;
                case 9 : pValues[nProp] >>= sTmp; sMailName = sTmp;     break;
                case 10: bIsNameFromColumn = *o3tl::doAccess<bool>(pValues[nProp]); break;
                case 11: pValues[nProp] >>= bAskForMailMergeInPrint; break;
            }
        }
    }
}

const Sequence<OUString>& SwCompareConfig::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        const int nCount = 5;
        aNames.realloc(nCount);
        static const char* aPropNames[] =
        {
            "Mode",                         // 0
            "UseRSID",                      // 1
            "IgnorePieces",             // 2
            "IgnoreLength", // 3
            "StoreRSID" // 4
        };
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

SwCompareConfig::SwCompareConfig() :
    ConfigItem("Office.Writer/Comparison",
        ConfigItemMode::DelayedUpdate|ConfigItemMode::ReleaseTree)
    ,m_bStoreRsid(true)
{
    m_eCmpMode = SwCompareMode::Auto;
    m_bUseRsid = false;
    m_bIgnorePieces = false;
    m_nPieceLen = 1;

    Load();
}

SwCompareConfig::~SwCompareConfig()
{
}

void SwCompareConfig::ImplCommit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    pValues[0] <<= static_cast<sal_Int32>(m_eCmpMode);
    pValues[1] <<= m_bUseRsid;
    pValues[2] <<= m_bIgnorePieces;
    pValues[3] <<= static_cast<sal_Int32>(m_nPieceLen);
    pValues[4] <<= m_bStoreRsid;

    PutProperties(aNames, aValues);
}

void SwCompareConfig::Load()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    assert(aValues.getLength() == aNames.getLength());
    for (sal_Int32 nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        if (pValues[nProp].hasValue())
        {
            sal_Int32 nVal = 0;
            pValues[nProp] >>= nVal;

            switch(nProp)
            {
                case 0 : m_eCmpMode = static_cast<SwCompareMode>(nVal); break;
                case 1 : m_bUseRsid = *o3tl::doAccess<bool>(pValues[nProp]); break;
                case 2 : m_bIgnorePieces = *o3tl::doAccess<bool>(pValues[nProp]); break;
                case 3 : m_nPieceLen = nVal; break;
                case 4 : m_bStoreRsid = *o3tl::doAccess<bool>(pValues[nProp]); break;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
