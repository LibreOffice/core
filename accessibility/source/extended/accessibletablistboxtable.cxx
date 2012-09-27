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


#include "accessibility/extended/accessibletablistboxtable.hxx"
#include "accessibility/extended/AccessibleBrowseBoxTableCell.hxx"
#include "accessibility/extended/AccessibleBrowseBoxCheckBoxCell.hxx"
#include <svtools/svtabbx.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>

//........................................................................
namespace accessibility
{
//........................................................................

    // class AccessibleTabListBoxTable ---------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    // -----------------------------------------------------------------------------
    // Ctor() and Dtor()
    // -----------------------------------------------------------------------------
    AccessibleTabListBoxTable::AccessibleTabListBoxTable( const Reference< XAccessible >& rxParent, SvHeaderTabListBox& rBox ) :

        AccessibleBrowseBoxTable( rxParent, rBox ),

        m_pTabListBox   ( &rBox )

    {
        m_pTabListBox->AddEventListener( LINK( this, AccessibleTabListBoxTable, WindowEventListener ) );
    }
    // -----------------------------------------------------------------------------
    AccessibleTabListBoxTable::~AccessibleTabListBoxTable()
    {
        if ( isAlive() )
        {
            m_pTabListBox = NULL;

            // increment ref count to prevent double call of Dtor
            osl_atomic_increment( &m_refCount );
            dispose();
        }
    }
    // -----------------------------------------------------------------------------
    void AccessibleTabListBoxTable::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
    {
        if ( isAlive() )
        {
            sal_uLong nEventId = rVclWindowEvent.GetId();
            switch ( nEventId )
            {
                case  VCLEVENT_OBJECT_DYING :
                {
                    m_pTabListBox->RemoveEventListener( LINK( this, AccessibleTabListBoxTable, WindowEventListener ) );
                    m_pTabListBox = NULL;
                    break;
                }

                case VCLEVENT_CONTROL_GETFOCUS :
                case VCLEVENT_CONTROL_LOSEFOCUS :
                {
                    uno::Any aOldValue, aNewValue;
                    if ( VCLEVENT_CONTROL_GETFOCUS == nEventId )
                        aNewValue <<= AccessibleStateType::FOCUSED;
                    else
                        aOldValue <<= AccessibleStateType::FOCUSED;
                    commitEvent( AccessibleEventId::STATE_CHANGED, aNewValue, aOldValue );
                    break;
                }

                case VCLEVENT_LISTBOX_SELECT :
                {
                    // First send an event that tells the listeners of a
                    // modified selection.  The active descendant event is
                    // send after that so that the receiving AT has time to
                    // read the text or name of the active child.
                    commitEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
                    if ( m_pTabListBox && m_pTabListBox->HasFocus() )
                    {
                        SvLBoxEntry* pEntry = static_cast< SvLBoxEntry* >( rVclWindowEvent.GetData() );
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
                                TriState eState = STATE_DONTKNOW;
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

                case VCLEVENT_CHECKBOX_TOGGLE :
                {
                    if ( m_pTabListBox && m_pTabListBox->HasFocus() )
                    {
                        SvLBoxEntry* pEntry = static_cast< SvLBoxEntry* >( rVclWindowEvent.GetData() );
                        if ( pEntry )
                        {
                            sal_Int32 nRow = m_pTabListBox->GetEntryPos( pEntry );
                            sal_uInt16 nCol = m_pTabListBox->GetCurrColumn();
                            TriState eState = STATE_DONTKNOW;
                            if ( m_pTabListBox->IsCellCheckBox( nRow, nCol, eState ) )
                            {
                                Reference< XAccessible > xChild =
                                    m_pTabListBox->CreateAccessibleCell( nRow, nCol );
                                AccessibleCheckBoxCell* pCell =
                                    static_cast< AccessibleCheckBoxCell* >( xChild.get() );
                                pCell->SetChecked( m_pTabListBox->IsItemChecked( pEntry, nCol ) );
                            }
                        }
                    }
                    break;
                }

                case VCLEVENT_TABLECELL_NAMECHANGED :
                {
                    if ( m_pTabListBox->IsTransientChildrenDisabled() )
                    {
                        commitEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
                        TabListBoxEventData* pData = static_cast< TabListBoxEventData* >( rVclWindowEvent.GetData() );
                        SvLBoxEntry* pEntry = pData != NULL ? pData->m_pEntry : NULL;
                        if ( pEntry )
                        {
                            sal_Int32 nRow = m_pTabListBox->GetEntryPos( pEntry );
                            sal_uInt16 nCol = pData->m_nColumn;
                            Reference< XAccessible > xChild =
                                m_pTabListBox->CreateAccessibleCell( nRow, nCol );
                            uno::Any aOldValue, aNewValue;
                            aOldValue <<= OUString( pData->m_sOldText );
                            OUString sNewText( m_pTabListBox->GetCellText( nRow, nCol ) );
                            aNewValue <<= sNewText;
                            TriState eState = STATE_DONTKNOW;

                            if ( m_pTabListBox->IsCellCheckBox( nRow, nCol, eState ) )
                            {
                                AccessibleCheckBoxCell* pCell =
                                    static_cast< AccessibleCheckBoxCell* >( xChild.get() );
                                pCell->commitEvent( AccessibleEventId::NAME_CHANGED, aOldValue, aNewValue );
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
            }
        }
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK( AccessibleTabListBoxTable, WindowEventListener, VclSimpleEvent*, pEvent )
    {
        OSL_ENSURE( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );
        if ( pEvent && pEvent->ISA( VclWindowEvent ) )
        {
            OSL_ENSURE( ( (VclWindowEvent*)pEvent )->GetWindow() && m_pTabListBox, "no event window" );
            ProcessWindowEvent( *(VclWindowEvent*)pEvent );
        }
        return 0;
    }
    // helpers --------------------------------------------------------------------

    void AccessibleTabListBoxTable::ensureValidIndex( sal_Int32 _nIndex ) const
            SAL_THROW( ( IndexOutOfBoundsException ) )
    {
        if ( ( _nIndex < 0 ) || ( _nIndex >= implGetCellCount() ) )
            throw IndexOutOfBoundsException();
    }

    sal_Bool AccessibleTabListBoxTable::implIsRowSelected( sal_Int32 _nRow ) const
    {
        return m_pTabListBox ? m_pTabListBox->IsSelected( m_pTabListBox->GetEntry( _nRow ) ) : sal_False;
    }

    void AccessibleTabListBoxTable::implSelectRow( sal_Int32 _nRow, sal_Bool _bSelect )
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
            SvLBoxEntry* pEntry = m_pTabListBox->FirstSelected();
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
    // -----------------------------------------------------------------------------
    // XInterface & XTypeProvider
    // -----------------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2(AccessibleTabListBoxTable, AccessibleBrowseBoxTable, AccessibleTabListBoxTableImplHelper)
    IMPLEMENT_FORWARD_XTYPEPROVIDER2(AccessibleTabListBoxTable, AccessibleBrowseBoxTable, AccessibleTabListBoxTableImplHelper)
    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------
    OUString AccessibleTabListBoxTable::getImplementationName (void) throw (RuntimeException)
    {
        return OUString( "com.sun.star.comp.svtools.AccessibleTabListBoxTable" );
    }
    // -----------------------------------------------------------------------------
    // XAccessibleSelection
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleTabListBoxTable::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();
        ensureValidIndex( nChildIndex );

        implSelectRow( implGetRow( nChildIndex ), sal_True );
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleTabListBoxTable::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();
        ensureValidIndex( nChildIndex );

        return implIsRowSelected( implGetRow( nChildIndex ) );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleTabListBoxTable::clearAccessibleSelection(  ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();

        m_pTabListBox->SetNoSelection();
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleTabListBoxTable::selectAllAccessibleChildren(  ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();

        m_pTabListBox->SelectAll();
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleTabListBoxTable::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();

        return implGetColumnCount() * implGetSelRowCount();
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleTabListBoxTable::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();

        sal_Int32 nRows = implGetSelRowCount();
        if ( nRows == 0 )
            throw IndexOutOfBoundsException();

        sal_Int32 nRow = implGetSelRow( nSelectedChildIndex % nRows );
        sal_Int32 nColumn = nSelectedChildIndex / nRows;
        return getAccessibleCellAt( nRow, nColumn );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleTabListBoxTable::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();
        ensureValidIndex( nSelectedChildIndex );

        implSelectRow( implGetRow( nSelectedChildIndex ), sal_False );
    }

//........................................................................
}// namespace accessibility
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
