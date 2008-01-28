/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessiblelistbox.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 14:14:03 $
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

#ifndef ACCESSIBILITY_EXT_ACCESSIBLELISTBOX_HXX_
#include <accessibility/extended/accessiblelistbox.hxx>
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBLELISTBOXENTRY_HXX_
#include <accessibility/extended/accessiblelistboxentry.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_POINT_HPP_
#include <com/sun/star/awt/Point.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
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

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif

//........................................................................
namespace accessibility
{
//........................................................................

    // class AccessibleListBox -----------------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    DBG_NAME(AccessibleListBox)

    // -----------------------------------------------------------------------------
    // Ctor() and Dtor()
    // -----------------------------------------------------------------------------
    AccessibleListBox::AccessibleListBox( SvTreeListBox& _rListBox, const Reference< XAccessible >& _xParent ) :

        VCLXAccessibleComponent( _rListBox.GetWindowPeer() ),
        m_xParent( _xParent )
    {
        DBG_CTOR( AccessibleListBox, NULL );
    }
    // -----------------------------------------------------------------------------
    AccessibleListBox::~AccessibleListBox()
    {
        DBG_DTOR( AccessibleListBox, NULL );
        if ( isAlive() )
        {
            // increment ref count to prevent double call of Dtor
            osl_incrementInterlockedCount( &m_refCount );
            dispose();
        }
    }
    IMPLEMENT_FORWARD_XINTERFACE2(AccessibleListBox, VCLXAccessibleComponent, AccessibleListBox_BASE)
    IMPLEMENT_FORWARD_XTYPEPROVIDER2(AccessibleListBox, VCLXAccessibleComponent, AccessibleListBox_BASE)
    // -----------------------------------------------------------------------------
    SvTreeListBox* AccessibleListBox::getListBox() const
    {
        return  static_cast< SvTreeListBox* >( const_cast<AccessibleListBox*>(this)->GetWindow() );
    }
    // -----------------------------------------------------------------------------
    void AccessibleListBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
    {
        if ( isAlive() )
        {
            switch ( rVclWindowEvent.GetId() )
            {
                case  VCLEVENT_CHECKBOX_TOGGLE :
                {
                    if ( getListBox() && getListBox()->HasFocus() )
                    {
                        SvLBoxEntry* pEntry = static_cast< SvLBoxEntry* >( rVclWindowEvent.GetData() );
                        if ( !pEntry )
                            pEntry = getListBox()->GetCurEntry();

                        if ( pEntry )
                        {
                            Reference< XAccessible > xChild = new AccessibleListBoxEntry( *getListBox(), pEntry, this );
                            uno::Any aOldValue, aNewValue;
                            aNewValue <<= xChild;
                            NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldValue, aNewValue );
                        }
                    }
                    break;
                }

                case VCLEVENT_LISTBOX_SELECT :
                {
                    // First send an event that tells the listeners of a
                    // modified selection.  The active descendant event is
                    // send after that so that the receiving AT has time to
                    // read the text or name of the active child.
                    NotifyAccessibleEvent( AccessibleEventId::SELECTION_CHANGED, Any(), Any() );
                    if ( getListBox() && getListBox()->HasFocus() )
                    {
                        SvLBoxEntry* pEntry = static_cast< SvLBoxEntry* >( rVclWindowEvent.GetData() );
                        if ( pEntry )
                        {
                            Reference< XAccessible > xChild = new AccessibleListBoxEntry( *getListBox(), pEntry, this );
                            uno::Any aOldValue, aNewValue;
                            aNewValue <<= xChild;
                            NotifyAccessibleEvent( AccessibleEventId::ACTIVE_DESCENDANT_CHANGED, aOldValue, aNewValue );
                        }
                    }
                    break;
                }
                default:
                    VCLXAccessibleComponent::ProcessWindowEvent (rVclWindowEvent);
            }
        }
    }
    // -----------------------------------------------------------------------------
    void AccessibleListBox::ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent )
    {
        switch ( rVclWindowEvent.GetId() )
        {
            case VCLEVENT_WINDOW_SHOW:
            case VCLEVENT_WINDOW_HIDE:
            {
            }
            break;
            default:
            {
                VCLXAccessibleComponent::ProcessWindowChildEvent( rVclWindowEvent );
            }
            break;
        }
    }

    // -----------------------------------------------------------------------------
    // XComponent
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        VCLXAccessibleComponent::disposing();
        m_xParent = NULL;
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleListBox::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }
    // -----------------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL AccessibleListBox::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBox::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
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
    Sequence< ::rtl::OUString > AccessibleListBox::getSupportedServiceNames_Static(void) throw( RuntimeException )
    {
        Sequence< ::rtl::OUString > aSupported(3);
        aSupported[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.accessibility.AccessibleContext") );
        aSupported[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.accessibility.AccessibleComponent") );
        aSupported[2] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.AccessibleTreeListBox") );
        return aSupported;
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString AccessibleListBox::getImplementationName_Static(void) throw( RuntimeException )
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.svtools.AccessibleTreeListBox") );
    }
    // -----------------------------------------------------------------------------
    // XAccessible
    // -----------------------------------------------------------------------------
    Reference< XAccessibleContext > SAL_CALL AccessibleListBox::getAccessibleContext(  ) throw (RuntimeException)
    {
        ensureAlive();
        return this;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleContext
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBox::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 nCount = 0;
        SvTreeListBox* pSvTreeListBox = getListBox();
        if ( pSvTreeListBox )
            nCount = pSvTreeListBox->GetLevelChildCount( NULL );

        return nCount;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBox::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException,RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        SvLBoxEntry* pEntry = getListBox()->GetEntry(i);
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        return new AccessibleListBoxEntry( *getListBox(), pEntry, this );
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBox::getAccessibleParent(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ensureAlive();
        return m_xParent;
    }
    // -----------------------------------------------------------------------------
    sal_Int16 SAL_CALL AccessibleListBox::getAccessibleRole(  ) throw (RuntimeException)
    {
        return AccessibleRole::TREE;
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleListBox::getAccessibleDescription(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        return getListBox()->GetAccessibleDescription();
    }
    // -----------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleListBox::getAccessibleName(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();
        return getListBox()->GetAccessibleName();
    }
    // -----------------------------------------------------------------------------
    // XAccessibleSelection
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvLBoxEntry* pEntry = getListBox()->GetEntry( nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        getListBox()->Select( pEntry, TRUE );
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBox::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvLBoxEntry* pEntry = getListBox()->GetEntry( nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        return getListBox()->IsSelected( pEntry );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::clearAccessibleSelection(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 i, nCount = 0;
        nCount = getListBox()->GetLevelChildCount( NULL );
        for ( i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( i );
            if ( getListBox()->IsSelected( pEntry ) )
                getListBox()->Select( pEntry, FALSE );
        }
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::selectAllAccessibleChildren(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 i, nCount = 0;
        nCount = getListBox()->GetLevelChildCount( NULL );
        for ( i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( i );
            if ( !getListBox()->IsSelected( pEntry ) )
                getListBox()->Select( pEntry, TRUE );
        }
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBox::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        sal_Int32 i, nSelCount = 0, nCount = 0;
        nCount = getListBox()->GetLevelChildCount( NULL );
        for ( i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( i );
            if ( getListBox()->IsSelected( pEntry ) )
                ++nSelCount;
        }

        return nSelCount;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBox::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild;
        sal_Int32 i, nSelCount = 0, nCount = 0;
        nCount = getListBox()->GetLevelChildCount( NULL );
        for ( i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( i );
            if ( getListBox()->IsSelected( pEntry ) )
                ++nSelCount;

            if ( nSelCount == ( nSelectedChildIndex + 1 ) )
            {
                xChild = new AccessibleListBoxEntry( *getListBox(), pEntry, this );
                break;
            }
        }

        return xChild;
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBox::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::comphelper::OExternalLockGuard aGuard( this );

        ensureAlive();

        SvLBoxEntry* pEntry = getListBox()->GetEntry( nSelectedChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        getListBox()->Select( pEntry, FALSE );
    }
    // -----------------------------------------------------------------------------
    void AccessibleListBox::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
    {
        VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );
        if ( getListBox() && isAlive() )
        {
            rStateSet.AddState( AccessibleStateType::FOCUSABLE );
            rStateSet.AddState( AccessibleStateType::MANAGES_DESCENDANTS );
            if ( getListBox()->GetSelectionMode() == MULTIPLE_SELECTION )
                rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );
        }
    }


//........................................................................
}// namespace accessibility
//........................................................................

