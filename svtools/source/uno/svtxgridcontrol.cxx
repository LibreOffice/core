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
#include <tools/debug.hxx>
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

using ::rtl::OUString;
using namespace ::svt::table;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt::grid;
using namespace ::com::sun::star::view;
using namespace ::toolkit;
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

//--------------------------------------------------------------------
SVTXGridControl::~SVTXGridControl()
{
}

::com::sun::star::uno::Any SVTXGridControl::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                SAL_STATIC_CAST( ::com::sun::star::awt::grid::XGridControl*, this ),
                                SAL_STATIC_CAST( ::com::sun::star::awt::grid::XGridDataListener*, this ),
                                SAL_STATIC_CAST( ::com::sun::star::awt::grid::XGridColumnListener*, this ),
                                SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( SVTXGridControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridControl>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataModel>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

sal_Int32 SAL_CALL SVTXGridControl::getItemIndexAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException)
{
    TableControl* pTable = (TableControl*)GetWindow();
    return pTable->GetCurrentRow( Point(x,y) );
}

void SAL_CALL SVTXGridControl::setToolTip(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& text, const com::sun::star::uno::Sequence< sal_Int32 >& columns) throw (::com::sun::star::uno::RuntimeException)
{
    TableControl* pTable = (TableControl*)GetWindow();
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

    TableControl* pTable = (TableControl*)GetWindow();
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
    //          case SelectionType_NONE:
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
                m_pTableModel->setHorizontalScrollbarVisibility(bHScroll);
            }
            break;
        }
        case BASEPROPERTY_VSCROLL:
        {
            sal_Bool bVScroll = true;
            if( aValue >>= bVScroll )
            {
                m_bVScroll = bVScroll;
                m_pTableModel->setVerticalScrollbarVisibility(bVScroll);
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
            sal_Int32 colorHeader = 0xFFFFFF;
            if( aValue >>= colorHeader )
            {
                m_pTableModel->setHeaderBackgroundColor(colorHeader);
            }
            break;
        }
        case BASEPROPERTY_GRID_LINE_COLOR:
        {
            sal_Int32 colorLine = 0xFFFFFF;
            if( aValue >>= colorLine )
            {
                m_pTableModel->setLineColor(colorLine);
            }
            break;
        }
        case BASEPROPERTY_GRID_EVEN_ROW_BACKGROUND:
        {
            sal_Int32 colorEvenRow = 0xFFFFFF;
            if( aValue >>= colorEvenRow )
            {
                m_pTableModel->setEvenRowBackgroundColor(colorEvenRow);
            }
            break;
        }
        case BASEPROPERTY_GRID_ROW_BACKGROUND:
        {
            sal_Int32 colorBackground = 0xFFFFFF;
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
                if(m_xDataModel != NULL)
                {
                    Sequence<Sequence< Any > > cellData = m_xDataModel->getData();
                    if(cellData.getLength()!= 0)
                    {
                        for (int i = 0; i < cellData.getLength(); i++)
                        {
                            std::vector< Any > newRow;
                            Sequence< Any > rawRowData = cellData[i];
                            //check whether the data row vector length matches with the column count
                            if(m_xColumnModel->getColumnCount() == 0)
                            {
                                for ( ::svt::table::ColPos col = 0; col < rawRowData.getLength(); ++col )
                                {
                                    UnoControlTableColumn* tableColumn = new UnoControlTableColumn();
                                    m_pTableModel->getColumnModel().push_back((PColumnModel)tableColumn);
                                }
                                m_xColumnModel->setDefaultColumns(rawRowData.getLength());
                            }
                            else
                                if((unsigned int)rawRowData.getLength()!=(unsigned)m_pTableModel->getColumnCount())
                                    throw GridInvalidDataException(rtl::OUString::createFromAscii("The column count doesn't match with the length of row data"), m_xDataModel);

                            for ( int k = 0; k < rawRowData.getLength(); k++)
                            {
                                newRow.push_back(rawRowData[k]);
                            }
                            m_pTableModel->getCellContent().push_back(newRow);
                        }

                        Sequence< ::rtl::OUString > rowHeaders = m_xDataModel->getRowHeaders();
                        std::vector< rtl::OUString > newRow(
                            comphelper::sequenceToContainer< std::vector<rtl::OUString > >(rowHeaders));
                        m_pTableModel->setRowCount(m_xDataModel->getRowCount());
                        m_pTableModel->setRowHeaderName(newRow);
                    }
                }
                else
                    throw GridInvalidDataException(rtl::OUString::createFromAscii("The data model isn't set!"), m_xDataModel);
                sal_Int32 fontHeight = pTable->PixelToLogic( Size( 0, pTable->GetTextHeight()+3 ), MAP_APPFONT ).Height();
                if(m_xDataModel->getRowHeight() == 0)
                {
                    m_pTableModel->setRowHeight(fontHeight);
                    m_xDataModel->setRowHeight(fontHeight);
                }
                else
                    m_pTableModel->setRowHeight(m_xDataModel->getRowHeight());
                m_pTableModel->setRowHeaderWidth(m_xDataModel->getRowHeaderWidth());
            }
            break;
        }
        case BASEPROPERTY_GRID_COLUMNMODEL:
        {
            m_xColumnModel = Reference< XGridColumnModel >( aValue, UNO_QUERY );
            if(m_xColumnModel != NULL)
            {
                if(m_xColumnModel->getColumnCount() != 0)
                {
                    Sequence<Reference< XGridColumn > > columns = m_xColumnModel->getColumns();
                    std::vector<Reference< XGridColumn > > aNewColumns(
                        comphelper::sequenceToContainer<std::vector<Reference< XGridColumn > > >(columns));
                    sal_Int32 fontHeight = pTable->PixelToLogic( Size( 0, pTable->GetTextHeight()+3 ), MAP_APPFONT ).Height();
                    if(m_xColumnModel->getColumnHeaderHeight() == 0)
                    {
                        m_pTableModel->setColumnHeaderHeight(fontHeight);
                        m_xColumnModel->setColumnHeaderHeight(fontHeight);
                    }
                    else
                        m_pTableModel->setColumnHeaderHeight(m_xColumnModel->getColumnHeaderHeight());
                    for ( ::svt::table::ColPos col = 0; col < m_xColumnModel->getColumnCount(); ++col )
                    {
                        UnoControlTableColumn* tableColumn = new UnoControlTableColumn(aNewColumns[col]);
                        Reference< XGridColumn > xGridColumn = m_xColumnModel->getColumn(col);
                        m_pTableModel->getColumnModel().push_back((PColumnModel)tableColumn);
                        tableColumn->setHorizontalAlign(xGridColumn->getHorizontalAlign());
                        tableColumn->setWidth(xGridColumn->getColumnWidth());
                        if(xGridColumn->getPreferredWidth() != 0)
                            tableColumn->setPreferredWidth(xGridColumn->getPreferredWidth());
                        if(xGridColumn->getMaxWidth() != 0)
                            tableColumn->setMaxWidth(xGridColumn->getMaxWidth());
                        if(xGridColumn->getMinWidth() != 0)
                            tableColumn->setMinWidth(xGridColumn->getMinWidth());
                        tableColumn->setResizable(xGridColumn->getResizeable());
                    }
                }
            }
            else
                throw GridInvalidModelException(rtl::OUString::createFromAscii("The column model isn't set!"), m_xColumnModel);

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

    const sal_uInt16 nPropId = GetPropertyId( PropertyName );
    TableControl* pTable = (TableControl*)GetWindow();
    if(pTable)
    {
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
//              case NO_SELECTION:
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
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds, true );
}
void SAL_CALL SVTXGridControl::setVisible( sal_Bool bVisible ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    TableControl* pTable = (TableControl*)GetWindow();
    if ( pTable )
    {
        pTable->SetModel(PTableModel(m_pTableModel));
        pTable->Show( bVisible );
    }
}
void SAL_CALL SVTXGridControl::setFocus() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    if ( GetWindow())
        GetWindow()->GrabFocus();
}
void SAL_CALL SVTXGridControl::rowAdded(const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    std::vector< Any > newRow;
    Sequence< Any > rawRowData = Event.rowData;
    int colCount = m_xColumnModel->getColumnCount();
    if(colCount == 0)
    {
        Reference<XGridColumnListener> listener(*this,UNO_QUERY_THROW);
        m_xColumnModel->setDefaultColumns(rawRowData.getLength());
        for ( ::svt::table::ColPos col = 0; col < rawRowData.getLength(); ++col )
        {
            UnoControlTableColumn* tableColumn = new UnoControlTableColumn();
            m_pTableModel->getColumnModel().push_back((PColumnModel)tableColumn);
            m_xColumnModel->getColumn(col)->addColumnListener(listener);
        }

    }
    else if((unsigned int)rawRowData.getLength()!=(unsigned)colCount)
        throw GridInvalidDataException(rtl::OUString::createFromAscii("The column count doesn't match with the length of row data"), m_xDataModel);

    for ( int k = 0; k < rawRowData.getLength(); k++)
        newRow.push_back(rawRowData[k]);
    m_pTableModel->getCellContent().push_back(newRow);
    if(m_pTableModel->hasRowHeaders())
        m_pTableModel->getRowHeaderName().push_back(Event.headerName);
    m_pTableModel->setRowCount(m_pTableModel->getCellContent().size());
    TableControl* pTable = (TableControl*)GetWindow();
    pTable->InvalidateDataWindow(m_pTableModel->getCellContent().size()-1, 0, false);
    if(pTable->isAccessibleAlive())
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

    TableControl* pTable = (TableControl*)GetWindow();
    if(Event.index == -1)
    {
        if(!isSelectionEmpty())
            deselectAllRows();
        if(m_pTableModel->hasRowHeaders())
            m_pTableModel->getRowHeaderName().clear();
        pTable->clearSelection();
        m_pTableModel->getCellContent().clear();
        if(pTable->isAccessibleAlive())
        {
            pTable->commitGridControlEvent(TABLE_MODEL_CHANGED,
                 makeAny( AccessibleTableModelChange(DELETE, 0, m_pTableModel->getColumnCount(), 0, m_pTableModel->getColumnCount())),
                Any());
        }
    }
    else if(Event.index >= 0 && Event.index < m_pTableModel->getRowCount())
    {
        if(isSelectedIndex(Event.index))
        {
            Sequence<sal_Int32> selected(1);
            selected[0]=Event.index;
            deselectRows(selected);
        }
        if(m_pTableModel->hasRowHeaders())
            m_pTableModel->getRowHeaderName().erase(m_pTableModel->getRowHeaderName().begin()+Event.index);
        std::vector<std::vector<Any> >::iterator rowPos =m_pTableModel->getCellContent().begin() + Event.index;
        m_pTableModel->getCellContent().erase( rowPos );
    }
    m_pTableModel->setRowCount(m_pTableModel->getCellContent().size());
    pTable->InvalidateDataWindow(Event.index, Event.index, true);
    if(pTable->isAccessibleAlive())
    {
        pTable->commitGridControlEvent(TABLE_MODEL_CHANGED,
             makeAny( AccessibleTableModelChange(DELETE, Event.index, Event.index+1, 0, m_pTableModel->getColumnCount())),
            Any());
    }
}

void SAL_CALL  SVTXGridControl::columnChanged(const ::com::sun::star::awt::grid::GridColumnEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = (TableControl*)GetWindow();
    if(Event.valueName == rtl::OUString::createFromAscii("ColumnResize"))
    {
        bool resizable = m_pTableModel->getColumnModel()[Event.index]->isResizable();
        Event.newValue>>=resizable;
        m_pTableModel->getColumnModel()[Event.index]->setResizable(resizable);
    }
    else if(Event.valueName == rtl::OUString::createFromAscii("ColWidth"))
    {
        sal_Int32 colWidth = m_pTableModel->getColumnModel()[Event.index]->getWidth();
        Event.newValue>>=colWidth;
        m_pTableModel->getColumnModel()[Event.index]->setWidth(colWidth);
    }
    else if(Event.valueName == rtl::OUString::createFromAscii("MaxWidth"))
    {
        sal_Int32 maxWidth = m_pTableModel->getColumnModel()[Event.index]->getMaxWidth();
        Event.newValue>>=maxWidth;
        m_pTableModel->getColumnModel()[Event.index]->setMaxWidth(maxWidth);
    }
    else if(Event.valueName == rtl::OUString::createFromAscii("MinWidth"))
    {
        sal_Int32 minWidth = m_pTableModel->getColumnModel()[Event.index]->getMinWidth();
        Event.newValue>>=minWidth;
        m_pTableModel->getColumnModel()[Event.index]->setMinWidth(minWidth);
    }
    else if(Event.valueName == rtl::OUString::createFromAscii("PrefWidth"))
    {
        sal_Int32 prefWidth = m_pTableModel->getColumnModel()[Event.index]->getPreferredWidth();
        Event.newValue>>=prefWidth;
        m_pTableModel->getColumnModel()[Event.index]->setPreferredWidth(prefWidth);
    }
    else if(Event.valueName == rtl::OUString::createFromAscii("HAlign"))
    {
        ::com::sun::star::style::HorizontalAlignment hAlign = m_pTableModel->getColumnModel()[Event.index]->getHorizontalAlign();
        Event.newValue>>=hAlign;
        m_pTableModel->getColumnModel()[Event.index]->setHorizontalAlign(hAlign);
    }
    else if(Event.valueName == rtl::OUString::createFromAscii("UpdateWidth"))
    {
        if(m_pTableModel->getColumnModel()[Event.index]->getPreferredWidth() != 0)
            m_xColumnModel->getColumn(Event.index)->updateColumn(rtl::OUString::createFromAscii("PrefWidth"), m_pTableModel->getColumnModel()[Event.index]->getPreferredWidth());
        m_xColumnModel->getColumn(Event.index)->updateColumn(rtl::OUString::createFromAscii("ColWidth"), m_pTableModel->getColumnModel()[Event.index]->getWidth());
    }
    pTable->Invalidate();
}
void SAL_CALL  SVTXGridControl::dataChanged(const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = (TableControl*)GetWindow();
    if(Event.valueName == rtl::OUString::createFromAscii("RowHeight"))
    {
        sal_Int32 rowHeight = m_pTableModel->getRowHeight();
        Event.newValue>>=rowHeight;
        m_pTableModel->setRowHeight(rowHeight);
        pTable->Invalidate();
    }
    else if(Event.valueName == rtl::OUString::createFromAscii("RowHeaderWidth"))
    {
        sal_Int32 rowHeaderWidth = m_pTableModel->getRowHeaderWidth();
        Event.newValue>>=rowHeaderWidth;
        m_pTableModel->setRowHeaderWidth(rowHeaderWidth);
        pTable->Invalidate();
    }
    else if(Event.valueName == rtl::OUString::createFromAscii("RowHeaders"))
    {
        Sequence< rtl::OUString > headers(0);
        Event.newValue>>=headers;
        std::vector< rtl::OUString > headerNames( comphelper::sequenceToContainer <std::vector< rtl::OUString > >(headers) );
        m_pTableModel->setRowHeaderName(headerNames);
        pTable->Invalidate();
    }
    else if(Event.valueName == rtl::OUString::createFromAscii("CellUpdated"))
    {
        std::vector< std::vector< Any > >& rowContent = m_pTableModel->getCellContent();
        sal_Int32 col = -1;
        Event.oldValue>>=col;
        rowContent[Event.index][col] = Event.newValue;
        pTable->InvalidateDataWindow(Event.index, Event.index, false);
    }
    else if(Event.valueName == rtl::OUString::createFromAscii("RowUpdated"))
    {
        std::vector<std::vector< Any > >& rowContent = m_pTableModel->getCellContent();
        Sequence< sal_Int32 > cols(0);
        Sequence< Any > values(0);
        Event.oldValue>>=cols;
        Event.newValue>>=values;
        for(int i = 0; i< cols.getLength(); i++)
        {
            if(cols[i]>=0 && cols[i]<m_pTableModel->getColumnCount())
                rowContent[Event.index][cols[i]]=values[i];
            else
                break;
        }
        pTable->InvalidateDataWindow(Event.index, Event.index, false);
    }
}

void SAL_CALL SVTXGridControl::disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXWindow::disposing( Source );
}

::sal_Int32 SAL_CALL SVTXGridControl::getMinSelectionIndex() throw (::com::sun::star::uno::RuntimeException)
{
    TableControl* pTable = (TableControl*)GetWindow();
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
    TableControl* pTable = (TableControl*)GetWindow();
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
    TableControl* pTable = (TableControl*)GetWindow();
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
    TableControl* pTable = (TableControl*)GetWindow();
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
    TableControl* pTable = (TableControl*)GetWindow();
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
    TableControl* pTable = (TableControl*)GetWindow();
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
    TableControl* pTable = (TableControl*)GetWindow();
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
    TableControl* pTable = (TableControl*)GetWindow();
    std::vector<RowPos>& selectedRows = pTable->GetSelectedRows();
    if(selectedRows.empty())
        return sal_True;
    else
        return sal_False;
}

::sal_Bool SAL_CALL SVTXGridControl::isSelectedIndex(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException)
{
    TableControl* pTable = (TableControl*)GetWindow();
    std::vector<RowPos>& selectedRows = pTable->GetSelectedRows();
    return std::find(selectedRows.begin(),selectedRows.end(), index) != selectedRows.end();
}

void SAL_CALL SVTXGridControl::selectRow(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException)
{
    if(index<0 || index>=m_pTableModel->getRowCount())
        return;
    TableControl* pTable = (TableControl*)GetWindow();
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
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    m_aSelectionListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

void SVTXGridControl::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xKeepAlive( this );

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_TABLEROW_SELECT:
        {
            TableControl* pTable = (TableControl*)GetWindow();

            if( pTable )
            {
                if ( m_aSelectionListeners.getLength() )
                {
                    ImplCallItemListeners();
                }
            }
        }
        break;

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

void SVTXGridControl::ImplCallItemListeners()
{
    TableControl* pTable = (TableControl*) GetWindow();
    if ( pTable && m_aSelectionListeners.getLength() )
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
