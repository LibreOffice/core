/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unocontroltablemodel.cxx,v $
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

#include "unocontroltablemodel.hxx"
#include <com/sun/star/view/SelectionType.hpp>
#include "svtools/table/gridtablerenderer.hxx"
#include "svtools/table/defaultinputhandler.hxx"
#include "svtools/table/tablecontrol.hxx"
#include <comphelper/sequence.hxx>
#include <rtl/ref.hxx>
#include <tools/debug.hxx>
#include <toolkit/helper/property.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/grid/XGridColumn.hpp>

using ::rtl::OUString;
using namespace ::svt::table;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt::grid;

using namespace ::svt::table;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::view;
using namespace ::toolkit;

class UnoControlTableColumn : public IColumnModel
    {
    private:
        ColumnID        m_nID;
        String          m_sName;
        bool            m_bIsResizable;
        TableMetrics    m_nWidth;
        TableMetrics    m_nMinWidth;
        TableMetrics    m_nMaxWidth;

    public:
        UnoControlTableColumn(Reference<XGridColumn>);

        // IColumnModel overridables
        virtual ColumnID        getID() const;
        virtual bool            setID( const ColumnID _nID );
        virtual String          getName() const;
        virtual void            setName( const String& _rName );
        virtual bool            isResizable() const;
        virtual void            setResizable( bool _bResizable );
        virtual TableMetrics    getWidth() const;
        virtual void            setWidth( TableMetrics _nWidth );
        virtual TableMetrics    getMinWidth() const;
        virtual void            setMinWidth( TableMetrics _nMinWidth );
        virtual TableMetrics    getMaxWidth() const;
        virtual void            setMaxWidth( TableMetrics _nMaxWidth );
    };

    //--------------------------------------------------------------------
    UnoControlTableColumn::UnoControlTableColumn(Reference<XGridColumn> m_xGridColumn)
        :m_nID( 0 )
        ,m_sName()
        ,m_bIsResizable( false )
        ,m_nWidth( 10 * 100 )    // 1 cm
        ,m_nMinWidth( 0 )   // no min width
        ,m_nMaxWidth( 0 )   // no max width
    {
        //m_nID = m_xGridColumn->getIdentifier();
        //m_nWidth = m_xGridColumn->getColumnWidth();
        m_sName = m_xGridColumn->getTitle();
    }

    //--------------------------------------------------------------------
    ColumnID UnoControlTableColumn::getID() const
    {
        return m_nID;
    }

    //--------------------------------------------------------------------
    bool UnoControlTableColumn::setID( const ColumnID _nID )
    {
        // TODO: conflict check

        m_nID = _nID;
        // TODO: notifications?

        return true;
    }

    //--------------------------------------------------------------------
    String UnoControlTableColumn::getName() const
    {
        return m_sName;
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setName( const String& _rName )
    {
        m_sName = _rName;
        // TODO: notifications?
    }

    //--------------------------------------------------------------------
    bool UnoControlTableColumn::isResizable() const
    {
        return m_bIsResizable;
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setResizable( bool _bResizable )
    {
        m_bIsResizable = _bResizable;
        // TODO: notifications?
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableColumn::getWidth() const
    {
        return m_nWidth;
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setWidth( TableMetrics _nWidth )
    {
        m_nWidth = _nWidth;
        // TODO: notifications?
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableColumn::getMinWidth() const
    {
        return m_nMinWidth;
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setMinWidth( TableMetrics _nMinWidth )
    {
        m_nMinWidth = _nMinWidth;
        // TODO: notifications?
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableColumn::getMaxWidth() const
    {
        return m_nMaxWidth;
    }

    //--------------------------------------------------------------------
    void UnoControlTableColumn::setMaxWidth( TableMetrics _nMaxWidth )
    {
        m_nMaxWidth = _nMaxWidth;
        // TODO: notifications?
    }

    //====================================================================
    //= DefaultTableModel_Impl
    //====================================================================
    struct UnoControlTableModel_Impl
    {
        ::std::vector< PColumnModel >   aColumns;
        TableSize                       nRowCount;
        bool                            bHasColumnHeaders;
        bool                            bHasRowHeaders;
        PTableRenderer                  pRenderer;
        PTableInputHandler              pInputHandler;
        TableMetrics                    nRowHeight;
        TableMetrics                    nColumnHeaderHeight;
        TableMetrics                    nRowHeaderWidth;
        std::vector<rtl::OUString>      aRowHeadersTitle;
        std::vector<std::vector<rtl::OUString> >    aCellContent;

        UnoControlTableModel_Impl()
            :nRowCount          ( 0         )
            ,bHasColumnHeaders  ( false     )
            ,bHasRowHeaders     ( false     )
            ,pRenderer          (           )
            ,pInputHandler      (           )
            ,nRowHeight         ( 4 * 100   )   // 40 mm
            ,nColumnHeaderHeight( 5 * 100   )   // 50 mm
            ,nRowHeaderWidth    ( 10 * 100  )    // 50 mm
            ,aRowHeadersTitle   ( 0         )
            ,aCellContent       ( 0         )
        {
        }
    };

    //====================================================================
    //= UnoControlTableModel
    //====================================================================
    //--------------------------------------------------------------------
    UnoControlTableModel::UnoControlTableModel()
        :m_pImpl( new UnoControlTableModel_Impl ),
        m_xDataModel(0),
        m_xColumnModel(0),
        m_bHasColumnHeaders(false),
        m_bHasRowHeaders(false),
        m_bVScroll(false),
        m_bHScroll(false)
    {
        m_pImpl->bHasColumnHeaders = m_bHasColumnHeaders;
        m_pImpl->bHasRowHeaders = m_bHasRowHeaders;
        m_pImpl->pRenderer.reset( new GridTableRenderer( *this ) );
        m_pImpl->pInputHandler.reset( new DefaultInputHandler );
    }

    //--------------------------------------------------------------------
    UnoControlTableModel::~UnoControlTableModel()
    {
        DELETEZ( m_pImpl );
    }

    //--------------------------------------------------------------------
    TableSize UnoControlTableModel::getColumnCount() const
    {
        m_pImpl->aColumns.resize( m_xColumnModel->getColumnCount());
        return (TableSize)m_pImpl->aColumns.size();
    }

    //--------------------------------------------------------------------
    TableSize UnoControlTableModel::getRowCount() const
    {
        return m_pImpl->nRowCount;
    }

    //--------------------------------------------------------------------
    bool UnoControlTableModel::hasColumnHeaders() const
    {
        return m_pImpl->bHasColumnHeaders;
    }

    //--------------------------------------------------------------------
    bool UnoControlTableModel::hasRowHeaders() const
    {
        return m_pImpl->bHasRowHeaders;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setRowHeaders(bool _bRowHeaders)
    {
        m_pImpl->bHasRowHeaders = _bRowHeaders;
    }
    //--------------------------------------------------------------------
    void UnoControlTableModel::setColumnHeaders(bool _bColumnHeaders)
    {
        m_pImpl->bHasColumnHeaders = _bColumnHeaders;
    }

    void UnoControlTableModel::setColumnCount(TableSize _nColCount)
    {
       m_pImpl->aColumns.resize( _nColCount);
    }
    //--------------------------------------------------------------------
    void UnoControlTableModel::setRowCount(TableSize _nRowCount)
    {
       m_pImpl->nRowCount = _nRowCount;
    }
    //--------------------------------------------------------------------
    bool UnoControlTableModel::isCellEditable( ColPos col, RowPos row ) const
    {
        (void)col;
        (void)row;
        return false;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::addTableModelListener( const PTableModelListener& listener )
    {
        (void) listener;
        //listener->onTableModelChanged(PTableModel(this));
        // TODO
        DBG_ERROR( "DefaultTableModel::addTableModelListener: not yet implemented!" );
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::removeTableModelListener( const PTableModelListener& listener )
    {
        (void)listener;
        // TODO
        DBG_ERROR( "DefaultTableModel::removeTableModelListener: not yet implemented!" );
    }

    //--------------------------------------------------------------------
    PColumnModel UnoControlTableModel::getColumnModel( ColPos column )
    {
        DBG_ASSERT( ( column >= 0 ) && ( column < getColumnCount() ),
            "DefaultTableModel::getColumnModel: invalid index!" );
        return m_pImpl->aColumns[ column ];
    }

    //--------------------------------------------------------------------
    PColumnModel UnoControlTableModel::getColumnModelByID( ColumnID id )
    {
        (void)id;
        // TODO
        DBG_ERROR( "DefaultTableModel::getColumnModelByID: not yet implemented!" );
        return PColumnModel();
    }

    //--------------------------------------------------------------------
    PTableRenderer UnoControlTableModel::getRenderer() const
    {
        return m_pImpl->pRenderer;
    }

    //--------------------------------------------------------------------
    PTableInputHandler UnoControlTableModel::getInputHandler() const
    {
        return m_pImpl->pInputHandler;
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableModel::getRowHeight() const
    {
        return m_pImpl->nRowHeight;
    }
        //--------------------------------------------------------------------
    void UnoControlTableModel::setRowHeight(TableMetrics _nRowHeight)
    {
        m_pImpl->nRowHeight = _nRowHeight;
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableModel::getColumnHeaderHeight() const
    {
        DBG_ASSERT( hasColumnHeaders(), "DefaultTableModel::getColumnHeaderHeight: invalid call!" );
        return m_pImpl->nColumnHeaderHeight;
    }

    //--------------------------------------------------------------------
    TableMetrics UnoControlTableModel::getRowHeaderWidth() const
    {
        DBG_ASSERT( hasRowHeaders(), "DefaultTableModel::getRowHeaderWidth: invalid call!" );
        return m_pImpl->nRowHeaderWidth;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::SetTitleHeight( TableMetrics _nHeight )
    {
        DBG_ASSERT( _nHeight > 0, "DefaultTableModel::SetTitleHeight: invalid height value!" );
        m_pImpl->nColumnHeaderHeight = _nHeight;
        // TODO: notification
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::SetHandleWidth( TableMetrics _nWidth )
    {
        DBG_ASSERT( _nWidth > 0, "DefaultTableModel::SetHandleWidth: invalid width value!" );
        m_pImpl->nRowHeaderWidth = _nWidth;
        // TODO: notification
    }

    //--------------------------------------------------------------------
    ScrollbarVisibility UnoControlTableModel::getVerticalScrollbarVisibility(int overAllHeight, int actHeight) const
    {
        if(overAllHeight>=actHeight && !m_bVScroll)
            return ScrollbarShowNever;
        else
            return ScrollbarShowAlways;
    }

    //--------------------------------------------------------------------
    ScrollbarVisibility UnoControlTableModel::getHorizontalScrollbarVisibility(int overAllWidth, int actWidth) const
    {
        if(overAllWidth>=actWidth && !m_bHScroll)
            return ScrollbarShowNever;
        else
            return ScrollbarShowAlways;
    }
    //--------------------------------------------------------------------
    void UnoControlTableModel::setCellContent(std::vector<std::vector<rtl::OUString> > cellContent)
    {
        if(cellContent.empty())
        {
            unsigned int i = m_pImpl->aColumns.size();
            std::vector<rtl::OUString> emptyCells;
            while(i!=0)
            {
                cellContent.push_back(emptyCells);
                --i;
            }
        }
        std::vector<rtl::OUString> cCC;
        for(::std::vector<std::vector<rtl::OUString> >::iterator iter = cellContent.begin(); iter!= cellContent.end();++iter)
        {
            cCC = *iter;
            m_pImpl->aCellContent.push_back(cCC);
        }
    }

    std::vector<std::vector<rtl::OUString> > UnoControlTableModel::getCellContent()
    {
        return m_pImpl->aCellContent;
    }

    //--------------------------------------------------------------------
    void UnoControlTableModel::setRowHeaderName(std::vector<rtl::OUString> cellColumnContent)
    {
        if(cellColumnContent.empty())
        {
            unsigned int i = m_pImpl->aColumns.size();
            while(i!=0)
            {
                cellColumnContent.push_back(rtl::OUString::createFromAscii(""));
                --i;
            }
        }
        for(::std::vector<rtl::OUString>::iterator iter = cellColumnContent.begin(); iter!= cellColumnContent.end();++iter)
        {
            rtl::OUString s = *iter;
            m_pImpl->aRowHeadersTitle.push_back(*iter);
        }
    }

    std::vector<rtl::OUString> UnoControlTableModel::getRowHeaderName()
    {
        return m_pImpl->aRowHeadersTitle;
    }

::com::sun::star::uno::Any UnoControlTableModel::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::grid::XGridControl*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::grid::XGridDataListener*, this ),
                                        //SAL_STATIC_CAST( com::sun::star::lang::XEventListener*, this ),
                                        //SAL_STATIC_CAST( com::sun::star::awt::XMouseListener*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XTypeProvider*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoControlTableModel )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridControl>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataModel>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumnModel > SAL_CALL UnoControlTableModel::getColumnModel(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return NULL;
}
void SAL_CALL UnoControlTableModel::setColumnModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridColumnModel >& model ) throw (::com::sun::star::uno::RuntimeException)
{
    (void)model;
}
::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataModel > SAL_CALL UnoControlTableModel::getDataModel(  ) throw (::com::sun::star::uno::RuntimeException)
{
    return NULL;
}
void SAL_CALL UnoControlTableModel::setDataModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridDataModel >& model ) throw (::com::sun::star::uno::RuntimeException)
{
    (void)model;
}
sal_Int32 SAL_CALL UnoControlTableModel::getItemIndexAtPoint(::sal_Int32 x, ::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException)
{
    TableControl* pTableControl = (TableControl*)GetWindow();
    return pTableControl->GetCurrentRow( Point(x,y) );
}

/*
void SAL_CALL UnoControlTableModel::addMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener > & listener ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXWindow::addMouseListener( listener );
}

void SAL_CALL UnoControlTableModel::removeMouseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseListener > & listener ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXWindow::removeMouseListener( listener );
}
*/
/*
void SAL_CALL UnoControlTableModel::mousePressed( const ::com::sun::star::awt::MouseEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    (void)rEvent;
}
void SAL_CALL UnoControlTableModel::mouseReleased( const ::com::sun::star::awt::MouseEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    (void)rEvent;
}
void SAL_CALL UnoControlTableModel::mouseEntered( const ::com::sun::star::awt::MouseEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    (void) rEvent;
}
void SAL_CALL UnoControlTableModel::mouseExited( const ::com::sun::star::awt::MouseEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    (void) rEvent;
}
*/
void SAL_CALL UnoControlTableModel::addSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
    (void)listener;
}

void SAL_CALL UnoControlTableModel::removeSelectionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::grid::XGridSelectionListener > & listener) throw (::com::sun::star::uno::RuntimeException)
{
    (void) listener;
}

void UnoControlTableModel::setProperty( const ::rtl::OUString& PropertyName, const Any& aValue) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TableControl* pTableControl = (TableControl*)GetWindow();

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
                if( pTableControl->getSelEngine()->GetSelectionMode() != eSelMode )
                    pTableControl->getSelEngine()->SetSelectionMode( eSelMode );
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
                setRowHeaders(rowHeader);
            }
            break;
        }

        case BASEPROPERTY_GRID_SHOWCOLUMNHEADER:
        {
            sal_Bool colHeader = true;
            if( aValue >>= colHeader )
            {
                setColumnHeaders(colHeader);
            }
            break;
        }
        case BASEPROPERTY_GRID_DATAMODEL:
        {
            m_xDataModel = Reference< XGridDataModel >( aValue, UNO_QUERY );
            Sequence<Sequence< ::rtl::OUString > > cellData = m_xDataModel->getData();
            Sequence<rtl::OUString> rowData(0);
            for(int i = 0; i< m_xDataModel->getRowCount();++i)
            {
                rowData = cellData[i];
                std::vector<rtl::OUString> newRow(
                    comphelper::sequenceToContainer< std::vector<rtl::OUString > >(rowData));
                if(newRow.size()<m_pImpl->aColumns.size())
                    newRow.resize(m_pImpl->aColumns.size(),rtl::OUString::createFromAscii(""));
                m_pImpl->aCellContent.push_back(newRow);
            }
            Sequence< ::rtl::OUString > rowHeaders = m_xDataModel->getRowHeaders();
            std::vector< rtl::OUString > newRow(
                comphelper::sequenceToContainer< std::vector<rtl::OUString > >(rowHeaders));
            m_pImpl->nRowCount = m_xDataModel->getRowCount();
            setRowHeaderName(newRow);
            break;
        }
        case BASEPROPERTY_GRID_COLUMNMODEL:
        {
            m_xColumnModel = Reference< XGridColumnModel >( aValue, UNO_QUERY );
            Sequence<Reference< XGridColumn > > columns = m_xColumnModel->getColumns();
            std::vector<Reference< XGridColumn > > aNewColumns(
                comphelper::sequenceToContainer<std::vector<Reference< XGridColumn > > >(columns));
            if(!m_pImpl->aColumns.empty())
                m_pImpl->aColumns.clear();
            for ( ::svt::table::ColPos col = 0; col < m_xColumnModel->getColumnCount(); ++col )
            {
                UnoControlTableColumn* tableColumn = new UnoControlTableColumn(aNewColumns[col]);
                m_pImpl->aColumns.push_back((PColumnModel)tableColumn);
            }
            break;
        }
        default:
            VCLXWindow::setProperty( PropertyName, aValue );
        break;
    }
}

Any UnoControlTableModel::getProperty( const ::rtl::OUString& PropertyName ) throw(RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    const sal_uInt16 nPropId = GetPropertyId( PropertyName );
    TableControl* pTableControl = (TableControl*)GetWindow();
    if(pTableControl)
    {
        switch(nPropId)
        {
        case BASEPROPERTY_GRID_SELECTIONMODE:
        {
            SelectionType eSelectionType;

            SelectionMode eSelMode = pTableControl->getSelEngine()->GetSelectionMode();
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
                return Any ((sal_Bool) pTableControl->GetModel()->hasRowHeaders());
            }
            case BASEPROPERTY_GRID_SHOWCOLUMNHEADER:
                return Any ((sal_Bool) pTableControl->GetModel()->hasColumnHeaders());
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

void UnoControlTableModel::ImplGetPropertyIds( std::list< sal_uInt16 > &rIds )
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
void SAL_CALL UnoControlTableModel::setVisible( sal_Bool bVisible ) throw(::com::sun::star::uno::RuntimeException)
{
    TableControl* pTable = (TableControl*)GetWindow();
    if ( pTable )
    {
        pTable->SetModel(PTableModel(this));
        pTable->Show( bVisible );
    }
}
void SAL_CALL UnoControlTableModel::setFocus() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    if ( GetWindow())
        GetWindow()->GrabFocus();
}
void SAL_CALL UnoControlTableModel::rowAdded(const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    std::vector<OUString> aNewRow(
        comphelper::sequenceToContainer< std::vector<rtl::OUString > >(Event.rowData));
    if(aNewRow.size()<m_pImpl->aColumns.size())
        aNewRow.resize(m_pImpl->aColumns.size(),rtl::OUString::createFromAscii(""));
    m_pImpl->aCellContent.push_back(aNewRow);
    if(hasRowHeaders())
        m_pImpl->aRowHeadersTitle.push_back(Event.headerName);
    m_pImpl->nRowCount=m_pImpl->aCellContent.size();
    TableControl* pTable = (TableControl*)GetWindow();
    pTable->InvalidateDataWindow(m_pImpl->nRowCount-1, false);
    //pTable->GrabFocus();
}

void SAL_CALL UnoControlTableModel::rowRemoved(const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    TableControl* pTable = (TableControl*)GetWindow();
    //unsigned int rows =m_pImpl->aCellContent.size()-1;
    if(Event.index == -1)
    {
        if(hasRowHeaders())
            m_pImpl->aRowHeadersTitle.clear();
        m_pImpl->aCellContent.clear();
    }
    else
    {
        pTable->removeSelectedRow(Event.index);
        if(m_pImpl->aCellContent.size()>1)
        {
            if(hasRowHeaders())
                m_pImpl->aRowHeadersTitle.erase(m_pImpl->aRowHeadersTitle.begin()+Event.index);
            m_pImpl->aCellContent.erase(m_pImpl->aCellContent.begin()+Event.index);

        }
        else
        {
            if(hasRowHeaders())
                m_pImpl->aRowHeadersTitle.clear();
            m_pImpl->aCellContent.clear();
            //m_pImpl->nRowCount=0;
        }
    }
    //pTable->InvalidateDataWindow(Event.index, true);
    setRowCount(m_pImpl->aCellContent.size());
    pTable->InvalidateDataWindow(Event.index, true);
    //pTable->Invalidate();
}

void SAL_CALL  UnoControlTableModel::dataChanged(const ::com::sun::star::awt::grid::GridDataEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    (void) Event;
}

 void SAL_CALL UnoControlTableModel::disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException)
 {
    VCLXWindow::disposing( Source );
 }

::sal_Int32 SAL_CALL UnoControlTableModel::getMinSelectionIndex() throw (::com::sun::star::uno::RuntimeException)
{
    return 0;
}

::sal_Int32 SAL_CALL UnoControlTableModel::getMaxSelectionIndex() throw (::com::sun::star::uno::RuntimeException)
{
    return 0;
}

void SAL_CALL UnoControlTableModel::insertIndexIntervall(::sal_Int32 start, ::sal_Int32 length) throw (::com::sun::star::uno::RuntimeException)
{
    (void)length;
    (void)start;
}

void SAL_CALL UnoControlTableModel::removeIndexIntervall(::sal_Int32 start, ::sal_Int32 end) throw (::com::sun::star::uno::RuntimeException)
{
    (void)end;
    (void)start;
}

::com::sun::star::uno::Sequence< ::sal_Int32 > SAL_CALL UnoControlTableModel::getSelection() throw (::com::sun::star::uno::RuntimeException)
{
    TableControl* pTable = (TableControl*)GetWindow();
    std::vector<RowPos> selectedRows = pTable->getSelectedRows();
    Sequence<sal_Int32> selectedRowsToSequence(comphelper::containerToSequence(selectedRows));
    return selectedRowsToSequence;
}

::sal_Bool SAL_CALL UnoControlTableModel::isCellEditable() throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

::sal_Bool SAL_CALL UnoControlTableModel::isSelectionEmpty() throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

::sal_Bool SAL_CALL UnoControlTableModel::isSelectedIndex(::sal_Int32 index) throw (::com::sun::star::uno::RuntimeException)
{
    (void)index;
    return sal_False;
}

void SAL_CALL UnoControlTableModel::selectRow(::sal_Int32 y) throw (::com::sun::star::uno::RuntimeException)
{
    (void)y;
}

void SAL_CALL UnoControlTableModel::selectColumn(::sal_Int32 x) throw (::com::sun::star::uno::RuntimeException)
{
    (void)x;
}
