/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#include "svtools/table/tablecontrol.hxx"
#include "svtools/table/defaultinputhandler.hxx"
#include "svtools/table/tablemodel.hxx"
#include "tablecontrol_impl.hxx"
#include "tablegeometry.hxx"
#include "svtools/table/tabledatawindow.hxx"
#include <com/sun/star/awt/XControl.hpp>
#include <vcl/scrbar.hxx>
#include <vcl/seleng.hxx>
#include <rtl/ref.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/image.hxx>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <functional>
#include <stdlib.h>

//........................................................................
namespace svt { namespace table
{
//........................................................................

    //====================================================================
    //= TempHideCursor
    //====================================================================
    class TempHideCursor
    {
    private:
        IAbstractTableControl&  m_rTable;

    public:
        TempHideCursor( IAbstractTableControl& _rTable )
            :m_rTable( _rTable )
        {
            m_rTable.hideCursor();
        }
        ~TempHideCursor()
        {
            m_rTable.showCursor();
        }
    };

    //====================================================================
    //= EmptyTableModel
    //====================================================================
    /** default implementation of an ->ITableModel, used as fallback when no
        real model is present

        Instances of this class are static in any way, and provide the least
        necessary default functionality for a table model.
    */
    class EmptyTableModel : public ITableModel
    {
    public:
        EmptyTableModel()
        {
        }

        // ITableModel overridables
        virtual TableSize           getColumnCount() const
        {
            return 0;
        }
        virtual TableSize           getRowCount() const
        {
            return 0;
        }
        virtual bool                hasColumnHeaders() const
        {
            return false;
        }
        virtual bool                hasRowHeaders() const
        {
            return false;
        }
    virtual void                setRowHeaders(bool _bRowHeaders)
        {
            (void)_bRowHeaders;
        }
    virtual void                setColumnHeaders(bool _bColumnHeaders)
        {
        (void)_bColumnHeaders;
        }
    void setColumnCount(TableSize _nColCount)
    {
        (void) _nColCount;
    }
    void setRowCount(TableSize _nRowCount)
    {
        (void)_nRowCount;
    }
        virtual bool                isCellEditable( ColPos col, RowPos row ) const
        {
            (void)col;
            (void)row;
            return false;
        }
        virtual void                addTableModelListener( const PTableModelListener& listener )
        {
            (void)listener;
            // ignore
        }
        virtual void                removeTableModelListener( const PTableModelListener& listener )
        {
            (void)listener;
            // ignore
        }
        virtual PColumnModel        getColumnModel( ColPos column )
        {
            OSL_FAIL( "EmptyTableModel::getColumnModel: invalid call!" );
            (void)column;
            return PColumnModel();
        }
        virtual PColumnModel        getColumnModelByID( ColumnID id )
        {
            OSL_FAIL( "EmptyTableModel::getColumnModel: invalid call!" );
            (void)id;
            return PColumnModel();
        }
        virtual PTableRenderer      getRenderer() const
        {
            return PTableRenderer();
        }
        virtual PTableInputHandler  getInputHandler() const
        {
            return PTableInputHandler();
        }
        virtual TableMetrics        getRowHeight() const
        {
            return 5 * 100;
        }
        virtual void setRowHeight(TableMetrics _nRowHeight)
        {
            (void)_nRowHeight;
        }
        virtual TableMetrics        getColumnHeaderHeight() const
        {
            return 0;
        }
        virtual TableMetrics        getRowHeaderWidth() const
        {
            return 0;
        }
    virtual ScrollbarVisibility getVerticalScrollbarVisibility(int , int ) const
        {
        return ScrollbarShowNever;
        }
        virtual ScrollbarVisibility getHorizontalScrollbarVisibility(int , int ) const
        {
        return ScrollbarShowNever;
        }
    virtual bool hasVerticalScrollbar()
    {
        return false;
    }
    virtual bool hasHorizontalScrollbar()
    {
        return false;
    }
    virtual void setCellContent(const std::vector<std::vector< ::com::sun::star::uno::Any > >& )
    {
    }
    virtual ::com::sun::star::util::Color getLineColor()
    {
        return 0;
    }
    virtual void setLineColor(::com::sun::star::util::Color )
    {
    }
    virtual ::com::sun::star::util::Color getHeaderBackgroundColor()
    {
        return -1;
    }
    virtual void setHeaderBackgroundColor(::com::sun::star::util::Color )
    {
    }
    virtual ::com::sun::star::util::Color getTextColor()
    {
        return 0;
    }
    virtual void setTextColor(::com::sun::star::util::Color )
    {
    }
    virtual ::com::sun::star::util::Color getOddRowBackgroundColor()
    {
        return -1;
    }
    virtual void setOddRowBackgroundColor(::com::sun::star::util::Color )
    {
    }
    virtual ::com::sun::star::style::VerticalAlignment getVerticalAlign()
    {
        return com::sun::star::style::VerticalAlignment(0);
    }
    virtual void setVerticalAlign(com::sun::star::style::VerticalAlignment )
    {
    }
    virtual ::com::sun::star::util::Color getEvenRowBackgroundColor()
    {
        return -1;
    }
    virtual void setEvenRowBackgroundColor(::com::sun::star::util::Color )
    {
    }
    virtual std::vector<std::vector< ::com::sun::star::uno::Any > >& getCellContent()
    {
        return m_aCellContent;
    }
    virtual void setRowHeaderName(const std::vector<rtl::OUString>& )
    {
    }
    virtual std::vector<rtl::OUString>& getRowHeaderName()
    {
        aRowHeaderNames.clear();
        aRowHeaderNames.push_back( rtl::OUString() );
        return aRowHeaderNames;
    }
    private:
        std::vector<rtl::OUString> aRowHeaderNames;
        std::vector<std::vector< ::com::sun::star::uno::Any > > m_aCellContent;
    };


    //====================================================================
    //= TableControl_Impl
    //====================================================================
    DBG_NAME( TableControl_Impl )

#if DBG_UTIL
    //====================================================================
    //= SuspendInvariants
    //====================================================================
    class SuspendInvariants
    {
    private:
        const TableControl_Impl&    m_rTable;
        sal_Int32                   m_nSuspendFlags;

    public:
        SuspendInvariants( const TableControl_Impl& _rTable, sal_Int32 _nSuspendFlags )
            :m_rTable( _rTable )
            ,m_nSuspendFlags( _nSuspendFlags )
        {
            //DBG_ASSERT( ( m_rTable.m_nRequiredInvariants & m_nSuspendFlags ) == m_nSuspendFlags,
            //    "SuspendInvariants: cannot suspend what is already suspended!" );
            const_cast< TableControl_Impl& >( m_rTable ).m_nRequiredInvariants &= ~m_nSuspendFlags;
        }
        ~SuspendInvariants()
        {
            const_cast< TableControl_Impl& >( m_rTable ).m_nRequiredInvariants |= m_nSuspendFlags;
        }
    };
    #define DBG_SUSPEND_INV( flags ) \
        SuspendInvariants aSuspendInv( *this, flags );
#else
    #define DBG_SUSPEND_INV( flags )
#endif

#if DBG_UTIL
    //====================================================================
    const char* TableControl_Impl_checkInvariants( const void* _pInstance )
    {
        return static_cast< const TableControl_Impl* >( _pInstance )->impl_checkInvariants();
    }

    namespace
    {
        template< typename SCALAR_TYPE >
        bool lcl_checkLimitsExclusive( SCALAR_TYPE _nValue, SCALAR_TYPE _nMin, SCALAR_TYPE _nMax )
        {
            return ( _nValue > _nMin ) && ( _nValue < _nMax );
        }

        template< typename SCALAR_TYPE >
        bool lcl_checkLimitsExclusive_OrDefault_OrFallback( SCALAR_TYPE _nValue, SCALAR_TYPE _nMin, SCALAR_TYPE _nMax,
            PTableModel _pModel, SCALAR_TYPE _nDefaultOrFallback )
        {
            if ( !_pModel )
                return _nValue == _nDefaultOrFallback;
            if ( _nMax <= _nMin )
                return _nDefaultOrFallback == _nValue;
            return lcl_checkLimitsExclusive( _nValue, _nMin, _nMax );
        }
    }

    //--------------------------------------------------------------------
    const sal_Char* TableControl_Impl::impl_checkInvariants() const
    {
        if ( !m_pModel )
            return "no model, not even an EmptyTableModel";

        if ( !m_pDataWindow )
            return "invalid data window!";

        if ( m_pModel->getColumnCount() != m_nColumnCount )
            return "column counts are inconsistent!";

        if ( m_pModel->getRowCount() != m_nRowCount )
            return "row counts are inconsistent!";

        if ( ( ( m_nCurColumn != COL_INVALID ) && !m_aColumnWidthsPixel.empty() && ( m_nCurColumn < 0 ) ) || ( m_nCurColumn >= (ColPos)m_aColumnWidthsPixel.size() ) )
            return "current column is invalid!";

        if ( m_aColumnWidthsPixel.size() != m_aAccColumnWidthsPixel.size() )
            return "columnd width caches are inconsistent!";

        if ( !lcl_checkLimitsExclusive_OrDefault_OrFallback( m_nTopRow, (RowPos)-1, m_nRowCount, getModel(), (RowPos)0 ) )
            return "invalid top row value!";

        if ( !lcl_checkLimitsExclusive_OrDefault_OrFallback( m_nCurRow, (RowPos)-1, m_nRowCount, getModel(), ROW_INVALID ) )
            return "invalid current row value!";

        if ( !lcl_checkLimitsExclusive_OrDefault_OrFallback( m_nLeftColumn, (ColPos)-1, m_nColumnCount, getModel(), (ColPos)0 ) )
            return "invalid current column value!";

        if ( !lcl_checkLimitsExclusive_OrDefault_OrFallback( m_nCurColumn, (ColPos)-1, m_nColumnCount, getModel(), COL_INVALID ) )
            return "invalid current column value!";

        if  ( m_pInputHandler != m_pModel->getInputHandler() )
            return "input handler is not the model-provided one!";

        // m_nColHeaderHeightPixel consistent with the model's value?
        {
            TableMetrics nHeaderHeight = m_pModel->hasColumnHeaders() ? m_pModel->getColumnHeaderHeight() : 0;
            nHeaderHeight = m_rAntiImpl.LogicToPixel( Size( 0, nHeaderHeight ), MAP_APPFONT ).Height();
            if ( nHeaderHeight != m_nColHeaderHeightPixel )
                return "column header heights are inconsistent!";
        }

        bool isDummyModel = dynamic_cast< const EmptyTableModel* >( m_pModel.get() ) != NULL;
        if ( !isDummyModel )
        {
            TableMetrics nRowHeight = m_pModel->getRowHeight();
            nRowHeight = m_rAntiImpl.LogicToPixel( Size( 0, nRowHeight ), MAP_APPFONT).Height();
            if ( nRowHeight != m_nRowHeightPixel )
                return "row heights are inconsistent!";
        }

        // m_nRowHeaderWidthPixel consistent with the model's value?
        {
            TableMetrics nHeaderWidth = m_pModel->hasRowHeaders() ? m_pModel->getRowHeaderWidth() : 0;
            nHeaderWidth = m_rAntiImpl.LogicToPixel( Size( nHeaderWidth, 0 ), MAP_APPFONT ).Width();
            if ( nHeaderWidth != m_nRowHeaderWidthPixel )
                return "row header widths are inconsistent!";
        }

        // TODO: check m_aColumnWidthsPixel and m_aAccColumnWidthsPixel
        if ( m_nCursorHidden < 0 )
            return "invalid hidden count for the cursor!";

        if ( ( m_nRequiredInvariants & INV_SCROLL_POSITION ) && m_pVScroll )
        {
            DBG_SUSPEND_INV( INV_SCROLL_POSITION );
                // prevent infinite recursion

            if ( m_pVScroll->GetThumbPos() != m_nTopRow )
                return "vertical scroll bar |position| is incorrect!";
            if ( m_pVScroll->GetRange().Max() != m_nRowCount )
                return "vertical scroll bar |range| is incorrect!";
            if ( m_pVScroll->GetVisibleSize() != impl_getVisibleRows( false ) )
                return "vertical scroll bar |visible size| is incorrect!";
        }

        if ( ( m_nRequiredInvariants & INV_SCROLL_POSITION ) && m_pHScroll )
        {
            DBG_SUSPEND_INV( INV_SCROLL_POSITION );
                // prevent infinite recursion

            if ( m_pHScroll->GetThumbPos() != m_nLeftColumn )
                return "horizontal scroll bar |position| is incorrect!";
            if ( m_pHScroll->GetRange().Max() != m_nColumnCount )
                return "horizontal scroll bar |range| is incorrect!";
            if ( m_pHScroll->GetVisibleSize() != impl_getVisibleColumns( false ) )
                return "horizontal scroll bar |visible size| is incorrect!";
        }

        return NULL;
    }
#endif

#define DBG_CHECK_ME() \
    DBG_CHKTHIS( TableControl_Impl, TableControl_Impl_checkInvariants )

    //--------------------------------------------------------------------
    TableControl_Impl::TableControl_Impl( TableControl& _rAntiImpl )
        :m_rAntiImpl            ( _rAntiImpl                    )
        ,m_pModel               ( new EmptyTableModel           )
        ,m_pInputHandler        (                               )
        ,m_nRowHeightPixel      ( 15                            )
    ,m_nColHeaderHeightPixel( 0                             )
        ,m_nRowHeaderWidthPixel ( 0                             )
        ,m_nColumnCount         ( 0                             )
        ,m_nRowCount            ( 0                             )
        ,m_nCurColumn           ( COL_INVALID                   )
        ,m_nCurRow              ( ROW_INVALID                   )
        ,m_nLeftColumn          ( 0                             )
    ,m_nTopRow              ( 0                             )
        ,m_nCursorHidden        ( 1                             )
        ,m_pDataWindow          ( new TableDataWindow( *this )  )
    ,m_pVScroll             ( NULL                          )
        ,m_pHScroll             ( NULL                          )
        ,m_pScrollCorner        ( NULL                          )
    ,m_pSelEngine       (               )
    ,m_nRowSelected     (               )
    ,m_pTableFunctionSet    ( new TableFunctionSet(this )   )
    ,m_nAnchor      (-1             )
    ,m_bResizing        ( false             )
    ,m_nResizingColumn  ( 0             )
    ,m_bResizingGrid    ( false             )
#if DBG_UTIL
        ,m_nRequiredInvariants ( INV_SCROLL_POSITION )
#endif
    {
        DBG_CTOR( TableControl_Impl, TableControl_Impl_checkInvariants );
    m_pSelEngine = new SelectionEngine(m_pDataWindow, m_pTableFunctionSet);
    m_pSelEngine->SetSelectionMode(SINGLE_SELECTION);
        m_pDataWindow->SetPosPixel( Point( 0, 0 ) );
        m_pDataWindow->Show();
    }

    //--------------------------------------------------------------------
    TableControl_Impl::~TableControl_Impl()
    {
        DBG_DTOR( TableControl_Impl, TableControl_Impl_checkInvariants );

        DELETEZ( m_pVScroll );
        DELETEZ( m_pHScroll );
        DELETEZ( m_pScrollCorner );
        DELETEZ( m_pTableFunctionSet );
        DELETEZ( m_pSelEngine );
        DELETEZ( m_pDataWindow );
    }

    //--------------------------------------------------------------------
    PTableModel TableControl_Impl::getModel() const
    {
        if ( dynamic_cast< const EmptyTableModel* >( m_pModel.get() ) != NULL )
            // if it's an EmptyTableModel, pretend that there is no model
            return PTableModel();

        return m_pModel;
    }

    //--------------------------------------------------------------------
    void TableControl_Impl::setModel( PTableModel _pModel )
    {
        DBG_CHECK_ME();

        TempHideCursor aHideCursor( *this );

        m_pModel = _pModel;
        if ( !m_pModel)
            m_pModel.reset( new EmptyTableModel );

        m_nCurRow = ROW_INVALID;
        m_nCurColumn = COL_INVALID;

        // recalc some model-dependent cached info
        impl_ni_updateCachedModelValues();

        // completely invalidate
        m_rAntiImpl.Invalidate();

        // reset cursor to (0,0)
        if ( m_nRowCount ) m_nCurRow = 0;
        if ( m_nColumnCount ) m_nCurColumn = 0;
    }

    //--------------------------------------------------------------------
    void TableControl_Impl::impl_getAllVisibleCellsArea( Rectangle& _rCellArea ) const
    {
        DBG_CHECK_ME();

        _rCellArea.Left() = 0;
        _rCellArea.Top() = 0;

        // determine the right-most border of the last column which is
        // at least partially visible
        _rCellArea.Right() = m_nRowHeaderWidthPixel;
        if ( !m_aAccColumnWidthsPixel.empty() )
        {
            // the number of pixels which are scroll out of the left hand
            // side of the window
            long nScrolledOutLeft = m_nLeftColumn == 0 ? 0 : m_aAccColumnWidthsPixel[ m_nLeftColumn - 1 ];

            ArrayOfLong::const_reverse_iterator loop = m_aAccColumnWidthsPixel.rbegin();
            do
            {
                _rCellArea.Right() = *loop++ - nScrolledOutLeft + m_nRowHeaderWidthPixel;
            }
            while ( (   loop != m_aAccColumnWidthsPixel.rend() )
                 && (   *loop - nScrolledOutLeft >= _rCellArea.Right() )
                 );
        }
        // so far, _rCellArea.Right() denotes the first pixel *after* the cell area
        --_rCellArea.Right();

        // determine the last row which is at least partially visible
        _rCellArea.Bottom() =
                m_nColHeaderHeightPixel
            +   impl_getVisibleRows( true ) * m_nRowHeightPixel
            -   1;
    }

    //--------------------------------------------------------------------
    void TableControl_Impl::impl_getAllVisibleDataCellArea( Rectangle& _rCellArea ) const
    {
        DBG_CHECK_ME();

        impl_getAllVisibleCellsArea( _rCellArea );
        _rCellArea.Left() = m_nRowHeaderWidthPixel;
        _rCellArea.Top() = m_nColHeaderHeightPixel;
    }

    //--------------------------------------------------------------------
    void TableControl_Impl::impl_ni_updateCachedModelValues()
    {
        m_nRowHeightPixel = 15;
        m_nColHeaderHeightPixel = 0;
        m_nRowHeaderWidthPixel = 0;
        m_pInputHandler.reset();
        m_nColumnCount = m_nRowCount = 0;

    m_nRowHeightPixel = m_rAntiImpl.LogicToPixel( Size( 0, m_pModel->getRowHeight() ), MAP_APPFONT ).Height();
    if ( m_pModel->hasColumnHeaders() )
           m_nColHeaderHeightPixel = m_rAntiImpl.LogicToPixel( Size( 0, m_pModel->getColumnHeaderHeight() ), MAP_APPFONT ).Height();
        if ( m_pModel->hasRowHeaders() )
            m_nRowHeaderWidthPixel = m_rAntiImpl.LogicToPixel( Size( m_pModel->getRowHeaderWidth(), 0 ), MAP_APPFONT).Width();

        impl_ni_updateColumnWidths();

        m_pInputHandler = m_pModel->getInputHandler();
        if ( !m_pInputHandler )
            m_pInputHandler.reset( new DefaultInputHandler );

        m_nColumnCount = m_pModel->getColumnCount();
        m_nRowCount = m_pModel->getRowCount();
    }

    //--------------------------------------------------------------------
    void TableControl_Impl::impl_ni_updateColumnWidths()
    {
        m_aColumnWidthsPixel.resize( 0 );
        m_aAccColumnWidthsPixel.resize( 0 );
        if ( !m_pModel )
            return;

        TableSize colCount = m_pModel->getColumnCount();

        m_aColumnWidthsPixel.reserve( colCount );
        m_aAccColumnWidthsPixel.reserve( colCount );
        if(colCount>0)
        {
            std::vector<sal_Int32> aPrePixelWidths(0);
            long accumulatedPixelWidth = 0;
            int lastResizableCol = -1;
            double gridWidth = m_rAntiImpl.GetOutputSizePixel().Width();
            if(m_pModel->hasRowHeaders())
            {
                TableMetrics rowHeaderWidth = m_pModel->getRowHeaderWidth();
                gridWidth-= m_rAntiImpl.LogicToPixel( Size( rowHeaderWidth, 0 ), MAP_APPFONT ).Width();
            }
            if(m_pModel->hasVerticalScrollbar())
            {
                sal_Int32 scrollbarWidth = m_rAntiImpl.GetSettings().GetStyleSettings().GetScrollBarSize();
                gridWidth-=scrollbarWidth;
            }
            double colWidthsSum = 0.0;
            double colWithoutFixedWidthsSum = 0.0;
            double minColWithoutFixedSum = 0.0;
            for ( ColPos col = 0; col < colCount; ++col )
            {
                PColumnModel pColumn = m_pModel->getColumnModel( col );
                DBG_ASSERT( !!pColumn, "TableControl_Impl::impl_ni_updateColumnWidths: invalid column returned by the model!" );
                if ( !pColumn )
                    continue;
                TableMetrics colWidth = 0;
                TableMetrics colPrefWidth = pColumn->getPreferredWidth();
                bool bResizable = pColumn->isResizable();
                if(pColumn->getMinWidth() == 0 && bResizable)
                {
                    pColumn->setMinWidth(1);
                    minColWithoutFixedSum+=m_rAntiImpl.PixelToLogic( Size( 1, 0 ), MAP_APPFONT ).Width();
                }
                if(pColumn->getMaxWidth() == 0 && bResizable)
                    pColumn->setMaxWidth(m_rAntiImpl.PixelToLogic( Size( (int)gridWidth, 0 ), MAP_APPFONT ).Width());
                if( colPrefWidth != 0)
                {
                    if(m_bResizingGrid)
                    {
                        colWidth = pColumn->getWidth();
                        pColumn->setPreferredWidth(0);
                    }
                    else
                    {
                        colWidth = colPrefWidth;
                        pColumn->setWidth(colPrefWidth);
                    }
                }
                else
                    colWidth = pColumn->getWidth();
                long pixelWidth = m_rAntiImpl.LogicToPixel( Size( colWidth, 0 ), MAP_APPFONT ).Width();
                if(bResizable && colPrefWidth == 0)
                {
                    colWithoutFixedWidthsSum+=pixelWidth;
                    lastResizableCol = col;
                }
                colWidthsSum+=pixelWidth;
                aPrePixelWidths.push_back(pixelWidth);
            }
            double gridWidthWithoutFixed = gridWidth - colWidthsSum + colWithoutFixedWidthsSum;
            double scalingFactor = 1.0;
            if(m_bResizingGrid)
            {
                if(gridWidthWithoutFixed > (minColWithoutFixedSum+colWidthsSum - colWithoutFixedWidthsSum))
                    scalingFactor = gridWidthWithoutFixed/colWithoutFixedWidthsSum;
            }
            else
            {
                if(colWidthsSum < gridWidthWithoutFixed)
                {
                    if(colWithoutFixedWidthsSum>0)
                        scalingFactor = gridWidthWithoutFixed/colWithoutFixedWidthsSum;
                }
            }
            for ( ColPos i = 0; i < colCount; ++i )
            {
                PColumnModel pColumn = m_pModel->getColumnModel( i );
                DBG_ASSERT( !!pColumn, "TableControl_Impl::impl_ni_updateColumnWidths: invalid column returned by the model!" );
                if ( !pColumn )
                    continue;
                if(pColumn->isResizable() && pColumn->getPreferredWidth() == 0)
                {
                    aPrePixelWidths[i]*=scalingFactor;
                    TableMetrics logicColWidth = m_rAntiImpl.PixelToLogic( Size( aPrePixelWidths[i], 0 ), MAP_APPFONT ).Width();
                    pColumn->setWidth(logicColWidth);
                }
                m_aColumnWidthsPixel.push_back( aPrePixelWidths[i] );
                m_aAccColumnWidthsPixel.push_back( accumulatedPixelWidth += aPrePixelWidths[i] );
            }
            if(gridWidth > m_aAccColumnWidthsPixel[colCount-1])
            {
                if(lastResizableCol >= 0)
                {
                    PColumnModel pColumn = m_pModel->getColumnModel(lastResizableCol);
                    m_aColumnWidthsPixel[lastResizableCol]+=gridWidth-m_aAccColumnWidthsPixel[colCount-1];
                    TableMetrics logicColWidth1 = m_rAntiImpl.PixelToLogic( Size( m_aColumnWidthsPixel[lastResizableCol], 0 ), MAP_APPFONT ).Width();
                    pColumn->setWidth(logicColWidth1);
                    while(lastResizableCol < colCount)
                    {
                        if(lastResizableCol == 0)
                            m_aAccColumnWidthsPixel[0] = m_aColumnWidthsPixel[lastResizableCol];
                        else
                            m_aAccColumnWidthsPixel[lastResizableCol]=m_aAccColumnWidthsPixel[lastResizableCol-1]+m_aColumnWidthsPixel[lastResizableCol];
                        ++lastResizableCol;
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------
    namespace
    {
        //................................................................
        /// determines whether a scrollbar is needed for the given values
        bool lcl_determineScrollbarNeed( ScrollbarVisibility _eVisibility,
            long _nVisibleUnits, long _nRange )
        {
            if ( _eVisibility == ScrollbarShowNever )
                return false;
            if ( _eVisibility == ScrollbarShowAlways )
                return true;
            return _nVisibleUnits > _nRange;
        }

        //................................................................
        void lcl_setButtonRepeat( Window& _rWindow, ULONG _nDelay )
        {
            AllSettings aSettings = _rWindow.GetSettings();
            MouseSettings aMouseSettings = aSettings.GetMouseSettings();

            aMouseSettings.SetButtonRepeat( _nDelay );
            aSettings.SetMouseSettings( aMouseSettings );

            _rWindow.SetSettings( aSettings, TRUE );
        }

        //................................................................
        void lcl_updateScrollbar( Window& _rParent, ScrollBar*& _rpBar,
            ScrollbarVisibility _eVisibility, long _nVisibleUnits,
            long _nPosition, long _nLineSize, long _nRange,
            bool _bHorizontal, const Link& _rScrollHandler )
        {
            // do we need the scrollbar?
            bool bNeedBar = lcl_determineScrollbarNeed( _eVisibility, _nVisibleUnits, _nRange );

            // do we currently have the scrollbar?
            bool bHaveBar = _rpBar != NULL;

            // do we need to correct the scrollbar visibility?
            if ( bHaveBar && !bNeedBar )
            {
                DELETEZ( _rpBar );
            }
            else if ( !bHaveBar && bNeedBar )
            {
                _rpBar = new ScrollBar(
                    &_rParent,
                    WB_DRAG | ( _bHorizontal ? WB_HSCROLL : WB_VSCROLL )
                );
                _rpBar->SetScrollHdl( _rScrollHandler );
                // get some speed into the scrolling ....
                lcl_setButtonRepeat( *_rpBar, 0 );
            }

            if ( _rpBar )
            {
                _rpBar->SetRange( Range( 0, _nRange ) );
                _rpBar->SetVisibleSize( _nVisibleUnits );
                _rpBar->SetPageSize( _nVisibleUnits );
                _rpBar->SetLineSize( _nLineSize );
                _rpBar->SetThumbPos( _nPosition );
                _rpBar->Show();
            }
        }

        //................................................................
        /** returns the number of rows fitting into the given range,
            for the given row height. Partially fitting rows are counted, too, if the
            respective parameter says so.
        */
        TableSize lcl_getRowsFittingInto( long _nOverallHeight, long _nRowHeightPixel, bool _bAcceptPartialRow = false )
        {
            return  _bAcceptPartialRow
                ?   ( _nOverallHeight + ( _nRowHeightPixel - 1 ) ) / _nRowHeightPixel
                :   _nOverallHeight / _nRowHeightPixel;
        }

        //................................................................
        /** returns the number of columns fitting into the given area,
            with the first visible column as given. Partially fitting columns are counted, too,
            if the respective parameter says so.
        */
        TableSize lcl_getColumnsVisibleWithin( const Rectangle& _rArea, ColPos _nFirstVisibleColumn,
            const TableControl_Impl& _rControl, bool _bAcceptPartialRow )
        {
            TableSize visibleColumns = 0;
            TableColumnGeometry aColumn( _rControl, _rArea, _nFirstVisibleColumn );
            while ( aColumn.isValid() )
            {
                if ( !_bAcceptPartialRow )
                    if ( aColumn.getRect().Right() > _rArea.Right() )
                        // this column is only partially visible, and this is not allowed
                        break;

                aColumn.moveRight();
                ++visibleColumns;
            }
            return visibleColumns;
        }

    }

    //--------------------------------------------------------------------
    void TableControl_Impl::impl_ni_updateScrollbars()
    {
        TempHideCursor aHideCursor( *this );

        // the width/height of a scrollbar, needed several times below
        long nScrollbarMetrics = m_rAntiImpl.GetSettings().GetStyleSettings().GetScrollBarSize();
        if ( m_rAntiImpl.IsZoom() )
            nScrollbarMetrics = (long)( nScrollbarMetrics * (double)m_rAntiImpl.GetZoom() );

        // determine the playground for the data cells (excluding headers)
        // TODO: what if the control is smaller than needed for the headers/scrollbars?
        Rectangle aDataCellPlayground( Point( 0, 0 ), m_rAntiImpl.GetOutputSizePixel() );
        aDataCellPlayground.Left() = m_nRowHeaderWidthPixel;
        aDataCellPlayground.Top() = m_nColHeaderHeightPixel;
        m_nRowCount = m_pModel->getRowCount();
        m_nColumnCount = m_pModel->getColumnCount();

        if(m_aAccColumnWidthsPixel.empty())
        {
            impl_ni_updateColumnWidths();
        }

        // do we need a vertical scrollbar?
        bool bFirstRoundVScrollNeed = false;
        if ( lcl_determineScrollbarNeed(
                m_pModel->getVerticalScrollbarVisibility(aDataCellPlayground.GetHeight(), m_nRowHeightPixel*m_nRowCount),
                lcl_getRowsFittingInto( aDataCellPlayground.GetHeight(), m_nRowHeightPixel ),
                m_nRowCount ) )
        {
            aDataCellPlayground.Right() -= nScrollbarMetrics;
            bFirstRoundVScrollNeed = true;
        }
        // do we need a horizontal scrollbar?
        if ( lcl_determineScrollbarNeed(
                m_pModel->getHorizontalScrollbarVisibility(aDataCellPlayground.GetWidth(), m_aAccColumnWidthsPixel[m_nColumnCount-1]),
                lcl_getColumnsVisibleWithin( aDataCellPlayground, m_nLeftColumn, *this, false ),
                m_nColumnCount ) )
        {
            aDataCellPlayground.Bottom() -= nScrollbarMetrics;

            // now that we just found that we need a horizontal scrollbar,
            // the need for a vertical one may have changed, since the horizontal
            // SB might just occupy enough space so that not all rows do fit
            // anymore
            if ( !bFirstRoundVScrollNeed && lcl_determineScrollbarNeed(
                    m_pModel->getVerticalScrollbarVisibility(aDataCellPlayground.GetHeight(),m_nRowHeightPixel*m_nRowCount),
                    lcl_getRowsFittingInto( aDataCellPlayground.GetHeight(), m_nRowHeightPixel ),
                    m_nRowCount ) )
            {
                aDataCellPlayground.Right() -= nScrollbarMetrics;
            }
        }
        // create or destroy the vertical scrollbar, as needed
        lcl_updateScrollbar(
            m_rAntiImpl,
            m_pVScroll,
            m_pModel->getVerticalScrollbarVisibility(aDataCellPlayground.GetHeight(),m_nRowHeightPixel*m_nRowCount),
            lcl_getRowsFittingInto( aDataCellPlayground.GetHeight(), m_nRowHeightPixel ),
                                                                    // visible units
            m_nTopRow,                                              // current position
            1,                                                      // line size
            m_nRowCount,                                            // range
            false,                                                  // vertical
            LINK( this, TableControl_Impl, OnScroll )               // scroll handler
        );
        // position it
        if ( m_pVScroll )
        {
            Rectangle aScrollbarArea(
                Point( aDataCellPlayground.Right() + 1, 0 ),
                Size( nScrollbarMetrics, aDataCellPlayground.Bottom() + 1 )
            );
            m_pVScroll->SetPosSizePixel(
                aScrollbarArea.TopLeft(), aScrollbarArea.GetSize() );
        }

        // create or destroy the horizontal scrollbar, as needed
        lcl_updateScrollbar(
            m_rAntiImpl,
            m_pHScroll,
            m_pModel->getHorizontalScrollbarVisibility(aDataCellPlayground.GetWidth(), m_aAccColumnWidthsPixel[m_nColumnCount-1]),
            lcl_getColumnsVisibleWithin( aDataCellPlayground, m_nLeftColumn, *this, false ),
                                                                    // visible units
            m_nLeftColumn,                                          // current position
            1,                                                      // line size
            m_nColumnCount,                                         // range
            true,                                                   // horizontal
            LINK( this, TableControl_Impl, OnScroll )               // scroll handler
        );
        // position it
        if ( m_pHScroll )
        {
            TableSize nVisibleUnits = lcl_getColumnsVisibleWithin( aDataCellPlayground, m_nLeftColumn, *this, false );
            int nRange = m_nColumnCount;
            if( m_nLeftColumn + nVisibleUnits == nRange-1)
            {
                if(m_aAccColumnWidthsPixel[nRange-2] - m_aAccColumnWidthsPixel[m_nLeftColumn] + m_aColumnWidthsPixel[nRange-1]>aDataCellPlayground.GetWidth())
                {
                    m_pHScroll->SetVisibleSize( nVisibleUnits -1 );
                    m_pHScroll->SetPageSize(nVisibleUnits -1);
                }
            }
            Rectangle aScrollbarArea(
                Point( 0, aDataCellPlayground.Bottom() + 1 ),
                Size( aDataCellPlayground.Right() + 1, nScrollbarMetrics )
            );
            m_pHScroll->SetPosSizePixel(
                aScrollbarArea.TopLeft(), aScrollbarArea.GetSize() );
        }

        // the corner window connecting the two scrollbars in the lower right corner
        bool bHaveScrollCorner = NULL != m_pScrollCorner;
        bool bNeedScrollCorner = ( NULL != m_pHScroll ) && ( NULL != m_pVScroll );
        if ( bHaveScrollCorner && !bNeedScrollCorner )
        {
            DELETEZ( m_pScrollCorner );
        }
        else if ( !bHaveScrollCorner && bNeedScrollCorner )
        {
            m_pScrollCorner = new ScrollBarBox( &m_rAntiImpl );
            m_pScrollCorner->SetSizePixel( Size( nScrollbarMetrics, nScrollbarMetrics ) );
            m_pScrollCorner->SetPosPixel( Point( aDataCellPlayground.Right() + 1, aDataCellPlayground.Bottom() + 1 ) );
            m_pScrollCorner->Show();
        }
        else if(bHaveScrollCorner && bNeedScrollCorner)
        {
            m_pScrollCorner->SetPosPixel( Point( aDataCellPlayground.Right() + 1, aDataCellPlayground.Bottom() + 1 ) );
            m_pScrollCorner->Show();
        }

        // resize the data window
        m_pDataWindow->SetSizePixel( Size(
            aDataCellPlayground.GetWidth() + m_nRowHeaderWidthPixel,
            aDataCellPlayground.GetHeight() + m_nColHeaderHeightPixel
        ) );
    }

    //--------------------------------------------------------------------
    void TableControl_Impl::onResize()
    {
        DBG_CHECK_ME();
        if(m_nRowCount != 0)
        {
            if(m_nColumnCount != 0)
            {
                if(m_bResizingGrid)
                    impl_ni_updateColumnWidths();
                invalidateRows();
                m_bResizingGrid = true;
            }
        }
        else
        {
            //In the case that column headers are defined but data hasn't yet been set,
            //only column headers will be shown
            if(m_pModel->hasColumnHeaders())
                if(m_nColHeaderHeightPixel>1)
                    m_pDataWindow->SetSizePixel( m_rAntiImpl.GetOutputSizePixel());
            if(m_nColumnCount != 0)
                impl_ni_updateScrollbars();
        }
    }

    //--------------------------------------------------------------------
    void TableControl_Impl::doPaintContent( const Rectangle& _rUpdateRect )
    {
        DBG_CHECK_ME();

        if ( !getModel() )
            return;
        PTableRenderer pRenderer = getModel()->getRenderer();
        DBG_ASSERT( !!pRenderer, "TableDataWindow::Paint: invalid renderer!" );
        if ( !pRenderer )
            return;

        // our current style settings, to be passed to the renderer
        const StyleSettings& rStyle = m_rAntiImpl.GetSettings().GetStyleSettings();
        m_nRowCount = m_pModel->getRowCount();
        TableSize nVisibleRows = impl_getVisibleRows(true);
        TableSize nActualRows = m_nRowCount;
        if(m_nRowCount>nVisibleRows)
            nActualRows = nVisibleRows;
        // the area occupied by all (at least partially) visible cells, including
        // headers
        Rectangle aAllCellsWithHeaders;
        impl_getAllVisibleCellsArea( aAllCellsWithHeaders );

        // ............................
        // draw the header column area
        if ( m_pModel->hasColumnHeaders() )
        {
            TableRowGeometry aHeaderRow( *this, Rectangle( Point( 0, 0 ),
                aAllCellsWithHeaders.BottomRight() ), ROW_COL_HEADERS );
            Rectangle aColRect(aHeaderRow.getRect());
            //to avoid double lines when scrolling horizontally
            if(m_nLeftColumn != 0)
                --aColRect.Left();
            pRenderer->PaintHeaderArea(
                *m_pDataWindow, aColRect, true, false, rStyle
            );
            // Note that strictly, aHeaderRow.getRect() also contains the intersection between column
            // and row header area. However, below we go to paint this intersection, again,
            // so this hopefully doesn't hurt if we already paint it here.

            for ( TableCellGeometry aCell( aHeaderRow, m_nLeftColumn );
                  aCell.isValid();
                  aCell.moveRight()
                )
            {
                if ( _rUpdateRect.GetIntersection( aCell.getRect() ).IsEmpty() )
                    continue;

                bool isActiveColumn = ( aCell.getColumn() == getCurColumn() );
                bool isSelectedColumn = false;
                pRenderer->PaintColumnHeader( aCell.getColumn(), isActiveColumn, isSelectedColumn,
                    *m_pDataWindow, aCell.getRect(), rStyle );
            }
        }
        // the area occupied by the row header, if any
        Rectangle aRowHeaderArea;
        if ( m_pModel->hasRowHeaders() )
        {
            aRowHeaderArea = aAllCellsWithHeaders;
            aRowHeaderArea.Right() = m_nRowHeaderWidthPixel - 1;
            if(m_nTopRow+nActualRows>m_nRowCount)
                aRowHeaderArea.Bottom() = m_nRowHeightPixel * (nActualRows -1)+ m_nColHeaderHeightPixel - 1;
            else
                aRowHeaderArea.Bottom() = m_nRowHeightPixel * nActualRows + m_nColHeaderHeightPixel - 1;
            //to avoid double lines when scrolling vertically
            if(m_nTopRow != 0)
                --aRowHeaderArea.Top();
            --aRowHeaderArea.Right();
            pRenderer->PaintHeaderArea(*m_pDataWindow, aRowHeaderArea, false, true, rStyle);
            // Note that strictly, aRowHeaderArea also contains the intersection between column
            // and row header area. However, below we go to paint this intersection, again,
            // so this hopefully doesn't hurt if we already paint it here.

            if ( m_pModel->hasColumnHeaders() )
            {
                TableCellGeometry aIntersection( *this, Rectangle( Point( 0, 0 ),
                    aAllCellsWithHeaders.BottomRight() ), COL_ROW_HEADERS, ROW_COL_HEADERS );
                Rectangle aInters(aIntersection.getRect());
                //to avoid double line when scrolling vertically
                if( m_nTopRow != 0 )
                {
                    --aInters.Top();
                    --aInters.Bottom();
                }
                --aInters.Right();
                pRenderer->PaintHeaderArea(
                    *m_pDataWindow, aInters, true, true, rStyle
                );
            }
        }

        // ............................
        // draw the table content row by row

        TableSize colCount = getModel()->getColumnCount();

        // paint all rows
        Rectangle aAllDataCellsArea;
        impl_getAllVisibleDataCellArea( aAllDataCellsArea );
        ::std::vector< std::vector< ::com::sun::star::uno::Any > >& aCellContent = m_pModel->getCellContent();
        for ( TableRowGeometry aRowIterator( *this, aAllCellsWithHeaders, getTopRow() );
              aRowIterator.isValid();
              aRowIterator.moveDown() )
        {
            if ( _rUpdateRect.GetIntersection( aRowIterator.getRect() ).IsEmpty() )
                continue;
            bool isActiveRow = ( aRowIterator.getRow() == getCurRow() );
            bool isSelectedRow = false;
            if(!m_nRowSelected.empty())
            {
                for(std::vector<RowPos>::iterator itSel=m_nRowSelected.begin();
                    itSel!=m_nRowSelected.end();++itSel)
                {
                    if(*itSel == aRowIterator.getRow())
                        isSelectedRow = true;
                }
            }
            Rectangle aRect = aRowIterator.getRect().GetIntersection( aAllDataCellsArea );
            //to avoid double lines
            if( aRowIterator.getRow() != 0 )
                --aRect.Top();
            if(m_nLeftColumn != 0)
                --aRect.Left();
            else
            {
                if(m_pModel->hasRowHeaders())
                    --aRect.Left();
            }
            // give the redenderer a chance to prepare the row
            pRenderer->PrepareRow( aRowIterator.getRow(), isActiveRow, isSelectedRow,
            *m_pDataWindow, aRect, rStyle );

            // paint the row header
            if ( m_pModel->hasRowHeaders() )
            {
                Rectangle aCurrentRowHeader( aRowHeaderArea.GetIntersection( aRowIterator.getRect() ) );
                rtl::OUString rowHeaderName = m_pModel->getRowHeaderName()[aRowIterator.getRow()];
                pRenderer->PaintRowHeader( isActiveRow, isSelectedRow, *m_pDataWindow, aCurrentRowHeader,
                    rStyle, rowHeaderName );
            }
            if ( !colCount )
                continue;
            // paint all cells in this row
            for ( TableCellGeometry aCell( aRowIterator, m_nLeftColumn );
                  aCell.isValid();
                  aCell.moveRight()
                )
            {
                bool isSelectedColumn = false;
                ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >xGraphic;
                ::com::sun::star::uno::Any rCellData = aCellContent[aRowIterator.getRow()][aCell.getColumn()];
                if(rCellData>>=xGraphic)
                {
                    Image* pImage = new Image(xGraphic);
                    if(pImage!=NULL)
                        pRenderer->PaintCellImage( aCell.getColumn(), isSelectedRow || isSelectedColumn, isActiveRow,
                                *m_pDataWindow, aCell.getRect(), rStyle, pImage );
                }
                else
                {
                    ::rtl::OUString sContent = convertToString(rCellData);
                    pRenderer->PaintCellString( aCell.getColumn(), isSelectedRow || isSelectedColumn, isActiveRow,
                        *m_pDataWindow, aCell.getRect(), rStyle, sContent );
                }
            }
        }
    }
    //--------------------------------------------------------------------
    void TableControl_Impl::hideCursor()
    {
        DBG_CHECK_ME();

        if ( ++m_nCursorHidden == 1 )
            impl_ni_doSwitchCursor( false );
    }

    //--------------------------------------------------------------------
    void TableControl_Impl::showCursor()
    {
        DBG_CHECK_ME();

        DBG_ASSERT( m_nCursorHidden > 0, "TableControl_Impl::showCursor: cursor not hidden!" );
        if ( --m_nCursorHidden == 0 )
            impl_ni_doSwitchCursor( true );
    }

    //--------------------------------------------------------------------
    bool TableControl_Impl::dispatchAction( TableControlAction _eAction )
    {
        DBG_CHECK_ME();

        bool bSuccess = false;
        Rectangle rCells;
        switch ( _eAction )
        {
        case cursorDown:
        if(m_pSelEngine->GetSelectionMode() == SINGLE_SELECTION)
        {
            //if other rows already selected, deselect them
            if(m_nRowSelected.size()>0)
            {
                for(std::vector<RowPos>::iterator it=m_nRowSelected.begin();
                        it!=m_nRowSelected.end();++it)
                {
                    invalidateSelectedRegion(*it, *it, rCells);
                }
                m_nRowSelected.clear();
            }
            if(m_nCurRow < m_nRowCount-1)
            {
                ++m_nCurRow;
                m_nRowSelected.push_back(m_nCurRow);
            }
            else
                m_nRowSelected.push_back(m_nCurRow);
            invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
            ensureVisible(m_nCurColumn,m_nCurRow,false);
            m_rAntiImpl.selectionChanged(true);
            bSuccess = true;
        }
        else
        {
            if ( m_nCurRow < m_nRowCount - 1 )
                bSuccess = goTo( m_nCurColumn, m_nCurRow + 1 );
        }
            break;

        case cursorUp:
        if(m_pSelEngine->GetSelectionMode() == SINGLE_SELECTION)
        {
            if(m_nRowSelected.size()>0)
            {
                for(std::vector<RowPos>::iterator it=m_nRowSelected.begin();
                    it!=m_nRowSelected.end();++it)
                {
                    invalidateSelectedRegion(*it, *it, rCells);
                }
                m_nRowSelected.clear();
            }
            if(m_nCurRow>0)
            {
                --m_nCurRow;
                m_nRowSelected.push_back(m_nCurRow);
                invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
            }
            else
            {
                m_nRowSelected.push_back(m_nCurRow);
                invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
            }
            ensureVisible(m_nCurColumn,m_nCurRow,false);
            m_rAntiImpl.selectionChanged(true);
            bSuccess = true;
        }
        else
        {
            if ( m_nCurRow > 0 )
                bSuccess = goTo( m_nCurColumn, m_nCurRow - 1 );
        }
        break;
        case cursorLeft:
            if ( m_nCurColumn > 0 )
                bSuccess = goTo( m_nCurColumn - 1, m_nCurRow );
            else
                if ( ( m_nCurColumn == 0) && ( m_nCurRow > 0 ) )
                    bSuccess = goTo( m_nColumnCount - 1, m_nCurRow - 1 );
            break;

        case cursorRight:
            if ( m_nCurColumn < m_nColumnCount - 1 )
                bSuccess = goTo( m_nCurColumn + 1, m_nCurRow );
            else
                if ( ( m_nCurColumn == m_nColumnCount - 1 ) && ( m_nCurRow < m_nRowCount - 1 ) )
                    bSuccess = goTo( 0, m_nCurRow + 1 );
            break;

        case cursorToLineStart:
            bSuccess = goTo( 0, m_nCurRow );
            break;

        case cursorToLineEnd:
            bSuccess = goTo( m_nColumnCount - 1, m_nCurRow );
            break;

        case cursorToFirstLine:
            bSuccess = goTo( m_nCurColumn, 0 );
            break;

        case cursorToLastLine:
            bSuccess = goTo( m_nCurColumn, m_nRowCount - 1 );
            break;

        case cursorPageUp:
        {
            RowPos nNewRow = ::std::max( (RowPos)0, m_nCurRow - impl_getVisibleRows( false ) );
            bSuccess = goTo( m_nCurColumn, nNewRow );
        }
        break;

        case cursorPageDown:
        {
            RowPos nNewRow = ::std::min( m_nRowCount - 1, m_nCurRow + impl_getVisibleRows( false ) );
            bSuccess = goTo( m_nCurColumn, nNewRow );
        }
        break;

        case cursorTopLeft:
            bSuccess = goTo( 0, 0 );
            break;

        case cursorBottomRight:
            bSuccess = goTo( m_nColumnCount - 1, m_nRowCount - 1 );
            break;
    case cursorSelectRow:
    {
        if(m_pSelEngine->GetSelectionMode() == NO_SELECTION)
            return bSuccess = false;
        //pos is the position of the current row in the vector of selected rows, if current row is selected
        int pos = getRowSelectedNumber(m_nRowSelected, m_nCurRow);
        //if current row is selected, it should be deselected, when ALT+SPACE are pressed
        if(pos>-1)
        {
            m_nRowSelected.erase(m_nRowSelected.begin()+pos);
            if(m_nRowSelected.empty() && m_nAnchor != -1)
                m_nAnchor = -1;
        }
        //else select the row->put it in the vector
        else
            m_nRowSelected.push_back(m_nCurRow);
        invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
        m_rAntiImpl.selectionChanged(true);
        bSuccess = true;
    }
        break;
    case cursorSelectRowUp:
    {
        if(m_pSelEngine->GetSelectionMode() == NO_SELECTION)
            return bSuccess = false;
        else if(m_pSelEngine->GetSelectionMode() == SINGLE_SELECTION)
        {
            //if there are other selected rows, deselect them
            return false;
        }
        else
        {
            //there are other selected rows
            if(m_nRowSelected.size()>0)
            {
                //the anchor wasn't set -> a region is not selected, that's why clear all selection
                //and select the current row
                if(m_nAnchor==-1)
                {
                    for(std::vector<RowPos>::iterator it=m_nRowSelected.begin();
                        it!=m_nRowSelected.end();++it)
                    {
                        invalidateSelectedRegion(*it, *it, rCells);
                    }
                    m_nRowSelected.clear();
                    m_nRowSelected.push_back(m_nCurRow);
                    invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
                }
                else
                {
                    //a region is already selected, prevRow is last selected row and the row above - nextRow - should be selected
                    int prevRow = getRowSelectedNumber(m_nRowSelected, m_nCurRow);
                    int nextRow = getRowSelectedNumber(m_nRowSelected, m_nCurRow-1);
                    if(prevRow>-1)
                     {
                         //if m_nCurRow isn't the upper one, can move up, otherwise not
                        if(m_nCurRow>0)
                             m_nCurRow--;
                         else
                             return bSuccess = true;
                         //if nextRow already selected, deselect it, otherwise select it
                         if(nextRow>-1 && m_nRowSelected[nextRow] == m_nCurRow)
                         {
                             m_nRowSelected.erase(m_nRowSelected.begin()+prevRow);
                             invalidateSelectedRegion(m_nCurRow+1, m_nCurRow+1, rCells);
                         }
                         else
                        {
                             m_nRowSelected.push_back(m_nCurRow);
                             invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
                         }
                     }
                    else
                    {
                        if(m_nCurRow>0)
                        {
                            m_nRowSelected.push_back(m_nCurRow);
                            m_nCurRow--;
                            m_nRowSelected.push_back(m_nCurRow);
                            invalidateSelectedRegion(m_nCurRow+1, m_nCurRow, rCells);
                        }
                    }
                }
            }
            else
            {
                //if nothing is selected and the current row isn't the upper one
                //select the current and one row above
                //otherwise select only the upper row
                if(m_nCurRow>0)
                {
                    m_nRowSelected.push_back(m_nCurRow);
                    m_nCurRow--;
                    m_nRowSelected.push_back(m_nCurRow);
                    invalidateSelectedRegion(m_nCurRow+1, m_nCurRow, rCells);
                }
                else
                {
                    m_nRowSelected.push_back(m_nCurRow);
                    invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
                }
            }
            m_pSelEngine->SetAnchor(TRUE);
            m_nAnchor = m_nCurRow;
            ensureVisible(m_nCurColumn, m_nCurRow, false);
            m_rAntiImpl.selectionChanged(true);
            bSuccess = true;
        }
    }
    break;
    case cursorSelectRowDown:
    {
        if(m_pSelEngine->GetSelectionMode() == NO_SELECTION)
            bSuccess = false;
        else if(m_pSelEngine->GetSelectionMode() == SINGLE_SELECTION)
        {
            bSuccess = false;
        }
        else
        {
            if(m_nRowSelected.size()>0)
            {
                //the anchor wasn't set -> a region is not selected, that's why clear all selection
                //and select the current row
                if(m_nAnchor==-1)
                {
                    for(std::vector<RowPos>::iterator it=m_nRowSelected.begin();
                        it!=m_nRowSelected.end();++it)
                    {
                        invalidateSelectedRegion(*it, *it, rCells);
                    }
                    m_nRowSelected.clear();
                    m_nRowSelected.push_back(m_nCurRow);
                    invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
                    }
                else
                {
                    //a region is already selected, prevRow is last selected row and the row beneath - nextRow - should be selected
                    int prevRow = getRowSelectedNumber(m_nRowSelected, m_nCurRow);
                    int nextRow = getRowSelectedNumber(m_nRowSelected, m_nCurRow+1);
                    if(prevRow>-1)
                     {
                         //if m_nCurRow isn't the last one, can move down, otherwise not
                         if(m_nCurRow<m_nRowCount-1)
                             m_nCurRow++;
                         else
                            return bSuccess = true;
                         //if next row already selected, deselect it, otherwise select it
                         if(nextRow>-1 && m_nRowSelected[nextRow] == m_nCurRow)
                         {
                             m_nRowSelected.erase(m_nRowSelected.begin()+prevRow);
                             invalidateSelectedRegion(m_nCurRow-1, m_nCurRow-1, rCells);
                         }
                         else
                         {
                             m_nRowSelected.push_back(m_nCurRow);
                             invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
                         }
                    }
                    else
                    {
                        if(m_nCurRow<m_nRowCount-1)
                        {
                            m_nRowSelected.push_back(m_nCurRow);
                            m_nCurRow++;
                            m_nRowSelected.push_back(m_nCurRow);
                            invalidateSelectedRegion(m_nCurRow-1, m_nCurRow, rCells);
                        }
                    }
                }
            }
            else
            {
                //there wasn't any selection, select current and row beneath, otherwise only row beneath
                if(m_nCurRow<m_nRowCount-1)
                {
                    m_nRowSelected.push_back(m_nCurRow);
                    m_nCurRow++;
                    m_nRowSelected.push_back(m_nCurRow);
                    invalidateSelectedRegion(m_nCurRow-1, m_nCurRow, rCells);
                }
                else
                {
                    m_nRowSelected.push_back(m_nCurRow);
                    invalidateSelectedRegion(m_nCurRow, m_nCurRow, rCells);
                }
            }
            m_pSelEngine->SetAnchor(TRUE);
            m_nAnchor = m_nCurRow;
            ensureVisible(m_nCurColumn, m_nCurRow, false);
            m_rAntiImpl.selectionChanged(true);
            bSuccess = true;
        }
    }
        break;
    case cursorSelectRowAreaTop:
    {
        if(m_pSelEngine->GetSelectionMode() == NO_SELECTION)
            bSuccess = false;
        else if(m_pSelEngine->GetSelectionMode() == SINGLE_SELECTION)
            bSuccess = false;
        else
        {
            //select the region between the current and the upper row
            RowPos iter = m_nCurRow;
            invalidateSelectedRegion(m_nCurRow, 0, rCells);
            //put the rows in vector
            while(iter>=0)
            {
                if(!isRowSelected(m_nRowSelected, iter))
                    m_nRowSelected.push_back(iter);
                --iter;
            }
            m_nCurRow = 0;
            m_nAnchor = m_nCurRow;
            m_pSelEngine->SetAnchor(TRUE);
            ensureVisible(m_nCurColumn, 0, false);
            m_rAntiImpl.selectionChanged(true);
            bSuccess = true;
        }
    }
        break;
    case cursorSelectRowAreaBottom:
    {
        if(m_pSelEngine->GetSelectionMode() == NO_SELECTION)
            return bSuccess = false;
        else if(m_pSelEngine->GetSelectionMode() == SINGLE_SELECTION)
            return bSuccess = false;
        //select the region between the current and the last row
        RowPos iter = m_nCurRow;
        invalidateSelectedRegion(m_nCurRow, m_nRowCount-1, rCells);
        //put the rows in the vector
        while(iter<=m_nRowCount)
        {
            if(!isRowSelected(m_nRowSelected, iter))
                m_nRowSelected.push_back(iter);
            ++iter;
        }
        m_nCurRow = m_nRowCount-1;
        m_nAnchor = m_nCurRow;
        m_pSelEngine->SetAnchor(TRUE);
        ensureVisible(m_nCurColumn, m_nRowCount-1, false);
        m_rAntiImpl.selectionChanged(true);
        bSuccess = true;
    }
        break;
        default:
            OSL_FAIL( "TableControl_Impl::dispatchAction: unsupported action!" );
        }
        return bSuccess;
    }

    //--------------------------------------------------------------------
    void TableControl_Impl::impl_ni_doSwitchCursor( bool _bShow )
    {
        PTableRenderer pRenderer = !!m_pModel ? m_pModel->getRenderer() : PTableRenderer();
        if ( !!pRenderer )
        {
            Rectangle aCellRect;
            impl_getCellRect( m_nCurColumn, m_nCurRow, aCellRect );
            if(!m_pModel->hasRowHeaders() && m_nCurColumn == 0)
                aCellRect.Left()++;
            if ( _bShow )
                pRenderer->ShowCellCursor( *m_pDataWindow, aCellRect);
            else
                pRenderer->HideCellCursor( *m_pDataWindow, aCellRect);
        }
    }

    //--------------------------------------------------------------------
    void TableControl_Impl::impl_getCellRect( ColPos _nColumn, RowPos _nRow, Rectangle& _rCellRect ) const
    {
        DBG_CHECK_ME();

        if  (   !m_pModel
            ||  ( COL_INVALID == _nColumn )
            ||  ( ROW_INVALID == _nRow )
            )
        {
            _rCellRect.SetEmpty();
            return;
        }

        Rectangle aAllCells;
        impl_getAllVisibleCellsArea( aAllCells );

        TableCellGeometry aCell( *this, aAllCells, _nColumn, _nRow );
        _rCellRect = aCell.getRect();
        _rCellRect.Top()--;_rCellRect.Left()--;
    }
    //-------------------------------------------------------------------------------
    RowPos TableControl_Impl::getCurrentRow(const Point& rPoint)
    {
        DBG_CHECK_ME();
        Rectangle rCellRect;
        RowPos newRowPos = -2;//-1 is HeaderRow
        ColPos newColPos = 0;
        for(int i=-1;i<m_nRowCount;i++)
        {
            for(int j=-1;j<m_nColumnCount;j++)
            {
                impl_getCellRect(j,i,rCellRect);
                if((rPoint.X() >= rCellRect.Left() && rPoint.X() <= rCellRect.Right()) && rPoint.Y() >= rCellRect.Top() && rPoint.Y() <= rCellRect.Bottom())
                {
                    newRowPos = i;
                    newColPos = j;
                    if(newColPos != -1)
                        m_nCurColumn = newColPos;
                    return newRowPos;
                }
            }
        }
        return newRowPos;
    }
    //-------------------------------------------------------------------------------
    void TableControl_Impl::setCursorAtCurrentCell(const Point& rPoint)
    {
        DBG_CHECK_ME();
        hideCursor();
        Rectangle rCellRect;
        RowPos newRowPos = -2;//-1 is HeaderRow
        ColPos newColPos = 0;
        for(int i=0;i<m_nRowCount;i++)
        {
            for(int j=-1;j<m_nColumnCount;j++)
            {
                impl_getCellRect(j,i,rCellRect);
                if((rPoint.X() >= rCellRect.Left() && rPoint.X() <= rCellRect.Right()) && rPoint.Y() >= rCellRect.Top() && rPoint.Y() <= rCellRect.Bottom())
                {
                    newRowPos = i;
                    m_nCurRow = newRowPos;
                    newColPos = j;
                    if(newColPos == -1)
                        m_nCurColumn = 0;
                    else
                        m_nCurColumn = newColPos;
                }
            }
        }
        showCursor();
    }
    //-------------------------------------------------------------------------------
    void TableControl_Impl::invalidateSelectedRegion(RowPos _nPrevRow, RowPos _nCurRow, Rectangle& _rCellRect)
    {
        DBG_CHECK_ME();
        Rectangle aAllCells;
        //get the visible area of the table control and set the Left and right border of the region to be repainted
        impl_getAllVisibleCellsArea( aAllCells );
        _rCellRect.Left() = aAllCells.Left();
        _rCellRect.Right() = aAllCells.Right();
        Rectangle rCells;
        //if only one row is selected
        if(_nPrevRow == _nCurRow)
        {
            impl_getCellRect(m_nCurColumn,_nCurRow,rCells);
            _rCellRect.Top()=--rCells.Top();
            _rCellRect.Bottom()=rCells.Bottom();
        }
        //if the region is above the current row
        else if(_nPrevRow < _nCurRow )
        {
            impl_getCellRect(m_nCurColumn,_nPrevRow,rCells);
            _rCellRect.Top()= --rCells.Top();
            impl_getCellRect(m_nCurColumn,_nCurRow,rCells);
            _rCellRect.Bottom()=rCells.Bottom();
        }
        //if the region is beneath the current row
        else
        {
            impl_getCellRect(m_nCurColumn,_nCurRow,rCells);
            _rCellRect.Top()= --rCells.Top();
            impl_getCellRect(m_nCurColumn,_nPrevRow,rCells);
            _rCellRect.Bottom()=rCells.Bottom();
        }
        m_pDataWindow->Invalidate(_rCellRect);
    }
    //-------------------------------------------------------------------------------
    //this method is to be called, when a new row is added
    void TableControl_Impl::invalidateRow(RowPos _nRowPos, Rectangle& _rCellRect)
    {
        if(m_nCurRow < 0)
            m_nCurRow = 0;
        if(m_nCursorHidden == 2)
            --m_nCursorHidden;
        impl_getAllVisibleCellsArea( _rCellRect );
        TableRowGeometry _rRow( *this, _rCellRect, _nRowPos);
        impl_ni_updateScrollbars();
        m_pDataWindow->Invalidate(_rRow.getRect());
    }
    //-------------------------------------------------------------------------------
    std::vector<RowPos>& TableControl_Impl::getSelectedRows()
    {
        return m_nRowSelected;
    }
    //--------------------------------------------------------------------
    void TableControl_Impl::clearSelection()
    {
        m_nRowSelected.clear();
    }
    //--------------------------------------------------------------------
    //-------------------------------------------------------------------------------
    void TableControl_Impl::removeSelectedRow(RowPos _nRowPos)
    {
        int i =0;
        //if the row is selected, remove it from the selection vector
        if(isRowSelected(m_nRowSelected, _nRowPos))
        {
            if(m_nRowSelected.size()>1)
                m_nRowSelected.erase(m_nRowSelected.begin()+_nRowPos);
            else
                m_nRowSelected.clear();
        }
        //after removing a row, row positions must be updated, so selected rows could stay selected
        if(m_nRowSelected.size()>1)
        {
            for(std::vector<RowPos>::iterator it=m_nRowSelected.begin();it!=m_nRowSelected.end();++it)
            {
                if(*it > _nRowPos)
                    m_nRowSelected[i]=*it-1;
                ++i;
            }
        }
        if(_nRowPos == 0)
            m_nCurRow = 0;
        else
            m_nCurRow = _nRowPos-1;
    }
    //------------------------------------------------------------------------------
    void TableControl_Impl::invalidateRows()
    {
        impl_ni_updateScrollbars();
        TableSize nVisibleRows = impl_getVisibleRows(true);
        TableSize nVisibleCols = impl_getVisibleColumns(true);
        if(m_nTopRow+nVisibleRows>m_nRowCount && m_nRowCount>=nVisibleRows)
            m_nTopRow--;
        else
            m_nTopRow = 0;
        if(m_nLeftColumn+nVisibleCols>m_nColumnCount && m_nColumnCount>=nVisibleCols)
            m_nLeftColumn--;
        else
            m_nLeftColumn = 0;
        m_pDataWindow->Invalidate();
    }

    //--------------------------------------------------------------------
    TableSize TableControl_Impl::impl_getVisibleRows( bool _bAcceptPartialRow ) const
    {
        DBG_CHECK_ME();

        DBG_ASSERT( m_pDataWindow, "TableControl_Impl::impl_getVisibleRows: no data window!" );

        return lcl_getRowsFittingInto(
            m_pDataWindow->GetOutputSizePixel().Height() - m_nColHeaderHeightPixel,
            m_nRowHeightPixel,
            _bAcceptPartialRow
        );
    }

    //--------------------------------------------------------------------
    TableSize TableControl_Impl::impl_getVisibleColumns( bool _bAcceptPartialRow ) const
    {
        DBG_CHECK_ME();

        DBG_ASSERT( m_pDataWindow, "TableControl_Impl::impl_getVisibleColumns: no data window!" );

        return lcl_getColumnsVisibleWithin(
            Rectangle( Point( 0, 0 ), m_pDataWindow->GetOutputSizePixel() ),
            m_nLeftColumn,
            *this,
            _bAcceptPartialRow
        );
    }

    //--------------------------------------------------------------------
    bool TableControl_Impl::goTo( ColPos _nColumn, RowPos _nRow )
    {
        DBG_CHECK_ME();

        // TODO: give veto listeners a chance

        if  (  ( _nColumn < -1 ) || ( _nColumn >= m_nColumnCount )
            || ( _nRow < -1 ) || ( _nRow >= m_nRowCount )
            )
            return false;

        TempHideCursor aHideCursor( *this );
        m_nCurColumn = _nColumn;
        m_nCurRow = _nRow;

        // ensure that the new cell is visible
        ensureVisible( m_nCurColumn, m_nCurRow, false );
        return true;
    }

    //--------------------------------------------------------------------
    void TableControl_Impl::ensureVisible( ColPos _nColumn, RowPos _nRow, bool _bAcceptPartialVisibility )
    {
        DBG_CHECK_ME();
        DBG_ASSERT( ( _nColumn >= 0 ) && ( _nColumn < m_nColumnCount )
                 && ( _nRow >= 0 ) && ( _nRow < m_nRowCount ),
                 "TableControl_Impl::ensureVisible: invalid coordinates!" );

        TempHideCursor aHideCursor( *this );

        if ( _nColumn < m_nLeftColumn )
            impl_ni_ScrollColumns( _nColumn - m_nLeftColumn );
        else
        {
            TableSize nVisibleColumns = impl_getVisibleColumns( _bAcceptPartialVisibility );
            if ( _nColumn > m_nLeftColumn + nVisibleColumns - 1 )
            {
                impl_ni_ScrollColumns( _nColumn - ( m_nLeftColumn + nVisibleColumns - 1 ) );
                // TODO: since not all columns have the same width, this might in theory result
                // in the column still not being visible.
            }
        }

        if ( _nRow < m_nTopRow )
            impl_ni_ScrollRows( _nRow - m_nTopRow );
        else
        {
            TableSize nVisibleRows = impl_getVisibleRows( _bAcceptPartialVisibility );
            if ( _nRow > m_nTopRow + nVisibleRows - 1 )
                impl_ni_ScrollRows( _nRow - ( m_nTopRow + nVisibleRows - 1 ) );
        }
    }

    //--------------------------------------------------------------------
    TableSize TableControl_Impl::impl_ni_ScrollRows( TableSize _nRowDelta )
    {
        // compute new top row
        RowPos nNewTopRow =
            ::std::max(
                ::std::min( (RowPos)( m_nTopRow + _nRowDelta ), (RowPos)( m_nRowCount - 1 ) ),
                (RowPos)0
            );

        RowPos nOldTopRow = m_nTopRow;
        m_nTopRow = nNewTopRow;

        // if updates are enabled currently, scroll the viewport
        if ( m_rAntiImpl.IsUpdateMode() && ( m_nTopRow != nOldTopRow ) )
        {
            DBG_SUSPEND_INV( INV_SCROLL_POSITION );
            TempHideCursor aHideCursor( *this );
            // TODO: call a onStartScroll at our listener (or better an own onStartScroll,
            // which hides the cursor and then calls the listener)
            // Same for onEndScroll

            // scroll the view port, if possible
            long nPixelDelta = m_nRowHeightPixel * ( m_nTopRow - nOldTopRow );

            Rectangle aDataArea( Point( 0, m_nColHeaderHeightPixel ), m_pDataWindow->GetOutputSizePixel() );

            if  (   m_pDataWindow->GetBackground().IsScrollable()
                &&  abs( nPixelDelta ) < aDataArea.GetHeight()
                )
            {
                m_pDataWindow->Scroll( 0, (long)-nPixelDelta, aDataArea, SCROLL_CLIP | SCROLL_UPDATE | SCROLL_CHILDREN);
            }
            else
                m_pDataWindow->Invalidate( INVALIDATE_UPDATE );

            // update the position at the vertical scrollbar
            m_pVScroll->SetThumbPos( m_nTopRow );
        }

        return (TableSize)( m_nTopRow - nOldTopRow );
    }

    //--------------------------------------------------------------------
    TableSize TableControl_Impl::impl_ni_ScrollColumns( TableSize _nColumnDelta )
    {
        // compute new left column
        ColPos nNewLeftColumn =
            ::std::max(
                ::std::min( (ColPos)( m_nLeftColumn + _nColumnDelta ), (ColPos)( m_nColumnCount - 1 ) ),
                (ColPos)0
            );

        ColPos nOldLeftColumn = m_nLeftColumn;
        m_nLeftColumn = nNewLeftColumn;

        // if updates are enabled currently, scroll the viewport
        if ( m_rAntiImpl.IsUpdateMode() && ( m_nLeftColumn != nOldLeftColumn ) )
        {
            DBG_SUSPEND_INV( INV_SCROLL_POSITION );
            TempHideCursor aHideCursor( *this );
            // TODO: call a onStartScroll at our listener (or better an own onStartScroll,
            // which hides the cursor and then calls the listener)
            // Same for onEndScroll

            // scroll the view port, if possible
            Rectangle aDataArea( Point( m_nRowHeaderWidthPixel, 0 ), m_pDataWindow->GetOutputSizePixel() );

            long nPixelDelta =
                ( m_nLeftColumn > 0 ? m_aAccColumnWidthsPixel[ m_nLeftColumn - 1 ] : 0 )
            -   ( nOldLeftColumn > 0 ? m_aAccColumnWidthsPixel[ nOldLeftColumn - 1 ] : 0 );

            if  (   m_pDataWindow->GetBackground().IsScrollable()
                &&  abs( nPixelDelta ) < aDataArea.GetWidth()
                )
            {
                m_pDataWindow->Scroll( (long)-nPixelDelta, 0, aDataArea, SCROLL_CLIP | SCROLL_UPDATE );
            }
            else
                m_pDataWindow->Invalidate( INVALIDATE_UPDATE );

            // update the position at the horizontal scrollbar
            m_pHScroll->SetThumbPos( m_nLeftColumn );
        }

        return (TableSize)( m_nLeftColumn - nOldLeftColumn );
    }
    //-------------------------------------------------------------------------------
    SelectionEngine* TableControl_Impl::getSelEngine()
    {
        return m_pSelEngine;
    }
    //-------------------------------------------------------------------------------
    TableDataWindow* TableControl_Impl::getDataWindow()
    {
        return m_pDataWindow;
    }
    //-------------------------------------------------------------------------------
    ScrollBar* TableControl_Impl::getHorzScrollbar()
    {
        return m_pHScroll;
    }
    //-------------------------------------------------------------------------------
    ScrollBar* TableControl_Impl::getVertScrollbar()
    {
        return m_pVScroll;
    }
    //-------------------------------------------------------------------------------
    BOOL TableControl_Impl::isRowSelected(const ::std::vector<RowPos>& selectedRows, RowPos current)
    {
        return ::std::find(selectedRows.begin(),selectedRows.end(),current) != selectedRows.end();
    }
        //-------------------------------------------------------------------------------
    bool TableControl_Impl::isRowSelected(RowPos current)
    {
        return ::std::find(m_nRowSelected.begin(),m_nRowSelected.end(),current) != m_nRowSelected.end();
    }
    //-------------------------------------------------------------------------------
    int TableControl_Impl::getRowSelectedNumber(const ::std::vector<RowPos>& selectedRows, RowPos current)
    {
        std::vector<RowPos>::const_iterator it = ::std::find(selectedRows.begin(),selectedRows.end(),current);
        if ( it != selectedRows.end() )
        {
            return it - selectedRows.begin();
        }
        return -1;
    }
    //-------------------------------------------------------------------------------
    bool TableControl_Impl::isTooltipActive()
    {
        return m_rAntiImpl.isTooltip();
    }
    //-------------------------------------------------------------------------------
    ::rtl::OUString& TableControl_Impl::setTooltip(const Point& rPoint )
    {
        ::rtl::OUString aTooltipText;
        RowPos current = getCurrentRow(rPoint);
        com::sun::star::uno::Sequence< sal_Int32 > cols = m_rAntiImpl.getColumnsForTooltip();
        com::sun::star::uno::Sequence< ::rtl::OUString > text = m_rAntiImpl.getTextForTooltip();
        if(text.getLength()==0 && cols.getLength()==0)
        {
            ::com::sun::star::uno::Any content = m_pModel->getCellContent()[current][m_nCurColumn];
            aTooltipText = convertToString(content);
        }
        else if(text.getLength() == 0)
        {
            for(int i=0; i<cols.getLength(); i++)
            {
                if(i==0)
                {
                    ::com::sun::star::uno::Any content = m_pModel->getCellContent()[current][cols[i]];
                    aTooltipText = convertToString(content);
                }
                else
                {
                    aTooltipText+= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "\n" ));
                    ::com::sun::star::uno::Any content = m_pModel->getCellContent()[current][cols[i]];
                    aTooltipText += convertToString(content);
                }
            }
        }
        else if(cols.getLength() == 0)
        {
            for(int i=0; i<text.getLength(); i++)
            {
                if(i==0)
                    aTooltipText = text[i];
                else
                {
                    aTooltipText+= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "\n" ));
                    aTooltipText+= text[i];
                }
            }
        }
        else
        {
            int nCols = cols.getLength();
            int mText = text.getLength();
            if(nCols < mText )
                cols.realloc(mText);
            else if(mText < nCols)
                text.realloc(nCols);
            for(int i=0; i<cols.getLength(); i++)
            {
                if(i==0)
                {
                    ::com::sun::star::uno::Any content = m_pModel->getCellContent()[current][cols[i]];
                    aTooltipText = text[i] + convertToString(content);
                }
                else
                {
                    aTooltipText+= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "\n" ));
                    aTooltipText+= text[i];
                    if(nCols > i)
                    {
                        ::com::sun::star::uno::Any content = m_pModel->getCellContent()[current][cols[i]];
                        ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >xGraphic;
                        aTooltipText += convertToString(content);
                    }
                }
            }
        }
        return m_aTooltipText = aTooltipText;
    }
    //--------------------------------------------------------------------
    void TableControl_Impl::resizeColumn(const Point& rPoint)
    {
        Pointer aNewPointer(POINTER_ARROW);
        int resizingColumn=m_nCurColumn-m_nLeftColumn;
        PColumnModel pColumn = m_pModel->getColumnModel(m_nCurColumn);
        impl_ni_getAccVisibleColWidths();
        int newColWidth = m_aColumnWidthsPixel[m_nCurColumn];
        //subtract 1 from m_aAccColumnWidthPixel because right border should be part of the current cell
        if(m_aVisibleColumnWidthsPixel[resizingColumn]-1 == rPoint.X() && pColumn->isResizable())
            aNewPointer = Pointer( POINTER_HSPLIT );
        //MouseButton was pressed but not yet released, mouse is moving
        if(m_bResizing)
        {
            if(rPoint.X() > m_pDataWindow->GetOutputSizePixel().Width() || rPoint.X() < m_aVisibleColumnWidthsPixel[resizingColumn]-newColWidth)
                aNewPointer = Pointer( POINTER_NOTALLOWED);
            else
                aNewPointer = Pointer( POINTER_HSPLIT );
            m_pDataWindow->HideTracking();
            int lineHeight = 0;
            if(m_pModel->hasColumnHeaders())
                lineHeight+= m_nColHeaderHeightPixel;
            lineHeight+=m_nRowHeightPixel*m_nRowCount;
            int gridHeight = m_pDataWindow->GetOutputSizePixel().Height();
            if(lineHeight >= gridHeight)
                lineHeight = gridHeight;
            m_pDataWindow->ShowTracking(Rectangle(Point(rPoint.X(),0), Size(1, lineHeight )),
                SHOWTRACK_SPLIT | SHOWTRACK_WINDOW);
        }
        m_pDataWindow->SetPointer(aNewPointer);
    }
    //--------------------------------------------------------------------
    bool TableControl_Impl::startResizeColumn(const Point& rPoint)
    {
        m_bResizingGrid = false;
        m_nResizingColumn = m_nCurColumn;
        PColumnModel pColumn = m_pModel->getColumnModel(m_nResizingColumn);
        if(m_aVisibleColumnWidthsPixel[m_nResizingColumn-m_nLeftColumn]-1 == rPoint.X() && pColumn->isResizable())
        {
            m_pDataWindow->CaptureMouse();
            m_bResizing = true;
        }
        return m_bResizing;
    }
    //--------------------------------------------------------------------
    bool TableControl_Impl::endResizeColumn(const Point& rPoint)
    {
        if(m_bResizing)
        {
            m_pDataWindow->HideTracking();
            PColumnModel pColumn = m_pModel->getColumnModel(m_nResizingColumn);
            int maxWidth = m_rAntiImpl.LogicToPixel( Size( pColumn->getMaxWidth(), 0 ), MAP_APPFONT ).Width();
            int minWidth = m_rAntiImpl.LogicToPixel( Size( pColumn->getMinWidth(), 0 ), MAP_APPFONT ).Width();
            int resizeCol = m_nResizingColumn-m_nLeftColumn;
            //new position of mouse
            int actX = rPoint.X();
            //old position of right border
            int oldX = m_aVisibleColumnWidthsPixel[resizeCol];
            //position of left border if cursor in the first cell
            int leftX = 0;
            if(m_nResizingColumn > m_nLeftColumn)
                leftX = m_aVisibleColumnWidthsPixel[resizeCol-1];
            else if(m_nResizingColumn == m_nLeftColumn && m_pModel->hasRowHeaders())
                leftX = m_rAntiImpl.LogicToPixel( Size( m_pModel->getRowHeaderWidth(), 0 ), MAP_APPFONT ).Width();
            int actWidth = actX - leftX;
            int newActWidth = 0;
            //minimize the column width
            if(oldX > actX && actX >= leftX)
            {
                if(minWidth < actWidth)
                {
                    newActWidth = m_rAntiImpl.PixelToLogic( Size( actWidth, 0 ), MAP_APPFONT ).Width();
                    pColumn->setPreferredWidth(newActWidth);
                }
                else
                    pColumn->setPreferredWidth(pColumn->getMinWidth());
                if(m_nLeftColumn != 0)
                    impl_updateLeftColumn();
        }
        else if(oldX < actX)
        {
            if(actWidth < maxWidth)
            {
                newActWidth = m_rAntiImpl.PixelToLogic( Size( actWidth, 0 ), MAP_APPFONT ).Width();
                pColumn->setPreferredWidth(newActWidth);
            }
            else
                pColumn->setPreferredWidth(pColumn->getMaxWidth());
        }
        m_nCurColumn = m_nResizingColumn;
        impl_ni_updateColumnWidths();
        impl_ni_updateScrollbars();
        m_pDataWindow->Invalidate(INVALIDATE_UPDATE);
        m_pDataWindow->SetPointer(Pointer());
        m_bResizing = false;
        m_bResizingGrid = true;
    }
    m_pDataWindow->ReleaseMouse();
    return m_bResizing;
    }
    //-------------------------------------------------------------------------------
    void TableControl_Impl::impl_ni_getAccVisibleColWidths()
    {
        TableSize nVisCols = impl_getVisibleColumns(true);
        int widthsPixel = 0;
        m_aVisibleColumnWidthsPixel.resize(0);
        m_aVisibleColumnWidthsPixel.reserve(nVisCols);
        int headerRowWidth = 0;
        if(m_pModel->hasRowHeaders())
        {
            headerRowWidth = m_rAntiImpl.LogicToPixel( Size(m_pModel->getRowHeaderWidth(), 0 ), MAP_APPFONT ).Width();
            widthsPixel+=headerRowWidth;
        }
        int col = m_nLeftColumn;
        while(nVisCols)
        {
            m_aVisibleColumnWidthsPixel.push_back(widthsPixel+=m_aColumnWidthsPixel[col]);
            col++;
            nVisCols--;
        }
    }
    //-------------------------------------------------------------------------------
    void TableControl_Impl::impl_updateLeftColumn()
    {
        int nVisCols = m_aVisibleColumnWidthsPixel.size();
        int headerRowWidth = 0;
        //sum of currently visible columns
        int widthsPixel = 0;
        //header pixel should be added, because header doesn't vanish when scrolling
        if(m_pModel->hasRowHeaders())
        {
            headerRowWidth = m_rAntiImpl.LogicToPixel( Size(m_pModel->getRowHeaderWidth(), 0 ), MAP_APPFONT ).Width();
            widthsPixel+=headerRowWidth;
        }
        int col = m_nLeftColumn;
        //add column width of the neighbour of the left column
        widthsPixel+=m_aColumnWidthsPixel[col-1];
        //compute the sum of the new column widths
        while(nVisCols)
        {
            PColumnModel pColumn = m_pModel->getColumnModel(col);
            int colWidth = pColumn->getWidth();
            int colPrefWidth = pColumn->getPreferredWidth();
            if(colPrefWidth!=0)
                colWidth = colPrefWidth;
            widthsPixel += m_rAntiImpl.LogicToPixel( Size( colWidth, 0 ), MAP_APPFONT ).Width();
            col++;
            nVisCols--;
        }
        //when the sum of all visible columns and the next to the left column is smaller than
        //window width, then update m_nLeftColumn
        if(widthsPixel<m_pDataWindow->GetOutputSizePixel().Width())
            m_nLeftColumn--;
    }
    //--------------------------------------------------------------------
    rtl::OUString TableControl_Impl::convertToString(const ::com::sun::star::uno::Any& value)
    {
        sal_Int32 nInt = 0;
        sal_Bool bBool = false;
        double fDouble = 0;
        ::rtl::OUString sNewString;
        ::rtl::OUString sConvertString;
        if(value >>= sConvertString)
            sNewString = sConvertString;
        else if(value >>= nInt)
            sNewString = sConvertString.valueOf(nInt);
        else if(value >>= bBool)
            sNewString = sConvertString.valueOf(bBool);
        else if(value >>= fDouble)
            sNewString = sConvertString.valueOf(fDouble);
        return sNewString;
    }
    Rectangle TableControl_Impl::calcHeaderRect(bool bColHeader)
    {
        Rectangle aRectTable, aRectTableWithHeaders;
        impl_getAllVisibleDataCellArea(aRectTable);
        impl_getAllVisibleCellsArea(aRectTableWithHeaders);
        Size aSizeTable(aRectTable.GetSize());
        Size aSizeTableWithHeaders(aRectTableWithHeaders.GetSize());
        if(bColHeader)
            return Rectangle(aRectTableWithHeaders.TopLeft(),Size(aSizeTableWithHeaders.Width()-aSizeTable.Width(), aSizeTableWithHeaders.Height()));
        else
            return Rectangle(aRectTableWithHeaders.TopLeft(),Size(aSizeTableWithHeaders.Width(), aSizeTableWithHeaders.Height()-aSizeTable.Height()));
    }
    Rectangle TableControl_Impl::calcTableRect()
    {
        Rectangle aRect;
        impl_getAllVisibleDataCellArea(aRect);
        return aRect;
    }

    //--------------------------------------------------------------------
    IMPL_LINK( TableControl_Impl, OnScroll, ScrollBar*, _pScrollbar )
    {
        DBG_ASSERT( ( _pScrollbar == m_pVScroll ) || ( _pScrollbar == m_pHScroll ),
            "TableControl_Impl::OnScroll: where did this come from?" );

        if ( _pScrollbar == m_pVScroll )
            impl_ni_ScrollRows( _pScrollbar->GetDelta() );
        else
            impl_ni_ScrollColumns( _pScrollbar->GetDelta() );

        return 0L;
    }
    //---------------------------------------------------------------------------------------
    TableFunctionSet::TableFunctionSet(TableControl_Impl* _pTableControl)
        :m_pTableControl( _pTableControl)
    ,m_nCurrentRow (-2)
    {
    }
    //-------------------------------------------------------------------------------
    TableFunctionSet::~TableFunctionSet()
    {
    }
    //-------------------------------------------------------------------------------
    void TableFunctionSet::BeginDrag()
    {
    }
    //-------------------------------------------------------------------------------
    void TableFunctionSet::CreateAnchor()
    {
        m_pTableControl->m_nAnchor = m_pTableControl->m_nCurRow;
    }
    //-------------------------------------------------------------------------------
    void TableFunctionSet::DestroyAnchor()
    {
        m_pTableControl->m_nAnchor = -1;
    }
    //-------------------------------------------------------------------------------
    BOOL TableFunctionSet::SetCursorAtPoint(const Point& rPoint, BOOL bDontSelectAtCursor)
    {
        BOOL bHandled = FALSE;
        Rectangle rCells;
        //curRow is the row where the mouse click happened, m_nCurRow is the last selected row, before the mouse click
        RowPos curRow = m_pTableControl->getCurrentRow(rPoint);
        if(curRow == -2)
            return FALSE;
        if( bDontSelectAtCursor )
        {
            if(m_pTableControl->m_nRowSelected.size()>1)
                m_pTableControl->m_pSelEngine->AddAlways(TRUE);
            bHandled = TRUE;
        }
        else if(m_pTableControl->m_nAnchor == m_pTableControl->m_nCurRow)
        {
            //selecting region,
            int diff = m_pTableControl->m_nCurRow - curRow;
            //selected region lies above the last selection
            if( diff >= 0)
            {
                //put selected rows in vector
                while(m_pTableControl->m_nAnchor>=curRow)
                {
                    bool isAlreadySelected = m_pTableControl->isRowSelected(m_pTableControl->m_nRowSelected, m_pTableControl->m_nAnchor);
                    //if row isn't selected, put it in vector, otherwise don't put it there, because it will be twice there
                    if(!isAlreadySelected)
                        m_pTableControl->m_nRowSelected.push_back(m_pTableControl->m_nAnchor);
                    m_pTableControl->m_nAnchor--;
                    diff--;
                }
                m_pTableControl->m_nAnchor++;
            }
            //selected region lies beneath the last selected row
            else
            {
                while(m_pTableControl->m_nAnchor<=curRow)
                {
                    bool isAlreadySelected = m_pTableControl->isRowSelected(m_pTableControl->m_nRowSelected, m_pTableControl->m_nAnchor);
                    if(!isAlreadySelected)
                        m_pTableControl->m_nRowSelected.push_back(m_pTableControl->m_nAnchor);
                    m_pTableControl->m_nAnchor++;
                    diff++;
                }
                m_pTableControl->m_nAnchor--;
            }
            m_pTableControl->invalidateSelectedRegion(m_pTableControl->m_nCurRow, curRow, rCells);
            bHandled = TRUE;
        }
        //no region selected
        else
        {
            if(m_pTableControl->m_nRowSelected.empty())
                m_pTableControl->m_nRowSelected.push_back(curRow);
            else
            {
                if(m_pTableControl->m_pSelEngine->GetSelectionMode()==SINGLE_SELECTION)
                {
                    DeselectAll();
                    m_pTableControl->m_nRowSelected.push_back(curRow);
                }
                else
                {
                    bool isAlreadySelected = m_pTableControl->isRowSelected(m_pTableControl->m_nRowSelected, curRow);
                    if(!isAlreadySelected)
                        m_pTableControl->m_nRowSelected.push_back(curRow);
                }
            }
            if(m_pTableControl->m_nRowSelected.size()>1 && m_pTableControl->m_pSelEngine->GetSelectionMode()!=SINGLE_SELECTION)
                m_pTableControl->m_pSelEngine->AddAlways(TRUE);
            m_pTableControl->invalidateSelectedRegion(curRow, curRow, rCells);
            bHandled = TRUE;
        }
        m_pTableControl->m_nCurRow = curRow;
        m_pTableControl->ensureVisible(m_pTableControl->m_nCurColumn,m_pTableControl->m_nCurRow,false);
        return bHandled;
    }
    //-------------------------------------------------------------------------------
    BOOL TableFunctionSet::IsSelectionAtPoint( const Point& rPoint )
    {
        m_pTableControl->m_pSelEngine->AddAlways(FALSE);
        if(m_pTableControl->m_nRowSelected.empty())
            return FALSE;
        else
        {
            RowPos curRow = m_pTableControl->getCurrentRow(rPoint);
            m_pTableControl->m_nAnchor = -1;
            bool selected = m_pTableControl->isRowSelected(m_pTableControl->m_nRowSelected, curRow);
            m_nCurrentRow = curRow;
            return selected;
        }
    }
    //-------------------------------------------------------------------------------
    void TableFunctionSet::DeselectAtPoint( const Point& rPoint )
    {
        (void)rPoint;
        long pos = 0;
        long i = 0;
        Rectangle rCells;
        for(std::vector<RowPos>::iterator it=m_pTableControl->m_nRowSelected.begin();
            it!=m_pTableControl->m_nRowSelected.end();++it)
        {
            if(*it == m_nCurrentRow)
            {
                pos = i;
                m_pTableControl->invalidateSelectedRegion(*it, *it, rCells);
            }
            ++i;
        }
        m_pTableControl->m_nRowSelected.erase(m_pTableControl->m_nRowSelected.begin()+pos);
    }
    //-------------------------------------------------------------------------------
    void TableFunctionSet::DeselectAll()
    {
        if(!m_pTableControl->m_nRowSelected.empty())
        {
            Rectangle rCells;
            for(std::vector<RowPos>::iterator it=m_pTableControl->m_nRowSelected.begin();
                it!=m_pTableControl->m_nRowSelected.end();++it)
            {
                m_pTableControl->invalidateSelectedRegion(*it, *it, rCells);
            }
            m_pTableControl->m_nRowSelected.clear();
        }
    }

//........................................................................
} } // namespace svt::table
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
