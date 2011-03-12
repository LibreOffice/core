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
#include "tablegeometry.hxx"
#include "tablecontrol_impl.hxx"
#include "accessibletableimp.hxx"
#include "svtools/table/tabledatawindow.hxx"
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace ::com::sun::star::uno;
using ::com::sun::star::accessibility::XAccessible;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::lang;
using namespace utl;
//........................................................................
namespace svt { namespace table
{
    //====================================================================
    //= AccessibleTableControl_Impl
    //====================================================================
    // ----------------------------------------------------------------------------
    Reference< XAccessible > AccessibleTableControl_Impl::getAccessibleTableHeader( AccessibleTableControlObjType _eObjType )
    {
        if ( m_pAccessible && m_pAccessible->isAlive() )
            return m_pAccessible->getTableHeader( _eObjType );
        return NULL;
    }
    // ----------------------------------------------------------------------------
    Reference< XAccessible > AccessibleTableControl_Impl::getAccessibleTable( )
    {
        if ( m_pAccessible && m_pAccessible->isAlive() )
            return m_pAccessible->getTable( );
        return NULL;
    }

    //====================================================================
    //= TableControl
    //====================================================================
    //--------------------------------------------------------------------
    TableControl::TableControl( Window* _pParent, WinBits _nStyle )
        :Control( _pParent, _nStyle )
        ,m_pImpl( new TableControl_Impl( *this ) )
        ,m_bSelectionChanged(false)
        ,m_bTooltip(false)
    {
        TableDataWindow* aTableData = m_pImpl->getDataWindow();
        aTableData->SetMouseButtonDownHdl( LINK( this, TableControl, ImplMouseButtonDownHdl ) );
        aTableData->SetMouseButtonUpHdl( LINK( this, TableControl, ImplMouseButtonUpHdl ) );
        aTableData->SetSelectHdl( LINK( this, TableControl, ImplSelectHdl ) );
        m_pAccessTable.reset(new ::svt::table::AccessibleTableControl_Impl());

        // by default, use the background as determined by the style settings
        const Color aWindowColor( GetSettings().GetStyleSettings().GetFieldColor() );
        SetBackground( Wallpaper( aWindowColor ) );
        SetFillColor( aWindowColor );
    }

    //--------------------------------------------------------------------
    TableControl::~TableControl()
    {
        ImplCallEventListeners( VCLEVENT_OBJECT_DYING );
        DELETEZ( m_pImpl );
        if ( m_pAccessTable->m_pAccessible )
        {
            m_pAccessTable->m_pAccessible->dispose();
        }
    }

    //--------------------------------------------------------------------
    void TableControl::GetFocus()
    {
        if ( !m_pImpl->getInputHandler()->GetFocus( *m_pImpl ) )
        {
            Control::GetFocus();
            Control::GrabFocus();
        }
    }

    //--------------------------------------------------------------------
    void TableControl::LoseFocus()
    {
        if ( !m_pImpl->getInputHandler()->LoseFocus( *m_pImpl ) )
            Control::LoseFocus();
    }

    //--------------------------------------------------------------------
    void TableControl::KeyInput( const KeyEvent& rKEvt )
    {
        if ( !m_pImpl->getInputHandler()->KeyInput( *m_pImpl, rKEvt ) )
            Control::KeyInput( rKEvt );
        else
        {
            if(m_bSelectionChanged)
            {
                Select();
                m_bSelectionChanged = false;
            }
        }
    }


    //--------------------------------------------------------------------
    void TableControl::StateChanged( StateChangedType i_nStateChange )
    {
        Control::StateChanged( i_nStateChange );

        // forward certain settings to the data window
        switch ( i_nStateChange )
        {
        case STATE_CHANGE_CONTROLBACKGROUND:
            if ( IsControlBackground() )
                getDataWindow()->SetControlBackground( GetControlBackground() );
            else
                getDataWindow()->SetControlBackground();
            break;

        case STATE_CHANGE_CONTROLFOREGROUND:
            if ( IsControlForeground() )
                getDataWindow()->SetControlForeground( GetControlForeground() );
            else
                getDataWindow()->SetControlForeground();
            break;

        case STATE_CHANGE_CONTROLFONT:
            if ( IsControlFont() )
                getDataWindow()->SetControlFont( GetControlFont() );
            else
                getDataWindow()->SetControlFont();
            break;
        }
    }

    //--------------------------------------------------------------------
    void TableControl::Resize()
    {
        Control::Resize();
        m_pImpl->onResize();
    }

    //--------------------------------------------------------------------
    void TableControl::SetModel( PTableModel _pModel )
    {
        m_pImpl->setModel( _pModel );
    }

    //--------------------------------------------------------------------
    PTableModel TableControl::GetModel() const
    {
        return m_pImpl->getModel();
    }

    //--------------------------------------------------------------------
    RowPos TableControl::GetTopRow() const
    {
        return m_pImpl->getTopRow();
    }

    //--------------------------------------------------------------------
    void TableControl::SetTopRow( RowPos _nRow )
    {
        // TODO
        (void)_nRow;
    }

    //--------------------------------------------------------------------
    sal_Int32 TableControl::GetCurrentRow() const
    {
        return m_pImpl->getCurRow();
    }

    //--------------------------------------------------------------------
    sal_Int32 TableControl::GetCurrentColumn() const
    {
        return m_pImpl->getCurColumn();
    }

    //--------------------------------------------------------------------
    bool TableControl::GoTo( ColPos _nColumn, RowPos _nRow )
    {
        return m_pImpl->goTo( _nColumn, _nRow );
    }
    //--------------------------------------------------------------------
    sal_Bool TableControl::GoToCell(sal_Int32 _nColPos, sal_Int32 _nRowPos)
    {
        return m_pImpl->goTo( _nColPos, _nRowPos );
    }
    //--------------------------------------------------------------------
    void TableControl::clearSelection()
    {
        m_pImpl->clearSelection();
    }
    //--------------------------------------------------------------------
    void TableControl::InvalidateDataWindow(RowPos _nRowStart, RowPos _nRowEnd, bool _bRemoved)
    {
        Rectangle _rRect;
        if(_bRemoved)
            m_pImpl->invalidateRows();
        else
        {
            if(m_bSelectionChanged)
            {
                m_pImpl->invalidateSelectedRegion(_nRowStart, _nRowEnd, _rRect);
                m_bSelectionChanged = false;
            }
            else
                m_pImpl->invalidateRow(_nRowStart, _rRect);
        }
    }
    //--------------------------------------------------------------------
    std::vector<sal_Int32>& TableControl::GetSelectedRows()
    {
        return m_pImpl->getSelectedRows();
    }
    //--------------------------------------------------------------------
    void TableControl::RemoveSelectedRow(RowPos _nRowPos)
    {
        m_pImpl->removeSelectedRow(_nRowPos);
    }
    //--------------------------------------------------------------------

    RowPos TableControl::GetCurrentRow(const Point& rPoint)
    {
        return m_pImpl->getCurrentRow( rPoint );
    }

    SelectionEngine* TableControl::getSelEngine()
    {
        return m_pImpl->getSelEngine();
    }

    TableDataWindow* TableControl::getDataWindow()
    {
        return m_pImpl->getDataWindow();
    }

    Reference< XAccessible > TableControl::CreateAccessible()
    {
        Window* pParent = GetAccessibleParentWindow();
        DBG_ASSERT( pParent, "TableControl::CreateAccessible - parent not found" );

        if( pParent && !m_pAccessTable->m_pAccessible)
        {
            Reference< XAccessible > xAccParent = pParent->GetAccessible();
            if( xAccParent.is() )
            {
                m_pAccessTable->m_pAccessible = getAccessibleFactory().createAccessibleTableControl(
                    xAccParent, *this
                );
            }
        }
        Reference< XAccessible > xAccessible;
        if ( m_pAccessTable->m_pAccessible )
            xAccessible = m_pAccessTable->m_pAccessible->getMyself();
        return xAccessible;
    }
    Reference<XAccessible> TableControl::CreateAccessibleControl( sal_Int32 _nIndex )
    {
        (void)_nIndex;
        DBG_ASSERT( FALSE, "TableControl::CreateAccessibleControl: to be overwritten!" );
        return NULL;
    }
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
// -----------------------------------------------------------------------------

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
// -----------------------------------------------------------------------------

::rtl::OUString TableControl::GetRowDescription( sal_Int32 _nRow) const
{
    (void)_nRow;
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "row description" ) );
}
// -----------------------------------------------------------------------------

::rtl::OUString TableControl::GetRowName( sal_Int32 _nIndex) const
{
    return GetModel()->getRowHeaderName()[_nIndex];
}
// -----------------------------------------------------------------------------

::rtl::OUString TableControl::GetColumnDescription( sal_uInt16 _nColumn) const
{
    (void)_nColumn;
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "col description" ) );
}
// -----------------------------------------------------------------------------

::rtl::OUString TableControl::GetColumnName( sal_Int32 _nIndex) const
{
    return GetModel()->getColumnModel(_nIndex)->getName();
}

// -----------------------------------------------------------------------------

::com::sun::star::uno::Any TableControl::GetCellContent( sal_Int32 _nRowPos, sal_Int32 _nColPos) const
{
    ::com::sun::star::uno::Any cellContent(::com::sun::star::uno::Any(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("")) ));
    std::vector<std::vector< ::com::sun::star::uno::Any > >& aTableContent = GetModel()->getCellContent();
    if(&aTableContent)
        cellContent = aTableContent[_nRowPos][_nColPos];
    return cellContent;
}
// -----------------------------------------------------------------------------

::rtl::OUString TableControl::GetAccessibleCellText( sal_Int32 _nRowPos, sal_Int32 _nColPos) const
{
    const ::com::sun::star::uno::Any cellContent = GetCellContent(_nRowPos, _nColPos);
    return m_pImpl->convertToString(cellContent);
}
// -----------------------------------------------------------------------------

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

Rectangle TableControl::GetWindowExtentsRelative( Window *pRelativeWindow ) const
{
    return Control::GetWindowExtentsRelative( pRelativeWindow );
}
//-----------------------------------------------------------------------------
void TableControl::GrabFocus()
{
    Control::GrabFocus();
}
// -----------------------------------------------------------------------------
Reference< XAccessible > TableControl::GetAccessible( BOOL bCreate )
{
    return Control::GetAccessible( bCreate );
}
// -----------------------------------------------------------------------------
Window* TableControl::GetAccessibleParentWindow() const
{
    return Control::GetAccessibleParentWindow();
}
// -----------------------------------------------------------------------------
Window* TableControl::GetWindowInstance()
{
    return this;
}

sal_Bool TableControl::HasRowHeader()
{
    return GetModel()->hasRowHeaders();
}
//--------------------------------------------------------------------------------
sal_Bool TableControl::HasColHeader()
{
    return GetModel()->hasColumnHeaders();
}
//--------------------------------------------------------------------------------
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
sal_Bool TableControl::ConvertPointToControlIndex( sal_Int32& _rnIndex, const Point& _rPoint )
{
    sal_Int32 nRow = m_pImpl->getCurrentRow(_rPoint);
    sal_Int32 nCol = GetCurrentColumn();
    _rnIndex = nRow * GetColumnCount() + nCol;
    return nRow>=0 ? sal_True : sal_False;
}

long TableControl::GetRowCount() const
{
    return m_pImpl->getRowCount();
}
long TableControl::GetColumnCount() const
{
    return m_pImpl->getColumnCount();
}
sal_Bool TableControl::HasRowHeader() const
{
    PTableModel pModel = GetModel();
    return pModel->hasRowHeaders();
}
sal_Int32 TableControl::GetSelectedRowCount() const
{
    return m_pImpl->getSelectedRows().size();
}
bool TableControl::IsRowSelected( long _nRow ) const
{
    return m_pImpl->isRowSelected(m_pImpl->getSelectedRows(), _nRow);
}
sal_Bool TableControl::ConvertPointToCellAddress( sal_Int32& _rnRow, sal_Int32& _rnColPos, const Point& _rPoint )
{
    _rnRow = m_pImpl->getCurrentRow(_rPoint);
    _rnColPos = GetCurrentColumn();
    return _rnRow>=0 ? sal_True : sal_False;
}
void TableControl::FillAccessibleStateSetForCell( ::utl::AccessibleStateSetHelper& _rStateSet, sal_Int32 _nRow, sal_uInt16 _nColumnPos ) const
{
    if ( GetCurrentRow() == _nRow && GetCurrentColumn() == _nColumnPos )
        _rStateSet.AddState( AccessibleStateType::FOCUSED );
    else // only transient when column is not focused
        _rStateSet.AddState( AccessibleStateType::TRANSIENT );
}
Rectangle TableControl::GetFieldCharacterBounds(sal_Int32 _nRow,sal_Int32 _nColumnPos,sal_Int32 nIndex)
{
    (void)_nRow;
    (void)_nColumnPos;
    return GetCharacterBounds(nIndex);
}
sal_Int32 TableControl::GetFieldIndexAtPoint(sal_Int32 _nRow,sal_Int32 _nColumnPos,const Point& _rPoint)
{
    (void)_nRow;
    (void)_nColumnPos;
    return GetIndexForPoint(_rPoint);
;
}
 // -----------------------------------------------------------------------------
sal_Bool TableControl::isAccessibleAlive( ) const
{
    return ( NULL != m_pAccessTable->m_pAccessible ) && m_pAccessTable->m_pAccessible->isAlive();
}

// -----------------------------------------------------------------------------
::svt::IAccessibleFactory& TableControl::getAccessibleFactory()
{
    return m_pAccessTable->m_aFactoryAccess.getFactory();
}
// -----------------------------------------------------------------------------
void TableControl::commitGridControlEvent( sal_Int16 _nEventId, const Any& _rNewValue, const Any& _rOldValue )
{
     if ( isAccessibleAlive() )
         m_pAccessTable->m_pAccessible->commitEvent( _nEventId, _rNewValue, _rOldValue);
}
// -----------------------------------------------------------------------------
Rectangle TableControl::calcHeaderRect(sal_Bool _bIsColumnBar,BOOL _bOnScreen)
{
    (void)_bOnScreen;
    return m_pImpl->calcHeaderRect(_bIsColumnBar);
}
// -----------------------------------------------------------------------------
Rectangle TableControl::calcTableRect(BOOL _bOnScreen)
{
    (void)_bOnScreen;
    return m_pImpl->calcTableRect();
}
//--------------------------------------------------------------------
::com::sun::star::uno::Sequence< sal_Int32 >& TableControl::getColumnsForTooltip()
{
    return  m_nCols;
}
//--------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString >& TableControl::getTextForTooltip()
{
    return  m_aText;
}
//--------------------------------------------------------------------
void TableControl::setTooltip(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aText, const ::com::sun::star::uno::Sequence< sal_Int32 >& nCols )
{
    m_aText = aText;
    m_nCols = nCols;
    m_bTooltip = true;
}
// -----------------------------------------------------------------------
void TableControl::selectionChanged(bool _bChanged)
{
    m_bSelectionChanged = _bChanged;
}
// -----------------------------------------------------------------------
bool TableControl::isTooltip()
{
    return m_bTooltip;
}
// -----------------------------------------------------------------------
IMPL_LINK( TableControl, ImplSelectHdl, void*, EMPTYARG )
{
    Select();
    return 1;
}
IMPL_LINK( TableControl, ImplMouseButtonDownHdl, MouseEvent*, pData )
{
    CallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, pData );
     return 1;
}
IMPL_LINK( TableControl, ImplMouseButtonUpHdl, MouseEvent*, pData )
{
     CallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONUP, pData );
     return 1;
}
// -----------------------------------------------------------------------
void TableControl::Select()
{
    ImplCallEventListenersAndHandler( VCLEVENT_TABLEROW_SELECT, m_aSelectHdl, this );
}
//........................................................................
}} // namespace svt::table
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
