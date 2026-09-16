/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
class ScTableColumnsObj;
class ScTableValidationObj;
class ScCellObj;
class ScScenariosObj;
class SolarMutexGuard;
class SvxUnoTextRangeBase;

namespace editeng { class SvxBorderLine; }

class ScLinkListener final : public SvtListener
{
    Link<const SfxHint&,void>  aLink;
public:
                    ScLinkListener(const Link<const SfxHint&,void>& rL) : aLink(rL) {}
    virtual         ~ScLinkListener() override;
    virtual void Notify( const SfxHint& rHint ) override;
};

typedef std::vector< cpo::uno::Reference< css::util::XModifyListener > > XModifyListenerArr_Impl;

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
    static void AssignTableBorderToAny( cpo::uno::Any& rAny,
            const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner, bool bInvalidateHorVerDist = false );
    static void AssignTableBorder2ToAny( cpo::uno::Any& rAny,
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

    cpo::uno::Reference<css::sheet::XSheetCellRanges>
                            QueryDifferences_Impl(const css::table::CellAddress& aCompare,
                                                    bool bColumnDiff);
    cpo::uno::Reference<cpo::uno::XInterface>
                            Find_Impl(const cpo::uno::Reference<css::util::XSearchDescriptor>& xDesc,
                                    const ScAddress* pLastPos);

protected:
    const ScMarkData*       GetMarkData();

    // GetItemPropertyMap for derived classes must contain all entries, including base class
    SC_DLLPUBLIC virtual const SfxItemPropertyMap& GetItemPropertyMap();
    css::beans::PropertyState GetOnePropertyState(
                                sal_uInt16 nItemWhich, const SfxItemPropertyMapEntry* pEntry );
    /// @throws cpo::uno::RuntimeException
    SC_DLLPUBLIC virtual void GetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                cpo::uno::Any& );
    /// @throws css::lang::IllegalArgumentException
    /// @throws cpo::uno::RuntimeException
    SC_DLLPUBLIC virtual void SetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                                const cpo::uno::Any& aValue );

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
    virtual double computeFunction( css::sheet::GeneralFunction nFunction ) override;
    virtual void   clearContents( sal_Int32 nContentFlags ) override;

                            // XPropertySet
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;
    SC_DLLPUBLIC virtual void   setPropertyValue( const OUString& aPropertyName,
                                    const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue(
                                    const OUString& PropertyName ) override;
    virtual void   addPropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void   removePropertyChangeListener( const OUString& aPropertyName,
                                    const cpo::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void   addVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void   removeVetoableChangeListener( const OUString& PropertyName,
                                    const cpo::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // XMultiPropertySet
    virtual void   setPropertyValues( const cpo::uno::Sequence< OUString >& aPropertyNames,
                                    const cpo::uno::Sequence< cpo::uno::Any >& aValues ) override;
    virtual cpo::uno::Sequence< cpo::uno::Any >
                            getPropertyValues( const cpo::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void addPropertiesChangeListener( const cpo::uno::Sequence< OUString >& aPropertyNames,
                                    const cpo::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void removePropertiesChangeListener( const cpo::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void firePropertiesChangeEvent( const cpo::uno::Sequence< OUString >& aPropertyNames,
                                    const cpo::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

                            // XTolerantMultiPropertySet
    virtual cpo::uno::Sequence< css::beans::SetPropertyTolerantFailed >
        setPropertyValuesTolerant( const cpo::uno::Sequence< OUString >& aPropertyNames,
                                    const cpo::uno::Sequence< cpo::uno::Any >& aValues ) override;
    virtual cpo::uno::Sequence< css::beans::GetPropertyTolerantResult >
        getPropertyValuesTolerant( const cpo::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual cpo::uno::Sequence< css::beans::GetDirectPropertyTolerantResult >
        getDirectPropertyValuesTolerant( const cpo::uno::Sequence< OUString >& aPropertyNames ) override;

                            // XPropertyState
    virtual css::beans::PropertyState getPropertyState(
                                    const OUString& PropertyName ) override;
    virtual cpo::uno::Sequence< css::beans::PropertyState >
                            getPropertyStates( const cpo::uno::Sequence<
                                        OUString >& aPropertyName ) override;
    virtual void   setPropertyToDefault( const OUString& PropertyName ) override;
    virtual cpo::uno::Any getPropertyDefault(
                                    const OUString& aPropertyName ) override;

                            // XIndent
    virtual void   decrementIndent() override;
    virtual void   incrementIndent() override;

                            // XChartData
    virtual void   addChartDataChangeEventListener(
                                const cpo::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual void   removeChartDataChangeEventListener( const cpo::uno::Reference<
                                    css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual double getNotANumber() override;
    virtual bool isNotANumber( double nNumber ) override;

                            // XChartDataArray
    virtual cpo::uno::Sequence< cpo::uno::Sequence< double > >
                            getData() override;
    virtual void   setData( const cpo::uno::Sequence< cpo::uno::Sequence< double > >& aData ) override;
    virtual cpo::uno::Sequence< OUString > getRowDescriptions() override;
    virtual void setRowDescriptions( const cpo::uno::Sequence< OUString >& aRowDescriptions ) override;
    virtual cpo::uno::Sequence< OUString > getColumnDescriptions() override;
    virtual void   setColumnDescriptions( const cpo::uno::Sequence< OUString >& aColumnDescriptions ) override;

                            // XCellRangesQuery
    virtual cpo::uno::Reference< css::sheet::XSheetCellRanges >
                            queryVisibleCells() override;
    virtual cpo::uno::Reference< css::sheet::XSheetCellRanges >
                            queryEmptyCells() override;
    virtual cpo::uno::Reference< css::sheet::XSheetCellRanges >
                            queryContentCells( sal_Int16 nContentFlags ) override;
    virtual cpo::uno::Reference< css::sheet::XSheetCellRanges >
                            queryFormulaCells( sal_Int32 nResultFlags ) override;
    virtual cpo::uno::Reference< css::sheet::XSheetCellRanges >
                            queryColumnDifferences(
                                const css::table::CellAddress& aCompare ) override;
    virtual cpo::uno::Reference< css::sheet::XSheetCellRanges >
                            queryRowDifferences(
                                const css::table::CellAddress& aCompare ) override;
    virtual cpo::uno::Reference< css::sheet::XSheetCellRanges >
                            queryIntersection(
                                const css::table::CellRangeAddress& aRange ) override;

                            // XFormulaQuery
    virtual cpo::uno::Reference< css::sheet::XSheetCellRanges >
                            queryDependents( bool bRecursive ) override;
    virtual cpo::uno::Reference< css::sheet::XSheetCellRanges >
                            queryPrecedents( bool bRecursive ) override;

                            // XSearchable
    virtual cpo::uno::Reference< css::util::XSearchDescriptor >
                            createSearchDescriptor() override;
    virtual cpo::uno::Reference< css::container::XIndexAccess >
                            findAll( const cpo::uno::Reference< css::util::XSearchDescriptor >& xDesc ) override;
    virtual cpo::uno::Reference< cpo::uno::XInterface >
                            findFirst( const cpo::uno::Reference< css::util::XSearchDescriptor >& xDesc ) override;
    virtual cpo::uno::Reference< cpo::uno::XInterface >
                            findNext( const cpo::uno::Reference< cpo::uno::XInterface >& xStartAt,
                                      const cpo::uno::Reference< css::util::XSearchDescriptor >& xDesc ) override;

                            // XReplaceable
    virtual cpo::uno::Reference< css::util::XReplaceDescriptor >
                            createReplaceDescriptor() override;
    virtual sal_Int32 replaceAll( const cpo::uno::Reference< css::util::XSearchDescriptor >& xDesc ) override;

                            // XModifyBroadcaster
    virtual void   addModifyListener( const cpo::uno::Reference< css::util::XModifyListener >& aListener ) override;
    virtual void   removeModifyListener( const cpo::uno::Reference< css::util::XModifyListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
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
    virtual cpo::uno::Reference< css::container::XEnumerationAccess >
                            getCells() override;
    virtual OUString getRangeAddressesAsString() override;
    virtual cpo::uno::Sequence< css::table::CellRangeAddress >
                            getRangeAddresses() override;

                            // XSheetCellRangeContainer
    virtual void   addRangeAddress( const css::table::CellRangeAddress& rRange,
                                        bool bMergeRanges ) override;
    virtual void   removeRangeAddress( const css::table::CellRangeAddress& rRange ) override;
    virtual void   addRangeAddresses( const cpo::uno::Sequence<
                                        css::table::CellRangeAddress >& rRanges,
                                        bool bMergeRanges ) override;
    virtual void   removeRangeAddresses( const cpo::uno::Sequence<
                                        css::table::CellRangeAddress >& rRanges ) override;

                            // XNameContainer
    virtual void   insertByName( const OUString& aName,
                                const cpo::uno::Any& aElement ) override;
    virtual void   removeByName( const OUString& Name ) override;

                            // XNameReplace
    virtual void   replaceByName( const OUString& aName,
                                const cpo::uno::Any& aElement ) override;

                            // XNameAccess
    virtual cpo::uno::Any getByName( const OUString& aName ) override;
    virtual cpo::uno::Sequence< OUString > getElementNames() override;
    virtual bool hasByName( const OUString& aName ) override;

                            // XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

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
                                cpo::uno::Any& ) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                                const cpo::uno::Any& aValue ) override;

    /// @throws css::lang::IndexOutOfBoundsException
    /// @throws cpo::uno::RuntimeException
    rtl::Reference< ScCellObj >
                            GetCellByPosition_Impl( sal_Int32 nColumn, sal_Int32 nRow );

            /// @throws cpo::uno::RuntimeException
            void            SetArrayFormula_Impl( const OUString& rFormula,
                                const formula::FormulaGrammar::Grammar eGrammar );

public:
    SC_DLLPUBLIC            ScCellRangeObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScCellRangeObj() override;

                            // uses ObjectShell from document, if set (returns NULL otherwise)
    static cpo::uno::Reference<css::table::XCellRange>
                            CreateRangeFromDoc( const ScDocument& rDoc, const ScRange& rR );

    virtual void            RefChanged() override;

                            // XCellRangeAddressable
    virtual css::table::CellRangeAddress getRangeAddress() override;

                            // XSheetCellRange
    virtual cpo::uno::Reference< css::sheet::XSpreadsheet >
                            getSpreadsheet() override;

                            // XArrayFormulaRange
    virtual OUString getArrayFormula() override;
    virtual void   setArrayFormula( const OUString& aFormula ) override;

                            // XArrayFormulaTokens
    virtual cpo::uno::Sequence< css::sheet::FormulaToken > getArrayTokens() override;
    SC_DLLPUBLIC virtual void   setArrayTokens( const cpo::uno::Sequence<
                                    css::sheet::FormulaToken >& aTokens ) override;

                            // XCellRangeData
    virtual cpo::uno::Sequence< cpo::uno::Sequence< cpo::uno::Any > > getDataArray() override;
    virtual void   setDataArray( const cpo::uno::Sequence< cpo::uno::Sequence< cpo::uno::Any > >& aArray ) override;

                            // XCellRangeFormula
    virtual cpo::uno::Sequence< cpo::uno::Sequence<
                            OUString > > getFormulaArray() override;
    virtual void   setFormulaArray( const cpo::uno::Sequence< cpo::uno::Sequence< OUString > >& aArray ) override;

                            // XMultipleOperation
    virtual void   setTableOperation(
                                const css::table::CellRangeAddress& aFormulaRange,
                                css::sheet::TableOperationMode nMode,
                                const css::table::CellAddress& aColumnCell,
                                const css::table::CellAddress& aRowCell ) override;

                            // XMergeable
    virtual void   merge( bool bMerge ) override;
    virtual bool getIsMerged() override;

                            // XCellSeries
    virtual void   fillSeries( css::sheet::FillDirection nFillDirection,
                                css::sheet::FillMode nFillMode,
                                css::sheet::FillDateMode nFillDateMode,
                                double fStep, double fEndValue ) override;
    virtual void   fillAuto( css::sheet::FillDirection nFillDirection,
                                sal_Int32 nSourceCount ) override;

                            // XAutoFormattable
    virtual void   autoFormat( const OUString& aName ) override;

                            // XSortable
    virtual cpo::uno::Sequence< css::beans::PropertyValue >
                            createSortDescriptor() override;
    virtual void   sort( const cpo::uno::Sequence<
                                css::beans::PropertyValue >& xDescriptor ) override;

                            // XSheetFilterableEx
    virtual cpo::uno::Reference< css::sheet::XSheetFilterDescriptor >
                            createFilterDescriptorByObject( const cpo::uno::Reference<
                                css::sheet::XSheetFilterable >& xObject ) override;

                            // XSheetFilterable
    virtual cpo::uno::Reference< css::sheet::XSheetFilterDescriptor >
                            createFilterDescriptor( bool bEmpty ) override;
    virtual void   filter( const cpo::uno::Reference<
                                css::sheet::XSheetFilterDescriptor >& xDescriptor ) override;

                            // XSubTotalCalculatable
    virtual cpo::uno::Reference< css::sheet::XSubTotalDescriptor >
                            createSubTotalDescriptor( bool bEmpty ) override;
    virtual void   applySubTotals(const cpo::uno::Reference< css::sheet::XSubTotalDescriptor >& xDescriptor,
                                bool bReplace) override;
    virtual void   removeSubTotals() override;

                            // XImportable
    virtual cpo::uno::Sequence< css::beans::PropertyValue >
                            createImportDescriptor( bool bEmpty ) override;
    virtual void   doImport( const cpo::uno::Sequence< css::beans::PropertyValue >& aDescriptor ) override;

                            // XCellFormatRangesSupplier
    virtual cpo::uno::Reference< css::container::XIndexAccess >
                            getCellFormatRanges() override;

                            // XUniqueCellFormatRangesSupplier
    virtual cpo::uno::Reference< css::container::XIndexAccess >
                            getUniqueCellFormatRanges() override;

                            // XColumnRowRange
    virtual cpo::uno::Reference< css::table::XTableColumns >
                            getColumns() override;
    virtual cpo::uno::Reference< css::table::XTableRows >
                            getRows() override;

                            // XCellRange
    virtual cpo::uno::Reference< css::table::XCell >
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) override;
    virtual cpo::uno::Reference< css::table::XCellRange >
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom ) override;
    virtual cpo::uno::Reference< css::table::XCellRange >
                            getCellRangeByName( const OUString& aRange ) override;
    /// @throws cpo::uno::RuntimeException
    cpo::uno::Reference< css::table::XCellRange >
                            getCellRangeByName( const OUString& aRange,  const ScAddress::Details& rDetails );

                            // XPropertySet override due to Range-Properties
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

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
    SC_DLLPUBLIC rtl::Reference< ScTableColumnsObj > getScColumns();
    SC_DLLPUBLIC rtl::Reference< ScTableRowsObj > getScRows();
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
                                cpo::uno::Any& ) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                                const cpo::uno::Any& aValue ) override;

    SvxUnoTextRangeBase*
    getSvxUnoTextRange(const cpo::uno::Reference<css::text::XTextRange>& xRange);
    bool insertScEditFieldObj(const cpo::uno::Reference<css::text::XTextRange>& xRange,
                              const cpo::uno::Reference<css::text::XTextContent>& xContent,
                              bool bAbsorb);

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
    virtual void   insertTextContent( const cpo::uno::Reference< css::text::XTextRange >& xRange,
                                const cpo::uno::Reference< css::text::XTextContent >& xContent,
                                bool bAbsorb ) override;
    virtual void   removeTextContent( const cpo::uno::Reference< css::text::XTextContent >& xContent ) override;

                            // XSimpleText
    virtual cpo::uno::Reference< css::text::XTextCursor >
                            createTextCursor() override;
    virtual cpo::uno::Reference< css::text::XTextCursor >
                            createTextCursorByRange( const cpo::uno::Reference< css::text::XTextRange >& aTextPosition ) override;
    virtual void   insertString( const cpo::uno::Reference< css::text::XTextRange >& xRange,
                                        const OUString& aString, bool bAbsorb ) override;
    virtual void   insertControlCharacter( const cpo::uno::Reference< css::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, bool bAbsorb ) override;

                            // XTextRange
    virtual cpo::uno::Reference< css::text::XText >
                            getText() override;
    virtual cpo::uno::Reference< css::text::XTextRange >
                            getStart() override;
    virtual cpo::uno::Reference< css::text::XTextRange >
                            getEnd() override;
    virtual OUString getString() override;
    virtual void   setString( const OUString& aString ) override;

                            // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            // XCell
    virtual OUString getFormula() override;
    virtual void   setFormula( const OUString& aFormula ) override;
    virtual void   setFormulaResult( double nValue ) override;
    virtual void   setFormulaString( const OUString& aFormula ) override;
    virtual double getValue() override;
    virtual void   setValue( double nValue ) override;
    virtual css::table::CellContentType getType() override;
    virtual sal_Int32 getError() override;

                            // XFormulaTokens
    virtual cpo::uno::Sequence< css::sheet::FormulaToken > getTokens() override;
    virtual void   setTokens( const cpo::uno::Sequence< css::sheet::FormulaToken >& aTokens ) override;

                            // XCellAddressable
    SC_DLLPUBLIC virtual css::table::CellAddress getCellAddress() override;

                            // XSheetAnnotationAnchor
    virtual cpo::uno::Reference< css::sheet::XSheetAnnotation >
                            getAnnotation() override;

                            // XTextFieldsSupplier
    virtual cpo::uno::Reference< css::container::XEnumerationAccess >
                            getTextFields() override;
    virtual cpo::uno::Reference< css::container::XNameAccess >
                            getTextFieldMasters() override;

                            // XPropertySet override due to cell properties
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

                            // XActionLockable
    virtual bool isActionLocked() override;
    virtual void   addActionLock() override;
    virtual void   removeActionLock() override;
    virtual void   setActionLocks( sal_Int16 nLock ) override;
    virtual sal_Int16 resetActionLocks() override;
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
                                cpo::uno::Any& ) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                                const cpo::uno::Any& aValue ) override;

public:
                            ScTableSheetObj(ScDocShell* pDocSh, SCTAB nTab);
    virtual                 ~ScTableSheetObj() override;

    void                    InitInsertSheet(ScDocShell* pDocSh, SCTAB nTab);

                            // XSpreadsheet
    virtual cpo::uno::Reference< css::sheet::XSheetCellCursor >
                            createCursor() override;
    virtual cpo::uno::Reference< css::sheet::XSheetCellCursor >
                            createCursorByRange( const cpo::uno::Reference<
                                css::sheet::XSheetCellRange >& aRange ) override;

                            // XSheetCellRange
    virtual cpo::uno::Reference< css::sheet::XSpreadsheet >
                            getSpreadsheet() override;

                            // XCellRange
    virtual cpo::uno::Reference< css::table::XCell >
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) override;
    virtual cpo::uno::Reference< css::table::XCellRange >
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom ) override;
    using ScCellRangeObj::getCellRangeByName;
    virtual cpo::uno::Reference< css::table::XCellRange >
                            getCellRangeByName( const OUString& aRange ) override;

                            // XNamed
    virtual OUString getName() override;
    virtual void   setName( const OUString& aName ) override;

                            // XSheetPageBreak
    virtual cpo::uno::Sequence< css::sheet::TablePageBreakData >
                            getColumnPageBreaks() override;
    virtual cpo::uno::Sequence< css::sheet::TablePageBreakData >
                            getRowPageBreaks() override;
    virtual void   removeAllManualPageBreaks() override;

                            // XCellRangeMovement
    virtual void   insertCells( const css::table::CellRangeAddress& aRange,
                                css::sheet::CellInsertMode nMode ) override;
    virtual void   removeRange( const css::table::CellRangeAddress& aRange,
                                css::sheet::CellDeleteMode nMode ) override;
    virtual void   moveRange( const css::table::CellAddress& aDestination,
                                const css::table::CellRangeAddress& aSource ) override;
    virtual void   copyRange( const css::table::CellAddress& aDestination,
                                const css::table::CellRangeAddress& aSource ) override;

                            // XTableChartsSupplier
    virtual cpo::uno::Reference< css::table::XTableCharts >
                            getCharts() override;

                            // XTablePivotChartsSupplier
    virtual cpo::uno::Reference<css::table::XTablePivotCharts>
                            getPivotCharts() override;

                            // XDataPilotTablesSupplier
    virtual cpo::uno::Reference< css::sheet::XDataPilotTables >
                            getDataPilotTables() override;

                            // XScenariosSupplier
    virtual cpo::uno::Reference< css::sheet::XScenarios >
                            getScenarios() override;

                            // XSheetAnnotationsSupplier
    virtual cpo::uno::Reference< css::sheet::XSheetAnnotations >
                            getAnnotations() override;

                            // XDrawPageSupplier
    virtual cpo::uno::Reference< css::drawing::XDrawPage >
                            getDrawPage() override;

                            // XPrintAreas
    virtual cpo::uno::Sequence< css::table::CellRangeAddress >
                            getPrintAreas() override;
    virtual void setPrintAreas( const cpo::uno::Sequence<
                                css::table::CellRangeAddress >& aPrintAreas ) override;
    virtual bool getPrintTitleColumns() override;
    virtual void   setPrintTitleColumns( bool bPrintTitleColumns ) override;
    virtual css::table::CellRangeAddress getTitleColumns() override;
    virtual void   setTitleColumns(
                                const css::table::CellRangeAddress& aTitleColumns ) override;
    virtual bool getPrintTitleRows() override;
    virtual void   setPrintTitleRows( bool bPrintTitleRows ) override;
    virtual css::table::CellRangeAddress getTitleRows() override;
    virtual void   setTitleRows(
                                const css::table::CellRangeAddress& aTitleRows ) override;

                            // XSheetLinkable
    virtual css::sheet::SheetLinkMode getLinkMode() override;
    virtual void   setLinkMode( css::sheet::SheetLinkMode nLinkMode ) override;
    virtual OUString getLinkUrl() override;
    virtual void   setLinkUrl( const OUString& aLinkUrl ) override;
    virtual OUString getLinkSheetName() override;
    virtual void   setLinkSheetName( const OUString& aLinkSheetName ) override;
    virtual void   link( const OUString& aUrl,
                                const OUString& aSheetName,
                                const OUString& aFilterName,
                                const OUString& aFilterOptions,
                                css::sheet::SheetLinkMode nMode ) override;

                            // XSheetAuditing
    virtual bool hideDependents( const css::table::CellAddress& aPosition ) override;
    virtual bool hidePrecedents( const css::table::CellAddress& aPosition ) override;
    virtual bool showDependents( const css::table::CellAddress& aPosition ) override;
    virtual bool showPrecedents( const css::table::CellAddress& aPosition ) override;
    virtual bool showErrors( const css::table::CellAddress& aPosition ) override;
    virtual bool showInvalid() override;
    virtual void   clearArrows() override;

                            // XSheetOutline
    virtual void   group( const css::table::CellRangeAddress& aRange,
                                css::table::TableOrientation nOrientation ) override;
    virtual void   ungroup( const css::table::CellRangeAddress& aRange,
                                css::table::TableOrientation nOrientation ) override;
    virtual void   autoOutline( const css::table::CellRangeAddress& aRange ) override;
    virtual void   clearOutline() override;
    virtual void   hideDetail( const css::table::CellRangeAddress& aRange ) override;
    virtual void   showDetail( const css::table::CellRangeAddress& aRange ) override;
    virtual void   showLevel( sal_Int16 nLevel,
                                css::table::TableOrientation nOrientation ) override;

                            // XProtectable
    virtual void   protect( const OUString& aPassword ) override;
    virtual void   unprotect( const OUString& aPassword ) override;
    virtual bool isProtected() override;

                            // XScenario
    virtual bool getIsScenario() override;
    virtual OUString getScenarioComment() override;
    virtual void   setScenarioComment( const OUString& aScenarioComment ) override;
    virtual void   addRanges( const cpo::uno::Sequence<
                                css::table::CellRangeAddress >& aRanges ) override;
    virtual void   apply() override;
                            // XScenarioEnhanced
    virtual cpo::uno::Sequence< css::table::CellRangeAddress >
                            getRanges(  ) override;

                            // XExternalSheetName
    virtual void   setExternalName( const OUString& aUrl, const OUString& aSheetName ) override;

                            // XEventsSupplier
    virtual cpo::uno::Reference< css::container::XNameReplace > getEvents() override;

                            // XPropertySet override due to sheet properties
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

    rtl::Reference< ScScenariosObj > getScScenarios() const;

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
                                cpo::uno::Any& ) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                                const cpo::uno::Any& aValue ) override;

public:
                            ScTableColumnObj(ScDocShell* pDocSh, SCCOL nCol, SCTAB nTab);
    virtual                 ~ScTableColumnObj() override;

                            // XNamed
    virtual OUString getName() override;
    virtual void   setName( const OUString& aName ) override;

                            // XPropertySet override due to column properties
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

protected:
    void AdjustUpdatedRanges(UpdateRefMode mode) override;
};

class ScTableRowObj final : public ScCellRangeObj
{
private:
    const SfxItemPropertySet*       pRowPropSet;

    virtual const SfxItemPropertyMap& GetItemPropertyMap() override;
    virtual void GetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                cpo::uno::Any& ) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertyMapEntry* pEntry,
                                                const cpo::uno::Any& aValue ) override;

public:
                            ScTableRowObj(ScDocShell* pDocSh, SCROW nRow, SCTAB nTab);
    virtual                 ~ScTableRowObj() override;

                            // XPropertySet override due to row properties
    virtual cpo::uno::Reference< css::beans::XPropertySetInfo >
                            getPropertySetInfo() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

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
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
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
    virtual bool hasMoreElements() override;
    virtual cpo::uno::Any nextElement() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
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
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
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
    virtual bool hasMoreElements() override;
    virtual cpo::uno::Any nextElement() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
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
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual cpo::uno::Reference< css::container::XEnumeration >
                            createEnumeration() override;

                            // XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
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
    virtual bool hasMoreElements() override;
    virtual cpo::uno::Any nextElement() override;

                            // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
