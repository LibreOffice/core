/*************************************************************************
 *
 *  $RCSfile: modcfg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: os $ $Date: 2000-10-19 13:26:48 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _MAILENUM_HXX //autogen
#include <goodies/mailenum.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif
#ifndef _WORDSEL_HXX
#include <svtools/wordsel.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _FACTORY_HXX //autogen
#include <so3/factory.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _ITABENUM_HXX
#include <itabenum.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#ifndef _FLDUPDE_HXX
#include <fldupde.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

#define C2U(cChar) OUString::createFromAscii(cChar)
#define GLOB_NAME_CALC      0
#define GLOB_NAME_IMPRESS   1
#define GLOB_NAME_DRAW      2
#define GLOB_NAME_MATH      3
#define GLOB_NAME_CHART     4

SV_IMPL_PTRARR_SORT(InsCapOptArr, InsCaptionOptPtr)
/* -----------------03.11.98 13:46-------------------
 *
 * --------------------------------------------------*/
InsCaptionOpt* InsCaptionOptArr::Find(const SwCapObjType eType, const SvGlobalName *pOleId) const
{
    for (USHORT i = 0; i < Count(); i++ )
    {
        InsCaptionOpt* pObj = GetObject(i);
        if (pObj->GetObjType() == eType &&
            (eType != OLE_CAP ||
             (pOleId &&
              (pObj->GetOleId() == *pOleId ||
               SvFactory::GetAutoConvertTo(pObj->GetOleId()) == *pOleId))))
            return pObj;
    }

    return 0;
}

/* -----------------03.11.98 15:05-------------------
 *
 * --------------------------------------------------*/

const InsCaptionOpt* SwModuleOptions::GetCapOption(
    BOOL bHTML, const SwCapObjType eType, const SvGlobalName *pOleId)
{
    if(bHTML)
    {
        DBG_ERROR("no caption option in sw/web!")
        return 0;
    }
    else
    {
        sal_Bool bFound = FALSE;
        if(eType == OLE_CAP && pOleId)
        {
            for( USHORT nId = 0; nId <= GLOB_NAME_CHART; nId++)
                bFound = *pOleId == aInsertConfig.aGlobalNames[nId  ];
            if(!bFound)
                return aInsertConfig.pOLEMiscOpt;
        }
        return aInsertConfig.pCapOptions->Find(eType, pOleId);
    }
}

/* -----------------03.11.98 15:05-------------------
 *
 * --------------------------------------------------*/

BOOL SwModuleOptions::SetCapOption(BOOL bHTML, const InsCaptionOpt* pOpt)
{
    BOOL bRet = FALSE;

    if(bHTML)
    {
        DBG_ERROR("no caption option in sw/web!")
    }
    else if (pOpt)
    {
        sal_Bool bFound = FALSE;
        if(pOpt->GetObjType() == OLE_CAP && &pOpt->GetOleId())
        {
            for( USHORT nId = 0; nId <= GLOB_NAME_CHART; nId++)
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
        bRet = TRUE;
    }

    return bRet;
}
/*-----------------13.01.97 12.44-------------------

--------------------------------------------------*/
SwModuleOptions::SwModuleOptions() :
    aRevisionConfig(),
    aInsertConfig(FALSE),
    aWebInsertConfig(TRUE),
    aTableConfig(FALSE),
    aWebTableConfig(TRUE),
    bHideFieldTips(FALSE)
{
    aRevisionConfig.Load();
}
/* -----------------------------10.10.00 16:22--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> SwRevisionConfig::GetPropertyNames()
{
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
    const int nCount = 8;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}
/*-- 10.10.00 16:22:22---------------------------------------------------

  -----------------------------------------------------------------------*/
SwRevisionConfig::SwRevisionConfig() :
    ConfigItem(C2U("Office.Writer/Revision"))
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
/*-- 10.10.00 16:22:23---------------------------------------------------

  -----------------------------------------------------------------------*/
SwRevisionConfig::~SwRevisionConfig()
{
}
/*-- 10.10.00 16:22:56---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwRevisionConfig::Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames)
{
    Load();
}
/*-- 10.10.00 16:22:56---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwRevisionConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Int32 nVal = -1;
        switch(nProp)
        {
            case 0 : nVal = aInsertAttr.nAttr   ; break;
            case 1 : nVal = aInsertAttr.nColor  ; break;
            case 2 : nVal = aDeletedAttr.nAttr  ; break;
            case 3 : nVal = aDeletedAttr.nColor ; break;
            case 4 : nVal = aFormatAttr.nAttr   ; break;
            case 5 : nVal = aFormatAttr.nColor  ; break;
            case 6 : nVal = nMarkAlign          ; break;
            case 7 : nVal = aMarkColor.GetColor(); break;
        }
        pValues[nProp] <<= nVal;
    }
    PutProperties(aNames, aValues);
}
/*-- 10.10.00 16:22:56---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwRevisionConfig::Load()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                sal_Int32 nVal;
                pValues[nProp] >>= nVal;
                switch(nProp)
                {
                    case 0 : aInsertAttr.nAttr      = nVal; break;
                    case 1 : aInsertAttr.nColor     = nVal; break;
                    case 2 : aDeletedAttr.nAttr     = nVal; break;
                    case 3 : aDeletedAttr.nColor    = nVal; break;
                    case 4 : aFormatAttr.nAttr      = nVal; break;
                    case 5 : aFormatAttr.nColor     = nVal; break;
                    case 6 : nMarkAlign             = nVal; break;
                    case 7 : aMarkColor.SetColor(nVal); break;
                }
            }
        }
    }
}
/* -----------------------------10.10.00 16:22--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> SwInsertConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Table/Header",                                 // 0
        "Table/RepeatHeader",                           // 1
        "Table/Border",                                 // 2
        "Table/Split",                                  // 3 from here not in writer/web
        "Caption/Automatic",                            // 4
        "Caption/StarWriterObject/Table/Enable",        // 5
        "Caption/StarWriterObject/Table/Settings/Category",     // 6
        "Caption/StarWriterObject/Table/Settings/Numbering",    // 7
        "Caption/StarWriterObject/Table/Settings/CaptionText",  // 8
        "Caption/StarWriterObject/Table/Settings/Delimiter",    // 9
        "Caption/StarWriterObject/Table/Settings/Level",        //10
        "Caption/StarWriterObject/Table/Settings/Position",     //11
        "Caption/StarWriterObject/Frame/Enable",                //12
        "Caption/StarWriterObject/Frame/Settings/Category",     //13
        "Caption/StarWriterObject/Frame/Settings/Numbering",    //14
        "Caption/StarWriterObject/Frame/Settings/CaptionText",  //15
        "Caption/StarWriterObject/Frame/Settings/Delimiter",    //16
        "Caption/StarWriterObject/Frame/Settings/Level",        //17
        "Caption/StarWriterObject/Frame/Settings/Position",     //18
        "Caption/StarWriterObject/Graphic/Enable",              //19
        "Caption/StarWriterObject/Graphic/Settings/Category",   //20
        "Caption/StarWriterObject/Graphic/Settings/Numbering",  //21
        "Caption/StarWriterObject/Graphic/Settings/CaptionText",//22
        "Caption/StarWriterObject/Graphic/Settings/Delimiter",  //23
        "Caption/StarWriterObject/Graphic/Settings/Level",      //24
        "Caption/StarOfficeObject/Calc/Enable",                 //25
        "Caption/StarOfficeObject/Calc/Settings/Category",      //26
        "Caption/StarOfficeObject/Calc/Settings/Numbering",     //27
        "Caption/StarOfficeObject/Calc/Settings/CaptionText",   //28
        "Caption/StarOfficeObject/Calc/Settings/Delimiter",     //29
        "Caption/StarOfficeObject/Calc/Settings/Level",         //30
        "Caption/StarOfficeObject/Calc/Settings/Position",      //31
        "Caption/StarOfficeObject/Impress/Enable",              //32
        "Caption/StarOfficeObject/Impress/Settings/Category",   //33
        "Caption/StarOfficeObject/Impress/Settings/Numbering",  //34
        "Caption/StarOfficeObject/Impress/Settings/CaptionText",//35
        "Caption/StarOfficeObject/Impress/Settings/Delimiter",  //36
        "Caption/StarOfficeObject/Impress/Settings/Level",      //37
        "Caption/StarOfficeObject/Impress/Settings/Position",   //38
        "Caption/StarOfficeObject/Chart/Enable",                //39
        "Caption/StarOfficeObject/Chart/Settings/Category",     //40
        "Caption/StarOfficeObject/Chart/Settings/Numbering",    //41
        "Caption/StarOfficeObject/Chart/Settings/CaptionText",  //42
        "Caption/StarOfficeObject/Chart/Settings/Delimiter",    //43
        "Caption/StarOfficeObject/Chart/Settings/Level",        //44
        "Caption/StarOfficeObject/Chart/Settings/Position",     //45
        "Caption/StarOfficeObject/Formula/Enable",              //46
        "Caption/StarOfficeObject/Formula/Settings/Category",   //47
        "Caption/StarOfficeObject/Formula/Settings/Numbering",  //48
        "Caption/StarOfficeObject/Formula/Settings/CaptionText",//49
        "Caption/StarOfficeObject/Formula/Settings/Delimiter",  //50
        "Caption/StarOfficeObject/Formula/Settings/Level",      //51
        "Caption/StarOfficeObject/Formula/Settings/Position",   //52
        "Caption/StarOfficeObject/Draw/Enable",                 //53
        "Caption/StarOfficeObject/Draw/Settings/Category",      //54
        "Caption/StarOfficeObject/Draw/Settings/Numbering",     //55
        "Caption/StarOfficeObject/Draw/Settings/CaptionText",   //56
        "Caption/StarOfficeObject/Draw/Settings/Delimiter",     //57
        "Caption/StarOfficeObject/Draw/Settings/Level",         //58
        "Caption/StarOfficeObject/Draw/Settings/Position",      //59
        "Caption/StarOfficeObject/OLEMisc/Enable",              //60
        "Caption/StarOfficeObject/OLEMisc/Settings/Category",   //61
        "Caption/StarOfficeObject/OLEMisc/Settings/Numbering",  //62
        "Caption/StarOfficeObject/OLEMisc/Settings/CaptionText",//63
        "Caption/StarOfficeObject/OLEMisc/Settings/Delimiter",  //64
        "Caption/StarOfficeObject/OLEMisc/Settings/Level",      //65
        "Caption/StarOfficeObject/OLEMisc/Settings/Position"    //66
    };
    const int nCount = bIsWeb ? 3: 67;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}
/*-- 10.10.00 16:22:22---------------------------------------------------

  -----------------------------------------------------------------------*/
SwInsertConfig::SwInsertConfig(sal_Bool bWeb) :
    ConfigItem(bWeb ? C2U("Office.WriterWeb/Insert") : C2U("Office.Writer/Insert")),
    bIsWeb(bWeb),
    pCapOptions(0),
    pOLEMiscOpt(0)
{
    aGlobalNames[GLOB_NAME_CALC   ] = SvGlobalName(SO3_SC_CLASSID);;
    aGlobalNames[GLOB_NAME_IMPRESS] = SvGlobalName(SO3_SIMPRESS_CLASSID);
    aGlobalNames[GLOB_NAME_DRAW   ] = SvGlobalName(SO3_SDRAW_CLASSID);
    aGlobalNames[GLOB_NAME_MATH   ] = SvGlobalName(SO3_SM_CLASSID);
    aGlobalNames[GLOB_NAME_CHART  ] = SvGlobalName(SO3_SCH_CLASSID);
    if(!bIsWeb)
        pCapOptions = new InsCaptionOptArr;
    Load();
}
/*-- 10.10.00 16:22:23---------------------------------------------------

  -----------------------------------------------------------------------*/
SwInsertConfig::~SwInsertConfig()
{
    delete pCapOptions;
    delete pOLEMiscOpt;
}
/*-- 10.10.00 16:22:56---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwInsertConfig::Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames)
{
    Load();
}
/*-- 10.10.00 16:22:56---------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_WriteOpt(const InsCaptionOpt& rOpt, Any* pValues, sal_Int32 nProp, sal_Int32 nOffset)
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
        case 3: pValues[nProp] <<= OUString(rOpt.GetCaption());  break;//CaptionText",
        case 4: pValues[nProp] <<= OUString(rOpt.GetSeparator());break;//Delimiter",
        case 5: pValues[nProp] <<= (sal_Int32)rOpt.GetLevel();   break;//Level",
        case 6: pValues[nProp] <<= (sal_Int32)rOpt.GetPos();     break;//Position",
    }
}
//-----------------------------------------------------------------------------
void SwInsertConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Int32 nVal = -1;
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
            case  0:
            {
                sal_Bool bVal = 0 !=(nInsTblFlags&HEADLINE); pValues[nProp].setValue(&bVal, rType);
            }
            break;//"Table/Header",
            case  1:
            {
                sal_Bool bVal = 0 !=(nInsTblFlags&REPEAT); pValues[nProp].setValue(&bVal, rType);
            }
            break;//"Table/RepeatHeader",
            case  2:
            {
                sal_Bool bVal = 0 !=(nInsTblFlags&DEFAULT_BORDER ); pValues[nProp].setValue(&bVal, rType);
            }
            break;//"Table/Border",
            case  3:
            {
                sal_Bool bVal = 0 !=(nInsTblFlags&SPLIT_LAYOUT); pValues[nProp].setValue(&bVal, rType);
            }
            break;//"Table/Split",
            case  4: pValues[nProp].setValue(&bInsWithCaption, rType);break;//"Caption/Automatic",

            case  5: case  6: case  7: case  8: case  9: case 10: case 11: //"Caption/StarWriterObject/Table/Position",
                    if(pWriterTableOpt)
                        lcl_WriteOpt(*pWriterTableOpt, pValues, nProp, nProp - 5);
            break;
            case 12: case 13: case 14: case 15: case 16: case 17: case 18:
                    if(pWriterFrameOpt)
                        lcl_WriteOpt(*pWriterFrameOpt, pValues, nProp, nProp - 12);
            case 19: case 20: case 21: case 22: case 23: case 24:
                    if(pWriterGraphicOpt)
                        lcl_WriteOpt(*pWriterGraphicOpt, pValues, nProp, nProp - 19);
                    break;
            case 25: case 26: case 27: case 28: case 29: case 30: case 31:
                    if(pOLECalcOpt)
                        lcl_WriteOpt(*pOLECalcOpt, pValues, nProp, nProp - 25);
            break;
            case 32: case 33: case 34: case 35: case 36: case 37: case 38:
                    if(pOLEImpressOpt)
                        lcl_WriteOpt(*pOLEImpressOpt, pValues, nProp, nProp - 32);
            break;
            case 39: case 40: case 41: case 42: case 43: case 44: case 45:
                    if(pOLEChartOpt)
                        lcl_WriteOpt(*pOLEChartOpt, pValues, nProp, nProp - 39);
            break;
            case 46: case 47: case 48: case 49: case 50: case 51: case 52:
                    if(pOLEFormulaOpt)
                        lcl_WriteOpt(*pOLEFormulaOpt, pValues, nProp, nProp - 46);
            break;
            case 53: case 54:   case 55: case 56: case 57: case 58: case 59:
                    if(pOLEDrawOpt)
                        lcl_WriteOpt(*pOLEDrawOpt, pValues, nProp, nProp - 53);
            break;
            case 60: case 61: case 62: case 63: case 64: case 65: case 66:
                    if(pOLEMiscOpt)
                        lcl_WriteOpt(*pOLEMiscOpt, pValues, nProp, nProp - 60);
            break;

        }
    }
    PutProperties(aNames, aValues);
}
/*-- 10.10.00 16:22:56---------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_ReadOpt(InsCaptionOpt& rOpt, const Any* pValues, sal_Int32 nProp, sal_Int32 nOffset)
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
            sal_Int32 nTemp;  pValues[nProp] >>= nTemp;
            rOpt.SetNumType(nTemp);
        }
        break;//Numbering",
        case 3:
        {
            OUString sTemp; pValues[nProp] >>= sTemp;
            rOpt.SetCaption(sTemp);
        }
        break;//CaptionText",
        case 4:
        {
            OUString sTemp; pValues[nProp] >>= sTemp;
            if(sTemp.getLength())
                rOpt.SetSeparator(sTemp[0]);
        }
        break;//Delimiter",
        case 5:
        {
            sal_Int32 nTemp;  pValues[nProp] >>= nTemp;
            rOpt.SetLevel(nTemp);
        }
        break;//Level",
        case 6:
        {
            sal_Int32 nTemp;  pValues[nProp] >>= nTemp;
            rOpt.SetPos(nTemp);
        }
        break;//Position",
    }
}
//-----------------------------------------------------------------------------
void SwInsertConfig::Load()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
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
        else if(aNames.getLength() > 2)
            return;

        nInsTblFlags = 0;
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0:
                    {
                        sal_Bool bVal = *(sal_Bool*)pValues[nProp].getValue();
                        if(bVal)
                            nInsTblFlags|= HEADLINE;
                    }
                    break;//"Table/Header",
                    case  1:
                    {
                        sal_Bool bVal = *(sal_Bool*)pValues[nProp].getValue();
                        if(bVal)
                            nInsTblFlags|= REPEAT;
                    }
                    break;//"Table/RepeatHeader",
                    case  2:
                    {
                        sal_Bool bVal = *(sal_Bool*)pValues[nProp].getValue();
                        if(bVal)
                            nInsTblFlags|= DEFAULT_BORDER;
                    }
                    break;//"Table/Border",
                    case  3:
                    {
                        sal_Bool bVal = *(sal_Bool*)pValues[nProp].getValue();
                        if(bVal)
                            nInsTblFlags|= SPLIT_LAYOUT;
                    }
                    break;//"Table/Split",
                    case  5: case  6: case  7: case  8: case  9: case 10: case 11:
                        if(!pWriterTableOpt)
                        {
                            pWriterTableOpt = new InsCaptionOpt(TABLE_CAP);
                            pCapOptions->Insert(pWriterTableOpt);
                        }
                        lcl_ReadOpt(*pWriterTableOpt, pValues, nProp, nProp - 5);
                    break;
                    case 12: case 13: case 14: case 15: case 16: case 17: case 18:
                        if(!pWriterFrameOpt)
                        {
                            pWriterFrameOpt = new InsCaptionOpt(FRAME_CAP);
                            pCapOptions->Insert(pWriterFrameOpt);
                        }
                        lcl_ReadOpt(*pWriterFrameOpt, pValues, nProp, nProp - 12);
                    case 19: case 20: case 21:case 22: case 23: case 24:
                        if(!pWriterGraphicOpt)
                        {
                            pWriterGraphicOpt = new InsCaptionOpt(GRAPHIC_CAP);
                            pCapOptions->Insert(pWriterGraphicOpt);
                        }
                        lcl_ReadOpt(*pWriterGraphicOpt, pValues, nProp, nProp - 19);
                        break;
                    case 25: case 26: case 27: case 28: case 29: case 30: case 31:
                        if(!pOLECalcOpt)
                        {
                            pOLECalcOpt = new InsCaptionOpt(OLE_CAP, &aGlobalNames[GLOB_NAME_CALC]);
                            pCapOptions->Insert(pOLECalcOpt);
                        }
                        lcl_ReadOpt(*pOLECalcOpt, pValues, nProp, nProp - 25);
                    break;
                    case 32: case 33: case 34: case 35: case 36: case 37: case 38:
                        if(!pOLEImpressOpt)
                        {
                            pOLEImpressOpt = new InsCaptionOpt(OLE_CAP, &aGlobalNames[GLOB_NAME_IMPRESS]);
                            pCapOptions->Insert(pOLEImpressOpt);
                        }
                        lcl_ReadOpt(*pOLEImpressOpt, pValues, nProp, nProp - 32);
                    break;
                    case 39: case 40: case 41: case 42: case 43: case 44: case 45:
                        if(!pOLEChartOpt)
                        {
                            pOLEChartOpt = new InsCaptionOpt(OLE_CAP, &aGlobalNames[GLOB_NAME_CHART]);
                            pCapOptions->Insert(pOLEChartOpt);
                        }
                        lcl_ReadOpt(*pOLEChartOpt, pValues, nProp, nProp - 39);
                    break;
                    case 46: case 47: case 48: case 49: case 50: case 51: case 52:
                        if(!pOLEFormulaOpt)
                        {
                            pOLEFormulaOpt = new InsCaptionOpt(OLE_CAP, &aGlobalNames[GLOB_NAME_MATH]);
                            pCapOptions->Insert(pOLEFormulaOpt);
                        }
                        lcl_ReadOpt(*pOLEFormulaOpt, pValues, nProp, nProp - 46);
                    break;
                    case 53: case 54:   case 55: case 56: case 57: case 58: case 59:
                        if(!pOLEDrawOpt)
                        {
                            pOLEDrawOpt = new InsCaptionOpt(OLE_CAP, &aGlobalNames[GLOB_NAME_DRAW]);
                            pCapOptions->Insert(pOLEDrawOpt);
                        }
                        lcl_ReadOpt(*pOLEDrawOpt, pValues, nProp, nProp - 653);
                    break;
                    case 60: case 61: case 62: case 63: case 64: case 65: case 66:
                        if(!pOLEMiscOpt)
                        {
                            pOLEMiscOpt = new InsCaptionOpt(OLE_CAP);
                        }
                        lcl_ReadOpt(*pOLEMiscOpt, pValues, nProp, nProp - 60);
                    break;

                }
            }
        }
    }
}
/* -----------------------------10.10.00 16:22--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> SwTableConfig::GetPropertyNames()
{
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
    const int nCount = 8;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}
/*-- 10.10.00 16:22:22---------------------------------------------------

  -----------------------------------------------------------------------*/
SwTableConfig::SwTableConfig(sal_Bool bWeb) :
    ConfigItem(bWeb ? C2U("Office.WriterWeb/Table") : C2U("Office.Writer/Table"))
{
    Load();
}
/*-- 10.10.00 16:22:23---------------------------------------------------

  -----------------------------------------------------------------------*/
SwTableConfig::~SwTableConfig()
{
}
/*-- 10.10.00 16:22:56---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwTableConfig::Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames)
{
    Load();
}
/*-- 10.10.00 16:22:56---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwTableConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case 0 : pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100(nTblHMove); break;  //"Shift/Row",
            case 1 : pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100(nTblVMove); break;     //"Shift/Column",
            case 2 : pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100(nTblHInsert); break;   //"Insert/Row",
            case 3 : pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100(nTblVInsert); break;   //"Insert/Column",
            case 4 : pValues[nProp] <<= (sal_Int32)eTblChgMode; break;   //"Change/Effect",
            case 5 : pValues[nProp].setValue(&bInsTblFormatNum, rType); break;  //"Input/NumberRecognition",
            case 6 : pValues[nProp].setValue(&bInsTblChangeNumFormat, rType); break;  //"Input/NumberFormatRecognition",
            case 7 : pValues[nProp].setValue(&bInsTblAlignNum, rType); break;  //"Input/Alignment"
        }
    }
    PutProperties(aNames, aValues);
}
/*-- 10.10.00 16:22:56---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwTableConfig::Load()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    EnableNotification(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                sal_Int32 nTemp;
                switch(nProp)
                {
                    case 0 : pValues[nProp] >>= nTemp; nTblHMove = (USHORT)MM100_TO_TWIP(nTemp); break;  //"Shift/Row",
                    case 1 : pValues[nProp] >>= nTemp; nTblVMove = (USHORT)MM100_TO_TWIP(nTemp); break;     //"Shift/Column",
                    case 2 : pValues[nProp] >>= nTemp; nTblHInsert = (USHORT)MM100_TO_TWIP(nTemp); break;   //"Insert/Row",
                    case 3 : pValues[nProp] >>= nTemp; nTblVInsert = (USHORT)MM100_TO_TWIP(nTemp); break;   //"Insert/Column",
                    case 4 : pValues[nProp] >>= nTemp; eTblChgMode = (TblChgMode)nTemp; break;   //"Change/Effect",
                    case 5 : bInsTblFormatNum = *(sal_Bool*)pValues[nProp].getValue();  break;  //"Input/NumberRecognition",
                    case 6 : bInsTblChangeNumFormat = *(sal_Bool*)pValues[nProp].getValue(); break;  //"Input/NumberFormatRecognition",
                    case 7 : bInsTblAlignNum = *(sal_Bool*)pValues[nProp].getValue(); break;  //"Input/Alignment"
                }
            }
        }
    }
}

