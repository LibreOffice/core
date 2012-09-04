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
#include <com/sun/star/accessibility/AccessibleEventId.hpp>

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

    namespace AccessibleEventId = ::com::sun::star::accessibility::AccessibleEventId;

    //==================================================================================================================
    //= TableControl
    //==================================================================================================================
    // -----------------------------------------------------------------------------------------------------------------
    TableControl::TableControl( Window* _pParent, WinBits _nStyle )
        :Control( _pParent, _nStyle )
        ,m_pImpl( new TableControl_Impl( *this ) )
    {
        TableDataWindow& rDataWindow = m_pImpl->getDataWindow();
        rDataWindow.SetSelectHdl( LINK( this, TableControl, ImplSelectHdl ) );

        // by default, use the background as determined by the style settings
        const Color aWindowColor( GetSettings().GetStyleSettings().GetFieldColor() );
        SetBackground( Wallpaper( aWindowColor ) );
        SetFillColor( aWindowColor );

        SetCompoundControl( true );
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
        else
        {
            if ( m_pImpl->isAccessibleAlive() )
            {
                m_pImpl->commitCellEvent( AccessibleEventId::STATE_CHANGED,
                                          makeAny( AccessibleStateType::FOCUSED ),
                                          Any()
                                        );
                    // Huh? What the heck? Why do we unconditionally notify a STATE_CHANGE/FOCUSED after each and every
                    // (handled) key stroke?

                m_pImpl->commitTableEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED,
                                           Any(),
                                           Any()
                                         );
                    // ditto: Why do we notify this unconditionally? We should find the right place to notify the
                    // ACTIVE_DESCENDANT_CHANGED event.
                    // Also, we should check if STATE_CHANGED/FOCUSED is really necessary: finally, the children are
                    // transient, aren't they?
            }
        }
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
    bool TableControl::GoTo( ColPos _nColumn, RowPos _nRow )
    {
        return m_pImpl->goTo( _nColumn, _nRow );
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
                aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Grid control" ) );
                break;
            case TCTYPE_TABLE:
                aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Grid conrol" ) );
                break;
            case TCTYPE_ROWHEADERBAR:
                aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "RowHeaderBar" ) );
                break;
            case TCTYPE_COLUMNHEADERBAR:
                aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ColumnHeaderBar" ) );
                break;
            case TCTYPE_TABLECELL:
                //the name of the cell constists of column name and row name if defined
                //if the name is equal to cell content, it'll be read twice
                if(GetModel()->hasColumnHeaders())
                {
                    aRetText = GetColumnName(_nCol);
                    aRetText += rtl::OUString::createFromAscii(" , ");
                }
                if(GetModel()->hasRowHeaders())
                {
                    aRetText += GetRowName(_nRow);
                    aRetText += rtl::OUString::createFromAscii(" , ");
                }
                //aRetText = GetAccessibleCellText(_nRow, _nCol);
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
                aRetText = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Grid control description" ) );
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
                // the description of the cell consists of column name and row name if defined
                // if the name is equal to cell content, it'll be read twice
                if ( GetModel()->hasColumnHeaders() )
                {
                    aRetText = GetColumnName( GetCurrentColumn() );
                    aRetText += rtl::OUString::createFromAscii( " , " );
                }
                if ( GetModel()->hasRowHeaders() )
                {
                    aRetText += GetRowName( GetCurrentRow() );
                }
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

                if ( m_pImpl->getSelEngine()->GetSelectionMode() == MULTIPLE_SELECTION )
                    rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE);

                if ( HasChildPathFocus() )
                    rStateSet.AddState( AccessibleStateType::FOCUSED );

                if ( IsActive() )
                    rStateSet.AddState( AccessibleStateType::ACTIVE );

                if ( m_pImpl->getDataWindow().IsEnabled() )
                {
                    rStateSet.AddState( AccessibleStateType::ENABLED );
                    rStateSet.AddState( AccessibleStateType::SENSITIVE );
                }

                if ( IsReallyVisible() )
                    rStateSet.AddState( AccessibleStateType::VISIBLE );

                if ( eObjType == TCTYPE_TABLE )
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
                    rStateSet.AddState( AccessibleStateType::FOCUSABLE );
                    if ( HasChildPathFocus() )
                        rStateSet.AddState( AccessibleStateType::FOCUSED );
                    rStateSet.AddState( AccessibleStateType::ACTIVE );
                    rStateSet.AddState( AccessibleStateType::TRANSIENT );
                    rStateSet.AddState( AccessibleStateType::SELECTABLE);
                    rStateSet.AddState( AccessibleStateType::VISIBLE );
                    rStateSet.AddState( AccessibleStateType::SHOWING );
                    if ( IsRowSelected( GetCurrentRow() ) )
                        // Hmm? Wouldn't we expect the affected row to be a parameter to this function?
                        rStateSet.AddState( AccessibleStateType::SELECTED );
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
    void TableControl::commitCellEventIfAccessibleAlive( sal_Int16 const i_eventID, const Any& i_newValue, const Any& i_oldValue )
    {
        if ( m_pImpl->isAccessibleAlive() )
            m_pImpl->commitCellEvent( i_eventID, i_newValue, i_oldValue );
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl::commitTableEventIfAccessibleAlive( sal_Int16 const i_eventID, const Any& i_newValue, const Any& i_oldValue )
    {
        if ( m_pImpl->isAccessibleAlive() )
            m_pImpl->commitTableEvent( i_eventID, i_newValue, i_oldValue );
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
        // TC_TABLE is always defined, no matter whether empty or not
        sal_Int32 count = 1;
        if ( GetModel()->hasRowHeaders() )
            ++count;
        if ( GetModel()->hasColumnHeaders() )
            ++count;
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
        if ( IsRowSelected( _nRow ) )
            _rStateSet.AddState( AccessibleStateType::SELECTED );
        if ( HasChildPathFocus() )
            _rStateSet.AddState( AccessibleStateType::FOCUSED );
        else // only transient when column is not focused
            _rStateSet.AddState( AccessibleStateType::TRANSIENT );

        _rStateSet.AddState( AccessibleStateType::VISIBLE );
        _rStateSet.AddState( AccessibleStateType::SHOWING );
        _rStateSet.AddState( AccessibleStateType::ENABLED );
        _rStateSet.AddState( AccessibleStateType::SENSITIVE );
        _rStateSet.AddState( AccessibleStateType::ACTIVE );

        (void)_nColumnPos;
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
    Rectangle TableControl::calcHeaderCellRect( sal_Bool _bIsColumnBar, sal_Int32 nPos )
    {
        return m_pImpl->calcHeaderCellRect( _bIsColumnBar, nPos );
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle TableControl::calcTableRect(sal_Bool _bOnScreen)
    {
        (void)_bOnScreen;
        return m_pImpl->calcTableRect();
    }

    //------------------------------------------------------------------------------------------------------------------
    Rectangle TableControl::calcCellRect( sal_Int32 _nRowPos, sal_Int32 _nColPos )
    {
        return m_pImpl->calcCellRect( _nRowPos, _nColPos );
    }

    //------------------------------------------------------------------------------------------------------------------
    IMPL_LINK_NOARG(TableControl, ImplSelectHdl)
    {
        Select();
        return 1;
    }

    //------------------------------------------------------------------------------------------------------------------
    void TableControl::Select()
    {
        ImplCallEventListenersAndHandler( VCLEVENT_TABLEROW_SELECT, m_pImpl->getSelectHandler(), this );

        if ( m_pImpl->isAccessibleAlive() )
        {
            m_pImpl->commitAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );

            m_pImpl->commitTableEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, Any(), Any() );
                // TODO: why do we notify this when the *selection* changed? Shouldn't we find a better place for this,
                // actually, when the active descendant, i.e. the current cell, *really* changed?
        }
    }

}} // namespace svt::table

//......................................................................................................................
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
