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
#include <com/sun/star/chart/XChartDataArray.hpp>
#include <com/sun/star/text/XTextTableCursor.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/sheet/XCellRangeData.hpp>
#include <com/sun/star/table/XAutoFormattable.hpp>

#include <cppuhelper/implbase.hxx>

#include <comphelper/uno3.hxx>

#include <svl/listener.hxx>

#include "calbck.hxx"
#include "TextCursorHelper.hxx"
#include "unotext.hxx"
#include "frmfmt.hxx"
#include "unocrsr.hxx"

class SwTable;
class SwTableBox;
class SwTableLine;
class SwTableCursor;
class SfxItemPropertySet;

typedef
cppu::WeakImplHelper
<
    css::table::XCell,
    css::lang::XServiceInfo,
    css::beans::XPropertySet,
    css::container::XEnumerationAccess
>
SwXCellBaseClass;
class SwXCell final : public SwXCellBaseClass,
    public SwXText,
    public SvtListener
{
    friend void   sw_setString( SwXCell &rCell, const OUString &rText,
                                bool bKeepNumberFormat );
    friend void   sw_setValue( SwXCell &rCell, double nVal );

    const SfxItemPropertySet*   m_pPropSet;
    SwTableBox*                 pBox;       // only set in non-XML import
    const SwStartNode*      pStartNode; // only set in XML import
    SwFrameFormat* m_pTableFormat;

    // table position where pBox was found last
    size_t nFndPos;
    static size_t const NOTFOUND = SAL_MAX_SIZE;

    virtual const SwStartNode *GetStartNode() const override;

    virtual css::uno::Reference< css::text::XTextCursor >
        CreateCursor() override;

    bool IsValid() const;

    virtual ~SwXCell() override;

    virtual void Notify(const SfxHint&) override;

public:
    SwXCell(SwFrameFormat* pTableFormat, SwTableBox* pBox, size_t nPos);
    SwXCell(SwFrameFormat* pTableFormat, const SwStartNode& rStartNode); // XML import interface


    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire(  ) throw() override;
    virtual void SAL_CALL release(  ) throw() override;

    //XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

    //XCell
    virtual OUString SAL_CALL getFormula(  ) override;
    virtual void SAL_CALL setFormula( const OUString& aFormula ) override;
    virtual double SAL_CALL getValue(  ) override;
    /// @throws css::uno::RuntimeException
    double getValue(  ) const
        { return const_cast<SwXCell*>(this)->getValue(); };
    virtual void SAL_CALL setValue( double nValue ) override;
    virtual css::table::CellContentType SAL_CALL getType(  ) override;
    virtual sal_Int32 SAL_CALL getError(  ) override;

    //XText
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL createTextCursor() override;
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL createTextCursorByRange(const css::uno::Reference< css::text::XTextRange > & aTextPosition) override;
    virtual void SAL_CALL  setString(const OUString& aString) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    //XEnumerationAccess - was: XParagraphEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL createEnumeration() override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    SwTableBox* GetTableBox() const { return pBox; }
    static SwXCell* CreateXCell(SwFrameFormat* pTableFormat, SwTableBox* pBox, SwTable *pTable = nullptr );
    SwTableBox* FindBox(SwTable* pTable, SwTableBox* pBox);
    SwFrameFormat* GetFrameFormat() const { return m_pTableFormat; }
    double GetForcedNumericalValue() const;
    css::uno::Any GetAny() const;
};

class SwXTextTableRow final
    : public cppu::WeakImplHelper<css::beans::XPropertySet, css::lang::XServiceInfo>
    , public SvtListener
{
    SwFrameFormat* m_pFormat;
    SwTableLine* pLine;
    const SfxItemPropertySet* m_pPropSet;

    SwFrameFormat* GetFrameFormat() { return m_pFormat; }
    const SwFrameFormat* GetFrameFormat() const { return m_pFormat; }
    virtual ~SwXTextTableRow() override;

public:
    SwXTextTableRow(SwFrameFormat* pFormat, SwTableLine* pLine);


    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    static SwTableLine* FindLine(SwTable* pTable, SwTableLine const * pLine);

    void Notify(const SfxHint&) override;
};

typedef cppu::WeakImplHelper<
    css::text::XTextTableCursor,
    css::lang::XServiceInfo,
    css::beans::XPropertySet> SwXTextTableCursor_Base;
class SW_DLLPUBLIC SwXTextTableCursor
    : public SwXTextTableCursor_Base
    , public SvtListener
    , public OTextCursorHelper
{
    SwFrameFormat* m_pFrameFormat;
    const SfxItemPropertySet* m_pPropSet;

public:
    SwXTextTableCursor(SwFrameFormat* pFormat, SwTableBox const* pBox);
    SwXTextTableCursor(SwFrameFormat& rTableFormat, const SwTableCursor* pTableSelection);
    DECLARE_XINTERFACE()

    //XTextTableCursor
    virtual OUString SAL_CALL getRangeName() override;
    virtual sal_Bool SAL_CALL gotoCellByName( const OUString& aCellName, sal_Bool bExpand ) override;
    virtual sal_Bool SAL_CALL goLeft( sal_Int16 nCount, sal_Bool bExpand ) override;
    virtual sal_Bool SAL_CALL goRight( sal_Int16 nCount, sal_Bool bExpand ) override;
    virtual sal_Bool SAL_CALL goUp( sal_Int16 nCount, sal_Bool bExpand ) override;
    virtual sal_Bool SAL_CALL goDown( sal_Int16 nCount, sal_Bool bExpand ) override;
    virtual void SAL_CALL gotoStart( sal_Bool bExpand ) override;
    virtual void SAL_CALL gotoEnd( sal_Bool bExpand ) override;
    virtual sal_Bool SAL_CALL mergeRange() override;
    virtual sal_Bool SAL_CALL splitRange( sal_Int16 Count, sal_Bool Horizontal ) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;


    // ITextCursorHelper
    virtual const SwPaM*        GetPaM() const override;
    virtual SwPaM*              GetPaM() override;
    virtual const SwDoc*        GetDoc() const override;
    virtual SwDoc*              GetDoc() override;

    virtual void Notify( const SfxHint& ) override;

    const SwUnoCursor&            GetCursor() const;
    SwUnoCursor&                  GetCursor();
    sw::UnoCursorPointer m_pUnoCursor;
    SwFrameFormat* GetFrameFormat() const { return m_pFrameFormat; }
};

struct SwRangeDescriptor
{
    sal_Int32 nTop;
    sal_Int32 nLeft;
    sal_Int32 nBottom;
    sal_Int32 nRight;

    void Normalize();
};

class SwXTextTable : public cppu::WeakImplHelper
<
    css::text::XTextTable,
    css::lang::XServiceInfo,
    css::table::XCellRange,
    css::chart::XChartDataArray,
    css::beans::XPropertySet,
    css::container::XNamed,
    css::table::XAutoFormattable,
    css::util::XSortable,
    css::lang::XUnoTunnel,
    css::sheet::XCellRangeData
>
{
private:
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    SwXTextTable();
    SwXTextTable(SwFrameFormat& rFrameFormat);
    virtual ~SwXTextTable() override;

public:
    static css::uno::Reference<css::text::XTextTable>
            CreateXTextTable(SwFrameFormat * pFrameFormat);

    SW_DLLPUBLIC static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    SW_DLLPUBLIC static void GetCellPosition(const OUString& rCellName, sal_Int32& o_rColumn, sal_Int32& o_rRow);

    SW_DLLPUBLIC SwFrameFormat* GetFrameFormat();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;


    //XTextTable
    virtual void SAL_CALL initialize( sal_Int32 nRows, sal_Int32 nColumns ) override;
    virtual css::uno::Reference< css::table::XTableRows > SAL_CALL getRows(  ) override;
    virtual css::uno::Reference< css::table::XTableColumns > SAL_CALL getColumns(  ) override;
    virtual css::uno::Reference< css::table::XCell > SAL_CALL getCellByName( const OUString& aCellName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getCellNames(  ) override;
    virtual css::uno::Reference< css::text::XTextTableCursor > SAL_CALL createCursorByCellName( const OUString& aCellName ) override;

    //XTextContent
    virtual void SAL_CALL attach(const css::uno::Reference< css::text::XTextRange > & xTextRange) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getAnchor(  ) override;

    //XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override;
    virtual void SAL_CALL removeEventListener(const css::uno::Reference< css::lang::XEventListener > & aListener) override;

    //XCellRange
    virtual css::uno::Reference< css::table::XCell > SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL getCellRangeByName( const OUString& aRange ) override;

    //XChartDataArray
    virtual css::uno::Sequence< css::uno::Sequence< double > > SAL_CALL getData(  ) override;
    virtual void SAL_CALL setData( const css::uno::Sequence< css::uno::Sequence< double > >& aData ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getRowDescriptions(  ) override;
    virtual void SAL_CALL setRowDescriptions( const css::uno::Sequence< OUString >& aRowDescriptions ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getColumnDescriptions(  ) override;
    virtual void SAL_CALL setColumnDescriptions( const css::uno::Sequence< OUString >& aColumnDescriptions ) override;

    //XChartData
    virtual void SAL_CALL addChartDataChangeEventListener( const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual void SAL_CALL removeChartDataChangeEventListener( const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual double SAL_CALL getNotANumber(  ) override;
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber ) override;

    //XSortable
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL createSortDescriptor() override;
    virtual void SAL_CALL sort(const css::uno::Sequence< css::beans::PropertyValue >& xDescriptor) override;

    //XAutoFormattable
    virtual void SAL_CALL autoFormat(const OUString& aName) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName(const OUString& Name_) override;

    //XCellRangeData
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > > SAL_CALL getDataArray(  ) override;
    virtual void SAL_CALL setDataArray( const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& aArray ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};

class SwXCellRange : public cppu::WeakImplHelper
<
    css::table::XCellRange,
    css::lang::XServiceInfo,
    css::lang::XUnoTunnel,
    css::beans::XPropertySet,
    css::chart::XChartDataArray,
    css::util::XSortable,
    css::sheet::XCellRangeData
>
{
private:
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    SwXCellRange(const sw::UnoCursorPointer& pCursor, SwFrameFormat& rFrameFormat, SwRangeDescriptor const & rDesc);
    virtual ~SwXCellRange() override;

public:
    static ::rtl::Reference<SwXCellRange> CreateXCellRange(
            const sw::UnoCursorPointer& pCursor, SwFrameFormat& rFrameFormat,
            SwRangeDescriptor const & rDesc);

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    void SetLabels(bool bFirstRowAsLabel, bool bFirstColumnAsLabel);

    std::vector<css::uno::Reference<css::table::XCell>> GetCells();

    const SwUnoCursor* GetTableCursor() const;

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    //XCellRange
    virtual css::uno::Reference< css::table::XCell > SAL_CALL getCellByPosition( sal_Int32 nColumn, sal_Int32 nRow ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL getCellRangeByPosition( sal_Int32 nLeft, sal_Int32 nTop, sal_Int32 nRight, sal_Int32 nBottom ) override;
    virtual css::uno::Reference< css::table::XCellRange > SAL_CALL getCellRangeByName( const OUString& aRange ) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName, const css::uno::Any& aValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XChartData
    virtual void SAL_CALL addChartDataChangeEventListener( const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual void SAL_CALL removeChartDataChangeEventListener( const css::uno::Reference< css::chart::XChartDataChangeEventListener >& aListener ) override;
    virtual double SAL_CALL getNotANumber(  ) override;
    virtual sal_Bool SAL_CALL isNotANumber( double nNumber ) override;

    //XChartDataArray
    virtual css::uno::Sequence< css::uno::Sequence< double > > SAL_CALL getData(  ) override;
    virtual void SAL_CALL setData( const css::uno::Sequence< css::uno::Sequence< double > >& aData ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getRowDescriptions(  ) override;
    virtual void SAL_CALL setRowDescriptions( const css::uno::Sequence< OUString >& aRowDescriptions ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getColumnDescriptions(  ) override;
    virtual void SAL_CALL setColumnDescriptions( const css::uno::Sequence< OUString >& aColumnDescriptions ) override;

    //XSortable
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL createSortDescriptor() override;
    virtual void SAL_CALL sort(const css::uno::Sequence< css::beans::PropertyValue >& xDescriptor) override;

    //XCellRangeData
    virtual css::uno::Sequence< css::uno::Sequence< css::uno::Any > > SAL_CALL getDataArray(  ) override;
    virtual void SAL_CALL setDataArray( const css::uno::Sequence< css::uno::Sequence< css::uno::Any > >& aArray ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

};

class SwXTableRows final : public cppu::WeakImplHelper
<
    css::table::XTableRows,
    css::lang::XServiceInfo
>
{
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;
    SwFrameFormat* GetFrameFormat();
    const SwFrameFormat* GetFrameFormat() const { return const_cast<SwXTableRows*>(this)->GetFrameFormat(); }
    virtual ~SwXTableRows() override;

public:
    SwXTableRows(SwFrameFormat& rFrameFormat);

    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XTableRows
    virtual void SAL_CALL insertByIndex(sal_Int32 nIndex, sal_Int32 nCount) override;
    virtual void SAL_CALL removeByIndex(sal_Int32 nIndex, sal_Int32 nCount) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class SwXTableColumns : public cppu::WeakImplHelper
<
    css::table::XTableColumns,
    css::lang::XServiceInfo
>
{
private:
    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;
    SwFrameFormat* GetFrameFormat() const;
protected:
    virtual ~SwXTableColumns() override;
public:
    SwXTableColumns(SwFrameFormat& rFrameFormat);


    //XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 nIndex) override;

    //XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    //XTableColumns
    virtual void SAL_CALL insertByIndex(sal_Int32 nIndex, sal_Int32 nCount) override;
    virtual void SAL_CALL removeByIndex(sal_Int32 nIndex, sal_Int32 nCount) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

int sw_CompareCellRanges(
        const OUString &rRange1StartCell, const OUString &rRange1EndCell,
        const OUString &rRange2StartCell, const OUString &rRange2EndCell,
        bool bCmpColsFirst );

void sw_NormalizeRange( OUString &rCell1, OUString &rCell2 );

OUString sw_GetCellName( sal_Int32 nColumn, sal_Int32 nRow );

int sw_CompareCellsByColFirst( const OUString &rCellName1, const OUString &rCellName2 );

int sw_CompareCellsByRowFirst( const OUString &rCellName1, const OUString &rCellName2 );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
