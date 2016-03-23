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

#include <tools/stream.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/syslocale.hxx>

#include "swtypes.hxx"
#include "hintids.hxx"
#include "uitool.hxx"
#include "usrpref.hxx"
#include "crstate.hxx"
#include <linguistic/lngprops.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <unotools/localedatawrapper.hxx>

#include <unomid.h>

using namespace utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

void SwMasterUsrPref::SetUsrPref(const SwViewOption &rCopy)
{
    *static_cast<SwViewOption*>(this) = rCopy;
}

SwMasterUsrPref::SwMasterUsrPref(bool bWeb) :
    eFieldUpdateFlags(AUTOUPD_OFF),
    nLinkUpdateMode(0),
    bIsHScrollMetricSet(false),
    bIsVScrollMetricSet(false),
    nDefTab( MM50 * 4 ),
    bIsSquaredPageMode(false),
    bIsAlignMathObjectsToBaseline(false),
    aContentConfig(bWeb, *this),
    aLayoutConfig(bWeb, *this),
    aGridConfig(bWeb, *this),
    aCursorConfig(*this),
    pWebColorConfig(bWeb ? new SwWebColorConfig(*this) : nullptr),
    bApplyCharUnit(false)
{
    if (utl::ConfigManager::IsAvoidConfig())
    {
        eHScrollMetric = eVScrollMetric = eUserMetric = FUNIT_CM;
        return;
    }
    MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    eUserMetric = MEASURE_METRIC == eSystem ? FUNIT_CM : FUNIT_INCH;
    eHScrollMetric = eVScrollMetric = eUserMetric;

    aContentConfig.Load();
    aLayoutConfig.Load();
    aGridConfig.Load();
    aCursorConfig.Load();
    if(pWebColorConfig)
        pWebColorConfig->Load();
}

SwMasterUsrPref::~SwMasterUsrPref()
{
    delete pWebColorConfig;
}

Sequence<OUString> SwContentViewConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Display/GraphicObject",                    //  0
        "Display/Table",                            //  1
        "Display/DrawingControl",                   //  2
        "Display/FieldCode",                        //  3
        "Display/Note",                             //  4
        "Display/ShowContentTips",                      //  5
        "NonprintingCharacter/MetaCharacters",     //   6
        "NonprintingCharacter/ParagraphEnd",        //  7
        "NonprintingCharacter/OptionalHyphen",      //  8
        "NonprintingCharacter/Space",               //  9
        "NonprintingCharacter/Break",               // 10
        "NonprintingCharacter/ProtectedSpace",      // 11
        "NonprintingCharacter/Tab",             // 12 //not in Writer/Web
        "NonprintingCharacter/HiddenText",      // 13
        "NonprintingCharacter/HiddenParagraph", // 14
        "NonprintingCharacter/HiddenCharacter",      // 15
        "Update/Link",                          // 16
        "Update/Field",                         // 17
        "Update/Chart"                          // 18

    };
    const int nCount = bWeb ? 12 : 19;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

SwContentViewConfig::SwContentViewConfig(bool bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? OUString("Office.WriterWeb/Content") :  OUString("Office.Writer/Content")),
    rParent(rPar),
    bWeb(bIsWeb)
{
    Load();
    EnableNotification( GetPropertyNames() );
}

SwContentViewConfig::~SwContentViewConfig()
{
}

void SwContentViewConfig::Notify( const Sequence< OUString > & /*rPropertyNames*/ )
{
    Load();
}

void SwContentViewConfig::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        bool bVal = false;
        switch(nProp)
        {
            case  0: bVal = rParent.IsGraphic();    break;// "Display/GraphicObject",
            case  1: bVal = rParent.IsTable();  break;// "Display/Table",
            case  2: bVal = rParent.IsDraw();       break;// "Display/DrawingControl",
            case  3: bVal = rParent.IsFieldName();    break;// "Display/FieldCode",
            case  4: bVal = rParent.IsPostIts();    break;// "Display/Note",
            case  5: bVal = rParent.IsShowContentTips(); break; // "Display/ShowContentTips"
            case  6: bVal = rParent.IsViewMetaChars(); break; //"NonprintingCharacter/MetaCharacters"
            case  7: bVal = rParent.IsParagraph(true); break;// "NonprintingCharacter/ParagraphEnd",
            case  8: bVal = rParent.IsSoftHyph(); break;// "NonprintingCharacter/OptionalHyphen",
            case  9: bVal = rParent.IsBlank(true);  break;// "NonprintingCharacter/Space",
            case 10: bVal = rParent.IsLineBreak(true);break;// "NonprintingCharacter/Break",
            case 11: bVal = rParent.IsHardBlank(); break;// "NonprintingCharacter/ProtectedSpace",
            case 12: bVal = rParent.IsTab(true);        break;// "NonprintingCharacter/Tab",
            case 13: bVal = rParent.IsShowHiddenField(); break;// "NonprintingCharacter/Fields: HiddenText",
            case 14: bVal = rParent.IsShowHiddenPara(); break;// "NonprintingCharacter/Fields: HiddenParagraph",
            case 15: bVal = rParent.IsShowHiddenChar(true);    break;// "NonprintingCharacter/HiddenCharacter",
            case 16: pValues[nProp] <<= rParent.GetUpdateLinkMode();    break;// "Update/Link",
            case 17: bVal = rParent.IsUpdateFields(); break;// "Update/Field",
            case 18: bVal = rParent.IsUpdateCharts(); break;// "Update/Chart"
        }
        if(nProp != 16)
            pValues[nProp] <<= bVal;
    }
    PutProperties(aNames, aValues);
}

void SwContentViewConfig::Load()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                bool bSet = nProp != 16 && *static_cast<sal_Bool const *>(pValues[nProp].getValue());
                switch(nProp)
                {
                    case  0: rParent.SetGraphic(bSet);  break;// "Display/GraphicObject",
                    case  1: rParent.SetTable(bSet);    break;// "Display/Table",
                    case  2: rParent.SetDraw(bSet);     break;// "Display/DrawingControl",
                    case  3: rParent.SetFieldName(bSet);  break;// "Display/FieldCode",
                    case  4: rParent.SetPostIts(bSet);  break;// "Display/Note",
                    case  5: rParent.SetShowContentTips(bSet);  break;// "Display/ShowContentTips",
                    case  6: rParent.SetViewMetaChars(bSet); break; //"NonprintingCharacter/MetaCharacters"
                    case  7: rParent.SetParagraph(bSet); break;// "NonprintingCharacter/ParagraphEnd",
                    case  8: rParent.SetSoftHyph(bSet); break;// "NonprintingCharacter/OptionalHyphen",
                    case  9: rParent.SetBlank(bSet);    break;// "NonprintingCharacter/Space",
                    case 10: rParent.SetLineBreak(bSet);break;// "NonprintingCharacter/Break",
                    case 11: rParent.SetHardBlank(bSet); break;// "NonprintingCharacter/ProtectedSpace",
                    case 12: rParent.SetTab(bSet);      break;// "NonprintingCharacter/Tab",
                    case 13: rParent.SetShowHiddenField(bSet);   break;// "NonprintingCharacter/Fields: HiddenText",
                    case 14: rParent.SetShowHiddenPara(bSet); break;// "NonprintingCharacter/Fields: HiddenParagraph",
                    case 15: rParent.SetShowHiddenChar(bSet); break;// "NonprintingCharacter/HiddenCharacter",
                    case 16:
                    {
                        sal_Int32 nSet = 0;
                        pValues[nProp] >>= nSet;
                        rParent.SetUpdateLinkMode(nSet, true);
                    }
                    break;// "Update/Link",
                    case 17: rParent.SetUpdateFields(bSet, true); break;// "Update/Field",
                    case 18: rParent.SetUpdateCharts(bSet, true); break;// "Update/Chart"
                }
            }
        }
    }
}

Sequence<OUString> SwLayoutViewConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Line/Guide",                           // 0
        "Window/HorizontalScroll",              // 1
        "Window/VerticalScroll",                // 2
        "Window/ShowRulers",                    // 3
        "Window/HorizontalRuler",               // 4
        "Window/VerticalRuler",                 // 5
        "Window/HorizontalRulerUnit",           // 6
        "Window/VerticalRulerUnit",             // 7
        "Window/SmoothScroll",                  // 8
        "Zoom/Value",                           // 9
        "Zoom/Type",                            //10
        "Other/IsAlignMathObjectsToBaseline",   //11
        "Other/MeasureUnit",                    //12
        // below properties are not available in WriterWeb
        "Other/TabStop",                        //13
        "Window/IsVerticalRulerRight",          //14
        "ViewLayout/Columns",                   //15
        "ViewLayout/BookMode",                  //16
        "Other/IsSquaredPageMode",              //17
        "Other/ApplyCharUnit",                  //18
        "Window/ShowScrollBarTips"              //19
    };
    const int nCount = bWeb ? 13 : 20;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

SwLayoutViewConfig::SwLayoutViewConfig(bool bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? OUString("Office.WriterWeb/Layout") :  OUString("Office.Writer/Layout"),
        ConfigItemMode::DelayedUpdate|ConfigItemMode::ReleaseTree),
    rParent(rPar),
    bWeb(bIsWeb)
{
}

SwLayoutViewConfig::~SwLayoutViewConfig()
{
}

void SwLayoutViewConfig::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        Any &rVal = pValues[nProp];
        switch(nProp)
        {
            case  0: rVal <<= rParent.IsCrossHair(); break;              // "Line/Guide",
            case  1: rVal <<= rParent.IsViewHScrollBar(); break;         // "Window/HorizontalScroll",
            case  2: rVal <<= rParent.IsViewVScrollBar(); break;         // "Window/VerticalScroll",
            case  3: rVal <<= rParent.IsViewAnyRuler(); break;           // "Window/ShowRulers"
            // #i14593# use IsView*Ruler(true) instead of IsView*Ruler()
            // this preserves the single ruler states even if "Window/ShowRulers" is off
            case  4: rVal <<= rParent.IsViewHRuler(true); break;         // "Window/HorizontalRuler",
            case  5: rVal <<= rParent.IsViewVRuler(true); break;         // "Window/VerticalRuler",
            case  6:
                if(rParent.bIsHScrollMetricSet)
                    rVal <<= (sal_Int32)rParent.eHScrollMetric;                     // "Window/HorizontalRulerUnit"
            break;
            case  7:
                if(rParent.bIsVScrollMetricSet)
                    rVal <<= (sal_Int32)rParent.eVScrollMetric;                     // "Window/VerticalRulerUnit"
            break;
            case  8: rVal <<= rParent.IsSmoothScroll(); break;                      // "Window/SmoothScroll",
            case  9: rVal <<= (sal_Int32)rParent.GetZoom(); break;                  // "Zoom/Value",
            case 10: rVal <<= (sal_Int32)rParent.GetZoomType(); break;              // "Zoom/Type",
            case 11: rVal <<= rParent.IsAlignMathObjectsToBaseline(); break;        // "Other/IsAlignMathObjectsToBaseline"
            case 12: rVal <<= (sal_Int32)rParent.GetMetric(); break;                // "Other/MeasureUnit",
            case 13: rVal <<= static_cast<sal_Int32>(convertTwipToMm100(rParent.GetDefTab())); break;// "Other/TabStop",
            case 14: rVal <<= rParent.IsVRulerRight(); break;                       // "Window/IsVerticalRulerRight",
            case 15: rVal <<= (sal_Int32)rParent.GetViewLayoutColumns(); break;     // "ViewLayout/Columns",
            case 16: rVal <<= rParent.IsViewLayoutBookMode(); break;                // "ViewLayout/BookMode",
            case 17: rVal <<= rParent.IsSquaredPageMode(); break;                   // "Other/IsSquaredPageMode",
            case 18: rVal <<= rParent.IsApplyCharUnit(); break;                     // "Other/ApplyCharUnit",
            case 19: rVal <<= rParent.IsShowScrollBarTips(); break;                 // "Window/ShowScrollBarTips",
        }
    }
    PutProperties(aNames, aValues);
}

void SwLayoutViewConfig::Load()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                sal_Int32   nInt32Val   = 0;
                bool    bSet        = false;
                pValues[nProp] >>= nInt32Val;
                pValues[nProp] >>= bSet;

                switch(nProp)
                {
                    case  0: rParent.SetCrossHair(bSet); break;// "Line/Guide",
                    case  1: rParent.SetViewHScrollBar(bSet); break;// "Window/HorizontalScroll",
                    case  2: rParent.SetViewVScrollBar(bSet); break;// "Window/VerticalScroll",
                    case  3: rParent.SetViewAnyRuler(bSet);break; // "Window/ShowRulers"
                    case  4: rParent.SetViewHRuler(bSet); break;// "Window/HorizontalRuler",
                    case  5: rParent.SetViewVRuler(bSet); break;// "Window/VerticalRuler",
                    case  6:
                    {
                        rParent.bIsHScrollMetricSet = true;
                        rParent.eHScrollMetric = ((FieldUnit)nInt32Val);  // "Window/HorizontalRulerUnit"
                    }
                    break;
                    case  7:
                    {
                        rParent.bIsVScrollMetricSet = true;
                        rParent.eVScrollMetric = ((FieldUnit)nInt32Val); // "Window/VerticalRulerUnit"
                    }
                    break;
                    case  8: rParent.SetSmoothScroll(bSet); break;// "Window/SmoothScroll",
                    case  9: rParent.SetZoom( static_cast< sal_uInt16 >(nInt32Val) ); break;// "Zoom/Value",
                    case 10: rParent.SetZoomType( static_cast< SvxZoomType >(nInt32Val) ); break;// "Zoom/Type",
                    case 11: rParent.SetAlignMathObjectsToBaseline(bSet); break;// "Other/IsAlignMathObjectsToBaseline"
                    case 12: rParent.SetMetric((FieldUnit)nInt32Val, true); break;// "Other/MeasureUnit",
                    case 13: rParent.SetDefTab(convertMm100ToTwip(nInt32Val), true); break;// "Other/TabStop",
                    case 14: rParent.SetVRulerRight(bSet); break;// "Window/IsVerticalRulerRight",
                    case 15: rParent.SetViewLayoutColumns( static_cast<sal_uInt16>(nInt32Val) ); break;// "ViewLayout/Columns",
                    case 16: rParent.SetViewLayoutBookMode(bSet); break;// "ViewLayout/BookMode",
                    case 17: rParent.SetDefaultPageMode(bSet,true); break;// "Other/IsSquaredPageMode",
                    case 18: rParent.SetApplyCharUnit(bSet); break;// "Other/ApplyUserChar"
                    case 19: rParent.SetShowScrollBarTips(bSet); break;// "Window/ShowScrollBarTips",
                }
            }
        }
    }
}

void SwLayoutViewConfig::Notify( const css::uno::Sequence< OUString >& ) {}

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

SwGridConfig::SwGridConfig(bool bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? OUString("Office.WriterWeb/Grid") :  OUString("Office.Writer/Grid"),
        ConfigItemMode::DelayedUpdate|ConfigItemMode::ReleaseTree),
    rParent(rPar)
{
}

SwGridConfig::~SwGridConfig()
{
}

void SwGridConfig::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] <<= rParent.IsSnap(); break;//      "Option/SnapToGrid",
            case  1: pValues[nProp] <<= rParent.IsGridVisible(); break;//"Option/VisibleGrid",
            case  2: pValues[nProp] <<= rParent.IsSynchronize(); break;//  "Option/Synchronize",
            case  3: pValues[nProp] <<= (sal_Int32)convertTwipToMm100(rParent.GetSnapSize().Width()); break;//      "Resolution/XAxis",
            case  4: pValues[nProp] <<= (sal_Int32)convertTwipToMm100(rParent.GetSnapSize().Height()); break;//      "Resolution/YAxis",
            case  5: pValues[nProp] <<= (sal_Int16)rParent.GetDivisionX(); break;//   "Subdivision/XAxis",
            case  6: pValues[nProp] <<= (sal_Int16)rParent.GetDivisionY(); break;//   "Subdivision/YAxis"
        }
    }
    PutProperties(aNames, aValues);
}

void SwGridConfig::Load()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        Size aSnap(rParent.GetSnapSize());
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                bool bSet = nProp < 3 && *static_cast<sal_Bool const *>(pValues[nProp].getValue());
                sal_Int32 nSet = 0;
                if(nProp >= 3)
                    pValues[nProp] >>= nSet;
                switch(nProp)
                {
                    case  0: rParent.SetSnap(bSet); break;//        "Option/SnapToGrid",
                    case  1: rParent.SetGridVisible(bSet); break;//"Option/VisibleGrid",
                    case  2: rParent.SetSynchronize(bSet); break;//  "Option/Synchronize",
                    case  3: aSnap.Width() = convertMm100ToTwip(nSet); break;//      "Resolution/XAxis",
                    case  4: aSnap.Height() = convertMm100ToTwip(nSet); break;//      "Resolution/YAxis",
                    case  5: rParent.SetDivisionX((short)nSet); break;//   "Subdivision/XAxis",
                    case  6: rParent.SetDivisionY((short)nSet); break;//   "Subdivision/YAxis"
                }
            }
        }
        rParent.SetSnapSize(aSnap);
    }
}

void SwGridConfig::Notify( const css::uno::Sequence< OUString >& ) {}

Sequence<OUString> SwCursorConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "DirectCursor/UseDirectCursor", // 0
        "DirectCursor/Insert",          // 1
        "Option/ProtectedArea",         // 2
        "Option/IgnoreProtectedArea"    // 3
    };
    const int nCount = SAL_N_ELEMENTS(aPropNames);
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}

SwCursorConfig::SwCursorConfig(SwMasterUsrPref& rPar) :
    ConfigItem("Office.Writer/Cursor",
        ConfigItemMode::DelayedUpdate|ConfigItemMode::ReleaseTree),
    rParent(rPar)
{
}

SwCursorConfig::~SwCursorConfig()
{
}

void SwCursorConfig::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] <<= rParent.IsShadowCursor(); break;//  "DirectCursor/UseDirectCursor",
            case  1: pValues[nProp] <<= (sal_Int32)rParent.GetShdwCursorFillMode();   break;//  "DirectCursor/Insert",
            case  2: pValues[nProp] <<= rParent.IsCursorInProtectedArea(); break;// "Option/ProtectedArea"
            case  3: pValues[nProp] <<= rParent.IsIgnoreProtectedArea(); break; // "Option/IgnoreProtectedArea"
        }
    }
    PutProperties(aNames, aValues);
}

void SwCursorConfig::Load()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {

        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                bool bSet = false;
                sal_Int32 nSet = 0;
                if(nProp != 1 )
                    bSet = *static_cast<sal_Bool const *>(pValues[nProp].getValue());
                else
                    pValues[nProp] >>= nSet;
                switch(nProp)
                {
                    case  0: rParent.SetShadowCursor(bSet);         break;//  "DirectCursor/UseDirectCursor",
                    case  1: rParent.SetShdwCursorFillMode((sal_uInt8)nSet); break;//  "DirectCursor/Insert",
                    case  2: rParent.SetCursorInProtectedArea(bSet); break;// "Option/ProtectedArea"
                    case  3: rParent.SetIgnoreProtectedArea(bSet); break; // "Option/IgnoreProtectedArea"
                }
            }
        }

    }
}

void SwCursorConfig::Notify( const css::uno::Sequence< OUString >& ) {}

SwWebColorConfig::SwWebColorConfig(SwMasterUsrPref& rPar) :
    ConfigItem("Office.WriterWeb/Background",
        ConfigItemMode::DelayedUpdate|ConfigItemMode::ReleaseTree),
    rParent(rPar),
    aPropNames(1)
{
    aPropNames.getArray()[0] = "Color";
}

SwWebColorConfig::~SwWebColorConfig()
{
}

void SwWebColorConfig::ImplCommit()
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

void SwWebColorConfig::Notify( const css::uno::Sequence< OUString >& ) {}

void SwWebColorConfig::Load()
{
    Sequence<Any> aValues = GetProperties(aPropNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aPropNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aPropNames.getLength())
    {
        for(int nProp = 0; nProp < aPropNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0:
                        sal_Int32 nSet = 0;
                        pValues[nProp] >>= nSet; rParent.SetRetoucheColor(nSet);
                    break;// "Color",
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
