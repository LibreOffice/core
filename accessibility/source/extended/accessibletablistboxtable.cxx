/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibletablistboxtable.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:34:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABLISTBOXTABLE_HXX_
#include "accessibility/extended/accessibletablistboxtable.hxx"
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBILEBROWSEBOXTABLECELL_HXX
#include "accessibility/extended/AccessibleBrowseBoxTableCell.hxx"
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXCHECKBOXCELL_HXX
#include "accessibility/extended/AccessibleBrowseBoxCheckBoxCell.hxx"
#endif
#ifndef _SVTABBX_HXX
#include <svtools/svtabbx.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

//........................................................................
namespace accessibility
{
//........................................................................

    // class TLBSolarGuard ---------------------------------------------------------

    /** Aquire the solar mutex. */
    class TLBSolarGuard : public ::vos::OGuard
    {
    public:
        inline TLBSolarGuard() : ::vos::OGuard( Application::GetSolarMutex() ) {}
    };

    // class AccessibleTabListBoxTable ---------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    DBG_NAME(AccessibleTabListBoxTable)

    // -----------------------------------------------------------------------------
    // Ctor() and Dtor()
    // -----------------------------------------------------------------------------
    AccessibleTabListBoxTable::AccessibleTabListBoxTable( const Reference< XAccessible >& rxParent, SvHeaderTabListBox& rBox ) :

        AccessibleBrowseBoxTable( rxParent, rBox ),

        m_pTabListBox   ( &rBox )

    {
        DBG_CTOR( AccessibleTabListBoxTable, NULL );

        m_pTabListBox->AddEventListener( LINK( this, AccessibleTabListBoxTable, WindowEventListener ) );
    }
    // -----------------------------------------------------------------------------
    AccessibleTabListBoxTable::~AccessibleTabListBoxTable()
    {
        DBG_DTOR( AccessibleTabListBoxTable, NULL );

        if ( isAlive() )
        {
            m_pTabListBox = NULL;

            // increment ref count to prevent double call of Dtor
            osl_incrementInterlockedCount( &m_refCount );
            dispose();
        }
    }
    // -----------------------------------------------------------------------------
    void AccessibleTabListBoxTable::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
    {
        if ( isAlive() )
        {
            ULONG nEventId = rVclWindowEvent.GetId();
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
                            USHORT nCol = m_pTabListBox->GetCurrColumn();
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
                            USHORT nCol = m_pTabListBox->GetCurrColumn();
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
            }
        }
    }
    // -----------------------------------------------------------------------------
    IMPL_LINK( AccessibleTabListBoxTable, WindowEventListener, VclSimpleEvent*, pEvent )
    {
        DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );
        if ( pEvent && pEvent->ISA( VclWindowEvent ) )
        {
            DBG_ASSERT( ( (VclWindowEvent*)pEvent )->GetWindow() && m_pTabListBox, "no event window" );
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

    void AccessibleTabListBoxTable::ensureValidPosition( sal_Int32 _nRow, sal_Int32 _nColumn ) const
            SAL_THROW( ( IndexOutOfBoundsException ) )
    {
        if ( ( _nRow < 0 ) || ( _nRow >= implGetRowCount() ) || ( _nColumn < 0 ) || ( _nColumn >= implGetColumnCount() ) )
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
    ::rtl::OUString AccessibleTabListBoxTable::getImplementationName (void) throw (RuntimeException)
    {
        return ::rtl::OUString::createFromAscii("com.sun.star.comp.svtools.AccessibleTabListBoxTable");
    }
    // -----------------------------------------------------------------------------
    // XAccessibleSelection
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleTabListBoxTable::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        TLBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();
        ensureValidIndex( nChildIndex );

        implSelectRow( implGetRow( nChildIndex ), sal_True );
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleTabListBoxTable::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        TLBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();
        ensureValidIndex( nChildIndex );

        return implIsRowSelected( implGetRow( nChildIndex ) );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleTabListBoxTable::clearAccessibleSelection(  ) throw (RuntimeException)
    {
        TLBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();

        m_pTabListBox->SetNoSelection();
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleTabListBoxTable::selectAllAccessibleChildren(  ) throw (RuntimeException)
    {
        TLBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();

        m_pTabListBox->SelectAll();
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleTabListBoxTable::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
    {
        TLBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();

        return implGetColumnCount() * implGetSelRowCount();
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleTabListBoxTable::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        TLBSolarGuard aSolarGuard;
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
        TLBSolarGuard aSolarGuard;
        ::osl::MutexGuard aGuard( getOslMutex() );

        ensureIsAlive();
        ensureValidIndex( nSelectedChildIndex );

        implSelectRow( implGetRow( nSelectedChildIndex ), sal_False );
    }

//........................................................................
}// namespace accessibility
//........................................................................

