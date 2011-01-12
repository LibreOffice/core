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
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::accessibility::AccessibleEventId;
using namespace ::com::sun::star::accessibility::AccessibleTableModelChangeType;
using ::com::sun::star::accessibility::AccessibleTableModelChange;


SVTXGridControl::SVTXGridControl()
    :m_pTableModel( new UnoControlTableModel() )
    ,m_xColumnModel( 0 )
    ,m_bHasColumnHeaders( false )
    ,m_bHasRowHeaders( false )
    ,m_bTableModelInitCompleted( false )
    ,m_nSelectedRowCount( 0 )
    ,m_nKnowRowCount( 0 )
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
sal_Int32 SAL_CALL SVTXGridControl::getRowAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable != NULL, "SVTXGridControl::getRowAtPoint: no control (anymore)!", -1 );

    sal_Int32 const nRow = pTable->getTableControlInterface().getRowAtPoint( Point( x, y ) );
    return ( nRow >= 0 ) ? nRow : -1;
}

// ---------------------------------------------------------------------------------------------------------------------
sal_Int32 SAL_CALL SVTXGridControl::getColumnAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable != NULL, "SVTXGridControl::getColumnAtPoint: no control (anymore)!", -1 );

    sal_Int32 const nColumn = pTable->getTableControlInterface().getColAtPoint( Point( x, y ) );
    return ( nColumn >= 0 ) ? nColumn : -1;
}

// ---------------------------------------------------------------------------------------------------------------------
sal_Int32 SAL_CALL SVTXGridControl::getCurrentColumn(  ) throw (RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable != NULL, "SVTXGridControl::getCurrentColumn: no control (anymore)!", -1 );

    sal_Int32 const nColumn = pTable->GetCurrentColumn();
    return ( nColumn >= 0 ) ? nColumn : -1;
}

// ---------------------------------------------------------------------------------------------------------------------
sal_Int32 SAL_CALL SVTXGridControl::getCurrentRow(  ) throw (RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN( pTable != NULL, "SVTXGridControl::getCurrentRow: no control (anymore)!", -1 );

    sal_Int32 const nRow = pTable->GetCurrentRow();
    return ( nRow >= 0 ) ? nRow : -1;
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::addSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
    m_aSelectionListeners.addInterface(listener);
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::removeSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
    m_aSelectionListeners.removeInterface(listener);
}

// ---------------------------------------------------------------------------------------------------------------------
void SVTXGridControl::setProperty( const ::rtl::OUString& PropertyName, const Any& aValue) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable != NULL, "SVTXGridControl::setProperty: no control (anymore)!" );

    switch( GetPropertyId( PropertyName ) )
    {
        case BASEPROPERTY_ROW_HEADER_WIDTH:
        {
            sal_Int32 rowHeaderWidth( -1 );
            aValue >>= rowHeaderWidth;
            ENSURE_OR_BREAK( rowHeaderWidth > 0, "SVTXGridControl::setProperty: illegal row header width!" );
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
            ENSURE_OR_BREAK( columnHeaderHeight > 0, "SVTXGridControl::setProperty: illegal column header height!" );
            m_pTableModel->setColumnHeaderHeight( columnHeaderHeight );
            // TODO: the model should broadcast this change itself, and the table should invalidate itself as needed
            pTable->Invalidate();
        }
        break;

        case BASEPROPERTY_USE_GRID_LINES:
        {
            GridTableRenderer* pGridRenderer = dynamic_cast< GridTableRenderer* >(
                m_pTableModel->getRenderer().get() );
            ENSURE_OR_BREAK( pGridRenderer != NULL, "SVTXGridControl::setProperty(UseGridLines): invalid renderer!" );
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
            ENSURE_OR_BREAK( rowHeight > 0, "SVTXGridControl::setProperty: illegal row height!" );
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
                throw GridInvalidDataException( ::rtl::OUString::createFromAscii("The data model isn't set!"), *this );

            m_pTableModel->setDataModel( xDataModel );
            impl_checkTableModelInit();

            // ensure default columns exist, if they have not previously been added
            sal_Int32 const nDataColumnCount = xDataModel->getColumnCount();
            if ( ( nDataColumnCount > 0 ) && ( m_xColumnModel->getColumnCount() == 0 ) )
                m_xColumnModel->setDefaultColumns( nDataColumnCount );
                // this will trigger notifications, which in turn will let us update our m_pTableModel
        }
        break;

        case BASEPROPERTY_GRID_COLUMNMODEL:
        {
            // remove all old columns
            m_pTableModel->removeAllColumns();

            // obtain new col model
            m_xColumnModel = Reference< XGridColumnModel >( aValue, UNO_QUERY_THROW );

            // announce to the TableModel
            m_pTableModel->setColumnModel( m_xColumnModel );

            // announce the table model to the table control, if we have everything which is needed
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
            m_nKnowRowCount = m_pTableModel->getRowCount();
            m_bTableModelInitCompleted = true;
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
    ::vos::OGuard aGuard( GetMutex() );

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
        aPropertyValue <<= m_xColumnModel;
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
        ENSURE_OR_BREAK( pGridRenderer != NULL, "SVTXGridControl::getProperty(UseGridLines): invalid renderer!" );
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
            Sequence< ::com::sun::star::util::Color > aAPIColors( aColors->size() );
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

void SVTXGridControl::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
{
    PushPropertyIds( rIds,
        BASEPROPERTY_GRID_SHOWROWHEADER,
        BASEPROPERTY_GRID_SHOWCOLUMNHEADER,
        BASEPROPERTY_GRID_DATAMODEL,
        BASEPROPERTY_GRID_COLUMNMODEL,
        BASEPROPERTY_GRID_SELECTIONMODE,
        BASEPROPERTY_GRID_HEADER_BACKGROUND,
        BASEPROPERTY_GRID_HEADER_TEXT_COLOR,
        BASEPROPERTY_GRID_LINE_COLOR,
        BASEPROPERTY_GRID_ROW_BACKGROUND_COLORS,
        0
    );
    VCLXWindow::ImplGetPropertyIds( rIds, true );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::rowsAdded( const GridDataEvent& i_event ) throw (RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int32 const affectedRows = i_event.Rows.getLength();
    ENSURE_OR_RETURN_VOID( affectedRows > 0, "SVTXGridControl::rowsAdded: invalid row count!" );

    Reference< XGridDataModel > const xDataModel( m_pTableModel->getDataModel(), UNO_QUERY_THROW );
    sal_Int32 const affectedColumns = i_event.Columns.getLength() ? i_event.Columns.getLength() : xDataModel->getColumnCount();
    ENSURE_OR_RETURN_VOID( affectedColumns > 0, "SVTXGridControl::rowsAdded: no columns at all?" );
    TableSize const columnCount = m_xColumnModel->getColumnCount();
    if ( columnCount == 0 )
    {
        m_xColumnModel->setDefaultColumns( affectedColumns );
            // this will trigger notifications, which in turn will let us update our m_pTableModel
    }

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::rowsAdded: no control (anymore)!" );

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

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::rowsRemoved( const GridDataEvent& i_event ) throw (RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::rowsRemoved: no control (anymore)!" );

    if ( i_event.Rows.getLength() == 0 )
    {
        if ( !isSelectionEmpty() )
            deselectAllRows();

        pTable->clearSelection();

        if ( pTable->isAccessibleAlive() )
        {
            pTable->commitGridControlEvent(
                TABLE_MODEL_CHANGED,
                 makeAny( AccessibleTableModelChange( DELETE, 0, m_pTableModel->getColumnCount(), 0, m_pTableModel->getColumnCount() ) ),
                Any()
            );
        }
    }
    else
    {
    #if OSL_DEBUG_LEVEL > 0
        for ( sal_Int32 row = 0; row < i_event.Rows.getLength() - 1; ++row )
        {
            OSL_ENSURE( i_event.Rows[row] < i_event.Rows[row], "SVTXGridControl::rowsRemoved: row indexes not sorted!" );
        }
    #endif
        sal_Int32 removedRowCount = 0;
        for ( sal_Int32 row = 0; row < i_event.Rows.getLength(); ++row )
        {
            sal_Int32 const rowIndex = i_event.Rows[row];
            ENSURE_OR_CONTINUE( ( rowIndex >= 0 ) && ( rowIndex < m_nKnowRowCount ),
                "SVTXGridControl::rowsRemoved: illegal row index!" );

            if ( isSelectedIndex( rowIndex ) )
            {
                Sequence< sal_Int32 > selected(1);
                selected[0] = rowIndex;
                deselectRows( selected );
            }

            if ( pTable->isAccessibleAlive() )
            {
                pTable->commitGridControlEvent(TABLE_MODEL_CHANGED,
                     makeAny( AccessibleTableModelChange(
                        DELETE, rowIndex - removedRowCount, rowIndex - removedRowCount + 1, 0, m_pTableModel->getColumnCount()
                        // the adjustment via removedRowCount is necessary here, since with every removed row, the
                        // *actual* index of all subsequent rows changes, but i_event.Rows still contains the original row indexes
                    ) ),
                    Any()
                );
            }
            ++removedRowCount;
        }

        // TODO: I don't think that the selected rows of the TableControl properly survive this - they might contain
        // too large indexes now.
        // Really, the ITableModel should broadcast the "rowsRemoved" event to its listeners, and the TableControl/_Impl
        // should do all necessary adjustments, including for its selection, itself.
    }

    m_nKnowRowCount = m_pTableModel->getRowCount();
    pTable->InvalidateDataWindow( 0, m_nKnowRowCount, true );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::dataChanged( const GridDataEvent& i_event ) throw (RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::dataChanged: no control (anymore)!" );

    // TODO: Our UnoControlTableModel should be a listener at the data model, and multiplex those events,
    // so the TableControl/_Impl can react on it.
    if ( i_event.Rows.getLength() == 0 )
        pTable->InvalidateDataWindow( 0, m_pTableModel->getRowCount(), false );
    else
        pTable->InvalidateDataWindow( i_event.Rows[0], i_event.Rows[ i_event.Rows.getLength() - 1 ], false );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::rowTitleChanged( const GridDataEvent& Event ) throw (RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = dynamic_cast< TableControl* >( GetWindow() );
    ENSURE_OR_RETURN_VOID( pTable, "SVTXGridControl::rowTitleChanged: no control (anymore)!" );

    // TODO: we could do better than this - invalidate the header area only
    pTable->Invalidate();
}


//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::elementInserted( const ContainerEvent& i_event ) throw (RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Reference< XGridColumn > const xGridColumn( i_event.Element, UNO_QUERY_THROW );

    sal_Int32 nIndex( m_pTableModel->getColumnCount() );
    OSL_VERIFY( i_event.Accessor >>= nIndex );
    m_pTableModel->insertColumn( nIndex, xGridColumn );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::elementRemoved( const ContainerEvent& i_event ) throw (RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int32 nIndex( -1 );
    OSL_VERIFY( i_event.Accessor >>= nIndex );
    m_pTableModel->removeColumn( nIndex );
}

//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::elementReplaced( const ContainerEvent& i_event ) throw (RuntimeException)
{
    OSL_ENSURE( false, "SVTXGridControl::elementReplaced: not implemented!" );
        // at the moment, the XGridColumnModel API does not allow replacing columns
    // TODO: replace the respective column in our table model
}


//----------------------------------------------------------------------------------------------------------------------
void SAL_CALL SVTXGridControl::disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXWindow::disposing( Source );
}

//----------------------------------------------------------------------------------------------------------------------
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

::sal_Bool SAL_CALL SVTXGridControl::isSelectedIndex( ::sal_Int32 index ) throw (::com::sun::star::uno::RuntimeException)
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
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

