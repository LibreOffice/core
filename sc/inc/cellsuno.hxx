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

#ifndef INCLUDED_SC_INC_CELLSUNO_HXX
#define INCLUDED_SC_INC_CELLSUNO_HXX

#include "global.hxx"
#include "rangelst.hxx"

#include <editeng/unotext.hxx>
#include <formula/grammar.hxx>
#include <rtl/ref.hxx>
#include <tools/link.hxx>
#include <svl/lstner.hxx>
#include <svl/listener.hxx>
#include <svl/itemprop.hxx>
#include <com/sun/star/table/XTableChartsSupplier.hpp>
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
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/TableBorder2.hpp>
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
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/beans/XTolerantMultiPropertySet.hpp>
#include <com/sun/star/sheet/XExternalSheetName.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <cppuhelper/implbase.hxx>

#include <memory>
#include <vector>

class ScDocShell;
class ScMarkData;
class ScMemChart;
class ScPrintRangeSaver;
class ScAttrRectIterator;
class ScCellRangeObj;
class ScLinkListener;
class ScPatternAttr;
class SvxBoxItem;
class SvxBoxInfoItem;
class SvxItemPropertySet;

namespace editeng { class SvxBorderLine; }

class ScLinkListener : public SvtListener
{
    Link<const SfxHint&,void>  aLink;
public:
                    ScLinkListener(const Link<const SfxHint&,void>& rL) : aLink(rL) {}
    virtual         ~ScLinkListener();
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

namespace ooo
{
    namespace vba {
        namespace excel {
    class ScVbaCellRangeAccess;  // Vba Helper class
    }
    }
}

class SC_DLLPUBLIC ScCellRangesBase : public css::beans::XPropertySet,
                         public css::beans::XMultiPropertySet,
                         public css::beans::XPropertyState,
                         public css::sheet::XSheetOperation,
                         public css::chart::XChartDataArray,
                         public css::util::XIndent,
                         public css::sheet::XCellRangesQuery,
                         public css::sheet::XFormulaQuery,
                         public css::util::XReplaceable,
                         public css::util::XModifyBroadcaster,
                         public css::lang::XServiceInfo,
                         public css::lang::XUnoTunnel,
                         public css::lang::XTypeProvider,
                         public css::beans::XTolerantMultiPropertySet,
                         public cppu::OWeakObject,
                         public SfxListener
{
    friend class ScTabViewObj;      // for select()
    friend class ScTableSheetObj;   // for createCursorByRange()
    friend class ooo::vba::excel::ScVbaCellRangeAccess;

private:
    css::uno::WeakReference<css::uno::XInterface> m_wThis;
    const SfxItemPropertySet* pPropSet;
    ScDocShell*             pDocShell;
    ScLinkListener*         pValueListener;
    ScPatternAttr*          pCurrentFlat;
    ScPatternAttr*          pCurrentDeep;
    SfxItemSet*             pCurrentDataSet;
    SfxItemSet*             pNoDfltCurrentDataSet;
    ScMarkData*             pMarkData;
    ScRangeList             aRanges;
    sal_Int64               nObjectId;
    bool                    bChartColAsHdr;
    bool                    bChartRowAsHdr;
    bool                    bCursorOnly;
    bool                    bGotDataChangedHint;
    XModifyListenerArr_Impl aValueListeners;

    DECL_LINK_TYPED( ValueListenerHdl, const SfxHint&, void );

private:
    void            PaintRanges_Impl( sal_uInt16 nPart );
    ScRangeListRef  GetLimitedChartRanges_Impl( long nDataColumns, long nDataRows ) const;
    void            ForceChartListener_Impl();
    ScMemChart*     CreateMemChart_Impl() const;

    const ScPatternAttr*    GetCurrentAttrsFlat();
    const ScPatternAttr*    GetCurrentAttrsDeep();
    SfxItemSet*             GetCurrentDataSet(bool bNoDflt = false);
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
    virtual const SfxItemPropertyMap& GetItemPropertyMap();
    css::beans::PropertyState GetOnePropertyState(
                                sal_uInt16 nItemWhich, const SfxItemPropertySimpleEntry* pEntry );
    virtual void            GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                css::uno::Any& )
                                throw(css::uno::RuntimeException,
                                      std::exception);
    virtual void            SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                                const css::uno::Any& aValue )
                                throw(css::lang::IllegalArgumentException,
                                      css::uno::RuntimeException,
                                      std::exception);

public:
                            ScCellRangesBase(ScDocShell* pDocSh, const ScRange& rR);
                            ScCellRangesBase(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual                 ~ScCellRangesBase();

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
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

                            // XSheetOperation
    virtual double SAL_CALL computeFunction( css::sheet::GeneralFunction nFunction )
                                throw(css::uno::Exception,
                                      css::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL   clearContents( sal_Int32 nContentFlags )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue )
                                throw(css::beans::UnknownPropertyException,
                                    css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XMultiPropertySet
    virtual void SAL_CALL   setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Sequence< css::uno::Any >& aValues )
                                throw (css::beans::PropertyVetoException,
                                    css::lang::IllegalArgumentException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL
                            getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames )
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener )
                                throw (css::uno::RuntimeException, std::exception) override;

                            // XTolerantMultiPropertySet
    virtual css::uno::Sequence< css::beans::SetPropertyTolerantFailed > SAL_CALL
        setPropertyValuesTolerant( const css::uno::Sequence< OUString >& aPropertyNames,
                                    const css::uno::Sequence< css::uno::Any >& aValues )
                                    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::GetPropertyTolerantResult > SAL_CALL
        getPropertyValuesTolerant( const css::uno::Sequence< OUString >& aPropertyNames )
                                    throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::GetDirectPropertyTolerantResult > SAL_CALL
        getDirectPropertyValuesTolerant( const css::uno::Sequence< OUString >& aPropertyNames )
                                    throw (css::uno::RuntimeException, std::exception) override;

                            // XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState(
                                    const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL
                            getPropertyStates( const css::uno::Sequence<
                                        OUString >& aPropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyToDefault( const OUString& PropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault(
                                    const OUString& aPropertyName )
                                throw(css::beans::UnknownPropertyException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XIndent
    virtual void SAL_CALL   decrementIndent() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   incrementIndent() throw(css::uno::RuntimeException, std::exception) override;

                            // XChartData
    virtual void SAL_CALL   addChartDataChangeEventListener(
                                const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL   removeChartDataChangeEventListener( const css::uno::Reference<
                                    css::chart::XChartDataChangeEventListener >& aListener )
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual double SAL_CALL getNotANumber() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XChartDataArray
    virtual css::uno::Sequence< css::uno::Sequence< double > > SAL_CALL
                            getData()
                                    throw(css::uno::RuntimeException,
                                          std::exception) override;
    virtual void SAL_CALL   setData( const css::uno::Sequence< css::uno::Sequence< double > >& aData )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getRowDescriptions()
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL setRowDescriptions( const css::uno::Sequence< OUString >& aRowDescriptions )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getColumnDescriptions()
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL   setColumnDescriptions( const css::uno::Sequence< OUString >& aColumnDescriptions )
                                    throw(css::uno::RuntimeException,
                                          std::exception) override;

                            // XCellRangesQuery
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryVisibleCells()
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryEmptyCells()
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryContentCells( sal_Int16 nContentFlags )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryFormulaCells( sal_Int32 nResultFlags )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryColumnDifferences(
                                const css::table::CellAddress& aCompare )
                                    throw(css::uno::RuntimeException,
                                          std::exception) override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryRowDifferences(
                                const css::table::CellAddress& aCompare )
                                    throw(css::uno::RuntimeException,
                                          std::exception) override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryIntersection(
                                const css::table::CellRangeAddress& aRange )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XFormulaQuery
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryDependents( sal_Bool bRecursive )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual css::uno::Reference< css::sheet::XSheetCellRanges > SAL_CALL
                            queryPrecedents( sal_Bool bRecursive )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;

                            // XSearchable
    virtual css::uno::Reference< css::util::XSearchDescriptor > SAL_CALL
                            createSearchDescriptor()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
                            findAll( const css::uno::Reference< css::util::XSearchDescriptor >& xDesc )
                                    throw(css::uno::RuntimeException,
                                          std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
                            findFirst( const css::uno::Reference< css::util::XSearchDescriptor >& xDesc )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
                            findNext( const css::uno::Reference< css::uno::XInterface >& xStartAt,
                                      const css::uno::Reference< css::util::XSearchDescriptor >& xDesc )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XReplaceable
    virtual css::uno::Reference< css::util::XReplaceDescriptor > SAL_CALL
                            createReplaceDescriptor() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL replaceAll( const css::uno::Reference< css::util::XSearchDescriptor >& xDesc )
                                    throw(css::uno::RuntimeException,
                                          std::exception) override;

                            // XModifyBroadcaster
    virtual void SAL_CALL   addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener )
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener )
                                throw (css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScCellRangesBase* getImplementation(const css::uno::Reference<css::uno::XInterface>& rObj);

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class SC_DLLPUBLIC ScCellRangesObj : public ScCellRangesBase,
                        public css::sheet::XSheetCellRangeContainer,
                        public css::container::XNameContainer,
                        public css::container::XEnumerationAccess
{
private:
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;

    ScCellRangeObj*         GetObjectByIndex_Impl(sal_Int32 nIndex) const;

public:
                            ScCellRangesObj(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual                 ~ScCellRangesObj();

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

    virtual void            RefChanged() override;

                            // XSheetCellRanges
    virtual css::uno::Reference< css::container::XEnumerationAccess > SAL_CALL
                            getCells() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getRangeAddressesAsString()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::table::CellRangeAddress > SAL_CALL
                            getRangeAddresses() throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetCellRangeContainer
    virtual void SAL_CALL   addRangeAddress( const css::table::CellRangeAddress& rRange,
                                        sal_Bool bMergeRanges )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeRangeAddress( const css::table::CellRangeAddress& rRange )
                                throw(css::container::NoSuchElementException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addRangeAddresses( const css::uno::Sequence<
                                        css::table::CellRangeAddress >& rRanges,
                                        sal_Bool bMergeRanges )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeRangeAddresses( const css::uno::Sequence<
                                        css::table::CellRangeAddress >& rRanges )
                                throw(css::container::NoSuchElementException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XNameContainer
    virtual void SAL_CALL   insertByName( const OUString& aName,
                                const css::uno::Any& aElement )
                                    throw (css::lang::IllegalArgumentException,
                                           css::container::ElementExistException,
                                           css::lang::WrappedTargetException,
                                           css::uno::RuntimeException,
                                           std::exception) override;
    virtual void SAL_CALL   removeByName( const OUString& Name )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XNameReplace
    virtual void SAL_CALL   replaceByName( const OUString& aName,
                                const css::uno::Any& aElement )
                                    throw(css::lang::IllegalArgumentException,
                                        css::container::NoSuchElementException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception) override;

                            // XNameAccess
    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class SC_DLLPUBLIC ScCellRangeObj : public ScCellRangesBase,
                       public css::sheet::XCellRangeAddressable,
                       public css::sheet::XSheetCellRange,
                       public css::sheet::XArrayFormulaRange,
                       public css::sheet::XArrayFormulaTokens,
                       public css::sheet::XCellRangeData,
                       public css::sheet::XCellRangeFormula,
                       public css::sheet::XMultipleOperation,
                       public css::util::XMergeable,
                       public css::sheet::XCellSeries,
                       public css::table::XAutoFormattable,
                       public css::util::XSortable,
                       public css::sheet::XSheetFilterableEx,
                       public css::sheet::XSubTotalCalculatable,
                       public css::util::XImportable,
                       public css::sheet::XCellFormatRangesSupplier,
                       public css::sheet::XUniqueCellFormatRangesSupplier,
                       public css::table::XColumnRowRange
{
private:
    const SfxItemPropertySet*       pRangePropSet;
    ScRange                 aRange;

protected:
    const ScRange&          GetRange() const    { return aRange; }
    virtual const SfxItemPropertyMap& GetItemPropertyMap() override;
    virtual void GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                css::uno::Any& )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                                const css::uno::Any& aValue )
                                throw(css::lang::IllegalArgumentException,
                                      css::uno::RuntimeException,
                                      std::exception) override;

    css::uno::Reference< css::table::XCell >
                            GetCellByPosition_Impl( sal_Int32 nColumn, sal_Int32 nRow )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::uno::RuntimeException);

            void            SetArrayFormula_Impl( const OUString& rFormula,
                                const OUString& rFormulaNmsp,
                                const formula::FormulaGrammar::Grammar eGrammar )
                                    throw (css::uno::RuntimeException, std::exception);

public:
                            ScCellRangeObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScCellRangeObj();

                            // uses ObjectShell from document, if set (returns NULL otherwise)
    static css::uno::Reference<css::table::XCellRange>
                            CreateRangeFromDoc( ScDocument* pDoc, const ScRange& rR );

    virtual css::uno::Any SAL_CALL queryInterface(
                                const css::uno::Type & rType )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

    virtual void            RefChanged() override;

                            // XCellRangeAddressable
    virtual css::table::CellRangeAddress SAL_CALL getRangeAddress()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetCellRange
    virtual css::uno::Reference< css::sheet::XSpreadsheet > SAL_CALL
                            getSpreadsheet() throw(css::uno::RuntimeException, std::exception) override;

                            // XArrayFormulaRange
    virtual OUString SAL_CALL getArrayFormula() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setArrayFormula( const OUString& aFormula )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XArrayFormulaTokens
    virtual css::uno::Sequence< css::sheet::FormulaToken > SAL_CALL getArrayTokens()
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   setArrayTokens( const css::uno::Sequence<
                                    css::sheet::FormulaToken >& aTokens )
                                throw (css::uno::RuntimeException, std::exception) override;

                            // XCellRangeData
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > > SAL_CALL getDataArray()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setDataArray( const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& aArray )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XCellRangeFormula
    virtual css::uno::Sequence< css::uno::Sequence<
                            OUString > > SAL_CALL getFormulaArray()
                                throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFormulaArray( const css::uno::Sequence< css::uno::Sequence< OUString > >& aArray )
                                throw (css::uno::RuntimeException, std::exception) override;

                            // XMultipleOperation
    virtual void SAL_CALL   setTableOperation(
                                const css::table::CellRangeAddress& aFormulaRange,
                                css::sheet::TableOperationMode nMode,
                                const css::table::CellAddress& aColumnCell,
                                const css::table::CellAddress& aRowCell )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XMergeable
    virtual void SAL_CALL   merge( sal_Bool bMerge ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getIsMerged() throw(css::uno::RuntimeException, std::exception) override;

                            // XCellSeries
    virtual void SAL_CALL   fillSeries( css::sheet::FillDirection nFillDirection,
                                css::sheet::FillMode nFillMode,
                                css::sheet::FillDateMode nFillDateMode,
                                double fStep, double fEndValue )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   fillAuto( css::sheet::FillDirection nFillDirection,
                                sal_Int32 nSourceCount )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XAutoFormattable
    virtual void SAL_CALL   autoFormat( const OUString& aName )
                                throw(css::lang::IllegalArgumentException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XSortable
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            createSortDescriptor() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   sort( const css::uno::Sequence<
                                css::beans::PropertyValue >& xDescriptor )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetFilterableEx
    virtual css::uno::Reference< css::sheet::XSheetFilterDescriptor > SAL_CALL
                            createFilterDescriptorByObject( const css::uno::Reference<
                                css::sheet::XSheetFilterable >& xObject )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetFilterable
    virtual css::uno::Reference< css::sheet::XSheetFilterDescriptor > SAL_CALL
                            createFilterDescriptor( sal_Bool bEmpty )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   filter( const css::uno::Reference<
                                css::sheet::XSheetFilterDescriptor >& xDescriptor )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XSubTotalCalculatable
    virtual css::uno::Reference< css::sheet::XSubTotalDescriptor > SAL_CALL
                            createSubTotalDescriptor( sal_Bool bEmpty )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   applySubTotals(const css::uno::Reference< css::sheet::XSubTotalDescriptor >& xDescriptor,
                                sal_Bool bReplace)
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   removeSubTotals() throw(css::uno::RuntimeException, std::exception) override;

                            // XImportable
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
                            createImportDescriptor( sal_Bool bEmpty )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   doImport( const css::uno::Sequence< css::beans::PropertyValue >& aDescriptor )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XCellFormatRangesSupplier
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
                            getCellFormatRanges() throw(css::uno::RuntimeException, std::exception) override;

                            // XUniqueCellFormatRangesSupplier
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL
                            getUniqueCellFormatRanges() throw(css::uno::RuntimeException, std::exception) override;

                            // XColumnRowRange
    virtual css::uno::Reference< css::table::XTableColumns > SAL_CALL
                            getColumns() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::table::XTableRows > SAL_CALL
                            getRows() throw(css::uno::RuntimeException, std::exception) override;

                            // XCellRange
    virtual css::uno::Reference< css::table::XCell > SAL_CALL
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom )
                                    throw(css::lang::IndexOutOfBoundsException,
                                        css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getCellRangeByName( const OUString& aRange )
                                throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::table::XCellRange >
                            getCellRangeByName( const OUString& aRange,  const ScAddress::Details& rDetails )
                                throw(css::uno::RuntimeException);

                            // XPropertySet override due to Range-Properties
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;
};

//! really derive cell from range?

class SC_DLLPUBLIC ScCellObj : public ScCellRangeObj,
                    public css::text::XText,
                    public css::container::XEnumerationAccess,
                    public css::table::XCell2,
                    public css::sheet::XFormulaTokens,
                    public css::sheet::XCellAddressable,
                    public css::sheet::XSheetAnnotationAnchor,
                    public css::text::XTextFieldsSupplier,
                    public css::document::XActionLockable
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
    css::table::CellContentType GetResultType_Impl();

protected:
    virtual const SfxItemPropertyMap& GetItemPropertyMap() override;
    virtual void GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                css::uno::Any& )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                                const css::uno::Any& aValue )
                                throw(css::lang::IllegalArgumentException,
                                        css::uno::RuntimeException, std::exception) override;

public:
    static const SvxItemPropertySet* GetEditPropertySet();
    static const SfxItemPropertyMap& GetCellPropertyMap();

                            ScCellObj(ScDocShell* pDocSh, const ScAddress& rP);
    virtual                 ~ScCellObj();

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

    virtual void            RefChanged() override;

    SvxUnoText&             GetUnoText();

                            // XML import needs to set results at formula cells,
                            // not meant for any other purpose.
    const ScAddress&        GetPosition() const { return aCellPos; }

    void                    InputEnglishString( const OUString& rText );

                            // XText
    virtual void SAL_CALL   insertTextContent( const css::uno::Reference< css::text::XTextRange >& xRange,
                                const css::uno::Reference< css::text::XTextContent >& xContent,
                                sal_Bool bAbsorb )
                                    throw(css::lang::IllegalArgumentException,
                                          css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeTextContent( const css::uno::Reference< css::text::XTextContent >& xContent )
                                    throw(css::container::NoSuchElementException,
                                          css::uno::RuntimeException, std::exception) override;

                            // XSimpleText
    virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL
                            createTextCursor() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL
                            createTextCursorByRange( const css::uno::Reference< css::text::XTextRange >& aTextPosition )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   insertString( const css::uno::Reference< css::text::XTextRange >& xRange,
                                        const OUString& aString, sal_Bool bAbsorb )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   insertControlCharacter( const css::uno::Reference< css::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                    throw(css::lang::IllegalArgumentException,
                                        css::uno::RuntimeException, std::exception) override;

                            // XTextRange
    virtual css::uno::Reference< css::text::XText > SAL_CALL
                            getText() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getStart() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getEnd() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setString( const OUString& aString )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XCell
    virtual OUString SAL_CALL getFormula() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFormula( const OUString& aFormula )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFormulaResult( double nValue )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setFormulaString( const OUString& aFormula )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getValue() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setValue( double nValue ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::table::CellContentType SAL_CALL getType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getError() throw(css::uno::RuntimeException, std::exception) override;

                            // XFormulaTokens
    virtual css::uno::Sequence< css::sheet::FormulaToken > SAL_CALL getTokens()
                                throw (css::uno::RuntimeException,
                                       std::exception) override;
    virtual void SAL_CALL   setTokens( const css::uno::Sequence< css::sheet::FormulaToken >& aTokens )
                                throw (css::uno::RuntimeException, std::exception) override;

                            // XCellAddressable
    virtual css::table::CellAddress SAL_CALL getCellAddress()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetAnnotationAnchor
    virtual css::uno::Reference< css::sheet::XSheetAnnotation > SAL_CALL
                            getAnnotation() throw(css::uno::RuntimeException, std::exception) override;

                            // XTextFieldsSupplier
    virtual css::uno::Reference< css::container::XEnumerationAccess > SAL_CALL
                            getTextFields() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL
                            getTextFieldMasters() throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet override due to cell properties
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addActionLock() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeActionLock() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setActionLocks( sal_Int16 nLock )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL resetActionLocks() throw(css::uno::RuntimeException, std::exception) override;
};

class ScTableSheetObj : public ScCellRangeObj,
                        public css::sheet::XSpreadsheet,
                        public css::container::XNamed,
                        public css::sheet::XSheetPageBreak,
                        public css::sheet::XCellRangeMovement,
                        public css::table::XTableChartsSupplier,
                        public css::sheet::XDataPilotTablesSupplier,
                        public css::sheet::XScenariosSupplier,
                        public css::sheet::XSheetAnnotationsSupplier,
                        public css::drawing::XDrawPageSupplier,
                        public css::sheet::XPrintAreas,
                        public css::sheet::XSheetLinkable,
                        public css::sheet::XSheetAuditing,
                        public css::sheet::XSheetOutline,
                        public css::util::XProtectable,
                        public css::sheet::XScenario,
                        public css::sheet::XScenarioEnhanced,
                        public css::sheet::XExternalSheetName,
                        public css::document::XEventsSupplier
{
    friend class ScTableSheetsObj;      // for insertByName()

private:
    const SfxItemPropertySet*       pSheetPropSet;

    SCTAB                   GetTab_Impl() const;
    void                    PrintAreaUndo_Impl( ScPrintRangeSaver* pOldRanges );

protected:
    virtual const SfxItemPropertyMap& GetItemPropertyMap() override;
    virtual void GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                css::uno::Any& )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                                const css::uno::Any& aValue )
                                throw(css::lang::IllegalArgumentException,
                                      css::uno::RuntimeException,
                                      std::exception) override;

public:
                            ScTableSheetObj(ScDocShell* pDocSh, SCTAB nTab);
    virtual                 ~ScTableSheetObj();

    void                    InitInsertSheet(ScDocShell* pDocSh, SCTAB nTab);

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

                            // XSpreadsheet
    virtual css::uno::Reference< css::sheet::XSheetCellCursor >
                            SAL_CALL createCursor() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sheet::XSheetCellCursor > SAL_CALL
                            createCursorByRange( const css::uno::Reference<
                                css::sheet::XSheetCellRange >& aRange )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetCellRange
    virtual css::uno::Reference< css::sheet::XSpreadsheet > SAL_CALL
                            getSpreadsheet() throw(css::uno::RuntimeException, std::exception) override;

                            // XCellRange
    virtual css::uno::Reference< css::table::XCell > SAL_CALL
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom )
                                    throw(css::lang::IndexOutOfBoundsException,
                                        css::uno::RuntimeException, std::exception) override;
    using ScCellRangeObj::getCellRangeByName;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL
                            getCellRangeByName( const OUString& aRange )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetPageBreak
    virtual css::uno::Sequence< css::sheet::TablePageBreakData > SAL_CALL
                            getColumnPageBreaks() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::sheet::TablePageBreakData > SAL_CALL
                            getRowPageBreaks() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeAllManualPageBreaks() throw(css::uno::RuntimeException, std::exception) override;

                            // XCellRangeMovement
    virtual void SAL_CALL   insertCells( const css::table::CellRangeAddress& aRange,
                                css::sheet::CellInsertMode nMode )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeRange( const css::table::CellRangeAddress& aRange,
                                css::sheet::CellDeleteMode nMode )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   moveRange( const css::table::CellAddress& aDestination,
                                const css::table::CellRangeAddress& aSource )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   copyRange( const css::table::CellAddress& aDestination,
                                const css::table::CellRangeAddress& aSource )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XTableChartsSupplier
    virtual css::uno::Reference< css::table::XTableCharts > SAL_CALL
                            getCharts() throw(css::uno::RuntimeException, std::exception) override;

                            // XDataPilotTablesSupplier
    virtual css::uno::Reference< css::sheet::XDataPilotTables > SAL_CALL
                            getDataPilotTables() throw(css::uno::RuntimeException, std::exception) override;

                            // XScenariosSupplier
    virtual css::uno::Reference< css::sheet::XScenarios > SAL_CALL
                            getScenarios() throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetAnnotationsSupplier
    virtual css::uno::Reference< css::sheet::XSheetAnnotations > SAL_CALL
                            getAnnotations() throw(css::uno::RuntimeException, std::exception) override;

                            // XDrawPageSupplier
    virtual css::uno::Reference< css::drawing::XDrawPage > SAL_CALL
                            getDrawPage() throw(css::uno::RuntimeException, std::exception) override;

                            // XPrintAreas
    virtual css::uno::Sequence< css::table::CellRangeAddress > SAL_CALL
                            getPrintAreas() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPrintAreas( const css::uno::Sequence<
                                css::table::CellRangeAddress >& aPrintAreas )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getPrintTitleColumns() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPrintTitleColumns( sal_Bool bPrintTitleColumns )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::table::CellRangeAddress SAL_CALL getTitleColumns()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setTitleColumns(
                                const css::table::CellRangeAddress& aTitleColumns )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getPrintTitleRows() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPrintTitleRows( sal_Bool bPrintTitleRows )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::table::CellRangeAddress SAL_CALL getTitleRows()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setTitleRows(
                                const css::table::CellRangeAddress& aTitleRows )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetLinkable
    virtual css::sheet::SheetLinkMode SAL_CALL getLinkMode()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setLinkMode( css::sheet::SheetLinkMode nLinkMode )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getLinkUrl() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setLinkUrl( const OUString& aLinkUrl )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getLinkSheetName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setLinkSheetName( const OUString& aLinkSheetName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   link( const OUString& aUrl,
                                const OUString& aSheetName,
                                const OUString& aFilterName,
                                const OUString& aFilterOptions,
                                css::sheet::SheetLinkMode nMode )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetAuditing
    virtual sal_Bool SAL_CALL hideDependents( const css::table::CellAddress& aPosition )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hidePrecedents( const css::table::CellAddress& aPosition )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL showDependents( const css::table::CellAddress& aPosition )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL showPrecedents( const css::table::CellAddress& aPosition )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL showErrors( const css::table::CellAddress& aPosition )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL showInvalid() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   clearArrows() throw(css::uno::RuntimeException, std::exception) override;

                            // XSheetOutline
    virtual void SAL_CALL   group( const css::table::CellRangeAddress& aRange,
                                css::table::TableOrientation nOrientation )
                                    throw (css::uno::RuntimeException,
                                           std::exception) override;
    virtual void SAL_CALL   ungroup( const css::table::CellRangeAddress& aRange,
                                css::table::TableOrientation nOrientation )
                                    throw (css::uno::RuntimeException,
                                           std::exception) override;
    virtual void SAL_CALL   autoOutline( const css::table::CellRangeAddress& aRange )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL   clearOutline()
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL   hideDetail( const css::table::CellRangeAddress& aRange )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL   showDetail( const css::table::CellRangeAddress& aRange )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void SAL_CALL   showLevel( sal_Int16 nLevel,
                                css::table::TableOrientation nOrientation )
                                    throw(css::uno::RuntimeException,
                                          std::exception) override;

                            // XProtectable
    virtual void SAL_CALL   protect( const OUString& aPassword )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   unprotect( const OUString& aPassword )
                                throw(css::lang::IllegalArgumentException,
                                    css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isProtected() throw(css::uno::RuntimeException, std::exception) override;

                            // XScenario
    virtual sal_Bool SAL_CALL getIsScenario() throw(css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getScenarioComment() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setScenarioComment( const OUString& aScenarioComment )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addRanges( const css::uno::Sequence<
                                css::table::CellRangeAddress >& aRanges )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   apply() throw(css::uno::RuntimeException, std::exception) override;
                            // XScenarioEnhanced
    virtual css::uno::Sequence< css::table::CellRangeAddress > SAL_CALL
                            getRanges(  )
                                    throw(css::uno::RuntimeException, std::exception) override;

                            // XExternalSheetName
    virtual void SAL_CALL   setExternalName( const OUString& aUrl, const OUString& aSheetName )
                                throw (css::container::ElementExistException,
                                       css::uno::RuntimeException, std::exception) override;

                            // XEventsSupplier
    virtual css::uno::Reference< css::container::XNameReplace > SAL_CALL getEvents()
                                throw (css::uno::RuntimeException, std::exception) override;

                            // XPropertySet override due to sheet properties
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScTableSheetObj* getImplementation(const css::uno::Reference< css::uno::XInterface>& rObj);

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScTableColumnObj : public ScCellRangeObj,
                         public css::container::XNamed
{
private:
    const SfxItemPropertySet*       pColPropSet;

protected:
    virtual const SfxItemPropertyMap& GetItemPropertyMap() override;
    virtual void GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                css::uno::Any& )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                                const css::uno::Any& aValue )
                                throw(css::lang::IllegalArgumentException,
                                      css::uno::RuntimeException,
                                      std::exception) override;

public:
                            ScTableColumnObj(ScDocShell* pDocSh, SCCOL nCol, SCTAB nTab);
    virtual                 ~ScTableColumnObj();

    virtual css::uno::Any SAL_CALL queryInterface(
                                const css::uno::Type & rType )
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   acquire() throw() override;
    virtual void SAL_CALL   release() throw() override;

                            // XNamed
    virtual OUString SAL_CALL getName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XPropertySet override due to column properties
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScTableRowObj : public ScCellRangeObj
{
private:
    const SfxItemPropertySet*       pRowPropSet;

protected:
    virtual const SfxItemPropertyMap& GetItemPropertyMap() override;
    virtual void GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                css::uno::Any& )
                                throw(css::uno::RuntimeException,
                                      std::exception) override;
    virtual void            SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                                const css::uno::Any& aValue )
                                throw(css::lang::IllegalArgumentException,
                                      css::uno::RuntimeException,
                                      std::exception) override;

public:
                            ScTableRowObj(ScDocShell* pDocSh, SCROW nRow, SCTAB nTab);
    virtual                 ~ScTableRowObj();

                            // XPropertySet override due to row properties
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScCellsObj : public cppu::WeakImplHelper<
                            css::container::XEnumerationAccess,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScRangeList             aRanges;

public:
                            ScCellsObj(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual                 ~ScCellsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScCellsEnumeration : public cppu::WeakImplHelper<
                                css::container::XEnumeration,
                                css::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScRangeList             aRanges;
    ScAddress               aPos;
    ScMarkData*             pMark;
    bool                    bAtEnd;

private:
    void                    Advance_Impl();
    void                    CheckPos_Impl();

public:
                            ScCellsEnumeration(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual                 ~ScCellsEnumeration();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL nextElement()
                                throw(css::container::NoSuchElementException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScCellFormatsObj : public cppu::WeakImplHelper<
                            css::container::XIndexAccess,
                            css::container::XEnumerationAccess,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScRange                 aTotalRange;

private:
    ScCellRangeObj*         GetObjectByIndex_Impl(long nIndex) const;

public:
                            ScCellFormatsObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScCellFormatsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScCellFormatsEnumeration : public cppu::WeakImplHelper<
                                    css::container::XEnumeration,
                                    css::lang::XServiceInfo >,
                                 public SfxListener
{
private:
    ScDocShell*             pDocShell;
    SCTAB                   nTab;
    ScAttrRectIterator*     pIter;
    ScRange                 aNext;
    bool                    bAtEnd;
    bool                    bDirty;

private:
    void                    Advance_Impl();
    ScCellRangeObj*         NextObject_Impl();

public:
                            ScCellFormatsEnumeration(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScCellFormatsEnumeration();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL nextElement()
                                throw(css::container::NoSuchElementException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

typedef std::vector< ScRangeList > ScMyRangeLists;

class ScUniqueCellFormatsObj : public cppu::WeakImplHelper<
                            css::container::XIndexAccess,
                            css::container::XEnumerationAccess,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*                     pDocShell;
    ScRange                         aTotalRange;
    ScMyRangeLists                  aRangeLists;

private:
    void                            GetObjects_Impl();

public:
                            ScUniqueCellFormatsObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScUniqueCellFormatsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                    css::lang::WrappedTargetException,
                                    css::uno::RuntimeException, std::exception) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

class ScUniqueCellFormatsEnumeration : public cppu::WeakImplHelper<
                                    css::container::XEnumeration,
                                    css::lang::XServiceInfo >,
                                 public SfxListener
{
private:
    ScMyRangeLists                  aRangeLists;
    ScDocShell*                     pDocShell;
    sal_Int32                       nCurrentPosition;

public:
                            ScUniqueCellFormatsEnumeration(ScDocShell* pDocShell, const ScMyRangeLists& rRangeLists);
    virtual                 ~ScUniqueCellFormatsEnumeration();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL nextElement()
                                throw(css::container::NoSuchElementException,
                                        css::lang::WrappedTargetException,
                                        css::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
