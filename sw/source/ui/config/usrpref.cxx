/*************************************************************************
 *
 *  $RCSfile: usrpref.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: dvo $ $Date: 2003-04-04 13:59:20 $
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

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif


#include "swtypes.hxx"
#include "hintids.hxx"
#include "uitool.hxx"
#include "usrpref.hxx"
#include "crstate.hxx"
#ifndef _OFA_OSPLCFG_HXX
#include <offmgr/osplcfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

using namespace utl;
using namespace rtl;
using namespace ::com::sun::star;
using namespace com::sun::star::uno;

#define C2U(cChar) OUString::createFromAscii(cChar)
#define C2S(cChar) String::CreateFromAscii(cChar)

void SwMasterUsrPref::SetUsrPref(const SwViewOption &rCopy)
{
    *((SwViewOption*)this) = rCopy;
}

SwMasterUsrPref::SwMasterUsrPref(BOOL bWeb) :
    aContentConfig(bWeb, *this),
    aLayoutConfig(bWeb, *this),
    aGridConfig(bWeb, *this),
    aCursorConfig(*this),
    pWebColorConfig(bWeb ? new SwWebColorConfig(*this) : 0),
    nFldUpdateFlags(0),
    nLinkUpdateMode(0),
    bIsHScrollMetricSet(sal_False),
    bIsVScrollMetricSet(sal_False),
    nDefTab( MM50 * 4 )
{
    MeasurementSystem eSystem = GetAppLocaleData().getMeasurementSystemEnum();
    eUserMetric = MEASURE_METRIC == eSystem ? FUNIT_CM : FUNIT_INCH;
    eHScrollMetric = eUserMetric;
    eVScrollMetric = eUserMetric;

    aContentConfig.Load();
    aLayoutConfig.Load();
    aGridConfig.Load();
    aCursorConfig.Load();
    if(pWebColorConfig)
        pWebColorConfig->Load();
}
/* -----------------------------13.02.01 09:48--------------------------------

 ---------------------------------------------------------------------------*/
SwMasterUsrPref::~SwMasterUsrPref()
{
    delete pWebColorConfig;
}
/*-- 28.09.00 09:55:32---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence<OUString> SwContentViewConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Display/GraphicObject",                    //  0
        "Display/Table",                            //  1
        "Display/DrawingControl",                   //  2
        "Display/FieldCode",                        //  3
        "Display/Note",                             //  4
        "Display/PreventTips",                      //  5
        "NonprintingCharacter/MetaCharacters",     //   6
        "NonprintingCharacter/ParagraphEnd",        //  7
        "NonprintingCharacter/OptionalHyphen",      //  8
        "NonprintingCharacter/Space",               //  9
        "NonprintingCharacter/Break",               // 10
        "NonprintingCharacter/ProtectedSpace",      // 11
            "NonprintingCharacter/Tab",             // 12 //not in Writer/Web
            "NonprintingCharacter/HiddenText",      // 13
            "NonprintingCharacter/HiddenParagraph", // 14
            "Update/Link",                          // 15
            "Update/Field",                         // 16
            "Update/Chart"                          // 17


    };
    const int nCount = bWeb ? 12 : 18;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwContentViewConfig::SwContentViewConfig(BOOL bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? C2U("Office.WriterWeb/Content") :  C2U("Office.Writer/Content"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    rParent(rPar),
    bWeb(bIsWeb)
{
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwContentViewConfig::~SwContentViewConfig()
{
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwContentViewConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Bool bVal = FALSE;
        switch(nProp)
        {
            case  0: bVal = rParent.IsGraphic();    break;// "Display/GraphicObject",
            case  1: bVal = rParent.IsTable();  break;// "Display/Table",
            case  2: bVal = rParent.IsDraw();       break;// "Display/DrawingControl",
            case  3: bVal = rParent.IsFldName();    break;// "Display/FieldCode",
            case  4: bVal = rParent.IsPostIts();    break;// "Display/Note",
            case  5: bVal = rParent.IsPreventTips(); break; // "Display/PreventTips"
            case  6: bVal = rParent.IsViewMetaChars(); break; //"NonprintingCharacter/MetaCharacters"
            case  7: bVal = rParent.IsParagraph(); break;// "NonprintingCharacter/ParagraphEnd",
            case  8: bVal = rParent.IsSoftHyph(); break;// "NonprintingCharacter/OptionalHyphen",
            case  9: bVal = rParent.IsBlank();  break;// "NonprintingCharacter/Space",
            case 10: bVal = rParent.IsLineBreak();break;// "NonprintingCharacter/Break",
            case 11: bVal = rParent.IsHardBlank(); break;// "NonprintingCharacter/ProtectedSpace",
            case 12: bVal = rParent.IsTab();        break;// "NonprintingCharacter/Tab",
            case 13: bVal = rParent.IsHidden(); break;// "NonprintingCharacter/HiddenText",
            case 14: bVal = rParent.IsShowHiddenPara(); break;// "NonprintingCharacter/HiddenParagraph",
            case 15: pValues[nProp] <<= rParent.GetUpdateLinkMode();    break;// "Update/Link",
            case 16: bVal = rParent.IsUpdateFields(); break;// "Update/Field",
            case 17: bVal = rParent.IsUpdateCharts(); break;// "Update/Chart"
        }
        if(nProp != 15)
            pValues[nProp].setValue(&bVal, ::getBooleanCppuType());
    }
    PutProperties(aNames, aValues);
}
/*-- 28.09.00 09:55:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwContentViewConfig::Load()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                sal_Bool bSet = nProp != 15 ? *(sal_Bool*)pValues[nProp].getValue() : sal_False;
                switch(nProp)
                {
                    case  0: rParent.SetGraphic(bSet);  break;// "Display/GraphicObject",
                    case  1: rParent.SetTable(bSet);    break;// "Display/Table",
                    case  2: rParent.SetDraw(bSet);     break;// "Display/DrawingControl",
                    case  3: rParent.SetFldName(bSet);  break;// "Display/FieldCode",
                    case  4: rParent.SetPostIts(bSet);  break;// "Display/Note",
                    case  5: rParent.SetPreventTips(bSet);  break;// "Display/PreventTips",
                    case  6: rParent.SetViewMetaChars(bSet); break; //"NonprintingCharacter/MetaCharacters"
                    case  7: rParent.SetParagraph(bSet); break;// "NonprintingCharacter/ParagraphEnd",
                    case  8: rParent.SetSoftHyph(bSet); break;// "NonprintingCharacter/OptionalHyphen",
                    case  9: rParent.SetBlank(bSet);    break;// "NonprintingCharacter/Space",
                    case 10: rParent.SetLineBreak(bSet);break;// "NonprintingCharacter/Break",
                    case 11: rParent.SetHardBlank(bSet); break;// "NonprintingCharacter/ProtectedSpace",
                    case 12: rParent.SetTab(bSet);      break;// "NonprintingCharacter/Tab",
                    case 13: rParent.SetHidden(bSet);   break;// "NonprintingCharacter/HiddenText",
                    case 14: rParent.SetShowHiddenPara(bSet); break;// "NonprintingCharacter/HiddenParagraph",
                    case 15:
                    {
                        sal_Int32 nSet; pValues[nProp] >>= nSet;
                        rParent.SetUpdateLinkMode(nSet, TRUE);
                    }
                    break;// "Update/Link",
                    case 16: rParent.SetUpdateFields(bSet, TRUE); break;// "Update/Field",
                    case 17: rParent.SetUpdateCharts(bSet, TRUE); break;// "Update/Chart"
                }
            }
        }
    }
}
/*-- 28.09.00 09:55:34---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence<OUString> SwLayoutViewConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Line/Guide",                       // 0
        "Line/SimpleControlPoint",          // 1
        "Line/LargeControlPoint",           // 2
        "Window/HorizontalScroll",          // 3
        "Window/VerticalScroll",            // 4
        "Window/ShowRulers",                // 5
        "Window/HorizontalRuler",           // 6
        "Window/VerticalRuler",             // 7
        "Window/HorizontalRulerUnit",       // 8
        "Window/VerticalRulerUnit",         // 9
        "Window/SmoothScroll",              //10
        "Zoom/Value",                       //11
        "Zoom/Type",                        //12
        "Other/MeasureUnit",                //13
        "Other/TabStop",                    //14
        "Window/IsVerticalRulerRight"       //15
    };
    const int nCount = bWeb ? 14 : 16;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
/*-- 28.09.00 09:55:34---------------------------------------------------

  -----------------------------------------------------------------------*/
SwLayoutViewConfig::SwLayoutViewConfig(BOOL bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? C2U("Office.WriterWeb/Layout") :  C2U("Office.Writer/Layout"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    rParent(rPar),
    bWeb(bIsWeb)
{
}
/*-- 28.09.00 09:55:35---------------------------------------------------

  -----------------------------------------------------------------------*/
SwLayoutViewConfig::~SwLayoutViewConfig()
{
}
/*-- 28.09.00 09:55:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwLayoutViewConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Bool bSet;
        switch(nProp)
        {
            case  0: bSet = rParent.IsCrossHair(); break;// "Line/Guide",
            case  1: bSet = rParent.IsSolidMarkHdl(); break;// "Line/SimpleControlPoint",
            case  2: bSet = rParent.IsBigMarkHdl(); break;// "Line/LargeControlPoint",
            case  3: bSet = rParent.IsViewHScrollBar(); break;// "Window/HorizontalScroll",
            case  4: bSet = rParent.IsViewVScrollBar(); break;// "Window/VerticalScroll",
            case  5: bSet = rParent.IsViewAnyRuler(); break; // "Window/ShowRulers"
            case  6: bSet = rParent.IsViewHRuler(); break;// "Window/HorizontalRuler",
            case  7: bSet = rParent.IsViewVRuler(); break;// "Window/VerticalRuler",
            case  8:
                if(rParent.bIsHScrollMetricSet)
                    pValues[nProp] <<= (sal_Int32)rParent.eHScrollMetric; // "Window/HorizontalRulerUnit"
            break;
            case  9:
                if(rParent.bIsVScrollMetricSet)
                    pValues[nProp] <<= (sal_Int32)rParent.eVScrollMetric; // "Window/VerticalRulerUnit"
            break;
            case 10: bSet = rParent.IsSmoothScroll(); break;// "Window/SmoothScroll",
            case 11: pValues[nProp] <<= (sal_Int32)rParent.GetZoom(); break;// "Zoom/Value",
            case 12: pValues[nProp] <<= (sal_Int32)rParent.GetZoomType(); break;// "Zoom/Type",
            case 13: pValues[nProp] <<= (sal_Int32)rParent.GetMetric(); break;// "Other/MeasureUnit",
            case 14: pValues[nProp] <<= TWIP_TO_MM100(rParent.GetDefTab()); break;// "Other/TabStop",
            case 15: bSet = rParent.IsVRulerRight(); break;// "Window/IsVerticalRulerRight",
        }
        if(nProp < 8 || nProp == 10)
            pValues[nProp].setValue(&bSet, ::getBooleanCppuType());
    }
    PutProperties(aNames, aValues);
}
/*-- 28.09.00 09:55:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwLayoutViewConfig::Load()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                sal_Bool bSet = nProp < 8 || nProp == 10 ? *(sal_Bool*)pValues[nProp].getValue() : sal_False;
                switch(nProp)
                {
                    case  0: rParent.SetCrossHair(bSet); break;// "Line/Guide",
                    case  1: rParent.SetSolidMarkHdl(bSet); break;// "Line/SimpleControlPoint",
                    case  2: rParent.SetBigMarkHdl(bSet); break;// "Line/LargeControlPoint",
                    case  3: rParent.SetViewHScrollBar(bSet); break;// "Window/HorizontalScroll",
                    case  4: rParent.SetViewVScrollBar(bSet); break;// "Window/VerticalScroll",
                    case  5: rParent.SetViewAnyRuler(bSet);break; // "Window/ShowRulers"
                    case  6: rParent.SetViewHRuler(bSet); break;// "Window/HorizontalRuler",
                    case  7: rParent.SetViewVRuler(bSet); break;// "Window/VerticalRuler",
                    case  8:
                    {
                        rParent.bIsHScrollMetricSet = sal_True;
                        sal_Int32 nUnit; pValues[nProp] >>= nUnit;
                        rParent.eHScrollMetric = ((FieldUnit)nUnit);  // "Window/HorizontalRulerUnit"
                    }
                    break;
                    case  9:
                    {
                        rParent.bIsVScrollMetricSet = sal_True;
                        sal_Int32 nUnit; pValues[nProp] >>= nUnit;
                        rParent.eVScrollMetric = ((FieldUnit)nUnit); // "Window/VerticalRulerUnit"
                    }
                    break;
                    case 10: rParent.SetSmoothScroll(bSet); break;// "Window/SmoothScroll",
                    case 11:
                    {
                        sal_Int32 nVal; pValues[nProp] >>= nVal;
                        rParent.SetZoom((USHORT)nVal);
                    }
                    break;// "Zoom/Value",
                    case 12:
                    {
                        sal_Int32 nVal; pValues[nProp] >>= nVal;
                        rParent.SetZoomType((BYTE)nVal);
                    }
                    break;// "Zoom/Type",
                    case 13:
                    {
                        sal_Int32 nUnit; pValues[nProp] >>= nUnit;
                        rParent.SetMetric((FieldUnit)nUnit, TRUE);
                    }
                    break;// "Other/MeasureUnit",
                    case 14:
                    {
                        sal_Int32 nTab; pValues[nProp] >>= nTab;
                        rParent.SetDefTab(MM100_TO_TWIP(nTab), TRUE);
                    }
                    break;// "Other/TabStop",
                    case 15: rParent.SetVRulerRight(bSet); break;// "Window/IsVerticalRulerRight",
                }
            }
        }
    }
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> SwGridConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Option/SnapToGrid",            // 0
        "Option/VisibleGrid",           // 1
        "Option/Synchronize",           // 2
        "Resolution/XAxis",             // 3
        "Resolution/YAxis",             // 4
        "Subdivision/XAxis",            // 5
        "Subdivision/YAxis"             // 6
    };
    const int nCount = 7;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
SwGridConfig::SwGridConfig(BOOL bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? C2U("Office.WriterWeb/Grid") :  C2U("Office.Writer/Grid"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    rParent(rPar),
    bWeb(bIsWeb)
{
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
SwGridConfig::~SwGridConfig()
{
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
void SwGridConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Bool bSet;
        switch(nProp)
        {
            case  0: bSet = rParent.IsSnap(); break;//      "Option/SnapToGrid",
            case  1: bSet = rParent.IsGridVisible(); break;//"Option/VisibleGrid",
            case  2: bSet = rParent.IsSynchronize(); break;//  "Option/Synchronize",
            case  3: pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100(rParent.GetSnapSize().Width()); break;//      "Resolution/XAxis",
            case  4: pValues[nProp] <<= (sal_Int32)TWIP_TO_MM100(rParent.GetSnapSize().Height()); break;//      "Resolution/YAxis",
            case  5: pValues[nProp] <<= (sal_Int16)rParent.GetDivisionX(); break;//   "Subdivision/XAxis",
            case  6: pValues[nProp] <<= (sal_Int16)rParent.GetDivisionY(); break;//   "Subdivision/YAxis"
        }
        if(nProp < 3)
              pValues[nProp].setValue(&bSet, ::getBooleanCppuType());
    }
    PutProperties(aNames, aValues);
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
void SwGridConfig::Load()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        Size aSnap(rParent.GetSnapSize());
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                sal_Bool bSet = nProp < 3 ? *(sal_Bool*)pValues[nProp].getValue() : sal_False;
                sal_Int32 nSet;
                if(nProp >= 3)
                    pValues[nProp] >>= nSet;
                switch(nProp)
                {
                    case  0: rParent.SetSnap(bSet); break;//        "Option/SnapToGrid",
                    case  1: rParent.SetGridVisible(bSet); break;//"Option/VisibleGrid",
                    case  2: rParent.SetSynchronize(bSet); break;//  "Option/Synchronize",
                    case  3: aSnap.Width() = MM100_TO_TWIP(nSet); break;//      "Resolution/XAxis",
                    case  4: aSnap.Height() = MM100_TO_TWIP(nSet); break;//      "Resolution/YAxis",
                    case  5: rParent.SetDivisionX((short)nSet); break;//   "Subdivision/XAxis",
                    case  6: rParent.SetDivisionY((short)nSet); break;//   "Subdivision/YAxis"
                }
            }
        }
        rParent.SetSnapSize(aSnap);
    }
}

/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> SwCursorConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "DirectCursor/UseDirectCursor", // 0
        "DirectCursor/Insert",          // 1
        "Option/ProtectedArea"          // 2
    };
    const int nCount = 3;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = C2U(aPropNames[i]);
    return aNames;
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
SwCursorConfig::SwCursorConfig(SwMasterUsrPref& rPar) :
    ConfigItem(C2U("Office.Writer/Cursor"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    rParent(rPar)
{
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
SwCursorConfig::~SwCursorConfig()
{
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
void SwCursorConfig::Commit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Bool bSet;
        switch(nProp)
        {
            case  0: bSet = rParent.IsShadowCursor();       break;//  "DirectCursor/UseDirectCursor",
            case  1: pValues[nProp] <<= (sal_Int32)rParent.GetShdwCrsrFillMode();   break;//  "DirectCursor/Insert",
            case  2: bSet = rParent.IsCursorInProtectedArea(); break;// "Option/ProtectedArea"
        }
        if(nProp != 1 )
              pValues[nProp].setValue(&bSet, ::getBooleanCppuType());
    }
    PutProperties(aNames, aValues);
}
/* -----------------------------19.01.01 13:07--------------------------------

 ---------------------------------------------------------------------------*/
void SwCursorConfig::Load()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {

        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                sal_Bool bSet;
                sal_Int32 nSet;
                if(nProp != 1 )
                    bSet = *(sal_Bool*)pValues[nProp].getValue();
                else
                    pValues[nProp] >>= nSet;
                switch(nProp)
                {
                    case  0: rParent.SetShadowCursor(bSet);         break;//  "DirectCursor/UseDirectCursor",
                    case  1: rParent.SetShdwCrsrFillMode((BYTE)nSet); break;//  "DirectCursor/Insert",
                    case  2: rParent.SetCursorInProtectedArea(bSet); break;// "Option/ProtectedArea"
                }
            }
        }

    }
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwWebColorConfig::SwWebColorConfig(SwMasterUsrPref& rPar) :
    ConfigItem(C2U("Office.WriterWeb/Background"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE),
    rParent(rPar),
    aPropNames(1)
{
    aPropNames.getArray()[0] = C2U("Color");
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwWebColorConfig::~SwWebColorConfig()
{
}
/*-- 28.09.00 09:55:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwWebColorConfig::Commit()
{
    Sequence<Any> aValues(aPropNames.getLength());
    Any* pValues = aValues.getArray();
    for(int nProp = 0; nProp < aPropNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] <<= (sal_Int32)rParent.GetRetoucheColor().GetColor();   break;// "Color",
        }
    }
    PutProperties(aPropNames, aValues);
}
/*-- 28.09.00 09:55:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwWebColorConfig::Load()
{
    Sequence<Any> aValues = GetProperties(aPropNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aPropNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aPropNames.getLength())
    {
        for(int nProp = 0; nProp < aPropNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0:
                        sal_Int32 nSet; pValues[nProp] >>= nSet; rParent.SetRetoucheColor(nSet);
                    break;// "Color",
                }
            }
        }
    }
}


