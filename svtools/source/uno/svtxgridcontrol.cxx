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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include "svtxgridcontrol.hxx"
#include "accessibletableimp.hxx"
#include <com/sun/star/view/SelectionType.hpp>
#include "svtools/table/gridtablerenderer.hxx"
#include "svtools/table/defaultinputhandler.hxx"
#include "svtools/table/tablecontrol.hxx"
#include "unocontroltablemodel.hxx"
#include <comphelper/sequence.hxx>
#include <rtl/ref.hxx>
#include <tools/diagnose_ex.h>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/grid/GridInvalidDataException.hpp>
#include <com/sun/star/awt/grid/GridInvalidModelException.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>

using namespace ::svt::table;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt::grid;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::accessibility::AccessibleEventId;
using namespace ::com::sun::star::accessibility::AccessibleTableModelChangeType;
using ::com::sun::star::accessibility::AccessibleTableModelChange;


SVTXGridControl::SVTXGridControl()
    :m_pTableModel (new UnoControlTableModel()),
    m_xDataModel(0),
    m_xColumnModel(0),
    m_bHasColumnHeaders(false),
    m_bHasRowHeaders(false),
    m_bVScroll(false),
    m_bHScroll(false),
    m_bUpdate(false),
    m_nSelectedRowCount(0),
    m_aSelectionListeners( *this )
{
}

// ---------------------------------------------------------------------------------------------------------------------
SVTXGridControl::~SVTXGridControl()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void SVTXGridControl::SetWindow( Window* pWindow )
{
    TableControl* pTable = dynamic_cast< TableControl* >( pWindow );
    ENSURE_OR_THROW( ( pTable != NULL ) || ( pWindow == NULL ), "SVTXGridControl::SetWindow: illegal window!" );

    SVTXGridControl_Base::SetWindow( pWindow );

    if ( pTable )
        pTable->SetModel( PTableModel( m_pTableModel ) );
}

// ---------------------------------------------------------------------------------------------------------------------
sal_Int32 SAL_CALL SVTXGridControl::getItemIndexAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable != NULL, "SVTXGridControl::getItemIndexAtPoint: no control (anymore)!", -1 );
    return pTable->GetRowAtPoint( Point( x, y ) );
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::setToolTip(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& text, const com::sun::star::uno::Sequence< sal_Int32 >& columns) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable != NULL, "SVTXGridControl::setToolTip: no control (anymore)!" );
    pTable->setTooltip(text, columns);
}

void SAL_CALL SVTXGridControl::addSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
    m_aSelectionListeners.addInterface(listener);
}

void SAL_CALL SVTXGridControl::removeSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
    m_aSelectionListeners.removeInterface(listener);
}

void SVTXGridControl::setProperty( const ::rtl::OUString& PropertyName, const Any& aValue) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable != NULL, "SVTXGridControl::setProperty: no control (anymore)!" );

    switch( GetPropertyId( PropertyName ) )
    {
        case BASEPROPERTY_BACKGROUNDCOLOR:
        {
            // let the base class handle this for the TableControl
            VCLXWindow::setProperty( PropertyName, aValue );
            // and forward to the grid control's data window
            if ( pTable->IsBackground() )
                pTable->getDataWindow()->SetBackground( pTable->GetBackground() );
            else
                pTable->getDataWindow()->SetBackground();
        }
        break;

        case BASEPROPERTY_GRID_SELECTIONMODE:
        {
            SelectionType eSelectionType;
            if( aValue >>= eSelectionType )
            {
                SelectionMode eSelMode;
                switch( eSelectionType )
                {
                case SelectionType_SINGLE:  eSelMode = SINGLE_SELECTION; break;
                case SelectionType_RANGE:   eSelMode = RANGE_SELECTION; break;
                case SelectionType_MULTI:   eSelMode = MULTIPLE_SELECTION; break;
                default:                    eSelMode = NO_SELECTION; break;
                }
                if( pTable->getSelEngine()->GetSelectionMode() != eSelMode )
                    pTable->getSelEngine()->SetSelectionMode( eSelMode );
            }
            break;
        }
        case BASEPROPERTY_HSCROLL:
        {
            sal_Bool bHScroll = true;
            if( aValue >>= bHScroll )
            {
                m_bHScroll = bHScroll;
                m_pTableModel->setHorizontalScrollbarVisibility( bHScroll ? ScrollbarShowAlways : ScrollbarShowSmart );
            }
            break;
        }
        case BASEPROPERTY_VSCROLL:
        {
            sal_Bool bVScroll = true;
            if( aValue >>= bVScroll )
            {
                m_bVScroll = bVScroll;
                m_pTableModel->setVerticalScrollbarVisibility( bVScroll ? ScrollbarShowAlways : ScrollbarShowSmart );
            }
            break;
        }

        case BASEPROPERTY_GRID_SHOWROWHEADER:
        {
            sal_Bool rowHeader = true;
            if( aValue >>= rowHeader )
            {
                m_pTableModel->setRowHeaders(rowHeader);
            }
            break;
        }
        case BASEPROPERTY_GRID_HEADER_BACKGROUND:
        {
            sal_Int32 colorHeader = COL_TRANSPARENT;
            if( aValue >>= colorHeader )
            {
                m_pTableModel->setHeaderBackgroundColor(colorHeader);
            }
            break;
        }
        case BASEPROPERTY_GRID_LINE_COLOR:
        {
            sal_Int32 colorLine = COL_TRANSPARENT;
            if( aValue >>= colorLine )
            {
                m_pTableModel->setLineColor(colorLine);
            }
            break;
        }
        case BASEPROPERTY_GRID_EVEN_ROW_BACKGROUND:
        {
            sal_Int32 colorEvenRow = COL_TRANSPARENT;
            if( aValue >>= colorEvenRow )
            {
                m_pTableModel->setEvenRowBackgroundColor(colorEvenRow);
            }
            break;
        }
        case BASEPROPERTY_GRID_ROW_BACKGROUND:
        {
            sal_Int32 colorBackground = COL_TRANSPARENT;
            if( aValue >>= colorBackground )
            {
                m_pTableModel->setOddRowBackgroundColor(colorBackground);
            }
            break;
        }
        case BASEPROPERTY_TEXTCOLOR:
        {
            sal_Int32 colorText = 0x000000;
            if( aValue >>= colorText )
            {
                m_pTableModel->setTextColor(colorText);
            }
            break;
        }
        case BASEPROPERTY_VERTICALALIGN:
        {
            com::sun::star::style::VerticalAlignment vAlign(com::sun::star::style::VerticalAlignment(0));
            if( aValue >>= vAlign )
            {
                switch( vAlign )
                {
                case com::sun::star::style::VerticalAlignment_TOP:  m_pTableModel->setVerticalAlign(com::sun::star::style::VerticalAlignment(0)); break;
                case com::sun::star::style::VerticalAlignment_MIDDLE:   m_pTableModel->setVerticalAlign(com::sun::star::style::VerticalAlignment(1)); break;
                case com::sun::star::style::VerticalAlignment_BOTTOM: m_pTableModel->setVerticalAlign(com::sun::star::style::VerticalAlignment(2)); break;
                default: m_pTableModel->setVerticalAlign(com::sun::star::style::VerticalAlignment(0)); break;
                }
            }
            break;
        }

        case BASEPROPERTY_GRID_SHOWCOLUMNHEADER:
        {
            sal_Bool colHeader = true;
            if( aValue >>= colHeader )
            {
                m_pTableModel->setColumnHeaders(colHeader);
            }
            break;
        }
        case BASEPROPERTY_GRID_DATAMODEL:
        {
            {
                m_xDataModel = Reference< XGridDataModel >( aValue, UNO_QUERY );
                if ( !m_xDataModel.is() )
                    throw GridInvalidDataException( ::rtl::OUString::createFromAscii("The data model isn't set!"), *this );

                Sequence< Sequence< Any > > cellData = m_xDataModel->getData();
                if ( cellData.getLength() != 0 )
                {
                    const sal_Int32 nDataRowCount = cellData.getLength();

                    const Sequence< ::rtl::OUString > rowHeaders = m_xDataModel->getRowHeaders();
                    const sal_Int32 nRowHeaderCount = rowHeaders.getLength();

                    if ( ( nRowHeaderCount != 0 ) && ( nRowHeaderCount != nDataRowCount ) )
                        throw GridInvalidDataException(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "row header / content data inconsistency" ) ), *this );

                    // check data consistency
                    sal_Int32 nDataColumnCount = 0;
                    for ( sal_Int32 i=0; i<nDataRowCount; ++i )
                    {
                        const Sequence< Any >& rRawRowData = cellData[i];
                        if ( ( i > 0 ) && ( rRawRowData.getLength() != nDataColumnCount ) )
                            throw GridInvalidDataException(
                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "inconsistent column counts in the row data" ) ), *this );
                        nDataColumnCount = rRawRowData.getLength();
                    }

                    // ensure default columns exist, if they have not previously been added
                    if ( ( nDataColumnCount > 0 ) && ( m_xColumnModel->getColumnCount() == 0 ) )
                        m_xColumnModel->setDefaultColumns( nDataColumnCount );
                        // this will trigger notifications, which in turn will let us update our m_pTableModel

                    // ensure the row data has as much columns as indicate by our model
                    if ( nDataColumnCount != m_pTableModel->getColumnCount() )
                        throw GridInvalidDataException(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Grid model's column count doesn't match the row data's column count." ) ),
                            *this );

                    // finally add the rows
                    // TODO: suspend the model's notifications for the single rows, do one notification after all
                    // rows have been inserted
                    for ( TableSize i = 0; i < nDataRowCount; ++i )
                    {
                        m_pTableModel->appendRow( cellData[i], rowHeaders[i] );
                    }
                }

                sal_Int32 fontHeight = pTable->PixelToLogic( Size( 0, pTable->GetTextHeight()+3 ), MAP_APPFONT ).Height();
                if ( m_xDataModel->getRowHeight() == 0 )
                    m_pTableModel->setRowHeight( fontHeight );
                else
                    m_pTableModel->setRowHeight( m_xDataModel->getRowHeight() );
                m_pTableModel->setRowHeaderWidth( m_xDataModel->getRowHeaderWidth() );
            }
            break;
        }
        case BASEPROPERTY_GRID_COLUMNMODEL:
        {
            // remove all old columns
            m_pTableModel->removeAllColumns();

            // obtain new col model
            m_xColumnModel = Reference< XGridColumnModel >( aValue, UNO_QUERY_THROW );

            // remove new columns
            impl_updateColumnsFromModel_nothrow();

            break;
        }
        default:
            VCLXWindow::setProperty( PropertyName, aValue );
        break;
    }
}

Any SVTXGridControl::getProperty( const ::rtl::OUString& PropertyName ) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable != NULL, "SVTXGridControl::getProperty: no control (anymore)!", Any() );

    const sal_uInt16 nPropId = GetPropertyId( PropertyName );
    switch(nPropId)
    {
    case BASEPROPERTY_GRID_SELECTIONMODE:
    {
        SelectionType eSelectionType;

        SelectionMode eSelMode = pTable->getSelEngine()->GetSelectionMode();
        switch( eSelMode )
        {
            case SINGLE_SELECTION:  eSelectionType = SelectionType_SINGLE; break;
            case RANGE_SELECTION:   eSelectionType = SelectionType_RANGE; break;
            case MULTIPLE_SELECTION:eSelectionType = SelectionType_MULTI; break;
            default:                eSelectionType = SelectionType_NONE; break;
        }
        return Any( eSelectionType );
    }
    case BASEPROPERTY_GRID_SHOWROWHEADER:
    {
        return Any ((sal_Bool) m_pTableModel->hasRowHeaders());
    }
    case BASEPROPERTY_GRID_SHOWCOLUMNHEADER:
        return Any ((sal_Bool) m_pTableModel->hasColumnHeaders());
    case BASEPROPERTY_GRID_DATAMODEL:
        return Any ( m_xDataModel );
    case BASEPROPERTY_GRID_COLUMNMODEL:
        return Any ( m_xColumnModel);
    case BASEPROPERTY_HSCROLL:
        return Any ( m_bHScroll);
    case BASEPROPERTY_VSCROLL:
        return Any ( m_bVScroll);
    }

    return VCLXWindow::getProperty( PropertyName );
}

void SVTXGridControl::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
        BASEPROPERTY_GRID_SHOWROWHEADER,
        BASEPROPERTY_GRID_SHOWCOLUMNHEADER,
        BASEPROPERTY_GRID_DATAMODEL,
        BASEPROPERTY_GRID_COLUMNMODEL,
        BASEPROPERTY_GRID_SELECTIONMODE,
        BASEPROPERTY_GRID_EVEN_ROW_BACKGROUND,
        BASEPROPERTY_GRID_HEADER_BACKGROUND,
        BASEPROPERTY_GRID_LINE_COLOR,
        BASEPROPERTY_GRID_ROW_BACKGROUND,
        0
    );
    VCLXWindow::ImplGetPropertyIds( rIds, true );
}

void SAL_CALL SVTXGridControl::rowAdded(const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    const TableSize rowDataLength = (TableSize)Event.RowData.getLength();
    const TableSize colCount = m_xColumnModel->getColumnCount();
    if ( colCount == 0 )
    {
        m_xColumnModel->setDefaultColumns( rowDataLength );
            // this will trigger notifications, which in turn will let us update our m_pTableModel
    }
    if ( rowDataLength != colCount )
        throw GridInvalidDataException( ::rtl::OUString::createFromAscii("The column count doesn't match with the length of row data"), *this );

    m_pTableModel->appendRow( Event.RowData, Event.HeaderName );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::rowAdded: no control (anymore)!" );

    pTable->InvalidateDataWindow( m_pTableModel->getRowCount() - 1, 0, false );
    if ( pTable->isAccessibleAlive() )
    {
        pTable->commitGridControlEvent(TABLE_MODEL_CHANGED,
             makeAny( AccessibleTableModelChange(INSERT, m_pTableModel->getRowCount()-1, m_pTableModel->getRowCount(), 0, m_pTableModel->getColumnCount())),
            Any());
        pTable->commitGridControlEvent(CHILD,
             makeAny( pTable->m_pAccessTable->m_pAccessible->getTableHeader(TCTYPE_ROWHEADERBAR)),
            Any());
        for (sal_Int32 i = 0 ; i <= m_pTableModel->getColumnCount() ; ++i)
        {
            pTable->commitGridControlEvent(
                CHILD,
                makeAny( pTable->m_pAccessTable->m_pAccessible->getTable() ),
                Any());
        }
    }
}

void SAL_CALL SVTXGridControl::rowRemoved(const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::rowRemoved: no control (anymore)!" );

    if(Event.RowIndex == -1)
    {
        if ( !isSelectionEmpty() )
            deselectAllRows();

        m_pTableModel->clearAllRows();

        pTable->clearSelection();

        if ( pTable->isAccessibleAlive() )
        {
            pTable->commitGridControlEvent(TABLE_MODEL_CHANGED,
                 makeAny( AccessibleTableModelChange(DELETE, 0, m_pTableModel->getColumnCount(), 0, m_pTableModel->getColumnCount())),
                Any());
        }
    }
    else if ( Event.RowIndex >= 0 && Event.RowIndex < m_pTableModel->getRowCount() )
    {
        if ( isSelectedIndex( Event.RowIndex ) )
        {
            Sequence< sal_Int32 > selected(1);
            selected[0]=Event.RowIndex;
            deselectRows( selected );
        }
        m_pTableModel->removeRow( Event.RowIndex );
    }
    pTable->InvalidateDataWindow(Event.RowIndex, Event.RowIndex, true);
    if(pTable->isAccessibleAlive())
    {
        pTable->commitGridControlEvent(TABLE_MODEL_CHANGED,
             makeAny( AccessibleTableModelChange(DELETE, Event.RowIndex, Event.RowIndex+1, 0, m_pTableModel->getColumnCount())),
            Any());
    }
}

void SAL_CALL SVTXGridControl::dataChanged(const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::dataChanged: no control (anymore)!" );

    if ( Event.AttributeName.equalsAscii( "RowHeight" ) )
    {
        sal_Int32 rowHeight = m_pTableModel->getRowHeight();
        Event.NewValue>>=rowHeight;
        m_pTableModel->setRowHeight(rowHeight);
        pTable->Invalidate();
    }
    else if ( Event.AttributeName.equalsAscii( "RowHeaderWidth" ) )
    {
        sal_Int32 rowHeaderWidth = m_pTableModel->getRowHeaderWidth();
        Event.NewValue>>=rowHeaderWidth;
        m_pTableModel->setRowHeaderWidth(rowHeaderWidth);
        pTable->Invalidate();
    }
    else if ( Event.AttributeName.equalsAscii( "RowHeaders" ) )
    {
        Sequence< rtl::OUString > rowHeaders;
        OSL_VERIFY( Event.NewValue >>= rowHeaders );
        m_pTableModel->setRowHeaderNames( rowHeaders );
        pTable->Invalidate();
    }
    else if ( Event.AttributeName.equalsAscii( "CellUpdated" ) )
    {
        sal_Int32 col = -1;
        OSL_VERIFY( Event.OldValue >>= col );

        m_pTableModel->updateCellContent( Event.RowIndex, col, Event.NewValue );
        pTable->InvalidateDataWindow( Event.RowIndex, Event.RowIndex, false );
    }
    else if ( Event.AttributeName.equalsAscii( "RowUpdated" ) )
    {
        Sequence< sal_Int32 > cols;
        OSL_VERIFY( Event.OldValue >>= cols );

        Sequence< Any > values;
        OSL_VERIFY( Event.NewValue >>= values );

        ENSURE_OR_RETURN_VOID( cols.getLength() == values.getLength(), "SVTXGridControl::dataChanged: inconsistent array lengths!" );

        const TableSize columnCount = m_pTableModel->getColumnCount();
        // TODO: suspend listener notification, so that the table model doesn't notify |cols.size()| different events.
        // Instead, only one event should be broadcasted.
        for ( ColPos i = 0; i< cols.getLength(); ++i )
        {
            ENSURE_OR_CONTINUE( ( cols[i] >= 0 ) && ( cols[i] < columnCount ), "illegal column index" );
            m_pTableModel->updateCellContent( Event.RowIndex, cols[i], values[i] );
        }

        pTable->InvalidateDataWindow( Event.RowIndex, Event.RowIndex, false );
    }
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::elementInserted( const ContainerEvent& i_event ) throw (RuntimeException)
{
    const Reference< XGridColumn > xGridColumn( i_event.Element, UNO_QUERY_THROW );
    sal_Int32 nIndex( m_pTableModel->getColumnCount() );
    OSL_VERIFY( i_event.Accessor >>= nIndex );

    m_pTableModel->insertColumn( nIndex, xGridColumn );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::elementRemoved( const ContainerEvent& i_event ) throw (RuntimeException)
{
    // TODO: remove the respective column from our table model
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::elementReplaced( const ContainerEvent& i_event ) throw (RuntimeException)
{
    // TODO: replace the respective column in our table model
}


void SAL_CALL SVTXGridControl::disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXWindow::disposing( Source );
}

::sal_Int32 SAL_CALL SVTXGridControl::getMinSelectionIndex() throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::getMinSelectionIndex: no control (anymore)!", -1 );

    std::vector<RowPos>& selectedRows = pTable->GetSelectedRows();
    if(selectedRows.empty())
        return -1;
    else
    {
        std::vector<RowPos>::iterator itStart = selectedRows.begin();
        std::vector<RowPos>::iterator itEnd = selectedRows.end();
        return *(std::min_element(itStart, itEnd));
    }
}

::sal_Int32 SAL_CALL SVTXGridControl::getMaxSelectionIndex() throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::getMaxSelectionIndex: no control (anymore)!", -1 );

    std::vector<RowPos>& selectedRows = pTable->GetSelectedRows();
    if(selectedRows.empty())
        return -1;
    else
    {
        std::vector<RowPos>::iterator itStart = selectedRows.begin();
        std::vector<RowPos>::iterator itEnd = selectedRows.end();
        return *(std::max_element(itStart, itEnd));
    }
}

void SAL_CALL SVTXGridControl::selectRows(const ::com::sun::star::uno::Sequence< ::sal_Int32 >& rangeOfRows) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::selectRows: no control (anymore)!" );

    SelectionMode eSelMode = pTable->getSelEngine()->GetSelectionMode();
    if(eSelMode != NO_SELECTION)
    {
        sal_Int32 start = rangeOfRows[0];
        int seqSize = rangeOfRows.getLength();
        sal_Int32 end = rangeOfRows[seqSize-1];
        if((start >= 0 && start < m_pTableModel->getRowCount()) && (end >= 0 && end < m_pTableModel->getRowCount()))
        {
            std::vector<RowPos>& selectedRows = pTable->GetSelectedRows();
            if(eSelMode == SINGLE_SELECTION)
            {
                if(!selectedRows.empty())
                    selectedRows.clear();
                if(rangeOfRows.getLength() == 1)
                    selectedRows.push_back(start);
                else
                    return;
            }
            else
            {
                for(int i=0;i<seqSize;i++)
                {
                    if(!isSelectedIndex(rangeOfRows[i]))
                        selectedRows.push_back(rangeOfRows[i]);
                }
            }
            pTable->selectionChanged(true);
            pTable->InvalidateDataWindow(start, end, false);
            SetSynthesizingVCLEvent( sal_True );
            pTable->Select();
            SetSynthesizingVCLEvent( sal_False );
        }
    }
}

void SAL_CALL SVTXGridControl::selectAllRows() throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::selectAllRows: no control (anymore)!" );

    SelectionMode eSelMode = pTable->getSelEngine()->GetSelectionMode();
    if(eSelMode != NO_SELECTION)
    {
        std::vector<RowPos>& selectedRows = pTable->GetSelectedRows();
        if(!selectedRows.empty())
            selectedRows.clear();
        for(int i=0;i<m_pTableModel->getRowCount();i++)
            selectedRows.push_back(i);
        pTable->Invalidate();
        SetSynthesizingVCLEvent( sal_True );
        pTable->Select();
        SetSynthesizingVCLEvent( sal_False );
    }
}

void SAL_CALL SVTXGridControl::deselectRows(const ::com::sun::star::uno::Sequence< ::sal_Int32 >& rangeOfRows) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::deselectRows: no control (anymore)!" );

    std::vector<RowPos>& selectedRows = pTable->GetSelectedRows();
    std::vector<RowPos>::iterator itStart = selectedRows.begin();
    std::vector<RowPos>::iterator itEnd = selectedRows.end();
    for(int i = 0; i < rangeOfRows.getLength(); i++ )
    {
        std::vector<RowPos>::iterator iter = std::find(itStart, itEnd, rangeOfRows[i]);
        selectedRows.erase(iter);
    }
    pTable->selectionChanged(true);
    pTable->Invalidate();
    SetSynthesizingVCLEvent( sal_True );
    pTable->Select();
    SetSynthesizingVCLEvent( sal_False );
}

void SAL_CALL SVTXGridControl::deselectAllRows() throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::deselectAllRows: no control (anymore)!" );

    std::vector<RowPos>& selectedRows = pTable->GetSelectedRows();
    if(!selectedRows.empty())
        selectedRows.clear();
    pTable->Invalidate();
    SetSynthesizingVCLEvent( sal_True );
    pTable->Select();
    SetSynthesizingVCLEvent( sal_False );
}

::com::sun::star::uno::Sequence< ::sal_Int32 > SAL_CALL SVTXGridControl::getSelection() throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::getSelection: no control (anymore)!", Sequence< sal_Int32 >() );

    std::vector<RowPos>& selectedRows = pTable->GetSelectedRows();
    Sequence<sal_Int32> selectedRowsToSequence(comphelper::containerToSequence(selectedRows));
    return selectedRowsToSequence;
}

::sal_Bool SAL_CALL SVTXGridControl::isCellEditable() throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

::sal_Bool SAL_CALL SVTXGridControl::isSelectionEmpty() throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::getSelection: no control (anymore)!", sal_True );

    std::vector<RowPos>& selectedRows = pTable->GetSelectedRows();
    if(selectedRows.empty())
        return sal_True;
    else
        return sal_False;
}

::sal_Bool SAL_CALL SVTXGridControl::isSelectedIndex(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::isSelectedIndex: no control (anymore)!", sal_False );

    std::vector<RowPos>& selectedRows = pTable->GetSelectedRows();
    return std::find(selectedRows.begin(),selectedRows.end(), index) != selectedRows.end();
}

void SAL_CALL SVTXGridControl::selectRow(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::isSelectedIndex: no control (anymore)!" );

    if(index<0 || index>=m_pTableModel->getRowCount())
        return;
    SelectionMode eSelMode = pTable->getSelEngine()->GetSelectionMode();
    if(eSelMode != NO_SELECTION)
    {
        std::vector<RowPos>& selectedRows = pTable->GetSelectedRows();
        if(eSelMode == MULTIPLE_SELECTION)
        {
            if(!isSelectedIndex(index))
                selectedRows.push_back(index);
            else
                return;
        }
        else if(eSelMode == SINGLE_SELECTION)
        {
            if(!selectedRows.empty())
            {
                if(!isSelectedIndex(index))
                    deselectRows(getSelection());
                else
                    return;
            }
            selectedRows.push_back(index);
        }
        pTable->selectionChanged(true);
        pTable->InvalidateDataWindow(index, index, false);
        SetSynthesizingVCLEvent( sal_True );
        pTable->Select();
        SetSynthesizingVCLEvent( sal_False );
    }
}

void SAL_CALL SVTXGridControl::selectColumn(::sal_Int32 x) throw (::com::sun::star::uno::RuntimeException)
{
    (void)x;
}
void SVTXGridControl::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    m_aSelectionListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

void SVTXGridControl::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_TABLEROW_SELECT:
        {
            TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
            ENSURE_OR_BREAK( pTable, "SVTXGridControl::ProcessWindowEvent: no control (anymore)!" );
            if ( m_aSelectionListeners.getLength() )
                ImplCallItemListeners();
        }
        break;

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

void SVTXGridControl::ImplCallItemListeners()
{
    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::ImplCallItemListeners: no control (anymore)!" );

    if ( m_aSelectionListeners.getLength() )
    {
        ::std::vector<sal_Int32> selRows = pTable->GetSelectedRows();
        ::com::sun::star::awt::grid::GridSelectionEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)this;
        aEvent.Column = 0;
        sal_Int32 actSelRowCount = selRows.size();
        sal_Int32 diff = actSelRowCount - m_nSelectedRowCount;
        //row added to selection
        if(diff >= 1)
        {
            aEvent.Action = com::sun::star::awt::grid::SelectionEventType(0);
            aEvent.Row = selRows[actSelRowCount-1];
            aEvent.Range = diff;
        }
        //selected row changed
        else if(diff == 0 && actSelRowCount != 0)
        {
            aEvent.Row = selRows[actSelRowCount-1];
            aEvent.Action = com::sun::star::awt::grid::SelectionEventType(2);
            aEvent.Range = 0;
        }
        else
        {
            //selection changed: multiple row deselected, only 1 row is selected
            if(actSelRowCount == 1)
            {
                aEvent.Row = selRows[actSelRowCount-1];
                aEvent.Action = com::sun::star::awt::grid::SelectionEventType(2);
            }
            //row is deselected
            else
            {
                aEvent.Row = pTable->GetCurrentRow();
                aEvent.Action = com::sun::star::awt::grid::SelectionEventType(1);
            }
            aEvent.Range = 0;
        }
        m_nSelectedRowCount=actSelRowCount;
        m_aSelectionListeners.selectionChanged( aEvent );
    }
}

void SVTXGridControl::impl_updateColumnsFromModel_nothrow()
{
    ENSURE_OR_RETURN_VOID( m_xColumnModel.is(), "no model!" );
    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable != NULL, "no table!" );

    try
    {
        if ( m_xColumnModel->getColumnCount() != 0 )
        {
            if ( m_xColumnModel->getColumnHeaderHeight() == 0 )
            {
                sal_Int32 fontHeight = pTable->PixelToLogic( Size( 0, pTable->GetTextHeight() + 3 ), MAP_APPFONT ).Height();
                m_pTableModel->setColumnHeaderHeight( fontHeight );
            }
            else
                m_pTableModel->setColumnHeaderHeight( m_xColumnModel->getColumnHeaderHeight() );

            const Sequence< Reference< XGridColumn > > columns = m_xColumnModel->getColumns();
            for (   const Reference< XGridColumn >* colRef = columns.getConstArray();
                    colRef != columns.getConstArray() + columns.getLength();
                    ++colRef
                )
            {
                ENSURE_OR_CONTINUE( colRef->is(), "illegal column!" );

                m_pTableModel->appendColumn( *colRef );
            }
        }

    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

