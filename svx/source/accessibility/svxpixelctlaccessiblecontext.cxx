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

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <osl/mutex.hxx>
#include <rtl/uuid.h>
#include <tools/debug.hxx>
#include <tools/gen.hxx>

#include <svx/dialogs.hrc>
#include "accessibility.hrc"
#include <svx/dlgctrl.hxx>

#include <svx/dialmgr.hxx>
#include <comphelper/accessibleeventnotifier.hxx>

#include <unotools/accessiblerelationsethelper.hxx>

#include "svxpixelctlaccessiblecontext.hxx"
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

SvxPixelCtlAccessible::SvxPixelCtlAccessible( SvxPixelCtl& rControl) :
    SvxPixelCtlAccessible_BASE(m_aMutex),
    mrPixelCtl(rControl),
    mnClientId(0)
{
}

SvxPixelCtlAccessible::~SvxPixelCtlAccessible()
{
    if( IsAlive() )
    {
        osl_atomic_increment( &m_refCount );
        dispose();      // set mpRepr = NULL & release all children
    }
}
uno::Reference< XAccessibleContext > SvxPixelCtlAccessible::getAccessibleContext(  )
    throw (uno::RuntimeException, std::exception)
{
    return this;
}

sal_Int32 SvxPixelCtlAccessible::getAccessibleChildCount(  ) throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return mrPixelCtl.GetSquares();
}
uno::Reference< XAccessible > SvxPixelCtlAccessible::getAccessibleChild( sal_Int32 i )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    if ( i < 0 || i >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();
    uno::Reference <XAccessible> xAcc;
    return CreateChild(i, mrPixelCtl.IndexToPoint(i));
}

uno::Reference< XAccessible > SvxPixelCtlAccessible::getAccessibleParent(  )
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    vcl::Window *pTabPage = getNonLayoutParent(&mrPixelCtl);
    if (!pTabPage || WINDOW_TABPAGE != pTabPage->GetType())
        return uno::Reference< XAccessible >();
    else
        return pTabPage->GetAccessible();
}

sal_Int32 SvxPixelCtlAccessible::getAccessibleIndexInParent(  )
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    sal_uInt16 nIdx = 0;
    vcl::Window *pTabPage = getNonLayoutParent(&mrPixelCtl);
    if (!pTabPage || WINDOW_TABPAGE != pTabPage->GetType())
        return -1;
    sal_uInt16 nChildren = pTabPage->GetChildCount();
    for(nIdx = 0; nIdx < nChildren; nIdx++)
        if(pTabPage->GetChild( nIdx ) == &mrPixelCtl)
            break;
    return nIdx;
}

sal_Int16 SvxPixelCtlAccessible::getAccessibleRole(  ) throw (uno::RuntimeException, std::exception)
{
    return AccessibleRole::LIST;
}

OUString SvxPixelCtlAccessible::getAccessibleDescription(  )
    throw (uno::RuntimeException, std::exception)
{

    ::osl::MutexGuard   aGuard( m_aMutex );
    return mrPixelCtl.GetAccessibleDescription();
}

OUString SvxPixelCtlAccessible::getAccessibleName(  )
    throw (uno::RuntimeException, std::exception)
{

    ::osl::MutexGuard   aGuard( m_aMutex );
    return mrPixelCtl.GetAccessibleName();
}

uno::Reference< XAccessibleRelationSet > SvxPixelCtlAccessible::getAccessibleRelationSet(  )
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    utl::AccessibleRelationSetHelper* rRelationSet = new utl::AccessibleRelationSetHelper;
    uno::Reference< css::accessibility::XAccessibleRelationSet > rSet = rRelationSet;
    vcl::Window *pLabeledBy = mrPixelCtl.GetAccessibleRelationLabeledBy();
    if ( pLabeledBy && pLabeledBy != &mrPixelCtl )
    {
        uno::Sequence< uno::Reference< uno::XInterface > > aSequence { pLabeledBy->GetAccessible() };
        rRelationSet->AddRelation( css::accessibility::AccessibleRelation( css::accessibility::AccessibleRelationType::LABELED_BY, aSequence ) );
    }

    vcl::Window* pMemberOf = mrPixelCtl.GetAccessibleRelationMemberOf();
    if ( pMemberOf && pMemberOf != &mrPixelCtl )
    {
        uno::Sequence< uno::Reference< uno::XInterface > > aSequence { pMemberOf->GetAccessible() };
        rRelationSet->AddRelation( css::accessibility::AccessibleRelation( css::accessibility::AccessibleRelationType::MEMBER_OF, aSequence ) );
    }
    return rSet;
}


uno::Reference< XAccessibleStateSet > SvxPixelCtlAccessible::getAccessibleStateSet(  )
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    uno::Reference< XAccessibleStateSet > xRet = pStateSetHelper;

    const sal_Int16 aStandardStates[] =
    {
        AccessibleStateType::FOCUSABLE,
        AccessibleStateType::SELECTABLE,
        AccessibleStateType::SHOWING,
        AccessibleStateType::VISIBLE,
        AccessibleStateType::OPAQUE,
        0
    };

    sal_Int16 nState = 0;
    while(aStandardStates[nState])
    {
        pStateSetHelper->AddState(aStandardStates[nState++]);
    }
    if(mrPixelCtl.IsEnabled())
        pStateSetHelper->AddState(AccessibleStateType::ENABLED);
    if(mrPixelCtl.HasFocus())
        pStateSetHelper->AddState(AccessibleStateType::FOCUSED);
    pStateSetHelper->AddState(AccessibleStateType::MANAGES_DESCENDANTS);

    return xRet;
}


css::lang::Locale SvxPixelCtlAccessible::getLocale(  )
    throw (IllegalAccessibleComponentStateException, uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    if( getAccessibleParent().is() )
    {
        uno::Reference< XAccessibleContext >        xParentContext( getAccessibleParent()->getAccessibleContext() );
        if( xParentContext.is() )
            return xParentContext->getLocale();
    }

    //  No locale and no parent.  Therefore throw exception to indicate this
    //  cluelessness.
    throw IllegalAccessibleComponentStateException();
}


sal_Bool SvxPixelCtlAccessible::containsPoint( const awt::Point& aPt )
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    Point aPoint(aPt.X, aPt.Y);
    return (aPoint.X() >= 0)
        && (aPoint.X() < mrPixelCtl.GetSizePixel().getWidth())
        && (aPoint.Y() >= 0)
        && (aPoint.Y() < mrPixelCtl.GetSizePixel().getHeight());
}
uno::Reference<XAccessible > SAL_CALL SvxPixelCtlAccessible::getAccessibleAtPoint (
        const awt::Point& aPoint)
    throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    ensureIsAlive();
    uno::Reference <XAccessible> xAcc;

    Point childPoint;
    childPoint.X() = aPoint.X;
    childPoint.Y() = aPoint.Y;

    Point pt= mrPixelCtl.PixelToLogic(childPoint);
    long nIndex = mrPixelCtl.PointToIndex(pt);
    return CreateChild(nIndex, mrPixelCtl.IndexToPoint(nIndex));
}

awt::Rectangle SvxPixelCtlAccessible::getBounds(  ) throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    awt::Rectangle aRet;
    Size aSz = mrPixelCtl.GetSizePixel();
    Point aPos = mrPixelCtl.GetPosPixel();
    aRet.X = aPos.X();
    aRet.Y = aPos.Y();
    aRet.Width = aSz.Width();
    aRet.Height = aSz.Height();
    return aRet;
}

awt::Point SvxPixelCtlAccessible::getLocation(  ) throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    Point aPos;
    aPos = mrPixelCtl.GetPosPixel();
    awt::Point aRet(aPos.X(), aPos.Y());
    return aRet;
}

awt::Point SvxPixelCtlAccessible::getLocationOnScreen(  ) throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    Rectangle rect = mrPixelCtl.GetWindowExtentsRelative(nullptr);
    return awt::Point(rect.Left(),rect.Top() );
}

awt::Size SvxPixelCtlAccessible::getSize(  ) throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    Size aSz = mrPixelCtl.GetSizePixel();
    return awt::Size(aSz.Width(),aSz.Height());
}
void SvxPixelCtlAccessible::grabFocus(  ) throw (uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    mrPixelCtl.GrabFocus();
}

sal_Int32 SvxPixelCtlAccessible::getForeground(  )
        throw (css::uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return mrPixelCtl.GetControlForeground().GetColor();
}

sal_Int32 SvxPixelCtlAccessible::getBackground(  )
        throw (css::uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return mrPixelCtl.GetControlBackground().GetColor();
}

OUString SvxPixelCtlAccessible::getImplementationName(  ) throw (uno::RuntimeException, std::exception)
{
    return OUString("SvxPixelCtlAccessible");
}

sal_Bool SvxPixelCtlAccessible::supportsService( const OUString& rServiceName )
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}

uno::Sequence< OUString > SvxPixelCtlAccessible::getSupportedServiceNames(  )
    throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "Accessible";
    pArray[1] = "AccessibleContext";
    pArray[2] = "AccessibleComponent";
    return aRet;
}


// XAccessibleSelection

void SAL_CALL SvxPixelCtlAccessible::selectAccessibleChild( sal_Int32 nChildIndex ) throw (lang::IndexOutOfBoundsException, RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    if ( nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();

    long nIndex = mrPixelCtl.ShowPosition(mrPixelCtl.IndexToPoint(nChildIndex));
    NotifyChild(nIndex,true,false);
}

sal_Bool SAL_CALL SvxPixelCtlAccessible::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (lang::IndexOutOfBoundsException, RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    return mrPixelCtl.GetFocusPosIndex() == nChildIndex;
}

void SAL_CALL SvxPixelCtlAccessible::clearAccessibleSelection(  ) throw (RuntimeException, std::exception)
{
}

void SAL_CALL SvxPixelCtlAccessible::selectAllAccessibleChildren(  ) throw (RuntimeException, std::exception)
{
}

sal_Int32 SAL_CALL SvxPixelCtlAccessible::getSelectedAccessibleChildCount(  ) throw (RuntimeException, std::exception)
{
    return 1;
}

uno::Reference< XAccessible > SAL_CALL SvxPixelCtlAccessible::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (lang::IndexOutOfBoundsException, RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    if ( nSelectedChildIndex >= 1)
        throw lang::IndexOutOfBoundsException();

    uno::Reference< XAccessible > xChild;
    if(m_xCurChild.is())
    {
        xChild = m_xCurChild;
    }
    return xChild;
}

void SAL_CALL SvxPixelCtlAccessible::deselectAccessibleChild( sal_Int32 ) throw (lang::IndexOutOfBoundsException, RuntimeException, std::exception)
{
}

// Added by lq
void SvxPixelCtlAccessible::ensureIsAlive() const
    throw ( lang::DisposedException )
{
    if( !IsAlive() )
        throw lang::DisposedException();
}

//XAccessibleEventBroadcaster
void SAL_CALL SvxPixelCtlAccessible::addAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )  throw( RuntimeException, std::exception )
{
    if (xListener.is())
    {
        ::osl::MutexGuard   aGuard( m_aMutex );
        if (!mnClientId)
                mnClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( mnClientId, xListener );
    }
}

void SAL_CALL SvxPixelCtlAccessible::removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
    throw( RuntimeException, std::exception )
{
    if (xListener.is())
    {
        ::osl::MutexGuard   aGuard( m_aMutex );

        sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( mnClientId, xListener );
        if ( !nListenerCount )
        {
            comphelper::AccessibleEventNotifier::revokeClient( mnClientId );
            mnClientId = 0;
        }
    }
}
void SvxPixelCtlAccessible::CommitChange( const AccessibleEventObject& rEvent )
{
    if (mnClientId)
        comphelper::AccessibleEventNotifier::addEvent( mnClientId, rEvent );
}

//Solution:Add the event handling method
void SvxPixelCtlAccessible::FireAccessibleEvent (short nEventId, const css::uno::Any& rOld, const css::uno::Any& rNew)
{
    const uno::Reference< XInterface >  xSource( *this );
    CommitChange( AccessibleEventObject( xSource, nEventId, rNew,rOld ) );
}

void SAL_CALL SvxPixelCtlAccessible::disposing()
{
    if( !rBHelper.bDisposed )
    {
        {
            ::osl::MutexGuard   aGuard( m_aMutex );
            if ( mnClientId )
            {
                comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
                mnClientId =  0;
            }
        }
        //mxParent.clear();

    }
}

void SvxPixelCtlAccessible::NotifyChild(long nIndex,bool bSelect ,bool bCheck)
{
    DBG_ASSERT( !(!bSelect && !bCheck),"" );//non is false

    SvxPixelCtlAccessibleChild *pChild= nullptr;

    if (m_xCurChild.is())
    {
        pChild= static_cast<SvxPixelCtlAccessibleChild*>(m_xCurChild.get());
        DBG_ASSERT(pChild,"Child Must be Valid");
        if (pChild->getAccessibleIndexInParent() == nIndex )
        {
            if (bSelect)
            {
                pChild->SelectChild(true);
            }
            if (bCheck)
            {
                pChild->ChangePixelColorOrBG( mrPixelCtl.GetBitmapPixel(sal_uInt16(nIndex)) != 0);
                pChild->CheckChild();
            }
            return ;
        }
    }
    uno::Reference <XAccessible> xNewChild =CreateChild(nIndex, mrPixelCtl.IndexToPoint(nIndex));
    SvxPixelCtlAccessibleChild *pNewChild= static_cast<SvxPixelCtlAccessibleChild*>(xNewChild.get());
    DBG_ASSERT(pNewChild,"Child Must be Valid");

    Any aNewValue,aOldValue;
    aNewValue<<= xNewChild;
    FireAccessibleEvent(    AccessibleEventId::ACTIVE_DESCENDANT_CHANGED,
                            aOldValue,
                            aNewValue );

    if (bSelect)
    {
        if (pChild)
        {
            pChild->SelectChild(false);
        }
        pNewChild->SelectChild(true);
    }
    if (bCheck)
    {
        pNewChild->CheckChild();
    }
    m_xCurChild= xNewChild;


}

uno::Reference<XAccessible> SvxPixelCtlAccessible::CreateChild (long nIndex,Point mPoint)
{
    bool bPixelColorOrBG = mrPixelCtl.GetBitmapPixel(sal_uInt16(nIndex)) != 0;
    Size size(mrPixelCtl.GetWidth() / mrPixelCtl.GetLineCount(),mrPixelCtl.GetHeight() / mrPixelCtl.GetLineCount());
    uno::Reference<XAccessible> xChild;
    xChild = new SvxPixelCtlAccessibleChild(mrPixelCtl,
                bPixelColorOrBG,
                Rectangle(mPoint,size),
                this,
                nIndex);

    return xChild;
}


void SvxPixelCtlAccessible::LoseFocus()
{
    m_xCurChild.clear();
}

void SvxPixelCtlAccessibleChild::CheckChild()
{
    Any aChecked;
    aChecked <<= AccessibleStateType::CHECKED;

    if (m_bPixelColorOrBG)//Current Child State
    {
        FireAccessibleEvent(    AccessibleEventId::STATE_CHANGED,
                                Any(),
                                aChecked);
    }
    else
    {
        FireAccessibleEvent(    AccessibleEventId::STATE_CHANGED,
                                aChecked,
                                Any() );
    }
}

void SvxPixelCtlAccessibleChild::SelectChild( bool bSelect)
{
    Any aSelected;
    aSelected <<= AccessibleStateType::SELECTED;

    if (bSelect)
    {
        FireAccessibleEvent(    AccessibleEventId::STATE_CHANGED,
                                Any(),
                                aSelected);
    }
    else
    {
        FireAccessibleEvent(    AccessibleEventId::STATE_CHANGED,
                                aSelected,
                                Any());
    }
}
void SvxPixelCtlAccessibleChild::FireAccessibleEvent (
    short nEventId,
    const css::uno::Any& rOld,
    const css::uno::Any& rNew)
{
    const uno::Reference< XInterface >  xSource( *this );
    CommitChange( AccessibleEventObject( xSource, nEventId, rNew,rOld ) );
}

SvxPixelCtlAccessibleChild::SvxPixelCtlAccessibleChild(
    SvxPixelCtl& rWindow,
    bool bPixelColorOrBG,
    const Rectangle& rBoundingBox,
    const uno::Reference<XAccessible>&  rxParent,
    long nIndexInParent ) :
    SvxPixelCtlAccessibleChild_BASE( m_aMutex ),
    mrParentWindow( rWindow ),
    mxParent(rxParent),
    m_bPixelColorOrBG(bPixelColorOrBG),
    mpBoundingBox( new Rectangle( rBoundingBox ) ),
    mnIndexInParent( nIndexInParent ),
    mnClientId( 0 )
{
}


SvxPixelCtlAccessibleChild::~SvxPixelCtlAccessibleChild()
{
    if( IsAlive() )
    {
        osl_atomic_increment( &m_refCount );
        dispose();      // set mpRepr = NULL & release all children
    }
}

// XAccessible
uno::Reference< XAccessibleContext> SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleContext() throw( RuntimeException, std::exception )
{
    return this;
}

// XAccessibleComponent
sal_Bool SAL_CALL SvxPixelCtlAccessibleChild::containsPoint( const awt::Point& rPoint ) throw( RuntimeException, std::exception )
{
    // no guard -> done in getBounds()
//  return GetBoundingBox().IsInside( VCLPoint( rPoint ) );
    return Rectangle( Point( 0, 0 ), GetBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
}

uno::Reference< XAccessible > SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleAtPoint( const awt::Point& ) throw( RuntimeException, std::exception )
{
    return uno::Reference< XAccessible >();
}

awt::Rectangle SAL_CALL SvxPixelCtlAccessibleChild::getBounds() throw( RuntimeException, std::exception )
{
    // no guard -> done in getBoundingBox()
    //Modified by lq, 09/26
    //return AWTRectangle( GetBoundingBox() );
    awt::Rectangle rect = AWTRectangle( GetBoundingBox() );
    rect.X = rect.X + mrParentWindow.GetClientWindowExtentsRelative().Left()-mrParentWindow.GetWindowExtentsRelative(nullptr).Left();
    rect.Y = rect.Y + mrParentWindow.GetClientWindowExtentsRelative().Top()-mrParentWindow.GetWindowExtentsRelative(nullptr).Top();
    return rect;
    // End
}

awt::Point SAL_CALL SvxPixelCtlAccessibleChild::getLocation() throw( RuntimeException, std::exception )
{
    // no guard -> done in getBoundingBox()
    return AWTPoint( GetBoundingBox().TopLeft() );
}

awt::Point SAL_CALL SvxPixelCtlAccessibleChild::getLocationOnScreen() throw( RuntimeException, std::exception )
{
    // no guard -> done in getBoundingBoxOnScreen()
    return AWTPoint( GetBoundingBoxOnScreen().TopLeft() );
}

awt::Size SAL_CALL SvxPixelCtlAccessibleChild::getSize() throw( RuntimeException, std::exception )
{
    // no guard -> done in getBoundingBox()
    return AWTSize( GetBoundingBox().GetSize() );
}

void SAL_CALL SvxPixelCtlAccessibleChild::grabFocus() throw( RuntimeException, std::exception )
{
}

sal_Int32 SvxPixelCtlAccessibleChild::getForeground(  )
        throw (css::uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    ThrowExceptionIfNotAlive();
    return mrParentWindow.GetControlForeground().GetColor();
}
sal_Int32 SvxPixelCtlAccessibleChild::getBackground(  )
        throw (css::uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    ThrowExceptionIfNotAlive();
    return mrParentWindow.GetControlBackground().GetColor();
}

// XAccessibleContext
sal_Int32 SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleChildCount() throw( RuntimeException, std::exception )
{
    return 0;
}

uno::Reference< XAccessible > SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleChild( sal_Int32 )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    throw lang::IndexOutOfBoundsException();
}

uno::Reference< XAccessible > SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleParent() throw( RuntimeException, std::exception )
{
    return mxParent;
}

sal_Int32 SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleIndexInParent() throw( RuntimeException, std::exception )
{
   return mnIndexInParent;
}

sal_Int16 SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleRole() throw( RuntimeException, std::exception )
{
    return AccessibleRole::CHECK_BOX;
}

OUString SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleDescription() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    return  GetName();
}

OUString SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleName() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard   aGuard( m_aMutex );
    return  GetName();
}

/** Return empty uno::Reference to indicate that the relation set is not
    supported.
*/
uno::Reference<XAccessibleRelationSet> SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleRelationSet() throw( RuntimeException, std::exception )
{
    return uno::Reference< XAccessibleRelationSet >();
}

uno::Reference< XAccessibleStateSet > SAL_CALL SvxPixelCtlAccessibleChild::getAccessibleStateSet() throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard                       aGuard( m_aMutex );
    utl::AccessibleStateSetHelper*          pStateSetHelper = new utl::AccessibleStateSetHelper;

    if( IsAlive() )
    {

        pStateSetHelper->AddState( AccessibleStateType::TRANSIENT );
        pStateSetHelper->AddState( AccessibleStateType::ENABLED );
        pStateSetHelper->AddState( AccessibleStateType::OPAQUE );
        pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );
        pStateSetHelper->AddState( AccessibleStateType::SHOWING );
        pStateSetHelper->AddState( AccessibleStateType::VISIBLE );

        long nIndex = mrParentWindow.GetFocusPosIndex();
        if ( nIndex == mnIndexInParent)
        {
            pStateSetHelper->AddState( AccessibleStateType::SELECTED );
        }
        if (mrParentWindow.GetBitmapPixel(sal_uInt16(mnIndexInParent)))
        {
            pStateSetHelper->AddState( AccessibleStateType::CHECKED );
        }
    }
    else
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

    return pStateSetHelper;
}

lang::Locale SAL_CALL SvxPixelCtlAccessibleChild::getLocale() throw( IllegalAccessibleComponentStateException, RuntimeException, std::exception )
{
    ::osl::MutexGuard                       aGuard( m_aMutex );
    if( mxParent.is() )
    {
        uno::Reference< XAccessibleContext >        xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
            return xParentContext->getLocale();
    }

    //  No locale and no parent.  Therefore throw exception to indicate this
    //  cluelessness.
    throw IllegalAccessibleComponentStateException();
}

void SAL_CALL SvxPixelCtlAccessibleChild::addAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
    throw( RuntimeException, std::exception )
{
    if (xListener.is())
    {
        ::osl::MutexGuard   aGuard( m_aMutex );
        if (!mnClientId)
            mnClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( mnClientId, xListener );
    }
}

void SAL_CALL SvxPixelCtlAccessibleChild::removeAccessibleEventListener( const uno::Reference< XAccessibleEventListener >& xListener )
    throw( RuntimeException, std::exception )
{
    if (xListener.is())
    {
        ::osl::MutexGuard   aGuard( m_aMutex );

        sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( mnClientId, xListener );
        if ( !nListenerCount )
        {
            // no listeners anymore
            // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
            // and at least to us not firing any events anymore, in case somebody calls
            // NotifyAccessibleEvent, again
            comphelper::AccessibleEventNotifier::revokeClient( mnClientId );
            mnClientId = 0;
        }
    }
}

// XServiceInfo
OUString SAL_CALL SvxPixelCtlAccessibleChild::getImplementationName() throw( RuntimeException, std::exception )
{
    return OUString( "SvxPixelCtlAccessibleChild" );
}

sal_Bool SAL_CALL SvxPixelCtlAccessibleChild::supportsService( const OUString& rServiceName ) throw( RuntimeException, std::exception )
{
    return cppu::supportsService( this, rServiceName );
}

Sequence< OUString > SAL_CALL SvxPixelCtlAccessibleChild::getSupportedServiceNames() throw( RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet(3);
    OUString* pArray = aRet.getArray();
    pArray[0] = "Accessible";
    pArray[1] = "AccessibleContext";
    pArray[2] = "AccessibleComponent";
    return aRet;
}

// internal
void SvxPixelCtlAccessibleChild::CommitChange( const AccessibleEventObject& rEvent )
{
    if (mnClientId)
        comphelper::AccessibleEventNotifier::addEvent( mnClientId, rEvent );
}

void SAL_CALL SvxPixelCtlAccessibleChild::disposing()
{
    if( !rBHelper.bDisposed )
    {
        ::osl::MutexGuard   aGuard( m_aMutex );

        // Send a disposing to all listeners.
        if ( mnClientId )
        {
            comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
            mnClientId =  0;
        }

        mxParent.clear();

        delete mpBoundingBox;
    }
}

void SvxPixelCtlAccessibleChild::ThrowExceptionIfNotAlive() throw( lang::DisposedException )
{
    if( IsNotAlive() )
        throw lang::DisposedException();
}

Rectangle SvxPixelCtlAccessibleChild::GetBoundingBoxOnScreen() throw( RuntimeException )
{
    ::osl::MutexGuard   aGuard( m_aMutex );

    // no ThrowExceptionIfNotAlive() because its done in GetBoundingBox()
    Rectangle           aRect( GetBoundingBox() );

    return Rectangle( mrParentWindow.OutputToAbsoluteScreenPixel( aRect.TopLeft() ), aRect.GetSize() );
}

Rectangle const & SvxPixelCtlAccessibleChild::GetBoundingBox() throw( RuntimeException )
{
    // no guard necessary, because no one changes mpBoundingBox after creating it
    ThrowExceptionIfNotAlive();

    return *mpBoundingBox;
}

OUString SvxPixelCtlAccessibleChild::GetName()
{
    sal_Int32 nXIndex = mnIndexInParent % mrParentWindow.GetLineCount();
    sal_Int32 nYIndex = mnIndexInParent / mrParentWindow.GetLineCount();

    OUString str = "("
                 + OUString::number(nXIndex)
                 + ","
                 + OUString::number(nYIndex)
                 + ")";
    return str;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
