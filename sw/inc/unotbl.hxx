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

#ifndef INCLUDED_SW_INC_UNOTBL_HXX
#define INCLUDED_SW_INC_UNOTBL_HXX

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

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <comphelper/uno3.hxx>

#include <calbck.hxx>
#include <TextCursorHelper.hxx>
#include <unotext.hxx>
#include <frmfmt.hxx>
#include <tuple>
#include <unocrsr.hxx>

class SwTable;
class SwTableBox;
class SwTableLine;
class SwTableCursor;
class SwTableBoxFormat;
class SwChartDataProvider;
class SwFrameFormat;

typedef
cppu::WeakImplHelper
<
    ::com::sun::star::table::XCell,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::container::XEnumerationAccess
>
SwXCellBaseClass;
class SwXCell final : public SwXCellBaseClass,
    public SwXText,
    public SwClient
{
    friend void   sw_setString( SwXCell &rCell, const OUString &rText,
                                bool bKeepNumberFormat );
    friend void   sw_setValue( SwXCell &rCell, double nVal );

    const SfxItemPropertySet*   m_pPropSet;
    SwTableBox*                 pBox;       // only set in non-XML import
    const SwStartNode*      pStartNode; // only set in XML import

    // table position where pBox was found last
    size_t nFndPos;
    static size_t const NOTFOUND = SAL_MAX_SIZE;

protected:
    virtual const SwStartNode *GetStartNode() const override;

    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >
        CreateCursor()
        throw (::com::sun::star::uno::RuntimeException) override;

    bool IsValid() const;

    virtual ~SwXCell();

    //SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;

public:
    SwXCell(SwFrameFormat* pTableFormat, SwTableBox* pBox, size_t nPos = NOTFOUND);
    SwXCell(SwFrameFormat* pTableFormat, const SwStartNode& rStartNode); // XML import interface

    TYPEINFO_OVERRIDE();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

    //XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XCell
    virtual OUString SAL_CALL getFormula(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFormula( const OUString& aFormula ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getValue(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getValue(  ) const throw(::com::sun::star::uno::RuntimeException, std::exception)
        { return const_cast<SwXCell*>(this)->getValue(); };
    virtual void SAL_CALL setValue( double nValue ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::table::CellContentType SAL_CALL getType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::table::CellContentType SAL_CALL getType(  ) const throw(::com::sun::star::uno::RuntimeException, std::exception)
        { return const_cast<SwXCell*>(this)->getType(); };
    virtual sal_Int32 SAL_CALL getError(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursor() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor >  SAL_CALL createTextCursorByRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & aTextPosition) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL  setString(const OUString& aString) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XEnumerationAccess - was: XParagraphEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    SwTableBox* GetTableBox() const { return pBox; }
    static SwXCell* CreateXCell(SwFrameFormat* pTableFormat, SwTableBox* pBox, SwTable *pTable = nullptr );
    SwTableBox* FindBox(SwTable* pTable, SwTableBox* pBox);
    SwFrameFormat* GetFrameFormat() const { return const_cast<SwFrameFormat*>(static_cast<const SwFrameFormat*>(GetRegisteredIn())); }
    double GetForcedNumericalValue() const;
    css::uno::Any GetAny() const;
};

class SwXTextTableRow final : public cppu::WeakImplHelper
<
    ::com::sun::star::beans::XPropertySet,
    ::com::sun::star::lang::XServiceInfo
>,
    public SwClient
{
    const SfxItemPropertySet*   m_pPropSet;
    SwTableLine*            pLine;

    SwFrameFormat* GetFrameFormat() { return static_cast<SwFrameFormat*>(GetRegisteredIn()); }
    const SwFrameFormat* GetFrameFormat() const { return const_cast<SwXTextTableRow*>(this)->GetFrameFormat(); }
protected:
    virtual ~SwXTextTableRow();
    //SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;

public:
    SwXTextTableRow(SwFrameFormat* pFormat, SwTableLine* pLine);

    TYPEINFO_OVERRIDE();

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw(::com::sun::star::beans::UnknownPropertyException,
              ::com::sun::star::beans::PropertyVetoException,
              ::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException,
              std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    const SwTableLine*  GetTableRow() const {return pLine;}
    static SwTableLine* FindLine(SwTable* pTable, SwTableLine* pLine);
};

typedef cppu::WeakImplHelper<
                                ::com::sun::star::text::XTextTableCursor,
                                ::com::sun::star::lang::XServiceInfo,
                                ::com::sun::star::beans::XPropertySet
                            > SwXTextTableCursor_Base;
class SW_DLLPUBLIC SwXTextTableCursor : public SwXTextTableCursor_Base
    ,public SwClient
    ,public OTextCursorHelper
{
    const SfxItemPropertySet*   m_pPropSet;

public:
    SwXTextTableCursor(SwFrameFormat* pFormat, SwTableBox* pBox);
    SwXTextTableCursor(SwFrameFormat& rTableFormat,
                        const SwTableCursor* pTableSelection);
    DECLARE_XINTERFACE()

    //XTextTableCursor
    virtual OUString SAL_CALL getRangeName()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL gotoCellByName( const OUString& aCellName, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL goLeft( sal_Int16 nCount, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL goRight( sal_Int16 nCount, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL goUp( sal_Int16 nCount, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL goDown( sal_Int16 nCount, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL gotoStart( sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL gotoEnd( sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL mergeRange()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual sal_Bool SAL_CALL splitRange( sal_Int16 Count, sal_Bool Horizontal )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

    // ITextCursorHelper
    virtual const SwPaM*        GetPaM() const override;
    virtual SwPaM*              GetPaM() override;
    virtual const SwDoc*        GetDoc() const override;
    virtual SwDoc*              GetDoc() override;

    const SwUnoCrsr&            GetCrsr() const;
    SwUnoCrsr&                  GetCrsr();
    sw::UnoCursorPointer m_pUnoCrsr;
    SwFrameFormat*       GetFrameFormat() const { return const_cast<SwFrameFormat*>(static_cast<const SwFrameFormat*>(GetRegisteredIn())); }
    virtual ~SwXTextTableCursor() { };
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
class SwXTextTable : public cppu::WeakImplHelper
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
private:
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::table::XTableRows > m_xRows;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::table::XTableColumns > m_xColumns;

    const SfxItemPropertySet*       m_pPropSet;

    // Descriptor-interface
    SwTableProperties_Impl*     pTableProps;
    OUString                    m_sTableName;
    bool                    bIsDescriptor;
    unsigned short              nRows;
    unsigned short              nColumns;

    bool m_bFirstRowAsLabel;
    bool m_bFirstColumnAsLabel;
protected:
    virtual ~SwXTextTable();
    SwXTextTable();
    SwXTextTable(SwFrameFormat& rFrameFormat);

public:
    static css::uno::Reference<css::text::XTextTable>
            CreateXTextTable(SwFrameFormat * pFrameFormat);

    SW_DLLPUBLIC static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    TYPEINFO_OVERRIDE();

    //XTextTable
    virtual void SAL_CALL initialize( sal_Int32 nRows, sal_Int32 nColumns ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableRows > SAL_CALL getRows(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XTableColumns > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL getCellByName( const OUString& aCellName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getCellNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextTableCursor > SAL_CALL createCursorByCellName( const OUString& aCellName )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;

    //XTextContent
    virtual void SAL_CALL attach(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XComponent
    virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
        throw (com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByName( const OUString& aRange )
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;

    //XChartDataArray
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > > SAL_CALL getData(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > >& aData ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getRowDescriptions(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRowDescriptions( const ::com::sun::star::uno::Sequence< OUString >& aRowDescriptions ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getColumnDescriptions(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setColumnDescriptions( const ::com::sun::star::uno::Sequence< OUString >& aColumnDescriptions ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XChartData
    virtual void SAL_CALL addChartDataChangeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeChartDataChangeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getNotANumber(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XSortable
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL createSortDescriptor() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL sort(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xDescriptor)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;

    //XAutoFormattable
    virtual void SAL_CALL autoFormat(const OUString& aName)
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) override;

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
        throw(::com::sun::star::beans::UnknownPropertyException,
              ::com::sun::star::beans::PropertyVetoException,
              ::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::lang::WrappedTargetException,
              ::com::sun::star::uno::RuntimeException,
              std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName )
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XNamed
    virtual OUString SAL_CALL getName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setName(const OUString& Name_) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XCellRangeData
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > > SAL_CALL getDataArray(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDataArray( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& aArray ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    void attachToRange(const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & xTextRange)throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception );

    sal_uInt16          getRowCount();
    sal_uInt16          getColumnCount();
    static ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange >     GetRangeByName(SwFrameFormat* pFormat, SwTable* pTable,
                        const OUString& sTLName, const OUString& sBRName,
                        SwRangeDescriptor& rDesc);

    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

    SwFrameFormat* GetFrameFormat() const { return const_cast<SwFrameFormat*>(static_cast<const SwFrameFormat*>(GetRegisteredIn())); }
    SW_DLLPUBLIC static void GetCellPosition(const OUString& rCellName, sal_Int32& o_rColumn, sal_Int32& o_rRow);

};

class SwXCellRange : public cppu::WeakImplHelper
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
    ::osl::Mutex m_Mutex;
    ::cppu::OInterfaceContainerHelper m_ChartListeners;

    SwRangeDescriptor           aRgDesc;
    const SfxItemPropertySet*   m_pPropSet;

    sw::UnoCursorPointer m_pTableCrsr;

    bool m_bFirstRowAsLabel;
    bool m_bFirstColumnAsLabel;
    std::tuple<sal_uInt32, sal_uInt32, sal_uInt32, sal_uInt32> getLabelCoordinates(bool bRow);
    css::uno::Sequence<OUString> getLabelDescriptions(bool bRow);
    void setLabelDescriptions(const css::uno::Sequence<OUString>& rDesc, bool bRow);

public:
    SwXCellRange(sw::UnoCursorPointer pCrsr, SwFrameFormat& rFrameFormat, SwRangeDescriptor& rDesc);
    void SetLabels(bool bFirstRowAsLabel, bool bFirstColumnAsLabel)
        { m_bFirstRowAsLabel = bFirstRowAsLabel, m_bFirstColumnAsLabel = bFirstColumnAsLabel; }
    virtual ~SwXCellRange() {};
    std::vector< css::uno::Reference< css::table::XCell > > GetCells();

    TYPEINFO_OVERRIDE();

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XCellRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCell > SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::table::XCellRange > SAL_CALL getCellRangeByName( const OUString& aRange ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue)
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
               ::com::sun::star::lang::WrappedTargetException,
               ::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    //XChartData
    virtual void SAL_CALL addChartDataChangeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeChartDataChangeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::chart::XChartDataChangeEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getNotANumber(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XChartDataArray
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > > SAL_CALL getData(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< double > >& aData ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getRowDescriptions(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRowDescriptions( const ::com::sun::star::uno::Sequence< OUString >& aRowDescriptions ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getColumnDescriptions(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setColumnDescriptions( const ::com::sun::star::uno::Sequence< OUString >& aColumnDescriptions ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XSortable
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL createSortDescriptor() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL sort(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xDescriptor)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;

    //XCellRangeData
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > > SAL_CALL getDataArray(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDataArray( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > >& aArray ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;

    SwFrameFormat*   GetFrameFormat() const { return const_cast<SwFrameFormat*>(static_cast<const SwFrameFormat*>(GetRegisteredIn())); }
    sal_uInt16      getRowCount();
    sal_uInt16      getColumnCount();

    const SwUnoCrsr* GetTableCrsr() const;
};

class SwXTableRows final : public cppu::WeakImplHelper
<
    ::com::sun::star::table::XTableRows,
    ::com::sun::star::lang::XServiceInfo
>,
    public SwClient

{
    SwFrameFormat* GetFrameFormat() { return static_cast<SwFrameFormat*>(GetRegisteredIn()); }
    const SwFrameFormat* GetFrameFormat() const { return const_cast<SwXTableRows*>(this)->GetFrameFormat(); }
protected:
    virtual ~SwXTableRows();
public:
    SwXTableRows(SwFrameFormat& rFrameFormat);

    TYPEINFO_OVERRIDE();

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XTableRows
    virtual void SAL_CALL insertByIndex(sal_Int32 nIndex, sal_Int32 nCount)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL removeByIndex(sal_Int32 nIndex, sal_Int32 nCount)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;
};

class SwXTableColumns : public cppu::WeakImplHelper
<
    ::com::sun::star::table::XTableColumns,
    ::com::sun::star::lang::XServiceInfo
>,
    public SwClient

{
    SwFrameFormat* GetFrameFormat() const { return const_cast<SwFrameFormat*>(static_cast<const SwFrameFormat*>(GetRegisteredIn())); }
protected:
    virtual ~SwXTableColumns();
public:
    SwXTableColumns(SwFrameFormat& rFrameFormat);

    TYPEINFO_OVERRIDE();

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) throw( ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //XTableColumns
    virtual void SAL_CALL insertByIndex(sal_Int32 nIndex, sal_Int32 nCount)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL removeByIndex(sal_Int32 nIndex, sal_Int32 nCount)
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew) override;
};

int sw_CompareCellRanges(
        const OUString &rRange1StartCell, const OUString &rRange1EndCell,
        const OUString &rRange2StartCell, const OUString &rRange2EndCell,
        bool bCmpColsFirst );

void sw_NormalizeRange( OUString &rCell1, OUString &rCell2 );

OUString sw_GetCellName( sal_Int32 nColumn, sal_Int32 nRow );

int sw_CompareCellsByColFirst( const OUString &rCellName1, const OUString &rCellName2 );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
