 /*************************************************************************
 *
 *  $RCSfile: acccontext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mib $ $Date: 2002-02-05 15:52:06 $
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
#ifdef DEBUG
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _ERRHDL_HXX
#include "errhdl.hxx"
#endif
#ifndef _SWTYPES_HXX
#include "swtypes.hxx"
#endif

#pragma hdrstop

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATESET_HPP_
#include <drafts/com/sun/star/accessibility/XAccessibleStateSet.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStatetype.hpp>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _ACCMAP_HXX
#include <accmap.hxx>
#endif
#ifndef _ACCCONTEXT_HXX
#include <acccontext.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::drafts::com::sun::star::accessibility;
using namespace ::rtl;

void SwAccessibleContext::LowerAdded( const SwFrm *pFrm )
{
    ::vos::ORef < SwAccessibleContext > xChildImpl(
            aAccMap.GetContextImpl( GetVisArea(), pFrm ) );

    OUString sPropName( RTL_CONSTASCII_USTRINGPARAM( "AccessibleChild" ) );
    PropertyChangeEvent aEvent;
    aEvent.PropertyName = sPropName;
    Reference < XAccessible > xChild( xChildImpl.getBodyPtr() );
    aEvent.NewValue <<= xChild;

    DBG_MSG2( "child added", xChildImpl.getBodyPtr() );
    PropertyChanged( aEvent );

    // Now update the frames childs. In fact, they are updated after
    // the update event for the parent has been send. This ensures
    // that the parent is valid when some of the child broadcasts an
    // event.
    xChildImpl->SetVisArea( GetVisArea() );
}

void SwAccessibleContext::LowerRemoved( const SwFrm *pFrm )
{
    // get child
    ::vos::ORef < SwAccessibleContext > xChildImpl(
            aAccMap.GetContextImpl( GetVisArea(), pFrm ) );

    // Now update the frame's children. They might vanish now, too. Updating
    // them now however makes sure they vanish before the current
    // object (and therfore their parent) is destroyed.
    xChildImpl->SetVisArea( GetVisArea() );

    OUString sPropName( RTL_CONSTASCII_USTRINGPARAM( "AccessibleChild" ) );
    PropertyChangeEvent aEvent;
    aEvent.PropertyName = sPropName;
    Reference < XAccessible > xChild( xChildImpl.getBodyPtr() );
    aEvent.OldValue <<= xChild;

    DBG_MSG2( "child removed", xChildImpl.getBodyPtr() );
    PropertyChanged( aEvent );

    xChildImpl->Dispose();
}

void SwAccessibleContext::LowerMoved( const SwFrm *pFrm )
{
    // get child
    ::vos::ORef < SwAccessibleContext > xChildImpl(
            aAccMap.GetContextImpl( GetVisArea(), pFrm ) );

    OUString sPropName( RTL_CONSTASCII_USTRINGPARAM( "AccessibleVisibleData" ) );
    PropertyChangeEvent aEvent;
    aEvent.PropertyName = sPropName;

    DBG_MSG2( "child moved", xChildImpl.getBodyPtr() );
    xChildImpl->PropertyChanged( aEvent );

    // Now update the frame's children. In fact, they are updated after
    // the update event for the parent has been send.
    xChildImpl->SetVisArea( GetVisArea() );

}

void SwAccessibleContext::Dispose()
{
    DBG_MSG( "disposed" );
    EventObject aEvent;
    Reference < XAccessibleContext > xThis( this );
    aEvent.Source = xThis;
    aPropChangedListeners.disposeAndClear( aEvent );
    aFocusListeners.disposeAndClear( aEvent );
    ASSERT( GetFrm(), "already disposed" );
    if( GetFrm() )
        aAccMap.RemoveContext( this );
    ClearFrm();
}

void SwAccessibleContext::PropertyChanged( PropertyChangeEvent& rEvent )
{
    Reference < XAccessibleContext > xThis( this );
    rEvent.Source = xThis;

    ::cppu::OInterfaceIteratorHelper aIter( aPropChangedListeners );
    while( aIter.hasMoreElements() )
    {
        Reference < XPropertyChangeListener > xListener( aIter.next(),
                                                         UNO_QUERY );
        xListener->propertyChange( rEvent );
    }

}

void SwAccessibleContext::SetStates(
        ::utl::AccessibleStateSetHelper& rStateSet )
{
    // DEFUNC and SHOWING
    if( GetFrm() )
    {
        ASSERT( GetBounds( GetFrm() ).IsOver( GetVisArea() ),
                "invisible object is not disposed" );
        rStateSet.AddState( AccessibleStateType::SHOWING );
    }
    else
    {
        rStateSet.AddState( AccessibleStateType::DEFUNC );
    }

    // EDITABLE
    if( IsEditable() )
        rStateSet.AddState( AccessibleStateType::EDITABLE );

    // ENABLED
    rStateSet.AddState( AccessibleStateType::ENABLED );

    // OPAQUE
    if( IsOpaque() )
        rStateSet.AddState( AccessibleStateType::OPAQUE );

    // VISIBLE
    rStateSet.AddState( AccessibleStateType::VISIBLE );
}

OUString SwAccessibleContext::GetResource( sal_uInt16 nResId,
                                           const OUString *pArg1,
                                           const OUString *pArg2 ) const
{
    String sStr;
    {
        vos::OGuard aGuard(Application::GetSolarMutex());

        sStr = SW_RES( nResId );
    }

    if( pArg1 )
    {
        sStr.SearchAndReplace( String::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "$(ARG1)" )),
                               String( *pArg1 ) );
    }
    if( pArg2 )
    {
        sStr.SearchAndReplace( String::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "$(ARG2)" )),
                               String( *pArg2 ) );
    }

    return OUString( sStr );
}

SwAccessibleContext::SwAccessibleContext( sal_Int16 nR,
                                          const Rectangle& rVisArea,
                                          const SwFrm *pF ) :
    SwAccessibleFrame( rVisArea, pF ),
    aPropChangedListeners( aMutex ),
    aFocusListeners( aMutex ),
    nRole( nR )
{
    DBG_MSG( "constructed" );
}

SwAccessibleContext::SwAccessibleContext( const OUString& rName,
                                          sal_Int16 nR,
                                          const Rectangle& rVisArea,
                                          const SwFrm *pF ) :
    SwAccessibleFrame( rVisArea, pF ),
    sName( rName ),
    aPropChangedListeners( aMutex ),
    aFocusListeners( aMutex ),
    nRole( nR )
{
    DBG_MSG( "constructed" );
}

SwAccessibleContext::~SwAccessibleContext()
{
    DBG_MSG( "destructed" );
    if( GetFrm() )
        aAccMap.RemoveContext( this );
}

Reference< XAccessibleContext > SAL_CALL
    SwAccessibleContext::getAccessibleContext( void )
{
    Reference < XAccessibleContext > xRet( this );
    return xRet;
}

long SAL_CALL SwAccessibleContext::getAccessibleChildCount( void )
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )
    return GetLowerCount();
}

Reference< XAccessible> SAL_CALL
    SwAccessibleContext::getAccessibleChild( long nIndex )
        throw (::com::sun::star::uno::RuntimeException,
                ::com::sun::star::lang::IndexOutOfBoundsException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    const SwFrm *pChild = GetLower( nIndex );
    if( 0 == pChild )
    {
        Reference < XAccessibleContext > xThis( this );
        IndexOutOfBoundsException aExcept(
                OUString( RTL_CONSTASCII_USTRINGPARAM("index out of bounds") ),
                xThis );                                        \
        throw aExcept;
    }

    Reference< XAccessible > xAcc( aAccMap.GetContext( GetVisArea(), pChild ) );

    return xAcc;
}

Reference< XAccessible> SAL_CALL SwAccessibleContext::getAccessibleParent (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    const SwFrm *pUpper = GetUpper();
    ASSERT( pUpper, "no upper found" );

    Reference< XAccessible > xAcc;
    if( pUpper )
        xAcc = aAccMap.GetContext( GetVisArea(), pUpper );

    ASSERT( xAcc.is(), "no parent found" );
    if( !xAcc.is() )
    {
        THROW_RUNTIME_EXCEPTION( XAccessibleContext, "parent missing" );
    }

    return xAcc;
}

sal_Int32 SAL_CALL SwAccessibleContext::getAccessibleIndexInParent (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleContext )

    const SwFrm *pUpper = GetUpper();
    ASSERT( pUpper, "no upper found" );

    sal_Int32 nIndex = -1;
    if( pUpper )
    {
        ::vos::ORef < SwAccessibleContext > xAccImpl(
            aAccMap.GetContextImpl( GetVisArea(), pUpper )  );
        ASSERT( xAccImpl.isValid(), "no parent found" );
        if( xAccImpl.isValid() )
            nIndex = xAccImpl->GetLowerIndex( GetFrm() );
    }
    if( -1 == nIndex )
    {
        THROW_RUNTIME_EXCEPTION( XAccessibleContext, "child not contained in parent" );
    }

    return nIndex;
}

sal_Int16 SAL_CALL SwAccessibleContext::getAccessibleRole (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return nRole;
}

OUString SAL_CALL SwAccessibleContext::getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    ASSERT( !this, "description needs to be overloaded" );
    THROW_RUNTIME_EXCEPTION( XAccessibleContext, "internal error (method must be overloaded)" );
}

OUString SAL_CALL SwAccessibleContext::getAccessibleName (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    return sName;
}

Reference< XAccessibleRelationSet> SAL_CALL
    SwAccessibleContext::getAccessibleRelationSet (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    // by default there are no relations
    Reference< XAccessibleRelationSet> xRet;
    return xRet;
}

Reference<XAccessibleStateSet> SAL_CALL
    SwAccessibleContext::getAccessibleStateSet (void)
        throw (::com::sun::star::uno::RuntimeException)
{
    ::utl::AccessibleStateSetHelper *pStateSet =
        new ::utl::AccessibleStateSetHelper;
    Reference<XAccessibleStateSet> xRet( pStateSet );

    SetStates( *pStateSet );

    return xRet;
}

Locale SAL_CALL SwAccessibleContext::getLocale (void)
        throw (::drafts::com::sun::star::accessibility::IllegalAccessibleComponentStateException, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    Locale aLoc( Application::GetSettings().GetUILocale() );
    return aLoc;
}

void SAL_CALL SwAccessibleContext::addPropertyChangeListener (
            const Reference< XPropertyChangeListener>& xListener)
        throw (com::sun::star::uno::RuntimeException)
{
    DBG_MSG( "property listener added" );
    aPropChangedListeners.addInterface( xListener );
}

void SAL_CALL SwAccessibleContext::removePropertyChangeListener (
            const Reference< XPropertyChangeListener>& xListener)
        throw (com::sun::star::uno::RuntimeException)
{
    DBG_MSG( "dlistener removed" );
    aPropChangedListeners.removeInterface( xListener );
}

sal_Bool SAL_CALL SwAccessibleContext::contains(
            const ::com::sun::star::awt::Point& aPoint )
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Rectangle aLogBounds( GetBounds( GetFrm() ) ); // twip rel to doc root

    Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to window
    Point aLogPoint( pWin->PixelToLogic( aPixPoint ) ); // twip rel to doc root

    return aLogBounds.IsInside( aLogPoint );
}


Reference< XAccessible > SAL_CALL SwAccessibleContext::getAccessibleAt(
                const awt::Point& aPoint )
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    Reference< XAccessible > xAcc;

    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aPixPoint( aPoint.X, aPoint.Y ); // px rel to window
    Point aLogPoint( pWin->PixelToLogic( aPixPoint ) ); // twip rel to doc root

    const SwFrm *pFrm = GetDescendantAt( aLogPoint );
    if( pFrm )
        xAcc = aAccMap.GetContext( GetVisArea(), pFrm );

    return xAcc;
}


awt::Rectangle SAL_CALL SwAccessibleContext::getBounds()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    const SwFrm *pUpper = GetUpper();
    ASSERT( pUpper, "no upper found" );
    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin && pUpper )

    Rectangle aLogBounds( GetBounds( GetFrm() ) ); // twip rel to doc root
    Point aUpperLogPos( GetBounds( pUpper ).TopLeft() ); // twip rel to doc root

    Rectangle aPixBounds( pWin->LogicToPixel( aLogBounds ) );
    awt::Rectangle aBox( aPixBounds.Left(), aPixBounds.Top(),
                         aPixBounds.GetWidth(), aPixBounds.GetHeight() );

    return aBox;
}


awt::Point SAL_CALL SwAccessibleContext::getLocation()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    const SwFrm *pUpper = GetUpper();
    ASSERT( pUpper, "no upper found" );
    Window *pWin = GetWindow();

    CHECK_FOR_WINDOW( XAccessibleComponent, pWin && pUpper )

    Point aLogPos( GetBounds( GetFrm() ).TopLeft() ); // twip rel to doc root
    Point aUpperLogPos( GetBounds( pUpper ).TopLeft() ); // twip rel to doc root

    Point aPixPos( pWin->LogicToPixel( aLogPos ) );
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Point SAL_CALL SwAccessibleContext::getLocationOnScreen()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Point aLogPos( GetBounds( GetFrm() ).TopLeft() ); // twip rel to doc root
    Point aPixPos( pWin->LogicToPixel( aLogPos ) );
    aPixPos = pWin->OutputToAbsoluteScreenPixel( aPixPos );
    awt::Point aLoc( aPixPos.X(), aPixPos.Y() );

    return aLoc;
}


::com::sun::star::awt::Size SAL_CALL SwAccessibleContext::getSize()
        throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());

    CHECK_FOR_DEFUNC( XAccessibleComponent )

    Window *pWin = GetWindow();
    CHECK_FOR_WINDOW( XAccessibleComponent, pWin )

    Size aLogSize( GetBounds( GetFrm() ).GetSize() ); // twip rel to whatever

    Size aPixSize( pWin->LogicToPixel( aLogSize ) );
    awt::Size aSize( aPixSize.Width(), aPixSize.Height() );

    return aSize;
}


sal_Bool SAL_CALL SwAccessibleContext::isShowing()
        throw (RuntimeException)
{
    return sal_True;
}


sal_Bool SAL_CALL SwAccessibleContext::isVisible()
        throw (RuntimeException)
{
    return sal_True;
}


sal_Bool SAL_CALL SwAccessibleContext::isFocusTraversable()
        throw (RuntimeException)
{
    return sal_False;
}


void SAL_CALL SwAccessibleContext::addFocusListener(
            const Reference<
                ::com::sun::star::awt::XFocusListener >& xListener )
        throw (RuntimeException)
{
    DBG_MSG( "focus listener added" );
    aFocusListeners.addInterface( xListener );
}


void SAL_CALL SwAccessibleContext::removeFocusListener(
            const Reference<
                ::com::sun::star::awt::XFocusListener >& xListener )
        throw (RuntimeException)
{
    DBG_MSG( "focus listener removed" );
    aFocusListeners.removeInterface( xListener );
}

void SAL_CALL SwAccessibleContext::grabFocus()
        throw (RuntimeException)
{
    // impossible
}


Any SAL_CALL SwAccessibleContext::getAccessibleKeyBinding()
        throw (RuntimeException)
{
    // There are no key bindings
    Any aAny;
    return aAny;
}


OUString SAL_CALL SwAccessibleContext::getImplementationName()
        throw( RuntimeException )
{
    ASSERT( !this, "implementation name needs to be overloaded" );

    THROW_RUNTIME_EXCEPTION( XServiceInfo, "implementation name needs to be overloaded" )
}

sal_Bool SAL_CALL
    SwAccessibleContext::supportsService (const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException)
{
    ASSERT( !this, "supports service needs to be overloaded" );
    THROW_RUNTIME_EXCEPTION( XServiceInfo, "supports service needs to be overloaded" )
}

Sequence< OUString > SAL_CALL SwAccessibleContext::getSupportedServiceNames()
        throw( ::com::sun::star::uno::RuntimeException )
{
    ASSERT( !this, "supported services names needs to be overloaded" );
    THROW_RUNTIME_EXCEPTION( XServiceInfo, "supported services needs to be overloaded" )
}

#ifdef DEBUG
void SwAccessibleContext::DbgMsg( const char *pMsg,
                                  const SwAccessibleContext *pAcc )
{
    static SvFileStream aStrm( String::CreateFromAscii("j:\\acc.log"),
                    STREAM_WRITE|STREAM_TRUNC|STREAM_SHARE_DENYNONE );
    ByteString aName( String(sName), RTL_TEXTENCODING_ASCII_US );
    aStrm << aName.GetBuffer();
    aStrm << " (";
    aStrm << ByteString::CreateFromInt32( GetVisArea().Left() ).GetBuffer();
    aStrm << ",";
    aStrm << ByteString::CreateFromInt32( GetVisArea().Top() ).GetBuffer();
    aStrm << ",";
    aStrm << ByteString::CreateFromInt32( GetVisArea().GetWidth() ).GetBuffer();
    aStrm << ",";
    aStrm << ByteString::CreateFromInt32( GetVisArea().GetHeight() ).GetBuffer();
    aStrm << "): ";
    aStrm << pMsg;
    if( pAcc )
    {
        ByteString aChild( String(pAcc->sName),
                              RTL_TEXTENCODING_ASCII_US );
        aStrm << ": ";
        aStrm << aChild.GetBuffer();
    }
    aStrm << "\r\n";
    aStrm.Flush();
}
#endif
