 /*************************************************************************
 *
 *  $RCSfile: accdoc.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:39:16 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif

#pragma hdrstop

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif

#ifndef _ACCDOC_HXX
#include <accdoc.hxx>
#endif
#ifndef _ACCESS_HRC
#include "access.hrc"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif

const sal_Char sServiceName[] = "drafts.com.sun.star.text.AccessibleTextDocumentView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleDocumentView";


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::accessibility;
using namespace ::rtl;

using ::com::sun::star::lang::IndexOutOfBoundsException;



//
// SwAccessibleDocumentBase: base class for SwAccessibleDocument and
// SwAccessiblePreview
//

SwAccessibleDocumentBase::SwAccessibleDocumentBase ( SwAccessibleMap *pMap ) :
    SwAccessibleContext( pMap, AccessibleRole::DOCUMENT,
                           pMap->GetShell()->GetDoc()->GetRootFrm() ),
    xParent( pMap->GetShell()->GetWin()->GetAccessibleParentWindow()->GetAccessible() ),
    pChildWin( 0 )
{
}

SwAccessibleDocumentBase::~SwAccessibleDocumentBase()
{
}

void SwAccessibleDocumentBase::SetVisArea()
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    SwRect aOldVisArea( GetVisArea() );
    const SwRect& rNewVisArea = GetMap()->GetVisArea();
    if( aOldVisArea != rNewVisArea )
    {
        SwAccessibleFrame::SetVisArea( GetMap()->GetVisArea() );
        ChildrenScrolled( GetFrm(), aOldVisArea );
    }
}

void SwAccessibleDocumentBase::AddChild( Window *pWin, sal_Bool bFireEvent )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    ASSERT( !pChildWin, "only one child window is supported" );
    if( !pChildWin )
    {
        pChildWin = pWin;

        if( bFireEvent )
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::ACCESSIBLE_CHILD_EVENT;
            aEvent.NewValue <<= pChildWin->GetAccessible();
            FireAccessibleEvent( aEvent );
        }
    }
}

void SwAccessibleDocumentBase::RemoveChild( Window *pWin )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    ASSERT( !pChildWin || pWin == pChildWin, "invalid child window to remove" );
    if( pChildWin && pWin == pChildWin )
    {
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::ACCESSIBLE_CHILD_EVENT;
        aEvent.OldValue <<= pChildWin->GetAccessible();
        FireAccessibleEvent( aEvent );

        pChildWin = 0;
    }
}

sal_Int32 SAL_CALL SwAccessibleDocumentBase::getAccessibleChildCount( void )
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // CHECK_FOR_DEFUNC is called by parent

    sal_Int32 nChildren = SwAccessibleContext::getAccessibleChildCount();
    if( !IsDisposing() && pChildWin )
        nChildren++;

    return nChildren;
}

Reference< XAccessible> SAL_CALL
    SwAccessibleDocumentBase::getAccessibleChild( long nIndex )
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( pChildWin  )
    {
        CHECK_FOR_DEFUNC( XAccessibleContext )
        if( nIndex == GetChildCount() )
            return pChildWin->GetAccessible();
    }

    return SwAccessibleContext::getAccessibleChild( nIndex );
}


Reference< XAccessible> SAL_CALL SwAccessibleDocumentBase::getAccessibleParent (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return xParent;
}

sal_Int32 SAL_CALL SwAccessibleDocumentBase::getAccessibleIndexInParent (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Reference < XAccessibleContext > xAcc( xParent->getAccessibleContext() );
    Reference < XAccessible > xThis( this );
    sal_Int32 nCount = xAcc->getAccessibleChildCount();

    for( sal_Int32 i=0; i < nCount; i++ )
    {
        if( xAcc->getAccessibleChild( i ) == xThis )
            return i;
    }
    return -1L;
}

OUString SAL_CALL SwAccessibleDocumentBase::getAccessibleDescription (void) throw (com::sun::star::uno::RuntimeException)
{
    return GetResource( STR_ACCESS_DOC_DESC );
}

awt::Rectangle SAL_CALL SwAccessibleDocumentBase::getBounds()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Rectangle aPixBounds( pWin->GetWindowExtentsRelative( pWin->GetAccessibleParentWindow() ) );
    awt::Rectangle aBox( aPixBounds.Left(), aPixBounds.Top(),
                         aPixBounds.GetWidth(), aPixBounds.GetHeight() );

    return aBox;
}


awt::Point SAL_CALL SwAccessibleDocumentBase::getLocation()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPos( pWin->GetWindowExtentsRelative( pWin->GetAccessibleParentWindow() ).TopLeft() );
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Point SAL_CALL SwAccessibleDocumentBase::getLocationOnScreen()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPos( pWin->GetWindowExtentsRelative( 0 ).TopLeft() );
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Size SAL_CALL SwAccessibleDocumentBase::getSize()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Size aPixSize( pWin->GetWindowExtentsRelative( 0 ).GetSize() );
    awt::Size aSize( aPixSize.Width(), aPixSize.Height() );

    return aSize;
}

sal_Bool SAL_CALL SwAccessibleDocumentBase::contains(
            const ::com::sun::star::awt::Point& aPoint )
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Rectangle aPixBounds( pWin->GetWindowExtentsRelative( 0 ) );
    aPixBounds.Move(-aPixBounds.Left(), -aPixBounds.Top());

    Point aPixPoint( aPoint.X, aPoint.Y );
    return aPixBounds.IsInside( aPixPoint );
}

Reference< XAccessible > SAL_CALL SwAccessibleDocumentBase::getAccessibleAt(
                const awt::Point& aPoint )
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( pChildWin  )
    {
        CHECK_FOR_DEFUNC( XAccessibleComponent )

        Window *pWin = GetWindow();
        CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

        Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to window
        if( pChildWin->GetWindowExtentsRelative( pWin ).IsInside( aPixPoint ) )
            return pChildWin->GetAccessible();
    }

    return SwAccessibleContext::getAccessibleAt( aPoint );
}

//
// SwAccessibeDocument
//

void SwAccessibleDocument::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // MULTISELECTABLE
    rStateSet.AddState( AccessibleStateType::MULTISELECTABLE );
}


SwAccessibleDocument::SwAccessibleDocument ( SwAccessibleMap *pMap ) :
    SwAccessibleDocumentBase( pMap ),
    aSelectionHelper( *this )
{
    SetName( GetResource( STR_ACCESS_DOC_NAME ) );
    Window *pWin = pMap->GetShell()->GetWin();
    if( pWin )
    {
        pWin->AddChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
        USHORT nCount =   pWin->GetChildCount();
        for( sal_uInt16 i=0; i < nCount; i++ )
        {
            Window *pChildWin = pWin->GetChild( i );
            if( pChildWin &&
                AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                AddChild( pChildWin, sal_False );
        }
    }
}

SwAccessibleDocument::~SwAccessibleDocument()
{
    Window *pWin = GetMap() ? GetMap()->GetShell()->GetWin() : 0;
    if( pWin )
        pWin->RemoveChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
}

void SwAccessibleDocument::Dispose( sal_Bool bRecursive )
{
    ASSERT( GetFrm() && GetMap(), "already disposed" );

    Window *pWin = GetMap() ? GetMap()->GetShell()->GetWin() : 0;
    if( pWin )
        pWin->RemoveChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
    SwAccessibleContext::Dispose( bRecursive );
}

IMPL_LINK( SwAccessibleDocument, WindowChildEventListener, VclSimpleEvent*, pEvent )
{
    DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "Unknown WindowEvent!" );
    if ( pEvent && pEvent->ISA( VclWindowEvent ) )
    {
        VclWindowEvent *pVclEvent = static_cast< VclWindowEvent * >( pEvent );
        DBG_ASSERT( pVclEvent->GetWindow(), "Window???" );
        switch ( pVclEvent->GetId() )
        {
        case VCLEVENT_WINDOW_SHOW:  // send create on show for direct accessible children
            {
                Window* pChildWin = pVclEvent->GetWindow();
                if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                {
                    AddChild( pChildWin );
                }
            }
            break;
        case VCLEVENT_WINDOW_HIDE:  // send destroy on hide for direct accessible children
        case VCLEVENT_OBJECT_DYING:  // send destroy on hide for direct accessible children
            {
                Window* pChildWin = pVclEvent->GetWindow();
                if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                {
                    RemoveChild( pChildWin );
                }
            }
            break;
        }
    }
    return 0;
}


OUString SAL_CALL SwAccessibleDocument::getImplementationName()
        throw( RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationName));
}

sal_Bool SAL_CALL SwAccessibleDocument::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (::com::sun::star::uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

Sequence< OUString > SAL_CALL SwAccessibleDocument::getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException )
{
    Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

//=====  XInterface  ======================================================

Any SwAccessibleDocument::queryInterface(
    const Type& rType )
    throw ( RuntimeException )
{
    Any aRet;
    if ( rType == ::getCppuType((Reference<XAccessibleSelection> *)NULL) )
    {
        Reference<XAccessibleSelection> aSelect = this;
        aRet <<= aSelect;
    }
    else
        aRet = SwAccessibleContext::queryInterface( rType );
    return aRet;
}

//====== XTypeProvider ====================================================
Sequence< Type > SAL_CALL SwAccessibleDocument::getTypes() throw(RuntimeException)
{
    Sequence< Type > aTypes( SwAccessibleDocumentBase::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    aTypes.realloc( nIndex + 1 );

    Type* pTypes = aTypes.getArray();
    pTypes[nIndex] = ::getCppuType( static_cast< Reference< XAccessibleSelection > * >( 0 ) );

    return aTypes;
}

Sequence< sal_Int8 > SAL_CALL SwAccessibleDocument::getImplementationId()
        throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

//=====  XAccessibleSelection  ============================================

void SwAccessibleDocument::selectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException )
{
    aSelectionHelper.selectAccessibleChild(nChildIndex);
}

sal_Bool SwAccessibleDocument::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException )
{
    return aSelectionHelper.isAccessibleChildSelected(nChildIndex);
}

void SwAccessibleDocument::clearAccessibleSelection(  )
    throw ( RuntimeException )
{
    aSelectionHelper.clearAccessibleSelection();
}

void SwAccessibleDocument::selectAllAccessible(  )
    throw ( RuntimeException )
{
    aSelectionHelper.selectAllAccessible();
}

sal_Int32 SwAccessibleDocument::getSelectedAccessibleChildCount(  )
    throw ( RuntimeException )
{
    return aSelectionHelper.getSelectedAccessibleChildCount();
}

Reference<XAccessible> SwAccessibleDocument::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException)
{
    return aSelectionHelper.getSelectedAccessibleChild(nSelectedChildIndex);
}

void SwAccessibleDocument::deselectSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( IndexOutOfBoundsException,
            RuntimeException )
{
    aSelectionHelper.deselectSelectedAccessibleChild(nSelectedChildIndex);
}
