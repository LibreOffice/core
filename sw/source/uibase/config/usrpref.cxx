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

#include <sal/config.h>

#include <cstring>
#include <osl/diagnose.h>
#include <o3tl/any.hxx>
#include <tools/UnitConversion.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/syslocale.hxx>

#include <usrpref.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <unotools/localedatawrapper.hxx>

using namespace utl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

void SwMasterUsrPref::SetUsrPref(const SwViewOption &rCopy)
{
    *static_cast<SwViewOption*>(this) = rCopy;
}

SwMasterUsrPref::SwMasterUsrPref(bool bWeb) :
    m_eFieldUpdateFlags(AUTOUPD_OFF),
    m_nLinkUpdateMode(0),
    m_bIsHScrollMetricSet(false),
    m_bIsVScrollMetricSet(false),
    m_nDefTabInMm100( 2000 ), // 2 cm
    m_bIsSquaredPageMode(false),
    m_bIsAlignMathObjectsToBaseline(false),
    m_aContentConfig(bWeb, *this),
    m_aLayoutConfig(bWeb, *this),
    m_aGridConfig(bWeb, *this),
    m_aCursorConfig(*this),
    m_pWebColorConfig(bWeb ? new SwWebColorConfig(*this) : nullptr),
    m_bApplyCharUnit(false)
{
    if (utl::ConfigManager::IsFuzzing())
    {
        m_eHScrollMetric = m_eVScrollMetric = m_eUserMetric = FieldUnit::CM;
        return;
    }
    MeasurementSystem eSystem = SvtSysLocale().GetLocaleData().getMeasurementSystemEnum();
    m_eUserMetric = MeasurementSystem::Metric == eSystem ? FieldUnit::CM : FieldUnit::INCH;
    m_eHScrollMetric = m_eVScrollMetric = m_eUserMetric;

    m_aContentConfig.Load();
    m_aLayoutConfig.Load();
    m_aGridConfig.Load();
    m_aCursorConfig.Load();
    if(m_pWebColorConfig)
        m_pWebColorConfig->Load();
}

SwMasterUsrPref::~SwMasterUsrPref()
{
}

const auto g_UpdateLinkIndex = 17;

Sequence<OUString> SwContentViewConfig::GetPropertyNames() const
{
    static constexpr const char*const aPropNames[] =
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
        "NonprintingCharacter/Bookmarks",       // 16
        "Update/Link",                          // 17
        "Update/Field",                         // 18
        "Update/Chart",                         // 19
        "Display/ShowInlineTooltips",           // 20
        "Display/UseHeaderFooterMenu",          // 21
        "Display/ShowOutlineContentVisibilityButton" // 22
    };
#if defined(__GNUC__) && !defined(__clang__)
    // clang 8.0.0 says strcmp isn't constexpr
    static_assert(std::strcmp("Update/Link", aPropNames[g_UpdateLinkIndex]) == 0);
#endif
    const int nCount = bWeb ? 12 : SAL_N_ELEMENTS(aPropNames);
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
            case 16: bVal = rParent.IsShowBookmarks(true);    break;// "NonprintingCharacter/Bookmarks",
            case 17: pValues[nProp] <<= rParent.GetUpdateLinkMode();    break;// "Update/Link",
            case 18: bVal = rParent.IsUpdateFields(); break;// "Update/Field",
            case 19: bVal = rParent.IsUpdateCharts(); break;// "Update/Chart"
            case 20: bVal = rParent.IsShowInlineTooltips(); break;// "Display/ShowInlineTooltips"
            case 21: bVal = rParent.IsUseHeaderFooterMenu(); break;// "Display/UseHeaderFooterMenu"
            case 22: bVal = rParent.IsShowOutlineContentVisibilityButton(); break;// "Display/ShowOutlineContentVisibilityButton"
        }
        if (nProp != g_UpdateLinkIndex)
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
    if(aValues.getLength() != aNames.getLength())
        return;

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            bool bSet = nProp != g_UpdateLinkIndex && *o3tl::doAccess<bool>(pValues[nProp]);
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
                case 16: rParent.SetShowBookmarks(bSet); break;// "NonprintingCharacter/Bookmarks",
                case 17:
                {
                    sal_Int32 nSet = 0;
                    pValues[nProp] >>= nSet;
                    rParent.SetUpdateLinkMode(nSet, true);
                }
                break;// "Update/Link",
                case 18: rParent.SetUpdateFields(bSet); break;// "Update/Field",
                case 19: rParent.SetUpdateCharts(bSet); break;// "Update/Chart"
                case 20: rParent.SetShowInlineTooltips(bSet); break;// "Display/ShowInlineTooltips"
                case 21: rParent.SetUseHeaderFooterMenu(bSet); break;// "Display/UseHeaderFooterMenu"
                case 22: rParent.SetShowOutlineContentVisibilityButton(bSet); break;// "Display/ShowOutlineContententVisibilityButton"
            }
        }
    }
}

Sequence<OUString> SwLayoutViewConfig::GetPropertyNames() const
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
        ConfigItemMode::ReleaseTree),
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
                if(rParent.m_bIsHScrollMetricSet)
                    rVal <<= static_cast<sal_Int32>(rParent.m_eHScrollMetric);                     // "Window/HorizontalRulerUnit"
            break;
            case  7:
                if(rParent.m_bIsVScrollMetricSet)
                    rVal <<= static_cast<sal_Int32>(rParent.m_eVScrollMetric);                     // "Window/VerticalRulerUnit"
            break;
            case  8: rVal <<= rParent.IsSmoothScroll(); break;                      // "Window/SmoothScroll",
            case  9: rVal <<= static_cast<sal_Int32>(rParent.GetZoom()); break;                  // "Zoom/Value",
            case 10: rVal <<= static_cast<sal_Int32>(rParent.GetZoomType()); break;              // "Zoom/Type",
            case 11: rVal <<= rParent.IsAlignMathObjectsToBaseline(); break;        // "Other/IsAlignMathObjectsToBaseline"
            case 12: rVal <<= static_cast<sal_Int32>(rParent.GetMetric()); break;                // "Other/MeasureUnit",
            case 13: rVal <<= rParent.GetDefTabInMm100(); break;// "Other/TabStop",
            case 14: rVal <<= rParent.IsVRulerRight(); break;                       // "Window/IsVerticalRulerRight",
            case 15: rVal <<= static_cast<sal_Int32>(rParent.GetViewLayoutColumns()); break;     // "ViewLayout/Columns",
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
    if(aValues.getLength() != aNames.getLength())
        return;

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
                    rParent.m_bIsHScrollMetricSet = true;
                    rParent.m_eHScrollMetric = static_cast<FieldUnit>(nInt32Val);  // "Window/HorizontalRulerUnit"
                }
                break;
                case  7:
                {
                    rParent.m_bIsVScrollMetricSet = true;
                    rParent.m_eVScrollMetric = static_cast<FieldUnit>(nInt32Val); // "Window/VerticalRulerUnit"
                }
                break;
                case  8: rParent.SetSmoothScroll(bSet); break;// "Window/SmoothScroll",
                case  9: rParent.SetZoom( static_cast< sal_uInt16 >(nInt32Val) ); break;// "Zoom/Value",
                case 10: rParent.SetZoomType( static_cast< SvxZoomType >(nInt32Val) ); break;// "Zoom/Type",
                case 11: rParent.SetAlignMathObjectsToBaseline(bSet, true); break;// "Other/IsAlignMathObjectsToBaseline"
                case 12: rParent.SetMetric(static_cast<FieldUnit>(nInt32Val), true); break;// "Other/MeasureUnit",
                case 13: rParent.SetDefTabInMm100(nInt32Val, true); break;// "Other/TabStop",
                case 14: rParent.SetVRulerRight(bSet); break;// "Window/IsVerticalRulerRight",
                case 15: rParent.SetViewLayoutColumns( static_cast<sal_uInt16>(nInt32Val) ); break;// "ViewLayout/Columns",
                case 16: rParent.SetViewLayoutBookMode(bSet); break;// "ViewLayout/BookMode",
                case 17: rParent.SetDefaultPageMode(bSet,true); break;// "Other/IsSquaredPageMode",
                case 18: rParent.SetApplyCharUnit(bSet, true); break;// "Other/ApplyUserChar"
                case 19: rParent.SetShowScrollBarTips(bSet); break;// "Window/ShowScrollBarTips",
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
        ConfigItemMode::ReleaseTree),
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
            case  3: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(rParent.GetSnapSize().Width())); break;//      "Resolution/XAxis",
            case  4: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(rParent.GetSnapSize().Height())); break;//      "Resolution/YAxis",
            case  5: pValues[nProp] <<= static_cast<sal_Int16>(rParent.GetDivisionX()); break;//   "Subdivision/XAxis",
            case  6: pValues[nProp] <<= static_cast<sal_Int16>(rParent.GetDivisionY()); break;//   "Subdivision/YAxis"
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
    if(aValues.getLength() != aNames.getLength())
        return;

    Size aSnap(rParent.GetSnapSize());
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            bool bSet = nProp < 3 && *o3tl::doAccess<bool>(pValues[nProp]);
            sal_Int32 nSet = 0;
            if(nProp >= 3)
                pValues[nProp] >>= nSet;
            switch(nProp)
            {
                case  0: rParent.SetSnap(bSet); break;//        "Option/SnapToGrid",
                case  1: rParent.SetGridVisible(bSet); break;//"Option/VisibleGrid",
                case  2: rParent.SetSynchronize(bSet); break;//  "Option/Synchronize",
                case  3: aSnap.setWidth( convertMm100ToTwip(nSet) ); break;//      "Resolution/XAxis",
                case  4: aSnap.setHeight( convertMm100ToTwip(nSet) ); break;//      "Resolution/YAxis",
                case  5: rParent.SetDivisionX(static_cast<short>(nSet)); break;//   "Subdivision/XAxis",
                case  6: rParent.SetDivisionY(static_cast<short>(nSet)); break;//   "Subdivision/YAxis"
            }
        }
    }
    rParent.SetSnapSize(aSnap);
}

void SwGridConfig::Notify( const css::uno::Sequence< OUString >& ) {}

Sequence<OUString> SwCursorConfig::GetPropertyNames()
{
    static const char* aPropNames[] =
    {
        "DirectCursor/UseDirectCursor", // 0
        "DirectCursor/Insert",          // 1
        "Option/ProtectedArea",         // 2
    };
    const int nCount = SAL_N_ELEMENTS(aPropNames);
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}

SwCursorConfig::SwCursorConfig(SwMasterUsrPref& rPar) :
    ConfigItem("Office.Writer/Cursor", ConfigItemMode::ReleaseTree),
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
            case  0: pValues[nProp] <<= rParent.IsShadowCursor();                   break; // "DirectCursor/UseDirectCursor",
            case  1: pValues[nProp] <<= static_cast<sal_Int32>(rParent.GetShdwCursorFillMode()); break; // "DirectCursor/Insert",
            case  2: pValues[nProp] <<= rParent.IsCursorInProtectedArea();          break; // "Option/ProtectedArea"
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
    if(aValues.getLength() != aNames.getLength())
        return;


    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            bool bSet = false;
            sal_Int32 nSet = 0;
            if(nProp != 1 )
                bSet = *o3tl::doAccess<bool>(pValues[nProp]);
            else
                pValues[nProp] >>= nSet;
            switch(nProp)
            {
                case  0: rParent.SetShadowCursor(bSet);                  break; // "DirectCursor/UseDirectCursor",
                case  1: rParent.SetShdwCursorFillMode(static_cast<SwFillMode>(nSet)); break; // "DirectCursor/Insert",
                case  2: rParent.SetCursorInProtectedArea(bSet);         break; // "Option/ProtectedArea"
            }
        }
    }
}

void SwCursorConfig::Notify( const css::uno::Sequence< OUString >& ) {}

SwWebColorConfig::SwWebColorConfig(SwMasterUsrPref& rPar) :
    ConfigItem("Office.WriterWeb/Background", ConfigItemMode::ReleaseTree),
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
            case  0: pValues[nProp] <<= rParent.GetRetoucheColor();   break;// "Color",
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
    if(aValues.getLength() != aPropNames.getLength())
        return;

    for(int nProp = 0; nProp < aPropNames.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            switch(nProp)
            {
                case  0:
                    Color nSet;
                    pValues[nProp] >>= nSet; rParent.SetRetoucheColor(nSet);
                break;// "Color",
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
