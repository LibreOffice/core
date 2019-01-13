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


#include <extended/accessibletablistboxtable.hxx>
#include <extended/AccessibleBrowseBoxTableCell.hxx>
#include <extended/AccessibleBrowseBoxCheckBoxCell.hxx>
#include <vcl/svtabbx.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>


namespace accessibility
{


    // class AccessibleTabListBoxTable ---------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;


    // Ctor() and Dtor()

    AccessibleTabListBoxTable::AccessibleTabListBoxTable( const Reference< XAccessible >& rxParent, SvHeaderTabListBox& rBox ) :

        AccessibleBrowseBoxTable( rxParent, rBox ),

        m_pTabListBox   ( &rBox )

    {
        m_pTabListBox->AddEventListener( LINK( this, AccessibleTabListBoxTable, WindowEventListener ) );
    }

    AccessibleTabListBoxTable::~AccessibleTabListBoxTable()
    {
        if ( isAlive() )
        {
            m_pTabListBox = nullptr;

            // increment ref count to prevent double call of Dtor
            osl_atomic_increment( &m_refCount );
            dispose();
        }
    }

    void AccessibleTabListBoxTable::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
    {
        if ( isAlive() )
        {
            switch ( VclEventId nEventId = rVclWindowEvent.GetId(); nEventId )
            {
                case  VclEventId::ObjectDying :
                {
                    m_pTabListBox->RemoveEventListener( LINK( this, AccessibleTabListBoxTable, WindowEventListener ) );
                    m_pTabListBox = nullptr;
                    break;
                }

                case VclEventId::ControlGetFocus :
                case VclEventId::ControlLoseFocus :
                {
                    uno::Any aOldValue, aNewValue;
                    if ( nEventId == VclEventId::ControlGetFocus )
                        aNewValue <<= AccessibleStateType::FOCUSED;
                    else
                        aOldValue <<= AccessibleStateType::FOCUSED;
                    commitEvent( AccessibleEventId::STATE_CHANGED, aNewValue, aOldValue );
                    break;
                }

                case VclEventId::ListboxSelect :
                {
                    // First send an event that tells the listeners of a
                    // modified selection.  The active descendant event is
                    // send after that so that the receiving AT has time to
                    // read the text or name of the active child.
                    commitEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
                    if ( m_pTabListBox && m_pTabListBox->HasFocus() )
                    {
                        SvTreeListEntry* pEntry = static_cast< SvTreeListEntry* >( rVclWindowEvent.GetData() );
                        if ( pEntry )
                        {
                            sal_Int32 nRow = m_pTabListBox->GetEntryPos( pEntry );
                            sal_uInt16 nCol = m_pTabListBox->GetCurrColumn();
                            Reference< XAccessible > xChild =
                                m_pTabListBox->CreateAccessibleCell( nRow, nCol );
                            uno::Any aOldValue, aNewValue;

                            if ( m_pTabListBox->IsTransientChildrenDisabled() )
                            {
                                aNewValue <<= AccessibleStateType::FOCUSED;
                                TriState eState = TRISTATE_INDET;
                                if ( m_pTabListBox->IsCellCheckBox( nRow, nCol, eState ) )
                                {
                                    AccessibleCheckBoxCell* pCell =
                                        static_cast< AccessibleCheckBoxCell* >( xChild.get() );
                                    pCell->commitEvent( AccessibleEventId::STATE_CHANGED, aNewValue, aOldValue );
                                }
                                else
                                {
                                    AccessibleBrowseBoxTableCell* pCell =
                                        static_cast< AccessibleBrowseBoxTableCell* >( xChild.get() );
                                    pCell->commitEvent( AccessibleEventId::STATE_CHANGED, aNewValue, aOldValue );
                                }
                            }
                            else
                            {
                                aNewValue <<= xChild;
                                commitEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aNewValue, aOldValue );
                            }
                        }
                    }
                    break;
                }
                case VclEventId::WindowGetFocus :
                {
                    uno::Any aOldValue, aNewValue;
                    aNewValue <<= AccessibleStateType::FOCUSED;
                    commitEvent( AccessibleEventId::STATE_CHANGED, aNewValue, aOldValue );
                    break;

                }
                case VclEventId::WindowLoseFocus :
                {
                    uno::Any aOldValue, aNewValue;
                    aOldValue <<= AccessibleStateType::FOCUSED;
                    commitEvent( AccessibleEventId::STATE_CHANGED, aNewValue, aOldValue );
                    break;
                }
                case VclEventId::ListboxTreeSelect:
                    {
                        SvTreeListEntry* pEntry = static_cast< SvTreeListEntry* >( rVclWindowEvent.GetData() );
                        if (pEntry)
                        {
                            sal_Int32 nRow = m_pTabListBox->GetEntryPos( pEntry );
                            Reference< XAccessible > xChild = m_pTabListBox->CreateAccessibleCell( nRow, m_pTabListBox->GetCurrColumn() );
                            TriState eState = TRISTATE_INDET;
                            if ( m_pTabListBox->IsCellCheckBox( nRow, m_pTabListBox->GetCurrColumn(), eState ) )
                            {
                                AccessibleCheckBoxCell* pCell = static_cast< AccessibleCheckBoxCell* >( xChild.get() );
                                pCell->commitEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
                            }
                            else
                            {
                                AccessibleBrowseBoxTableCell* pCell = static_cast< AccessibleBrowseBoxTableCell* >( xChild.get() );
                                pCell->commitEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
                            }
                        }
                    }
                    break;
                case VclEventId::ListboxTreeFocus:
                    {
                        if ( m_pTabListBox && m_pTabListBox->HasFocus() )
                        {
                            uno::Any aOldValue, aNewValue;
                            SvTreeListEntry* pEntry = static_cast< SvTreeListEntry* >( rVclWindowEvent.GetData() );
                            if ( pEntry )
                            {
                                sal_Int32 nRow = m_pTabListBox->GetEntryPos( pEntry );
                                m_xCurChild = m_pTabListBox->CreateAccessibleCell( nRow, m_pTabListBox->GetCurrColumn() );
                                aNewValue <<= m_xCurChild;
                                commitEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aNewValue ,aOldValue);
                            }
                            else
                            {
                                aNewValue <<= AccessibleStateType::FOCUSED;
                                commitEvent( AccessibleEventId::STATE_CHANGED, aNewValue ,aOldValue);
                            }
                        }
                    }
                    break;

                case VclEventId::CheckboxToggle :
                {
                    if ( m_pTabListBox && m_pTabListBox->HasFocus() )
                    {
                        SvTreeListEntry* pEntry = static_cast< SvTreeListEntry* >( rVclWindowEvent.GetData() );
                        if ( pEntry )
                        {
                            sal_Int32 nRow = m_pTabListBox->GetEntryPos( pEntry );
                            sal_uInt16 nCol = m_pTabListBox->GetCurrColumn();
                            TriState eState = TRISTATE_INDET;
                            if ( m_pTabListBox->IsCellCheckBox( nRow, nCol, eState ) )
                            {
                                Reference< XAccessible > xChild =
                                    m_pTabListBox->CreateAccessibleCell( nRow, nCol );
                                AccessibleCheckBoxCell* pCell =
                                    static_cast< AccessibleCheckBoxCell* >( xChild.get() );
                                pCell->SetChecked( SvHeaderTabListBox::IsItemChecked( pEntry, nCol ) );
                            }
                        }
                    }
                    break;
                }

                case VclEventId::TableCellNameChanged :
                {
                    if ( m_pTabListBox->IsTransientChildrenDisabled() )
                    {
                        commitEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
                        TabListBoxEventData* pData = static_cast< TabListBoxEventData* >( rVclWindowEvent.GetData() );
                        SvTreeListEntry* pEntry = pData != nullptr ? pData->m_pEntry : nullptr;
                        if ( pEntry )
                        {
                            sal_Int32 nRow = m_pTabListBox->GetEntryPos( pEntry );
                            sal_uInt16 nCol = pData->m_nColumn;
                            Reference< XAccessible > xChild =
                                m_pTabListBox->CreateAccessibleCell( nRow, nCol );
                            uno::Any aOldValue, aNewValue;
                            aOldValue <<= pData->m_sOldText;
                            OUString sNewText( m_pTabListBox->GetCellText( nRow, nCol ) );
                            aNewValue <<= sNewText;
                            TriState eState = TRISTATE_INDET;

                            if ( m_pTabListBox->IsCellCheckBox( nRow, nCol, eState ) )
                            {
                                AccessibleCheckBoxCell* pCell =
                                    static_cast< AccessibleCheckBoxCell* >( xChild.get() );
                                pCell->commitEvent( AccessibleEventId::NAME_CHANGED, aNewValue, aOldValue );
                            }
                            else
                            {
                                AccessibleBrowseBoxTableCell* pCell =
                                    static_cast< AccessibleBrowseBoxTableCell* >( xChild.get() );
                                pCell->nameChanged( sNewText, pData->m_sOldText );
                            }
                        }
                    }
                    break;
                }
                default: break;
            }
        }
    }

    IMPL_LINK( AccessibleTabListBoxTable, WindowEventListener, VclWindowEvent&, rEvent, void )
    {
        OSL_ENSURE( rEvent.GetWindow() && m_pTabListBox, "no event window" );
        ProcessWindowEvent( rEvent );
    }
    // helpers --------------------------------------------------------------------

    void AccessibleTabListBoxTable::ensureValidIndex( sal_Int32 _nIndex ) const
    {
        if ( ( _nIndex < 0 ) || ( _nIndex >= (implGetRowCount() * implGetColumnCount()) ) )
            throw IndexOutOfBoundsException();
    }

    void AccessibleTabListBoxTable::implSelectRow( sal_Int32 _nRow, bool _bSelect )
    {
        if ( m_pTabListBox )
            m_pTabListBox->Select( m_pTabListBox->GetEntry( _nRow ), _bSelect );
    }

    sal_Int32 AccessibleTabListBoxTable::implGetRowCount() const
    {
        return m_pTabListBox ? m_pTabListBox->GetEntryCount() : 0;
    }

    sal_Int32 AccessibleTabListBoxTable::implGetColumnCount() const
    {
        return m_pTabListBox ? m_pTabListBox->GetColumnCount() : 0;
    }

    sal_Int32 AccessibleTabListBoxTable::implGetSelRowCount() const
    {
        return m_pTabListBox ? m_pTabListBox->GetSelectionCount() : 0;
    }

    sal_Int32 AccessibleTabListBoxTable::implGetSelRow( sal_Int32 nSelRow ) const
    {
        if ( m_pTabListBox )
        {
            sal_Int32 nRow = 0;
            SvTreeListEntry* pEntry = m_pTabListBox->FirstSelected();
            while ( pEntry )
            {
                ++nRow;
                if ( nRow == nSelRow )
                    return m_pTabListBox->GetEntryPos( pEntry );
                pEntry = m_pTabListBox->NextSelected( pEntry );
            }
        }

        return 0;
    }

    // XInterface & XTypeProvider

    IMPLEMENT_FORWARD_XINTERFACE2(AccessibleTabListBoxTable, AccessibleBrowseBoxTable, AccessibleTabListBoxTableImplHelper)
    IMPLEMENT_FORWARD_XTYPEPROVIDER2(AccessibleTabListBoxTable, AccessibleBrowseBoxTable, AccessibleTabListBoxTableImplHelper)

    // XServiceInfo

    OUString AccessibleTabListBoxTable::getImplementationName()
    {
        return OUString( "com.sun.star.comp.svtools.AccessibleTabListBoxTable" );
    }

    // XAccessibleSelection

    void SAL_CALL AccessibleTabListBoxTable::selectAccessibleChild( sal_Int32 nChildIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getMutex() );

        ensureIsAlive();
        ensureValidIndex( nChildIndex );

        implSelectRow( implGetRow( nChildIndex ), true );
    }

    sal_Bool SAL_CALL AccessibleTabListBoxTable::isAccessibleChildSelected( sal_Int32 nChildIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getMutex() );

        ensureIsAlive();
        ensureValidIndex( nChildIndex );

        return m_pTabListBox && m_pTabListBox->IsSelected( m_pTabListBox->GetEntry( implGetRow( nChildIndex ) ) );
    }

    void SAL_CALL AccessibleTabListBoxTable::clearAccessibleSelection(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getMutex() );

        ensureIsAlive();

        m_pTabListBox->SetNoSelection();
    }

    void SAL_CALL AccessibleTabListBoxTable::selectAllAccessibleChildren(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getMutex() );

        ensureIsAlive();

        m_pTabListBox->SelectAll();
    }

    sal_Int32 SAL_CALL AccessibleTabListBoxTable::getSelectedAccessibleChildCount(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getMutex() );

        ensureIsAlive();

        return implGetColumnCount() * implGetSelRowCount();
    }

    Reference< XAccessible > SAL_CALL AccessibleTabListBoxTable::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getMutex() );

        ensureIsAlive();

        sal_Int32 nRows = implGetSelRowCount();
        if ( nRows == 0 )
            throw IndexOutOfBoundsException();

        sal_Int32 nRow = implGetSelRow( nSelectedChildIndex % nRows );
        sal_Int32 nColumn = nSelectedChildIndex / nRows;
        return getAccessibleCellAt( nRow, nColumn );
    }

    void SAL_CALL AccessibleTabListBoxTable::deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getMutex() );

        ensureIsAlive();
        ensureValidIndex( nSelectedChildIndex );

        implSelectRow( implGetRow( nSelectedChildIndex ), false );
    }


}// namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
