/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_CELLSUNO_HXX
#define SC_CELLSUNO_HXX


#include "rangelst.hxx"			// ScRangeList

#include <bf_svtools/lstner.hxx>

#include <bf_svtools/itemprop.hxx>

#include <com/sun/star/table/XTableChartsSupplier.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
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
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#include <com/sun/star/sheet/XScenariosSupplier.hpp>
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#include <com/sun/star/sheet/XSheetAuditing.hpp>
#include <com/sun/star/sheet/XSheetOutline.hpp>
#include <com/sun/star/util/XProtectable.hpp>
#include <com/sun/star/sheet/XScenario.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/document/XActionLockable.hpp>

#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>

#include <vector>
namespace binfilter {

class ScDocShell;
class ScMarkData;
class SchMemChart;
class ScPrintRangeSaver;
class ScAttrRectIterator;
class ScCellRangeObj;
class SvxUnoText;
class ScLinkListener;
class ScPatternAttr;
class SvxBorderLine;
class SvxBoxItem;
class SvxBoxInfoItem;


typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyChangeListener >* XPropertyChangeListenerPtr;
SV_DECL_PTRARR_DEL( XPropertyChangeListenerArr_Impl, XPropertyChangeListenerPtr, 4, 4 )//STRIP008 ;

class ScNamedEntry;
typedef ScNamedEntry* ScNamedEntryPtr;
SV_DECL_PTRARR_DEL( ScNamedEntryArr_Impl, ScNamedEntryPtr, 4, 4 )//STRIP008 ;


//	ScCellRangesBase - Basisklasse fuer ScCellRangesObj (mit Index-Access)
//									und ScCellRangeObj (ohne Index-Access)

//	XServiceInfo ist in den Ableitungen implementiert

class ScHelperFunctions
{
public:
    static const SvxBorderLine* GetBorderLine( SvxBorderLine& rLine, const ::com::sun::star::table::BorderLine& rStruct );
    static void FillBoxItems( SvxBoxItem& rOuter, SvxBoxInfoItem& rInner, const ::com::sun::star::table::TableBorder& rBorder );
    static void FillBorderLine( ::com::sun::star::table::BorderLine& rStruct, const SvxBorderLine* pLine );
    static void FillTableBorder( ::com::sun::star::table::TableBorder& rBorder,
                            const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner );
    static void ApplyBorder( ScDocShell* pDocShell, const ScRangeList& rRanges,
                        const SvxBoxItem& rOuter, const SvxBoxInfoItem& rInner );
};

class ScCellRangesBase : public ::com::sun::star::beans::XPropertySet,
                         public ::com::sun::star::beans::XMultiPropertySet,
                         public ::com::sun::star::beans::XPropertyState,
                         public ::com::sun::star::sheet::XSheetOperation,
                         public ::com::sun::star::chart::XChartDataArray,
                         public ::com::sun::star::util::XIndent,
                         public ::com::sun::star::sheet::XCellRangesQuery,
                         public ::com::sun::star::sheet::XFormulaQuery,
                         public ::com::sun::star::util::XReplaceable,
                         public ::com::sun::star::lang::XServiceInfo,
                         public ::com::sun::star::lang::XUnoTunnel,
                         public ::com::sun::star::lang::XTypeProvider,
                         public cppu::OWeakObject,
                         public SfxListener
{
    friend class ScTabViewObj;		// fuer select()
    friend class ScTableSheetObj;	// fuer createCursorByRange()

private:
    SfxItemPropertySet		aPropSet;
    ScDocShell*				pDocShell;
    ScLinkListener*			pValueListener;
    ScPatternAttr*			pCurrentFlat;
    ScPatternAttr*			pCurrentDeep;
    SfxItemSet*				pCurrentDataSet;
    ScMarkData*				pMarkData;
    ScRangeList				aRanges;
    BOOL					bChartColAsHdr;
    BOOL					bChartRowAsHdr;
    BOOL					bCursorOnly;
    BOOL					bValueChangePosted;
    XPropertyChangeListenerArr_Impl aValueListeners;

    DECL_LINK( ValueListenerHdl, SfxHint* );
    DECL_LINK( ValueChanged, ::com::sun::star::beans::PropertyChangeEvent* );

private:
    void			PaintRanges_Impl( USHORT nPart );
    ScRangeListRef	GetLimitedChartRanges_Impl( long nDataColumns, long nDataRows ) const;
    void			ForceChartListener_Impl();
    SchMemChart*	CreateMemChart_Impl() const;

    const ScPatternAttr*	GetCurrentAttrsFlat();
    const ScPatternAttr*	GetCurrentAttrsDeep();
    SfxItemSet*				GetCurrentDataSet();
    const ScMarkData*		GetMarkData();
    void					ForgetCurrentAttrs();

    ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges>
                            QueryDifferences_Impl(const ::com::sun::star::table::CellAddress& aCompare,
                                                    BOOL bColumnDiff);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
                            Find_Impl(const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::util::XSearchDescriptor>& xDesc,
                                    const ScAddress* pLastPos);

protected:
    // GetItemPropertyMap for derived classes must contain all entries, including base class
    virtual const SfxItemPropertyMap* GetItemPropertyMap();
    virtual ::com::sun::star::beans::PropertyState GetOnePropertyState(
                                USHORT nItemWhich, const SfxItemPropertyMap* pMap );
    virtual void GetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                ::com::sun::star::uno::Any& )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void			SetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                                const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

public:
                            ScCellRangesBase(ScDocShell* pDocSh, const ScRange& rR);
                            ScCellRangesBase(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual					~ScCellRangesBase();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	acquire() throw();
    virtual void SAL_CALL	release() throw();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual void			RefChanged();

                            // aus Ableitungen, aber auch per getImplementation
    ScDocShell*				GetDocShell() const		{ return pDocShell; }
    ScDocument*				GetDocument() const;
    const ScRangeList&		GetRangeList() const	{ return aRanges; }
    void					AddRange(const ScRange& rRange, const sal_Bool bMergeRanges);

                            // per Service erzeugtes Objekt zum Leben erwecken:
    void					InitInsertRange(ScDocShell* pDocSh, const ScRange& rR);

    void					SetNewRange(const ScRange& rNew);	// fuer Cursor
    void					SetNewRanges(const ScRangeList& rNew);

    BOOL					IsCursorOnly() const			{ return bCursorOnly; }

                            // XSheetOperation
    virtual double SAL_CALL	computeFunction( ::com::sun::star::sheet::GeneralFunction nFunction )
                                throw(::com::sun::star::uno::Exception,
                                        ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	clearContents( sal_Int32 nContentFlags )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPropertyValue( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XMultiPropertySet
    virtual void SAL_CALL	setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames,
                                    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues )
                                throw (::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL
                            getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
                                throw (::com::sun::star::uno::RuntimeException);

                            // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState(
                                    const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL
                            getPropertyStates( const ::com::sun::star::uno::Sequence<
                                        ::rtl::OUString >& aPropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPropertyToDefault( const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault(
                                    const ::rtl::OUString& aPropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XIndent
    virtual void SAL_CALL	decrementIndent() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	incrementIndent() throw(::com::sun::star::uno::RuntimeException);

                            // XChartData
    virtual void SAL_CALL	addChartDataChangeEventListener(
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeChartDataChangeEventListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL	getNotANumber() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XChartDataArray
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > > SAL_CALL
                            getData() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setData( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::uno::Sequence< double > >& aData )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getRowDescriptions()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRowDescriptions( const ::com::sun::star::uno::Sequence<
                                ::rtl::OUString >& aRowDescriptions )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getColumnDescriptions()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setColumnDescriptions( const ::com::sun::star::uno::Sequence<
                                ::rtl::OUString >& aColumnDescriptions )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XCellRangesQuery
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryVisibleCells() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryEmptyCells() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryContentCells( sal_Int16 nContentFlags )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryFormulaCells( sal_Int32 nResultFlags )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryColumnDifferences(
                                const ::com::sun::star::table::CellAddress& aCompare )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryRowDifferences(
                                const ::com::sun::star::table::CellAddress& aCompare )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryIntersection(
                                const ::com::sun::star::table::CellRangeAddress& aRange )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XFormulaQuery
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryDependents( sal_Bool bRecursive )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellRanges > SAL_CALL
                            queryPrecedents( sal_Bool bRecursive )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XSearchable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XSearchDescriptor > SAL_CALL
                            createSearchDescriptor()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
                            findAll( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XSearchDescriptor >& xDesc )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                            findFirst( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XSearchDescriptor >& xDesc )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                            findNext( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::uno::XInterface >& xStartAt,
                                      const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::util::XSearchDescriptor >& xDesc )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XReplaceable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XReplaceDescriptor > SAL_CALL
                            createReplaceDescriptor() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL replaceAll( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::util::XSearchDescriptor >& xDesc )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScCellRangesBase* getImplementation( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::uno::XInterface> xObj );

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScCellRangesObj : public ScCellRangesBase,
                        public ::com::sun::star::sheet::XSheetCellRangeContainer,
                        public ::com::sun::star::container::XNameContainer,
                        public ::com::sun::star::container::XEnumerationAccess
{
private:
    ScNamedEntryArr_Impl	aNamedEntries;

    ScCellRangeObj*			GetObjectByIndex_Impl(USHORT nIndex) const;

public:
                            ScCellRangesObj(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual					~ScCellRangesObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	acquire() throw();
    virtual void SAL_CALL	release() throw();

    virtual void			RefChanged();

                            // XSheetCellRanges
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess > SAL_CALL
                            getCells() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRangeAddressesAsString()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress > SAL_CALL
                            getRangeAddresses() throw(::com::sun::star::uno::RuntimeException);

                            // XSheetCellRangeContainer
    virtual void SAL_CALL	addRangeAddress( const ::com::sun::star::table::CellRangeAddress& rRange,
                                        sal_Bool bMergeRanges )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeRangeAddress( const ::com::sun::star::table::CellRangeAddress& rRange )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addRangeAddresses( const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::table::CellRangeAddress >& rRanges,
                                        sal_Bool bMergeRanges )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeRangeAddresses( const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::table::CellRangeAddress >& rRanges )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XNameContainer
    virtual void SAL_CALL	insertByName( const ::rtl::OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::container::ElementExistException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeByName( const ::rtl::OUString& Name )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XNameReplace
    virtual void SAL_CALL	replaceByName( const ::rtl::OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XNameAccess
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScCellRangeObj : public ScCellRangesBase,
                       public ::com::sun::star::sheet::XCellRangeAddressable,
                       public ::com::sun::star::sheet::XSheetCellRange,
                       public ::com::sun::star::sheet::XArrayFormulaRange,
                       public ::com::sun::star::sheet::XCellRangeData,
                       public ::com::sun::star::sheet::XCellRangeFormula,
                       public ::com::sun::star::sheet::XMultipleOperation,
                       public ::com::sun::star::util::XMergeable,
                       public ::com::sun::star::sheet::XCellSeries,
                       public ::com::sun::star::table::XAutoFormattable,
                       public ::com::sun::star::util::XSortable,
                       public ::com::sun::star::sheet::XSheetFilterableEx,
                       public ::com::sun::star::sheet::XSubTotalCalculatable,
                       public ::com::sun::star::util::XImportable,
                       public ::com::sun::star::sheet::XCellFormatRangesSupplier,
                       public ::com::sun::star::sheet::XUniqueCellFormatRangesSupplier,
                       public ::com::sun::star::table::XColumnRowRange
{
private:
    SfxItemPropertySet		aRangePropSet;
    ScRange					aRange;

protected:
    const ScRange&			GetRange() const	{ return aRange; }
    virtual const SfxItemPropertyMap* GetItemPropertyMap();
    virtual void GetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                ::com::sun::star::uno::Any& )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void			SetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                                const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell >
                            GetCellByPosition_Impl( sal_Int32 nColumn, sal_Int32 nRow )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);


public:
                            ScCellRangeObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual					~ScCellRangeObj();

                            // uses ObjectShell from document, if set (returns NULL otherwise)
    static ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange>
                            CreateRangeFromDoc( ScDocument* pDoc, const ScRange& rR );

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	acquire() throw();
    virtual void SAL_CALL	release() throw();

    virtual void			RefChanged();

    // XCellRange ist Basisklasse von XSheetCellRange und XSheetOperation
//	operator XCellRangeRef() const	{ return (XSheetCellRange*)this; }

                            // XCellRangeAddressable
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getRangeAddress()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XSheetCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > SAL_CALL
                            getSpreadsheet() throw(::com::sun::star::uno::RuntimeException);

                            // XArrayFormulaRange
    virtual ::rtl::OUString SAL_CALL getArrayFormula() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL 	setArrayFormula( const ::rtl::OUString& aFormula )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XCellRangeData
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence<
                            ::com::sun::star::uno::Any > > SAL_CALL getDataArray()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setDataArray( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::uno::Any > >& aArray )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XCellRangeFormula
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence<
                            ::rtl::OUString > > SAL_CALL getFormulaArray()
                                throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setFormulaArray( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::uno::Sequence< ::rtl::OUString > >& aArray )
                                throw (::com::sun::star::uno::RuntimeException);

                            // XMultipleOperation
    virtual void SAL_CALL	setTableOperation(
                                const ::com::sun::star::table::CellRangeAddress& aFormulaRange,
                                ::com::sun::star::sheet::TableOperationMode nMode,
                                const ::com::sun::star::table::CellAddress& aColumnCell,
                                const ::com::sun::star::table::CellAddress& aRowCell )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XMergeable
    virtual void SAL_CALL	merge( sal_Bool bMerge ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getIsMerged() throw(::com::sun::star::uno::RuntimeException);

                            // XCellSeries
    virtual void SAL_CALL	fillSeries( ::com::sun::star::sheet::FillDirection nFillDirection,
                                ::com::sun::star::sheet::FillMode nFillMode,
                                ::com::sun::star::sheet::FillDateMode nFillDateMode,
                                double fStep, double fEndValue )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	fillAuto( ::com::sun::star::sheet::FillDirection nFillDirection,
                                sal_Int32 nSourceCount )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XAutoFormattable
    virtual void SAL_CALL	autoFormat( const ::rtl::OUString& aName )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XSortable
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            createSortDescriptor() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	sort( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& xDescriptor )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XSheetFilterableEx
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetFilterDescriptor > SAL_CALL
                            createFilterDescriptorByObject( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XSheetFilterable >& xObject )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XSheetFilterable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetFilterDescriptor > SAL_CALL
                            createFilterDescriptor( sal_Bool bEmpty )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	filter( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XSheetFilterDescriptor >& xDescriptor )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XSubTotalCalculatable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSubTotalDescriptor > SAL_CALL
                            createSubTotalDescriptor( sal_Bool bEmpty )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	applySubTotals( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XSubTotalDescriptor >& xDescriptor,
                                sal_Bool bReplace ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeSubTotals() throw(::com::sun::star::uno::RuntimeException);

                            // XImportable
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            createImportDescriptor( sal_Bool bEmpty )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	doImport( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& aDescriptor )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XCellFormatRangesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
                            getCellFormatRanges() throw(::com::sun::star::uno::RuntimeException);

                            // XUniqueCellFormatRangesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
                            getUniqueCellFormatRanges() throw(::com::sun::star::uno::RuntimeException);

                            // XColumnRowRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableColumns > SAL_CALL
                            getColumns() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableRows > SAL_CALL
                            getRows() throw(::com::sun::star::uno::RuntimeException);

                            // XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom )
                                    throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByName( const ::rtl::OUString& aRange )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet ueberladen wegen Range-Properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);
};


//!	really derive cell from range?

class ScCellObj : public ScCellRangeObj,
                  public ::com::sun::star::text::XText,
                  public ::com::sun::star::container::XEnumerationAccess,
                  public ::com::sun::star::table::XCell,
                  public ::com::sun::star::sheet::XCellAddressable,
                  public ::com::sun::star::sheet::XSheetAnnotationAnchor,
                  public ::com::sun::star::text::XTextFieldsSupplier,
                    public ::com::sun::star::document::XActionLockable
{
private:
    SvxUnoText*				pUnoText;
    SfxItemPropertySet		aCellPropSet;
    ScAddress				aCellPos;
    sal_Int16				nActionLockCount;

private:
    String		GetInputString_Impl(BOOL bEnglish) const;
    String		GetOutputString_Impl() const;
    void		SetString_Impl(const String& rString, BOOL bInterpret, BOOL bEnglish);
    double		GetValue_Impl() const;
    void		SetValue_Impl(double fValue);
    ::com::sun::star::table::CellContentType GetResultType_Impl();

protected:
    virtual const SfxItemPropertyMap* GetItemPropertyMap();
    virtual void GetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                ::com::sun::star::uno::Any& )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void			SetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                                const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

public:
    static const SfxItemPropertyMap* GetEditPropertyMap();

                            ScCellObj(ScDocShell* pDocSh, const ScAddress& rP);
    virtual					~ScCellObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	acquire() throw();
    virtual void SAL_CALL	release() throw();

    virtual void			RefChanged();

    SvxUnoText&				GetUnoText();

                            // XML import needs to set results at formula cells,
                            // not meant for any other purpose.
    void					SetFormulaResultString( const ::rtl::OUString& rResult );
    void					SetFormulaResultDouble( double fResult );
    const ScAddress&		GetPosition() const { return aCellPos; }

                            // XText
    virtual void SAL_CALL	insertTextContent( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextRange >& xRange,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextContent >& xContent,
                                sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeTextContent( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::text::XTextContent >& xContent )
                                    throw(::com::sun::star::container::NoSuchElementException,
                                            ::com::sun::star::uno::RuntimeException);

                            // XSimpleText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursorByRange( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& aTextPosition )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	insertString( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        const ::rtl::OUString& aString, sal_Bool bAbsorb )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	insertControlCharacter( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setString( const ::rtl::OUString& aString )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XCell
    virtual ::rtl::OUString SAL_CALL getFormula() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setFormula( const ::rtl::OUString& aFormula )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getValue() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setValue( double nValue ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellContentType SAL_CALL getType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getError() throw(::com::sun::star::uno::RuntimeException);

                            // XCellAddressable
    virtual ::com::sun::star::table::CellAddress SAL_CALL getCellAddress()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XSheetAnnotationAnchor
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetAnnotation > SAL_CALL
                            getAnnotation() throw(::com::sun::star::uno::RuntimeException);

                            // XTextFieldsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess > SAL_CALL
                            getTextFields() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL
                            getTextFieldMasters() throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet ueberladen wegen Zell-Properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XActionLockable
    virtual sal_Bool SAL_CALL isActionLocked() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addActionLock() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeActionLock() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setActionLocks( sal_Int16 nLock )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL resetActionLocks() throw(::com::sun::star::uno::RuntimeException);

};


class ScTableSheetObj : public ScCellRangeObj,
                        public ::com::sun::star::sheet::XSpreadsheet,
                        public ::com::sun::star::container::XNamed,
                        public ::com::sun::star::sheet::XSheetPageBreak,
                        public ::com::sun::star::sheet::XCellRangeMovement,
                        public ::com::sun::star::table::XTableChartsSupplier,
                        public ::com::sun::star::sheet::XDataPilotTablesSupplier,
                        public ::com::sun::star::sheet::XScenariosSupplier,
                        public ::com::sun::star::sheet::XSheetAnnotationsSupplier,
                        public ::com::sun::star::drawing::XDrawPageSupplier,
                        public ::com::sun::star::sheet::XPrintAreas,
                        public ::com::sun::star::sheet::XSheetLinkable,
                        public ::com::sun::star::sheet::XSheetAuditing,
                        public ::com::sun::star::sheet::XSheetOutline,
                        public ::com::sun::star::util::XProtectable,
                        public ::com::sun::star::sheet::XScenario
{
    friend class ScTableSheetsObj;		// fuer insertByName()

private:
    SfxItemPropertySet		aSheetPropSet;

    USHORT					GetTab_Impl() const;
    void					PrintAreaUndo_Impl( ScPrintRangeSaver* pOldRanges );

protected:
    virtual const SfxItemPropertyMap* GetItemPropertyMap();
    virtual void GetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                ::com::sun::star::uno::Any& )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void			SetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                                const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

public:
                            ScTableSheetObj(ScDocShell* pDocSh, USHORT nTab);
    virtual					~ScTableSheetObj();

    void					InitInsertSheet(ScDocShell* pDocSh, USHORT nTab);

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	acquire() throw();
    virtual void SAL_CALL	release() throw();

                            // XSpreadsheet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellCursor >
                            SAL_CALL createCursor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetCellCursor > SAL_CALL
                            createCursorByRange( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XSheetCellRange >& aRange )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XSheetCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > SAL_CALL
                            getSpreadsheet() throw(::com::sun::star::uno::RuntimeException);

                            // XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom )
                                    throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByName( const ::rtl::OUString& aRange )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XSheetPageBreak
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::TablePageBreakData > SAL_CALL
                            getColumnPageBreaks() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::TablePageBreakData > SAL_CALL
                            getRowPageBreaks() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeAllManualPageBreaks() throw(::com::sun::star::uno::RuntimeException);

                            // XCellRangeMovement
    virtual void SAL_CALL	insertCells( const ::com::sun::star::table::CellRangeAddress& aRange,
                                ::com::sun::star::sheet::CellInsertMode nMode )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	removeRange( const ::com::sun::star::table::CellRangeAddress& aRange,
                                ::com::sun::star::sheet::CellDeleteMode nMode )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	moveRange( const ::com::sun::star::table::CellAddress& aDestination,
                                const ::com::sun::star::table::CellRangeAddress& aSource )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	copyRange( const ::com::sun::star::table::CellAddress& aDestination,
                                const ::com::sun::star::table::CellRangeAddress& aSource )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XTableChartsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableCharts > SAL_CALL
                            getCharts() throw(::com::sun::star::uno::RuntimeException);

                            // XDataPilotTablesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XDataPilotTables > SAL_CALL
                            getDataPilotTables() throw(::com::sun::star::uno::RuntimeException);

                            // XScenariosSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XScenarios > SAL_CALL
                            getScenarios() throw(::com::sun::star::uno::RuntimeException);

                            // XSheetAnnotationsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSheetAnnotations > SAL_CALL
                            getAnnotations() throw(::com::sun::star::uno::RuntimeException);

                            // XDrawPageSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL
                            getDrawPage() throw(::com::sun::star::uno::RuntimeException);

                            // XPrintAreas
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress > SAL_CALL
                            getPrintAreas() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPrintAreas( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::table::CellRangeAddress >& aPrintAreas )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getPrintTitleColumns() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPrintTitleColumns( sal_Bool bPrintTitleColumns )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getTitleColumns()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setTitleColumns(
                                const ::com::sun::star::table::CellRangeAddress& aTitleColumns )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getPrintTitleRows() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPrintTitleRows( sal_Bool bPrintTitleRows )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getTitleRows()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setTitleRows(
                                const ::com::sun::star::table::CellRangeAddress& aTitleRows )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XSheetLinkable
    virtual ::com::sun::star::sheet::SheetLinkMode SAL_CALL getLinkMode()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setLinkMode( ::com::sun::star::sheet::SheetLinkMode nLinkMode )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLinkUrl() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setLinkUrl( const ::rtl::OUString& aLinkUrl )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLinkSheetName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setLinkSheetName( const ::rtl::OUString& aLinkSheetName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	link( const ::rtl::OUString& aUrl,
                                const ::rtl::OUString& aSheetName,
                                const ::rtl::OUString& aFilterName,
                                const ::rtl::OUString& aFilterOptions,
                                ::com::sun::star::sheet::SheetLinkMode nMode )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XSheetAuditing
    virtual sal_Bool SAL_CALL hideDependents( const ::com::sun::star::table::CellAddress& aPosition )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hidePrecedents( const ::com::sun::star::table::CellAddress& aPosition )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL showDependents( const ::com::sun::star::table::CellAddress& aPosition )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL showPrecedents( const ::com::sun::star::table::CellAddress& aPosition )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL showErrors( const ::com::sun::star::table::CellAddress& aPosition )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL showInvalid() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	clearArrows() throw(::com::sun::star::uno::RuntimeException);

                            // XSheetOutline
    virtual void SAL_CALL	group( const ::com::sun::star::table::CellRangeAddress& aRange,
                                ::com::sun::star::table::TableOrientation nOrientation )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	ungroup( const ::com::sun::star::table::CellRangeAddress& aRange,
                                ::com::sun::star::table::TableOrientation nOrientation )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	autoOutline( const ::com::sun::star::table::CellRangeAddress& aRange )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	clearOutline() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	hideDetail( const ::com::sun::star::table::CellRangeAddress& aRange )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	showDetail( const ::com::sun::star::table::CellRangeAddress& aRange )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	showLevel( sal_Int16 nLevel,
                                ::com::sun::star::table::TableOrientation nOrientation )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XProtectable
    virtual void SAL_CALL	protect( const ::rtl::OUString& aPassword )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	unprotect( const ::rtl::OUString& aPassword )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isProtected() throw(::com::sun::star::uno::RuntimeException);

                            // XScenario
    virtual sal_Bool SAL_CALL getIsScenario() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getScenarioComment() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setScenarioComment( const ::rtl::OUString& aScenarioComment )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	addRanges( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::table::CellRangeAddress >& aRanges )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	apply() throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet ueberladen wegen Sheet-Properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence<
                                    sal_Int8 >& aIdentifier )
                                throw(::com::sun::star::uno::RuntimeException);

    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScTableSheetObj* getImplementation( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::uno::XInterface> xObj );

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScTableColumnObj : public ScCellRangeObj,
                         public ::com::sun::star::container::XNamed
{
private:
    SfxItemPropertySet		aColPropSet;

protected:
    virtual const SfxItemPropertyMap* GetItemPropertyMap();
    virtual void GetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                ::com::sun::star::uno::Any& )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void			SetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                                const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

public:
                            ScTableColumnObj(ScDocShell* pDocSh, USHORT nCol, USHORT nTab);
    virtual					~ScTableColumnObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	acquire() throw();
    virtual void SAL_CALL	release() throw();

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet ueberladen wegen Spalten-Properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScTableRowObj : public ScCellRangeObj
{
private:
    SfxItemPropertySet		aRowPropSet;

protected:
    virtual const SfxItemPropertyMap* GetItemPropertyMap();
    virtual void GetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                ::com::sun::star::uno::Any& )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void			SetOnePropertyValue( const SfxItemPropertyMap* pMap,
                                                const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

public:
                            ScTableRowObj(ScDocShell* pDocSh, USHORT nRow, USHORT nTab);
    virtual					~ScTableRowObj();

                            // XPropertySet ueberladen wegen Zeilen-Properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScCellsObj : public cppu::WeakImplHelper2<
                            ::com::sun::star::container::XEnumerationAccess,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*				pDocShell;
    ScRangeList				aRanges;

public:
                            ScCellsObj(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual					~ScCellsObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScCellsEnumeration : public cppu::WeakImplHelper2<
                                ::com::sun::star::container::XEnumeration,
                                ::com::sun::star::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*				pDocShell;
    ScRangeList				aRanges;
    ScAddress				aPos;
    ScMarkData*				pMark;
    BOOL					bAtEnd;

private:
    void					Advance_Impl();
    void					CheckPos_Impl();

public:
                            ScCellsEnumeration(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual					~ScCellsEnumeration();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
                                throw(::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};

class ScCellFormatsObj : public cppu::WeakImplHelper3<
                            ::com::sun::star::container::XIndexAccess,
                            ::com::sun::star::container::XEnumerationAccess,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*				pDocShell;
    ScRange					aTotalRange;

private:
    ScCellRangeObj*			GetObjectByIndex_Impl(long nIndex) const;

public:
                            ScCellFormatsObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual					~ScCellFormatsObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScCellFormatsEnumeration : public cppu::WeakImplHelper2<
                                    ::com::sun::star::container::XEnumeration,
                                    ::com::sun::star::lang::XServiceInfo >,
                                 public SfxListener
{
private:
    ScDocShell*				pDocShell;
    USHORT					nTab;
    ScAttrRectIterator*		pIter;
    ScRange					aNext;
    BOOL					bAtEnd;
    BOOL					bDirty;

private:
    void					Advance_Impl();
    ScCellRangeObj*			NextObject_Impl();

public:
                            ScCellFormatsEnumeration(ScDocShell* pDocSh, const ScRange& rR);
    virtual					~ScCellFormatsEnumeration();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
                                throw(::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};

typedef ::std::vector< ScRangeList > ScMyRangeLists;

class ScUniqueCellFormatsObj : public cppu::WeakImplHelper3<
                            ::com::sun::star::container::XIndexAccess,
                            ::com::sun::star::container::XEnumerationAccess,
                            ::com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*						pDocShell;
    ScRange							aTotalRange;
    ScMyRangeLists					aRangeLists;

private:
    void							GetObjects_Impl();

public:
                            ScUniqueCellFormatsObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual					~ScUniqueCellFormatsObj();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(::com::sun::star::lang::IndexOutOfBoundsException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL
                            createEnumeration() throw(::com::sun::star::uno::RuntimeException);

                            // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};

class ScUniqueCellFormatsEnumeration : public cppu::WeakImplHelper2<
                                    ::com::sun::star::container::XEnumeration,
                                    ::com::sun::star::lang::XServiceInfo >,
                                 public SfxListener
{
private:
    ScMyRangeLists					aRangeLists;
    ScDocShell*						pDocShell;
    sal_Int32						nCurrentPosition;

public:
                            ScUniqueCellFormatsEnumeration(ScDocShell* pDocShell, const ScMyRangeLists& rRangeLists);
    virtual					~ScUniqueCellFormatsEnumeration();

    virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
                                throw(::com::sun::star::container::NoSuchElementException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
