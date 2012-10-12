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

#ifndef SW_UNOTBL_HXX
#define SW_UNOTBL_HXX

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/util/XSortable.hpp>
#include <com/sun/star/chart/XChartData.hpp>
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/text/XTextTableCursor.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeData.hpp>
#include <com/sun/star/table/XAutoFormattable.hpp>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase7.hxx>
#include <cppuhelper/implbase10.hxx>

#include <comphelper/uno3.hxx>
#include <tools/string.hxx>

#include <calbck.hxx>
#include <TextCursorHelper.hxx>
#include <unoevtlstnr.hxx>
#include <unotext.hxx>


class SwUnoCrsr;
class SwTable;
class SwTableBox;
class SwTableLine;
class SwTableCursor;
class SwTableBoxFmt;
class SwChartDataProvider;
class SwFrmFmt;


class SwChartEventListenerContainer : public SwEventListenerContainer
{
    public:
        SwChartEventListenerContainer( ::com::sun::star::uno::XInterface* pxParentL) :
            SwEventListenerContainer(pxParentL){}
        void ChartDataChanged();
};

typedef
cppu::WeakImplHelper4
<
    ::com::sun::star::table::XCell,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::container::XEnumerationAccess
>
SwXCellBaseClass;
class SwXCell : public SwXCellBaseClass,
    public SwXText,
    public SwClient
{
    friend void   sw_setString( SwXCell &rCell, const rtl::OUString &rTxt,
                                 sal_Bool bKeepNumberFmt = sal_False );
    friend double sw_getValue( SwXCell &rCell );
    friend void   sw_setValue( SwXCell &rCell, double nVal );


    const SfxItemPropertySet*   m_pPropSet;
    SwTableBox*                 pBox;       // only set in non-XML import
    const SwStartNode*      pStartNode; // only set in XML import

    // table position where pBox was found last
    size_t nFndPos;
    static size_t const NOTFOUND = SAL_MAX_SIZE;

protected:
    virtual const SwStartNode *GetStartNode() const;

    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >
        CreateCursor()
        throw (::com::sun::star::uno::RuntimeException);

    bool IsValid() const;

    virtual ~SwXCell();

    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:
    SwXCell(SwFrmFmt* pTblFmt, SwTableBox* pBox, size_t nPos = NOTFOUND);
    SwXCell(SwFrmFmt* pTblFmt, const SwStartNode& rStartNode); // XML import interface


    TYPEINFO();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw();
    virtual void SAL_CALL release(  ) throw();

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    //XCell
    virtual ::rtl::OUString SAL_CALL getFormula(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setFormula( const ::rtl::OUString& aFormula ) throw(::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getValue(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setValue( double nValue ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::table::CellContentType SAL_CALL getType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getError(  ) throw(::com::sun::star::uno::RuntimeException);

    //XText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursor(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursorByRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & aTextPosition) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL  setString(const rtl::OUString& aString) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //XEnumerationAccess - was: XParagraphEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration(void) throw( ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    SwTableBox*   GetTblBox()const {return pBox;}
    static SwXCell*     CreateXCell(SwFrmFmt* pTblFmt, SwTableBox* pBox, SwTable *pTbl = 0 );
    SwTableBox*     FindBox(SwTable* pTable, SwTableBox* pBox);

    SwFrmFmt* GetFrmFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
};

class SwXTextTableRow : public cppu::WeakImplHelper2
<
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::lang::XServiceInfo
>,
    public SwClient
{
    const SfxItemPropertySet*   m_pPropSet;
    SwTableLine*            pLine;

    SwFrmFmt* GetFrmFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
protected:
    virtual ~SwXTextTableRow();
    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

public:
    SwXTextTableRow(SwFrmFmt* pFmt, SwTableLine* pLine);


    TYPEINFO();

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    const SwTableLine*  GetTblRow() const {return pLine;}
    static SwTableLine* FindLine(SwTable* pTable, SwTableLine* pLine);
};

typedef cppu::WeakImplHelper3<
                                ::com::sun::star::text::XTextTableCursor,
                                ::com::sun::star::lang::XServiceInfo,
                                ::com::sun::star::beans::XPropertySet
                            > SwXTextTableCursor_Base;
class SW_DLLPUBLIC SwXTextTableCursor : public SwXTextTableCursor_Base
    ,public SwClient
    ,public OTextCursorHelper
{
    SwDepend                aCrsrDepend;
    const SfxItemPropertySet*   m_pPropSet;

protected:
    virtual ~SwXTextTableCursor();
public:
    SwXTextTableCursor(SwFrmFmt* pFmt, SwTableBox* pBox);
    SwXTextTableCursor(SwFrmFmt& rTableFmt,
                        const SwTableCursor* pTableSelection);


    DECLARE_XINTERFACE()

    //XTextTableCursor
    virtual ::rtl::OUString SAL_CALL getRangeName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL gotoCellByName( const ::rtl::OUString& aCellName, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL goLeft( sal_Int16 nCount, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL goRight( sal_Int16 nCount, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL goUp( sal_Int16 nCount, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL goDown( sal_Int16 nCount, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoStart( sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoEnd( sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL mergeRange(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL splitRange( sal_Int16 Count, sal_Bool Horizontal ) throw(::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

    // ITextCursorHelper
    virtual const SwPaM*        GetPaM() const;
    virtual SwPaM*              GetPaM();
    virtual const SwDoc*        GetDoc() const;
    virtual SwDoc*              GetDoc();

    const SwUnoCrsr*            GetCrsr() const;
    SwUnoCrsr*                  GetCrsr();
    SwFrmFmt*       GetFrmFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
};

struct SwRangeDescriptor
{
    sal_Int32 nTop;
    sal_Int32 nLeft;
    sal_Int32 nBottom;
    sal_Int32 nRight;

    void Normalize();
};

class SwTableProperties_Impl;
class SwXTextTable : public cppu::WeakImplHelper10
<
    ::com::sun::star::text::XTextTable,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::table::XCellRange,
    ::com::sun::star::chart::XChartDataArray,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::container::XNamed,
    ::com::sun::star::table::XAutoFormattable,
    ::com::sun::star::util::XSortable,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::sheet::XCellRangeData
>,
    public SwClient
{
    SwEventListenerContainer        aLstnrCntnr;
    SwChartEventListenerContainer   aChartLstnrCntnr;
    const SfxItemPropertySet*       m_pPropSet;

    // Descriptor-interface
    SwTableProperties_Impl*     pTableProps;
    String                      m_sTableName;
    sal_Bool                        bIsDescriptor;
    unsigned short              nRows;
    unsigned short              nColumns;


    sal_Bool                        bFirstRowAsLabel    :1;
    sal_Bool                        bFirstColumnAsLabel :1;
protected:
    virtual ~SwXTextTable();
public:
    SwXTextTable();
    SwXTextTable(SwFrmFmt& rFrmFmt);


    SW_DLLPUBLIC static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    TYPEINFO();

    //XTextTable
    virtual void SAL_CALL initialize( sal_Int32 nRows, sal_Int32 nColumns ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableRows > SAL_CALL getRows(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableColumns > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL getCellByName( const ::rtl::OUString& aCellName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getCellNames(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextTableCursor > SAL_CALL createCursorByCellName( const ::rtl::OUString& aCellName ) throw(::com::sun::star::uno::RuntimeException);

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException);

    //XComponent
    virtual void SAL_CALL dispose(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException );

    //XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) throw(com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByName( const ::rtl::OUString& aRange ) throw(::com::sun::star::uno::RuntimeException);

    //XChartDataArray
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > > SAL_CALL getData(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > >& aData ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getRowDescriptions(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRowDescriptions( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRowDescriptions ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getColumnDescriptions(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setColumnDescriptions( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aColumnDescriptions ) throw(::com::sun::star::uno::RuntimeException);

    //XChartData
    virtual void SAL_CALL addChartDataChangeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChartDataChangeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getNotANumber(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber ) throw(::com::sun::star::uno::RuntimeException);

    //XSortable
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL createSortDescriptor(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL sort(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xDescriptor) throw( ::com::sun::star::uno::RuntimeException );

    //XAutoFormattable
    virtual void SAL_CALL autoFormat(const rtl::OUString& aName) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XNamed
    virtual rtl::OUString SAL_CALL getName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setName(const rtl::OUString& Name_) throw( ::com::sun::star::uno::RuntimeException );

    //XCellRangeData
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > > SAL_CALL getDataArray(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDataArray( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& aArray ) throw (::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    sal_uInt16          getRowCount(void);
    sal_uInt16          getColumnCount(void);
    ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >     GetRangeByName(SwFrmFmt* pFmt, SwTable* pTable,
                        const String& sTLName, const String& sBRName,
                        SwRangeDescriptor& rDesc);

    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

    SwFrmFmt* GetFrmFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
};

class SwXCellRange : public cppu::WeakImplHelper7
<
    ::com::sun::star::table::XCellRange,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::chart::XChartDataArray,
    ::com::sun::star::util::XSortable,
    ::com::sun::star::sheet::XCellRangeData
>,
    public SwClient
{
    SwDepend                        aCursorDepend; //the cursor is removed after the doc has been removed
    SwChartEventListenerContainer   aChartLstnrCntnr;

    SwRangeDescriptor           aRgDesc;
    const SfxItemPropertySet*   m_pPropSet;

    SwUnoCrsr*                  pTblCrsr;

    sal_Bool                        bFirstRowAsLabel    :1;
    sal_Bool                        bFirstColumnAsLabel :1;

public:
    SwXCellRange(SwUnoCrsr* pCrsr, SwFrmFmt& rFrmFmt, SwRangeDescriptor& rDesc);
    ~SwXCellRange();

    TYPEINFO();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);


    //XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) throw(com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByName( const ::rtl::OUString& aRange ) throw(::com::sun::star::uno::RuntimeException);

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XChartData
    virtual void SAL_CALL addChartDataChangeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChartDataChangeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual double SAL_CALL getNotANumber(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber ) throw(::com::sun::star::uno::RuntimeException);

    //XChartDataArray
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > > SAL_CALL getData(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > >& aData ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getRowDescriptions(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRowDescriptions( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRowDescriptions ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getColumnDescriptions(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setColumnDescriptions( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aColumnDescriptions ) throw(::com::sun::star::uno::RuntimeException);

    //XSortable
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL createSortDescriptor(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL sort(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xDescriptor) throw( ::com::sun::star::uno::RuntimeException );

    //XCellRangeData
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > > SAL_CALL getDataArray(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDataArray( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& aArray ) throw (::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

    SwFrmFmt*   GetFrmFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
    sal_uInt16      getRowCount(void);
    sal_uInt16      getColumnCount(void);

    const SwUnoCrsr* GetTblCrsr() const;

    // for SwChartDataSequence
    void GetDataSequence(
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > *pAnySeq,
            ::com::sun::star::uno::Sequence< ::rtl::OUString > *pTxtSeq,
            ::com::sun::star::uno::Sequence< double > *pDblSeq,
            sal_Bool bForceNumberResults = sal_False ) throw (::com::sun::star::uno::RuntimeException);

};

class SwXTableRows : public cppu::WeakImplHelper2
<
    ::com::sun::star::table::XTableRows,
    ::com::sun::star::lang::XServiceInfo
>,
    public SwClient

{
    SwFrmFmt* GetFrmFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
protected:
    virtual ~SwXTableRows();
public:
    SwXTableRows(SwFrmFmt& rFrmFmt);


    TYPEINFO();

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XTableRows
    virtual void SAL_CALL insertByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);
};

class SwXTableColumns : public cppu::WeakImplHelper2
<
    ::com::sun::star::table::XTableColumns,
    ::com::sun::star::lang::XServiceInfo
>,
    public SwClient

{
    SwFrmFmt* GetFrmFmt() const { return (SwFrmFmt*)GetRegisteredIn(); }
protected:
    virtual ~SwXTableColumns();
public:
    SwXTableColumns(SwFrmFmt& rFrmFmt);


    TYPEINFO();

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException );

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    //XTableColumns
    virtual void SAL_CALL insertByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeByIndex(sal_Int32 nIndex, sal_Int32 nCount) throw( ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);
};

#endif




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
