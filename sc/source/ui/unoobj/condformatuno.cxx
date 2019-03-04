/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <condformatuno.hxx>

#include <document.hxx>
#include <conditio.hxx>
#include <colorscale.hxx>
#include <docsh.hxx>
#include <miscuno.hxx>
#include <compiler.hxx>
#include <tokenarray.hxx>

#include <cellsuno.hxx>
#include <convuno.hxx>

#include <vcl/svapp.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#include <com/sun/star/sheet/DataBarAxis.hpp>
#include <com/sun/star/sheet/IconSetType.hpp>
#include <com/sun/star/sheet/ConditionFormatOperator.hpp>
#include <com/sun/star/sheet/DataBarEntryType.hpp>
#include <com/sun/star/sheet/ColorScaleEntryType.hpp>
#include <com/sun/star/sheet/IconSetFormatEntry.hpp>
#include <com/sun/star/sheet/ConditionEntryType.hpp>
#include <com/sun/star/sheet/DateType.hpp>

namespace {

enum CondFormatProperties
{
    ID,
    CondFormat_Range
};

const SfxItemPropertyMapEntry* getCondFormatPropset()
{
    static const SfxItemPropertyMapEntry aCondFormatPropertyMap_Impl[] =
    {
        {OUString("ID"), ID, cppu::UnoType<sal_Int32>::get(), 0, 0},
        {OUString("Range"), CondFormat_Range, cppu::UnoType<sheet::XSheetCellRanges>::get(), 0, 0},
        {OUString(), 0, css::uno::Type(), 0, 0}
    };
    return aCondFormatPropertyMap_Impl;
}

enum ConditionEntryProperties
{
    StyleName,
    Formula1,
    Formula2,
    Operator
};

const SfxItemPropertyMapEntry* getConditionEntryrPropSet()
{
    static const SfxItemPropertyMapEntry aConditionEntryPropertyMap_Impl[] =
    {
        {OUString("StyleName"), StyleName, cppu::UnoType<OUString>::get(), 0, 0},
        {OUString("Formula1"), Formula1, cppu::UnoType<OUString>::get(), 0, 0},
        {OUString("Formula2"), Formula2, cppu::UnoType<OUString>::get(), 0, 0},
        {OUString("Operator"), Operator, cppu::UnoType<decltype(sheet::ConditionFormatOperator::EQUAL)>::get(), 0, 0 },
        {OUString(), 0, css::uno::Type(), 0, 0}
    };
    return aConditionEntryPropertyMap_Impl;
}

struct ConditionEntryApiMap
{
    ScConditionMode const eMode;
    sal_Int32 const nApiMode;
};

static ConditionEntryApiMap const aConditionEntryMap[] =
{
    {ScConditionMode::Equal, sheet::ConditionFormatOperator::EQUAL},
    {ScConditionMode::Less, sheet::ConditionFormatOperator::LESS},
    {ScConditionMode::Greater, sheet::ConditionFormatOperator::GREATER},
    {ScConditionMode::EqLess, sheet::ConditionFormatOperator::LESS_EQUAL},
    {ScConditionMode::EqGreater, sheet::ConditionFormatOperator::GREATER_EQUAL},
    {ScConditionMode::NotEqual, sheet::ConditionFormatOperator::NOT_EQUAL},
    {ScConditionMode::Between, sheet::ConditionFormatOperator::BETWEEN},
    {ScConditionMode::NotBetween, sheet::ConditionFormatOperator::NOT_BETWEEN},
    {ScConditionMode::Duplicate, sheet::ConditionFormatOperator::DUPLICATE},
    {ScConditionMode::NotDuplicate, sheet::ConditionFormatOperator::UNIQUE},
    {ScConditionMode::Direct, sheet::ConditionFormatOperator::EXPRESSION},
    {ScConditionMode::Top10, sheet::ConditionFormatOperator::TOP_N_ELEMENTS},
    {ScConditionMode::Bottom10, sheet::ConditionFormatOperator::BOTTOM_N_ELEMENTS},
    {ScConditionMode::TopPercent, sheet::ConditionFormatOperator::TOP_N_PERCENT},
    {ScConditionMode::BottomPercent, sheet::ConditionFormatOperator::BOTTOM_N_PERCENT},
    {ScConditionMode::AboveAverage, sheet::ConditionFormatOperator::ABOVE_AVERAGE},
    {ScConditionMode::BelowAverage, sheet::ConditionFormatOperator::BELOW_AVERAGE},
    {ScConditionMode::AboveEqualAverage, sheet::ConditionFormatOperator::ABOVE_EQUAL_AVERAGE},
    {ScConditionMode::BelowEqualAverage, sheet::ConditionFormatOperator::BELOW_EQUAL_AVERAGE},
    {ScConditionMode::Error, sheet::ConditionFormatOperator::ERROR},
    {ScConditionMode::NoError, sheet::ConditionFormatOperator::NO_ERROR},
    {ScConditionMode::BeginsWith, sheet::ConditionFormatOperator::BEGINS_WITH},
    {ScConditionMode::EndsWith, sheet::ConditionFormatOperator::ENDS_WITH},
    {ScConditionMode::ContainsText, sheet::ConditionFormatOperator::CONTAINS},
    {ScConditionMode::NotContainsText, sheet::ConditionFormatOperator::NOT_CONTAINS},
    {ScConditionMode::NONE, sheet::ConditionFormatOperator::EQUAL},
};

enum ColorScaleProperties
{
    ColorScaleEntries
};

const SfxItemPropertyMapEntry* getColorScalePropSet()
{
    static const SfxItemPropertyMapEntry aColorScalePropertyMap_Impl[] =
    {
        {OUString("ColorScaleEntries"), ColorScaleEntries, cppu::UnoType<uno::Sequence< sheet::XColorScaleEntry >>::get(), 0, 0 },
        {OUString(), 0, css::uno::Type(), 0, 0}
    };
    return aColorScalePropertyMap_Impl;
}

struct ColorScaleEntryTypeApiMap
{
    ScColorScaleEntryType const eType;
    sal_Int32 const nApiType;
};

static ColorScaleEntryTypeApiMap const aColorScaleEntryTypeMap[] =
{
    { COLORSCALE_MIN, sheet::ColorScaleEntryType::COLORSCALE_MIN },
    { COLORSCALE_MAX, sheet::ColorScaleEntryType::COLORSCALE_MAX },
    { COLORSCALE_VALUE, sheet::ColorScaleEntryType::COLORSCALE_VALUE },
    { COLORSCALE_FORMULA, sheet::ColorScaleEntryType::COLORSCALE_FORMULA },
    { COLORSCALE_PERCENT, sheet::ColorScaleEntryType::COLORSCALE_PERCENT },
    { COLORSCALE_PERCENTILE, sheet::ColorScaleEntryType::COLORSCALE_PERCENTILE }
};

enum DataBarProperties
{
    AxisPosition,
    UseGradient,
    UseNegativeColor,
    DataBar_ShowValue,
    DataBar_Color,
    AxisColor,
    NegativeColor,
    DataBarEntries,
    MinimumLength,
    MaximumLength
};

const SfxItemPropertyMapEntry* getDataBarPropSet()
{
    static const SfxItemPropertyMapEntry aDataBarPropertyMap_Impl[] =
    {
        {OUString("AxisPosition"), AxisPosition, cppu::UnoType<decltype(sheet::DataBarAxis::AXIS_AUTOMATIC)>::get(), 0, 0 },
        {OUString("UseGradient"), UseGradient, cppu::UnoType<bool>::get(), 0, 0 },
        {OUString("UseNegativeColor"), UseNegativeColor, cppu::UnoType<bool>::get(), 0, 0 },
        {OUString("ShowValue"), DataBar_ShowValue, cppu::UnoType<bool>::get(), 0, 0 },
        {OUString("Color"), DataBar_Color, cppu::UnoType<sal_Int32>::get(), 0, 0},
        {OUString("AxisColor"), AxisColor, cppu::UnoType<sal_Int32>::get(), 0, 0},
        {OUString("NegativeColor"), NegativeColor, cppu::UnoType<sal_Int32>::get(), 0, 0},
        {OUString("DataBarEntries"), DataBarEntries, cppu::UnoType<uno::Sequence< sheet::XDataBarEntry >>::get(), 0, 0 },
        {OUString("MinimumLength"), MinimumLength, cppu::UnoType<double>::get(), 0, 0 },
        {OUString("MaximumLength"), MaximumLength, cppu::UnoType<double>::get(), 0, 0 },
        {OUString(), 0, css::uno::Type(), 0, 0}
    };
    return aDataBarPropertyMap_Impl;
}

struct DataBarAxisApiMap
{
    databar::ScAxisPosition const ePos;
    sal_Int32 const nApiPos;
};

static DataBarAxisApiMap const aDataBarAxisMap[] =
{
    { databar::NONE, sheet::DataBarAxis::AXIS_NONE },
    { databar::AUTOMATIC, sheet::DataBarAxis::AXIS_AUTOMATIC },
    { databar::MIDDLE, sheet::DataBarAxis::AXIS_MIDDLE }
};

struct DataBarEntryTypeApiMap
{
    ScColorScaleEntryType const eType;
    sal_Int32 const nApiType;
};

static DataBarEntryTypeApiMap const aDataBarEntryTypeMap[] =
{
    { COLORSCALE_AUTO, sheet::DataBarEntryType::DATABAR_AUTO },
    { COLORSCALE_MIN, sheet::DataBarEntryType::DATABAR_MIN },
    { COLORSCALE_MAX, sheet::DataBarEntryType::DATABAR_MAX },
    { COLORSCALE_VALUE, sheet::DataBarEntryType::DATABAR_VALUE },
    { COLORSCALE_FORMULA, sheet::DataBarEntryType::DATABAR_FORMULA },
    { COLORSCALE_PERCENT, sheet::DataBarEntryType::DATABAR_PERCENT },
    { COLORSCALE_PERCENTILE, sheet::DataBarEntryType::DATABAR_PERCENTILE }
};

enum IconSetProperties
{
    Icons,
    Reverse,
    ShowValue,
    IconSetEntries
};

const SfxItemPropertyMapEntry* getIconSetPropSet()
{
    static const SfxItemPropertyMapEntry aIconSetPropertyMap_Impl[] =
    {
        {OUString("Icons"), Icons, cppu::UnoType<decltype(sheet::IconSetType::ICONSET_3SYMBOLS)>::get(), 0, 0 },
        {OUString("Reverse"), Reverse, cppu::UnoType<bool>::get(), 0, 0 },
        {OUString("ShowValue"), ShowValue, cppu::UnoType<bool>::get(), 0, 0 },
        {OUString("IconSetEntries"), IconSetEntries, cppu::UnoType<uno::Sequence< sheet::XIconSetEntry >>::get(), 0, 0 },
        {OUString(), 0, css::uno::Type(), 0, 0}
    };
    return aIconSetPropertyMap_Impl;
}

struct IconSetTypeApiMap
{
    ScIconSetType const eType;
    sal_Int32 const nApiType;
};

const IconSetTypeApiMap aIconSetApiMap[] =
{
    { IconSet_3Arrows, sheet::IconSetType::ICONSET_3ARROWS },
    { IconSet_3ArrowsGray, sheet::IconSetType::ICONSET_3ARROWS_GRAY },
    { IconSet_3Flags, sheet::IconSetType::ICONSET_3FLAGS },
    { IconSet_3TrafficLights1, sheet::IconSetType::ICONSET_3TRAFFICLIGHTS1 },
    { IconSet_3TrafficLights2, sheet::IconSetType::ICONSET_3TRAFFICLIGHTS2 },
    { IconSet_3Signs, sheet::IconSetType::ICONSET_3SIGNS },
    { IconSet_3Symbols, sheet::IconSetType::ICONSET_3SYMBOLS },
    { IconSet_3Symbols2, sheet::IconSetType::ICONSET_3SYMBOLS2 },
    { IconSet_3Smilies, sheet::IconSetType::ICONSET_3SMILIES },
    { IconSet_3ColorSmilies, sheet::IconSetType::ICONSET_3COLOR_SIMILIES },
    { IconSet_4Arrows, sheet::IconSetType::ICONSET_4ARROWS },
    { IconSet_4ArrowsGray, sheet::IconSetType::ICONSET_4ARROWS_GRAY },
    { IconSet_4Rating, sheet::IconSetType::ICONSET_4RATING },
    { IconSet_4RedToBlack, sheet::IconSetType::ICONSET_4RED_TO_BLACK },
    { IconSet_4TrafficLights, sheet::IconSetType::ICONSET_4TRAFFICLIGHTS },
    { IconSet_5Arrows, sheet::IconSetType::ICONSET_5ARROWS },
    { IconSet_5ArrowsGray, sheet::IconSetType::ICONSET_4ARROWS_GRAY },
    { IconSet_5Ratings, sheet::IconSetType::ICONSET_5RATINGS },
    { IconSet_5Quarters, sheet::IconSetType::ICONSET_5QUARTERS },
};

struct IconSetEntryTypeApiMap
{
    ScColorScaleEntryType const eType;
    sal_Int32 const nApiType;
};

static IconSetEntryTypeApiMap const aIconSetEntryTypeMap[] =
{
    { COLORSCALE_MIN, sheet::IconSetFormatEntry::ICONSET_MIN },
    { COLORSCALE_VALUE, sheet::IconSetFormatEntry::ICONSET_VALUE },
    { COLORSCALE_FORMULA, sheet::IconSetFormatEntry::ICONSET_FORMULA },
    { COLORSCALE_PERCENT, sheet::IconSetFormatEntry::ICONSET_PERCENT },
    { COLORSCALE_PERCENTILE, sheet::IconSetFormatEntry::ICONSET_PERCENTILE }
};

enum DateProperties
{
    Date_StyleName,
    DateType
};

const SfxItemPropertyMapEntry* getCondDatePropSet()
{
    static const SfxItemPropertyMapEntry aCondDatePropertyMap_Impl[] =
    {
        {OUString("StyleName"), StyleName, cppu::UnoType<OUString>::get(), 0, 0},
        {OUString("DateType"), Icons, cppu::UnoType<decltype(sheet::DateType::TODAY)>::get(), 0, 0 },
        {OUString(), 0, css::uno::Type(), 0, 0}
    };
    return aCondDatePropertyMap_Impl;
}

struct DateTypeApiMap
{
    condformat::ScCondFormatDateType const eType;
    sal_Int32 const nApiType;
};

static DateTypeApiMap const aDateTypeApiMap[] =
{
    { condformat::TODAY, sheet::DateType::TODAY },
    { condformat::YESTERDAY, sheet::DateType::YESTERDAY },
    { condformat::TOMORROW, sheet::DateType::TOMORROW },
    { condformat::LAST7DAYS, sheet::DateType::LAST7DAYS },
    { condformat::THISWEEK, sheet::DateType::THISWEEK },
    { condformat::LASTWEEK, sheet::DateType::LASTWEEK },
    { condformat::NEXTWEEK, sheet::DateType::NEXTWEEK },
    { condformat::THISMONTH, sheet::DateType::THISMONTH },
    { condformat::LASTMONTH, sheet::DateType::LASTMONTH },
    { condformat::NEXTMONTH, sheet::DateType::NEXTMONTH },
    { condformat::THISYEAR, sheet::DateType::THISYEAR },
    { condformat::LASTYEAR, sheet::DateType::LASTYEAR },
    { condformat::NEXTYEAR, sheet::DateType::NEXTYEAR }
};

}

ScCondFormatsObj::ScCondFormatsObj(ScDocShell* pDocShell, SCTAB nTab):
    mnTab(nTab),
    mpDocShell(pDocShell)
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScCondFormatsObj::~ScCondFormatsObj()
{
    if (mpDocShell)
        mpDocShell->GetDocument().RemoveUnoObject(*this);
}

void ScCondFormatsObj::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if ( rHint.GetId() == SfxHintId::Dying )
    {
        mpDocShell = nullptr;
    }
}

sal_Int32 ScCondFormatsObj::createByRange(const uno::Reference< sheet::XSheetCellRanges >& xRanges)
{
    SolarMutexGuard aGuard;
    if (!mpDocShell)
        throw lang::IllegalArgumentException();

    if (!xRanges.is())
        throw lang::IllegalArgumentException();

    uno::Sequence<table::CellRangeAddress> aRanges =
        xRanges->getRangeAddresses();

    ScRangeList aCoreRange;
    for (sal_Int32 i = 0, n = aRanges.getLength(); i < n; ++i)
    {
        ScRange aRange;
        ScUnoConversion::FillScRange(aRange, aRanges[i]);
        aCoreRange.Join(aRange);
    }

    if (aCoreRange.empty())
        throw lang::IllegalArgumentException();

    SCTAB nTab = aCoreRange[0].aStart.Tab();

    auto pNewFormat = std::make_unique<ScConditionalFormat>(0, &mpDocShell->GetDocument());
    pNewFormat->SetRange(aCoreRange);
    return mpDocShell->GetDocument().AddCondFormat(std::move(pNewFormat), nTab);
}

void ScCondFormatsObj::removeByID(const sal_Int32 nID)
{
    SolarMutexGuard aGuard;
    ScConditionalFormatList* pFormatList = getCoreObject();
    pFormatList->erase(nID);
}

uno::Sequence<uno::Reference<sheet::XConditionalFormat> > ScCondFormatsObj::getConditionalFormats()
{
    SolarMutexGuard aGuard;
    ScConditionalFormatList* pFormatList = getCoreObject();
    size_t n = pFormatList->size();
    uno::Sequence<uno::Reference<sheet::XConditionalFormat> > aCondFormats(n);
    sal_Int32 i = 0;
    for (const auto& rFormat : *pFormatList)
    {
        uno::Reference<sheet::XConditionalFormat> xCondFormat(new ScCondFormatObj(mpDocShell, this, rFormat->GetKey()));
        aCondFormats[i] = xCondFormat;
        ++i;
    }

    return aCondFormats;
}

sal_Int32 ScCondFormatsObj::getLength()
{
    SolarMutexGuard aGuard;
    ScConditionalFormatList* pFormatList = getCoreObject();
    return pFormatList->size();
}

ScConditionalFormatList* ScCondFormatsObj::getCoreObject()
{
    if (!mpDocShell)
        throw uno::RuntimeException();

    ScConditionalFormatList* pList = mpDocShell->GetDocument().GetCondFormList(mnTab);
    if (!pList)
        throw uno::RuntimeException();

    return pList;
}

namespace {

uno::Reference<beans::XPropertySet> createConditionEntry(const ScFormatEntry* pEntry,
        rtl::Reference<ScCondFormatObj> const & xParent)
{
    switch (pEntry->GetType())
    {
        case ScFormatEntry::Type::Condition:
            return new ScConditionEntryObj(xParent,
                    static_cast<const ScCondFormatEntry*>(pEntry));
        break;
        case ScFormatEntry::Type::Colorscale:
            return new ScColorScaleFormatObj(xParent,
                    static_cast<const ScColorScaleFormat*>(pEntry));
        break;
        case ScFormatEntry::Type::Databar:
            return new ScDataBarFormatObj(xParent,
                    static_cast<const ScDataBarFormat*>(pEntry));
        break;
        case ScFormatEntry::Type::Iconset:
            return new ScIconSetFormatObj(xParent,
                    static_cast<const ScIconSetFormat*>(pEntry));
        break;
        case ScFormatEntry::Type::Date:
            return new ScCondDateFormatObj(xParent,
                    static_cast<const ScCondDateFormatEntry*>(pEntry));
        break;
        default:
        break;
    }
    return uno::Reference<beans::XPropertySet>();
}

}

ScCondFormatObj::ScCondFormatObj(ScDocShell* pDocShell, rtl::Reference<ScCondFormatsObj> const & xCondFormats,
        sal_Int32 nKey):
    mxCondFormatList(xCondFormats),
    mpDocShell(pDocShell),
    maPropSet(getCondFormatPropset()),
    mnKey(nKey)
{
}

ScCondFormatObj::~ScCondFormatObj()
{
}

ScConditionalFormat* ScCondFormatObj::getCoreObject()
{
    ScConditionalFormatList* pList = mxCondFormatList->getCoreObject();
    ScConditionalFormat* pFormat = pList->GetFormat(mnKey);
    if (!pFormat)
        throw uno::RuntimeException();

    return pFormat;
}

ScDocShell* ScCondFormatObj::getDocShell()
{
    return mpDocShell;
}

void ScCondFormatObj::createEntry(const sal_Int32 nType, const sal_Int32 nPos)
{
    SolarMutexGuard aGuard;
    ScConditionalFormat* pFormat = getCoreObject();
    if (nPos > sal_Int32(pFormat->size()))
        throw lang::IllegalArgumentException();

    ScFormatEntry* pNewEntry = nullptr;
    ScDocument* pDoc = &mpDocShell->GetDocument();
    switch (nType)
    {
        case sheet::ConditionEntryType::CONDITION:
            pNewEntry = new ScCondFormatEntry(ScConditionMode::Equal, "", "",
                    pDoc, pFormat->GetRange().GetTopLeftCorner(), "");
        break;
        case sheet::ConditionEntryType::COLORSCALE:
            pNewEntry = new ScColorScaleFormat(pDoc);
            static_cast<ScColorScaleFormat*>(pNewEntry)->EnsureSize();
        break;
        case sheet::ConditionEntryType::DATABAR:
            pNewEntry = new ScDataBarFormat(pDoc);
            static_cast<ScDataBarFormat*>(pNewEntry)->EnsureSize();
        break;
        case sheet::ConditionEntryType::ICONSET:
            pNewEntry = new ScIconSetFormat(pDoc);
            static_cast<ScIconSetFormat*>(pNewEntry)->EnsureSize();
        break;
        case sheet::ConditionEntryType::DATE:
            pNewEntry = new ScCondDateFormatEntry(pDoc);
        break;
        default:
            SAL_WARN("sc", "unknown conditional format type");
            throw lang::IllegalArgumentException();
    }

    pFormat->AddEntry(pNewEntry);
}

void ScCondFormatObj::removeByIndex(const sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    if (getCoreObject()->size() >= size_t(nIndex))
        throw lang::IllegalArgumentException();

    getCoreObject()->RemoveEntry(nIndex);
}

uno::Type ScCondFormatObj::getElementType()
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

sal_Bool ScCondFormatObj::hasElements()
{
    SolarMutexGuard aGuard;
    ScConditionalFormat* pFormat = getCoreObject();
    return !pFormat->IsEmpty();
}

sal_Int32 ScCondFormatObj::getCount()
{
    SolarMutexGuard aGuard;
    ScConditionalFormat* pFormat = getCoreObject();

    return pFormat->size();
}

uno::Any ScCondFormatObj::getByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    if (getCoreObject()->size() <= size_t(nIndex))
        throw lang::IllegalArgumentException();

    const ScFormatEntry* pEntry = getCoreObject()->GetEntry(nIndex);
    uno::Reference<beans::XPropertySet> xCondEntry =
        createConditionEntry(pEntry, this);
    return uno::Any(xCondEntry);
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCondFormatObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap()));
    return aRef;
}

void SAL_CALL ScCondFormatObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = maPropSet.getPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    switch(pEntry->nWID)
    {
        case ID:
            throw lang::IllegalArgumentException();
        break;
        case CondFormat_Range:
        {
            uno::Reference<sheet::XSheetCellRanges> xRange;
            if (aValue >>= xRange)
            {
                ScConditionalFormat* pFormat = getCoreObject();
                uno::Sequence<table::CellRangeAddress> aRanges =
                    xRange->getRangeAddresses();
                ScRangeList aTargetRange;
                for (size_t i = 0, n = aRanges.getLength(); i < n; ++i)
                {
                    ScRange aRange;
                    ScUnoConversion::FillScRange(aRange, aRanges[i]);
                    aTargetRange.Join(aRange);
                }
                pFormat->SetRange(aTargetRange);
            }
        }
        break;
        default:
            SAL_WARN("sc", "unknown property");
    }
}

uno::Any SAL_CALL ScCondFormatObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = maPropSet.getPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    uno::Any aAny;
    switch(pEntry->nWID)
    {
        case ID:
            aAny <<= sal_Int32(getCoreObject()->GetKey());
        break;
        case CondFormat_Range:
        {
            const ScRangeList& rRange = getCoreObject()->GetRange();
            uno::Reference<sheet::XSheetCellRanges> xRange;
            xRange.set(new ScCellRangesObj(mpDocShell, rRange));
            aAny <<= xRange;
        }
        break;
        default:
            SAL_WARN("sc", "unknown property");
    }
    return aAny;
}

void SAL_CALL ScCondFormatObj::addPropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    SAL_WARN("sc", "not implemented");
}

namespace {

bool isObjectStillAlive(const ScConditionalFormat* pFormat, const ScFormatEntry* pEntry)
{
    for(size_t i = 0, n= pFormat->size(); i < n; ++i)
    {
        if (pFormat->GetEntry(i) == pEntry)
            return true;
    }
    return false;
}

}

ScConditionEntryObj::ScConditionEntryObj(rtl::Reference<ScCondFormatObj> const & xParent,
        const ScCondFormatEntry* pFormat):
    mpDocShell(xParent->getDocShell()),
    mxParent(xParent),
    maPropSet(getConditionEntryrPropSet()),
    mpFormat(pFormat)
{
}

ScConditionEntryObj::~ScConditionEntryObj()
{
}

ScCondFormatEntry* ScConditionEntryObj::getCoreObject()
{
    ScConditionalFormat* pFormat = mxParent->getCoreObject();
    if (isObjectStillAlive(pFormat, mpFormat))
        return const_cast<ScCondFormatEntry*>(mpFormat);

    throw lang::IllegalArgumentException();
}

sal_Int32 ScConditionEntryObj::getType()
{
    return sheet::ConditionEntryType::CONDITION;
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScConditionEntryObj::getPropertySetInfo()
{
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScConditionEntryObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = maPropSet.getPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    switch(pEntry->nWID)
    {
        case StyleName:
        {
            OUString aStyleName;
            if ((aValue >>= aStyleName) && !aStyleName.isEmpty())
                getCoreObject()->UpdateStyleName(aStyleName);
        }
        break;
        case Formula1:
        {
            OUString aFormula;
            if ((aValue >>= aFormula) && !aFormula.isEmpty())
            {
                ScCompiler aComp(&mpDocShell->GetDocument(), getCoreObject()->GetSrcPos());
                aComp.SetGrammar(mpDocShell->GetDocument().GetGrammar());
                std::unique_ptr<ScTokenArray> pArr(aComp.CompileString(aFormula));
                getCoreObject()->SetFormula1(*pArr);
            }
        }
        break;
        case Formula2:
        {
            OUString aFormula;
            if ((aValue >>= aFormula) && !aFormula.isEmpty())
            {
                ScCompiler aComp(&mpDocShell->GetDocument(), getCoreObject()->GetSrcPos());
                aComp.SetGrammar(mpDocShell->GetDocument().GetGrammar());
                std::unique_ptr<ScTokenArray> pArr(aComp.CompileString(aFormula));
                getCoreObject()->SetFormula2(*pArr);
            }
        }
        break;
        case Operator:
        {
            sal_Int32 nVal;
            if (aValue >>= nVal)
            {
                for (ConditionEntryApiMap const & rEntry : aConditionEntryMap)
                {
                    if (rEntry.nApiMode == nVal)
                    {
                        getCoreObject()->SetOperation(rEntry.eMode);
                        break;
                    }
                }
            }
        }
        break;
        default:
            SAL_WARN("sc", "unsupported property");
    }
}

uno::Any SAL_CALL ScConditionEntryObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = maPropSet.getPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    uno::Any aAny;
    switch(pEntry->nWID)
    {
        case StyleName:
            aAny <<= getCoreObject()->GetStyle();
        break;
        case Formula1:
        {
            ScAddress aCursor = getCoreObject()->GetSrcPos();
            OUString aFormula = getCoreObject()->GetExpression(aCursor, 0);
            aAny <<= aFormula;
        }
        break;
        case Formula2:
        {
            ScAddress aCursor = getCoreObject()->GetSrcPos();
            OUString aFormula = getCoreObject()->GetExpression(aCursor, 1);
            aAny <<= aFormula;
        }
        break;
        case Operator:
        {
            ScConditionMode eMode = getCoreObject()->GetOperation();
            for (ConditionEntryApiMap const & rEntry : aConditionEntryMap)
            {
                if (rEntry.eMode == eMode)
                {
                    aAny <<= rEntry.nApiMode;
                    break;
                }
            }
        }
        break;
        default:
            SAL_WARN("sc", "unsupported property");
    }
    return aAny;
}

void SAL_CALL ScConditionEntryObj::addPropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScConditionEntryObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScConditionEntryObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScConditionEntryObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    SAL_WARN("sc", "not implemented");
}

ScColorScaleFormatObj::ScColorScaleFormatObj(rtl::Reference<ScCondFormatObj> const & xParent,
        const ScColorScaleFormat* pFormat):
    mxParent(xParent),
    maPropSet(getColorScalePropSet()),
    mpFormat(pFormat)
{
}

ScColorScaleFormatObj::~ScColorScaleFormatObj()
{
}

ScColorScaleFormat* ScColorScaleFormatObj::getCoreObject()
{
    ScConditionalFormat* pFormat = mxParent->getCoreObject();
    if (isObjectStillAlive(pFormat, mpFormat))
        return const_cast<ScColorScaleFormat*>(mpFormat);

    throw lang::IllegalArgumentException();
}

sal_Int32 ScColorScaleFormatObj::getType()
{
    return sheet::ConditionEntryType::COLORSCALE;
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScColorScaleFormatObj::getPropertySetInfo()
{
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

namespace {

void setColorScaleEntry(ScColorScaleEntry* pEntry, uno::Reference<sheet::XColorScaleEntry> const & xEntry)
{
    ScColorScaleEntryType eType = ScColorScaleEntryType();
    sal_Int32 nApiType = xEntry->getType();
    bool bFound = false;
    for (ColorScaleEntryTypeApiMap const & rEntry : aColorScaleEntryTypeMap)
    {
        if (rEntry.nApiType == nApiType)
        {
            eType = rEntry.eType;
            bFound = true;
            break;
        }
    }

    if (!bFound)
        throw lang::IllegalArgumentException();

    pEntry->SetType(eType);
    pEntry->SetColor(Color(xEntry->getColor()));
    switch (eType)
    {
        case COLORSCALE_FORMULA:
            // TODO: Implement
        break;
        default:
        {
            double nVal = xEntry->getFormula().toDouble();
            pEntry->SetValue(nVal);
        }
        break;
    }
}

}

void SAL_CALL ScColorScaleFormatObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = maPropSet.getPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    switch(pEntry->nWID)
    {
        case ColorScaleEntries:
        {
            uno::Sequence<uno::Reference<sheet::XColorScaleEntry> > aEntries;
            if (!(aValue >>= aEntries))
                throw lang::IllegalArgumentException();

            if (aEntries.getLength() < 2)
                throw lang::IllegalArgumentException();

            // TODO: we need to make sure that there are enough entries
            size_t n = size_t(aEntries.getLength());
            for (size_t i = 0; i < n; ++i)
            {
                setColorScaleEntry(getCoreObject()->GetEntry(i), aEntries[i]);
            }

        }
        break;
        default:
            SAL_WARN("sc", "unknown property");
    }
}

uno::Any SAL_CALL ScColorScaleFormatObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = maPropSet.getPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    uno::Any aAny;

    switch(pEntry->nWID)
    {
        case ColorScaleEntries:
        {
            uno::Sequence<uno::Reference<sheet::XColorScaleEntry> > aEntries(getCoreObject()->size());
            for (size_t i = 0; i < getCoreObject()->size(); ++i)
            {
                aEntries[i] = new ScColorScaleEntryObj(this, i);
            }
            aAny <<= aEntries;
        }
        break;
        default:
            SAL_WARN("sc", "unknown property");
    }

    return aAny;
}

void SAL_CALL ScColorScaleFormatObj::addPropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScColorScaleFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScColorScaleFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScColorScaleFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    SAL_WARN("sc", "not implemented");
}

ScColorScaleEntryObj::ScColorScaleEntryObj(rtl::Reference<ScColorScaleFormatObj> const & xParent,
        size_t nPos):
    mxParent(xParent),
    mnPos(nPos)
{
}

ScColorScaleEntryObj::~ScColorScaleEntryObj()
{
}

ScColorScaleEntry* ScColorScaleEntryObj::getCoreObject()
{
    ScColorScaleFormat* pFormat = mxParent->getCoreObject();
    if (pFormat->size() <= mnPos)
        throw lang::IllegalArgumentException();

    return pFormat->GetEntry(mnPos);
}

sal_Int32 ScColorScaleEntryObj::getColor()
{
    Color aColor = getCoreObject()->GetColor();
    return sal_Int32(aColor);
}

void ScColorScaleEntryObj::setColor(sal_Int32 aColor)
{
    getCoreObject()->SetColor(Color(aColor));
}

sal_Int32 ScColorScaleEntryObj::getType()
{
    ScColorScaleEntry* pEntry = getCoreObject();
    for (ColorScaleEntryTypeApiMap const & rEntry : aColorScaleEntryTypeMap)
    {
        if (rEntry.eType == pEntry->GetType())
        {
            return rEntry.nApiType;
        }
    }

    throw lang::IllegalArgumentException();
}

void ScColorScaleEntryObj::setType(sal_Int32 nType)
{
    ScColorScaleEntry* pEntry = getCoreObject();
    for (ColorScaleEntryTypeApiMap const & rEntry : aColorScaleEntryTypeMap)
    {
        if (rEntry.nApiType == nType)
        {
            pEntry->SetType(rEntry.eType);
            return;
        }
    }
    throw lang::IllegalArgumentException();
}

OUString ScColorScaleEntryObj::getFormula()
{
    ScColorScaleEntry* pEntry = getCoreObject();
    switch (pEntry->GetType())
    {
        case COLORSCALE_FORMULA:
            // TODO: Implement
        break;
        default:
            return OUString::number(pEntry->GetValue());
    }

    return OUString();
}

void ScColorScaleEntryObj::setFormula(const OUString& rFormula)
{
    ScColorScaleEntry* pEntry = getCoreObject();
    switch (pEntry->GetType())
    {
        case COLORSCALE_FORMULA:
            // TODO: Implement
            // pEntry->SetFormula(rFormula);
        break;
        default:
            pEntry->SetValue(rFormula.toDouble());
        break;
    }
}


ScDataBarFormatObj::ScDataBarFormatObj(rtl::Reference<ScCondFormatObj> const & xParent,
        const ScDataBarFormat* pFormat):
    mxParent(xParent),
    maPropSet(getDataBarPropSet()),
    mpFormat(pFormat)
{
}

ScDataBarFormatObj::~ScDataBarFormatObj()
{
}

ScDataBarFormat* ScDataBarFormatObj::getCoreObject()
{
    ScConditionalFormat* pFormat = mxParent->getCoreObject();
    if (isObjectStillAlive(pFormat, mpFormat))
        return const_cast<ScDataBarFormat*>(mpFormat);

    throw lang::IllegalArgumentException();
}

sal_Int32 ScDataBarFormatObj::getType()
{
    return sheet::ConditionEntryType::DATABAR;
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDataBarFormatObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

namespace {

void setDataBarEntry(ScColorScaleEntry* pEntry, uno::Reference<sheet::XDataBarEntry> const & xEntry)
{
    ScColorScaleEntryType eType = ScColorScaleEntryType();
    sal_Int32 nApiType = xEntry->getType();
    bool bFound = false;
    for (DataBarEntryTypeApiMap const & rEntry : aDataBarEntryTypeMap)
    {
        if (rEntry.nApiType == nApiType)
        {
            eType = rEntry.eType;
            bFound = true;
            break;
        }
    }

    if (!bFound)
        throw lang::IllegalArgumentException();

    pEntry->SetType(eType);
    switch (eType)
    {
        case COLORSCALE_FORMULA:
            // TODO: Implement
        break;
        default:
        {
            double nVal = xEntry->getFormula().toDouble();
            pEntry->SetValue(nVal);
        }
        break;
    }
}

}

void SAL_CALL ScDataBarFormatObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = maPropSet.getPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    switch(pEntry->nWID)
    {
        case AxisPosition:
        {
            sal_Int32 nVal;
            if (aValue >>= nVal)
            {
                for (DataBarAxisApiMap const & rEntry : aDataBarAxisMap)
                {
                    if (rEntry.nApiPos == nVal)
                    {
                        getCoreObject()->GetDataBarData()->meAxisPosition =
                            rEntry.ePos;
                        break;
                    }
                }
            }
        }
        break;
        case UseGradient:
        {
            bool bUseGradient = true;
            if (aValue >>= bUseGradient)
            {
                getCoreObject()->GetDataBarData()->mbGradient = bUseGradient;
            }
        }
        break;
        case UseNegativeColor:
        {
            bool bUseNegativeColor = false;
            if (aValue >>= bUseNegativeColor)
            {
                getCoreObject()->GetDataBarData()->mbNeg = bUseNegativeColor;
                if (bUseNegativeColor && !getCoreObject()->GetDataBarData()->mpNegativeColor)
                {
                    getCoreObject()->GetDataBarData()->mpNegativeColor.reset(new Color(COL_AUTO));
                }
            }
        }
        break;
        case DataBar_ShowValue:
        {
            bool bShowValue = true;
            if (aValue >>= bShowValue)
            {
                getCoreObject()->GetDataBarData()->mbOnlyBar = !bShowValue;
            }
        }
        break;
        case DataBar_Color:
        {
            Color nColor = COL_AUTO;
            if (aValue >>= nColor)
            {
                getCoreObject()->GetDataBarData()->maPositiveColor = nColor;
            }
        }
        break;
        case AxisColor:
        {
            Color nAxisColor = COL_AUTO;
            if (aValue >>= nAxisColor)
            {
                getCoreObject()->GetDataBarData()->maAxisColor = nAxisColor;
            }
        }
        break;
        case NegativeColor:
        {
            Color nNegativeColor = COL_AUTO;
            if (!(aValue >>= nNegativeColor) || !getCoreObject()->GetDataBarData()->mbNeg)
                throw lang::IllegalArgumentException();

            (*getCoreObject()->GetDataBarData()->mpNegativeColor) = nNegativeColor;

        }
        break;
        case DataBarEntries:
        {
            uno::Sequence<uno::Reference<sheet::XDataBarEntry> > aEntries;
            if (!(aValue >>= aEntries))
                throw lang::IllegalArgumentException();

            if (aEntries.getLength() != 2)
                throw lang::IllegalArgumentException();

            setDataBarEntry(getCoreObject()->GetDataBarData()->mpLowerLimit.get(),
                    aEntries[0]);
            setDataBarEntry(getCoreObject()->GetDataBarData()->mpUpperLimit.get(),
                    aEntries[1]);

        }
        break;
        case MinimumLength:
        {
            double nLength = 0;
            if (!(aValue >>= nLength) || nLength >= 100 || nLength < 0)
                throw lang::IllegalArgumentException();
            getCoreObject()->GetDataBarData()->mnMinLength = nLength;

        }
        break;
        case MaximumLength:
        {
            double nLength = 0;
            if (!(aValue >>= nLength) || nLength > 100 || nLength <= 0)
                throw lang::IllegalArgumentException();
            getCoreObject()->GetDataBarData()->mnMaxLength = nLength;

        }
        break;
    }
}

uno::Any SAL_CALL ScDataBarFormatObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = maPropSet.getPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    uno::Any aAny;
    switch(pEntry->nWID)
    {
        case AxisPosition:
        {
            databar::ScAxisPosition ePos = getCoreObject()->GetDataBarData()->meAxisPosition;
            sal_Int32 nApiPos = sheet::DataBarAxis::AXIS_NONE;
            for (DataBarAxisApiMap const & rEntry : aDataBarAxisMap)
            {
                if (rEntry.ePos == ePos)
                {
                    nApiPos = rEntry.nApiPos;
                }
            }

            aAny <<= nApiPos;
        }
        break;
        case UseGradient:
        {
           aAny <<= getCoreObject()->GetDataBarData()->mbGradient;
        }
        break;
        case UseNegativeColor:
        {
           aAny <<= getCoreObject()->GetDataBarData()->mbNeg;
        }
        break;
        case DataBar_ShowValue:
        {
            aAny <<= !getCoreObject()->GetDataBarData()->mbOnlyBar;
        }
        break;
        case DataBar_Color:
        {
            aAny <<= getCoreObject()->GetDataBarData()->maPositiveColor;
        }
        break;
        case AxisColor:
        {
            aAny <<= getCoreObject()->GetDataBarData()->maAxisColor;
        }
        break;
        case NegativeColor:
        {
            if (getCoreObject()->GetDataBarData()->mbNeg && getCoreObject()->GetDataBarData()->mpNegativeColor)
            {
                aAny <<= *getCoreObject()->GetDataBarData()->mpNegativeColor;
            }
        }
        break;
        case DataBarEntries:
        {
            uno::Sequence<uno::Reference<sheet::XDataBarEntry> > aEntries(2);
            aEntries[0] = new ScDataBarEntryObj(this, 0);
            aEntries[1] = new ScDataBarEntryObj(this, 1);
            aAny <<= aEntries;
        }
        break;
    }
    return aAny;
}

void SAL_CALL ScDataBarFormatObj::addPropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScDataBarFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScDataBarFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScDataBarFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    SAL_WARN("sc", "not implemented");
}

ScDataBarEntryObj::ScDataBarEntryObj(rtl::Reference<ScDataBarFormatObj> const & xParent,
        size_t nPos):
    mxParent(xParent),
    mnPos(nPos)
{
}

ScDataBarEntryObj::~ScDataBarEntryObj()
{
}

ScColorScaleEntry* ScDataBarEntryObj::getCoreObject()
{
    ScDataBarFormat* pFormat = mxParent->getCoreObject();
    ScColorScaleEntry* pEntry;
    if (mnPos == 0)
        pEntry = pFormat->GetDataBarData()->mpLowerLimit.get();
    else
        pEntry = pFormat->GetDataBarData()->mpUpperLimit.get();

    return pEntry;
}

sal_Int32 ScDataBarEntryObj::getType()
{
    ScColorScaleEntry* pEntry = getCoreObject();
    for (DataBarEntryTypeApiMap const & rEntry : aDataBarEntryTypeMap)
    {
        if (rEntry.eType == pEntry->GetType())
        {
            return rEntry.nApiType;
        }
    }

    throw lang::IllegalArgumentException();
}

void ScDataBarEntryObj::setType(sal_Int32 nType)
{
    ScColorScaleEntry* pEntry = getCoreObject();
    for (DataBarEntryTypeApiMap const & rEntry : aDataBarEntryTypeMap)
    {
        if (rEntry.nApiType == nType)
        {
            pEntry->SetType(rEntry.eType);
            return;
        }
    }
    throw lang::IllegalArgumentException();
}

OUString ScDataBarEntryObj::getFormula()
{
    ScColorScaleEntry* pEntry = getCoreObject();
    switch (pEntry->GetType())
    {
        case COLORSCALE_FORMULA:
            // TODO: Implement
        break;
        default:
            return OUString::number(pEntry->GetValue());
    }

    return OUString();
}

void ScDataBarEntryObj::setFormula(const OUString& rFormula)
{
    ScColorScaleEntry* pEntry = getCoreObject();
    switch (pEntry->GetType())
    {
        case COLORSCALE_FORMULA:
            // TODO: Implement
            // pEntry->SetFormula(rFormula);
        break;
        default:
            pEntry->SetValue(rFormula.toDouble());
        break;
    }
}


ScIconSetFormatObj::ScIconSetFormatObj(rtl::Reference<ScCondFormatObj> const & xParent,
        const ScIconSetFormat* pFormat):
    mxParent(xParent),
    maPropSet(getIconSetPropSet()),
    mpFormat(pFormat)
{
}

ScIconSetFormatObj::~ScIconSetFormatObj()
{
}

ScIconSetFormat* ScIconSetFormatObj::getCoreObject()
{
    ScConditionalFormat* pFormat = mxParent->getCoreObject();
    if (isObjectStillAlive(pFormat, mpFormat))
        return const_cast<ScIconSetFormat*>(mpFormat);

    throw lang::IllegalArgumentException();
}

sal_Int32 ScIconSetFormatObj::getType()
{
    return sheet::ConditionEntryType::ICONSET;
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScIconSetFormatObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

namespace {

void setIconSetEntry(ScIconSetFormat* pFormat, uno::Reference<sheet::XIconSetEntry> const & xEntry, size_t nPos)
{
    ScIconSetFormatData* pData = pFormat->GetIconSetData();
    ScColorScaleEntryType eType = ScColorScaleEntryType();
    sal_Int32 nApiType = xEntry->getType();
    bool bFound = false;
    for (IconSetEntryTypeApiMap const & rEntry : aIconSetEntryTypeMap)
    {
        if (rEntry.nApiType == nApiType)
        {
            eType = rEntry.eType;
            bFound = true;
            break;
        }
    }

    if (!bFound)
        throw lang::IllegalArgumentException();

    pData->m_Entries[nPos]->SetType(eType);
    switch (eType)
    {
        case COLORSCALE_FORMULA:
            // TODO: Implement
        break;
        default:
        {
            double nVal = xEntry->getFormula().toDouble();
            pData->m_Entries[nPos]->SetValue(nVal);
        }
        break;
    }
}

}

void SAL_CALL ScIconSetFormatObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = maPropSet.getPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    switch(pEntry->nWID)
    {
        case ShowValue:
        {
            bool bShowValue = true;
            aValue >>= bShowValue;
            getCoreObject()->GetIconSetData()->mbShowValue = bShowValue;
        }
        break;
        case Reverse:
        {
            bool bReverse = false;
            aValue >>= bReverse;
            getCoreObject()->GetIconSetData()->mbReverse = bReverse;
        }
        break;
        case Icons:
        {
            sal_Int32 nApiType = -1;
            aValue >>= nApiType;
            ScIconSetType eType = IconSet_3Arrows;
            bool bFound = false;
            for (const IconSetTypeApiMap & rEntry : aIconSetApiMap)
            {
                if (rEntry.nApiType == nApiType)
                {
                    eType = rEntry.eType;
                    bFound = true;
                    break;
                }
            }

            if (!bFound)
            {
                throw lang::IllegalArgumentException();
            }

            // TODO: we need to make sure that there are enough entries
            getCoreObject()->GetIconSetData()->eIconSetType = eType;
        }
        break;
        case IconSetEntries:
        {
            uno::Sequence<uno::Reference<sheet::XIconSetEntry> > aEntries;
            if (!(aValue >>= aEntries))
                throw lang::IllegalArgumentException();

            // TODO: we need to check that the number of entries
            // corresponds to the icon type
            sal_Int32 nLength = aEntries.getLength();
            for (size_t i = 0; i < size_t(nLength); ++i)
            {
                setIconSetEntry(getCoreObject(), aEntries[i], i);
            }

        }
        break;
        default:
        break;
    }
}

uno::Any SAL_CALL ScIconSetFormatObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = maPropSet.getPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    uno::Any aAny;

    switch(pEntry->nWID)
    {
        case ShowValue:
            aAny <<= getCoreObject()->GetIconSetData()->mbShowValue;
        break;
        case Reverse:
            aAny <<= getCoreObject()->GetIconSetData()->mbReverse;
        break;
        case Icons:
        {
            ScIconSetType eType = getCoreObject()->GetIconSetData()->eIconSetType;
            for (const IconSetTypeApiMap & rEntry : aIconSetApiMap)
            {
                if (rEntry.eType == eType)
                {
                    aAny <<= rEntry.nApiType;
                    break;
                }
            }
        }
        break;
        case IconSetEntries:
        {
            size_t nSize = getCoreObject()->size();
            uno::Sequence<uno::Reference<sheet::XIconSetEntry> > aEntries(nSize);
            for (size_t i = 0; i < nSize; ++i)
            {
                aEntries[i] = new ScIconSetEntryObj(this, i);
            }
            aAny <<= aEntries;
        }
        break;
        default:
            SAL_WARN("sc", "unknown property");
    }
    return aAny;
}

void SAL_CALL ScIconSetFormatObj::addPropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScIconSetFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScIconSetFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScIconSetFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    SAL_WARN("sc", "not implemented");
}

ScIconSetEntryObj::ScIconSetEntryObj(rtl::Reference<ScIconSetFormatObj> const & xParent,
        size_t nPos):
    mxParent(xParent),
    mnPos(nPos)
{
}

ScIconSetEntryObj::~ScIconSetEntryObj()
{
}

ScColorScaleEntry* ScIconSetEntryObj::getCoreObject()
{
    ScIconSetFormat* pFormat = mxParent->getCoreObject();
    if (pFormat->GetIconSetData()->m_Entries.size() <= mnPos)
        throw lang::IllegalArgumentException();

    return pFormat->GetIconSetData()->m_Entries[mnPos].get();
}

sal_Int32 ScIconSetEntryObj::getType()
{
    ScColorScaleEntry* pEntry = getCoreObject();
    // the first entry always is minimum
    if (mnPos == 0)
        return sheet::IconSetFormatEntry::ICONSET_MIN;

    for (IconSetEntryTypeApiMap const & rEntry : aIconSetEntryTypeMap)
    {
        if (rEntry.eType == pEntry->GetType())
        {
            return rEntry.nApiType;
        }
    }

    throw lang::IllegalArgumentException();
}

void ScIconSetEntryObj::setType(sal_Int32 nType)
{
    // first entry is always MIN
    if (mnPos == 0)
        return;

    ScColorScaleEntry* pEntry = getCoreObject();
    for (IconSetEntryTypeApiMap const & rEntry : aIconSetEntryTypeMap)
    {
        if (rEntry.nApiType == nType)
        {
            pEntry->SetType(rEntry.eType);
            return;
        }
    }
    throw lang::IllegalArgumentException();
}

OUString ScIconSetEntryObj::getFormula()
{
    ScColorScaleEntry* pEntry = getCoreObject();
    switch (pEntry->GetType())
    {
        case COLORSCALE_FORMULA:
            // TODO: Implement
        break;
        default:
            return OUString::number(pEntry->GetValue());
    }

    return OUString();
}

void ScIconSetEntryObj::setFormula(const OUString& rFormula)
{
    ScColorScaleEntry* pEntry = getCoreObject();
    switch (pEntry->GetType())
    {
        case COLORSCALE_FORMULA:
            // TODO: Implement
            // pEntry->SetFormula(rFormula);
        break;
        default:
            pEntry->SetValue(rFormula.toDouble());
        break;
    }
}

ScCondDateFormatObj::ScCondDateFormatObj(rtl::Reference<ScCondFormatObj> const & xParent,
        const ScCondDateFormatEntry* pFormat):
    mxParent(xParent),
    maPropSet(getCondDatePropSet()),
    mpFormat(pFormat)
{
}

ScCondDateFormatObj::~ScCondDateFormatObj()
{
}

ScCondDateFormatEntry* ScCondDateFormatObj::getCoreObject()
{
    ScConditionalFormat* pFormat = mxParent->getCoreObject();
    if (isObjectStillAlive(pFormat, mpFormat))
        return const_cast<ScCondDateFormatEntry*>(mpFormat);

    throw lang::IllegalArgumentException();
}

sal_Int32 ScCondDateFormatObj::getType()
{
    return sheet::ConditionEntryType::DATE;
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCondDateFormatObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScCondDateFormatObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = maPropSet.getPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    switch(pEntry->nWID)
    {
        case Date_StyleName:
        {
            OUString aStyleName;
            if (!(aValue >>= aStyleName))
                throw lang::IllegalArgumentException();

            getCoreObject()->SetStyleName(aStyleName);

        }
        break;
        case DateType:
        {
            sal_Int32 nApiType = -1;
            if (!(aValue >>= nApiType))
                throw lang::IllegalArgumentException();

            for (DateTypeApiMap const & rEntry : aDateTypeApiMap)
            {
                if (rEntry.nApiType == nApiType)
                {
                    getCoreObject()->SetDateType(rEntry.eType);
                    break;
                }
            }
        }
        break;
        default:
        break;
    }
}

uno::Any SAL_CALL ScCondDateFormatObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    const SfxItemPropertyMap& rPropertyMap = maPropSet.getPropertyMap();     // from derived class
    const SfxItemPropertySimpleEntry* pEntry = rPropertyMap.getByName( aPropertyName );
    if ( !pEntry )
        throw beans::UnknownPropertyException();

    uno::Any aAny;

    switch(pEntry->nWID)
    {
        case Date_StyleName:
        {
            OUString aStyleName = getCoreObject()->GetStyleName();
            aAny <<= aStyleName;
        }
        break;
        case DateType:
        {
            condformat::ScCondFormatDateType eType = getCoreObject()->GetDateType();
            for (DateTypeApiMap const & rEntry : aDateTypeApiMap)
            {
                if (rEntry.eType == eType)
                {
                    aAny <<= rEntry.nApiType;
                    break;
                }
            }
        }
        break;
        default:
            SAL_WARN("sc", "unknown property");
    }
    return aAny;
}

void SAL_CALL ScCondDateFormatObj::addPropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondDateFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondDateFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondDateFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
{
    SAL_WARN("sc", "not implemented");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
