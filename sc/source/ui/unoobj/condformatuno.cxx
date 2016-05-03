/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "condformatuno.hxx"

#include "document.hxx"
#include "conditio.hxx"
#include "colorscale.hxx"
#include "docsh.hxx"
#include "miscuno.hxx"
#include "compiler.hxx"
#include "tokenarray.hxx"

#include "cellsuno.hxx"
#include "convuno.hxx"

#include <vcl/svapp.hxx>
#include <rtl/ustring.hxx>

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
    ScConditionMode eMode;
    sal_Int32 nApiMode;
};

ConditionEntryApiMap aConditionEntryMap[] =
{
    {SC_COND_EQUAL, sheet::ConditionFormatOperator::EQUAL},
    {SC_COND_LESS, sheet::ConditionFormatOperator::LESS},
    {SC_COND_GREATER, sheet::ConditionFormatOperator::GREATER},
    {SC_COND_EQLESS, sheet::ConditionFormatOperator::LESS_EQUAL},
    {SC_COND_EQGREATER, sheet::ConditionFormatOperator::GREATER_EQUAL},
    {SC_COND_NOTEQUAL, sheet::ConditionFormatOperator::NOT_EQUAL},
    {SC_COND_BETWEEN, sheet::ConditionFormatOperator::BETWEEN},
    {SC_COND_NOTBETWEEN, sheet::ConditionFormatOperator::NOT_BETWEEN},
    {SC_COND_DUPLICATE, sheet::ConditionFormatOperator::DUPLICATE},
    {SC_COND_NOTDUPLICATE, sheet::ConditionFormatOperator::UNIQUE},
    {SC_COND_DIRECT, sheet::ConditionFormatOperator::EXPRESSION},
    {SC_COND_TOP10, sheet::ConditionFormatOperator::TOP_N_ELEMENTS},
    {SC_COND_BOTTOM10, sheet::ConditionFormatOperator::BOTTOM_N_ELEMENTS},
    {SC_COND_TOP_PERCENT, sheet::ConditionFormatOperator::TOP_N_PERCENT},
    {SC_COND_BOTTOM_PERCENT, sheet::ConditionFormatOperator::BOTTOM_N_PERCENT},
    {SC_COND_ABOVE_AVERAGE, sheet::ConditionFormatOperator::ABOVE_AVERAGE},
    {SC_COND_BELOW_AVERAGE, sheet::ConditionFormatOperator::BELOW_AVERAGE},
    {SC_COND_ABOVE_EQUAL_AVERAGE, sheet::ConditionFormatOperator::ABOVE_EQUAL_AVERAGE},
    {SC_COND_BELOW_EQUAL_AVERAGE, sheet::ConditionFormatOperator::BELOW_EQUAL_AVERAGE},
    {SC_COND_ERROR, sheet::ConditionFormatOperator::ERROR},
    {SC_COND_NOERROR, sheet::ConditionFormatOperator::NO_ERROR},
    {SC_COND_BEGINS_WITH, sheet::ConditionFormatOperator::BEGINS_WITH},
    {SC_COND_ENDS_WITH, sheet::ConditionFormatOperator::ENDS_WITH},
    {SC_COND_CONTAINS_TEXT, sheet::ConditionFormatOperator::CONTAINS},
    {SC_COND_NOT_CONTAINS_TEXT, sheet::ConditionFormatOperator::NOT_CONTAINS},
    {SC_COND_NONE, sheet::ConditionFormatOperator::EQUAL},
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
    ScColorScaleEntryType eType;
    sal_Int32 nApiType;
};

ColorScaleEntryTypeApiMap aColorScaleEntryTypeMap[] =
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
    databar::ScAxisPosition ePos;
    sal_Int32 nApiPos;
};

DataBarAxisApiMap aDataBarAxisMap[] =
{
    { databar::NONE, sheet::DataBarAxis::AXIS_NONE },
    { databar::AUTOMATIC, sheet::DataBarAxis::AXIS_AUTOMATIC },
    { databar::MIDDLE, sheet::DataBarAxis::AXIS_MIDDLE }
};

struct DataBarEntryTypeApiMap
{
    ScColorScaleEntryType eType;
    sal_Int32 nApiType;
};

DataBarEntryTypeApiMap aDataBarEntryTypeMap[] =
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
    ScIconSetType eType;
    sal_Int32 nApiType;
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
    ScColorScaleEntryType eType;
    sal_Int32 nApiType;
};

IconSetEntryTypeApiMap aIconSetEntryTypeMap[] =
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
    condformat::ScCondFormatDateType eType;
    sal_Int32 nApiType;
};

DateTypeApiMap aDateTypeApiMap[] =
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
    if ( dynamic_cast<const SfxSimpleHint*>(&rHint) &&
            static_cast<const SfxSimpleHint&>(rHint).GetId() == SFX_HINT_DYING )
    {
        mpDocShell = nullptr;       // ungueltig geworden
    }
}

sal_Int32 ScCondFormatsObj::createByRange(const uno::Reference< sheet::XSheetCellRanges >& xRanges)
    throw(uno::RuntimeException, std::exception)
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

    SCTAB nTab = aCoreRange[0]->aStart.Tab();

    ScConditionalFormat* pNewFormat = new ScConditionalFormat(0, &mpDocShell->GetDocument());
    pNewFormat->SetRange(aCoreRange);
    return mpDocShell->GetDocument().AddCondFormat(pNewFormat, nTab);
}

void ScCondFormatsObj::removeByID(const sal_Int32 nID)
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScConditionalFormatList* pFormatList = getCoreObject();
    pFormatList->erase(nID);
}

uno::Sequence<uno::Reference<sheet::XConditionalFormat> > ScCondFormatsObj::getConditionalFormats()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScConditionalFormatList* pFormatList = getCoreObject();
    size_t n = pFormatList->size();
    uno::Sequence<uno::Reference<sheet::XConditionalFormat> > aCondFormats(n);
    sal_Int32 i = 0;
    for (ScConditionalFormatList::const_iterator itr = pFormatList->begin(); itr != pFormatList->end(); ++itr, ++i) {
        uno::Reference<sheet::XConditionalFormat> xCondFormat(new ScCondFormatObj(mpDocShell, this, (*itr)->GetKey()));
        aCondFormats[i] = xCondFormat;
    }

    return aCondFormats;
}

sal_Int32 ScCondFormatsObj::getLength()
    throw(uno::RuntimeException, std::exception)
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
        rtl::Reference<ScCondFormatObj> xParent)
{
    switch (pEntry->GetType())
    {
        case condformat::CONDITION:
            return new ScConditionEntryObj(xParent,
                    static_cast<const ScCondFormatEntry*>(pEntry));
        break;
        case condformat::COLORSCALE:
            return new ScColorScaleFormatObj(xParent,
                    static_cast<const ScColorScaleFormat*>(pEntry));
        break;
        case condformat::DATABAR:
            return new ScDataBarFormatObj(xParent,
                    static_cast<const ScDataBarFormat*>(pEntry));
        break;
        case condformat::ICONSET:
            return new ScIconSetFormatObj(xParent,
                    static_cast<const ScIconSetFormat*>(pEntry));
        break;
        case condformat::DATE:
            return new ScCondDateFormatObj(xParent,
                    static_cast<const ScCondDateFormatEntry*>(pEntry));
        break;
        default:
        break;
    }
    return uno::Reference<beans::XPropertySet>();
}

}

ScCondFormatObj::ScCondFormatObj(ScDocShell* pDocShell, rtl::Reference<ScCondFormatsObj> xCondFormats,
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
    throw(uno::RuntimeException, std::exception)
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
            pNewEntry = new ScCondFormatEntry(SC_COND_EQUAL, "", "",
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
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    if (getCoreObject()->size() >= size_t(nIndex))
        throw lang::IllegalArgumentException();

    getCoreObject()->RemoveEntry(nIndex);
}

uno::Type ScCondFormatObj::getElementType()
    throw(uno::RuntimeException, std::exception)
{
    return cppu::UnoType<beans::XPropertySet>::get();
}

sal_Bool ScCondFormatObj::hasElements()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScConditionalFormat* pFormat = getCoreObject();
    return !pFormat->IsEmpty();
}

sal_Int32 ScCondFormatObj::getCount()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    ScConditionalFormat* pFormat = getCoreObject();

    return pFormat->size();
}

uno::Any ScCondFormatObj::getByIndex(sal_Int32 nIndex)
    throw(uno::RuntimeException, std::exception)
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
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap()));
    return aRef;
}

void SAL_CALL ScCondFormatObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
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
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
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
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

namespace {

bool isObjectStillAlive(ScConditionalFormat* pFormat, const ScFormatEntry* pEntry)
{
    for(size_t i = 0, n= pFormat->size(); i < n; ++i)
    {
        if (pFormat->GetEntry(i) == pEntry)
            return true;
    }
    return false;
}

}

ScConditionEntryObj::ScConditionEntryObj(rtl::Reference<ScCondFormatObj> xParent,
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
    throw(uno::RuntimeException, std::exception)
{
    return sheet::ConditionEntryType::CONDITION;
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScConditionEntryObj::getPropertySetInfo()
    throw(uno::RuntimeException, std::exception)
{
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScConditionEntryObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
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
                for (size_t i = 0; i < SAL_N_ELEMENTS(aConditionEntryMap); ++i)
                {
                    if (aConditionEntryMap[i].nApiMode == nVal)
                    {
                        getCoreObject()->SetOperation(aConditionEntryMap[i].eMode);
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
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
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
            for (size_t i = 0; i < SAL_N_ELEMENTS(aConditionEntryMap); ++i)
            {
                if (aConditionEntryMap[i].eMode == eMode)
                {
                    aAny <<= aConditionEntryMap[i].nApiMode;
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
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScConditionEntryObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScConditionEntryObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScConditionEntryObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

ScColorScaleFormatObj::ScColorScaleFormatObj(rtl::Reference<ScCondFormatObj> xParent,
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
    throw(uno::RuntimeException, std::exception)
{
    return sheet::ConditionEntryType::COLORSCALE;
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScColorScaleFormatObj::getPropertySetInfo()
    throw(uno::RuntimeException, std::exception)
{
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

namespace {

void setColorScaleEntry(ScColorScaleEntry* pEntry, uno::Reference<sheet::XColorScaleEntry> xEntry)
{
    ScColorScaleEntryType eType = ScColorScaleEntryType();
    sal_Int32 nApiType = xEntry->getType();
    bool bFound = false;
    for (size_t i = 0; i < SAL_N_ELEMENTS(aColorScaleEntryTypeMap); ++i)
    {
        if (aColorScaleEntryTypeMap[i].nApiType == nApiType)
        {
            eType = aColorScaleEntryTypeMap[i].eType;
            bFound = true;
            break;
        }
    }

    if (!bFound)
        throw lang::IllegalArgumentException();

    pEntry->SetType(eType);
    pEntry->SetColor(xEntry->getColor());
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
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
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
            if (aValue >>= aEntries)
            {
                if (aEntries.getLength() < 2)
                    throw lang::IllegalArgumentException();

                // TODO: we need to make sure that there are enough entries
                size_t n = size_t(aEntries.getLength());
                for (size_t i = 0; i < n; ++i)
                {
                    setColorScaleEntry(getCoreObject()->GetEntry(i), aEntries[i]);
                }
            }
            else
                throw lang::IllegalArgumentException();
        }
        break;
        default:
            SAL_WARN("sc", "unknown property");
    }
}

uno::Any SAL_CALL ScColorScaleFormatObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
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
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScColorScaleFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScColorScaleFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScColorScaleFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

ScColorScaleEntryObj::ScColorScaleEntryObj(rtl::Reference<ScColorScaleFormatObj> xParent,
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

util::Color ScColorScaleEntryObj::getColor()
    throw(uno::RuntimeException, std::exception)
{
    Color aColor = getCoreObject()->GetColor();
    return aColor.GetColor();
}

void ScColorScaleEntryObj::setColor(util::Color aColor)
    throw(uno::RuntimeException, std::exception)
{
    getCoreObject()->SetColor(Color(aColor));
}

sal_Int32 ScColorScaleEntryObj::getType()
    throw(uno::RuntimeException, std::exception)
{
    ScColorScaleEntry* pEntry = getCoreObject();
    for (size_t i = 0; i < SAL_N_ELEMENTS(aColorScaleEntryTypeMap); ++i)
    {
        if (aColorScaleEntryTypeMap[i].eType == pEntry->GetType())
        {
            return aColorScaleEntryTypeMap[i].nApiType;
        }
    }

    throw lang::IllegalArgumentException();
}

void ScColorScaleEntryObj::setType(sal_Int32 nType)
    throw(uno::RuntimeException, std::exception)
{
    ScColorScaleEntry* pEntry = getCoreObject();
    for (size_t i = 0; i < SAL_N_ELEMENTS(aColorScaleEntryTypeMap); ++i)
    {
        if (aColorScaleEntryTypeMap[i].nApiType == nType)
        {
            pEntry->SetType(aColorScaleEntryTypeMap[i].eType);
            return;
        }
    }
    throw lang::IllegalArgumentException();
}

OUString ScColorScaleEntryObj::getFormula()
    throw(uno::RuntimeException, std::exception)
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
    throw(uno::RuntimeException, std::exception)
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


ScDataBarFormatObj::ScDataBarFormatObj(rtl::Reference<ScCondFormatObj> xParent,
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
    throw(uno::RuntimeException, std::exception)
{
    return sheet::ConditionEntryType::DATABAR;
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScDataBarFormatObj::getPropertySetInfo()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

namespace {

void setDataBarEntry(ScColorScaleEntry* pEntry, uno::Reference<sheet::XDataBarEntry> xEntry)
{
    ScColorScaleEntryType eType = ScColorScaleEntryType();
    sal_Int32 nApiType = xEntry->getType();
    bool bFound = false;
    for (size_t i = 0; i < SAL_N_ELEMENTS(aDataBarEntryTypeMap); ++i)
    {
        if (aDataBarEntryTypeMap[i].nApiType == nApiType)
        {
            eType = aDataBarEntryTypeMap[i].eType;
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
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
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
                for (size_t i = 0; i < SAL_N_ELEMENTS(aDataBarAxisMap); ++i)
                {
                    if (aDataBarAxisMap[i].nApiPos == nVal)
                    {
                        getCoreObject()->GetDataBarData()->meAxisPosition =
                            aDataBarAxisMap[i].ePos;
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
            sal_Int32 nColor = COL_AUTO;
            if (aValue >>= nColor)
            {
                getCoreObject()->GetDataBarData()->maPositiveColor.SetColor(nColor);
            }
        }
        break;
        case AxisColor:
        {
            sal_Int32 nAxisColor = COL_AUTO;
            if (aValue >>= nAxisColor)
            {
                getCoreObject()->GetDataBarData()->maAxisColor.SetColor(nAxisColor);
            }
        }
        break;
        case NegativeColor:
        {
            sal_Int32 nNegativeColor = COL_AUTO;
            if ((aValue >>= nNegativeColor) && getCoreObject()->GetDataBarData()->mbNeg)
            {
                getCoreObject()->GetDataBarData()->mpNegativeColor->SetColor(nNegativeColor);
            }
            else
                throw lang::IllegalArgumentException();
        }
        break;
        case DataBarEntries:
        {
            uno::Sequence<uno::Reference<sheet::XDataBarEntry> > aEntries;
            if (aValue >>= aEntries)
            {
                if (aEntries.getLength() != 2)
                    throw lang::IllegalArgumentException();

                setDataBarEntry(getCoreObject()->GetDataBarData()->mpLowerLimit.get(),
                        aEntries[0]);
                setDataBarEntry(getCoreObject()->GetDataBarData()->mpUpperLimit.get(),
                        aEntries[1]);
            }
            else
                throw lang::IllegalArgumentException();
        }
        break;
        case MinimumLength:
        {
            double nLength = 0;
            if ((aValue >>= nLength) && nLength < 100 && nLength >= 0)
            {
                getCoreObject()->GetDataBarData()->mnMinLength = nLength;
            }
            else throw lang::IllegalArgumentException();
        }
        break;
        case MaximumLength:
        {
            double nLength = 0;
            if ((aValue >>= nLength) && nLength <= 100 && nLength > 0)
            {
                getCoreObject()->GetDataBarData()->mnMaxLength = nLength;
            }
            else throw lang::IllegalArgumentException();
        }
        break;
    }
}

uno::Any SAL_CALL ScDataBarFormatObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
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
            for (size_t i = 0; i < SAL_N_ELEMENTS(aDataBarAxisMap); ++i)
            {
                if (aDataBarAxisMap[i].ePos == ePos)
                {
                    nApiPos = aDataBarAxisMap[i].nApiPos;
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
            sal_Int32 nColor = getCoreObject()->GetDataBarData()->maPositiveColor.GetColor();
            aAny <<= nColor;
        }
        break;
        case AxisColor:
        {
            sal_Int32 nAxisColor = getCoreObject()->GetDataBarData()->maAxisColor.GetColor();
            aAny <<= nAxisColor;
        }
        break;
        case NegativeColor:
        {
            if (getCoreObject()->GetDataBarData()->mbNeg && getCoreObject()->GetDataBarData()->mpNegativeColor)
            {
                sal_Int32 nNegativeColor = getCoreObject()->GetDataBarData()->mpNegativeColor->GetColor();
                aAny <<= nNegativeColor;
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
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScDataBarFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScDataBarFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScDataBarFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

ScDataBarEntryObj::ScDataBarEntryObj(rtl::Reference<ScDataBarFormatObj> xParent,
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
    throw(uno::RuntimeException, std::exception)
{
    ScColorScaleEntry* pEntry = getCoreObject();
    for (size_t i = 0; i < SAL_N_ELEMENTS(aDataBarEntryTypeMap); ++i)
    {
        if (aDataBarEntryTypeMap[i].eType == pEntry->GetType())
        {
            return aDataBarEntryTypeMap[i].nApiType;
        }
    }

    throw lang::IllegalArgumentException();
}

void ScDataBarEntryObj::setType(sal_Int32 nType)
    throw(uno::RuntimeException, std::exception)
{
    ScColorScaleEntry* pEntry = getCoreObject();
    for (size_t i = 0; i < SAL_N_ELEMENTS(aDataBarEntryTypeMap); ++i)
    {
        if (aDataBarEntryTypeMap[i].nApiType == nType)
        {
            pEntry->SetType(aDataBarEntryTypeMap[i].eType);
            return;
        }
    }
    throw lang::IllegalArgumentException();
}

OUString ScDataBarEntryObj::getFormula()
    throw(uno::RuntimeException, std::exception)
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
    throw(uno::RuntimeException, std::exception)
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


ScIconSetFormatObj::ScIconSetFormatObj(rtl::Reference<ScCondFormatObj> xParent,
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
    throw(uno::RuntimeException, std::exception)
{
    return sheet::ConditionEntryType::ICONSET;
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScIconSetFormatObj::getPropertySetInfo()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

namespace {

void setIconSetEntry(ScIconSetFormat* pFormat, uno::Reference<sheet::XIconSetEntry> xEntry, size_t nPos)
{
    ScIconSetFormatData* pData = pFormat->GetIconSetData();
    ScColorScaleEntryType eType = ScColorScaleEntryType();
    sal_Int32 nApiType = xEntry->getType();
    bool bFound = false;
    for (size_t i = 0; i < SAL_N_ELEMENTS(aIconSetEntryTypeMap); ++i)
    {
        if (aIconSetEntryTypeMap[i].nApiType == nApiType)
        {
            eType = aIconSetEntryTypeMap[i].eType;
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
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
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
            for (size_t i = 0; i < SAL_N_ELEMENTS(aIconSetApiMap); ++i)
            {
                if (aIconSetApiMap[i].nApiType == nApiType)
                {
                    eType = aIconSetApiMap[i].eType;
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
            if (aValue >>= aEntries)
            {
                // TODO: we need to check that the number of entries
                // corresponds to the icon type
                sal_Int32 nLength = aEntries.getLength();
                for (size_t i = 0; i < size_t(nLength); ++i)
                {
                    setIconSetEntry(getCoreObject(), aEntries[i], i);
                }
            }
            else
                throw lang::IllegalArgumentException();
        }
        break;
        default:
        break;
    }
}

uno::Any SAL_CALL ScIconSetFormatObj::getPropertyValue( const OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
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
            for (size_t i = 0; i < SAL_N_ELEMENTS(aIconSetApiMap); ++i)
            {
                if (aIconSetApiMap[i].eType == eType)
                {
                    aAny <<= aIconSetApiMap[i].nApiType;
                    break;
                }
            }
        }
        break;
        case IconSetEntries:
        {
            uno::Sequence<uno::Reference<sheet::XIconSetEntry> > aEntries(getCoreObject()->size());
            size_t i = 0;
            for (auto it = getCoreObject()->begin(), itEnd = getCoreObject()->end(); it != itEnd; ++it, ++i)
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
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScIconSetFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScIconSetFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScIconSetFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

ScIconSetEntryObj::ScIconSetEntryObj(rtl::Reference<ScIconSetFormatObj> xParent,
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
    throw(uno::RuntimeException, std::exception)
{
    ScColorScaleEntry* pEntry = getCoreObject();
    // the first entry always is minimum
    if (mnPos == 0)
        return sheet::IconSetFormatEntry::ICONSET_MIN;

    for (size_t i = 0; i < SAL_N_ELEMENTS(aIconSetEntryTypeMap); ++i)
    {
        if (aIconSetEntryTypeMap[i].eType == pEntry->GetType())
        {
            return aIconSetEntryTypeMap[i].nApiType;
        }
    }

    throw lang::IllegalArgumentException();
}

void ScIconSetEntryObj::setType(sal_Int32 nType)
    throw(uno::RuntimeException, std::exception)
{
    // first entry is always MIN
    if (mnPos == 0)
        return;

    ScColorScaleEntry* pEntry = getCoreObject();
    for (size_t i = 0; i < SAL_N_ELEMENTS(aIconSetEntryTypeMap); ++i)
    {
        if (aIconSetEntryTypeMap[i].nApiType == nType)
        {
            pEntry->SetType(aIconSetEntryTypeMap[i].eType);
            return;
        }
    }
    throw lang::IllegalArgumentException();
}

OUString ScIconSetEntryObj::getFormula()
    throw(uno::RuntimeException, std::exception)
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
    throw(uno::RuntimeException, std::exception)
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

ScCondDateFormatObj::ScCondDateFormatObj(rtl::Reference<ScCondFormatObj> xParent,
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
    throw(uno::RuntimeException, std::exception)
{
    return sheet::ConditionEntryType::DATE;
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCondDateFormatObj::getPropertySetInfo()
    throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef(
        new SfxItemPropertySetInfo( maPropSet.getPropertyMap() ));
    return aRef;
}

void SAL_CALL ScCondDateFormatObj::setPropertyValue(
                        const OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
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
            if (aValue >>= aStyleName)
            {
                getCoreObject()->SetStyleName(aStyleName);
            }
            else
                throw lang::IllegalArgumentException();
        }
        break;
        case DateType:
        {
            sal_Int32 nApiType = -1;
            if (!(aValue >>= nApiType))
                throw lang::IllegalArgumentException();

            for (size_t i = 0; i < SAL_N_ELEMENTS(aDateTypeApiMap); ++i)
            {
                if (aDateTypeApiMap[i].nApiType == nApiType)
                {
                    getCoreObject()->SetDateType(aDateTypeApiMap[i].eType);
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
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException, std::exception)
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
            for (size_t i = 0; i < SAL_N_ELEMENTS(aDateTypeApiMap); ++i)
            {
                if (aDateTypeApiMap[i].eType == eType)
                {
                    aAny <<= aDateTypeApiMap[i].nApiType;
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
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondDateFormatObj::removePropertyChangeListener( const OUString& /* aPropertyName */,
                            const uno::Reference<beans::XPropertyChangeListener>& /* aListener */)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondDateFormatObj::addVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

void SAL_CALL ScCondDateFormatObj::removeVetoableChangeListener( const OUString&,
                            const uno::Reference<beans::XVetoableChangeListener>&)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SAL_WARN("sc", "not implemented");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
