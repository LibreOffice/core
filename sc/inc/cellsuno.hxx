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
#include <boost/scoped_ptr.hpp>

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
    Link<>  aLink;
public:
                    ScLinkListener(const Link<>& rL) : aLink(rL) {}
    virtual         ~ScLinkListener();
    virtual void Notify( const SfxHint& rHint ) SAL_OVERRIDE;
};

typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::util::XModifyListener > XModifyListenerRef;
typedef std::vector<XModifyListenerRef> XModifyListenerArr_Impl;

//  ScCellRangesBase - base class for ScCellRangesObj (with access by index)
//                                and ScCellRangeObj  (without access by index)

//  XServiceInfo is implemented in derived classes

class ScHelperFunctions
{
public:
    static const ::editeng::SvxBorderLine* GetBorderLine( ::editeng::SvxBorderLine& rLine, const com::sun::star::table::BorderLine& rStruct );
    static const ::editeng::SvxBorderLine* GetBorderLine( ::editeng::SvxBorderLine& rLine, const com::sun::star::table::BorderLine2& rStruct );
    static void FillBoxItems( SvxBoxItem& rOuter, SvxBoxInfoItem& rInner, const com::sun::star::table::TableBorder& rBorder );
    static void FillBoxItems( SvxBoxItem& rOuter, SvxBoxInfoItem& rInner, const com::sun::star::table::TableBorder2& rBorder );
    static void FillBorderLine( com::sun::star::table::BorderLine& rStruct, const ::editeng::SvxBorderLine* pLine );
    static void FillBorderLine( com::sun::star::table::BorderLine2& rStruct, const ::editeng::SvxBorderLine* pLine );
    static void AssignTableBorderToAny( com::sun::star::uno::Any& rAny,
            const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner, bool bInvalidateHorVerDist = false );
    static void AssignTableBorder2ToAny( com::sun::star::uno::Any& rAny,
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

class SC_DLLPUBLIC ScCellRangesBase : public com::sun::star::beans::XPropertySet,
                         public com::sun::star::beans::XMultiPropertySet,
                         public com::sun::star::beans::XPropertyState,
                         public com::sun::star::sheet::XSheetOperation,
                         public com::sun::star::chart::XChartDataArray,
                         public com::sun::star::util::XIndent,
                         public com::sun::star::sheet::XCellRangesQuery,
                         public com::sun::star::sheet::XFormulaQuery,
                         public com::sun::star::util::XReplaceable,
                         public com::sun::star::util::XModifyBroadcaster,
                         public com::sun::star::lang::XServiceInfo,
                         public com::sun::star::lang::XUnoTunnel,
                         public com::sun::star::lang::XTypeProvider,
                         public com::sun::star::beans::XTolerantMultiPropertySet,
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

    DECL_LINK( ValueListenerHdl, SfxHint* );

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

    com::sun::star::uno::Reference<com::sun::star::sheet::XSheetCellRanges>
                            QueryDifferences_Impl(const com::sun::star::table::CellAddress& aCompare,
                                                    bool bColumnDiff);
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface>
                            Find_Impl(const com::sun::star::uno::Reference<
                                        com::sun::star::util::XSearchDescriptor>& xDesc,
                                    const ScAddress* pLastPos);

protected:
    const ScMarkData*       GetMarkData();

    // GetItemPropertyMap for derived classes must contain all entries, including base class
    virtual const SfxItemPropertyMap& GetItemPropertyMap();
    ::com::sun::star::beans::PropertyState GetOnePropertyState(
                                sal_uInt16 nItemWhich, const SfxItemPropertySimpleEntry* pEntry );
    virtual void            GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                ::com::sun::star::uno::Any& )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception);
    virtual void            SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                                const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                      ::com::sun::star::uno::RuntimeException,
                                      std::exception);

public:
                            ScCellRangesBase(ScDocShell* pDocSh, const ScRange& rR);
                            ScCellRangesBase(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual                 ~ScCellRangesBase();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   acquire() throw() SAL_OVERRIDE;
    virtual void SAL_CALL   release() throw() SAL_OVERRIDE;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;
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
    virtual double SAL_CALL computeFunction( ::com::sun::star::sheet::GeneralFunction nFunction )
                                throw(::com::sun::star::uno::Exception,
                                      ::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   clearContents( sal_Int32 nContentFlags )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XMultiPropertySet
    virtual void SAL_CALL   setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames,
                                    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues )
                                throw (::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL
                            getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XTolerantMultiPropertySet
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::SetPropertyTolerantFailed > SAL_CALL
        setPropertyValuesTolerant( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames,
                                    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues )
                                    throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::GetPropertyTolerantResult > SAL_CALL
        getPropertyValuesTolerant( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames )
                                    throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::GetDirectPropertyTolerantResult > SAL_CALL
        getDirectPropertyValuesTolerant( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames )
                                    throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL
                            getPropertyStates( const ::com::sun::star::uno::Sequence<
                                        OUString >& aPropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setPropertyToDefault( const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault(
                                    const OUString& aPropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XIndent
    virtual void SAL_CALL   decrementIndent() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   incrementIndent() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XChartData
    virtual void SAL_CALL   addChartDataChangeEventListener(
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeChartDataChangeEventListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
                                throw (::com::sun::star::uno::RuntimeException,
                                       std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getNotANumber() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XChartDataArray
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > > SAL_CALL
                            getData()
                                    throw(::com::sun::star::uno::RuntimeException,
                                          std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setData( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::uno::Sequence< double > >& aData )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getRowDescriptions()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setRowDescriptions( const ::com::sun::star::uno::Sequence<
                                OUString >& aRowDescriptions )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getColumnDescriptions()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setColumnDescriptions( const ::com::sun::star::uno::Sequence<
                                OUString >& aColumnDescriptions )
                                    throw(::com::sun::star::uno::RuntimeException,
                                          std::exception) SAL_OVERRIDE;

                            // XCellRangesQuery
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryVisibleCells()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryEmptyCells()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryContentCells( sal_Int16 nContentFlags )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryFormulaCells( sal_Int32 nResultFlags )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryColumnDifferences(
                                const ::com::sun::star::table::CellAddress& aCompare )
                                    throw(::com::sun::star::uno::RuntimeException,
                                          std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryRowDifferences(
                                const ::com::sun::star::table::CellAddress& aCompare )
                                    throw(::com::sun::star::uno::RuntimeException,
                                          std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryIntersection(
                                const ::com::sun::star::table::CellRangeAddress& aRange )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XFormulaQuery
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryDependents( sal_Bool bRecursive )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryPrecedents( sal_Bool bRecursive )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;

                            // XSearchable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor > SAL_CALL
                            createSearchDescriptor()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
                            findAll( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XSearchDescriptor >& xDesc )
                                    throw(::com::sun::star::uno::RuntimeException,
                                          std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                            findFirst( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XSearchDescriptor >& xDesc )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                            findNext( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::uno::XInterface >& xStartAt,
                                      const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::util::XSearchDescriptor >& xDesc )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XReplaceable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XReplaceDescriptor > SAL_CALL
                            createReplaceDescriptor() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL replaceAll( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XSearchDescriptor >& xDesc )
                                    throw(::com::sun::star::uno::RuntimeException,
                                          std::exception) SAL_OVERRIDE;

                            // XModifyBroadcaster
    virtual void SAL_CALL   addModifyListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XModifyListener >& aListener )
                                throw (::com::sun::star::uno::RuntimeException,
                                       std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeModifyListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::util::XModifyListener >& aListener )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScCellRangesBase* getImplementation(const css::uno::Reference<css::uno::XInterface>& rObj);

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class SC_DLLPUBLIC ScCellRangesObj : public ScCellRangesBase,
                        public com::sun::star::sheet::XSheetCellRangeContainer,
                        public com::sun::star::container::XNameContainer,
                        public com::sun::star::container::XEnumerationAccess
{
private:
    struct Impl;
    ::boost::scoped_ptr<Impl> m_pImpl;

    ScCellRangeObj*         GetObjectByIndex_Impl(sal_Int32 nIndex) const;

public:
                            ScCellRangesObj(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual                 ~ScCellRangesObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   acquire() throw() SAL_OVERRIDE;
    virtual void SAL_CALL   release() throw() SAL_OVERRIDE;

    virtual void            RefChanged() SAL_OVERRIDE;

                            // XSheetCellRanges
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess > SAL_CALL
                            getCells() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getRangeAddressesAsString()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress > SAL_CALL
                            getRangeAddresses() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSheetCellRangeContainer
    virtual void SAL_CALL   addRangeAddress( const ::com::sun::star::table::CellRangeAddress& rRange,
                                        sal_Bool bMergeRanges )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeRangeAddress( const ::com::sun::star::table::CellRangeAddress& rRange )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addRangeAddresses( const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::table::CellRangeAddress >& rRanges,
                                        sal_Bool bMergeRanges )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeRangeAddresses( const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::table::CellRangeAddress >& rRanges )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XNameContainer
    virtual void SAL_CALL   insertByName( const OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw (::com::sun::star::lang::IllegalArgumentException,
                                           ::com::sun::star::container::ElementExistException,
                                           ::com::sun::star::lang::WrappedTargetException,
                                           ::com::sun::star::uno::RuntimeException,
                                           std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeByName( const OUString& Name )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XNameReplace
    virtual void SAL_CALL   replaceByName( const OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class SC_DLLPUBLIC ScCellRangeObj : public ScCellRangesBase,
                       public com::sun::star::sheet::XCellRangeAddressable,
                       public com::sun::star::sheet::XSheetCellRange,
                       public com::sun::star::sheet::XArrayFormulaRange,
                       public com::sun::star::sheet::XArrayFormulaTokens,
                       public com::sun::star::sheet::XCellRangeData,
                       public com::sun::star::sheet::XCellRangeFormula,
                       public com::sun::star::sheet::XMultipleOperation,
                       public com::sun::star::util::XMergeable,
                       public com::sun::star::sheet::XCellSeries,
                       public com::sun::star::table::XAutoFormattable,
                       public com::sun::star::util::XSortable,
                       public com::sun::star::sheet::XSheetFilterableEx,
                       public com::sun::star::sheet::XSubTotalCalculatable,
                       public com::sun::star::util::XImportable,
                       public com::sun::star::sheet::XCellFormatRangesSupplier,
                       public com::sun::star::sheet::XUniqueCellFormatRangesSupplier,
                       public com::sun::star::table::XColumnRowRange
{
private:
    const SfxItemPropertySet*       pRangePropSet;
    ScRange                 aRange;

protected:
    const ScRange&          GetRange() const    { return aRange; }
    virtual const SfxItemPropertyMap& GetItemPropertyMap() SAL_OVERRIDE;
    virtual void GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                ::com::sun::star::uno::Any& )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void            SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                                const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                      ::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;

    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >
                            GetCellByPosition_Impl( sal_Int32 nColumn, sal_Int32 nRow )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);

            void            SetArrayFormula_Impl( const OUString& rFormula,
                                const OUString& rFormulaNmsp,
                                const formula::FormulaGrammar::Grammar eGrammar )
                                    throw (css::uno::RuntimeException, std::exception);

public:
                            ScCellRangeObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScCellRangeObj();

                            // uses ObjectShell from document, if set (returns NULL otherwise)
    static com::sun::star::uno::Reference<com::sun::star::table::XCellRange>
                            CreateRangeFromDoc( ScDocument* pDoc, const ScRange& rR );

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   acquire() throw() SAL_OVERRIDE;
    virtual void SAL_CALL   release() throw() SAL_OVERRIDE;

    virtual void            RefChanged() SAL_OVERRIDE;

                            // XCellRangeAddressable
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getRangeAddress()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSheetCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > SAL_CALL
                            getSpreadsheet() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XArrayFormulaRange
    virtual OUString SAL_CALL getArrayFormula() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setArrayFormula( const OUString& aFormula )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XArrayFormulaTokens
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > SAL_CALL getArrayTokens()
                                throw (::com::sun::star::uno::RuntimeException,
                                       std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setArrayTokens( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::sheet::FormulaToken >& aTokens )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XCellRangeData
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence<
                            ::com::sun::star::uno::Any > > SAL_CALL getDataArray()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setDataArray( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::uno::Any > >& aArray )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XCellRangeFormula
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence<
                            OUString > > SAL_CALL getFormulaArray()
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setFormulaArray( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::uno::Sequence< OUString > >& aArray )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XMultipleOperation
    virtual void SAL_CALL   setTableOperation(
                                const ::com::sun::star::table::CellRangeAddress& aFormulaRange,
                                ::com::sun::star::sheet::TableOperationMode nMode,
                                const ::com::sun::star::table::CellAddress& aColumnCell,
                                const ::com::sun::star::table::CellAddress& aRowCell )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XMergeable
    virtual void SAL_CALL   merge( sal_Bool bMerge ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getIsMerged() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XCellSeries
    virtual void SAL_CALL   fillSeries( ::com::sun::star::sheet::FillDirection nFillDirection,
                                ::com::sun::star::sheet::FillMode nFillMode,
                                ::com::sun::star::sheet::FillDateMode nFillDateMode,
                                double fStep, double fEndValue )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   fillAuto( ::com::sun::star::sheet::FillDirection nFillDirection,
                                sal_Int32 nSourceCount )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XAutoFormattable
    virtual void SAL_CALL   autoFormat( const OUString& aName )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSortable
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            createSortDescriptor() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   sort( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& xDescriptor )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSheetFilterableEx
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetFilterDescriptor > SAL_CALL
                            createFilterDescriptorByObject( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XSheetFilterable >& xObject )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSheetFilterable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetFilterDescriptor > SAL_CALL
                            createFilterDescriptor( sal_Bool bEmpty )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   filter( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XSheetFilterDescriptor >& xDescriptor )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSubTotalCalculatable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSubTotalDescriptor > SAL_CALL
                            createSubTotalDescriptor( sal_Bool bEmpty )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   applySubTotals(const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XSubTotalDescriptor >& xDescriptor,
                                sal_Bool bReplace)
                                throw (::com::sun::star::uno::RuntimeException,
                                       std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeSubTotals() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XImportable
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            createImportDescriptor( sal_Bool bEmpty )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   doImport( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& aDescriptor )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XCellFormatRangesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
                            getCellFormatRanges() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XUniqueCellFormatRangesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
                            getUniqueCellFormatRanges() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XColumnRowRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableColumns > SAL_CALL
                            getColumns() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableRows > SAL_CALL
                            getRows() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom )
                                    throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByName( const OUString& aRange )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >
                            getCellRangeByName( const OUString& aRange,  const ScAddress::Details& rDetails )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet override due to Range-Properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

//! really derive cell from range?

class SC_DLLPUBLIC ScCellObj : public ScCellRangeObj,
                  public com::sun::star::text::XText,
                  public com::sun::star::container::XEnumerationAccess,
                  public com::sun::star::table::XCell2,
                  public com::sun::star::sheet::XFormulaTokens,
                  public com::sun::star::sheet::XCellAddressable,
                  public com::sun::star::sheet::XSheetAnnotationAnchor,
                  public com::sun::star::text::XTextFieldsSupplier,
                    public com::sun::star::document::XActionLockable
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
    com::sun::star::table::CellContentType GetResultType_Impl();

protected:
    virtual const SfxItemPropertyMap& GetItemPropertyMap() SAL_OVERRIDE;
    virtual void GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                ::com::sun::star::uno::Any& )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void            SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                                const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

public:
    static const SvxItemPropertySet* GetEditPropertySet();
    static const SfxItemPropertyMap& GetCellPropertyMap();

                            ScCellObj(ScDocShell* pDocSh, const ScAddress& rP);
    virtual                 ~ScCellObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   acquire() throw() SAL_OVERRIDE;
    virtual void SAL_CALL   release() throw() SAL_OVERRIDE;

    virtual void            RefChanged() SAL_OVERRIDE;

    SvxUnoText&             GetUnoText();

                            // XML import needs to set results at formula cells,
                            // not meant for any other purpose.
    const ScAddress&        GetPosition() const { return aCellPos; }

    void                    InputEnglishString( const OUString& rText );

                            // XText
    virtual void SAL_CALL   insertTextContent( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextRange >& xRange,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextContent >& xContent,
                                sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeTextContent( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::text::XTextContent >& xContent )
                                    throw(::com::sun::star::container::NoSuchElementException,
                                            ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSimpleText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursor() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursorByRange( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& aTextPosition )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   insertString( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        const OUString& aString, sal_Bool bAbsorb )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   insertControlCharacter( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setString( const OUString& aString )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XCell
    virtual OUString SAL_CALL getFormula() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setFormula( const OUString& aFormula )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setFormulaResult( double nValue )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setFormulaString( const OUString& aFormula )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual double SAL_CALL getValue() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setValue( double nValue ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::table::CellContentType SAL_CALL getType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getError() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XFormulaTokens
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::FormulaToken > SAL_CALL getTokens()
                                throw (::com::sun::star::uno::RuntimeException,
                                       std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setTokens( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::sheet::FormulaToken >& aTokens )
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XCellAddressable
    virtual ::com::sun::star::table::CellAddress SAL_CALL getCellAddress()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSheetAnnotationAnchor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetAnnotation > SAL_CALL
                            getAnnotation() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XTextFieldsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess > SAL_CALL
                            getTextFields() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL
                            getTextFieldMasters() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XPropertySet override due to cell properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addActionLock() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeActionLock() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setActionLocks( sal_Int16 nLock )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int16 SAL_CALL resetActionLocks() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class ScTableSheetObj : public ScCellRangeObj,
                        public com::sun::star::sheet::XSpreadsheet,
                        public com::sun::star::container::XNamed,
                        public com::sun::star::sheet::XSheetPageBreak,
                        public com::sun::star::sheet::XCellRangeMovement,
                        public com::sun::star::table::XTableChartsSupplier,
                        public com::sun::star::sheet::XDataPilotTablesSupplier,
                        public com::sun::star::sheet::XScenariosSupplier,
                        public com::sun::star::sheet::XSheetAnnotationsSupplier,
                        public com::sun::star::drawing::XDrawPageSupplier,
                        public com::sun::star::sheet::XPrintAreas,
                        public com::sun::star::sheet::XSheetLinkable,
                        public com::sun::star::sheet::XSheetAuditing,
                        public com::sun::star::sheet::XSheetOutline,
                        public com::sun::star::util::XProtectable,
                        public com::sun::star::sheet::XScenario,
                        public com::sun::star::sheet::XScenarioEnhanced,
                        public com::sun::star::sheet::XExternalSheetName,
                        public com::sun::star::document::XEventsSupplier
{
    friend class ScTableSheetsObj;      // for insertByName()

private:
    const SfxItemPropertySet*       pSheetPropSet;

    SCTAB                   GetTab_Impl() const;
    void                    PrintAreaUndo_Impl( ScPrintRangeSaver* pOldRanges );

protected:
    virtual const SfxItemPropertyMap& GetItemPropertyMap() SAL_OVERRIDE;
    virtual void GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                ::com::sun::star::uno::Any& )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void            SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                                const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                      ::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;

public:
                            ScTableSheetObj(ScDocShell* pDocSh, SCTAB nTab);
    virtual                 ~ScTableSheetObj();

    void                    InitInsertSheet(ScDocShell* pDocSh, SCTAB nTab);

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   acquire() throw() SAL_OVERRIDE;
    virtual void SAL_CALL   release() throw() SAL_OVERRIDE;

                            // XSpreadsheet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellCursor >
                            SAL_CALL createCursor() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellCursor > SAL_CALL
                            createCursorByRange( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XSheetCellRange >& aRange )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSheetCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > SAL_CALL
                            getSpreadsheet() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom )
                                    throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    using ScCellRangeObj::getCellRangeByName;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByName( const OUString& aRange )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XNamed
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSheetPageBreak
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::TablePageBreakData > SAL_CALL
                            getColumnPageBreaks() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::TablePageBreakData > SAL_CALL
                            getRowPageBreaks() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeAllManualPageBreaks() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XCellRangeMovement
    virtual void SAL_CALL   insertCells( const ::com::sun::star::table::CellRangeAddress& aRange,
                                ::com::sun::star::sheet::CellInsertMode nMode )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   removeRange( const ::com::sun::star::table::CellRangeAddress& aRange,
                                ::com::sun::star::sheet::CellDeleteMode nMode )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   moveRange( const ::com::sun::star::table::CellAddress& aDestination,
                                const ::com::sun::star::table::CellRangeAddress& aSource )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   copyRange( const ::com::sun::star::table::CellAddress& aDestination,
                                const ::com::sun::star::table::CellRangeAddress& aSource )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XTableChartsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableCharts > SAL_CALL
                            getCharts() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XDataPilotTablesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotTables > SAL_CALL
                            getDataPilotTables() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XScenariosSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XScenarios > SAL_CALL
                            getScenarios() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSheetAnnotationsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetAnnotations > SAL_CALL
                            getAnnotations() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XDrawPageSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL
                            getDrawPage() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XPrintAreas
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress > SAL_CALL
                            getPrintAreas() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setPrintAreas( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::table::CellRangeAddress >& aPrintAreas )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getPrintTitleColumns() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setPrintTitleColumns( sal_Bool bPrintTitleColumns )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getTitleColumns()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setTitleColumns(
                                const ::com::sun::star::table::CellRangeAddress& aTitleColumns )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL getPrintTitleRows() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setPrintTitleRows( sal_Bool bPrintTitleRows )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getTitleRows()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setTitleRows(
                                const ::com::sun::star::table::CellRangeAddress& aTitleRows )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSheetLinkable
    virtual ::com::sun::star::sheet::SheetLinkMode SAL_CALL getLinkMode()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setLinkMode( ::com::sun::star::sheet::SheetLinkMode nLinkMode )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getLinkUrl() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setLinkUrl( const OUString& aLinkUrl )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getLinkSheetName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setLinkSheetName( const OUString& aLinkSheetName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   link( const OUString& aUrl,
                                const OUString& aSheetName,
                                const OUString& aFilterName,
                                const OUString& aFilterOptions,
                                ::com::sun::star::sheet::SheetLinkMode nMode )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSheetAuditing
    virtual sal_Bool SAL_CALL hideDependents( const ::com::sun::star::table::CellAddress& aPosition )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hidePrecedents( const ::com::sun::star::table::CellAddress& aPosition )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL showDependents( const ::com::sun::star::table::CellAddress& aPosition )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL showPrecedents( const ::com::sun::star::table::CellAddress& aPosition )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL showErrors( const ::com::sun::star::table::CellAddress& aPosition )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL showInvalid() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   clearArrows() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XSheetOutline
    virtual void SAL_CALL   group( const ::com::sun::star::table::CellRangeAddress& aRange,
                                ::com::sun::star::table::TableOrientation nOrientation )
                                    throw (::com::sun::star::uno::RuntimeException,
                                           std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   ungroup( const ::com::sun::star::table::CellRangeAddress& aRange,
                                ::com::sun::star::table::TableOrientation nOrientation )
                                    throw (::com::sun::star::uno::RuntimeException,
                                           std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   autoOutline( const ::com::sun::star::table::CellRangeAddress& aRange )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   clearOutline()
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   hideDetail( const ::com::sun::star::table::CellRangeAddress& aRange )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   showDetail( const ::com::sun::star::table::CellRangeAddress& aRange )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   showLevel( sal_Int16 nLevel,
                                ::com::sun::star::table::TableOrientation nOrientation )
                                    throw(::com::sun::star::uno::RuntimeException,
                                          std::exception) SAL_OVERRIDE;

                            // XProtectable
    virtual void SAL_CALL   protect( const OUString& aPassword )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   unprotect( const OUString& aPassword )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL isProtected() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XScenario
    virtual sal_Bool SAL_CALL getIsScenario() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getScenarioComment() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setScenarioComment( const OUString& aScenarioComment )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   addRanges( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::table::CellRangeAddress >& aRanges )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   apply() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
                            // XScenarioEnhanced
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress > SAL_CALL
                            getRanges(  )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XExternalSheetName
    virtual void SAL_CALL   setExternalName( const OUString& aUrl, const OUString& aSheetName )
                                throw (::com::sun::star::container::ElementExistException,
                                       ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XEventsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL getEvents()
                                throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XPropertySet override due to sheet properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScTableSheetObj* getImplementation(const css::uno::Reference< com::sun::star::uno::XInterface>& rObj);

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class ScTableColumnObj : public ScCellRangeObj,
                         public com::sun::star::container::XNamed
{
private:
    const SfxItemPropertySet*       pColPropSet;

protected:
    virtual const SfxItemPropertyMap& GetItemPropertyMap() SAL_OVERRIDE;
    virtual void GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                ::com::sun::star::uno::Any& )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void            SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                                const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                      ::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;

public:
                            ScTableColumnObj(ScDocShell* pDocSh, SCCOL nCol, SCTAB nTab);
    virtual                 ~ScTableColumnObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   acquire() throw() SAL_OVERRIDE;
    virtual void SAL_CALL   release() throw() SAL_OVERRIDE;

                            // XNamed
    virtual OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL   setName( const OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XPropertySet override due to column properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class ScTableRowObj : public ScCellRangeObj
{
private:
    const SfxItemPropertySet*       pRowPropSet;

protected:
    virtual const SfxItemPropertyMap& GetItemPropertyMap() SAL_OVERRIDE;
    virtual void GetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                ::com::sun::star::uno::Any& )
                                throw(::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;
    virtual void            SetOnePropertyValue( const SfxItemPropertySimpleEntry* pEntry,
                                                const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                      ::com::sun::star::uno::RuntimeException,
                                      std::exception) SAL_OVERRIDE;

public:
                            ScTableRowObj(ScDocShell* pDocSh, SCROW nRow, SCTAB nTab);
    virtual                 ~ScTableRowObj();

                            // XPropertySet override due to row properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class ScCellsObj : public cppu::WeakImplHelper<
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScRangeList             aRanges;

public:
                            ScCellsObj(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual                 ~ScCellsObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class ScCellsEnumeration : public cppu::WeakImplHelper<
                                com::sun::star::container::XEnumeration,
                                com::sun::star::lang::XServiceInfo >,
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
                                throw(::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class ScCellFormatsObj : public cppu::WeakImplHelper<
                            com::sun::star::container::XIndexAccess,
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::lang::XServiceInfo >,
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class ScCellFormatsEnumeration : public cppu::WeakImplHelper<
                                    com::sun::star::container::XEnumeration,
                                    com::sun::star::lang::XServiceInfo >,
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
                                throw(::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

typedef std::vector< ScRangeList > ScMyRangeLists;

class ScUniqueCellFormatsObj : public cppu::WeakImplHelper<
                            com::sun::star::container::XIndexAccess,
                            com::sun::star::container::XEnumerationAccess,
                            com::sun::star::lang::XServiceInfo >,
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

class ScUniqueCellFormatsEnumeration : public cppu::WeakImplHelper<
                                    com::sun::star::container::XEnumeration,
                                    com::sun::star::lang::XServiceInfo >,
                                 public SfxListener
{
private:
    ScMyRangeLists                  aRangeLists;
    ScDocShell*                     pDocShell;
    sal_Int32                       nCurrentPosition;

public:
                            ScUniqueCellFormatsEnumeration(ScDocShell* pDocShell, const ScMyRangeLists& rRangeLists);
    virtual                 ~ScUniqueCellFormatsEnumeration();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
                                throw(::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
