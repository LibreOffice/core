/*************************************************************************
 *
 *  $RCSfile: cellsuno.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: sab $ $Date: 2001-04-06 08:38:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_CELLSUNO_HXX
#define SC_CELLSUNO_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"           // ScRange, ScAddress
#endif

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"         // ScRangeList
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif

#ifndef _COM_SUN_STAR_TABLE_XTABLECHARTSSUPPLIER_HPP_
#include <com/sun/star/table/XTableChartsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XCHARTDATAARRAY_HPP_
#include <com/sun/star/chart/XChartDataArray.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELDSSUPPLIER_HPP_
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETCELLRANGE_HPP_
#include <com/sun/star/sheet/XSheetCellRange.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XFORMULAQUERY_HPP_
#include <com/sun/star/sheet/XFormulaQuery.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEADDRESSABLE_HPP_
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETOPERATION_HPP_
#include <com/sun/star/sheet/XSheetOperation.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETCELLRANGES_HPP_
#include <com/sun/star/sheet/XSheetCellRanges.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLFORMATRANGESSUPPLIER_HPP_
#include <com/sun/star/sheet/XCellFormatRangesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGESQUERY_HPP_
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETFILTERABLEEX_HPP_
#include <com/sun/star/sheet/XSheetFilterableEx.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEET_HPP_
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETPAGEBREAK_HPP_
#include <com/sun/star/sheet/XSheetPageBreak.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEMOVEMENT_HPP_
#include <com/sun/star/sheet/XCellRangeMovement.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XPRINTAREAS_HPP_
#include <com/sun/star/sheet/XPrintAreas.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETLINKABLE_HPP_
#include <com/sun/star/sheet/XSheetLinkable.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSUBTOTALCALCULATABLE_HPP_
#include <com/sun/star/sheet/XSubTotalCalculatable.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XARRAYFORMULARANGE_HPP_
#include <com/sun/star/sheet/XArrayFormulaRange.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEDATA_HPP_
#include <com/sun/star/sheet/XCellRangeData.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLSERIES_HPP_
#include <com/sun/star/sheet/XCellSeries.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XMULTIPLEOPERATION_HPP_
#include <com/sun/star/sheet/XMultipleOperation.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLADDRESSABLE_HPP_
#include <com/sun/star/sheet/XCellAddressable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREPLACEABLE_HPP_
#include <com/sun/star/util/XReplaceable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XINDENT_HPP_
#include <com/sun/star/util/XIndent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMERGEABLE_HPP_
#include <com/sun/star/util/XMergeable.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_XAUTOFORMATTABLE_HPP_
#include <com/sun/star/table/XAutoFormattable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSORTABLE_HPP_
#include <com/sun/star/util/XSortable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XIMPORTABLE_HPP_
#include <com/sun/star/util/XImportable.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_XCOLUMNROWRANGE_HPP_
#include <com/sun/star/table/XColumnRowRange.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDATAPILOTTABLESSUPPLIER_HPP_
#include <com/sun/star/sheet/XDataPilotTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETANNOTATIONANCHOR_HPP_
#include <com/sun/star/sheet/XSheetAnnotationAnchor.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSCENARIOSSUPPLIER_HPP_
#include <com/sun/star/sheet/XScenariosSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETANNOTATIONSSUPPLIER_HPP_
#include <com/sun/star/sheet/XSheetAnnotationsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETAUDITING_HPP_
#include <com/sun/star/sheet/XSheetAuditing.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETOUTLINE_HPP_
#include <com/sun/star/sheet/XSheetOutline.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XPROTECTABLE_HPP_
#include <com/sun/star/util/XProtectable.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSCENARIO_HPP_
#include <com/sun/star/sheet/XScenario.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

class ScDocShell;
class ScMarkData;
class SchMemChart;
class ScPrintRangeSaver;
class ScAttrRectIterator;
class ScCellRangeObj;
class SvxUnoText;
class ScLinkListener;
class ScPatternAttr;


typedef ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertyChangeListener >* XPropertyChangeListenerPtr;
SV_DECL_PTRARR_DEL( XPropertyChangeListenerArr_Impl, XPropertyChangeListenerPtr, 4, 4 );

class ScNamedEntry;
typedef ScNamedEntry* ScNamedEntryPtr;
SV_DECL_PTRARR_DEL( ScNamedEntryArr_Impl, ScNamedEntryPtr, 4, 4 );


//  ScCellRangesBase - Basisklasse fuer ScCellRangesObj (mit Index-Access)
//                                  und ScCellRangeObj (ohne Index-Access)

//  XServiceInfo ist in den Ableitungen implementiert

class ScCellRangesBase : public com::sun::star::beans::XPropertySet,
                         public com::sun::star::beans::XMultiPropertySet,
                         public com::sun::star::beans::XPropertyState,
                         public com::sun::star::sheet::XSheetOperation,
                         public com::sun::star::chart::XChartDataArray,
                         public com::sun::star::util::XIndent,
                         public com::sun::star::sheet::XCellRangesQuery,
                         public com::sun::star::sheet::XFormulaQuery,
                         public com::sun::star::util::XReplaceable,
                         public com::sun::star::lang::XServiceInfo,
                         public com::sun::star::lang::XUnoTunnel,
                         public com::sun::star::lang::XTypeProvider,
                         public cppu::OWeakObject,
                         public SfxListener
{
    friend class ScTabViewObj;      // fuer select()
    friend class ScTableSheetObj;   // fuer createCursorByRange()

private:
    SfxItemPropertySet      aPropSet;
    ScDocShell*             pDocShell;
    ScLinkListener*         pValueListener;
    ScPatternAttr*          pCurrentFlat;
    ScPatternAttr*          pCurrentDeep;
    ScRangeList             aRanges;
    BOOL                    bChartColAsHdr;
    BOOL                    bChartRowAsHdr;
    BOOL                    bValueChangePosted;
    XPropertyChangeListenerArr_Impl aValueListeners;

    DECL_LINK( ValueListenerHdl, SfxHint* );
    DECL_LINK( ValueChanged, com::sun::star::beans::PropertyChangeEvent* );

private:
    void            PaintRanges_Impl( USHORT nPart );
    SchMemChart*    CreateMemChart_Impl() const;

    const ScPatternAttr*    GetCurrentAttrsFlat();
    const ScPatternAttr*    GetCurrentAttrsDeep();
    void                    ForgetCurrentAttrs();

    com::sun::star::uno::Reference<com::sun::star::sheet::XSheetCellRanges>
                            QueryDifferences_Impl(const com::sun::star::table::CellAddress& aCompare,
                                                    BOOL bColumnDiff);
    com::sun::star::uno::Reference<com::sun::star::uno::XInterface>
                            Find_Impl(const com::sun::star::uno::Reference<
                                        com::sun::star::util::XSearchDescriptor>& xDesc,
                                    const ScAddress* pLastPos);

    ::com::sun::star::beans::PropertyState SAL_CALL GetPropertyState(
                                    USHORT nWhich,
                                    const String& PropertyName );

public:
                            ScCellRangesBase();     // fuer SMART_REFLECTION Krempel
                            ScCellRangesBase(ScDocShell* pDocSh, const ScRange& rR);
                            ScCellRangesBase(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual                 ~ScCellRangesBase();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   release() throw(::com::sun::star::uno::RuntimeException);

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
    virtual void            RefChanged();

                            // aus Ableitungen, aber auch per getImplementation
    ScDocShell*             GetDocShell() const     { return pDocShell; }
    ScDocument*             GetDocument() const;
    const ScRangeList&      GetRangeList() const    { return aRanges; }

                            // per Service erzeugtes Objekt zum Leben erwecken:
    void                    InitInsertRange(ScDocShell* pDocSh, const ScRange& rR);

    void                    SetNewRange(const ScRange& rNew);   // fuer Cursor
    void                    SetNewRanges(const ScRangeList& rNew);

                            // XSheetOperation
    virtual double SAL_CALL computeFunction( ::com::sun::star::sheet::GeneralFunction nFunction )
                                throw(::com::sun::star::uno::Exception,
                                        ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   clearContents( sal_Int32 nContentFlags )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
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
    virtual void SAL_CALL   addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeVetoableChangeListener( const ::rtl::OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XMultiPropertySet
    virtual void SAL_CALL   setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames,
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
    virtual void SAL_CALL   setPropertyToDefault( const ::rtl::OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault(
                                    const ::rtl::OUString& aPropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XIndent
    virtual void SAL_CALL   decrementIndent() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   incrementIndent() throw(::com::sun::star::uno::RuntimeException);

                            // XChartData
    virtual void SAL_CALL   addChartDataChangeEventListener(
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeChartDataChangeEventListener( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::chart::XChartDataChangeEventListener >& aListener )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getNotANumber() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XChartDataArray
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > > SAL_CALL
                            getData() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setData( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::uno::Sequence< double > >& aData )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getRowDescriptions()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRowDescriptions( const ::com::sun::star::uno::Sequence<
                                ::rtl::OUString >& aRowDescriptions )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getColumnDescriptions()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setColumnDescriptions( const ::com::sun::star::uno::Sequence<
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

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScCellRangesBase* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XInterface> xObj );

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScCellRangesObj : public ScCellRangesBase,
                        public com::sun::star::sheet::XSheetCellRanges,
                        public com::sun::star::container::XNameContainer,
                        public com::sun::star::container::XEnumerationAccess
{
private:
    ScNamedEntryArr_Impl    aNamedEntries;

    ScCellRangeObj*         GetObjectByIndex_Impl(USHORT nIndex) const;

public:
                            ScCellRangesObj(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual                 ~ScCellRangesObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   release() throw(::com::sun::star::uno::RuntimeException);

    virtual void            RefChanged();

                            // XSheetCellRanges
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumerationAccess > SAL_CALL
                            getCells() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getRangeAddressesAsString()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::table::CellRangeAddress > SAL_CALL
                            getRangeAddresses() throw(::com::sun::star::uno::RuntimeException);

                            // XNameContainer
    virtual void SAL_CALL   insertByName( const ::rtl::OUString& aName,
                                const ::com::sun::star::uno::Any& aElement )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::container::ElementExistException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeByName( const ::rtl::OUString& Name )
                                throw(::com::sun::star::container::NoSuchElementException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XNameReplace
    virtual void SAL_CALL   replaceByName( const ::rtl::OUString& aName,
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
                       public com::sun::star::sheet::XCellRangeAddressable,
                       public com::sun::star::sheet::XSheetCellRange,
                       public com::sun::star::sheet::XArrayFormulaRange,
                       public com::sun::star::sheet::XCellRangeData,
                       public com::sun::star::sheet::XMultipleOperation,
                       public com::sun::star::util::XMergeable,
                       public com::sun::star::sheet::XCellSeries,
                       public com::sun::star::table::XAutoFormattable,
                       public com::sun::star::util::XSortable,
                       public com::sun::star::sheet::XSheetFilterableEx,
                       public com::sun::star::sheet::XSubTotalCalculatable,
                       public com::sun::star::util::XImportable,
                       public com::sun::star::sheet::XCellFormatRangesSupplier,
                       public com::sun::star::table::XColumnRowRange
{
private:
    SfxItemPropertySet      aRangePropSet;
    ScRange                 aRange;

protected:
    const ScRange&          GetRange() const    { return aRange; }

public:
                            ScCellRangeObj(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScCellRangeObj();

                            // uses ObjectShell from document, if set (returns NULL otherwise)
    static com::sun::star::uno::Reference<com::sun::star::table::XCellRange>
                            CreateRangeFromDoc( ScDocument* pDoc, const ScRange& rR );

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   release() throw(::com::sun::star::uno::RuntimeException);

    virtual void            RefChanged();

    // XCellRange ist Basisklasse von XSheetCellRange und XSheetOperation
//  operator XCellRangeRef() const  { return (XSheetCellRange*)this; }

                            // XCellRangeAddressable
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getRangeAddress()
                                throw(::com::sun::star::uno::RuntimeException);

                            // XSheetCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSpreadsheet > SAL_CALL
                            getSpreadsheet() throw(::com::sun::star::uno::RuntimeException);

                            // XArrayFormulaRange
    virtual ::rtl::OUString SAL_CALL getArrayFormula() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setArrayFormula( const ::rtl::OUString& aFormula )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XCellRangeData
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence<
                            ::com::sun::star::uno::Any > > SAL_CALL getDataArray()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setDataArray( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::uno::Any > >& aArray )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XMultipleOperation
    virtual void SAL_CALL   setTableOperation(
                                const ::com::sun::star::table::CellRangeAddress& aFormulaRange,
                                ::com::sun::star::sheet::TableOperationMode nMode,
                                const ::com::sun::star::table::CellAddress& aColumnCell,
                                const ::com::sun::star::table::CellAddress& aRowCell )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XMergeable
    virtual void SAL_CALL   merge( sal_Bool bMerge ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getIsMerged() throw(::com::sun::star::uno::RuntimeException);

                            // XCellSeries
    virtual void SAL_CALL   fillSeries( ::com::sun::star::sheet::FillDirection nFillDirection,
                                ::com::sun::star::sheet::FillMode nFillMode,
                                ::com::sun::star::sheet::FillDateMode nFillDateMode,
                                double fStep, double fEndValue )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   fillAuto( ::com::sun::star::sheet::FillDirection nFillDirection,
                                sal_Int32 nSourceCount )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XAutoFormattable
    virtual void SAL_CALL   autoFormat( const ::rtl::OUString& aName )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);

                            // XSortable
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            createSortDescriptor() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   sort( const ::com::sun::star::uno::Sequence<
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
    virtual void SAL_CALL   filter( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XSheetFilterDescriptor >& xDescriptor )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XSubTotalCalculatable
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sheet::XSubTotalDescriptor > SAL_CALL
                            createSubTotalDescriptor( sal_Bool bEmpty )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   applySubTotals( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::sheet::XSubTotalDescriptor >& xDescriptor,
                                sal_Bool bReplace ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeSubTotals() throw(::com::sun::star::uno::RuntimeException);

                            // XImportable
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
                            createImportDescriptor( sal_Bool bEmpty )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   doImport( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& aDescriptor )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XCellFormatRangesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
                            getCellFormatRanges() throw(::com::sun::star::uno::RuntimeException);

                            // XColumnRowRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableColumns > SAL_CALL
                            getColumns() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableRows > SAL_CALL
                            getRows() throw(::com::sun::star::uno::RuntimeException);

                            // XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL
                            getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByName( const ::rtl::OUString& aRange )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet ueberladen wegen Range-Properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
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


//! really derive cell from range?

class ScCellObj : public ScCellRangeObj,
                  public com::sun::star::text::XText,
                  public com::sun::star::container::XEnumerationAccess,
                  public com::sun::star::table::XCell,
                  public com::sun::star::sheet::XCellAddressable,
                  public com::sun::star::sheet::XSheetAnnotationAnchor,
                  public com::sun::star::text::XTextFieldsSupplier
{
private:
    SvxUnoText*             pUnoText;
    SfxItemPropertySet      aCellPropSet;
    ScAddress               aCellPos;

private:
    String      GetInputString_Impl(BOOL bEnglish) const;
    String      GetOutputString_Impl() const;
    void        SetString_Impl(const String& rString, BOOL bInterpret, BOOL bEnglish);
    double      GetValue_Impl() const;
    void        SetValue_Impl(double fValue);
    com::sun::star::table::CellContentType GetResultType_Impl();

public:
    static const SfxItemPropertyMap* GetEditPropertyMap();

                            ScCellObj(ScDocShell* pDocSh, const ScAddress& rP);
    virtual                 ~ScCellObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   release() throw(::com::sun::star::uno::RuntimeException);

    virtual void            RefChanged();

    SvxUnoText&             GetUnoText();

                            // XML import needs to set results at formula cells,
                            // not meant for any other purpose.
    void                    SetFormulaResultString( const ::rtl::OUString& rResult );
    void                    SetFormulaResultDouble( double fResult );
    const ScAddress&        GetPosition() const { return aCellPos; }

                            // XText
    virtual void SAL_CALL   insertTextContent( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextRange >& xRange,
                                const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::text::XTextContent >& xContent,
                                sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeTextContent( const ::com::sun::star::uno::Reference<
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
    virtual void SAL_CALL   insertString( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        const ::rtl::OUString& aString, sal_Bool bAbsorb )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   insertControlCharacter( const ::com::sun::star::uno::Reference<
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
    virtual void SAL_CALL   setString( const ::rtl::OUString& aString )
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
    virtual void SAL_CALL   setFormula( const ::rtl::OUString& aFormula )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getValue() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setValue( double nValue ) throw(::com::sun::star::uno::RuntimeException);
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
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
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
                            //! Listener-Krempel ??!?

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
                        public com::sun::star::sheet::XScenario
{
    friend class ScTableSheetsObj;      // fuer insertByName()

private:
    SfxItemPropertySet      aSheetPropSet;

    USHORT                  GetTab_Impl() const;
    void                    PrintAreaUndo_Impl( ScPrintRangeSaver* pOldRanges );

public:
                            ScTableSheetObj(ScDocShell* pDocSh, USHORT nTab);
    virtual                 ~ScTableSheetObj();

    void                    InitInsertSheet(ScDocShell* pDocSh, USHORT nTab);

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   release() throw(::com::sun::star::uno::RuntimeException);

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
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop,
                                sal_Int32 nRight, sal_Int32 nBottom )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL
                            getCellRangeByName( const ::rtl::OUString& aRange )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XSheetPageBreak
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::TablePageBreakData > SAL_CALL
                            getColumnPageBreaks() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::sheet::TablePageBreakData > SAL_CALL
                            getRowPageBreaks() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeAllManualPageBreaks() throw(::com::sun::star::uno::RuntimeException);

                            // XCellRangeMovement
    virtual void SAL_CALL   insertCells( const ::com::sun::star::table::CellRangeAddress& aRange,
                                ::com::sun::star::sheet::CellInsertMode nMode )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   removeRange( const ::com::sun::star::table::CellRangeAddress& aRange,
                                ::com::sun::star::sheet::CellDeleteMode nMode )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   moveRange( const ::com::sun::star::table::CellAddress& aDestination,
                                const ::com::sun::star::table::CellRangeAddress& aSource )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   copyRange( const ::com::sun::star::table::CellAddress& aDestination,
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
    virtual void SAL_CALL   setPrintTitleColumns( sal_Bool bPrintTitleColumns )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getTitleColumns()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setTitleColumns(
                                const ::com::sun::star::table::CellRangeAddress& aTitleColumns )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getPrintTitleRows() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPrintTitleRows( sal_Bool bPrintTitleRows )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellRangeAddress SAL_CALL getTitleRows()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setTitleRows(
                                const ::com::sun::star::table::CellRangeAddress& aTitleRows )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XSheetLinkable
    virtual ::com::sun::star::sheet::SheetLinkMode SAL_CALL getLinkMode()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setLinkMode( ::com::sun::star::sheet::SheetLinkMode nLinkMode )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLinkUrl() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setLinkUrl( const ::rtl::OUString& aLinkUrl )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLinkSheetName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setLinkSheetName( const ::rtl::OUString& aLinkSheetName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   link( const ::rtl::OUString& aUrl,
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
    virtual void SAL_CALL   clearArrows() throw(::com::sun::star::uno::RuntimeException);

                            // XSheetOutline
    virtual void SAL_CALL   group( const ::com::sun::star::table::CellRangeAddress& aRange,
                                ::com::sun::star::table::TableOrientation nOrientation )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   ungroup( const ::com::sun::star::table::CellRangeAddress& aRange,
                                ::com::sun::star::table::TableOrientation nOrientation )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   autoOutline( const ::com::sun::star::table::CellRangeAddress& aRange )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   clearOutline() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   hideDetail( const ::com::sun::star::table::CellRangeAddress& aRange )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   showDetail( const ::com::sun::star::table::CellRangeAddress& aRange )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   showLevel( sal_Int16 nLevel,
                                ::com::sun::star::table::TableOrientation nOrientation )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XProtectable
    virtual void SAL_CALL   protect( const ::rtl::OUString& aPassword )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   unprotect( const ::rtl::OUString& aPassword )
                                throw(::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isProtected() throw(::com::sun::star::uno::RuntimeException);

                            // XScenario
    virtual sal_Bool SAL_CALL getIsScenario() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getScenarioComment() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setScenarioComment( const ::rtl::OUString& aScenarioComment )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   addRanges( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::table::CellRangeAddress >& aRanges )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   apply() throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet ueberladen wegen Sheet-Properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
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

    static const com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static ScTableSheetObj* getImplementation( const com::sun::star::uno::Reference<
                                    com::sun::star::uno::XInterface> xObj );

                            // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScTableColumnObj : public ScCellRangeObj,
                         public com::sun::star::container::XNamed
{
private:
    SfxItemPropertySet      aColPropSet;

public:
                            ScTableColumnObj(ScDocShell* pDocSh, USHORT nCol, USHORT nTab);
    virtual                 ~ScTableColumnObj();

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
                                const ::com::sun::star::uno::Type & rType )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   acquire() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   release() throw(::com::sun::star::uno::RuntimeException);

                            // XNamed
    virtual ::rtl::OUString SAL_CALL getName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setName( const ::rtl::OUString& aName )
                                throw(::com::sun::star::uno::RuntimeException);

                            // XPropertySet ueberladen wegen Spalten-Properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
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
                            //! Listener-Krempel ??!?

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
    SfxItemPropertySet      aRowPropSet;

public:
                            ScTableRowObj(ScDocShell* pDocSh, USHORT nRow, USHORT nTab);
    virtual                 ~ScTableRowObj();

                            // XPropertySet ueberladen wegen Zeilen-Properties
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setPropertyValue( const ::rtl::OUString& aPropertyName,
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
                            //! Listener-Krempel ??!?

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


class ScCellsObj : public cppu::WeakImplHelper2<
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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

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
                                com::sun::star::container::XEnumeration,
                                com::sun::star::lang::XServiceInfo >,
                            public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScRangeList             aRanges;
    ScAddress               aPos;
    ScMarkData*             pMark;
    BOOL                    bAtEnd;

private:
    void                    Advance_Impl();
    void                    CheckPos_Impl();

public:
                            ScCellsEnumeration(ScDocShell* pDocSh, const ScRangeList& rR);
    virtual                 ~ScCellsEnumeration();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

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

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

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
                                    com::sun::star::container::XEnumeration,
                                    com::sun::star::lang::XServiceInfo >,
                                 public SfxListener
{
private:
    ScDocShell*             pDocShell;
    USHORT                  nTab;
    ScAttrRectIterator*     pIter;
    ScRange                 aNext;
    BOOL                    bAtEnd;
    BOOL                    bDirty;

private:
    void                    Advance_Impl();
    ScCellRangeObj*         NextObject_Impl();

public:
                            ScCellFormatsEnumeration(ScDocShell* pDocSh, const ScRange& rR);
    virtual                 ~ScCellFormatsEnumeration();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

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


#endif

