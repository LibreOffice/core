/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SVTXGridControl.cxx,v $
 * $Revision: 1.32 $
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
#include <com/sun/star/view/SelectionType.hpp>
#include "svtools/table/gridtablerenderer.hxx"
#include "svtools/table/defaultinputhandler.hxx"
#include "svtools/table/tablecontrol.hxx"
#include "unocontroltablemodel.hxx"
#include <comphelper/sequence.hxx>
#include <rtl/ref.hxx>
#include <tools/debug.hxx>
#include <toolkit/helper/property.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/grid/XGridColumn.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChange.hpp>
#include <com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>


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
    m_bHScroll(false)
{
}

//--------------------------------------------------------------------
SVTXGridControl::~SVTXGridControl()
{
    DELETEZ(m_pTableModel);
}

::com::sun::star::uno::Any SVTXGridControl::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::grid::XGridControl*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::grid::XGridDataListener*, this ),
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

::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumnModel > SAL_CALL SVTXGridControl::getColumnModel(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return NULL;
}
void SAL_CALL SVTXGridControl::setColumnModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumnModel >& model ) throw (::com::sun::star::uno::RuntimeException)
{
    (void)model;
}
::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataModel > SAL_CALL SVTXGridControl::getDataModel(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return NULL;
}
void SAL_CALL SVTXGridControl::setDataModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataModel >& model ) throw (::com::sun::star::uno::RuntimeException)
{
    (void)model;
}
sal_Int32 SAL_CALL SVTXGridControl::getItemIndexAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException)
{
    TableControl* pTable = (TableControl*)GetWindow();
    return pTable->GetCurrentRow( Point(x,y) );
}

void SAL_CALL SVTXGridControl::addSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
    (void)listener;
}

void SAL_CALL SVTXGridControl::removeSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
    (void) listener;
}

void SVTXGridControl::setProperty( const ::rtl::OUString& PropertyName, const Any& aValue) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTable = (TableControl*)GetWindow();

    switch( GetPropertyId( PropertyName ) )
    {
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
            }
            break;
        }
        case BASEPROPERTY_VSCROLL:
        {
            sal_Bool bVScroll = true;
            if( aValue >>= bVScroll )
            {
                m_bVScroll = bVScroll;
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
            m_xDataModel = Reference< XGridDataModel >( aValue, UNO_QUERY );
            Sequence<Sequence< ::rtl::OUString > > cellData = m_xDataModel->getData();
            Sequence<rtl::OUString> rowData(0);
            std::vector< std::vector< rtl::OUString > > aCellContent(0);
            for(int i = 0; i< m_xDataModel->getRowCount();++i)
            {
                rowData = cellData[i];
                std::vector<rtl::OUString> newRow(
                    comphelper::sequenceToContainer< std::vector<rtl::OUString > >(rowData));
                if(newRow.size() < (unsigned)m_pTableModel->getColumnCount())
                    newRow.resize( m_pTableModel->getColumnCount(),rtl::OUString::createFromAscii(""));
                aCellContent.push_back(newRow);
            }
            m_pTableModel->setCellContent(aCellContent);
            Sequence< ::rtl::OUString > rowHeaders = m_xDataModel->getRowHeaders();
            std::vector< rtl::OUString > newRow(
                comphelper::sequenceToContainer< std::vector<rtl::OUString > >(rowHeaders));
            m_pTableModel->setRowCount(m_xDataModel->getRowCount());
            m_pTableModel->setRowHeaderName(newRow);
            break;
        }
        case BASEPROPERTY_GRID_COLUMNMODEL:
        {
            m_xColumnModel = Reference< XGridColumnModel >( aValue, UNO_QUERY );
            Sequence<Reference< XGridColumn > > columns = m_xColumnModel->getColumns();
            std::vector<Reference< XGridColumn > > aNewColumns(
                comphelper::sequenceToContainer<std::vector<Reference< XGridColumn > > >(columns));
        /*  if(m_pTable->GetColumnCount().size()>0)
                m_pTable->GetColumnName.clear();*/
            for ( ::svt::table::ColPos col = 0; col < m_xColumnModel->getColumnCount(); ++col )
            {
                UnoControlTableColumn* tableColumn = new UnoControlTableColumn(aNewColumns[col]);
                m_pTableModel->getColumnModel().push_back((PColumnModel)tableColumn);
            }
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
                     0);
    VCLXWindow::ImplGetPropertyIds( rIds, true );
}
void SAL_CALL SVTXGridControl::setVisible( sal_Bool bVisible ) throw(::com::sun::star::uno::RuntimeException)
{
    TableControl* pTable = (TableControl*)GetWindow();
    if ( pTable )
    {
        pTable->SetModel(PTableModel(m_pTableModel));
        //m_pTable->SetPosSizePixel( Point( nPosX, nPosY ), Size(nWidth, nHeight) );
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
    std::vector<OUString> aNewRow(
        comphelper::sequenceToContainer< std::vector<rtl::OUString > >(Event.rowData));
    if(aNewRow.size()< (unsigned)m_pTableModel->getColumnCount())
        aNewRow.resize(m_pTableModel->getColumnCount(),rtl::OUString::createFromAscii(""));
    m_pTableModel->getCellContent().push_back(aNewRow);
    if(m_pTableModel->hasRowHeaders())
        m_pTableModel->getRowHeaderName().push_back(Event.headerName);
    m_pTableModel->setRowCount(m_pTableModel->getRowHeaderName().size());
    TableControl* pTable = (TableControl*)GetWindow();
    pTable->InvalidateDataWindow(m_pTableModel->getRowHeaderName().size()-1, false);
    //pTable->GrabFocus();
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
    TableControl* pTable = (TableControl*)GetWindow();
    //unsigned int rows =m_pImpl->aCellContent.size()-1;
    if(Event.index == -1)
    {
        if(m_pTableModel->hasRowHeaders())
            m_pTableModel->getRowHeaderName().clear();
        m_pTableModel->getCellContent().clear();
        if(pTable->isAccessibleAlive())
        {
            pTable->commitGridControlEvent(TABLE_MODEL_CHANGED,
                 makeAny( AccessibleTableModelChange(DELETE, 0, m_pTableModel->getColumnCount(), 0, m_pTableModel->getColumnCount())),
                Any());
        }
    }
    else
    {
        pTable->removeSelectedRow(Event.index);
        if(m_pTableModel->getCellContent().size()>1)
        {
            if(m_pTableModel->hasRowHeaders())
                m_pTableModel->getRowHeaderName().erase(m_pTableModel->getRowHeaderName().begin()+Event.index);
            m_pTableModel->getCellContent().erase(m_pTableModel->getCellContent().begin()+Event.index);

        }
        else
        {
            if(m_pTableModel->hasRowHeaders())
                m_pTableModel->getRowHeaderName().clear();
            m_pTableModel->getCellContent().clear();
            //m_pImpl->nRowCount=0;
        }
    }
    //pTable->InvalidateDataWindow(Event.index, true);
    m_pTableModel->setRowCount(m_pTableModel->getCellContent().size());
    pTable->InvalidateDataWindow(Event.index, true);
    if(pTable->isAccessibleAlive())
    {
        pTable->commitGridControlEvent(TABLE_MODEL_CHANGED,
             makeAny( AccessibleTableModelChange(DELETE, Event.index, Event.index+1, 0, m_pTableModel->getColumnCount())),
            Any());
        //pTable->commitGridControlEvent(CHILD,
     //     makeAny( pTable->m_pAccessTable->m_pAccessible->getTableHeader(TCTYPE_ROWHEADERBAR)),
        //  Any());
        //for (sal_Int32 i = 0 ; i <= m_pTableModel->getColumnCount() ; ++i)
        //{
        //  pTable->commitGridControlEvent(
  //              CHILD,
        //      makeAny( pTable->m_pAccessTable->m_pAccessible->getTable() ),
        //      Any());
        //}
    }
    //pTable->Invalidate();
}

void SAL_CALL  SVTXGridControl::dataChanged(const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    (void) Event;
}

 void SAL_CALL SVTXGridControl::disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException)
 {
    VCLXWindow::disposing( Source );
 }

::sal_Int32 SAL_CALL SVTXGridControl::getMinSelectionIndex() throw (::com::sun::star::uno::RuntimeException)
{
    return 0;
}

::sal_Int32 SAL_CALL SVTXGridControl::getMaxSelectionIndex() throw (::com::sun::star::uno::RuntimeException)
{
    return 0;
}

void SAL_CALL SVTXGridControl::insertIndexIntervall(::sal_Int32 start, ::sal_Int32 length) throw (::com::sun::star::uno::RuntimeException)
{
    (void)length;
    (void)start;
}

void SAL_CALL SVTXGridControl::removeIndexIntervall(::sal_Int32 start, ::sal_Int32 end) throw (::com::sun::star::uno::RuntimeException)
{
    (void)end;
    (void)start;
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
    return sal_False;
}

::sal_Bool SAL_CALL SVTXGridControl::isSelectedIndex(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException)
{
    (void)index;
    return sal_False;
}

void SAL_CALL SVTXGridControl::selectRow(::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException)
{
    (void)y;
}

void SAL_CALL SVTXGridControl::selectColumn(::sal_Int32 x) throw (::com::sun::star::uno::RuntimeException)
{
    (void)x;
}
