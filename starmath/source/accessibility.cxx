/*************************************************************************
 *
 *  $RCSfile: accessibility.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tl $ $Date: 2002-05-15 13:49:22 $
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
#ifndef _COM_SUN_STAR_AWT_FOCUSEVENT_HPP_
#include <com/sun/star/awt/FocusEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
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
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SVX_UNOEDACC_HXX_
#include <svx/unoedacc.hxx>
#endif

#ifndef _ACCESSIBILITY_HXX_
#include "accessibility.hxx"
#endif
#ifndef APPLICAT_HXX
#include <applicat.hxx>
#endif
#ifndef DOCUMENT_HXX
#include <document.hxx>
#endif
#ifndef VIEW_HXX
#include <view.hxx>
#endif

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace drafts::com::sun::star::accessibility;


//////////////////////////////////////////////////////////////////////

SmAccessibility::SmAccessibility( SmGraphicWindow *pGraphicWin ) :
    pWin    (pGraphicWin),
    aFocusListeners     (aListenerMutex),
    aAccEventListeners  (aListenerMutex)
{
    DBG_ASSERT( pWin, "SmAccessibility: window missing" );
    //++aRefCount;
}


SmAccessibility::SmAccessibility( const SmAccessibility &rSmAcc ) :
    aFocusListeners     (aListenerMutex),
    aAccEventListeners  (aListenerMutex)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
    pWin = rSmAcc.pWin;
    DBG_ASSERT( pWin, "SmAccessibility: window missing" );
    //++aRefCount;
}


SmAccessibility::~SmAccessibility()
{
/*
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (--aRefCount == 0)
    {
    }
*/
}


SmDocShell * SmAccessibility::GetDoc_Impl()
{
    SmViewShell *pView = pWin ? pWin->GetView() : 0;
    return pView ? pView->GetDoc() : 0;
}

void SmAccessibility::ClearWin()
{
    pWin = 0;   // implicitly results in AccessibleStateType::DEFUNC set

    EventObject aEvtObj;
    aEvtObj.Source = (XAccessible *) this;
    aFocusListeners   .disposeAndClear( aEvtObj );
    aAccEventListeners.disposeAndClear( aEvtObj );
}

void SmAccessibility::LaunchFocusEvent( USHORT nGetFocusFlags, BOOL bFocusGained,
        Reference< XAccessible > &rxAccessible )
{
    if (rxAccessible.is())
    {
        awt::FocusEvent  aEvt;
        aEvt.Source = rxAccessible;

        cppu::OInterfaceIteratorHelper aIt( aFocusListeners );
        while (aIt.hasMoreElements())
        {
            Reference< awt::XFocusListener > xRef( aIt.next(), UNO_QUERY );
            if (xRef.is())
            {
                if (bFocusGained)
                    xRef->focusGained( aEvt );
                else
                    xRef->focusLost( aEvt );
            }
        }
    }
}


uno::Reference< XAccessibleContext > SAL_CALL SmAccessibility::getAccessibleContext()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return this;
}

sal_Bool SAL_CALL SmAccessibility::contains( const awt::Point& aPoint )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    Size aSz( pWin->GetSizePixel() );
    return  aPoint.X >= 0  &&  aPoint.Y >= 0  &&
            aPoint.X < aSz.Width()  &&  aPoint.Y < aSz.Height();
}

uno::Reference< XAccessible > SAL_CALL SmAccessibility::getAccessibleAt(
        const awt::Point& aPoint )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    XAccessible *pRes = 0;
    if (contains( aPoint ))
        pRes = this;
    return pRes;
}

awt::Rectangle SAL_CALL SmAccessibility::getBounds()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    Point aPt( pWin->GetPosPixel() );
    Size  aSz( pWin->GetSizePixel() );
    return awt::Rectangle( aPt.X(), aPt.Y(), aSz.Width(), aSz.Height() );
}

awt::Point SAL_CALL SmAccessibility::getLocation()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    Point aPt( pWin->GetPosPixel() );
    return awt::Point( aPt.X(), aPt.Y() );
}

awt::Point SAL_CALL SmAccessibility::getLocationOnScreen()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    Point aPt( pWin->OutputToAbsoluteScreenPixel( Point() ) );
    return awt::Point( aPt.X(), aPt.Y() );
}

awt::Size SAL_CALL SmAccessibility::getSize()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    Size aSz( pWin->GetSizePixel() );
    return awt::Size( aSz.Width(), aSz.Height() );
}

sal_Bool SAL_CALL SmAccessibility::isShowing()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    return pWin->IsVisible();
}

sal_Bool SAL_CALL SmAccessibility::isVisible()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    return pWin->IsReallyVisible();
}

sal_Bool SAL_CALL SmAccessibility::isFocusTraversable()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return TRUE;
}

void SAL_CALL SmAccessibility::addFocusListener(
        const uno::Reference< awt::XFocusListener >& xListener )
    throw (RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
    if (pWin)   // not disposing (about to destroy view shell)
        aFocusListeners.addInterface( xListener );
}

void SAL_CALL SmAccessibility::removeFocusListener(
        const uno::Reference< awt::XFocusListener >& xListener )
    throw (RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
    aFocusListeners.removeInterface( xListener );
}

void SAL_CALL SmAccessibility::grabFocus()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    pWin->GrabFocus();
}

uno::Any SAL_CALL SmAccessibility::getAccessibleKeyBinding()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return uno::Any();
}


sal_Int32 SAL_CALL SmAccessibility::getAccessibleChildCount()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return 0;
}

Reference< XAccessible > SAL_CALL SmAccessibility::getAccessibleChild(
        sal_Int32 i )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    throw IndexOutOfBoundsException();  // there is no child...
    return 0;
}

Reference< XAccessible > SAL_CALL SmAccessibility::getAccessibleParent()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    if (!pWin)
        throw RuntimeException();

    Window *pAccParent = pWin->GetAccessibleParentWindow();
    DBG_ASSERT( pAccParent, "accessible parent missing" );
    return pAccParent ? pAccParent->GetAccessible() : 0;
}

sal_Int32 SAL_CALL SmAccessibility::getAccessibleIndexInParent()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    sal_Int32 nIdx = -1;
    Window *pAccParent = pWin ? pWin->GetAccessibleParentWindow() : 0;
    if (pAccParent)
    {
        USHORT nCnt = pAccParent->GetAccessibleChildWindowCount();
        for (USHORT i = 0;  i < nCnt  &&  nIdx == -1;  ++i)
            if (pAccParent->GetAccessibleChildWindow( i ) == pWin)
                nIdx = i;
    }
    return nIdx;
}

sal_Int16 SAL_CALL SmAccessibility::getAccessibleRole()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return AccessibleRole::DOCUMENT;
}

OUString SAL_CALL SmAccessibility::getAccessibleDescription()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SmDocShell *pDoc = GetDoc_Impl();
    return pDoc ? pDoc->GetText() : OUString();
}

OUString SAL_CALL SmAccessibility::getAccessibleName()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return String( SmResId(RID_DOCUMENTSTR) );
}

Reference< XAccessibleRelationSet > SAL_CALL SmAccessibility::getAccessibleRelationSet()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return 0;   // no relation set
}

Reference< XAccessibleStateSet > SAL_CALL SmAccessibility::getAccessibleStateSet()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    ::utl::AccessibleStateSetHelper *pStateSet =
            new ::utl::AccessibleStateSetHelper;

    Reference<XAccessibleStateSet> xStateSet( pStateSet );

    if (!pWin)
        pStateSet->AddState( AccessibleStateType::DEFUNC );
    else
    {
        //pStateSet->AddState( AccessibleStateType::EDITABLE );
        //pStateSet->AddState( AccessibleStateType::HORIZONTAL );
        //pStateSet->AddState( AccessibleStateType::TRANSIENT );
        pStateSet->AddState( AccessibleStateType::ENABLED );
        pStateSet->AddState( AccessibleStateType::FOCUSABLE );
        if (pWin->HasFocus())
            pStateSet->AddState( AccessibleStateType::FOCUSED );
        if (pWin->IsActive())
            pStateSet->AddState( AccessibleStateType::ACTIVE );
        if (pWin->IsVisible())
            pStateSet->AddState( AccessibleStateType::SHOWING );
        if (pWin->IsReallyVisible())
            pStateSet->AddState( AccessibleStateType::VISIBLE );
        if (COL_TRANSPARENT != pWin->GetBackground().GetColor().GetColor())
            pStateSet->AddState( AccessibleStateType::OPAQUE );
    }

    return xStateSet;
}

Locale SAL_CALL SmAccessibility::getLocale()
    throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    // should be the document language...
    // We use the language of the localized symbol names here.
    return Application::GetSettings().GetUILocale();
}


sal_Int32 SAL_CALL SmAccessibility::getCaretPosition()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return 0;
}

sal_Unicode SAL_CALL SmAccessibility::getCharacter( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return '\0';
}

Sequence< beans::PropertyValue > SAL_CALL SmAccessibility::getCharacterAttributes(
        sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return Sequence< beans::PropertyValue >();
}

awt::Rectangle SAL_CALL SmAccessibility::getCharacterBounds( sal_Int32 nIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return awt::Rectangle();
}

sal_Int32 SAL_CALL SmAccessibility::getCharacterCount()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return 0;
}

sal_Int32 SAL_CALL SmAccessibility::getIndexAtPoint( const awt::Point& aPoint )
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return 0;
}

OUString SAL_CALL SmAccessibility::getSelectedText()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return OUString();
}

sal_Int32 SAL_CALL SmAccessibility::getSelectionStart()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return 0;
}

sal_Int32 SAL_CALL SmAccessibility::getSelectionEnd()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return 0;
}

sal_Bool SAL_CALL SmAccessibility::setSelection(
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return FALSE;
}

OUString SAL_CALL SmAccessibility::getText()
    throw (RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return OUString();
}

OUString SAL_CALL SmAccessibility::getTextRange(
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return OUString();
}

OUString SAL_CALL SmAccessibility::getTextAtIndex(
        sal_Int32 nIndex,
        sal_Int16 aTextType )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return OUString();
}

OUString SAL_CALL SmAccessibility::getTextBeforeIndex(
        sal_Int32 nIndex,
        sal_Int16 aTextType )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return OUString();
}

OUString SAL_CALL SmAccessibility::getTextBehindIndex(
        sal_Int32 nIndex,
        sal_Int16 aTextType )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return OUString();
}

sal_Bool SAL_CALL SmAccessibility::copyText(
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex )
    throw (IndexOutOfBoundsException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return FALSE;
}


void SAL_CALL SmAccessibility::addEventListener(
        const Reference< XAccessibleEventListener >& xListener )
    throw (RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
    if (pWin)   // not disposing (about to destroy view shell)
        aAccEventListeners.addInterface( xListener );
}

void SAL_CALL SmAccessibility::removeEventListener(
        const Reference< XAccessibleEventListener >& xListener )
    throw (RuntimeException)
{
    //vos::OGuard aGuard(Application::GetSolarMutex());
    aAccEventListeners.removeInterface( xListener );
}

