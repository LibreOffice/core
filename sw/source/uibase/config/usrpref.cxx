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

#include <osl/diagnose.h>
#include <o3tl/any.hxx>
#include <tools/UnitConversion.hxx>
#include <comphelper/configuration.hxx>
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
    m_aFmtAidsAutoComplConfig(*this),
    m_bApplyCharUnit(false)
{
    if (comphelper::IsFuzzing())
    {
        m_eHScrollMetric = m_eVScrollMetric = m_eUserMetric = FieldUnit::CM;
        // match defaults
        SetCore2Option(true, ViewOptCoreFlags2::CursorInProt);
        SetCore2Option(false, ViewOptCoreFlags2::HiddenPara);
        m_nDefTabInMm100 = 1250;
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
    m_aFmtAidsAutoComplConfig.Load();
}

SwMasterUsrPref::~SwMasterUsrPref()
{
}

const auto g_UpdateLinkIndex = 17;
const auto g_DefaultAnchor = 25;

Sequence<OUString> SwContentViewConfig::GetPropertyNames() const
{
    static constexpr const char*const aPropNames[] =
    {
        "Display/GraphicObject",                    //  0
        "Display/Table",                            //  1
        "Display/DrawingControl",                   //  2
        "Display/FieldCode",                        //  3
        "Display/Note",                             //  4
        "Display/ShowContentTips",                  //  5
        "NonprintingCharacter/MetaCharacters",      //  6
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
        "Display/ShowOutlineContentVisibilityButton", // 22
        "Display/TreatSubOutlineLevelsAsContent",     // 23
        "Display/ShowChangesInMargin",          // 24
        "Display/DefaultAnchor"                 // 25
    };
#if defined(__GNUC__) && !defined(__clang__)
    // clang 8.0.0 says strcmp isn't constexpr
    static_assert(std::strcmp("Update/Link", aPropNames[g_UpdateLinkIndex]) == 0);
    static_assert(std::strcmp("Display/DefaultAnchor", aPropNames[g_DefaultAnchor]) == 0);
#endif
    const int nCount = m_bWeb ? 12 : SAL_N_ELEMENTS(aPropNames);
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

SwContentViewConfig::SwContentViewConfig(bool bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? u"Office.WriterWeb/Content"_ustr :  u"Office.Writer/Content"_ustr),
    m_rParent(rPar),
    m_bWeb(bIsWeb)
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
            case  0: bVal = m_rParent.IsGraphic();    break;// "Display/GraphicObject",
            case  1: bVal = m_rParent.IsTable();  break;// "Display/Table",
            case  2: bVal = m_rParent.IsDraw();       break;// "Display/DrawingControl",
            case  3: bVal = m_rParent.IsFieldName();    break;// "Display/FieldCode",
            case  4: bVal = m_rParent.IsPostIts();    break;// "Display/Note",
            case  5: bVal = m_rParent.IsShowContentTips(); break; // "Display/ShowContentTips"
            case  6: bVal = m_rParent.IsViewMetaChars(); break; //"NonprintingCharacter/MetaCharacters"
            case  7: bVal = m_rParent.IsParagraph(true); break;// "NonprintingCharacter/ParagraphEnd",
            case  8: bVal = m_rParent.IsSoftHyph(); break;// "NonprintingCharacter/OptionalHyphen",
            case  9: bVal = m_rParent.IsBlank(true);  break;// "NonprintingCharacter/Space",
            case 10: bVal = m_rParent.IsLineBreak(true);break;// "NonprintingCharacter/Break",
            case 11: bVal = m_rParent.IsHardBlank(); break;// "NonprintingCharacter/ProtectedSpace",
            case 12: bVal = m_rParent.IsTab(true);        break;// "NonprintingCharacter/Tab",
            case 13: bVal = m_rParent.IsShowHiddenField(); break;// "NonprintingCharacter/Fields: HiddenText",
            case 14: bVal = m_rParent.IsShowHiddenPara(); break;// "NonprintingCharacter/Fields: HiddenParagraph",
            case 15: bVal = m_rParent.IsShowHiddenChar(true);    break;// "NonprintingCharacter/HiddenCharacter",
            case 16: bVal = m_rParent.IsShowBookmarks(true);    break;// "NonprintingCharacter/Bookmarks",
            case 17: pValues[nProp] <<= m_rParent.GetUpdateLinkMode();    break;// "Update/Link",
            case 18: bVal = m_rParent.IsUpdateFields(); break;// "Update/Field",
            case 19: bVal = m_rParent.IsUpdateCharts(); break;// "Update/Chart"
            case 20: bVal = m_rParent.IsShowInlineTooltips(); break;// "Display/ShowInlineTooltips"
            case 21: bVal = m_rParent.IsUseHeaderFooterMenu(); break;// "Display/UseHeaderFooterMenu"
            case 22: bVal = m_rParent.IsShowOutlineContentVisibilityButton(); break;// "Display/ShowOutlineContentVisibilityButton"
            case 23: bVal = m_rParent.IsTreatSubOutlineLevelsAsContent(); break;// "Display/TreatSubOutlineLevelsAsContent"
            case 24: bVal = m_rParent.IsShowChangesInMargin(); break;// "Display/ShowChangesInMargin"
            case 25: pValues[nProp] <<= m_rParent.GetDefaultAnchor(); break;// "Display/DefaultAnchor"
        }
        if ((nProp != g_UpdateLinkIndex) && (nProp != g_DefaultAnchor))
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
            bool bSet = ((nProp != g_UpdateLinkIndex) && (nProp != g_DefaultAnchor))
                        && *o3tl::doAccess<bool>(pValues[nProp]);
            switch(nProp)
            {
                case  0: m_rParent.SetGraphic(bSet);  break;// "Display/GraphicObject",
                case  1: m_rParent.SetTable(bSet);    break;// "Display/Table",
                case  2: m_rParent.SetDraw(bSet);     break;// "Display/DrawingControl",
                case  3: m_rParent.SetFieldName(bSet);  break;// "Display/FieldCode",
                case  4: m_rParent.SetPostIts(bSet);  break;// "Display/Note",
                case  5: m_rParent.SetShowContentTips(bSet);  break;// "Display/ShowContentTips",
                case  6: m_rParent.SetViewMetaChars(bSet); break; //"NonprintingCharacter/MetaCharacters"
                case  7: m_rParent.SetParagraph(bSet); break;// "NonprintingCharacter/ParagraphEnd",
                case  8: m_rParent.SetSoftHyph(bSet); break;// "NonprintingCharacter/OptionalHyphen",
                case  9: m_rParent.SetBlank(bSet);    break;// "NonprintingCharacter/Space",
                case 10: m_rParent.SetLineBreak(bSet);break;// "NonprintingCharacter/Break",
                case 11: m_rParent.SetHardBlank(bSet); break;// "NonprintingCharacter/ProtectedSpace",
                case 12: m_rParent.SetTab(bSet);      break;// "NonprintingCharacter/Tab",
                case 13: m_rParent.SetShowHiddenField(bSet);   break;// "NonprintingCharacter/Fields: HiddenText",
                case 14: m_rParent.SetShowHiddenPara(bSet); break;// "NonprintingCharacter/Fields: HiddenParagraph",
                case 15: m_rParent.SetShowHiddenChar(bSet); break;// "NonprintingCharacter/HiddenCharacter",
                case 16: m_rParent.SetShowBookmarks(bSet); break;// "NonprintingCharacter/Bookmarks",
                case 17:
                {
                    sal_Int32 nSet = 0;
                    pValues[nProp] >>= nSet;
                    m_rParent.SetUpdateLinkMode(nSet, true);
                }
                break;// "Update/Link",
                case 18: m_rParent.SetUpdateFields(bSet); break;// "Update/Field",
                case 19: m_rParent.SetUpdateCharts(bSet); break;// "Update/Chart"
                case 20: m_rParent.SetShowInlineTooltips(bSet); break;// "Display/ShowInlineTooltips"
                case 21: m_rParent.SetUseHeaderFooterMenu(bSet); break;// "Display/UseHeaderFooterMenu"
                case 22: m_rParent.SetShowOutlineContentVisibilityButton(bSet); break;// "Display/ShowOutlineContententVisibilityButton"
                case 23: m_rParent.SetTreatSubOutlineLevelsAsContent(bSet); break;// "Display/TreatSubOutlineLevelsAsContent"
                case 24: m_rParent.SetShowChangesInMargin(bSet); break;// "Display/ShowChangesInMargin"
                case 25:
                {
                    sal_Int32 nSet = 0;
                    pValues[nProp] >>= nSet;
                    m_rParent.SetDefaultAnchor(nSet);
                }
                break; // "Display/DefaultAnchor"
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
        "Window/ShowScrollBarTips",             //19
    };
    const int nCount = m_bWeb ? 13 : 20;
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < nCount; i++)
    {
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    }
    return aNames;
}

SwLayoutViewConfig::SwLayoutViewConfig(bool bIsWeb, SwMasterUsrPref& rPar) :
    ConfigItem(bIsWeb ? u"Office.WriterWeb/Layout"_ustr :  u"Office.Writer/Layout"_ustr,
        ConfigItemMode::ReleaseTree),
    m_rParent(rPar),
    m_bWeb(bIsWeb)
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
            case  0: rVal <<= m_rParent.IsCrossHair(); break;              // "Line/Guide",
            case  1: rVal <<= m_rParent.IsViewHScrollBar(); break;         // "Window/HorizontalScroll",
            case  2: rVal <<= m_rParent.IsViewVScrollBar(); break;         // "Window/VerticalScroll",
            case  3: rVal <<= m_rParent.IsViewAnyRuler(); break;           // "Window/ShowRulers"
            // #i14593# use IsView*Ruler(true) instead of IsView*Ruler()
            // this preserves the single ruler states even if "Window/ShowRulers" is off
            case  4: rVal <<= m_rParent.IsViewHRuler(true); break;         // "Window/HorizontalRuler",
            case  5: rVal <<= m_rParent.IsViewVRuler(true); break;         // "Window/VerticalRuler",
            case  6:
                if(m_rParent.m_bIsHScrollMetricSet)
                    rVal <<= static_cast<sal_Int32>(m_rParent.m_eHScrollMetric);                     // "Window/HorizontalRulerUnit"
            break;
            case  7:
                if(m_rParent.m_bIsVScrollMetricSet)
                    rVal <<= static_cast<sal_Int32>(m_rParent.m_eVScrollMetric);                     // "Window/VerticalRulerUnit"
            break;
            case  8: rVal <<= m_rParent.IsSmoothScroll(); break;                      // "Window/SmoothScroll",
            case  9: rVal <<= static_cast<sal_Int32>(m_rParent.GetZoom()); break;                  // "Zoom/Value",
            case 10: rVal <<= static_cast<sal_Int32>(m_rParent.GetZoomType()); break;              // "Zoom/Type",
            case 11: rVal <<= m_rParent.IsAlignMathObjectsToBaseline(); break;        // "Other/IsAlignMathObjectsToBaseline"
            case 12: rVal <<= static_cast<sal_Int32>(m_rParent.GetMetric()); break;                // "Other/MeasureUnit",
            case 13: rVal <<= m_rParent.GetDefTabInMm100(); break;// "Other/TabStop",
            case 14: rVal <<= m_rParent.IsVRulerRight(); break;                       // "Window/IsVerticalRulerRight",
            case 15: rVal <<= static_cast<sal_Int32>(m_rParent.GetViewLayoutColumns()); break;     // "ViewLayout/Columns",
            case 16: rVal <<= m_rParent.IsViewLayoutBookMode(); break;                // "ViewLayout/BookMode",
            case 17: rVal <<= m_rParent.IsSquaredPageMode(); break;                   // "Other/IsSquaredPageMode",
            case 18: rVal <<= m_rParent.IsApplyCharUnit(); break;                     // "Other/ApplyCharUnit",
            case 19: rVal <<= m_rParent.IsShowScrollBarTips(); break;                 // "Window/ShowScrollBarTips",
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
                case  0: m_rParent.SetCrossHair(bSet); break;// "Line/Guide",
                case  1: m_rParent.SetViewHScrollBar(bSet); break;// "Window/HorizontalScroll",
                case  2: m_rParent.SetViewVScrollBar(bSet); break;// "Window/VerticalScroll",
                case  3: m_rParent.SetViewAnyRuler(bSet);break; // "Window/ShowRulers"
                case  4: m_rParent.SetViewHRuler(bSet); break;// "Window/HorizontalRuler",
                case  5: m_rParent.SetViewVRuler(bSet); break;// "Window/VerticalRuler",
                case  6:
                {
                    m_rParent.m_bIsHScrollMetricSet = true;
                    m_rParent.m_eHScrollMetric = static_cast<FieldUnit>(nInt32Val);  // "Window/HorizontalRulerUnit"
                }
                break;
                case  7:
                {
                    m_rParent.m_bIsVScrollMetricSet = true;
                    m_rParent.m_eVScrollMetric = static_cast<FieldUnit>(nInt32Val); // "Window/VerticalRulerUnit"
                }
                break;
                case  8: m_rParent.SetSmoothScroll(bSet); break;// "Window/SmoothScroll",
                case  9: m_rParent.SetZoom( static_cast< sal_uInt16 >(nInt32Val) ); break;// "Zoom/Value",
                case 10: m_rParent.SetZoomType( static_cast< SvxZoomType >(nInt32Val) ); break;// "Zoom/Type",
                case 11: m_rParent.SetAlignMathObjectsToBaseline(bSet, true); break;// "Other/IsAlignMathObjectsToBaseline"
                case 12: m_rParent.SetMetric(static_cast<FieldUnit>(nInt32Val), true); break;// "Other/MeasureUnit",
                case 13: m_rParent.SetDefTabInMm100(nInt32Val, true); break;// "Other/TabStop",
                case 14: m_rParent.SetVRulerRight(bSet); break;// "Window/IsVerticalRulerRight",
                case 15: m_rParent.SetViewLayoutColumns( o3tl::narrowing<sal_uInt16>(nInt32Val) ); break;// "ViewLayout/Columns",
                case 16: m_rParent.SetViewLayoutBookMode(bSet); break;// "ViewLayout/BookMode",
                case 17: m_rParent.SetDefaultPageMode(bSet,true); break;// "Other/IsSquaredPageMode",
                case 18: m_rParent.SetApplyCharUnit(bSet, true); break;// "Other/ApplyUserChar"
                case 19: m_rParent.SetShowScrollBarTips(bSet); break;// "Window/ShowScrollBarTips",
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
    ConfigItem(bIsWeb ? u"Office.WriterWeb/Grid"_ustr :  u"Office.Writer/Grid"_ustr,
        ConfigItemMode::ReleaseTree),
    m_rParent(rPar)
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
            case  0: pValues[nProp] <<= m_rParent.IsSnap(); break;//      "Option/SnapToGrid",
            case  1: pValues[nProp] <<= m_rParent.IsGridVisible(); break;//"Option/VisibleGrid",
            case  2: pValues[nProp] <<= m_rParent.IsSynchronize(); break;//  "Option/Synchronize",
            case  3: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(m_rParent.GetSnapSize().Width())); break;//      "Resolution/XAxis",
            case  4: pValues[nProp] <<= static_cast<sal_Int32>(convertTwipToMm100(m_rParent.GetSnapSize().Height())); break;//      "Resolution/YAxis",
            case  5: pValues[nProp] <<= static_cast<sal_Int16>(m_rParent.GetDivisionX()); break;//   "Subdivision/XAxis",
            case  6: pValues[nProp] <<= static_cast<sal_Int16>(m_rParent.GetDivisionY()); break;//   "Subdivision/YAxis"
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

    Size aSnap(m_rParent.GetSnapSize());
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
                case  0: m_rParent.SetSnap(bSet); break;//        "Option/SnapToGrid",
                case  1: m_rParent.SetGridVisible(bSet); break;//"Option/VisibleGrid",
                case  2: m_rParent.SetSynchronize(bSet); break;//  "Option/Synchronize",
                case  3: aSnap.setWidth( o3tl::toTwips(nSet, o3tl::Length::mm100) ); break;//      "Resolution/XAxis",
                case  4: aSnap.setHeight( o3tl::toTwips(nSet, o3tl::Length::mm100) ); break;//      "Resolution/YAxis",
                case  5: m_rParent.SetDivisionX(static_cast<short>(nSet)); break;//   "Subdivision/XAxis",
                case  6: m_rParent.SetDivisionY(static_cast<short>(nSet)); break;//   "Subdivision/YAxis"
            }
        }
    }
    m_rParent.SetSnapSize(aSnap);
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
    ConfigItem(u"Office.Writer/Cursor"_ustr, ConfigItemMode::ReleaseTree),
    m_rParent(rPar)
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
            case  0: pValues[nProp] <<= m_rParent.IsShadowCursor();                   break; // "DirectCursor/UseDirectCursor",
            case  1: pValues[nProp] <<= static_cast<sal_Int32>(m_rParent.GetShdwCursorFillMode()); break; // "DirectCursor/Insert",
            case  2: pValues[nProp] <<= m_rParent.IsCursorInProtectedArea();          break; // "Option/ProtectedArea"
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
                case  0: m_rParent.SetShadowCursor(bSet);                  break; // "DirectCursor/UseDirectCursor",
                case  1: m_rParent.SetShdwCursorFillMode(static_cast<SwFillMode>(nSet)); break; // "DirectCursor/Insert",
                case  2: m_rParent.SetCursorInProtectedArea(bSet);         break; // "Option/ProtectedArea"
            }
        }
    }
}

void SwCursorConfig::Notify( const css::uno::Sequence< OUString >& ) {}

Sequence<OUString> SwFmtAidsAutoComplConfig::GetPropertyNames()
{
    static const char* aPropNames[] = {
        "EncloseWithCharacters", // 0
    };
    const int nCount = SAL_N_ELEMENTS(aPropNames);
    Sequence<OUString> aNames(nCount);
    OUString* pNames = aNames.getArray();
    for (int i = 0; i < nCount; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);
    return aNames;
}

SwFmtAidsAutoComplConfig::SwFmtAidsAutoComplConfig(SwMasterUsrPref& rPar)
    : ConfigItem(u"Office.Writer/FmtAidsAutocomplete"_ustr, ConfigItemMode::ReleaseTree)
    , m_rParent(rPar)
{
}

SwFmtAidsAutoComplConfig::~SwFmtAidsAutoComplConfig() {}

void SwFmtAidsAutoComplConfig::ImplCommit()
{
    Sequence<OUString> aNames = GetPropertyNames();

    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for (int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch (nProp)
        {
            case 0:
                pValues[nProp] <<= m_rParent.IsEncloseWithCharactersOn();
                break; // "FmtAidsAutocomplete/EncloseWithCharacters"
        }
    }
    PutProperties(aNames, aValues);
}

void SwFmtAidsAutoComplConfig::Load()
{
    Sequence<OUString> aNames = GetPropertyNames();
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if (aValues.getLength() != aNames.getLength())
        return;

    for (int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        if (pValues[nProp].hasValue())
        {
            switch (nProp)
            {
                case 0:
                {
                    bool bSet = false;
                    pValues[nProp] >>= bSet;
                    m_rParent.SetEncloseWithCharactersOn(bSet);
                    break; // "FmtAidsAutocomplete/EncloseWithCharacters"
                }
            }
        }
    }
}

void SwFmtAidsAutoComplConfig::Notify(const css::uno::Sequence<OUString>&) {}

SwWebColorConfig::SwWebColorConfig(SwMasterUsrPref& rPar) :
    ConfigItem(u"Office.WriterWeb/Background"_ustr, ConfigItemMode::ReleaseTree),
    m_rParent(rPar),
    m_aPropNames(1)
{
    m_aPropNames.getArray()[0] = "Color";
}

SwWebColorConfig::~SwWebColorConfig()
{
}

void SwWebColorConfig::ImplCommit()
{
    Sequence<Any> aValues(m_aPropNames.getLength());
    Any* pValues = aValues.getArray();
    for(int nProp = 0; nProp < m_aPropNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case  0: pValues[nProp] <<= m_rParent.GetRetoucheColor();   break;// "Color",
        }
    }
    PutProperties(m_aPropNames, aValues);
}

void SwWebColorConfig::Notify( const css::uno::Sequence< OUString >& ) {}

void SwWebColorConfig::Load()
{
    Sequence<Any> aValues = GetProperties(m_aPropNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == m_aPropNames.getLength(), "GetProperties failed");
    if(aValues.getLength() != m_aPropNames.getLength())
        return;

    for(int nProp = 0; nProp < m_aPropNames.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            switch(nProp)
            {
                case  0:
                    Color nSet;
                    pValues[nProp] >>= nSet; m_rParent.SetRetoucheColor(nSet);
                break;// "Color",
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
