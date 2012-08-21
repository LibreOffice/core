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


#include "svtools/table/tablecontrol.hxx"

#include "tablegeometry.hxx"
#include "tablecontrol_impl.hxx"
#include "tabledatawindow.hxx"

#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <tools/diagnose_ex.h>

using namespace ::com::sun::star::uno;
using ::com::sun::star::accessibility::XAccessible;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::lang;
using namespace utl;
//......................................................................................................................
namespace svt { namespace table
{
//......................................................................................................................

    //==================================================================================================================
    //= TableControl
    //==================================================================================================================
    // -----------------------------------------------------------------------------------------------------------------
    TableControl::TableControl( Window* _pParent, WinBits _nStyle )
        :Control( _pParent, _nStyle )
        ,m_pImpl( new TableControl_Impl( *this ) )
    {
        TableDataWindow& rDataWindow = m_pImpl->getDataWindow();
        rDataWindow.SetMouseButtonDownHdl( LINK( this, TableControl, ImplMouseButtonDownHdl ) );
        rDataWindow.SetMouseButtonUpHdl( LINK( this, TableControl, ImplMouseButtonUpHdl ) );
        rDataWindow.SetSelectHdl( LINK( this, TableControl, ImplSelectHdl ) );

        // by default, use the background as determined by the style settings
        const Color aWindowColor( GetSettings().GetStyleSettings().GetFieldColor() );
        SetBackground( Wallpaper( aWindowColor ) );
        SetFillColor( aWindowColor );
    }

    // -----------------------------------------------------------------------------------------------------------------
    TableControl::~TableControl()
    {
        ImplCallEventListeners( VCLEVENT_OBJECT_DYING );

        m_pImpl->setModel( PTableModel() );
        m_pImpl->disposeAccessible();
        m_pImpl.reset();
    }

    // -----------------------------------------------------------------------------------------------------------------
    void TableControl::GetFocus()
    {
        if ( !m_pImpl->getInputHandler()->GetFocus( *m_pImpl ) )
            Control::GetFocus();
    }

    // -----------------------------------------------------------------------------------------------------------------
    void TableControl::LoseFocus()
    {
        if ( !m_pImpl->getInputHandler()->LoseFocus( *m_pImpl ) )
            Control::LoseFocus();
    }

    // -----------------------------------------------------------------------------------------------------------------
    void TableControl::KeyInput( const KeyEvent& rKEvt )
    {
        if ( !m_pImpl->getInputHandler()->KeyInput( *m_pImpl, rKEvt ) )
            Control::KeyInput( rKEvt );
    }


    // -----------------------------------------------------------------------------------------------------------------
    void TableControl::StateChanged( StateChangedType i_nStateChange )
    {
        Control::StateChanged( i_nStateChange );

        // forward certain settings to the data window
        switch ( i_nStateChange )
        {
        case STATE_CHANGE_CONTROL_FOCUS:
            m_pImpl->invalidateSelectedRows();
            break;

        case STATE_CHANGE_CONTROLBACKGROUND:
            if ( IsControlBackground() )
                getDataWindow().SetControlBackground( GetControlBackground() );
            else
                getDataWindow().SetControlBackground();
            break;

        case STATE_CHANGE_CONTROLFOREGROUND:
            if ( IsControlForeground() )
                getDataWindow().SetControlForeground( GetControlForeground() );
            else
                getDataWindow().SetControlForeground();
            break;

        case STATE_CHANGE_CONTROLFONT:
            if ( IsControlFont() )
                getDataWindow().SetControlFont( GetControlFont() );
            else
                getDataWindow().SetControlFont();
            break;
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    void TableControl::Resize()
    {
        Control::Resize();
        m_pImpl->onResize();
    }

    // -----------------------------------------------------------------------------------------------------------------
    void TableControl::SetModel( PTableModel _pModel )
    {
        m_pImpl->setModel( _pModel );
    }

    // -----------------------------------------------------------------------------------------------------------------
    PTableModel TableControl::GetModel() const
    {
        return m_pImpl->getModel();
    }

    // -----------------------------------------------------------------------------------------------------------------
    sal_Int32 TableControl::GetCurrentRow() const
    {
        return m_pImpl->getCurrentRow();
    }

    // -----------------------------------------------------------------------------------------------------------------
    sal_Int32 TableControl::GetCurrentColumn() const
    {
        return m_pImpl->getCurrentColumn();
    }

    // -----------------------------------------------------------------------------------------------------------------
    sal_Bool TableControl::GoToCell(sal_Int32 _nColPos, sal_Int32 _nRowPos)
    {
        return m_pImpl->goTo( _nColPos, _nRowPos );
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Int32 TableControl::GetSelectedRowCount() const
    {
        return sal_Int32( m_pImpl->getSelectedRowCount() );
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Int32 TableControl::GetSelectedRowIndex( sal_Int32 const i_selectionIndex ) const
    {
        return sal_Int32( m_pImpl->getSelectedRowIndex( i_selectionIndex ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    bool TableControl::IsRowSelected( sal_Int32 const i_rowIndex ) const
    {
        return m_pImpl->isRowSelected( i_rowIndex );
    }

    // -----------------------------------------------------------------------------------------------------------------
    void TableControl::SelectRow( RowPos const i_rowIndex, bool const i_select )
    {
        ENSURE_OR_RETURN_VOID( ( i_rowIndex >= 0 ) && ( i_rowIndex < m_pImpl->getModel()->getRowCount() ),
            "TableControl::SelectRow: no control (anymore)!" );

        if ( i_select )
        {
            if ( !m_pImpl->markRowAsSelected( i_rowIndex ) )
                // nothing to do
                return;
        }
        else
        {
            m_pImpl->markRowAsDeselected( i_rowIndex );
        }

        m_pImpl->invalidateRowRange( i_rowIndex, i_rowIndex );
        Select();
    }

    // -----------------------------------------------------------------------------------------------------------------
    void TableControl::SelectAllRows( bool const i_select )
    {
        if ( i_select )
        {
            if ( !m_pImpl->markAllRowsAsSelected() )
                // nothing to do
                return;
        }
        else
        {
            if ( !m_pImpl->markAllRowsAsDeselected() )
                // nothing to do
                return;
        }


        Invalidate();
            // TODO: can't we do better than this, and invalidate only the rows which changed?
        Select();
    }

    // -----------------------------------------------------------------------------------------------------------------
    ITableControl& TableControl::getTableControlInterface()
    {
        return *m_pImpl;
    }

    // -----------------------------------------------------------------------------------------------------------------
    SelectionEngine* TableControl::getSelEngine()
    {
        return m_pImpl->getSelEngine();
    }

    // -----------------------------------------------------------------------------------------------------------------
    Window& TableControl::getDataWindow()
    {
        return m_pImpl->getDataWindow();
    }

    // -----------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > TableControl::CreateAccessible()
    {
        Window* pParent = GetAccessibleParentWindow();
        ENSURE_OR_RETURN( pParent, "TableControl::CreateAccessible - parent not found", NULL );

        return m_pImpl->getAccessible( *pParent );
    }

    // -----------------------------------------------------------------------------------------------------------------
    Reference<XAccessible> TableControl::CreateAccessibleControl( sal_Int32 _nIndex )
    {
        (void)_nIndex;
        DBG_ASSERT( sal_False, "TableControl::CreateAccessibleControl: to be overwritten!" );
        return NULL;
    }

    // -----------------------------------------------------------------------------------------------------------------
    ::rtl::OUString TableControl::GetAccessibleObjectName( AccessibleTableControlObjType eObjType, sal_Int32 _nRow, sal_Int32 _nCol) const
    {
        ::rtl::OUString aRetText;
        //Window* pWin;
        switch( eObjType )
        {
            case TCTYPE_GRIDCONTROL:
                aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "GridControl" ) );
                break;
            case TCTYPE_TABLE:
                aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Table" ) );
                break;
            case TCTYPE_ROWHEADERBAR:
                aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RowHeaderBar" ) );
                break;
            case TCTYPE_COLUMNHEADERBAR:
                aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ColumnHeaderBar" ) );
                break;
            case TCTYPE_TABLECELL:
                aRetText = GetAccessibleCellText(_nRow, _nCol);
                break;
            case TCTYPE_ROWHEADERCELL:
                aRetText = GetRowName(_nRow);
                break;
            case TCTYPE_COLUMNHEADERCELL:
                aRetText = GetColumnName(_nCol);
                break;
            default:
                OSL_FAIL("GridControl::GetAccessibleName: invalid enum!");
        }
        return aRetText;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString TableControl::GetAccessibleObjectDescription( AccessibleTableControlObjType eObjType, sal_Int32 ) const
    {
        ::rtl::OUString aRetText;
        switch( eObjType )
        {
            case TCTYPE_GRIDCONTROL:
                aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "GridControl description" ) );
                break;
            case TCTYPE_TABLE:
                    aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TABLE description" ) );
                break;
            case TCTYPE_ROWHEADERBAR:
                    aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ROWHEADERBAR description" ) );
                break;
            case TCTYPE_COLUMNHEADERBAR:
                    aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "COLUMNHEADERBAR description" ) );
                break;
            case TCTYPE_TABLECELL:
                    aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TABLECELL description" ) );
                break;
            case TCTYPE_ROWHEADERCELL:
                    aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ROWHEADERCELL description" ) );
                break;
            case TCTYPE_COLUMNHEADERCELL:
                    aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "COLUMNHEADERCELL description" ) );
                break;
        }
        return aRetText;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString TableControl::GetRowDescription( sal_Int32 _nRow) const
    {
        (void)_nRow;
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "row description" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString TableControl::GetRowName( sal_Int32 _nIndex) const
    {
        ::rtl::OUString sRowName;
        GetModel()->getRowHeading( _nIndex ) >>= sRowName;
        return sRowName;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString TableControl::GetColumnDescription( sal_uInt16 _nColumn) const
    {
        (void)_nColumn;
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "col description" ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString TableControl::GetColumnName( sal_Int32 _nIndex) const
    {
        return GetModel()->getColumnModel(_nIndex)->getName();
    }

    //------------------------------------------------------------------------------------------------------------------
    ::com::sun::star::uno::Any TableControl::GetCellContent( sal_Int32 _nRowPos, sal_Int32 _nColPos ) const
    {
        Any aCellContent;
        GetModel()->getCellContent( _nColPos, _nRowPos, aCellContent );
        return aCellContent;
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString TableControl::GetAccessibleCellText( sal_Int32 _nRowPos, sal_Int32 _nColPos) const
    {
        return m_pImpl->getCellContentAsString( _nRowPos, _nColPos );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl::FillAccessibleStateSet(
            ::utl::AccessibleStateSetHelper& rStateSet,
            AccessibleTableControlObjType eObjType ) const
    {
        switch( eObjType )
        {
            case TCTYPE_GRIDCONTROL:
            case TCTYPE_TABLE:

                rStateSet.AddState( AccessibleStateType::FOCUSABLE );
                rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE);
                if ( HasFocus() )
                    rStateSet.AddState( AccessibleStateType::FOCUSED );
                if ( IsActive() )
                    rStateSet.AddState( AccessibleStateType::ACTIVE );
                if ( IsEnabled() )
                    rStateSet.AddState( AccessibleStateType::ENABLED );
                if ( IsReallyVisible() )
                    rStateSet.AddState( AccessibleStateType::VISIBLE );
                rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );

                break;
            case TCTYPE_ROWHEADERBAR:
                rStateSet.AddState( AccessibleStateType::VISIBLE );
                rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
                break;
            case TCTYPE_COLUMNHEADERBAR:
                rStateSet.AddState( AccessibleStateType::VISIBLE );
                rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
                break;
            case TCTYPE_TABLECELL:
                {
                    rStateSet.AddState( AccessibleStateType::TRANSIENT );
                    rStateSet.AddState( AccessibleStateType::SELECTABLE);
                    if( GetSelectedRowCount()>0)
                        rStateSet.AddState( AccessibleStateType::SELECTED);
                }
                break;
            case TCTYPE_ROWHEADERCELL:
                rStateSet.AddState( AccessibleStateType::VISIBLE );
                rStateSet.AddState( AccessibleStateType::TRANSIENT );
                break;
            case TCTYPE_COLUMNHEADERCELL:
                rStateSet.AddState( AccessibleStateType::VISIBLE );
                break;
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle TableControl::GetWindowExtentsRelative( Window *pRelativeWindow ) const
    {
        return Control::GetWindowExtentsRelative( pRelativeWindow );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl::GrabFocus()
    {
        Control::GrabFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > TableControl::GetAccessible( sal_Bool bCreate )
    {
        return Control::GetAccessible( bCreate );
    }

    //------------------------------------------------------------------------------------------------------------------
    Window* TableControl::GetAccessibleParentWindow() const
    {
        return Control::GetAccessibleParentWindow();
    }

    //------------------------------------------------------------------------------------------------------------------
    Window* TableControl::GetWindowInstance()
    {
        return this;
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool TableControl::HasRowHeader()
    {
        return GetModel()->hasRowHeaders();
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool TableControl::HasColHeader()
    {
        return GetModel()->hasColumnHeaders();
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Int32 TableControl::GetAccessibleControlCount() const
    {
        sal_Int32 count = 0;
        if(GetRowCount()>0)
            count+=1;
        if(GetModel()->hasRowHeaders())
            count+=1;
        if(GetModel()->hasColumnHeaders())
            count+=1;
        return count;
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool TableControl::ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint )
    {
        sal_Int32 nRow = m_pImpl->getRowAtPoint( _rPoint );
        sal_Int32 nCol = m_pImpl->getColAtPoint( _rPoint );
        _rnIndex = nRow * GetColumnCount() + nCol;
        return nRow >= 0 ? sal_True : sal_False;
    }

    //------------------------------------------------------------------------------------------------------------------
    long TableControl::GetRowCount() const
    {
        return GetModel()->getRowCount();
    }

    //------------------------------------------------------------------------------------------------------------------
    long TableControl::GetColumnCount() const
    {
        return GetModel()->getColumnCount();
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool TableControl::HasRowHeader() const
    {
        return GetModel()->hasRowHeaders();
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool TableControl::ConvertPointToCellAddress( sal_Int32& _rnRow, sal_Int32& _rnColPos, const Point& _rPoint )
    {
        _rnRow = m_pImpl->getRowAtPoint( _rPoint );
        _rnColPos = m_pImpl->getColAtPoint( _rPoint );
        return _rnRow >= 0 ? sal_True : sal_False;
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl::FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const
    {
        if ( GetCurrentRow() == _nRow && GetCurrentColumn() == _nColumnPos )
            _rStateSet.AddState( AccessibleStateType::FOCUSED );
        else // only transient when column is not focused
            _rStateSet.AddState( AccessibleStateType::TRANSIENT );
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle TableControl::GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex)
    {
        (void)_nRow;
        (void)_nColumnPos;
        return GetCharacterBounds(nIndex);
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Int32 TableControl::GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint)
    {
        (void)_nRow;
        (void)_nColumnPos;
        return GetIndexForPoint(_rPoint);
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle TableControl::calcHeaderRect(sal_Bool _bIsColumnBar,sal_Bool _bOnScreen)
    {
        (void)_bOnScreen;
        return m_pImpl->calcHeaderRect( _bIsColumnBar ? false : true );
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle TableControl::calcTableRect(sal_Bool _bOnScreen)
    {
        (void)_bOnScreen;
        return m_pImpl->calcTableRect();
    }

    //------------------------------------------------------------------------------------------------------------------
    IMPL_LINK_NOARG(TableControl, ImplSelectHdl)
    {
        Select();
        return 1;
    }

    //------------------------------------------------------------------------------------------------------------------
    IMPL_LINK( TableControl, ImplMouseButtonDownHdl, MouseEvent*, pData )
    {
        CallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, pData );
        return 1;
    }

    //------------------------------------------------------------------------------------------------------------------
    IMPL_LINK( TableControl, ImplMouseButtonUpHdl, MouseEvent*, pData )
    {
         CallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONUP, pData );
        return 1;
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl::Select()
    {
        ImplCallEventListenersAndHandler( VCLEVENT_TABLEROW_SELECT, m_pImpl->getSelectHandler(), this );
    }

}} // namespace svt::table

//......................................................................................................................
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
