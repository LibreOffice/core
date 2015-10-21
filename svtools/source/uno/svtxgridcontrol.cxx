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

#include "svtxgridcontrol.hxx"
#include <com/sun/star/view/SelectionType.hpp>
#include "table/tablecontrolinterface.hxx"
#include "table/gridtablerenderer.hxx"
#include "table/tablecontrol.hxx"
#include "unocontroltablemodel.hxx"
#include <comphelper/sequence.hxx>
#include <rtl/ref.hxx>
#include <tools/diagnose_ex.h>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/grid/GridInvalidDataException.hpp>
#include <com/sun/star/awt/grid/GridInvalidModelException.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>

#include <vcl/svapp.hxx>

using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::awt::grid::XGridSelectionListener;
using ::com::sun::star::style::VerticalAlignment;
using ::com::sun::star::style::VerticalAlignment_TOP;
using ::com::sun::star::view::SelectionType;
using ::com::sun::star::view::SelectionType_NONE;
using ::com::sun::star::view::SelectionType_RANGE;
using ::com::sun::star::view::SelectionType_SINGLE;
using ::com::sun::star::view::SelectionType_MULTI;
using ::com::sun::star::awt::grid::XGridDataModel;
using ::com::sun::star::awt::grid::GridInvalidDataException;
using ::com::sun::star::lang::EventObject;
using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::awt::grid::XGridColumnModel;
using ::com::sun::star::awt::grid::GridSelectionEvent;
using ::com::sun::star::awt::grid::XGridColumn;
using ::com::sun::star::container::ContainerEvent;
using ::com::sun::star::awt::grid::GridDataEvent;
using ::com::sun::star::awt::grid::GridInvalidModelException;
using ::com::sun::star::util::VetoException;

namespace AccessibleEventId = ::com::sun::star::accessibility::AccessibleEventId;
namespace AccessibleStateType = ::com::sun::star::accessibility::AccessibleStateType;

using namespace ::svt::table;


SVTXGridControl::SVTXGridControl()
    :m_xTableModel( new UnoControlTableModel() )
    ,m_bTableModelInitCompleted( false )
    ,m_aSelectionListeners( *this )
{
}


SVTXGridControl::~SVTXGridControl()
{
}


void SVTXGridControl::SetWindow( const VclPtr< vcl::Window > &pWindow )
{
    SVTXGridControl_Base::SetWindow( pWindow );
    impl_checkTableModelInit();
}


void SVTXGridControl::impl_checkColumnIndex_throw( ::svt::table::TableControl const & i_table, sal_Int32 const i_columnIndex ) const
{
    if ( ( i_columnIndex < 0 ) || ( i_columnIndex >= i_table.GetColumnCount() ) )
        throw IndexOutOfBoundsException( OUString(), *const_cast< SVTXGridControl* >( this ) );
}


void SVTXGridControl::impl_checkRowIndex_throw( ::svt::table::TableControl const & i_table, sal_Int32 const i_rowIndex ) const
{
    if ( ( i_rowIndex < 0 ) || ( i_rowIndex >= i_table.GetRowCount() ) )
        throw IndexOutOfBoundsException( OUString(), *const_cast< SVTXGridControl* >( this ) );
}


sal_Int32 SAL_CALL SVTXGridControl::getRowAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::getRowAtPoint: no control (anymore)!", -1 );

    TableCell const tableCell = pTable->getTableControlInterface().hitTest( Point( x, y ) );
    return ( tableCell.nRow >= 0 ) ? tableCell.nRow : -1;
}


sal_Int32 SAL_CALL SVTXGridControl::getColumnAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::getColumnAtPoint: no control (anymore)!", -1 );

    TableCell const tableCell = pTable->getTableControlInterface().hitTest( Point( x, y ) );
    return ( tableCell.nColumn >= 0 ) ? tableCell.nColumn : -1;
}


sal_Int32 SAL_CALL SVTXGridControl::getCurrentColumn(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::getCurrentColumn: no control (anymore)!", -1 );

    sal_Int32 const nColumn = pTable->GetCurrentColumn();
    return ( nColumn >= 0 ) ? nColumn : -1;
}


sal_Int32 SAL_CALL SVTXGridControl::getCurrentRow(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::getCurrentRow: no control (anymore)!", -1 );

    sal_Int32 const nRow = pTable->GetCurrentRow();
    return ( nRow >= 0 ) ? nRow : -1;
}


void SAL_CALL SVTXGridControl::goToCell( ::sal_Int32 i_columnIndex, ::sal_Int32 i_rowIndex ) throw (RuntimeException, IndexOutOfBoundsException, VetoException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::getCurrentRow: no control (anymore)!" );

    impl_checkColumnIndex_throw( *pTable, i_columnIndex );
    impl_checkRowIndex_throw( *pTable, i_rowIndex );

    pTable->GoTo( i_columnIndex, i_rowIndex );
}


void SAL_CALL SVTXGridControl::addSelectionListener(const Reference< XGridSelectionListener > & listener) throw (RuntimeException, std::exception)
{
    m_aSelectionListeners.addInterface(listener);
}


void SAL_CALL SVTXGridControl::removeSelectionListener(const Reference< XGridSelectionListener > & listener) throw (RuntimeException, std::exception)
{
    m_aSelectionListeners.removeInterface(listener);
}


void SVTXGridControl::setProperty( const OUString& PropertyName, const Any& aValue) throw(RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::setProperty: no control (anymore)!" );

    switch( GetPropertyId( PropertyName ) )
    {
        case BASEPROPERTY_ROW_HEADER_WIDTH:
        {
            sal_Int32 rowHeaderWidth( -1 );
            aValue >>= rowHeaderWidth;
            if ( rowHeaderWidth <= 0 )
            {
                SAL_WARN( "svtools.uno", "SVTXGridControl::setProperty: illegal row header width!" );
                break;
            }

            m_xTableModel->setRowHeaderWidth( rowHeaderWidth );
            // TODO: the model should broadcast this change itself, and the table should invalidate itself as needed
            pTable->Invalidate();
        }
        break;

        case BASEPROPERTY_COLUMN_HEADER_HEIGHT:
        {
            sal_Int32 columnHeaderHeight = 0;
            if ( !aValue.hasValue() )
            {
                columnHeaderHeight = pTable->PixelToLogic( Size( 0, pTable->GetTextHeight() + 3 ), MAP_APPFONT ).Height();
            }
            else
            {
                aValue >>= columnHeaderHeight;
            }
            if ( columnHeaderHeight <= 0 )
            {
                SAL_WARN( "svtools.uno", "SVTXGridControl::setProperty: illegal column header width!" );
                break;
            }

            m_xTableModel->setColumnHeaderHeight( columnHeaderHeight );
            // TODO: the model should broadcast this change itself, and the table should invalidate itself as needed
            pTable->Invalidate();
        }
        break;

        case BASEPROPERTY_USE_GRID_LINES:
        {
            GridTableRenderer* pGridRenderer = dynamic_cast< GridTableRenderer* >(
                m_xTableModel->getRenderer().get() );
            if ( !pGridRenderer )
            {
                SAL_WARN( "svtools.uno", "SVTXGridControl::setProperty(UseGridLines): invalid renderer!" );
                break;
            }

            bool bUseGridLines = false;
            OSL_VERIFY( aValue >>= bUseGridLines );
            pGridRenderer->useGridLines( bUseGridLines );
            pTable->Invalidate();
        }
        break;

        case BASEPROPERTY_ROW_HEIGHT:
        {
            sal_Int32 rowHeight = 0;
            if ( !aValue.hasValue() )
            {
                rowHeight = pTable->PixelToLogic( Size( 0, pTable->GetTextHeight() + 3 ), MAP_APPFONT ).Height();
            }
            else
            {
                aValue >>= rowHeight;
            }
            m_xTableModel->setRowHeight( rowHeight );
            if ( rowHeight <= 0 )
            {
                SAL_WARN( "svtools.uno", "SVTXGridControl::setProperty: illegal row height!" );
                break;
            }

            // TODO: the model should broadcast this change itself, and the table should invalidate itself as needed
            pTable->Invalidate();
        }
        break;

        case BASEPROPERTY_BACKGROUNDCOLOR:
        {
            // let the base class handle this for the TableControl
            VCLXWindow::setProperty( PropertyName, aValue );
            // and forward to the grid control's data window
            if ( pTable->IsBackground() )
                pTable->getDataWindow().SetBackground( pTable->GetBackground() );
            else
                pTable->getDataWindow().SetBackground();
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
            bool bHScroll = true;
            if( aValue >>= bHScroll )
                m_xTableModel->setHorizontalScrollbarVisibility( bHScroll ? ScrollbarShowAlways : ScrollbarShowSmart );
            break;
        }

        case BASEPROPERTY_VSCROLL:
        {
            bool bVScroll = true;
            if( aValue >>= bVScroll )
            {
                m_xTableModel->setVerticalScrollbarVisibility( bVScroll ? ScrollbarShowAlways : ScrollbarShowSmart );
            }
            break;
        }

        case BASEPROPERTY_GRID_SHOWROWHEADER:
        {
            bool rowHeader = true;
            if( aValue >>= rowHeader )
            {
                m_xTableModel->setRowHeaders(rowHeader);
            }
            break;
        }

        case BASEPROPERTY_GRID_ROW_BACKGROUND_COLORS:
            m_xTableModel->setRowBackgroundColors( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_GRID_LINE_COLOR:
            m_xTableModel->setLineColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_GRID_HEADER_BACKGROUND:
            m_xTableModel->setHeaderBackgroundColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_GRID_HEADER_TEXT_COLOR:
            m_xTableModel->setHeaderTextColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_ACTIVE_SEL_BACKGROUND_COLOR:
            m_xTableModel->setActiveSelectionBackColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_INACTIVE_SEL_BACKGROUND_COLOR:
            m_xTableModel->setInactiveSelectionBackColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_ACTIVE_SEL_TEXT_COLOR:
            m_xTableModel->setActiveSelectionTextColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_INACTIVE_SEL_TEXT_COLOR:
            m_xTableModel->setInactiveSelectionTextColor( aValue );
            pTable->Invalidate();
            break;


        case BASEPROPERTY_TEXTCOLOR:
            m_xTableModel->setTextColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_TEXTLINECOLOR:
            m_xTableModel->setTextLineColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_VERTICALALIGN:
        {
            VerticalAlignment eAlign( VerticalAlignment_TOP );
            if ( aValue >>= eAlign )
                m_xTableModel->setVerticalAlign( eAlign );
            break;
        }

        case BASEPROPERTY_GRID_SHOWCOLUMNHEADER:
        {
            bool colHeader = true;
            if( aValue >>= colHeader )
            {
                m_xTableModel->setColumnHeaders(colHeader);
            }
            break;
        }
        case BASEPROPERTY_GRID_DATAMODEL:
        {
            Reference< XGridDataModel > const xDataModel( aValue, UNO_QUERY );
            if ( !xDataModel.is() )
                throw GridInvalidDataException("Invalid data model.", *this );

            m_xTableModel->setDataModel( xDataModel );
            impl_checkTableModelInit();
        }
        break;

        case BASEPROPERTY_GRID_COLUMNMODEL:
        {
            // obtain new col model
            Reference< XGridColumnModel > const xColumnModel( aValue, UNO_QUERY );
            if ( !xColumnModel.is() )
                throw GridInvalidModelException("Invalid column model.", *this );

            // remove all old columns
            m_xTableModel->removeAllColumns();

            // announce to the TableModel
            m_xTableModel->setColumnModel( xColumnModel );
            impl_checkTableModelInit();

            // add new columns
            impl_updateColumnsFromModel_nothrow();
            break;
        }
        default:
            VCLXWindow::setProperty( PropertyName, aValue );
        break;
    }
}


void SVTXGridControl::impl_checkTableModelInit()
{
    if ( !m_bTableModelInitCompleted && m_xTableModel->hasColumnModel() && m_xTableModel->hasDataModel() )
    {
        VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
        if ( pTable )
        {
            pTable->SetModel( PTableModel( m_xTableModel ) );

            m_bTableModelInitCompleted = true;

            // ensure default columns exist, if they have not previously been added
            Reference< XGridDataModel > const xDataModel( m_xTableModel->getDataModel(), UNO_QUERY_THROW );
            Reference< XGridColumnModel > const xColumnModel( m_xTableModel->getColumnModel(), UNO_QUERY_THROW );

            sal_Int32 const nDataColumnCount = xDataModel->getColumnCount();
            if ( ( nDataColumnCount > 0 ) && ( xColumnModel->getColumnCount() == 0 ) )
                xColumnModel->setDefaultColumns( nDataColumnCount );
                // this will trigger notifications, which in turn will let us update our m_xTableModel
        }
    }
}

namespace
{
    void lcl_convertColor( ::boost::optional< ::Color > const & i_color, Any & o_colorValue )
    {
        if ( !i_color )
            o_colorValue.clear();
        else
            o_colorValue <<= i_color->GetColor();
    }
}

Any SVTXGridControl::getProperty( const OUString& PropertyName ) throw(RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::getProperty: no control (anymore)!", Any() );

    Any aPropertyValue;

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
        aPropertyValue <<= eSelectionType;
        break;
    }

    case BASEPROPERTY_GRID_SHOWROWHEADER:
        aPropertyValue <<= m_xTableModel->hasRowHeaders();
        break;

    case BASEPROPERTY_GRID_SHOWCOLUMNHEADER:
        aPropertyValue <<= m_xTableModel->hasColumnHeaders();
        break;

    case BASEPROPERTY_GRID_DATAMODEL:
        aPropertyValue <<= m_xTableModel->getDataModel();
        break;

    case BASEPROPERTY_GRID_COLUMNMODEL:
        aPropertyValue <<= m_xTableModel->getColumnModel();
        break;

    case BASEPROPERTY_HSCROLL:
        {
            bool const bHasScrollbar = ( m_xTableModel->getHorizontalScrollbarVisibility() != ScrollbarShowNever );
            aPropertyValue <<= bHasScrollbar;
            break;
        }

    case BASEPROPERTY_VSCROLL:
        {
            bool const bHasScrollbar = ( m_xTableModel->getVerticalScrollbarVisibility() != ScrollbarShowNever );
            aPropertyValue <<= bHasScrollbar;
            break;
        }

    case BASEPROPERTY_USE_GRID_LINES:
    {
        GridTableRenderer* pGridRenderer = dynamic_cast< GridTableRenderer* >(
            m_xTableModel->getRenderer().get() );
        if ( !pGridRenderer )
        {
            SAL_WARN( "svtools.uno", "SVTXGridControl::getProperty(UseGridLines): invalid renderer!" );
            break;
        }

        aPropertyValue <<= pGridRenderer->useGridLines();
    }
    break;

    case BASEPROPERTY_GRID_ROW_BACKGROUND_COLORS:
    {
        ::boost::optional< ::std::vector< ::Color > > aColors( m_xTableModel->getRowBackgroundColors() );
        if ( !aColors )
            aPropertyValue.clear();
        else
        {
            Sequence< css::util::Color > aAPIColors( aColors->size() );
            for ( size_t i=0; i<aColors->size(); ++i )
            {
                aAPIColors[i] = aColors->at(i).GetColor();
            }
            aPropertyValue <<= aAPIColors;
        }
    }
    break;

    case BASEPROPERTY_GRID_LINE_COLOR:
        lcl_convertColor( m_xTableModel->getLineColor(), aPropertyValue );
        break;

    case BASEPROPERTY_GRID_HEADER_BACKGROUND:
        lcl_convertColor( m_xTableModel->getHeaderBackgroundColor(), aPropertyValue );
        break;

    case BASEPROPERTY_GRID_HEADER_TEXT_COLOR:
        lcl_convertColor( m_xTableModel->getHeaderTextColor(), aPropertyValue );
        break;

    case BASEPROPERTY_ACTIVE_SEL_BACKGROUND_COLOR:
        lcl_convertColor( m_xTableModel->getActiveSelectionBackColor(), aPropertyValue );
        break;

    case BASEPROPERTY_INACTIVE_SEL_BACKGROUND_COLOR:
        lcl_convertColor( m_xTableModel->getInactiveSelectionBackColor(), aPropertyValue );
        break;

    case BASEPROPERTY_ACTIVE_SEL_TEXT_COLOR:
        lcl_convertColor( m_xTableModel->getActiveSelectionTextColor(), aPropertyValue );
        break;

    case BASEPROPERTY_INACTIVE_SEL_TEXT_COLOR:
        lcl_convertColor( m_xTableModel->getInactiveSelectionTextColor(), aPropertyValue );
        break;

    case BASEPROPERTY_TEXTCOLOR:
        lcl_convertColor( m_xTableModel->getTextColor(), aPropertyValue );
        break;

    case BASEPROPERTY_TEXTLINECOLOR:
        lcl_convertColor( m_xTableModel->getTextLineColor(), aPropertyValue );
        break;

    default:
        aPropertyValue = VCLXWindow::getProperty( PropertyName );
        break;
    }

    return aPropertyValue;
}


void SAL_CALL SVTXGridControl::rowsInserted( const GridDataEvent& i_event ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    m_xTableModel->notifyRowsInserted( i_event );
}


void SAL_CALL
 SVTXGridControl::rowsRemoved( const GridDataEvent& i_event ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    m_xTableModel->notifyRowsRemoved( i_event );
}


void SAL_CALL SVTXGridControl::dataChanged( const GridDataEvent& i_event ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    m_xTableModel->notifyDataChanged( i_event );

    // if the data model is sortable, a dataChanged event is also fired in case the sort order changed.
    // So, just in case, invalidate the column header area, too.
    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::dataChanged: no control (anymore)!" );
    pTable->getTableControlInterface().invalidate( TableAreaColumnHeaders );
}


void SAL_CALL SVTXGridControl::rowHeadingChanged( const GridDataEvent& ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::rowHeadingChanged: no control (anymore)!" );

    // TODO: we could do better than this - invalidate the header area only
    pTable->getTableControlInterface().invalidate( TableAreaRowHeaders );
}


void SAL_CALL SVTXGridControl::elementInserted( const ContainerEvent& i_event ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    Reference< XGridColumn > const xGridColumn( i_event.Element, UNO_QUERY_THROW );

    sal_Int32 nIndex( m_xTableModel->getColumnCount() );
    OSL_VERIFY( i_event.Accessor >>= nIndex );
    m_xTableModel->insertColumn( nIndex, xGridColumn );
}


void SAL_CALL SVTXGridControl::elementRemoved( const ContainerEvent& i_event ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    sal_Int32 nIndex( -1 );
    OSL_VERIFY( i_event.Accessor >>= nIndex );
    m_xTableModel->removeColumn( nIndex );
}


void SAL_CALL SVTXGridControl::elementReplaced( const ContainerEvent& ) throw (RuntimeException, std::exception)
{
    OSL_ENSURE( false, "SVTXGridControl::elementReplaced: not implemented!" );
        // at the moment, the XGridColumnModel API does not allow replacing columns
    // TODO: replace the respective column in our table model
}



void SAL_CALL SVTXGridControl::disposing( const EventObject& Source ) throw(RuntimeException, std::exception)
{
    VCLXWindow::disposing( Source );
}


void SAL_CALL SVTXGridControl::selectRow( ::sal_Int32 i_rowIndex ) throw (RuntimeException, IndexOutOfBoundsException, std::exception )
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::selectRow: no control (anymore)!" );

    impl_checkRowIndex_throw( *pTable, i_rowIndex );

    pTable->SelectRow( i_rowIndex, true );
}


void SAL_CALL SVTXGridControl::selectAllRows() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::selectAllRows: no control (anymore)!" );

    pTable->SelectAllRows( true );
}


void SAL_CALL SVTXGridControl::deselectRow( ::sal_Int32 i_rowIndex ) throw (RuntimeException, IndexOutOfBoundsException, std::exception )
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::deselectRow: no control (anymore)!" );

    impl_checkRowIndex_throw( *pTable, i_rowIndex );

    pTable->SelectRow( i_rowIndex, false );
}


void SAL_CALL SVTXGridControl::deselectAllRows() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::deselectAllRows: no control (anymore)!" );

    pTable->SelectAllRows( false );
}


Sequence< ::sal_Int32 > SAL_CALL SVTXGridControl::getSelectedRows() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::getSelectedRows: no control (anymore)!", Sequence< sal_Int32 >() );

    sal_Int32 selectionCount = pTable->GetSelectedRowCount();
    Sequence< sal_Int32 > selectedRows( selectionCount );
    for ( sal_Int32 i=0; i<selectionCount; ++i )
        selectedRows[i] = pTable->GetSelectedRowIndex(i);
    return selectedRows;
}


sal_Bool SAL_CALL SVTXGridControl::hasSelectedRows() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::hasSelectedRows: no control (anymore)!", sal_True );

    return pTable->GetSelectedRowCount() > 0;
}


sal_Bool SAL_CALL SVTXGridControl::isRowSelected( ::sal_Int32 index ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::isRowSelected: no control (anymore)!", sal_False );

    return pTable->IsRowSelected( index );
}


void SVTXGridControl::dispose() throw(RuntimeException, std::exception)
{
    EventObject aObj;
    aObj.Source = static_cast<cppu::OWeakObject*>(this);
    m_aSelectionListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}


void SVTXGridControl::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    SolarMutexGuard aGuard;

    Reference< XWindow > xKeepAlive( this );

    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::ProcessWindowEvent: no control (anymore)!" );

    bool handled = false;
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_TABLEROW_SELECT:
        {
            if ( m_aSelectionListeners.getLength() )
                ImplCallItemListeners();
            handled = true;
        }
        break;

        case VCLEVENT_CONTROL_GETFOCUS:
        {
            // TODO: this doesn't belong here. It belongs into the TableControl/_Impl, so A11Y also
            // works when the control is used outside the UNO context
             if ( pTable->GetRowCount()>0 )
             {
                pTable->commitCellEventIfAccessibleAlive(
                    AccessibleEventId::STATE_CHANGED,
                    makeAny( AccessibleStateType::FOCUSED ),
                    Any()
                );
                pTable->commitTableEventIfAccessibleAlive(
                    AccessibleEventId::ACTIVE_DESCENDANT_CHANGED,
                    Any(),
                    Any()
                );
            }
            else
            {
                pTable->commitTableEventIfAccessibleAlive(
                    AccessibleEventId::STATE_CHANGED,
                    makeAny( AccessibleStateType::FOCUSED ),
                    Any()
                );
             }
        }
        break;

        case VCLEVENT_CONTROL_LOSEFOCUS:
        {
            // TODO: this doesn't belong here. It belongs into the TableControl/_Impl, so A11Y also
            // works when the control is used outside the UNO context
            if ( pTable->GetRowCount()>0 )
            {
                pTable->commitCellEventIfAccessibleAlive(
                    AccessibleEventId::STATE_CHANGED,
                    Any(),
                    makeAny( AccessibleStateType::FOCUSED )
                );
            }
            else
            {
                pTable->commitTableEventIfAccessibleAlive(
                    AccessibleEventId::STATE_CHANGED,
                    Any(),
                    makeAny( AccessibleStateType::FOCUSED )
                );
            }
        }
        break;
    }

    if ( !handled )
        VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
}


void SVTXGridControl::setEnable( sal_Bool bEnable ) throw(::com::sun::star::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    m_xTableModel->setEnabled( bEnable );
    vcl::Window * pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->Enable( bEnable );
        pWindow->EnableInput( bEnable );
        pWindow->Invalidate();
    }
}


void SVTXGridControl::ImplCallItemListeners()
{
    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::ImplCallItemListeners: no control (anymore)!" );

    if ( m_aSelectionListeners.getLength() )
    {
        GridSelectionEvent aEvent;
        aEvent.Source = static_cast<cppu::OWeakObject*>(this);

        sal_Int32 const nSelectedRowCount( pTable->GetSelectedRowCount() );
        aEvent.SelectedRowIndexes.realloc( nSelectedRowCount );
        for ( sal_Int32 i=0; i<nSelectedRowCount; ++i )
            aEvent.SelectedRowIndexes[i] = pTable->GetSelectedRowIndex( i );
        m_aSelectionListeners.selectionChanged( aEvent );
    }
}


void SVTXGridControl::impl_updateColumnsFromModel_nothrow()
{
    Reference< XGridColumnModel > const xColumnModel( m_xTableModel->getColumnModel() );
    ENSURE_OR_RETURN_VOID( xColumnModel.is(), "no model!" );
    VclPtr< TableControl > pTable = GetAsDynamic< TableControl >();
    ENSURE_OR_RETURN_VOID( pTable, "no table!" );

    try
    {
        const Sequence< Reference< XGridColumn > > columns = xColumnModel->getColumns();
        for (   const Reference< XGridColumn >* colRef = columns.getConstArray();
                colRef != columns.getConstArray() + columns.getLength();
                ++colRef
            )
        {
            if ( !colRef->is() )
            {
                SAL_WARN( "svtools.uno", "illegal column!" );
                continue;
            }

            m_xTableModel->appendColumn( *colRef );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
