/*************************************************************************
 *
 *  $RCSfile: vclxaccessiblecomponent.cxx,v $
 *
 *  $Revision: 1.44 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-11 17:05:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEEVENTLISTENER_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleEventListener.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLERELATIONTYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRelationType.hpp>
#endif

#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECOMPONENT_HXX_
#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#endif
#ifndef _TOOLKIT_HELPER_EXTERNALLOCK_HXX_
#include <toolkit/helper/externallock.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXFONT_HXX_
#include <toolkit/awt/vclxfont.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX_
#include <unotools/accessiblerelationsethelper.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif

#ifndef VCLEVENT_WINDOW_FRAMETITLECHANGED
#define VCLEVENT_WINDOW_FRAMETITLECHANGED   1018    // pData = XubString* = oldTitle
#endif

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;
using namespace ::comphelper;


DBG_NAME(VCLXAccessibleComponent);


//  ----------------------------------------------------
//  class VCLXAccessibleComponent
//  ----------------------------------------------------
VCLXAccessibleComponent::VCLXAccessibleComponent( VCLXWindow* pVCLXindow )
    : AccessibleExtendedComponentHelper_BASE( new VCLExternalSolarLock() )
    , OAccessibleImplementationAccess( )
{
    DBG_CTOR( VCLXAccessibleComponent, 0 );
    mpVCLXindow = pVCLXindow;
    mxWindow = pVCLXindow;

    m_pSolarLock = static_cast< VCLExternalSolarLock* >( getExternalLock( ) );

    DBG_ASSERT( pVCLXindow->GetWindow(), "VCLXAccessibleComponent - no window!" );
    if ( pVCLXindow->GetWindow() )
    {
      pVCLXindow->GetWindow()->AddEventListener( LINK( this, VCLXAccessibleComponent, WindowEventListener ) );
      pVCLXindow->GetWindow()->AddChildEventListener( LINK( this, VCLXAccessibleComponent, WindowChildEventListener ) );
    }

    // announce the XAccessible of our creator to the base class
    lateInit( pVCLXindow );
}

VCLXAccessibleComponent::~VCLXAccessibleComponent()
{
    DBG_DTOR( VCLXAccessibleComponent, 0 );

    ensureDisposed();

    if ( mpVCLXindow && mpVCLXindow->GetWindow() )
    {
        mpVCLXindow->GetWindow()->RemoveEventListener( LINK( this, VCLXAccessibleComponent, WindowEventListener ) );
        mpVCLXindow->GetWindow()->RemoveChildEventListener( LINK( this, VCLXAccessibleComponent, WindowChildEventListener ) );
    }

    delete m_pSolarLock;
    m_pSolarLock = NULL;
    // This is not completely sure. If we assume that the base class dtor calls some method which
    // uses this lock, the we crash. However, as the base class' dtor does not have a chance to call _out_
    // virtual methods, this is no problem as long as the base class is safe, i.e. does not use the external
    // lock from within it's dtor. At the moment, we _know_ the base class is safe in this respect, so
    // let's assume it keeps this way.
    // @see OAccessibleContextHelper::OAccessibleContextHelper( IMutex* )
}

IMPLEMENT_FORWARD_XINTERFACE3( VCLXAccessibleComponent, AccessibleExtendedComponentHelper_BASE, OAccessibleImplementationAccess, VCLXAccessibleComponent_BASE )
IMPLEMENT_FORWARD_XTYPEPROVIDER3( VCLXAccessibleComponent, AccessibleExtendedComponentHelper_BASE, OAccessibleImplementationAccess, VCLXAccessibleComponent_BASE )

::rtl::OUString VCLXAccessibleComponent::getImplementationName() throw (uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleWindow" );
}

sal_Bool VCLXAccessibleComponent::supportsService( const ::rtl::OUString& rServiceName ) throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
    const ::rtl::OUString* pNames = aNames.getConstArray();
    const ::rtl::OUString* pEnd = pNames + aNames.getLength();
    for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
        ;

    return pNames != pEnd;
}

uno::Sequence< ::rtl::OUString > VCLXAccessibleComponent::getSupportedServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "drafts.com.sun.star.awt.AccessibleWindow" );
    return aNames;
}

IMPL_LINK( VCLXAccessibleComponent, WindowEventListener, VclSimpleEvent*, pEvent )
{
    DBG_CHKTHIS(VCLXAccessibleComponent,0);

    DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );
    if ( pEvent && pEvent->ISA( VclWindowEvent ) )
    {
        DBG_ASSERT( ((VclWindowEvent*)pEvent)->GetWindow(), "Window???" );
        if( !((VclWindowEvent*)pEvent)->GetWindow()->IsAccessibilityEventsSuppressed() || ( pEvent->GetId() == VCLEVENT_OBJECT_DYING ) )
        {
            ProcessWindowEvent( *(VclWindowEvent*)pEvent );
        }
    }
    return 0;
}

IMPL_LINK( VCLXAccessibleComponent, WindowChildEventListener, VclSimpleEvent*, pEvent )
{
    DBG_CHKTHIS(VCLXAccessibleComponent,0);

    DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );
    if ( pEvent && pEvent->ISA( VclWindowEvent ) )
    {
        DBG_ASSERT( ((VclWindowEvent*)pEvent)->GetWindow(), "Window???" );
        if( !((VclWindowEvent*)pEvent)->GetWindow()->IsAccessibilityEventsSuppressed() )
        {
            // #103087# to prevent an early release of the component
            uno::Reference< accessibility::XAccessibleContext > xTmp = this;

            ProcessWindowChildEvent( *(VclWindowEvent*)pEvent );
        }
    }
    return 0;
}

uno::Reference< accessibility::XAccessible > VCLXAccessibleComponent::GetChildAccessible( const VclWindowEvent& rVclWindowEvent )
{
    // checks if the data in the window event is our direct child
    // and returns its accessible

    // MT: Change this later, normaly a show/hide event shouldn't have the Window* in pData.
    Window* pChildWindow = (Window *) rVclWindowEvent.GetData();
    if( pChildWindow && GetWindow() == pChildWindow->GetAccessibleParentWindow() )
        return pChildWindow->GetAccessible( rVclWindowEvent.GetId() == VCLEVENT_WINDOW_SHOW );
    else
        return uno::Reference< accessibility::XAccessible > ();
}

void VCLXAccessibleComponent::ProcessWindowChildEvent( const VclWindowEvent& rVclWindowEvent )
{
    uno::Any aOldValue, aNewValue;
    uno::Reference< accessibility::XAccessible > xAcc;

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_SHOW:  // send create on show for direct accessible children
        {
            xAcc = GetChildAccessible( rVclWindowEvent );
            if( xAcc.is() )
            {
                aNewValue <<= xAcc;
                NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_CHILD_EVENT, aOldValue, aNewValue );
            }
        }
        break;
        case VCLEVENT_WINDOW_HIDE:  // send destroy on hide for direct accessible children
        {
            xAcc = GetChildAccessible( rVclWindowEvent );
            if( xAcc.is() )
            {
                aOldValue <<= xAcc;
                NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_CHILD_EVENT, aOldValue, aNewValue );
            }
        }
        break;
    }
}

void VCLXAccessibleComponent::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    uno::Any aOldValue, aNewValue;

    Window* pWindow = rVclWindowEvent.GetWindow();
    DBG_ASSERT( pWindow, "VCLXAccessibleComponent::ProcessWindowEvent - Window?" );

    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_OBJECT_DYING:
        {
            pWindow->RemoveEventListener( LINK( this, VCLXAccessibleComponent, WindowEventListener ) );
            pWindow->RemoveChildEventListener( LINK( this, VCLXAccessibleComponent, WindowChildEventListener ) );
            mxWindow.clear();
            mpVCLXindow = NULL;
        }
        break;
        //
        // dont handle CHILDCREATED events here
        // they are handled separately as child events, see ProcessWindowChildEvent above
        //
        /*
        case VCLEVENT_WINDOW_CHILDCREATED:
        {
            Window* pWindow = (Window*) rVclWindowEvent.GetData();
            DBG_ASSERT( pWindow, "VCLEVENT_WINDOW_CHILDCREATED - Window=?" );
            aNewValue <<= pWindow->GetAccessible();
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_CHILD_EVENT, aOldValue, aNewValue );
        }
        break;
        */
        case VCLEVENT_WINDOW_CHILDDESTROYED:
        {
            Window* pWindow = (Window*) rVclWindowEvent.GetData();
            DBG_ASSERT( pWindow, "VCLEVENT_WINDOW_CHILDDESTROYED - Window=?" );
            if ( pWindow->GetAccessible( FALSE ).is() )
            {
                aOldValue <<= pWindow->GetAccessible( FALSE );
                NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_CHILD_EVENT, aOldValue, aNewValue );
            }
        }
        break;

        //
        // show and hide will be handled as child events only and are
        // responsible for sending create/destroy events, see ProcessWindowChildEvent above
        //
        /*
        case VCLEVENT_WINDOW_SHOW:
        {
            aNewValue <<= accessibility::AccessibleStateType::VISIBLE;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );

            aNewValue <<= accessibility::AccessibleStateType::SHOWING;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );

            aNewValue.clear();
            aOldValue <<= accessibility::AccessibleStateType::INVALID;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_HIDE:
        {
            aOldValue <<= accessibility::AccessibleStateType::VISIBLE;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );

            aOldValue <<= accessibility::AccessibleStateType::SHOWING;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );

            aOldValue.clear();
            aNewValue <<= accessibility::AccessibleStateType::INVALID;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
        }
        break;
        */
        case VCLEVENT_WINDOW_ACTIVATE:
        {
            // avoid notification if a child frame is already active
            // only one frame may be active at a given time
            if( !pWindow->HasActiveChildFrame() )
            {
                aNewValue <<= accessibility::AccessibleStateType::ACTIVE;
                NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
            }
        }
        break;
        case VCLEVENT_WINDOW_DEACTIVATE:
        {
            aOldValue <<= accessibility::AccessibleStateType::ACTIVE;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_GETFOCUS:
        case VCLEVENT_CONTROL_GETFOCUS:
        {
            if( (pWindow->IsCompoundControl() && rVclWindowEvent.GetId() == VCLEVENT_CONTROL_GETFOCUS) ||
                (!pWindow->IsCompoundControl() && rVclWindowEvent.GetId() == VCLEVENT_WINDOW_GETFOCUS) )
            {
                // if multiple listeners were registered it is possible that the
                // focus was changed during event processing (eg SfxTopWindow )
                // #106082# allow ChildPathFocus only for CompoundControls, for windows the focus must be in the window itself
                if( (pWindow->IsCompoundControl() && pWindow->HasChildPathFocus()) ||
                    (!pWindow->IsCompoundControl() && pWindow->HasFocus()) )
                {
                    aNewValue <<= accessibility::AccessibleStateType::FOCUSED;
                    NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_LOSEFOCUS:
        case VCLEVENT_CONTROL_LOSEFOCUS:
        {
            if( (pWindow->IsCompoundControl() && rVclWindowEvent.GetId() == VCLEVENT_CONTROL_LOSEFOCUS) ||
                (!pWindow->IsCompoundControl() && rVclWindowEvent.GetId() == VCLEVENT_WINDOW_LOSEFOCUS) )
            {
                aOldValue <<= accessibility::AccessibleStateType::FOCUSED;
                NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
            }
        }
        break;
        case VCLEVENT_WINDOW_FRAMETITLECHANGED:
        {
            ::rtl::OUString aOldName( *((::rtl::OUString*) rVclWindowEvent.GetData()) );
            ::rtl::OUString aNewName( getAccessibleName() );
            aOldValue <<= aOldName;
            aNewValue <<= aNewName;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_NAME_EVENT, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_ENABLED:
        {
            aNewValue <<= accessibility::AccessibleStateType::ENABLED;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_DISABLED:
        {
            aOldValue <<= accessibility::AccessibleStateType::ENABLED;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_MOVE:
        case VCLEVENT_WINDOW_RESIZE:
        {
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_BOUNDRECT_EVENT, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_MENUBARADDED:
        {
            MenuBar* pMenuBar = (MenuBar*) rVclWindowEvent.GetData();
            if ( pMenuBar )
            {
                uno::Reference< accessibility::XAccessible > xChild( pMenuBar->GetAccessible() );
                if ( xChild.is() )
                {
                    aNewValue <<= xChild;
                    NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_CHILD_EVENT, aOldValue, aNewValue );
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_MENUBARREMOVED:
        {
            MenuBar* pMenuBar = (MenuBar*) rVclWindowEvent.GetData();
            if ( pMenuBar )
            {
                uno::Reference< accessibility::XAccessible > xChild( pMenuBar->GetAccessible() );
                if ( xChild.is() )
                {
                    aOldValue <<= xChild;
                    NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_CHILD_EVENT, aOldValue, aNewValue );
                }
            }
        }
        break;
        case VCLEVENT_WINDOW_ICONIFIED:
        {
            aNewValue <<= accessibility::AccessibleStateType::ICONIFIED;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
        }
        break;
        case VCLEVENT_WINDOW_RESTORED:
        {
            aOldValue <<= accessibility::AccessibleStateType::ICONIFIED;
            NotifyAccessibleEvent( accessibility::AccessibleEventId::ACCESSIBLE_STATE_EVENT, aOldValue, aNewValue );
        }
        break;
        default:
        {
        }
        break;
    }
}

void VCLXAccessibleComponent::disposing()
{
    if ( mpVCLXindow && mpVCLXindow->GetWindow() )
    {
        mpVCLXindow->GetWindow()->RemoveEventListener( LINK( this, VCLXAccessibleComponent, WindowEventListener ) );
        mpVCLXindow->GetWindow()->RemoveChildEventListener( LINK( this, VCLXAccessibleComponent, WindowChildEventListener ) );
    }

    AccessibleExtendedComponentHelper_BASE::disposing();

    mxWindow.clear();
    mpVCLXindow = NULL;
}

Window* VCLXAccessibleComponent::GetWindow() const
{
    return GetVCLXWindow() ? GetVCLXWindow()->GetWindow() : NULL;
}

void VCLXAccessibleComponent::FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet )
{
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        Window *pLabeledBy = pWindow->GetLabeledBy();
        if ( pLabeledBy && pLabeledBy != pWindow )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence(1);
            aSequence[0] = pLabeledBy->GetAccessible();
            rRelationSet.AddRelation( accessibility::AccessibleRelation( accessibility::AccessibleRelationType::LABELED_BY, aSequence ) );
        }

        Window* pLabelFor = pWindow->GetLabelFor();
        if ( pLabelFor && pLabelFor != pWindow )
        {
            uno::Sequence< uno::Reference< uno::XInterface > > aSequence(1);
            aSequence[0] = pLabelFor->GetAccessible();
            rRelationSet.AddRelation( accessibility::AccessibleRelation( accessibility::AccessibleRelationType::LABEL_FOR, aSequence ) );
        }
    }
}

void VCLXAccessibleComponent::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        if ( pWindow->IsVisible() )
        {
            rStateSet.AddState( accessibility::AccessibleStateType::VISIBLE );
            rStateSet.AddState( accessibility::AccessibleStateType::SHOWING );
        }
        else
        {
            rStateSet.AddState( accessibility::AccessibleStateType::INVALID );
        }

        if ( pWindow->IsEnabled() )
            rStateSet.AddState( accessibility::AccessibleStateType::ENABLED );

        if ( pWindow->HasChildPathFocus() )
            rStateSet.AddState( accessibility::AccessibleStateType::ACTIVE );

        Window* pChild = pWindow->GetWindow( WINDOW_FIRSTCHILD );
        if ( ( !pWindow->IsCompoundControl() && pWindow->HasFocus() ) ||
             ( pWindow->IsCompoundControl() && pChild && pChild->HasFocus() ) )
            rStateSet.AddState( accessibility::AccessibleStateType::FOCUSED );

        if ( pWindow->IsWait() )
            rStateSet.AddState( accessibility::AccessibleStateType::BUSY );

        if ( pWindow->GetStyle() & WB_SIZEABLE )
            rStateSet.AddState( accessibility::AccessibleStateType::RESIZABLE );

        if( pWindow->IsDialog() )
        {
            Dialog *pDlg = static_cast< Dialog* >( pWindow );
            if( pDlg->IsInExecute() )
                rStateSet.AddState( accessibility::AccessibleStateType::MODAL );
        }
    }
    else
    {
        rStateSet.AddState( accessibility::AccessibleStateType::DEFUNC );
    }

/*

MUST BE SET FROM DERIVED CLASSES:

CHECKED
COLLAPSED
EXPANDED
EXPANDABLE
EDITABLE
FOCUSABLE
HORIZONTAL
VERTICAL
ICONIFIED
MULTILINE
MULTISELECTABLE
PRESSED
SELECTABLE
SELECTED
SINGLE_LINE
TRANSIENT

    */
}


// accessibility::XAccessibleContext
sal_Int32 VCLXAccessibleComponent::getAccessibleChildCount() throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nChildren = 0;
    if ( GetWindow() )
        nChildren = GetWindow()->GetAccessibleChildWindowCount();

    return nChildren;
}

uno::Reference< accessibility::XAccessible > VCLXAccessibleComponent::getAccessibleChild( sal_Int32 i ) throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( i >= getAccessibleChildCount() )
        throw lang::IndexOutOfBoundsException();

    uno::Reference< accessibility::XAccessible > xAcc;
    if ( GetWindow() )
    {
        Window* pChild = GetWindow()->GetAccessibleChildWindow( (USHORT)i );
        if ( pChild )
            xAcc = pChild->GetAccessible();
    }

    return xAcc;
}

uno::Reference< accessibility::XAccessible > VCLXAccessibleComponent::getVclParent() const
{
    uno::Reference< accessibility::XAccessible > xAcc;
    if ( GetWindow() )
    {
        Window* pParent = GetWindow()->GetAccessibleParentWindow();
        if ( pParent )
            xAcc = pParent->GetAccessible();
    }
    return xAcc;
}

uno::Reference< accessibility::XAccessible > VCLXAccessibleComponent::getAccessibleParent(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    uno::Reference< accessibility::XAccessible > xAcc( implGetForeignControlledParent() );
    if ( !xAcc.is() )
        // we do _not_ have a foreign-controlled parent -> default to our VCL parent
        xAcc = getVclParent();

    return xAcc;
}

sal_Int32 VCLXAccessibleComponent::getAccessibleIndexInParent(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndex = -1;

    uno::Reference< accessibility::XAccessible > xAcc( implGetForeignControlledParent() );
    if ( xAcc.is() )
    {   // we _do_ have a foreign-controlled parent -> use the base class' implementation,
        // which goes the UNO way
        nIndex = AccessibleExtendedComponentHelper_BASE::getAccessibleIndexInParent( );
    }
    else
    {
        if ( GetWindow() )
        {
            Window* pParent = GetWindow()->GetAccessibleParentWindow();
            if ( pParent )
            {
                /*
                for ( USHORT n = pParent->GetAccessibleChildWindowCount(); n; )
                {
                    Window* pChild = pParent->GetAccessibleChildWindow( --n );
                    if ( pChild == GetWindow() )
                    {
                        nIndex = n;
                        break;
                    }
                }
                */
                //  Iterate over all the parent's children and search for this object.
                // this should be compatible with the code in SVX
                uno::Reference< accessibility::XAccessible > xParentAcc( pParent->GetAccessible() );
                if ( xParentAcc.is() )
                {
                    uno::Reference< accessibility::XAccessibleContext > xParentContext ( xParentAcc->getAccessibleContext() );
                    if ( xParentContext.is() )
                    {
                        sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
                        for ( sal_Int32 i=0; i<nChildCount; i++ )
                        {
                            uno::Reference< accessibility::XAccessible > xChild( xParentContext->getAccessibleChild(i) );
                            if ( xChild.is() )
                            {
                                uno::Reference< accessibility::XAccessibleContext > xChildContext = xChild->getAccessibleContext();
                                if ( xChildContext == (accessibility::XAccessibleContext*) this )
                                {
                                    nIndex = i;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return nIndex;
}

sal_Int16 VCLXAccessibleComponent::getAccessibleRole(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int16 nRole = 0;

    if ( GetWindow() )
        nRole = GetWindow()->GetAccessibleRole();

    return nRole;
}

::rtl::OUString VCLXAccessibleComponent::getAccessibleDescription(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    ::rtl::OUString aDescription;

    if ( GetWindow() )
        aDescription = GetWindow()->GetAccessibleDescription();

    return aDescription;
}

::rtl::OUString VCLXAccessibleComponent::getAccessibleName(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    ::rtl::OUString aName;
    if ( GetWindow() )
    {
        aName = GetWindow()->GetAccessibleName();
#ifdef DEBUG
        aName += String( RTL_CONSTASCII_USTRINGPARAM( " (Type = " ) );
        aName += String::CreateFromInt32( GetWindow()->GetType() );
        aName += String( RTL_CONSTASCII_USTRINGPARAM( ")" ) );
#endif
    }
    return aName;
}

uno::Reference< accessibility::XAccessibleRelationSet > VCLXAccessibleComponent::getAccessibleRelationSet(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    uno::Reference< accessibility::XAccessibleRelationSet > xSet = pRelationSetHelper;
    FillAccessibleRelationSet( *pRelationSetHelper );
    return xSet;
}

uno::Reference< accessibility::XAccessibleStateSet > VCLXAccessibleComponent::getAccessibleStateSet(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    uno::Reference< accessibility::XAccessibleStateSet > xSet = pStateSetHelper;
    FillAccessibleStateSet( *pStateSetHelper );
    return xSet;
}

lang::Locale VCLXAccessibleComponent::getLocale() throw (accessibility::IllegalAccessibleComponentStateException, uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLocale();
}

uno::Reference< accessibility::XAccessible > VCLXAccessibleComponent::getAccessibleAt( const awt::Point& rPoint ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    uno::Reference< accessibility::XAccessible > xAcc;

    if ( GetWindow() )
    {
        Window* pWindow = GetWindow()->FindWindow( VCLPoint( rPoint ) );
        if ( pWindow && pWindow != GetWindow() )
            xAcc = pWindow->GetAccessible();

    }
    return xAcc;
}

// accessibility::XAccessibleComponent
awt::Rectangle VCLXAccessibleComponent::implGetBounds() throw (uno::RuntimeException)
{
    awt::Rectangle aBounds ( 0, 0, 0, 0 );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        Rectangle aRect = pWindow->GetWindowExtentsRelative( NULL );
        aBounds = AWTRectangle( aRect );
        Window* pParent = pWindow->GetAccessibleParentWindow();
        if ( pParent )
        {
            Rectangle aParentRect = pParent->GetWindowExtentsRelative( NULL );
            awt::Point aParentScreenLoc = AWTPoint( aParentRect.TopLeft() );
            aBounds.X -= aParentScreenLoc.X;
            aBounds.Y -= aParentScreenLoc.Y;
        }
    }

    uno::Reference< accessibility::XAccessible > xParent( implGetForeignControlledParent() );
    if ( xParent.is() )
    {   // hmm, we can't rely on our VCL coordinates, as in the Accessibility Hierarchy, somebody gave
        // us a parent which is different from our VCL parent
        // (actually, we did not check if it's really different ...)

        // the screen location of the foreign parent
        uno::Reference< accessibility::XAccessibleComponent > xParentComponent( xParent->getAccessibleContext(), uno::UNO_QUERY );
        DBG_ASSERT( xParentComponent.is(), "VCLXAccessibleComponent::implGetBounds: invalid (foreign) parent component!" );

        awt::Point aScreenLocForeign( 0, 0 );
        if ( xParentComponent.is() )
            aScreenLocForeign = xParentComponent->getLocationOnScreen();

        // the screen location of the VCL parent
        xParent = getVclParent();
        if ( xParent.is() )
            xParentComponent = xParentComponent.query( xParent->getAccessibleContext() );

        awt::Point aScreenLocVCL( 0, 0 );
        if ( xParentComponent.is() )
            aScreenLocVCL = xParentComponent->getLocationOnScreen();

        // the difference between them
        awt::Size aOffset( aScreenLocVCL.X - aScreenLocForeign.X, aScreenLocVCL.Y - aScreenLocForeign.Y );
        // move the bounds
        aBounds.X += aOffset.Width;
        aBounds.Y += aOffset.Height;
    }

    return aBounds;
}

awt::Point VCLXAccessibleComponent::getLocationOnScreen(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    awt::Point aPos;
    if ( GetWindow() )
    {
        Rectangle aRect = GetWindow()->GetWindowExtentsRelative( NULL );
        aPos.X = aRect.Left();
        aPos.Y = aRect.Top();
    }

    return aPos;
}

void VCLXAccessibleComponent::grabFocus(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    uno::Reference< accessibility::XAccessibleStateSet > xStates = getAccessibleStateSet();
    if ( mxWindow.is() && xStates.is() && xStates->contains( accessibility::AccessibleStateType::FOCUSABLE ) )
        mxWindow->setFocus();
}

sal_Int32 SAL_CALL VCLXAccessibleComponent::getForeground(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        if ( pWindow->IsControlForeground() )
            nColor = pWindow->GetControlForeground().GetColor();
        else
        {
            Font aFont;
            if ( pWindow->IsControlFont() )
                aFont = pWindow->GetControlFont();
            else
                aFont = pWindow->GetFont();
            nColor = aFont.GetColor().GetColor();
        }
    }

    return nColor;
}

sal_Int32 SAL_CALL VCLXAccessibleComponent::getBackground(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        if ( pWindow->IsControlBackground() )
            nColor = pWindow->GetControlBackground().GetColor();
        else
            nColor = pWindow->GetBackground().GetColor().GetColor();
    }

    return nColor;
}

// XAccessibleExtendedComponent

uno::Reference< awt::XFont > SAL_CALL VCLXAccessibleComponent::getFont(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    uno::Reference< awt::XFont > xFont;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        uno::Reference< awt::XDevice > xDev( pWindow->GetComponentInterface(), uno::UNO_QUERY );
        if ( xDev.is() )
        {
            Font aFont;
            if ( pWindow->IsControlFont() )
                aFont = pWindow->GetControlFont();
            else
                aFont = pWindow->GetFont();
            VCLXFont* pVCLXFont = new VCLXFont;
            pVCLXFont->Init( *xDev.get(), aFont );
            xFont = pVCLXFont;
        }
    }

    return xFont;
}

::rtl::OUString SAL_CALL VCLXAccessibleComponent::getTitledBorderText(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    ::rtl::OUString sRet;
    if ( GetWindow() )
        sRet = GetWindow()->GetText();

    return sRet;
}

::rtl::OUString SAL_CALL VCLXAccessibleComponent::getToolTipText(  ) throw (uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    ::rtl::OUString sRet;
    if ( GetWindow() )
        sRet = GetWindow()->GetQuickHelpText();

    return sRet;
}

