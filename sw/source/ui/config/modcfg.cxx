/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <hintids.hxx>
#include <comphelper/classids.hxx>
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
    m_aInsCapOptArr.push_back(pObj); 
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
        if(eType == OLE_CAP && pOleId)
        {
            bool bFound = false;
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
        if(pOpt->GetObjType() == OLE_CAP && &pOpt->GetOleId())
        {
            bool bFound = false;
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

OUString SwModuleOptions::ConvertWordDelimiter(const OUString& rDelim, sal_Bool bFromUI)
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
                            sReturn += OUString(nChar);
                        break;
                    }

                    default:    
                        sReturn += "\\";
                        i--;
                        break;
                }
            }
            else
                sReturn += OUString(c);
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
                        sReturn += OUString(c);
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
            "TextDisplay/Insert/Attribute",             
            "TextDisplay/Insert/Color",                 
            "TextDisplay/Delete/Attribute",             
            "TextDisplay/Delete/Color",                 
            "TextDisplay/ChangedAttribute/Attribute",   
            "TextDisplay/ChangedAttribute/Color",       
            "LinesChanged/Mark",                        
            "LinesChanged/Color"                        
        };
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

SwRevisionConfig::SwRevisionConfig() :
    ConfigItem("Office.Writer/Revision",
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

void SwRevisionConfig::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

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

static void lcl_ConvertCfgToAttr(sal_Int32 nVal, AuthorCharAttr& rAttr, bool bDelete = false)
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
                    case 2 : lcl_ConvertCfgToAttr(nVal, aDeletedAttr, true); break;
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
    INS_PROP_TABLE_REPEATHEADER,                        
    INS_PROP_TABLE_BORDER,                              
    INS_PROP_TABLE_SPLIT,                               
    INS_PROP_CAP_AUTOMATIC,                             
    INS_PROP_CAP_CAPTIONORDERNUMBERINGFIRST,            
    INS_PROP_CAP_OBJECT_TABLE_ENABLE,                   
    INS_PROP_CAP_OBJECT_TABLE_CATEGORY,                 
    INS_PROP_CAP_OBJECT_TABLE_NUMBERING,                
    INS_PROP_CAP_OBJECT_TABLE_NUMBERINGSEPARATOR,       
    INS_PROP_CAP_OBJECT_TABLE_CAPTIONTEXT,              
    INS_PROP_CAP_OBJECT_TABLE_DELIMITER,                
    INS_PROP_CAP_OBJECT_TABLE_LEVEL,                    
    INS_PROP_CAP_OBJECT_TABLE_POSITION,                 
    INS_PROP_CAP_OBJECT_TABLE_CHARACTERSTYLE,           
    INS_PROP_CAP_OBJECT_FRAME_ENABLE,                   
    INS_PROP_CAP_OBJECT_FRAME_CATEGORY,                 
    INS_PROP_CAP_OBJECT_FRAME_NUMBERING,                
    INS_PROP_CAP_OBJECT_FRAME_NUMBERINGSEPARATOR,       
    INS_PROP_CAP_OBJECT_FRAME_CAPTIONTEXT,              
    INS_PROP_CAP_OBJECT_FRAME_DELIMITER,                
    INS_PROP_CAP_OBJECT_FRAME_LEVEL,                    
    INS_PROP_CAP_OBJECT_FRAME_POSITION,                 
    INS_PROP_CAP_OBJECT_FRAME_CHARACTERSTYLE,           
    INS_PROP_CAP_OBJECT_GRAPHIC_ENABLE,                          
    INS_PROP_CAP_OBJECT_GRAPHIC_CATEGORY,               
    INS_PROP_CAP_OBJECT_GRAPHIC_NUMBERING,              
    INS_PROP_CAP_OBJECT_GRAPHIC_NUMBERINGSEPARATOR,     
    INS_PROP_CAP_OBJECT_GRAPHIC_CAPTIONTEXT,            
    INS_PROP_CAP_OBJECT_GRAPHIC_DELIMITER,              
    INS_PROP_CAP_OBJECT_GRAPHIC_LEVEL,                  
    INS_PROP_CAP_OBJECT_GRAPHIC_POSITION,               
    INS_PROP_CAP_OBJECT_GRAPHIC_CHARACTERSTYLE,         
    INS_PROP_CAP_OBJECT_GRAPHIC_APPLYATTRIBUTES,        
    INS_PROP_CAP_OBJECT_CALC_ENABLE,                             
    INS_PROP_CAP_OBJECT_CALC_CATEGORY,                  
    INS_PROP_CAP_OBJECT_CALC_NUMBERING,                 
    INS_PROP_CAP_OBJECT_CALC_NUMBERINGSEPARATOR,        
    INS_PROP_CAP_OBJECT_CALC_CAPTIONTEXT,               
    INS_PROP_CAP_OBJECT_CALC_DELIMITER,                 
    INS_PROP_CAP_OBJECT_CALC_LEVEL,                     
    INS_PROP_CAP_OBJECT_CALC_POSITION,                  
    INS_PROP_CAP_OBJECT_CALC_CHARACTERSTYLE,            
    INS_PROP_CAP_OBJECT_CALC_APPLYATTRIBUTES,           
    INS_PROP_CAP_OBJECT_IMPRESS_ENABLE,                          
    INS_PROP_CAP_OBJECT_IMPRESS_CATEGORY,               
    INS_PROP_CAP_OBJECT_IMPRESS_NUMBERING,              
    INS_PROP_CAP_OBJECT_IMPRESS_NUMBERINGSEPARATOR,     
    INS_PROP_CAP_OBJECT_IMPRESS_CAPTIONTEXT,            
    INS_PROP_CAP_OBJECT_IMPRESS_DELIMITER,              
    INS_PROP_CAP_OBJECT_IMPRESS_LEVEL,                  
    INS_PROP_CAP_OBJECT_IMPRESS_POSITION,               
    INS_PROP_CAP_OBJECT_IMPRESS_CHARACTERSTYLE,         
    INS_PROP_CAP_OBJECT_IMPRESS_APPLYATTRIBUTES,        
    INS_PROP_CAP_OBJECT_CHART_ENABLE,                            
    INS_PROP_CAP_OBJECT_CHART_CATEGORY,                 
    INS_PROP_CAP_OBJECT_CHART_NUMBERING,                
    INS_PROP_CAP_OBJECT_CHART_NUMBERINGSEPARATOR,       
    INS_PROP_CAP_OBJECT_CHART_CAPTIONTEXT,              
    INS_PROP_CAP_OBJECT_CHART_DELIMITER,                
    INS_PROP_CAP_OBJECT_CHART_LEVEL,                    
    INS_PROP_CAP_OBJECT_CHART_POSITION,                 
    INS_PROP_CAP_OBJECT_CHART_CHARACTERSTYLE,           
    INS_PROP_CAP_OBJECT_CHART_APPLYATTRIBUTES,          
    INS_PROP_CAP_OBJECT_FORMULA_ENABLE,                          
    INS_PROP_CAP_OBJECT_FORMULA_CATEGORY,               
    INS_PROP_CAP_OBJECT_FORMULA_NUMBERING,              
    INS_PROP_CAP_OBJECT_FORMULA_NUMBERINGSEPARATOR,     
    INS_PROP_CAP_OBJECT_FORMULA_CAPTIONTEXT,            
    INS_PROP_CAP_OBJECT_FORMULA_DELIMITER,              
    INS_PROP_CAP_OBJECT_FORMULA_LEVEL,                  
    INS_PROP_CAP_OBJECT_FORMULA_POSITION,               
    INS_PROP_CAP_OBJECT_FORMULA_CHARACTERSTYLE,         
    INS_PROP_CAP_OBJECT_FORMULA_APPLYATTRIBUTES,        
    INS_PROP_CAP_OBJECT_DRAW_ENABLE,                             
    INS_PROP_CAP_OBJECT_DRAW_CATEGORY,                  
    INS_PROP_CAP_OBJECT_DRAW_NUMBERING,                 
    INS_PROP_CAP_OBJECT_DRAW_NUMBERINGSEPARATOR,        
    INS_PROP_CAP_OBJECT_DRAW_CAPTIONTEXT,               
    INS_PROP_CAP_OBJECT_DRAW_DELIMITER,                 
    INS_PROP_CAP_OBJECT_DRAW_LEVEL,                     
    INS_PROP_CAP_OBJECT_DRAW_POSITION,                  
    INS_PROP_CAP_OBJECT_DRAW_CHARACTERSTYLE,            
    INS_PROP_CAP_OBJECT_DRAW_APPLYATTRIBUTES,           
    INS_PROP_CAP_OBJECT_OLEMISC_ENABLE,                          
    INS_PROP_CAP_OBJECT_OLEMISC_CATEGORY,               
    INS_PROP_CAP_OBJECT_OLEMISC_NUMBERING,              
    INS_PROP_CAP_OBJECT_OLEMISC_NUMBERINGSEPARATOR,     
    INS_PROP_CAP_OBJECT_OLEMISC_CAPTIONTEXT,            
    INS_PROP_CAP_OBJECT_OLEMISC_DELIMITER,              
    INS_PROP_CAP_OBJECT_OLEMISC_LEVEL,                  
    INS_PROP_CAP_OBJECT_OLEMISC_POSITION,               
    INS_PROP_CAP_OBJECT_OLEMISC_CHARACTERSTYLE,         
    INS_PROP_CAP_OBJECT_OLEMISC_APPLYATTRIBUTES        
};
const Sequence<OUString>& SwInsertConfig::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    static Sequence<OUString> aWebNames;
    if(!aNames.getLength())
    {
        static const char* aPropNames[] =
        {
            "Table/Header",                                                 
            "Table/RepeatHeader",                                           
            "Table/Border",                                                 
            "Table/Split",                                                  
            "Caption/Automatic",                                            
            "Caption/CaptionOrderNumberingFirst",                           
            "Caption/WriterObject/Table/Enable",                            
            "Caption/WriterObject/Table/Settings/Category",                 
            "Caption/WriterObject/Table/Settings/Numbering",                
            "Caption/WriterObject/Table/Settings/NumberingSeparator",       
            "Caption/WriterObject/Table/Settings/CaptionText",              
            "Caption/WriterObject/Table/Settings/Delimiter",                
            "Caption/WriterObject/Table/Settings/Level",                    
            "Caption/WriterObject/Table/Settings/Position",                 
            "Caption/WriterObject/Table/Settings/CharacterStyle",           
            "Caption/WriterObject/Frame/Enable",                            
            "Caption/WriterObject/Frame/Settings/Category",                 
            "Caption/WriterObject/Frame/Settings/Numbering",                
            "Caption/WriterObject/Frame/Settings/NumberingSeparator",       
            "Caption/WriterObject/Frame/Settings/CaptionText",              
            "Caption/WriterObject/Frame/Settings/Delimiter",                
            "Caption/WriterObject/Frame/Settings/Level",                    
            "Caption/WriterObject/Frame/Settings/Position",                 
            "Caption/WriterObject/Frame/Settings/CharacterStyle",           
            "Caption/WriterObject/Graphic/Enable",                          
            "Caption/WriterObject/Graphic/Settings/Category",               
            "Caption/WriterObject/Graphic/Settings/Numbering",              
            "Caption/WriterObject/Graphic/Settings/NumberingSeparator",     
            "Caption/WriterObject/Graphic/Settings/CaptionText",            
            "Caption/WriterObject/Graphic/Settings/Delimiter",              
            "Caption/WriterObject/Graphic/Settings/Level",                  
            "Caption/WriterObject/Graphic/Settings/Position",               
            "Caption/WriterObject/Graphic/Settings/CharacterStyle",         
            "Caption/WriterObject/Graphic/Settings/ApplyAttributes",        
            "Caption/OfficeObject/Calc/Enable",                             
            "Caption/OfficeObject/Calc/Settings/Category",                  
            "Caption/OfficeObject/Calc/Settings/Numbering",                 
            "Caption/OfficeObject/Calc/Settings/NumberingSeparator",        
            "Caption/OfficeObject/Calc/Settings/CaptionText",               
            "Caption/OfficeObject/Calc/Settings/Delimiter",                 
            "Caption/OfficeObject/Calc/Settings/Level",                     
            "Caption/OfficeObject/Calc/Settings/Position",                  
            "Caption/OfficeObject/Calc/Settings/CharacterStyle",            
            "Caption/OfficeObject/Calc/Settings/ApplyAttributes",           
            "Caption/OfficeObject/Impress/Enable",                          
            "Caption/OfficeObject/Impress/Settings/Category",               
            "Caption/OfficeObject/Impress/Settings/Numbering",              
            "Caption/OfficeObject/Impress/Settings/NumberingSeparator",     
            "Caption/OfficeObject/Impress/Settings/CaptionText",            
            "Caption/OfficeObject/Impress/Settings/Delimiter",              
            "Caption/OfficeObject/Impress/Settings/Level",                  
            "Caption/OfficeObject/Impress/Settings/Position",               
            "Caption/OfficeObject/Impress/Settings/CharacterStyle",         
            "Caption/OfficeObject/Impress/Settings/ApplyAttributes",        
            "Caption/OfficeObject/Chart/Enable",                            
            "Caption/OfficeObject/Chart/Settings/Category",                 
            "Caption/OfficeObject/Chart/Settings/Numbering",                
            "Caption/OfficeObject/Chart/Settings/NumberingSeparator",       
            "Caption/OfficeObject/Chart/Settings/CaptionText",              
            "Caption/OfficeObject/Chart/Settings/Delimiter",                
            "Caption/OfficeObject/Chart/Settings/Level",                    
            "Caption/OfficeObject/Chart/Settings/Position",                 
            "Caption/OfficeObject/Chart/Settings/CharacterStyle",           
            "Caption/OfficeObject/Chart/Settings/ApplyAttributes",          
            "Caption/OfficeObject/Formula/Enable",                          
            "Caption/OfficeObject/Formula/Settings/Category",               
            "Caption/OfficeObject/Formula/Settings/Numbering",              
            "Caption/OfficeObject/Formula/Settings/NumberingSeparator",     
            "Caption/OfficeObject/Formula/Settings/CaptionText",            
            "Caption/OfficeObject/Formula/Settings/Delimiter",              
            "Caption/OfficeObject/Formula/Settings/Level",                  
            "Caption/OfficeObject/Formula/Settings/Position",               
            "Caption/OfficeObject/Formula/Settings/CharacterStyle",         
            "Caption/OfficeObject/Formula/Settings/ApplyAttributes",        
            "Caption/OfficeObject/Draw/Enable",                             
            "Caption/OfficeObject/Draw/Settings/Category",                  
            "Caption/OfficeObject/Draw/Settings/Numbering",                 
            "Caption/OfficeObject/Draw/Settings/NumberingSeparator",        
            "Caption/OfficeObject/Draw/Settings/CaptionText",               
            "Caption/OfficeObject/Draw/Settings/Delimiter",                 
            "Caption/OfficeObject/Draw/Settings/Level",                     
            "Caption/OfficeObject/Draw/Settings/Position",                  
            "Caption/OfficeObject/Draw/Settings/CharacterStyle",            
            "Caption/OfficeObject/Draw/Settings/ApplyAttributes",           
            "Caption/OfficeObject/OLEMisc/Enable",                          
            "Caption/OfficeObject/OLEMisc/Settings/Category",               
            "Caption/OfficeObject/OLEMisc/Settings/Numbering",              
            "Caption/OfficeObject/OLEMisc/Settings/NumberingSeparator",     
            "Caption/OfficeObject/OLEMisc/Settings/CaptionText",            
            "Caption/OfficeObject/OLEMisc/Settings/Delimiter",              
            "Caption/OfficeObject/OLEMisc/Settings/Level",                  
            "Caption/OfficeObject/OLEMisc/Settings/Position",               
            "Caption/OfficeObject/OLEMisc/Settings/CharacterStyle",         
            "Caption/OfficeObject/OLEMisc/Settings/ApplyAttributes"         
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

SwInsertConfig::SwInsertConfig(sal_Bool bWeb) :
    ConfigItem(bWeb ? OUString("Office.WriterWeb/Insert") : OUString("Office.Writer/Insert"),
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
        break;
        case 1: pValues[nProp] <<= OUString(rOpt.GetCategory()); break;
        case 2: pValues[nProp] <<= (sal_Int32)rOpt.GetNumType(); break;
        case 3: pValues[nProp] <<= rOpt.GetNumSeparator(); break;
        case 4: pValues[nProp] <<= OUString(rOpt.GetCaption());  break;
        case 5: pValues[nProp] <<= OUString(rOpt.GetSeparator());break;
        case 6: pValues[nProp] <<= (sal_Int32)rOpt.GetLevel();   break;
        case 7: pValues[nProp] <<= (sal_Int32)rOpt.GetPos();     break;
        case 8: pValues[nProp] <<= (OUString&)rOpt.GetCharacterStyle(); break; 
        case 9: pValues[nProp] <<= rOpt.CopyAttributes(); break; 
    }
}

void SwInsertConfig::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

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
            break;
            case INS_PROP_TABLE_REPEATHEADER:
            {
                sal_Bool bVal = (aInsTblOpts.mnRowsToRepeat>0); pValues[nProp].setValue(&bVal, rType);
            }
            break;
            case INS_PROP_TABLE_BORDER:
            {
                sal_Bool bVal = 0 !=(aInsTblOpts.mnInsMode & tabopts::DEFAULT_BORDER ); pValues[nProp].setValue(&bVal, rType);
            }
            break;
            case INS_PROP_TABLE_SPLIT:
            {
                sal_Bool bVal = 0 !=(aInsTblOpts.mnInsMode & tabopts::SPLIT_LAYOUT); pValues[nProp].setValue(&bVal, rType);
            }
            break;
            case INS_PROP_CAP_AUTOMATIC: pValues[nProp].setValue(&bInsWithCaption, rType);break;
            case INS_PROP_CAP_CAPTIONORDERNUMBERINGFIRST:
                pValues[nProp] <<= bCaptionOrderNumberingFirst;
            break;

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
        break;
        case 1:
        {
            OUString sTemp; pValues[nProp] >>= sTemp;
            rOpt.SetCategory(sTemp);
        }
        break;
        case 2:
        {
            sal_Int32 nTemp = 0;
            pValues[nProp] >>= nTemp;
            rOpt.SetNumType(sal::static_int_cast< sal_uInt16, sal_Int32>(nTemp));
        }
        break;
        case 3:
        {
            OUString sTemp; pValues[nProp] >>= sTemp;
            rOpt.SetNumSeparator(sTemp);
        }
        break;
        case 4:
        {
            OUString sTemp; pValues[nProp] >>= sTemp;
            rOpt.SetCaption(sTemp);
        }
        break;
        case 5:
        {
            OUString sTemp;
            if(pValues[nProp] >>= sTemp)
                rOpt.SetSeparator(sTemp);
        }
        break;
        case 6:
        {
            sal_Int32 nTemp = 0;
            pValues[nProp] >>= nTemp;
            rOpt.SetLevel(sal::static_int_cast< sal_uInt16, sal_Int32>(nTemp));
        }
        break;
        case 7:
        {
            sal_Int32 nTemp = 0;
            pValues[nProp] >>= nTemp;
            rOpt.SetPos(sal::static_int_cast< sal_uInt16, sal_Int32>(nTemp));
        }
        break;
        case 8 : 
        {
            OUString sTemp; pValues[nProp] >>= sTemp;
            rOpt.SetCharacterStyle( sTemp );
        }
        break;
        case 9 : 
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
                    break;
                    case INS_PROP_TABLE_REPEATHEADER:
                    {
                        aInsTblOpts.mnRowsToRepeat = bBool? 1 : 0;

                    }
                    break;
                    case INS_PROP_TABLE_BORDER:
                    {
                        if(bBool)
                            nInsTblFlags|= tabopts::DEFAULT_BORDER;
                    }
                    break;
                    case INS_PROP_TABLE_SPLIT:
                    {
                        if(bBool)
                            nInsTblFlags|= tabopts::SPLIT_LAYOUT;
                    }
                    break;
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
        "Shift/Row",                    
        "Shift/Column",                 
        "Insert/Row",                   
        "Insert/Column",                
        "Change/Effect",                
        "Input/NumberRecognition",      
        "Input/NumberFormatRecognition",
        "Input/Alignment"               
    };
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}

SwTableConfig::SwTableConfig(sal_Bool bWeb) :
    ConfigItem(bWeb ? OUString("Office.WriterWeb/Table") : OUString("Office.Writer/Table"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE)
{
    Load();
}

SwTableConfig::~SwTableConfig()
{
}

void SwTableConfig::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

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
            case 0 : pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(nTblHMove); break;   
            case 1 : pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(nTblVMove); break;     
            case 2 : pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(nTblHInsert); break;   
            case 3 : pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100_UNSIGNED(nTblVInsert); break;   
            case 4 : pValues[nProp] <<= (sal_Int32)eTblChgMode; break;   
            case 5 : pValues[nProp].setValue(&bInsTblFormatNum, rType); break;  
            case 6 : pValues[nProp].setValue(&bInsTblChangeNumFormat, rType); break;  
            case 7 : pValues[nProp].setValue(&bInsTblAlignNum, rType); break;  
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
                    case 0 : pValues[nProp] >>= nTemp; nTblHMove = (sal_uInt16)MM100_TO_TWIP(nTemp); break;  
                    case 1 : pValues[nProp] >>= nTemp; nTblVMove = (sal_uInt16)MM100_TO_TWIP(nTemp); break;     
                    case 2 : pValues[nProp] >>= nTemp; nTblHInsert = (sal_uInt16)MM100_TO_TWIP(nTemp); break;   
                    case 3 : pValues[nProp] >>= nTemp; nTblVInsert = (sal_uInt16)MM100_TO_TWIP(nTemp); break;   
                    case 4 : pValues[nProp] >>= nTemp; eTblChgMode = (TblChgMode)nTemp; break;   
                    case 5 : bInsTblFormatNum = *(sal_Bool*)pValues[nProp].getValue();  break;  
                    case 6 : bInsTblChangeNumFormat = *(sal_Bool*)pValues[nProp].getValue(); break;  
                    case 7 : bInsTblAlignNum = *(sal_Bool*)pValues[nProp].getValue(); break;  
                }
            }
        }
    }
}

SwMiscConfig::SwMiscConfig() :
    ConfigItem("Office.Writer",
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
            "Statistics/WordNumber/Delimiter",          
            "DefaultFont/Document",                     
            "Index/ShowPreview",                        
            "Misc/GraphicToGalleryAsLink",              
            "Numbering/Graphic/KeepRatio",              
            "FormLetter/PrintOutput/SinglePrintJobs",   
            "FormLetter/MailingOutput/Format",          
            "FormLetter/FileOutput/FileName/FromDatabaseField",  
            "FormLetter/FileOutput/Path",               
            "FormLetter/FileOutput/FileName/FromManualSetting",   
            "FormLetter/FileOutput/FileName/Generation",
            "FormLetter/PrintOutput/AskForMerge"        
        };
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

void SwMiscConfig::Notify( const ::com::sun::star::uno::Sequence< OUString >& ) {}

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
                pValues[nProp] <<=
                    SwModuleOptions::ConvertWordDelimiter(sWordDelimiter, sal_False);
            break;
            case 1 : pValues[nProp].setValue(&bDefaultFontsInCurrDocOnly, rType); break;
            case 2 : pValues[nProp].setValue(&bShowIndexPreview, rType) ;        break;
            case 3 : pValues[nProp].setValue(&bGrfToGalleryAsLnk, rType);        break;
            case 4 : pValues[nProp].setValue(&bNumAlignSize, rType);            break;
            case 5 : pValues[nProp].setValue(&bSinglePrintJob, rType);          break;
            case 6 : pValues[nProp] <<= nMailingFormats;             break;
            case 7 : pValues[nProp] <<= sNameFromColumn;  break;
            case 8 : pValues[nProp] <<= sMailingPath;     break;
            case 9 : pValues[nProp] <<= sMailName;        break;
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
            "Mode",							
            "UseRSID",						
            "IgnorePieces",				
            "IgnoreLength"					
        };
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

SwCompareConfig::SwCompareConfig() :
    ConfigItem("Office.Writer/Comparison",
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
