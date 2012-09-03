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

#include "svtxgridcontrol.hxx"
#include <com/sun/star/view/SelectionType.hpp>
#include "svtools/table/tablecontrolinterface.hxx"
#include "svtools/table/gridtablerenderer.hxx"
#include "svtools/table/tablecontrol.hxx"
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

/** === begin UNO using === **/
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
/** === end UNO using === **/

namespace AccessibleEventId = ::com::sun::star::accessibility::AccessibleEventId;
namespace AccessibleStateType = ::com::sun::star::accessibility::AccessibleStateType;

using ::svt::table::TableControl;

typedef ::com::sun::star::util::Color   UnoColor;

// ---------------------------------------------------------------------------------------------------------------------
SVTXGridControl::SVTXGridControl()
    :m_pTableModel( new UnoControlTableModel() )
    ,m_bTableModelInitCompleted( false )
    ,m_aSelectionListeners( *this )
{
}

// ---------------------------------------------------------------------------------------------------------------------
SVTXGridControl::~SVTXGridControl()
{
}

// ---------------------------------------------------------------------------------------------------------------------
void SVTXGridControl::SetWindow( Window* pWindow )
{
    SVTXGridControl_Base::SetWindow( pWindow );
    impl_checkTableModelInit();
}

// ---------------------------------------------------------------------------------------------------------------------
sal_Int32 SAL_CALL SVTXGridControl::getRowAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable != NULL, "SVTXGridControl::getRowAtPoint: no control (anymore)!", -1 );

    TableCell const tableCell = pTable->getTableControlInterface().hitTest( Point( x, y ) );
    return ( tableCell.nRow >= 0 ) ? tableCell.nRow : -1;
}

// ---------------------------------------------------------------------------------------------------------------------
sal_Int32 SAL_CALL SVTXGridControl::getColumnAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable != NULL, "SVTXGridControl::getColumnAtPoint: no control (anymore)!", -1 );

    TableCell const tableCell = pTable->getTableControlInterface().hitTest( Point( x, y ) );
    return ( tableCell.nColumn >= 0 ) ? tableCell.nColumn : -1;
}

// ---------------------------------------------------------------------------------------------------------------------
sal_Int32 SAL_CALL SVTXGridControl::getCurrentColumn(  ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable != NULL, "SVTXGridControl::getCurrentColumn: no control (anymore)!", -1 );

    sal_Int32 const nColumn = pTable->GetCurrentColumn();
    return ( nColumn >= 0 ) ? nColumn : -1;
}

// ---------------------------------------------------------------------------------------------------------------------
sal_Int32 SAL_CALL SVTXGridControl::getCurrentRow(  ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable != NULL, "SVTXGridControl::getCurrentRow: no control (anymore)!", -1 );

    sal_Int32 const nRow = pTable->GetCurrentRow();
    return ( nRow >= 0 ) ? nRow : -1;
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::addSelectionListener(const Reference< XGridSelectionListener > & listener) throw (RuntimeException)
{
    m_aSelectionListeners.addInterface(listener);
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::removeSelectionListener(const Reference< XGridSelectionListener > & listener) throw (RuntimeException)
{
    m_aSelectionListeners.removeInterface(listener);
}

// ---------------------------------------------------------------------------------------------------------------------
void SVTXGridControl::setProperty( const ::rtl::OUString& PropertyName, const Any& aValue) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable != NULL, "SVTXGridControl::setProperty: no control (anymore)!" );

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

            m_pTableModel->setRowHeaderWidth( rowHeaderWidth );
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

            m_pTableModel->setColumnHeaderHeight( columnHeaderHeight );
            // TODO: the model should broadcast this change itself, and the table should invalidate itself as needed
            pTable->Invalidate();
        }
        break;

        case BASEPROPERTY_USE_GRID_LINES:
        {
            GridTableRenderer* pGridRenderer = dynamic_cast< GridTableRenderer* >(
                m_pTableModel->getRenderer().get() );
            if ( !pGridRenderer )
            {
                SAL_WARN( "svtools.uno", "SVTXGridControl::setProperty(UseGridLines): invalid renderer!" );
                break;
            }

            sal_Bool bUseGridLines = sal_False;
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
            m_pTableModel->setRowHeight( rowHeight );
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
            sal_Bool bHScroll = true;
            if( aValue >>= bHScroll )
                m_pTableModel->setHorizontalScrollbarVisibility( bHScroll ? ScrollbarShowAlways : ScrollbarShowSmart );
            break;
        }

        case BASEPROPERTY_VSCROLL:
        {
            sal_Bool bVScroll = true;
            if( aValue >>= bVScroll )
            {
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

        case BASEPROPERTY_GRID_ROW_BACKGROUND_COLORS:
            m_pTableModel->setRowBackgroundColors( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_GRID_LINE_COLOR:
            m_pTableModel->setLineColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_GRID_HEADER_BACKGROUND:
            m_pTableModel->setHeaderBackgroundColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_GRID_HEADER_TEXT_COLOR:
            m_pTableModel->setHeaderTextColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_ACTIVE_SEL_BACKGROUND_COLOR:
            m_pTableModel->setActiveSelectionBackColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_INACTIVE_SEL_BACKGROUND_COLOR:
            m_pTableModel->setInactiveSelectionBackColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_ACTIVE_SEL_TEXT_COLOR:
            m_pTableModel->setActiveSelectionTextColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_INACTIVE_SEL_TEXT_COLOR:
            m_pTableModel->setInactiveSelectionTextColor( aValue );
            pTable->Invalidate();
            break;


        case BASEPROPERTY_TEXTCOLOR:
            m_pTableModel->setTextColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_TEXTLINECOLOR:
            m_pTableModel->setTextLineColor( aValue );
            pTable->Invalidate();
            break;

        case BASEPROPERTY_VERTICALALIGN:
        {
            VerticalAlignment eAlign( VerticalAlignment_TOP );
            if ( aValue >>= eAlign )
                m_pTableModel->setVerticalAlign( eAlign );
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
            Reference< XGridDataModel > const xDataModel( aValue, UNO_QUERY );
            if ( !xDataModel.is() )
                throw GridInvalidDataException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid data model." ) ), *this );

            m_pTableModel->setDataModel( xDataModel );
            impl_checkTableModelInit();
        }
        break;

        case BASEPROPERTY_GRID_COLUMNMODEL:
        {
            // obtain new col model
            Reference< XGridColumnModel > const xColumnModel( aValue, UNO_QUERY );
            if ( !xColumnModel.is() )
                throw GridInvalidModelException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Invalid column model." ) ), *this );

            // remove all old columns
            m_pTableModel->removeAllColumns();

            // announce to the TableModel
            m_pTableModel->setColumnModel( xColumnModel );
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
    if ( !m_bTableModelInitCompleted && m_pTableModel->hasColumnModel() && m_pTableModel->hasDataModel() )
    {
        TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
        if ( pTable )
        {
            pTable->SetModel( PTableModel( m_pTableModel ) );

            m_bTableModelInitCompleted = true;

            // ensure default columns exist, if they have not previously been added
            Reference< XGridDataModel > const xDataModel( m_pTableModel->getDataModel(), UNO_QUERY_THROW );
            Reference< XGridColumnModel > const xColumnModel( m_pTableModel->getColumnModel(), UNO_QUERY_THROW );

            sal_Int32 const nDataColumnCount = xDataModel->getColumnCount();
            if ( ( nDataColumnCount > 0 ) && ( xColumnModel->getColumnCount() == 0 ) )
                xColumnModel->setDefaultColumns( nDataColumnCount );
                // this will trigger notifications, which in turn will let us update our m_pTableModel
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

Any SVTXGridControl::getProperty( const ::rtl::OUString& PropertyName ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable != NULL, "SVTXGridControl::getProperty: no control (anymore)!", Any() );

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
        aPropertyValue <<=  sal_Bool( m_pTableModel->hasRowHeaders() );
        break;

    case BASEPROPERTY_GRID_SHOWCOLUMNHEADER:
        aPropertyValue <<=  sal_Bool( m_pTableModel->hasColumnHeaders() );
        break;

    case BASEPROPERTY_GRID_DATAMODEL:
        aPropertyValue <<= m_pTableModel->getDataModel();
        break;

    case BASEPROPERTY_GRID_COLUMNMODEL:
        aPropertyValue <<= m_pTableModel->getColumnModel();
        break;

    case BASEPROPERTY_HSCROLL:
        {
            sal_Bool const bHasScrollbar = ( m_pTableModel->getHorizontalScrollbarVisibility() != ScrollbarShowNever );
            aPropertyValue <<= bHasScrollbar;
            break;
        }

    case BASEPROPERTY_VSCROLL:
        {
            sal_Bool const bHasScrollbar = ( m_pTableModel->getVerticalScrollbarVisibility() != ScrollbarShowNever );
            aPropertyValue <<= bHasScrollbar;
            break;
        }

    case BASEPROPERTY_USE_GRID_LINES:
    {
        GridTableRenderer* pGridRenderer = dynamic_cast< GridTableRenderer* >(
            m_pTableModel->getRenderer().get() );
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
        ::boost::optional< ::std::vector< ::Color > > aColors( m_pTableModel->getRowBackgroundColors() );
        if ( !aColors )
            aPropertyValue.clear();
        else
        {
            Sequence< UnoColor > aAPIColors( aColors->size() );
            for ( size_t i=0; i<aColors->size(); ++i )
            {
                aAPIColors[i] = aColors->at(i).GetColor();
            }
            aPropertyValue <<= aAPIColors;
        }
    }
    break;

    case BASEPROPERTY_GRID_LINE_COLOR:
        lcl_convertColor( m_pTableModel->getLineColor(), aPropertyValue );
        break;

    case BASEPROPERTY_GRID_HEADER_BACKGROUND:
        lcl_convertColor( m_pTableModel->getHeaderBackgroundColor(), aPropertyValue );
        break;

    case BASEPROPERTY_GRID_HEADER_TEXT_COLOR:
        lcl_convertColor( m_pTableModel->getHeaderTextColor(), aPropertyValue );
        break;

    case BASEPROPERTY_ACTIVE_SEL_BACKGROUND_COLOR:
        lcl_convertColor( m_pTableModel->getActiveSelectionBackColor(), aPropertyValue );
        break;

    case BASEPROPERTY_INACTIVE_SEL_BACKGROUND_COLOR:
        lcl_convertColor( m_pTableModel->getInactiveSelectionBackColor(), aPropertyValue );
        break;

    case BASEPROPERTY_ACTIVE_SEL_TEXT_COLOR:
        lcl_convertColor( m_pTableModel->getActiveSelectionTextColor(), aPropertyValue );
        break;

    case BASEPROPERTY_INACTIVE_SEL_TEXT_COLOR:
        lcl_convertColor( m_pTableModel->getInactiveSelectionTextColor(), aPropertyValue );
        break;

    case BASEPROPERTY_TEXTCOLOR:
        lcl_convertColor( m_pTableModel->getTextColor(), aPropertyValue );
        break;

    case BASEPROPERTY_TEXTLINECOLOR:
        lcl_convertColor( m_pTableModel->getTextLineColor(), aPropertyValue );
        break;

    default:
        aPropertyValue = VCLXWindow::getProperty( PropertyName );
        break;
    }

    return aPropertyValue;
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::rowsInserted( const GridDataEvent& i_event ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    m_pTableModel->notifyRowsInserted( i_event );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL//----------------------------------------------------------------------------------------------------------------------
 SVTXGridControl::rowsRemoved( const GridDataEvent& i_event ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    m_pTableModel->notifyRowsRemoved( i_event );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::dataChanged( const GridDataEvent& i_event ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    m_pTableModel->notifyDataChanged( i_event );

    // if the data model is sortable, a dataChanged event is also fired in case the sort order changed.
    // So, just in case, invalidate the column header area, too.
    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::dataChanged: no control (anymore)!" );
    pTable->getTableControlInterface().invalidate( TableAreaColumnHeaders );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::rowHeadingChanged( const GridDataEvent& i_event ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    OSL_UNUSED( i_event );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::rowHeadingChanged: no control (anymore)!" );

    // TODO: we could do better than this - invalidate the header area only
    pTable->getTableControlInterface().invalidate( TableAreaRowHeaders );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::elementInserted( const ContainerEvent& i_event ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    Reference< XGridColumn > const xGridColumn( i_event.Element, UNO_QUERY_THROW );

    sal_Int32 nIndex( m_pTableModel->getColumnCount() );
    OSL_VERIFY( i_event.Accessor >>= nIndex );
    m_pTableModel->insertColumn( nIndex, xGridColumn );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::elementRemoved( const ContainerEvent& i_event ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    sal_Int32 nIndex( -1 );
    OSL_VERIFY( i_event.Accessor >>= nIndex );
    m_pTableModel->removeColumn( nIndex );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::elementReplaced( const ContainerEvent& i_event ) throw (RuntimeException)
{
    OSL_ENSURE( false, "SVTXGridControl::elementReplaced: not implemented!" );
        // at the moment, the XGridColumnModel API does not allow replacing columns
    OSL_UNUSED( i_event );
    // TODO: replace the respective column in our table model
}


//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::disposing( const EventObject& Source ) throw(RuntimeException)
{
    VCLXWindow::disposing( Source );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::selectRow( ::sal_Int32 i_rowIndex ) throw (RuntimeException, IndexOutOfBoundsException )
{
    SolarMutexGuard aGuard;

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::selectRow: no control (anymore)!" );

    if ( ( i_rowIndex < 0 ) || ( i_rowIndex >= pTable->GetRowCount() ) )
        throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

    pTable->SelectRow( i_rowIndex, true );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::selectAllRows() throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::selectAllRows: no control (anymore)!" );

    pTable->SelectAllRows( true );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::deselectRow( ::sal_Int32 i_rowIndex ) throw (RuntimeException, IndexOutOfBoundsException )
{
    SolarMutexGuard aGuard;

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::deselectRow: no control (anymore)!" );

    if ( ( i_rowIndex < 0 ) || ( i_rowIndex >= pTable->GetRowCount() ) )
        throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

    pTable->SelectRow( i_rowIndex, false );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::deselectAllRows() throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::deselectAllRows: no control (anymore)!" );

    pTable->SelectAllRows( false );
}

//----------------------------------------------------------------------------------------------------------------------
Sequence< ::sal_Int32 > SAL_CALL SVTXGridControl::getSelectedRows() throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::getSelectedRows: no control (anymore)!", Sequence< sal_Int32 >() );

    sal_Int32 selectionCount = pTable->GetSelectedRowCount();
    Sequence< sal_Int32 > selectedRows( selectionCount );
    for ( sal_Int32 i=0; i<selectionCount; ++i )
        selectedRows[i] = pTable->GetSelectedRowIndex(i);
    return selectedRows;
}

//----------------------------------------------------------------------------------------------------------------------
::sal_Bool SAL_CALL SVTXGridControl::hasSelectedRows() throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::hasSelectedRows: no control (anymore)!", sal_True );

    return pTable->GetSelectedRowCount() > 0;
}

//----------------------------------------------------------------------------------------------------------------------
::sal_Bool SAL_CALL SVTXGridControl::isRowSelected( ::sal_Int32 index ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable, "SVTXGridControl::isRowSelected: no control (anymore)!", sal_False );

    return pTable->IsRowSelected( index );
}

//----------------------------------------------------------------------------------------------------------------------
void SVTXGridControl::dispose() throw(RuntimeException)
{
    EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    m_aSelectionListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

//----------------------------------------------------------------------------------------------------------------------
void SVTXGridControl::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    SolarMutexGuard aGuard;

    Reference< XWindow > xKeepAlive( this );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
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

//----------------------------------------------------------------------------------------------------------------------
void SVTXGridControl::ImplCallItemListeners()
{
    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::ImplCallItemListeners: no control (anymore)!" );

    if ( m_aSelectionListeners.getLength() )
    {
        GridSelectionEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)this;

        sal_Int32 const nSelectedRowCount( pTable->GetSelectedRowCount() );
        aEvent.SelectedRowIndexes.realloc( nSelectedRowCount );
        for ( sal_Int32 i=0; i<nSelectedRowCount; ++i )
            aEvent.SelectedRowIndexes[i] = pTable->GetSelectedRowIndex( i );
        m_aSelectionListeners.selectionChanged( aEvent );
    }
}

//----------------------------------------------------------------------------------------------------------------------
void SVTXGridControl::impl_updateColumnsFromModel_nothrow()
{
    Reference< XGridColumnModel > const xColumnModel( m_pTableModel->getColumnModel() );
    ENSURE_OR_RETURN_VOID( xColumnModel.is(), "no model!" );
    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable != NULL, "no table!" );

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

            m_pTableModel->appendColumn( *colRef );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
