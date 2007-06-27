/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibleiconchoicectrl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:32:24 $
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

#ifndef ACCESSIBILITY_EXT_ACCESSIBLEICONCHOICECTRL_HXX_
#include "accessibility/extended/accessibleiconchoicectrl.hxx"
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBLEICONCHOICECTRLENTRY_HXX_
#include "accessibility/extended/accessibleiconchoicectrlentry.hxx"
#endif
#ifndef _ICNVW_HXX
#include <svtools/ivctrl.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

//........................................................................
namespace accessibility
{
//........................................................................

    // class AccessibleIconChoiceCtrl ----------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    DBG_NAME(AccessibleIconChoiceCtrl)

    // -----------------------------------------------------------------------------
    // Ctor() and Dtor()
    // -----------------------------------------------------------------------------
    AccessibleIconChoiceCtrl::AccessibleIconChoiceCtrl( SvtIconChoiceCtrl& _rIconCtrl, const Reference< XAccessible >& _xParent ) :

        VCLXAccessibleComponent( _rIconCtrl.GetWindowPeer() ),
        m_xParent       ( _xParent )
    {
        DBG_CTOR( AccessibleIconChoiceCtrl, NULL );
    }
    // -----------------------------------------------------------------------------
    AccessibleIconChoiceCtrl::~AccessibleIconChoiceCtrl()
    {
        DBG_DTOR( AccessibleIconChoiceCtrl, NULL );
    }
    // -----------------------------------------------------------------------------
    IMPLEMENT_FORWARD_XINTERFACE2(AccessibleIconChoiceCtrl, VCLXAccessibleComponent, AccessibleIconChoiceCtrl_BASE)
    IMPLEMENT_FORWARD_XTYPEPROVIDER2(AccessibleIconChoiceCtrl, VCLXAccessibleComponent, AccessibleIconChoiceCtrl_BASE)
    // -----------------------------------------------------------------------------
    void AccessibleIconChoiceCtrl::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
    {
        if ( isAlive() )
        {
            switch ( rVclWindowEvent.GetId() )
            {
                case VCLEVENT_LISTBOX_SELECT :
                {
                    // First send an event that tells the listeners of a
                    // modified selection.  The active descendant event is
                    // send after that so that the receiving AT has time to
                    // read the text or name of the active child.
                    NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
                    SvtIconChoiceCtrl* pCtrl = getCtrl();
                    if ( pCtrl && pCtrl->HasFocus() )
                    {
                        SvxIconChoiceCtrlEntry* pEntry = static_cast< SvxIconChoiceCtrlEntry* >( rVclWindowEvent.GetData() );
                        if ( pEntry )
                        {
                            ULONG nPos = pCtrl->GetEntryListPos( pEntry );
                            Reference< XAccessible > xChild = new AccessibleIconChoiceCtrlEntry( *pCtrl, nPos, this );
                            uno::Any aOldValue, aNewValue;
                            aNewValue <<= xChild;
                            NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldValue, aNewValue );
                        }
                    }
                    break;
                }
                default:
                    VCLXAccessibleComponent::ProcessWindowChildEvent (rVclWindowEvent);
            }
        }
    }
    // -----------------------------------------------------------------------------
    // XComponent
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleIconChoiceCtrl::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        m_xParent = NULL;
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleIconChoiceCtrl::getImplementationName() throw (RuntimeException)
    {
        return getImplementationName_Static();
    }
    // -----------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL AccessibleIconChoiceCtrl::getSupportedServiceNames() throw (RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleIconChoiceCtrl::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported( getSupportedServiceNames() );
        const ::rtl::OUString* pSupported = aSupported.getConstArray();
        const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
        for ( ; pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported )
            ;

        return pSupported != pEnd;
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo - static methods
    // -----------------------------------------------------------------------------
    Sequence< ::rtl::OUString > AccessibleIconChoiceCtrl::getSupportedServiceNames_Static(void) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(3);
        aSupported[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.accessibility.AccessibleContext") );
        aSupported[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.accessibility.AccessibleComponent") );
        aSupported[2] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.AccessibleIconChoiceControl") );
        return aSupported;
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString AccessibleIconChoiceCtrl::getImplementationName_Static(void) throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.svtools.AccessibleIconChoiceControl") );
    }
    // -----------------------------------------------------------------------------
    // XAccessible
    // -----------------------------------------------------------------------------
    Reference< XAccessibleContext > SAL_CALL AccessibleIconChoiceCtrl::getAccessibleContext(  ) throw (RuntimeException)
    {
        ensureAlive();
        return this;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleContext
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleIconChoiceCtrl::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        return getCtrl()->GetEntryCount();
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrl::getAccessibleChild( sal_Int32 i ) throw (RuntimeException, IndexOutOfBoundsException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        SvtIconChoiceCtrl* pCtrl = getCtrl();
        SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry(i);
        if ( !pEntry )
            throw RuntimeException();

        return new AccessibleIconChoiceCtrlEntry( *pCtrl, i, this );
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrl::getAccessibleParent(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ensureAlive();
        return m_xParent;
    }
    // -----------------------------------------------------------------------------
    sal_Int16 SAL_CALL AccessibleIconChoiceCtrl::getAccessibleRole(  ) throw (RuntimeException)
    {
        return AccessibleRole::TREE;
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleIconChoiceCtrl::getAccessibleDescription(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        return getCtrl()->GetAccessibleDescription();
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleIconChoiceCtrl::getAccessibleName(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        ::rtl::OUString sName = getCtrl()->GetAccessibleName();
        if ( sName.getLength() == 0 )
            sName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IconChoiceControl" ) );
        return sName;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleSelection
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleIconChoiceCtrl::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvtIconChoiceCtrl* pCtrl = getCtrl();
        SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        pCtrl->SetCursor( pEntry );
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleIconChoiceCtrl::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvtIconChoiceCtrl* pCtrl = getCtrl();
        SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        return ( pCtrl->GetCursor() == pEntry );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleIconChoiceCtrl::clearAccessibleSelection(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        getCtrl()->SetNoSelection();
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleIconChoiceCtrl::selectAllAccessibleChildren(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 i, nCount = 0;
        SvtIconChoiceCtrl* pCtrl = getCtrl();
        nCount = pCtrl->GetEntryCount();
        for ( i = 0; i < nCount; ++i )
        {
            SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( i );
            if ( pCtrl->GetCursor() != pEntry )
                pCtrl->SetCursor( pEntry );
        }
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleIconChoiceCtrl::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 i, nSelCount = 0, nCount = 0;
        SvtIconChoiceCtrl* pCtrl = getCtrl();
        nCount = pCtrl->GetEntryCount();
        for ( i = 0; i < nCount; ++i )
        {
            SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( i );
            if ( pCtrl->GetCursor() == pEntry )
                ++nSelCount;
        }

        return nSelCount;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrl::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild;
        sal_Int32 i, nSelCount = 0, nCount = 0;
        SvtIconChoiceCtrl* pCtrl = getCtrl();
        nCount = pCtrl->GetEntryCount();
        for ( i = 0; i < nCount; ++i )
        {
            SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( i );
            if ( pCtrl->GetCursor() == pEntry )
                ++nSelCount;

            if ( nSelCount == ( nSelectedChildIndex + 1 ) )
            {
                xChild = new AccessibleIconChoiceCtrlEntry( *pCtrl, i, this );
                break;
            }
        }

        return xChild;
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleIconChoiceCtrl::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild;
        sal_Int32 i, nSelCount = 0, nCount = 0;
        SvtIconChoiceCtrl* pCtrl = getCtrl();
        nCount = pCtrl->GetEntryCount();
        bool bFound = false;
        for ( i = 0; i < nCount; ++i )
        {
            SvxIconChoiceCtrlEntry* pEntry = pCtrl->GetEntry( i );
            if ( pEntry->IsSelected() )
            {
                ++nSelCount;
                if ( i == nSelectedChildIndex )
                    bFound = true;
            }
        }

        // if only one entry is selected and its index is choosen to deselect -> no selection anymore
        if ( 1 == nSelCount && bFound )
            pCtrl->SetNoSelection();
    }
    // -----------------------------------------------------------------------------
    void AccessibleIconChoiceCtrl::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
    {
        VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );
        if ( isAlive() )
        {
            rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
            rStateSet.AddState( AccessibleStateType::SELECTABLE );
        }
    }
    // -----------------------------------------------------------------------------
    SvtIconChoiceCtrl* AccessibleIconChoiceCtrl::getCtrl()
    {
        return static_cast<SvtIconChoiceCtrl*>(GetWindow());
    }

//........................................................................
}// namespace accessibility
//........................................................................

