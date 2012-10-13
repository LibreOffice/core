/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <hintids.hxx>
#include <sot/clsids.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <svl/mailenum.hxx>
#include <svx/svxids.hrc>
#include <editeng/svxenum.hxx>
#include <osl/diagnose.h>

#include <tools/globname.hxx>
#include <swtypes.hxx>
#include <itabenum.hxx>
#include <modcfg.hxx>
#include <fldupde.hxx>
#include <unotools/syslocaleoptions.hxx>
#include <caption.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <unomid.h>

using namespace utl;
using rtl::OUString;
using namespace com::sun::star::uno;

#define GLOB_NAME_CALC      0
#define GLOB_NAME_IMPRESS   1
#define GLOB_NAME_DRAW      2
#define GLOB_NAME_MATH      3
#define GLOB_NAME_CHART     4

InsCaptionOpt* InsCaptionOptArr::Find(const SwCapObjType eType, const SvGlobalName *pOleId)
{
    for (InsCapOptArr::iterator aI = m_aInsCapOptArr.begin(); aI != m_aInsCapOptArr.end(); ++aI)
    {
        InsCaptionOpt &rObj = *aI;
        if (rObj.GetObjType() == eType && (eType != OLE_CAP || (pOleId && rObj.GetOleId() == *pOleId)))
            return &rObj;
    }

    return NULL;
}

void InsCaptionOptArr::Insert(InsCaptionOpt* pObj)
{
    m_aInsCapOptArr.push_back(pObj); //takes ownership
}

const InsCaptionOpt* SwModuleOptions::GetCapOption(
    sal_Bool bHTML, const SwCapObjType eType, const SvGlobalName *pOleId)
{
    if(bHTML)
    {
        OSL_FAIL("no caption option in sw/web!");
        return 0;
    }
    else
    {
        sal_Bool bFound = sal_False;
        if(eType == OLE_CAP && pOleId)
        {
            for( sal_uInt16 nId = 0; nId <= GLOB_NAME_CHART && !bFound; nId++)
                bFound = *pOleId == aInsertConfig.aGlobalNames[nId  ];
            if(!bFound)
                return aInsertConfig.pOLEMiscOpt;
        }
        return aInsertConfig.pCapOptions->Find(eType, pOleId);
    }
}

sal_Bool SwModuleOptions::SetCapOption(sal_Bool bHTML, const InsCaptionOpt* pOpt)
{
    sal_Bool bRet = sal_False;

    if(bHTML)
    {
        OSL_FAIL("no caption option in sw/web!");
    }
    else if (pOpt)
    {
        sal_Bool bFound = sal_False;
        if(pOpt->GetObjType() == OLE_CAP && &pOpt->GetOleId())
        {
            for( sal_uInt16 nId = 0; nId <= GLOB_NAME_CHART; nId++)
                bFound = pOpt->GetOleId() == aInsertConfig.aGlobalNames[nId  ];
            if(!bFound)
            {
                if(aInsertConfig.pOLEMiscOpt)
                    *aInsertConfig.pOLEMiscOpt = *pOpt;
                else
                    aInsertConfig.pOLEMiscOpt = new InsCaptionOpt(*pOpt);
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
        bRet = sal_True;
    }

    return bRet;
}

SwModuleOptions::SwModuleOptions() :
    aInsertConfig(sal_False),
    aWebInsertConfig(sal_True),
    aTableConfig(sal_False),
    aWebTableConfig(sal_True),
    bHideFieldTips(sal_False)
{
}

String SwModuleOptions::ConvertWordDelimiter(const String& rDelim, sal_Bool bFromUI)
{
    String sReturn;
    if(bFromUI)
    {
        String sChar;

        xub_StrLen i = 0;
        sal_Unicode c;

        while (i < rDelim.Len())
        {
            c = rDelim.GetChar(i++);

            if (c == '\\')
            {
                c = rDelim.GetChar(i++);

                switch (c)
                {
                    case 'n':   sReturn += '\n';    break;
                    case 't':   sReturn += '\t';    break;
                    case '\\':  sReturn += '\\';    break;

                    case 'x':
                    {
                        sal_Unicode nVal, nChar;
                        sal_Bool bValidData = sal_True;
                        xub_StrLen n;
                        for( n = 0, nChar = 0; n < 2 && i < rDelim.Len(); ++n, ++i )
                        {
                            if( ((nVal = rDelim.GetChar( i )) >= '0') && ( nVal <= '9') )
                                nVal -= '0';
                            else if( (nVal >= 'A') && (nVal <= 'F') )
                                nVal -= 'A' - 10;
                            else if( (nVal >= 'a') && (nVal <= 'f') )
                                nVal -= 'a' - 10;
                            else
                            {
                                OSL_FAIL("wrong hex value" );
                                bValidData = sal_False;
                                break;
                            }

                            (nChar <<= 4 );
                            nChar = nChar + nVal;
                        }
                        if( bValidData )
                            sReturn += nChar;
                        break;
                    }

                    default:    // Unknown, so insert backslash
                        sReturn += '\\';
                        i--;
                        break;
                }
            }
            else
                sReturn += c;
        }
    }
    else
    {
        for (xub_StrLen i = 0; i < rDelim.Len(); i++)
        {
            sal_Unicode c = rDelim.GetChar(i);

            switch (c)
            {
                case '\n':  sReturn.AppendAscii(RTL_CONSTASCII_STRINGPARAM("\\n")); break;
                case '\t':  sReturn.AppendAscii(RTL_CONSTASCII_STRINGPARAM("\\t")); break;
                case '\\':  sReturn.AppendAscii(RTL_CONSTASCII_STRINGPARAM("\\\\"));    break;

                default:
                    if( c <= 0x1f || c >= 0x7f )
                    {
                        sReturn.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "\\x" ))
                            += String::CreateFromInt32( c, 16 );
                    }
                    else
                        sReturn += c;
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
    ConfigItem(C2U("Office.Writer/Revision"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE)
{
    aInsertAttr.nItemId = SID_ATTR_CHAR_UNDERLINE;
    aInsertAttr.nAttr = UNDERLINE_SINGLE;
    aInsertAttr.nColor = COL_TRANSPARENT;
    aDeletedAttr.nItemId = SID_ATTR_CHAR_STRIKEOUT;
    aDeletedAttr.nAttr = STRIKEOUT_SINGLE;
    aDeletedAttr.nColor = COL_TRANSPARENT;
    aFormatAttr.nItemId = SID_ATTR_CHAR_WEIGHT;
    aFormatAttr.nAttr = WEIGHT_BOLD;
    aFormatAttr.nColor = COL_BLACK;

    Load();
}

SwRevisionConfig::~SwRevisionConfig()
{
}

static sal_Int32 lcl_ConvertAttrToCfg(const AuthorCharAttr& rAttr)
{
    sal_Int32 nRet = 0;
    switch(rAttr.nItemId)
    {
        case  SID_ATTR_CHAR_WEIGHT: nRet = 1; break;
        case  SID_ATTR_CHAR_POSTURE: nRet = 2; break;
        case  SID_ATTR_CHAR_UNDERLINE: nRet = UNDERLINE_SINGLE == rAttr.nAttr ? 3 : 4; break;
        case  SID_ATTR_CHAR_STRIKEOUT: nRet = 3; break;
        case  SID_ATTR_CHAR_CASEMAP:
        {
            switch(rAttr.nAttr)
            {
                case  SVX_CASEMAP_VERSALIEN   : nRet = 5;break;
                case  SVX_CASEMAP_GEMEINE     : nRet = 6;break;
                case  SVX_CASEMAP_KAPITAELCHEN: nRet = 7;break;
                case  SVX_CASEMAP_TITEL       : nRet = 8;break;
            }
        }
        break;
        case SID_ATTR_BRUSH : nRet = 9; break;
    }
    return nRet;
}

void SwRevisionConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

void SwRevisionConfig::Commit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Int32 nVal = -1;
        switch(nProp)
        {
            case 0 : nVal = lcl_ConvertAttrToCfg(aInsertAttr); break;
            case 1 : nVal = aInsertAttr.nColor  ; break;
            case 2 : nVal = lcl_ConvertAttrToCfg(aDeletedAttr); break;
            case 3 : nVal = aDeletedAttr.nColor ; break;
            case 4 : nVal = lcl_ConvertAttrToCfg(aFormatAttr); break;
            case 5 : nVal = aFormatAttr.nColor  ; break;
            case 6 : nVal = nMarkAlign          ; break;
            case 7 : nVal = aMarkColor.GetColor(); break;
        }
        pValues[nProp] <<= nVal;
    }
    PutProperties(aNames, aValues);
}

static void lcl_ConvertCfgToAttr(sal_Int32 nVal, AuthorCharAttr& rAttr, sal_Bool bDelete = sal_False)
{
    rAttr.nItemId = rAttr.nAttr = 0;
    switch(nVal)
    {
        case 1: rAttr.nItemId = SID_ATTR_CHAR_WEIGHT;   rAttr.nAttr = WEIGHT_BOLD              ; break;
        case 2: rAttr.nItemId = SID_ATTR_CHAR_POSTURE;  rAttr.nAttr = ITALIC_NORMAL            ; break;
        case 3: if(bDelete)
                {
                    rAttr.nItemId = SID_ATTR_CHAR_STRIKEOUT;
                    rAttr.nAttr = STRIKEOUT_SINGLE;
                }
                else
                {
                    rAttr.nItemId = SID_ATTR_CHAR_UNDERLINE;
                    rAttr.nAttr = UNDERLINE_SINGLE;
                }
        break;
        case 4: rAttr.nItemId = SID_ATTR_CHAR_UNDERLINE;rAttr.nAttr = UNDERLINE_DOUBLE         ; break;
        case 5: rAttr.nItemId = SID_ATTR_CHAR_CASEMAP;  rAttr.nAttr = SVX_CASEMAP_VERSALIEN    ; break;
        case 6: rAttr.nItemId = SID_ATTR_CHAR_CASEMAP;  rAttr.nAttr = SVX_CASEMAP_GEMEINE      ; break;
        case 7: rAttr.nItemId = SID_ATTR_CHAR_CASEMAP;  rAttr.nAttr = SVX_CASEMAP_KAPITAELCHEN ; break;
        case 8: rAttr.nItemId = SID_ATTR_CHAR_CASEMAP;  rAttr.nAttr = SVX_CASEMAP_TITEL        ; break;
        case 9: rAttr.nItemId = SID_ATTR_BRUSH; break;
    }
}
void SwRevisionConfig::Load()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                sal_Int32 nVal = 0;
                pValues[nProp] >>= nVal;
                switch(nProp)
                {
                    case 0 : lcl_ConvertCfgToAttr(nVal, aInsertAttr); break;
                    case 1 : aInsertAttr.nColor     = nVal; break;
                    case 2 : lcl_ConvertCfgToAttr(nVal, aDeletedAttr, sal_True); break;
                    case 3 : aDeletedAttr.nColor    = nVal; break;
                    case 4 : lcl_ConvertCfgToAttr(nVal, aFormatAttr); break;
                    case 5 : aFormatAttr.nColor     = nVal; break;
                    case 6 : nMarkAlign = sal::static_int_cast< sal_uInt16, sal_Int32>(nVal); break;
                    case 7 : aMarkColor.SetColor(nVal); break;
                }
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
            pNames[i] = rtl::OUString::createFromAscii(aPropNames[i]);
        for(i = 0; i < nWebCount; i++)
            pWebNames[i] = rtl::OUString::createFromAscii(aPropNames[i]);
    }
    return bIsWeb ? aWebNames : aNames;
}

SwInsertConfig::SwInsertConfig(sal_Bool bWeb) :
    ConfigItem(bWeb ? C2U("Office.WriterWeb/Insert") : C2U("Office.Writer/Insert"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    pCapOptions(0),
    pOLEMiscOpt(0),
    bInsWithCaption( sal_False ),
    bCaptionOrderNumberingFirst( sal_False ),
    aInsTblOpts(0,0),
    bIsWeb(bWeb)
{
    aGlobalNames[GLOB_NAME_CALC   ] = SvGlobalName(SO3_SC_CLASSID);
    aGlobalNames[GLOB_NAME_IMPRESS] = SvGlobalName(SO3_SIMPRESS_CLASSID);
    aGlobalNames[GLOB_NAME_DRAW   ] = SvGlobalName(SO3_SDRAW_CLASSID);
    aGlobalNames[GLOB_NAME_MATH   ] = SvGlobalName(SO3_SM_CLASSID);
    aGlobalNames[GLOB_NAME_CHART  ] = SvGlobalName(SO3_SCH_CLASSID);
    if(!bIsWeb)
        pCapOptions = new InsCaptionOptArr;

    Load();
}

SwInsertConfig::~SwInsertConfig()
{
    delete pCapOptions;
    delete pOLEMiscOpt;
}

static void lcl_WriteOpt(const InsCaptionOpt& rOpt, Any* pValues, sal_Int32 nProp, sal_Int32 nOffset)
{
    switch(nOffset)
    {
        case 0:
        {
            sal_Bool bTemp = rOpt.UseCaption();
            pValues[nProp].setValue(&bTemp, ::getBooleanCppuType());
        }
        break;//Enable
        case 1: pValues[nProp] <<= OUString(rOpt.GetCategory()); break;//Category
        case 2: pValues[nProp] <<= (sal_Int32)rOpt.GetNumType(); break;//Numbering",
        case 3: pValues[nProp] <<= rOpt.GetNumSeparator(); break;//NumberingSeparator",
        case 4: pValues[nProp] <<= OUString(rOpt.GetCaption());  break;//CaptionText",
        case 5: pValues[nProp] <<= OUString(rOpt.GetSeparator());break;//Delimiter",
        case 6: pValues[nProp] <<= (sal_Int32)rOpt.GetLevel();   break;//Level",
        case 7: pValues[nProp] <<= (sal_Int32)rOpt.GetPos();     break;//Position",
        case 8: pValues[nProp] <<= (::rtl::OUString&)rOpt.GetCharacterStyle(); break; //CharacterStyle
        case 9: pValues[nProp] <<= rOpt.CopyAttributes(); break; //ApplyAttributes
    }
}

void SwInsertConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

void SwInsertConfig::Commit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        const InsCaptionOpt* pWriterTableOpt = 0;
        const InsCaptionOpt* pWriterFrameOpt = 0;
        const InsCaptionOpt* pWriterGraphicOpt = 0;
        const InsCaptionOpt* pOLECalcOpt = 0;
        const InsCaptionOpt* pOLEImpressOpt = 0;
        const InsCaptionOpt* pOLEChartOpt = 0;
        const InsCaptionOpt* pOLEFormulaOpt = 0;
        const InsCaptionOpt* pOLEDrawOpt = 0;
        if(pCapOptions)
        {
            pWriterTableOpt = pCapOptions->Find(TABLE_CAP, 0);
            pWriterFrameOpt = pCapOptions->Find(FRAME_CAP, 0);
            pWriterGraphicOpt = pCapOptions->Find(GRAPHIC_CAP, 0);
            pOLECalcOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_CALC]);
            pOLEImpressOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_IMPRESS]);
            pOLEDrawOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_DRAW   ]);
            pOLEFormulaOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_MATH   ]);
            pOLEChartOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_CHART  ]);
        }
        switch(nProp)
        {
            case INS_PROP_TABLE_HEADER:
            {
                sal_Bool bVal = 0 !=(aInsTblOpts.mnInsMode & tabopts::HEADLINE); pValues[nProp].setValue(&bVal, rType);
            }
            break;//"Table/Header",
            case INS_PROP_TABLE_REPEATHEADER:
            {
                sal_Bool bVal = (aInsTblOpts.mnRowsToRepeat>0); pValues[nProp].setValue(&bVal, rType);
            }
            break;//"Table/RepeatHeader",
            case INS_PROP_TABLE_BORDER:
            {
                sal_Bool bVal = 0 !=(aInsTblOpts.mnInsMode & tabopts::DEFAULT_BORDER ); pValues[nProp].setValue(&bVal, rType);
            }
            break;//"Table/Border",
            case INS_PROP_TABLE_SPLIT:
            {
                sal_Bool bVal = 0 !=(aInsTblOpts.mnInsMode & tabopts::SPLIT_LAYOUT); pValues[nProp].setValue(&bVal, rType);
            }
            break;//"Table/Split",
            case INS_PROP_CAP_AUTOMATIC: pValues[nProp].setValue(&bInsWithCaption, rType);break;//"Caption/Automatic",
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
            rOpt.UseCaption() = *(sal_Bool*)pValues[nProp].getValue();
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
            ::rtl::OUString sTemp; pValues[nProp] >>= sTemp;
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
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        InsCaptionOpt* pWriterTableOpt = 0;
        InsCaptionOpt* pWriterFrameOpt = 0;
        InsCaptionOpt* pWriterGraphicOpt = 0;
        InsCaptionOpt* pOLECalcOpt = 0;
        InsCaptionOpt* pOLEImpressOpt = 0;
        InsCaptionOpt* pOLEChartOpt = 0;
        InsCaptionOpt* pOLEFormulaOpt = 0;
        InsCaptionOpt* pOLEDrawOpt = 0;
        if(pCapOptions)
        {
            pWriterTableOpt = pCapOptions->Find(TABLE_CAP, 0);
            pWriterFrameOpt = pCapOptions->Find(FRAME_CAP, 0);
            pWriterGraphicOpt = pCapOptions->Find(GRAPHIC_CAP, 0);
            pOLECalcOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_CALC]);
            pOLEImpressOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_IMPRESS]);
            pOLEDrawOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_DRAW   ]);
            pOLEFormulaOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_MATH   ]);
            pOLEChartOpt = pCapOptions->Find(OLE_CAP, &aGlobalNames[GLOB_NAME_CHART  ]);
        }
        else if(!bIsWeb)
            return;

        sal_uInt16 nInsTblFlags = 0;
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                sal_Bool bBool = nProp < INS_PROP_CAP_OBJECT_TABLE_ENABLE ? *(sal_Bool*)pValues[nProp].getValue() : sal_False;
                switch(nProp)
                {
                    case INS_PROP_TABLE_HEADER:
                    {
                        if(bBool)
                            nInsTblFlags|= tabopts::HEADLINE;
                    }
                    break;//"Table/Header",
                    case INS_PROP_TABLE_REPEATHEADER:
                    {
                        aInsTblOpts.mnRowsToRepeat = bBool? 1 : 0;

                    }
                    break;//"Table/RepeatHeader",
                    case INS_PROP_TABLE_BORDER:
                    {
                        if(bBool)
                            nInsTblFlags|= tabopts::DEFAULT_BORDER;
                    }
                    break;//"Table/Border",
                    case INS_PROP_TABLE_SPLIT:
                    {
                        if(bBool)
                            nInsTblFlags|= tabopts::SPLIT_LAYOUT;
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
                            pOLEMiscOpt = new InsCaptionOpt(OLE_CAP);
                        }
                        lcl_ReadOpt(*pOLEMiscOpt, pValues, nProp, nProp - INS_PROP_CAP_OBJECT_OLEMISC_ENABLE);
                    break;
                }
            }
            else if(nProp == INS_PROP_CAP_CAPTIONORDERNUMBERINGFIRST)
            {
                //#i61007#  initialize caption order, right now only HUNGARIAN seems to need a different order
                SvtSysLocaleOptions aSysLocaleOptions;
                OUString sLang = aSysLocaleOptions.GetLocaleConfigString();
                bCaptionOrderNumberingFirst = ( !sLang.isEmpty() && sLang.matchAsciiL( "hu", 2 )) ? sal_True : sal_False;
            }

        }
        aInsTblOpts.mnInsMode = nInsTblFlags;
    }
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

SwTableConfig::SwTableConfig(sal_Bool bWeb) :
    ConfigItem(bWeb ? C2U("Office.WriterWeb/Table") : C2U("Office.Writer/Table"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE)
{
    Load();
}

SwTableConfig::~SwTableConfig()
{
}

void SwTableConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

void SwTableConfig::Commit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0 : pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(nTblHMove); break;   //"Shift/Row",
            case 1 : pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(nTblVMove); break;     //"Shift/Column",
            case 2 : pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(nTblHInsert); break;   //"Insert/Row",
            case 3 : pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(nTblVInsert); break;   //"Insert/Column",
            case 4 : pValues[nProp] <<= (sal_Int32)eTblChgMode; break;   //"Change/Effect",
            case 5 : pValues[nProp].setValue(&bInsTblFormatNum, rType); break;  //"Input/NumberRecognition",
            case 6 : pValues[nProp].setValue(&bInsTblChangeNumFormat, rType); break;  //"Input/NumberFormatRecognition",
            case 7 : pValues[nProp].setValue(&bInsTblAlignNum, rType); break;  //"Input/Alignment"
        }
    }
    PutProperties(aNames, aValues);
}

void SwTableConfig::Load()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                sal_Int32 nTemp = 0;
                switch(nProp)
                {
                    case 0 : pValues[nProp] >>= nTemp; nTblHMove = (sal_uInt16)MM100_TO_TWIP(nTemp); break;  //"Shift/Row",
                    case 1 : pValues[nProp] >>= nTemp; nTblVMove = (sal_uInt16)MM100_TO_TWIP(nTemp); break;     //"Shift/Column",
                    case 2 : pValues[nProp] >>= nTemp; nTblHInsert = (sal_uInt16)MM100_TO_TWIP(nTemp); break;   //"Insert/Row",
                    case 3 : pValues[nProp] >>= nTemp; nTblVInsert = (sal_uInt16)MM100_TO_TWIP(nTemp); break;   //"Insert/Column",
                    case 4 : pValues[nProp] >>= nTemp; eTblChgMode = (TblChgMode)nTemp; break;   //"Change/Effect",
                    case 5 : bInsTblFormatNum = *(sal_Bool*)pValues[nProp].getValue();  break;  //"Input/NumberRecognition",
                    case 6 : bInsTblChangeNumFormat = *(sal_Bool*)pValues[nProp].getValue(); break;  //"Input/NumberFormatRecognition",
                    case 7 : bInsTblAlignNum = *(sal_Bool*)pValues[nProp].getValue(); break;  //"Input/Alignment"
                }
            }
        }
    }
}

SwMiscConfig::SwMiscConfig() :
    ConfigItem(C2U("Office.Writer"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    bDefaultFontsInCurrDocOnly(sal_False),
    bShowIndexPreview(sal_False),
    bGrfToGalleryAsLnk(sal_True),
    bNumAlignSize(sal_True),
    bSinglePrintJob(sal_False),
    bIsNameFromColumn(sal_True),
    bAskForMailMergeInPrint(sal_True),
    nMailingFormats(0)
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
            pNames[i] = rtl::OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

void SwMiscConfig::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

void SwMiscConfig::Commit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0 :
                pValues[nProp] <<= OUString(
                    SwModuleOptions::ConvertWordDelimiter(sWordDelimiter, sal_False));
            break;
            case 1 : pValues[nProp].setValue(&bDefaultFontsInCurrDocOnly, rType); break;
            case 2 : pValues[nProp].setValue(&bShowIndexPreview, rType) ;        break;
            case 3 : pValues[nProp].setValue(&bGrfToGalleryAsLnk, rType);        break;
            case 4 : pValues[nProp].setValue(&bNumAlignSize, rType);            break;
            case 5 : pValues[nProp].setValue(&bSinglePrintJob, rType);          break;
            case 6 : pValues[nProp] <<= nMailingFormats;             break;
            case 7 : pValues[nProp] <<= OUString(sNameFromColumn);  break;
            case 8 : pValues[nProp] <<= OUString(sMailingPath);     break;
            case 9 : pValues[nProp] <<= OUString(sMailName);        break;
            case 10: pValues[nProp].setValue(&bIsNameFromColumn, rType);break;
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
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        OUString sTmp;
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case 0 : pValues[nProp] >>= sTmp;
                        sWordDelimiter = SwModuleOptions::ConvertWordDelimiter(sTmp, sal_True);
                    break;
                    case 1 : bDefaultFontsInCurrDocOnly = *(sal_Bool*)pValues[nProp].getValue(); break;
                    case 2 : bShowIndexPreview = *(sal_Bool*)pValues[nProp].getValue(); break;
                    case 3 : bGrfToGalleryAsLnk = *(sal_Bool*)pValues[nProp].getValue(); break;
                    case 4 : bNumAlignSize = *(sal_Bool*)pValues[nProp].getValue(); break;
                    case 5 : bSinglePrintJob = *(sal_Bool*)pValues[nProp].getValue(); break;
                    case 6 : pValues[nProp] >>= nMailingFormats;              ; break;
                    case 7 : pValues[nProp] >>= sTmp; sNameFromColumn = sTmp; break;
                    case 8 : pValues[nProp] >>= sTmp; sMailingPath = sTmp;  break;
                    case 9 : pValues[nProp] >>= sTmp; sMailName = sTmp;     break;
                    case 10: bIsNameFromColumn = *(sal_Bool*)pValues[nProp].getValue(); break;
                    case 11: pValues[nProp] >>= bAskForMailMergeInPrint; break;
                }
            }
        }
    }
}

const Sequence<OUString>& SwCompareConfig::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        const int nCount = 4;
        aNames.realloc(nCount);
        static const char* aPropNames[] =
        {
            "Mode",							// 0
            "UseRSID",						// 1
            "IgnorePieces",				// 2
            "IgnoreLength"					// 3
        };
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

SwCompareConfig::SwCompareConfig() :
    ConfigItem(C2U("Office.Writer/Comparison"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE)
{
    eCmpMode = SVX_CMP_AUTO;
    bUseRsid = 0;
    bIgnorePieces = 0;
    nPieceLen = 1;

    Load();
}

SwCompareConfig::~SwCompareConfig()
{
}

void SwCompareConfig::Commit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

   const Type& rType = ::getBooleanCppuType();

    pValues[0] <<= (sal_Int32) eCmpMode;
    pValues[1].setValue(&bUseRsid, rType);
    pValues[2].setValue(&bIgnorePieces, rType);
    pValues[3] <<= (sal_Int32) nPieceLen;

    PutProperties(aNames, aValues);
}

void SwCompareConfig::Load()
{
    const Sequence<OUString>& aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                sal_Int32 nVal = 0;
                pValues[nProp] >>= nVal;

                switch(nProp)
                {
                    case 0 : eCmpMode = (SvxCompareMode) nVal; break;;
                    case 1 : bUseRsid = *(sal_Bool*)pValues[nProp].getValue(); break;
                    case 2 : bIgnorePieces = *(sal_Bool*)pValues[nProp].getValue(); break;
                    case 3 : nPieceLen = nVal; break;
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
