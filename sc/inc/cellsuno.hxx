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

#pragma once

#include <config_options.h>
#include "address.hxx"
#include "rangelst.hxx"

#include <formula/grammar.hxx>
#include <rtl/ref.hxx>
#include <sal/types.h>
#include <tools/link.hxx>
#include <svl/itemset.hxx>
#include <svl/lstner.hxx>
#include <svl/listener.hxx>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/table/XTablePivotChartsSupplier.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#include <com/sun/star/sheet/XFormulaQuery.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XSheetOperation.hpp>
#include <com/sun/star/sheet/XSheetCellRangeContainer.hpp>
#include <com/sun/star/sheet/XCellFormatRangesSupplier.hpp>
#include <com/sun/star/sheet/XUniqueCellFormatRangesSupplier.hpp>
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#include <com/sun/star/sheet/XSheetFilterableEx.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XSheetPageBreak.hpp>
#include <com/sun/star/sheet/XCellRangeMovement.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/sheet/XPrintAreas.hpp>
#include <com/sun/star/sheet/XSheetLinkable.hpp>
#include <com/sun/star/sheet/XSubTotalCalculatable.hpp>
#include <com/sun/star/sheet/XArrayFormulaRange.hpp>
#include <com/sun/star/sheet/XCellRangeData.hpp>
#include <com/sun/star/sheet/XCellRangeFormula.hpp>
#include <com/sun/star/sheet/XCellSeries.hpp>
#include <com/sun/star/sheet/XMultipleOperation.hpp>
#include <com/sun/star/sheet/XFormulaTokens.hpp>
#include <com/sun/star/sheet/XArrayFormulaTokens.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XIndent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/util/XMergeable.hpp>
#include <com/sun/star/table/XAutoFormattable.hpp>
#include <com/sun/star/util/XSortable.hpp>
#include <com/sun/star/util/XImportable.hpp>
#include <com/sun/star/table/XColumnRowRange.hpp>
#include <com/sun/star/table/XCell2.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/sheet/XScenariosSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XSheetAuditing.hpp>
#include <com/sun/star/sheet/XSheetOutline.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/sheet/XScenario.hpp>
#include <com/sun/star/sheet/XScenarioEnhanced.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/beans/XTolerantMultiPropertySet.hpp>
#include <com/sun/star/sheet/XExternalSheetName.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <cppuhelper/implbase.hxx>

#include <memory>
#include <optional>
#include <vector>

namespace com::sun::star::table { struct BorderLine2; }
namespace com::sun::star::table { struct BorderLine; }
namespace com::sun::star::table { struct TableBorder2; }
namespace com::sun::star::table { struct TableBorder; }

class ScDocShell;
class ScMarkData;
class ScMemChart;
class ScPrintRangeSaver;
class ScAttrRectIterator;
class ScCellRangeObj;
class ScPatternAttr;
class SvxBoxItem;
class SvxBoxInfoItem;
class SvxItemPropertySet;
class SvxUnoText;
class ScDocument;
class SfxHint;
class SfxItemPropertyMap;
class SfxItemPropertySet;
struct SfxItemPropertyMapEntry;
class ScTableRowsObj;
class ScTableValidationObj;
class ScCellObj;
class SolarMutexGuard;

namespace editeng { class SvxBorderLine; }

class ScLinkListener final : public SvtListener
{
    Link<const SfxHint&,void>  aLink;
public:
                    ScLinkListener(const Link<const SfxHint&,void>& rL) : aLink(rL) {}
    virtual         ~ScLinkListener() override;
    virtual void Notify( const SfxHint& rHint ) override;
};

typedef std::vector< css::uno::Reference< css::util::XModifyListener > > XModifyListenerArr_Impl;

//  ScCellRangesBase - base class for ScCellRangesObj (with access by index)
//                                and ScCellRangeObj  (without access by index)

//  XServiceInfo is implemented in derived classes

class ScHelperFunctions
{
public:
    static const ::editeng::SvxBorderLine* GetBorderLine( ::editeng::SvxBorderLine& rLine, const css::table::BorderLine& rStruct );
    static const ::editeng::SvxBorderLine* GetBorderLine( ::editeng::SvxBorderLine& rLine, const css::table::BorderLine2& rStruct );
    static void FillBoxItems( SvxBoxItem& rOuter, SvxBoxInfoItem& rInner, const css::table::TableBorder& rBorder );
    static void FillBoxItems( SvxBoxItem& rOuter, SvxBoxInfoItem& rInner, const css::table::TableBorder2& rBorder );
    static void FillBorderLine( css::table::BorderLine& rStruct, const ::editeng::SvxBorderLine* pLine );
    static void FillBorderLine( css::table::BorderLine2& rStruct, const ::editeng::SvxBorderLine* pLine );
    static void AssignTableBorderToAny( css::uno::Any& rAny,
            const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner, bool bInvalidateHorVerDist = false );
    static void AssignTableBorder2ToAny( css::uno::Any& rAny,
            const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner, bool bInvalidateHorVerDist = false );
    static void ApplyBorder( ScDocShell* pDocShell, const ScRangeList& rRanges,
                        const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner );
};

namespace ooo::vba::excel {
    class ScVbaCellRangeAccess;  // Vba Helper class
}

class SAL_DLLPUBLIC_RTTI SAL_LOPLUGIN_ANNOTATE("crosscast") ScCellRangesBase :
                         public cppu::WeakImplHelper<
                            css::beans::XPropertySet,
                             css::beans::XMultiPropertySet,
                             css::beans::XPropertyState,
                             css::sheet::XSheetOperation,
                             css::chart::XChartDataArray,
                             css::util::XIndent,
                             css::sheet::XCellRangesQuery,
                             css::sheet::XFormulaQuery,
                             css::util::XReplaceable,
                             css::util::XModifyBroadcaster,
                             css::lang::XServiceInfo,
                             css::beans::XTolerantMultiPropertySet>,
                         public SfxListener
{
    friend class ScTabViewObj;      // for select()
    friend class ScTableSheetObj;   // for createCursorByRange()
    friend class ooo::vba::excel::ScVbaCellRangeAccess;

private:
    const SfxItemPropertySet* pPropSet;
    ScDocShell*             pDocShell;
    std::unique_ptr<ScLinkListener> pValueListener;
    std::unique_ptr<ScPatternAttr>  pCurrentFlat;
    std::unique_ptr<ScPatternAttr>  pCurrentDeep;
    std::optional<SfxItemSet>       moCurrentDataSet;
    std::optional<SfxItemSet>       moNoDfltCurrentDataSet;
    std::unique_ptr<ScMarkData>     pMarkData;
    ScRangeList             aRanges;
    sal_Int64               nObjectId;
    bool                    bChartColAsHdr;
    bool                    bChartRowAsHdr;
    bool                    bCursorOnly;
    bool                    bGotDataChangedHint;
    XModifyListenerArr_Impl aValueListeners;

    DECL_LINK( ValueListenerHdl, const SfxHint&, void );

private:
    void            PaintGridRanges_Impl();
    ScRangeListRef  GetLimitedChartRanges_Impl( sal_Int32 nDataColumns, sal_Int32 nDataRows ) const;
    void            ForceChartListener_Impl();
    std::unique_ptr<ScMemChart> CreateMemChart_Impl() const;

    const ScPatternAttr*    GetCurrentAttrsFlat();
    const ScPatternAttr*    GetCurrentAttrsDeep();
    SC_DLLPUBLIC SfxItemSet* GetCurrentDataSet(bool bNoDflt = false);
    void                    ForgetMarkData();
    void                    ForgetCurrentAttrs();

    css::uno::Reference<css::sheet::XSheetCellRanges>
                            QueryDifferences_Impl(const css::table::CellAddress& aCompare,
                                                    bool bColumnDiff);
    css::uno::Reference<css::uno::XInterface>
                            Find_Impl(const css::uno::Reference<css::util::XSearchDescriptor>& xDesc,
                                    const ScAddress* pLastPos);

protected:
    const ScMarkData*       GetMarkData();

    // GetItemPropertyMap for derived classes must contain all entries, including base class
    SC_DLLPUBLIC virtual const SfxItemPropertyMap& GetItemPropertyMap();
    css::beans::PropertyState GetOnePropertyState(
                                sal_uInt16 nItemWhich, const SfxItemPropertyMapEntry* pEntry );
    /// @throws css::uno::RuntimeException
    SC_DLLPUBLIC virtual void GetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                css::uno::Any& );
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::uno::RuntimeException
    SC_DLLPUBLIC virtual void SetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                                const css::uno::Any& aValue );

    ScRangeList& AccessRanges() { return aRanges; }
    SC_DLLPUBLIC virtual void AdjustUpdatedRanges(UpdateRefMode mode);

public:
                            ScCellRangesBase(ScDocShell* pDocSh, const ScRange& rR);
                            ScCellRangesBase(ScDocShell* pDocSh, ScRangeList aR);
    virtual                 ~ScCellRangesBase() override;

    SC_DLLPUBLIC virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    virtual void            RefChanged();

                            // from derived classes and by getImplementation
    ScDocShell*             GetDocShell() const     { return pDocShell; }
    ScDocument*             GetDocument() const;
    const ScRangeList&      GetRangeList() const    { return aRanges; }
    void                    AddRange(const ScRange& rRange, const bool bMergeRanges);

                            // arouse object created via service:
    void                    InitInsertRange(ScDocShell* pDocSh, const ScRange& rR);

    void                    SetNewRange(const ScRange& rNew);   // for cursor
    void                    SetNewRanges(const ScRangeList& rNew);

    void                    SetCursorOnly(bool bSet);
    bool                    IsCursorOnly() const            { return bCursorOnly; }

    SC_DLLPUBLIC rtl::Reference<ScTableValidationObj> getValidation();
    SC_DLLPUBLIC void setValidation(const rtl::Reference<ScTableValidationObj>&);

                            // XSheetOperation
    virtual double SAL_CALL computeFunction( css::sheet::GeneralFunction nFunction ) override;
    virtual void SAL_CALL   clearContents( sal_Int32 nContentFlags ) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // XMultiPropertySet
    virtual void SAL_CALL   setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL
                            getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

                            // XTolerantMultiPropertySet
    virtual css::uno::Sequence< css::beans::SetPropertyTolerantFailed > SAL_CALL
        setPropertyValuesTolerant( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::beans::GetPropertyTolerantResult > SAL_CALL
        getPropertyValuesTolerant( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual css::uno::Sequence< css::beans::GetDirectPropertyTolerantResult > SAL_CALL
        getDirectPropertyValuesTolerant( const css::uno::Sequence< OUString >& aPropertyNames ) override;

                            // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState(
                                    const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL
                            getPropertyStates( const css::uno::Sequence<
                                        OUString >& aPropertyName ) override;
    virtual void SAL_CALL   setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault(
                                    const OUString& aPropertyName ) override;

                            // XIndent
    virtual void SAL_CALL   decrementIndent() override;
    virtual void SAL_CALL   incrementIndent() override;

                            // XChartData
    virtual void SAL_CALL   addChartDataChangeEventListener(
                                const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual void SAL_CALL   removeChartDataChangeEventListener( const css::uno::Reference<
                                    css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual double SAL_CALL getNotANumber() override;
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber ) override;

                            // XChartDataArray
    virtual css::uno::Sequence< css::uno::Sequence< double > > SAL_CALL
                            getData() override;
    virtual void SAL_CALL   setData( const css::uno::Sequence< css::uno::Sequence< double > >& aData ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getRowDescriptions() override;
    virtual void SAL_CALL setRowDescriptions( const css::uno::Sequence< OUString >& aRowDescriptions ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getColumnDescriptions() override;
    virtual void SAL_CALL   setColumnDescriptions( const css::uno::Sequence< OUString >& aColumnDescriptions ) override;

                            // XCellRangesQuery
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryVisibleCells() override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryEmptyCells() override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryContentCells( sal_Int16 nContentFlags ) override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryFormulaCells( sal_Int32 nResultFlags ) override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryColumnDifferences(
                                const css::table::CellAddress& aCompare ) override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryRowDifferences(
                                const css::table::CellAddress& aCompare ) override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryIntersection(
                                const css::table::CellRangeAddress& aRange ) override;

                            // XFormulaQuery
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryDependents( sal_Bool bRecursive ) override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryPrecedents( sal_Bool bRecursive ) override;

                            // XSearchable
    virtual css::uno::Reference< css::util::XSearchDescriptor > SAL_CALL
                            createSearchDescriptor() override;
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
                            findAll( const css::uno::Reference< css::util::XSearchDescriptor >& xDesc ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
                            findFirst( const css::uno::Reference< css::util::XSearchDescriptor >& xDesc ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
                            findNext( const css::uno::Reference< css::uno::XInterface >& xStartAt,
                                      const css::uno::Reference< css::util::XSearchDescriptor >& xDesc ) override;

                            // XReplaceable
    virtual css::uno::Reference< css::util::XReplaceDescriptor > SAL_CALL
                            createReplaceDescriptor() override;
    virtual sal_Int32 SAL_CALL replaceAll( const css::uno::Reference< css::util::XSearchDescriptor >& xDesc ) override;

                            // XModifyBroadcaster
    virtual void SAL_CALL   addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void SAL_CALL   removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

using ScCellRangesObj_BASE = cppu::ImplInheritanceHelper<ScCellRangesBase,
                                                         css::sheet::XSheetCellRangeContainer,
                                                         css::container::XNameContainer,
                                                         css::container::XEnumerationAccess>;
class SC_DLLPUBLIC ScCellRangesObj final : public ScCellRangesObj_BASE
{
public:
    struct ScNamedEntry
    {
        OUString  aName;
        ScRange   aRange;
        const OUString& GetName() const { return aName; }
        const ScRange& GetRange() const { return aRange; }
    };
private:
    std::vector<ScNamedEntry> m_aNamedEntries;

    rtl::Reference<ScCellRangeObj> GetObjectByIndex_Impl(sal_Int32 nIndex) const;

public:
                            ScCellRangesObj(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual                 ~ScCellRangesObj() override;

    virtual void            RefChanged() override;

                            // XSheetCellRanges
    virtual css::uno::Reference< css::container::XEnumerationAccess > SAL_CALL
                            getCells() override;
    virtual OUString SAL_CALL getRangeAddressesAsString() override;
    virtual css::uno::Sequence< css::table::CellRangeAddress > SAL_CALL
                            getRangeAddresses() override;

                            // XSheetCellRangeContainer
    virtual void SAL_CALL   addRangeAddress( const css::table::CellRangeAddress& rRange,
                                        sal_Bool bMergeRanges ) override;
    virtual void SAL_CALL   removeRangeAddress( const css::table::CellRangeAddress& rRange ) override;
    virtual void SAL_CALL   addRangeAddresses( const css::uno::Sequence<
                                        css::table::CellRangeAddress >& rRanges,
                                        sal_Bool bMergeRanges ) override;
    virtual void SAL_CALL   removeRangeAddresses( const css::uno::Sequence<
                                        css::table::CellRangeAddress >& rRanges ) override;

                            // XNameContainer
    virtual void SAL_CALL   insertByName( const OUString& aName,
                                const css::uno::Any& aElement ) override;
    virtual void SAL_CALL   removeByName( const OUString& Name ) override;

                            // XNameReplace
    virtual void SAL_CALL   replaceByName( const OUString& aName,
                                const css::uno::Any& aElement ) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    void addRangeAddresses( const ScRangeList& rRanges, bool bMergeRanges );
};

using ScCellRangeObj_BASE = cppu::ImplInheritanceHelper<ScCellRangesBase,
                                                        css::sheet::XCellRangeAddressable,
                                                        css::sheet::XSheetCellRange,
                                                        css::sheet::XArrayFormulaRange,
                                                        css::sheet::XArrayFormulaTokens,
                                                        css::sheet::XCellRangeData,
                                                        css::sheet::XCellRangeFormula,
                                                        css::sheet::XMultipleOperation,
                                                        css::util::XMergeable,
                                                        css::sheet::XCellSeries,
                                                        css::table::XAutoFormattable,
                                                        css::util::XSortable,
                                                        css::sheet::XSheetFilterableEx,
                                                        css::sheet::XSubTotalCalculatable,
                                                        css::util::XImportable,
                                                        css::sheet::XCellFormatRangesSupplier,
                                                        css::sheet::XUniqueCellFormatRangesSupplier,
                                                        css::table::XColumnRowRange>;
class SAL_DLLPUBLIC_RTTI ScCellRangeObj : public ScCellRangeObj_BASE
{
private:
    const SfxItemPropertySet*       pRangePropSet;
    ScRange                 aRange;

protected:
    const ScRange&          GetRange() const    { return aRange; }
    virtual const SfxItemPropertyMap& GetItemPropertyMap() override;
    virtual void GetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                css::uno::Any& ) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                                const css::uno::Any& aValue ) override;

    /// @throws css::lang::IndexOutOfBoundsException
    /// @throws css::uno::RuntimeException
    rtl::Reference< ScCellObj >
                            GetCellByPosition_Impl( sal_Int32 nColumn, sal_Int32 nRow );

            /// @throws css::uno::RuntimeException
            void            SetArrayFormula_Impl( const OUString& rFormula,
                                const formula::FormulaGrammar::Grammar eGrammar );

public:
    SC_DLLPUBLIC            ScCellRangeObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScCellRangeObj() override;

                            // uses ObjectShell from document, if set (returns NULL otherwise)
    static css::uno::Reference<css::table::XCellRange>
                            CreateRangeFromDoc( const ScDocument& rDoc, const ScRange& rR );

    virtual void            RefChanged() override;

                            // XCellRangeAddressable
    virtual css::table::CellRangeAddress SAL_CALL getRangeAddress() override;

                            // XSheetCellRange
    virtual css::uno::Reference< css::sheet::XSpreadsheet > SAL_CALL
                            getSpreadsheet() override;

                            // XArrayFormulaRange
    virtual OUString SAL_CALL getArrayFormula() override;
    virtual void SAL_CALL   setArrayFormula( const OUString& aFormula ) override;

                            // XArrayFormulaTokens
    virtual css::uno::Sequence< css::sheet::FormulaToken > SAL_CALL getArrayTokens() override;
    SC_DLLPUBLIC virtual void SAL_CALL   setArrayTokens( const css::uno::Sequence<
                                    css::sheet::FormulaToken >& aTokens ) override;

                            // XCellRangeData
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > > SAL_CALL getDataArray() override;
    virtual void SAL_CALL   setDataArray( const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& aArray ) override;

                            // XCellRangeFormula
    virtual css::uno::Sequence< css::uno::Sequence<
                            OUString > > SAL_CALL getFormulaArray() override;
    virtual void SAL_CALL   setFormulaArray( const css::uno::Sequence< css::uno::Sequence< OUString > >& aArray ) override;

                            // XMultipleOperation
    virtual void SAL_CALL   setTableOperation(
                                const css::table::CellRangeAddress& aFormulaRange,
                                css::sheet::TableOperationMode nMode,
                                const css::table::CellAddress& aColumnCell,
                                const css::table::CellAddress& aRowCell ) override;

                            // XMergeable
    virtual void SAL_CALL   merge( sal_Bool bMerge ) override;
    virtual sal_Bool SAL_CALL getIsMerged() override;

                            // XCellSeries
    virtual void SAL_CALL   fillSeries( css::sheet::FillDirection nFillDirection,
                                css::sheet::FillMode nFillMode,
                                css::sheet::FillDateMode nFillDateMode,
                                double fStep, double fEndValue ) override;
    virtual void SAL_CALL   fillAuto( css::sheet::FillDirection nFillDirection,
                                sal_Int32 nSourceCount ) override;

                            // XAutoFormattable
    virtual void SAL_CALL   autoFormat( const OUString& aName ) override;

                            // XSortable
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            createSortDescriptor() override;
    virtual void SAL_CALL   sort( const css::uno::Sequence<
                                css::beans::PropertyValue >& xDescriptor ) override;

                            // XSheetFilterableEx
    virtual css::uno::Reference< css::sheet::XSheetFilterDescriptor > SAL_CALL
                            createFilterDescriptorByObject( const css::uno::Reference<
                                css::sheet::XSheetFilterable >& xObject ) override;

                            // XSheetFilterable
    virtual css::uno::Reference< css::sheet::XSheetFilterDescriptor > SAL_CALL
                            createFilterDescriptor( sal_Bool bEmpty ) override;
    virtual void SAL_CALL   filter( const css::uno::Reference<
                                css::sheet::XSheetFilterDescriptor >& xDescriptor ) override;

                            // XSubTotalCalculatable
    virtual css::uno::Reference< css::sheet::XSubTotalDescriptor > SAL_CALL
                            createSubTotalDescriptor( sal_Bool bEmpty ) override;
    virtual void SAL_CALL   applySubTotals(const css::uno::Reference< css::sheet::XSubTotalDescriptor >& xDescriptor,
                                sal_Bool bReplace) override;
    virtual void SAL_CALL   removeSubTotals() override;

                            // XImportable
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            createImportDescriptor( sal_Bool bEmpty ) override;
    virtual void SAL_CALL   doImport( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor ) override;

                            // XCellFormatRangesSupplier
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
                            getCellFormatRanges() override;

                            // XUniqueCellFormatRangesSupplier
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
                            getUniqueCellFormatRanges() override;

                            // XColumnRowRange
    virtual css::uno::Reference< css::table::XTableColumns > SAL_CALL
                            getColumns() override;
    virtual css::uno::Reference< css::table::XTableRows > SAL_CALL
                            getRows() override;

                            // XCellRange
    virtual css::uno::Reference< css::table::XCell > SAL_CALL
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getCellRangeByName( const OUString& aRange ) override;
    /// @throws css::uno::RuntimeException
    css::uno::Reference< css::table::XCellRange >
                            getCellRangeByName( const OUString& aRange,  const ScAddress::Details& rDetails );

                            // XPropertySet override due to Range-Properties
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    rtl::Reference< ScCellRangeObj >
                            getScCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom );
    SC_DLLPUBLIC rtl::Reference< ScCellRangeObj >
                            getScCellRangeByName( const OUString& aRange );
    rtl::Reference< ScCellRangeObj >
                            getScCellRangeByName( const OUString& aRange, const ScAddress::Details& rDetails );
    rtl::Reference< ScTableRowsObj >
                            getScRowsByPosition( SolarMutexGuard& rGuard, sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom );
    rtl::Reference< ScTableRowsObj > getScRows();
    SC_DLLPUBLIC rtl::Reference< ScCellObj >
                            getScCellByPosition( sal_Int32 nColumn, sal_Int32 nRow );
};

//! really derive cell from range?

using ScCellObj_BASE = cppu::ImplInheritanceHelper<ScCellRangeObj,
                                                   css::text::XText,
                                                   css::container::XEnumerationAccess,
                                                   css::table::XCell2,
                                                   css::sheet::XFormulaTokens,
                                                   css::sheet::XCellAddressable,
                                                   css::sheet::XSheetAnnotationAnchor,
                                                   css::text::XTextFieldsSupplier,
                                                   css::document::XActionLockable>;
class SAL_DLLPUBLIC_RTTI ScCellObj final : public ScCellObj_BASE
{
private:
    rtl::Reference<SvxUnoText> mxUnoText;
    const SfxItemPropertySet*   pCellPropSet;
    ScAddress               aCellPos;
    sal_Int16               nActionLockCount;

private:
    OUString    GetInputString_Impl(bool bEnglish) const;
    OUString    GetOutputString_Impl() const;
    void        SetString_Impl(const OUString& rString, bool bInterpret, bool bEnglish);
    double      GetValue_Impl() const;
    void        SetValue_Impl(double fValue);
    css::table::CellContentType GetContentType_Impl();
    sal_Int32 GetResultType_Impl() const;

    virtual const SfxItemPropertyMap& GetItemPropertyMap() override;
    virtual void GetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                css::uno::Any& ) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                                const css::uno::Any& aValue ) override;

public:
    static const SvxItemPropertySet* GetEditPropertySet();
    static const SfxItemPropertyMap& GetCellPropertyMap();

                            ScCellObj(ScDocShell* pDocSh, const ScAddress& rP);
    SC_DLLPUBLIC virtual    ~ScCellObj() override;

    virtual void            RefChanged() override;

    SvxUnoText&             GetUnoText();

                            // XML import needs to set results at formula cells,
                            // not meant for any other purpose.
    const ScAddress&        GetPosition() const { return aCellPos; }

    SC_DLLPUBLIC void       InputEnglishString( const OUString& rText );

                            // XText
    virtual void SAL_CALL   insertTextContent( const css::uno::Reference< css::text::XTextRange >& xRange,
                                const css::uno::Reference< css::text::XTextContent >& xContent,
                                sal_Bool bAbsorb ) override;
    virtual void SAL_CALL   removeTextContent( const css::uno::Reference< css::text::XTextContent >& xContent ) override;

                            // XSimpleText
    virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL
                            createTextCursor() override;
    virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL
                            createTextCursorByRange( const css::uno::Reference< css::text::XTextRange >& aTextPosition ) override;
    virtual void SAL_CALL   insertString( const css::uno::Reference< css::text::XTextRange >& xRange,
                                        const OUString& aString, sal_Bool bAbsorb ) override;
    virtual void SAL_CALL   insertControlCharacter( const css::uno::Reference< css::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, sal_Bool bAbsorb ) override;

                            // XTextRange
    virtual css::uno::Reference< css::text::XText > SAL_CALL
                            getText() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getStart() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getEnd() override;
    virtual OUString SAL_CALL getString() override;
    virtual void SAL_CALL   setString( const OUString& aString ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XCell
    virtual OUString SAL_CALL getFormula() override;
    virtual void SAL_CALL   setFormula( const OUString& aFormula ) override;
    virtual void SAL_CALL   setFormulaResult( double nValue ) override;
    virtual void SAL_CALL   setFormulaString( const OUString& aFormula ) override;
    virtual double SAL_CALL getValue() override;
    virtual void SAL_CALL   setValue( double nValue ) override;
    virtual css::table::CellContentType SAL_CALL getType() override;
    virtual sal_Int32 SAL_CALL getError() override;

                            // XFormulaTokens
    virtual css::uno::Sequence< css::sheet::FormulaToken > SAL_CALL getTokens() override;
    virtual void SAL_CALL   setTokens( const css::uno::Sequence< css::sheet::FormulaToken >& aTokens ) override;

                            // XCellAddressable
    SC_DLLPUBLIC virtual css::table::CellAddress SAL_CALL getCellAddress() override;

                            // XSheetAnnotationAnchor
    virtual css::uno::Reference< css::sheet::XSheetAnnotation > SAL_CALL
                            getAnnotation() override;

                            // XTextFieldsSupplier
    virtual css::uno::Reference< css::container::XEnumerationAccess > SAL_CALL
                            getTextFields() override;
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
                            getTextFieldMasters() override;

                            // XPropertySet override due to cell properties
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

                            // XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked() override;
    virtual void SAL_CALL   addActionLock() override;
    virtual void SAL_CALL   removeActionLock() override;
    virtual void SAL_CALL   setActionLocks( sal_Int16 nLock ) override;
    virtual sal_Int16 SAL_CALL resetActionLocks() override;
};

using ScTableSheetObj_BASE = cppu::ImplInheritanceHelper<ScCellRangeObj,
                                                         css::sheet::XSpreadsheet,
                                                         css::container::XNamed,
                                                         css::sheet::XSheetPageBreak,
                                                         css::sheet::XCellRangeMovement,
                                                         css::table::XTableChartsSupplier,
                                                         css::table::XTablePivotChartsSupplier,
                                                         css::sheet::XDataPilotTablesSupplier,
                                                         css::sheet::XScenariosSupplier,
                                                         css::sheet::XSheetAnnotationsSupplier,
                                                         css::drawing::XDrawPageSupplier,
                                                         css::sheet::XPrintAreas,
                                                         css::sheet::XSheetLinkable,
                                                         css::sheet::XSheetAuditing,
                                                         css::sheet::XSheetOutline,
                                                         css::util::XProtectable,
                                                         css::sheet::XScenario,
                                                         css::sheet::XScenarioEnhanced,
                                                         css::sheet::XExternalSheetName,
                                                         css::document::XEventsSupplier>;
class SC_DLLPUBLIC ScTableSheetObj final : public ScTableSheetObj_BASE
{
    friend class ScTableSheetsObj;      // for insertByName()

private:
    const SfxItemPropertySet*       pSheetPropSet;

    SCTAB                   GetTab_Impl() const;
    void                    PrintAreaUndo_Impl( std::unique_ptr<ScPrintRangeSaver> pOldRanges );

    virtual const SfxItemPropertyMap& GetItemPropertyMap() override;
    virtual void GetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                css::uno::Any& ) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                                const css::uno::Any& aValue ) override;

public:
                            ScTableSheetObj(ScDocShell* pDocSh, SCTAB nTab);
    virtual                 ~ScTableSheetObj() override;

    void                    InitInsertSheet(ScDocShell* pDocSh, SCTAB nTab);

                            // XSpreadsheet
    virtual css::uno::Reference< css::sheet::XSheetCellCursor >
                            SAL_CALL createCursor() override;
    virtual css::uno::Reference< css::sheet::XSheetCellCursor > SAL_CALL
                            createCursorByRange( const css::uno::Reference<
                                css::sheet::XSheetCellRange >& aRange ) override;

                            // XSheetCellRange
    virtual css::uno::Reference< css::sheet::XSpreadsheet > SAL_CALL
                            getSpreadsheet() override;

                            // XCellRange
    virtual css::uno::Reference< css::table::XCell > SAL_CALL
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom ) override;
    using ScCellRangeObj::getCellRangeByName;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getCellRangeByName( const OUString& aRange ) override;

                            // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL   setName( const OUString& aName ) override;

                            // XSheetPageBreak
    virtual css::uno::Sequence< css::sheet::TablePageBreakData > SAL_CALL
                            getColumnPageBreaks() override;
    virtual css::uno::Sequence< css::sheet::TablePageBreakData > SAL_CALL
                            getRowPageBreaks() override;
    virtual void SAL_CALL   removeAllManualPageBreaks() override;

                            // XCellRangeMovement
    virtual void SAL_CALL   insertCells( const css::table::CellRangeAddress& aRange,
                                css::sheet::CellInsertMode nMode ) override;
    virtual void SAL_CALL   removeRange( const css::table::CellRangeAddress& aRange,
                                css::sheet::CellDeleteMode nMode ) override;
    virtual void SAL_CALL   moveRange( const css::table::CellAddress& aDestination,
                                const css::table::CellRangeAddress& aSource ) override;
    virtual void SAL_CALL   copyRange( const css::table::CellAddress& aDestination,
                                const css::table::CellRangeAddress& aSource ) override;

                            // XTableChartsSupplier
    virtual css::uno::Reference< css::table::XTableCharts > SAL_CALL
                            getCharts() override;

                            // XTablePivotChartsSupplier
    virtual css::uno::Reference<css::table::XTablePivotCharts> SAL_CALL
                            getPivotCharts() override;

                            // XDataPilotTablesSupplier
    virtual css::uno::Reference< css::sheet::XDataPilotTables > SAL_CALL
                            getDataPilotTables() override;

                            // XScenariosSupplier
    virtual css::uno::Reference< css::sheet::XScenarios > SAL_CALL
                            getScenarios() override;

                            // XSheetAnnotationsSupplier
    virtual css::uno::Reference< css::sheet::XSheetAnnotations > SAL_CALL
                            getAnnotations() override;

                            // XDrawPageSupplier
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL
                            getDrawPage() override;

                            // XPrintAreas
    virtual css::uno::Sequence< css::table::CellRangeAddress > SAL_CALL
                            getPrintAreas() override;
    virtual void SAL_CALL setPrintAreas( const css::uno::Sequence<
                                css::table::CellRangeAddress >& aPrintAreas ) override;
    virtual sal_Bool SAL_CALL getPrintTitleColumns() override;
    virtual void SAL_CALL   setPrintTitleColumns( sal_Bool bPrintTitleColumns ) override;
    virtual css::table::CellRangeAddress SAL_CALL getTitleColumns() override;
    virtual void SAL_CALL   setTitleColumns(
                                const css::table::CellRangeAddress& aTitleColumns ) override;
    virtual sal_Bool SAL_CALL getPrintTitleRows() override;
    virtual void SAL_CALL   setPrintTitleRows( sal_Bool bPrintTitleRows ) override;
    virtual css::table::CellRangeAddress SAL_CALL getTitleRows() override;
    virtual void SAL_CALL   setTitleRows(
                                const css::table::CellRangeAddress& aTitleRows ) override;

                            // XSheetLinkable
    virtual css::sheet::SheetLinkMode SAL_CALL getLinkMode() override;
    virtual void SAL_CALL   setLinkMode( css::sheet::SheetLinkMode nLinkMode ) override;
    virtual OUString SAL_CALL getLinkUrl() override;
    virtual void SAL_CALL   setLinkUrl( const OUString& aLinkUrl ) override;
    virtual OUString SAL_CALL getLinkSheetName() override;
    virtual void SAL_CALL   setLinkSheetName( const OUString& aLinkSheetName ) override;
    virtual void SAL_CALL   link( const OUString& aUrl,
                                const OUString& aSheetName,
                                const OUString& aFilterName,
                                const OUString& aFilterOptions,
                                css::sheet::SheetLinkMode nMode ) override;

                            // XSheetAuditing
    virtual sal_Bool SAL_CALL hideDependents( const css::table::CellAddress& aPosition ) override;
    virtual sal_Bool SAL_CALL hidePrecedents( const css::table::CellAddress& aPosition ) override;
    virtual sal_Bool SAL_CALL showDependents( const css::table::CellAddress& aPosition ) override;
    virtual sal_Bool SAL_CALL showPrecedents( const css::table::CellAddress& aPosition ) override;
    virtual sal_Bool SAL_CALL showErrors( const css::table::CellAddress& aPosition ) override;
    virtual sal_Bool SAL_CALL showInvalid() override;
    virtual void SAL_CALL   clearArrows() override;

                            // XSheetOutline
    virtual void SAL_CALL   group( const css::table::CellRangeAddress& aRange,
                                css::table::TableOrientation nOrientation ) override;
    virtual void SAL_CALL   ungroup( const css::table::CellRangeAddress& aRange,
                                css::table::TableOrientation nOrientation ) override;
    virtual void SAL_CALL   autoOutline( const css::table::CellRangeAddress& aRange ) override;
    virtual void SAL_CALL   clearOutline() override;
    virtual void SAL_CALL   hideDetail( const css::table::CellRangeAddress& aRange ) override;
    virtual void SAL_CALL   showDetail( const css::table::CellRangeAddress& aRange ) override;
    virtual void SAL_CALL   showLevel( sal_Int16 nLevel,
                                css::table::TableOrientation nOrientation ) override;

                            // XProtectable
    virtual void SAL_CALL   protect( const OUString& aPassword ) override;
    virtual void SAL_CALL   unprotect( const OUString& aPassword ) override;
    virtual sal_Bool SAL_CALL isProtected() override;

                            // XScenario
    virtual sal_Bool SAL_CALL getIsScenario() override;
    virtual OUString SAL_CALL getScenarioComment() override;
    virtual void SAL_CALL   setScenarioComment( const OUString& aScenarioComment ) override;
    virtual void SAL_CALL   addRanges( const css::uno::Sequence<
                                css::table::CellRangeAddress >& aRanges ) override;
    virtual void SAL_CALL   apply() override;
                            // XScenarioEnhanced
    virtual css::uno::Sequence< css::table::CellRangeAddress > SAL_CALL
                            getRanges(  ) override;

                            // XExternalSheetName
    virtual void SAL_CALL   setExternalName( const OUString& aUrl, const OUString& aSheetName ) override;

                            // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents() override;

                            // XPropertySet override due to sheet properties
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

protected:
    void AdjustUpdatedRanges(UpdateRefMode mode) override;
};

using ScTableColumnObj_BASE = cppu::ImplInheritanceHelper<ScCellRangeObj,
                                                          css::container::XNamed>;
class ScTableColumnObj final : public ScTableColumnObj_BASE
{
private:
    const SfxItemPropertySet*       pColPropSet;

    virtual const SfxItemPropertyMap& GetItemPropertyMap() override;
    virtual void GetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                css::uno::Any& ) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                                const css::uno::Any& aValue ) override;

public:
                            ScTableColumnObj(ScDocShell* pDocSh, SCCOL nCol, SCTAB nTab);
    virtual                 ~ScTableColumnObj() override;

                            // XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL   setName( const OUString& aName ) override;

                            // XPropertySet override due to column properties
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

protected:
    void AdjustUpdatedRanges(UpdateRefMode mode) override;
};

class ScTableRowObj final : public ScCellRangeObj
{
private:
    const SfxItemPropertySet*       pRowPropSet;

    virtual const SfxItemPropertyMap& GetItemPropertyMap() override;
    virtual void GetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                css::uno::Any& ) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                                const css::uno::Any& aValue ) override;

public:
                            ScTableRowObj(ScDocShell* pDocSh, SCROW nRow, SCTAB nTab);
    virtual                 ~ScTableRowObj() override;

                            // XPropertySet override due to row properties
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

protected:
    void AdjustUpdatedRanges(UpdateRefMode mode) override;
};

class ScCellsObj final : public cppu::WeakImplHelper<
                            css::container::XEnumerationAccess,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScRangeList             aRanges;

public:
                            ScCellsObj(ScDocShell* pDocSh, ScRangeList aR);
    virtual                 ~ScCellsObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScCellsEnumeration final : public cppu::WeakImplHelper<
                                css::container::XEnumeration,
                                css::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScRangeList             aRanges;
    ScAddress               aPos;
    std::unique_ptr<ScMarkData> pMark;
    bool                    bAtEnd;

private:
    void                    Advance_Impl();
    void                    CheckPos_Impl();

public:
                            ScCellsEnumeration(ScDocShell* pDocSh, ScRangeList aR);
    virtual                 ~ScCellsEnumeration() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual css::uno::Any SAL_CALL nextElement() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScCellFormatsObj final : public cppu::WeakImplHelper<
                            css::container::XIndexAccess,
                            css::container::XEnumerationAccess,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScRange                 aTotalRange;

private:
    rtl::Reference<ScCellRangeObj> GetObjectByIndex_Impl(tools::Long nIndex) const;

public:
                            ScCellFormatsObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScCellFormatsObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScCellFormatsEnumeration final : public cppu::WeakImplHelper<
                                    css::container::XEnumeration,
                                    css::lang::XServiceInfo >,
                                 public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB                   nTab;
    std::unique_ptr<ScAttrRectIterator> pIter;
    ScRange                 aNext;
    bool                    bAtEnd;
    bool                    bDirty;

private:
    void                    Advance_Impl();
    rtl::Reference<ScCellRangeObj> NextObject_Impl();

public:
                            ScCellFormatsEnumeration(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScCellFormatsEnumeration() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual css::uno::Any SAL_CALL nextElement() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScUniqueCellFormatsObj final : public cppu::WeakImplHelper<
                            css::container::XIndexAccess,
                            css::container::XEnumerationAccess,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*                     pDocShell;
    std::vector<ScRangeList>        aRangeLists;

public:
                            ScUniqueCellFormatsObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScUniqueCellFormatsObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScUniqueCellFormatsEnumeration final : public cppu::WeakImplHelper<
                                    css::container::XEnumeration,
                                    css::lang::XServiceInfo >,
                                 public SfxListener
{
private:
    std::vector<ScRangeList>        aRangeLists;
    ScDocShell*                     pDocShell;
    sal_Int32                       nCurrentPosition;

public:
                            ScUniqueCellFormatsEnumeration(ScDocShell* pDocShell, std::vector<ScRangeList>&& rRangeLists);
    virtual                 ~ScUniqueCellFormatsEnumeration() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() override;
    virtual css::uno::Any SAL_CALL nextElement() override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
