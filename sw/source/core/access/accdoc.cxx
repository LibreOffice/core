 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accdoc.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:13:10 $
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
#include "precompiled_sw.hxx"
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif


#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
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
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif

const sal_Char sServiceName[] = "com.sun.star.text.AccessibleTextDocumentView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleDocumentView";


using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using ::rtl::OUString;

using lang::IndexOutOfBoundsException;



//
// SwAccessibleDocumentBase: base class for SwAccessibleDocument and
// SwAccessiblePreview
//

SwAccessibleDocumentBase::SwAccessibleDocumentBase ( SwAccessibleMap* pInitMap ) :
    SwAccessibleContext( pInitMap, AccessibleRole::DOCUMENT,
                         pInitMap->GetShell()->getIDocumentLayoutAccess()->GetRootFrm() ),
    mxParent( pInitMap->GetShell()->GetWin()->GetAccessibleParentWindow()->GetAccessible() ),
    mpChildWin( 0 )
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
        // --> OD 2007-12-07 #i58139#
        // showing state of document view needs also be updated.
        // Thus, call method <Scrolled(..)> instead of <ChildrenScrolled(..)>
//        ChildrenScrolled( GetFrm(), aOldVisArea );
        Scrolled( aOldVisArea );
        // <--
    }
}

void SwAccessibleDocumentBase::AddChild( Window *pWin, sal_Bool bFireEvent )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    ASSERT( !mpChildWin, "only one child window is supported" );
    if( !mpChildWin )
    {
        mpChildWin = pWin;

        if( bFireEvent )
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::CHILD;
            aEvent.NewValue <<= mpChildWin->GetAccessible();
            FireAccessibleEvent( aEvent );
        }
    }
}

void SwAccessibleDocumentBase::RemoveChild( Window *pWin )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    ASSERT( !mpChildWin || pWin == mpChildWin, "invalid child window to remove" );
    if( mpChildWin && pWin == mpChildWin )
    {
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::CHILD;
        aEvent.OldValue <<= mpChildWin->GetAccessible();
        FireAccessibleEvent( aEvent );

        mpChildWin = 0;
    }
}

sal_Int32 SAL_CALL SwAccessibleDocumentBase::getAccessibleChildCount( void )
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    // CHECK_FOR_DEFUNC is called by parent

    sal_Int32 nChildren = SwAccessibleContext::getAccessibleChildCount();
    if( !IsDisposing() && mpChildWin )
        nChildren++;

    return nChildren;
}

uno::Reference< XAccessible> SAL_CALL
    SwAccessibleDocumentBase::getAccessibleChild( sal_Int32 nIndex )
        throw (uno::RuntimeException,
                lang::IndexOutOfBoundsException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( mpChildWin  )
    {
        CHECK_FOR_DEFUNC( XAccessibleContext )
        if( nIndex == GetChildCount() )
            return mpChildWin->GetAccessible();
    }

    return SwAccessibleContext::getAccessibleChild( nIndex );
}


uno::Reference< XAccessible> SAL_CALL SwAccessibleDocumentBase::getAccessibleParent (void)
        throw (uno::RuntimeException)
{
    return mxParent;
}

sal_Int32 SAL_CALL SwAccessibleDocumentBase::getAccessibleIndexInParent (void)
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Reference < XAccessibleContext > xAcc( mxParent->getAccessibleContext() );
    uno::Reference < XAccessible > xThis( this );
    sal_Int32 nCount = xAcc->getAccessibleChildCount();

    for( sal_Int32 i=0; i < nCount; i++ )
    {
        if( xAcc->getAccessibleChild( i ) == xThis )
            return i;
    }
    return -1L;
}

OUString SAL_CALL SwAccessibleDocumentBase::getAccessibleDescription (void)
    throw (uno::RuntimeException)
{
    return GetResource( STR_ACCESS_DOC_DESC );
}

awt::Rectangle SAL_CALL SwAccessibleDocumentBase::getBounds()
        throw (uno::RuntimeException)
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
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPos( pWin->GetWindowExtentsRelative( pWin->GetAccessibleParentWindow() ).TopLeft() );
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Point SAL_CALL SwAccessibleDocumentBase::getLocationOnScreen()
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPos( pWin->GetWindowExtentsRelative( 0 ).TopLeft() );
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Size SAL_CALL SwAccessibleDocumentBase::getSize()
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Size aPixSize( pWin->GetWindowExtentsRelative( 0 ).GetSize() );
    awt::Size aSize( aPixSize.Width(), aPixSize.Height() );

    return aSize;
}

sal_Bool SAL_CALL SwAccessibleDocumentBase::containsPoint(
            const awt::Point& aPoint )
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Rectangle aPixBounds( pWin->GetWindowExtentsRelative( 0 ) );
    aPixBounds.Move(-aPixBounds.Left(), -aPixBounds.Top());

    Point aPixPoint( aPoint.X, aPoint.Y );
    return aPixBounds.IsInside( aPixPoint );
}

uno::Reference< XAccessible > SAL_CALL SwAccessibleDocumentBase::getAccessibleAtPoint(
                const awt::Point& aPoint )
        throw (uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    if( mpChildWin  )
    {
        CHECK_FOR_DEFUNC( XAccessibleComponent )

        Window *pWin = GetWindow();
        CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

        Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to window
        if( mpChildWin->GetWindowExtentsRelative( pWin ).IsInside( aPixPoint ) )
            return mpChildWin->GetAccessible();
    }

    return SwAccessibleContext::getAccessibleAtPoint( aPoint );
}

//
// SwAccessibeDocument
//

void SwAccessibleDocument::GetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    SwAccessibleContext::GetStates( rStateSet );

    // MULTISELECTABLE
    rStateSet.AddState( AccessibleStateType::MULTI_SELECTABLE );
}


SwAccessibleDocument::SwAccessibleDocument ( SwAccessibleMap* pInitMap ) :
    SwAccessibleDocumentBase( pInitMap ),
    maSelectionHelper( *this )
{
    SetName( GetResource( STR_ACCESS_DOC_NAME ) );
    Window *pWin = pInitMap->GetShell()->GetWin();
    if( pWin )
    {
        pWin->AddChildEventListener( LINK( this, SwAccessibleDocument, WindowChildEventListener ));
        USHORT nCount =   pWin->GetChildCount();
        for( sal_uInt16 i=0; i < nCount; i++ )
        {
            Window* pChildWin = pWin->GetChild( i );
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
                Window* pChildWin = static_cast< Window* >( pVclEvent->GetData() );
                if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                {
                    AddChild( pChildWin );
                }
            }
            break;
        case VCLEVENT_WINDOW_HIDE:  // send destroy on hide for direct accessible children
            {
                Window* pChildWin = static_cast< Window* >( pVclEvent->GetData() );
                if( pChildWin && AccessibleRole::EMBEDDED_OBJECT == pChildWin->GetAccessibleRole() )
                {
                    RemoveChild( pChildWin );
                }
            }
            break;
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
        throw( uno::RuntimeException )
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM(sImplementationName));
}

sal_Bool SAL_CALL SwAccessibleDocument::supportsService(
        const ::rtl::OUString& sTestServiceName)
    throw (uno::RuntimeException)
{
    return sTestServiceName.equalsAsciiL( sServiceName,
                                          sizeof(sServiceName)-1 ) ||
           sTestServiceName.equalsAsciiL( sAccessibleServiceName,
                                             sizeof(sAccessibleServiceName)-1 );
}

uno::Sequence< OUString > SAL_CALL SwAccessibleDocument::getSupportedServiceNames()
        throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( RTL_CONSTASCII_USTRINGPARAM(sServiceName) );
    pArray[1] = OUString( RTL_CONSTASCII_USTRINGPARAM(sAccessibleServiceName) );
    return aRet;
}

//=====  XInterface  ======================================================

uno::Any SwAccessibleDocument::queryInterface(
    const uno::Type& rType )
    throw ( uno::RuntimeException )
{
    uno::Any aRet;
    if ( rType == ::getCppuType( static_cast< uno::Reference< XAccessibleSelection > * >( 0 ) ) )
    {
        uno::Reference<XAccessibleSelection> aSelect = this;
        aRet <<= aSelect;
    }
    else
        aRet = SwAccessibleContext::queryInterface( rType );
    return aRet;
}

//====== XTypeProvider ====================================================
uno::Sequence< uno::Type > SAL_CALL SwAccessibleDocument::getTypes()
    throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aTypes( SwAccessibleDocumentBase::getTypes() );

    sal_Int32 nIndex = aTypes.getLength();
    aTypes.realloc( nIndex + 1 );

    uno::Type* pTypes = aTypes.getArray();
    pTypes[nIndex] = ::getCppuType( static_cast< uno::Reference< XAccessibleSelection > * >( 0 ) );

    return aTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwAccessibleDocument::getImplementationId()
        throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Sequence< sal_Int8 > aId( 16 );
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
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    maSelectionHelper.selectAccessibleChild(nChildIndex);
}

sal_Bool SwAccessibleDocument::isAccessibleChildSelected(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    return maSelectionHelper.isAccessibleChildSelected(nChildIndex);
}

void SwAccessibleDocument::clearAccessibleSelection(  )
    throw ( uno::RuntimeException )
{
    maSelectionHelper.clearAccessibleSelection();
}

void SwAccessibleDocument::selectAllAccessibleChildren(  )
    throw ( uno::RuntimeException )
{
    maSelectionHelper.selectAllAccessibleChildren();
}

sal_Int32 SwAccessibleDocument::getSelectedAccessibleChildCount(  )
    throw ( uno::RuntimeException )
{
    return maSelectionHelper.getSelectedAccessibleChildCount();
}

uno::Reference<XAccessible> SwAccessibleDocument::getSelectedAccessibleChild(
    sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException)
{
    return maSelectionHelper.getSelectedAccessibleChild(nSelectedChildIndex);
}

// --> OD 2004-11-16 #111714# - index has to be treated as global child index.
void SwAccessibleDocument::deselectAccessibleChild(
    sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException,
            uno::RuntimeException )
{
    maSelectionHelper.deselectAccessibleChild( nChildIndex );
}
