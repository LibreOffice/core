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

#include "AccessibleSlideSorterView.hxx"
#include "AccessibleSlideSorterObject.hxx"

#include "SlideSorter.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsFocusManager.hxx"
#include "controller/SlsSelectionManager.hxx"
#include "view/SlideSorterView.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "SlideSorterViewShell.hxx"

#include "ViewShellHint.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"

#include "sdresid.hxx"
#include "accessibility.hrc"
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <rtl/ref.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

/** Inner implementation class of the AccessibleSlideSorterView.

    Note that some event broadcasting is done asynchronously because
    otherwise it could lead to deadlocks on (at least) some Solaris
    machines.  Probably (but unverified) this can happen on all GTK based
    systems.  The asynchronous broadcasting is just a workaround for a
    poorly understood problem.
*/
class AccessibleSlideSorterView::Implementation
    : public SfxListener
{
public:
    Implementation (
        AccessibleSlideSorterView& rAccessibleSlideSorter,
        ::sd::slidesorter::SlideSorter& rSlideSorter,
        vcl::Window* pWindow);
    virtual ~Implementation (void);

    void RequestUpdateChildren (void);
    void Clear (void);
    sal_Int32 GetVisibleChildCount (void) const;
    AccessibleSlideSorterObject* GetAccessibleChild (sal_Int32 nIndex);
    AccessibleSlideSorterObject* GetVisibleChild (sal_Int32 nIndex);

    void ConnectListeners (void);
    void ReleaseListeners (void);
    void Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint) SAL_OVERRIDE;
    DECL_LINK(WindowEventListener, VclWindowEvent*);
    DECL_LINK(SelectionChangeListener, void*);
    DECL_LINK(BroadcastSelectionChange, void*);
    DECL_LINK(FocusChangeListener, void*);
    DECL_LINK(VisibilityChangeListener, void*);
    DECL_LINK(UpdateChildrenCallback, void*);

    void Activated(void);
private:
    AccessibleSlideSorterView& mrAccessibleSlideSorter;
    ::sd::slidesorter::SlideSorter& mrSlideSorter;
    typedef ::std::vector<rtl::Reference<AccessibleSlideSorterObject> > PageObjectList;
    PageObjectList maPageObjects;
    sal_Int32 mnFirstVisibleChild;
    sal_Int32 mnLastVisibleChild;
    bool mbListeningToDocument;
    vcl::Window* mpWindow;
    sal_Int32 mnFocusedIndex;
    bool mbModelChangeLocked;
    ImplSVEvent * mnUpdateChildrenUserEventId;
    ImplSVEvent * mnSelectionChangeUserEventId;

    void UpdateChildren (void);
};

//===== AccessibleSlideSorterView =============================================

AccessibleSlideSorterView::AccessibleSlideSorterView(
    ::sd::slidesorter::SlideSorter& rSlideSorter,
    const Reference<XAccessible>& rxParent,
    vcl::Window* pContentWindow)
    : AccessibleSlideSorterViewBase(MutexOwner::maMutex),
      mrSlideSorter(rSlideSorter),
      mxParent(rxParent),
      mnClientId(0),
      mpContentWindow(pContentWindow)
{
}

void AccessibleSlideSorterView::Init()
{
    mpImpl.reset(new Implementation(*this,mrSlideSorter,mpContentWindow));
}

AccessibleSlideSorterView::~AccessibleSlideSorterView (void)
{
    Destroyed ();
}

void AccessibleSlideSorterView::FireAccessibleEvent (
    short nEventId,
    const uno::Any& rOldValue,
    const uno::Any& rNewValue )
{
    if (mnClientId != 0)
    {
        AccessibleEventObject aEventObject;

        aEventObject.Source = Reference<XWeak>(this);
        aEventObject.EventId = nEventId;
        aEventObject.NewValue = rNewValue;
        aEventObject.OldValue = rOldValue;

        comphelper::AccessibleEventNotifier::addEvent (mnClientId, aEventObject);
    }
}

void SAL_CALL AccessibleSlideSorterView::disposing (void)
{
    if (mnClientId != 0)
    {
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
        mnClientId = 0;
    }
    mpImpl.reset();
}

AccessibleSlideSorterObject* AccessibleSlideSorterView::GetAccessibleChildImplementation (
    sal_Int32 nIndex)
{
    AccessibleSlideSorterObject* pResult = NULL;
    ::osl::MutexGuard aGuard (maMutex);

    if (nIndex>=0 && nIndex<mpImpl->GetVisibleChildCount())
        pResult = mpImpl->GetVisibleChild(nIndex);

    return pResult;
}

void AccessibleSlideSorterView::Destroyed (void)
{
    ::osl::MutexGuard aGuard (maMutex);

    // Send a disposing to all listeners.
    if (mnClientId != 0)
    {
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( mnClientId, *this );
        mnClientId = 0;
    }
}

//=====  XAccessible  =========================================================

Reference<XAccessibleContext > SAL_CALL
    AccessibleSlideSorterView::getAccessibleContext (void)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed ();
    return this;
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL AccessibleSlideSorterView::getAccessibleChildCount (void)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (maMutex);
    return mpImpl->GetVisibleChildCount();
}

Reference<XAccessible > SAL_CALL
    AccessibleSlideSorterView::getAccessibleChild (sal_Int32 nIndex)
    throw (lang::IndexOutOfBoundsException, RuntimeException, std::exception)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (maMutex);

    if (nIndex<0 || nIndex>=mpImpl->GetVisibleChildCount())
        throw lang::IndexOutOfBoundsException();

    return  mpImpl->GetVisibleChild(nIndex);
}

Reference<XAccessible > SAL_CALL AccessibleSlideSorterView::getAccessibleParent (void)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    Reference<XAccessible> xParent;

    if (mpContentWindow != NULL)
    {
        vcl::Window* pParent = mpContentWindow->GetAccessibleParentWindow();
        if (pParent != NULL)
            xParent = pParent->GetAccessible();
    }

    return xParent;
}

sal_Int32 SAL_CALL AccessibleSlideSorterView::getAccessibleIndexInParent (void)
    throw (uno::RuntimeException, std::exception)
{
    OSL_ASSERT(getAccessibleParent().is());
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    sal_Int32 nIndexInParent(-1);

    Reference<XAccessibleContext> xParentContext (getAccessibleParent()->getAccessibleContext());
    if (xParentContext.is())
    {
        sal_Int32 nChildCount (xParentContext->getAccessibleChildCount());
        for (sal_Int32 i=0; i<nChildCount; ++i)
            if (xParentContext->getAccessibleChild(i).get()
                    == static_cast<XAccessible*>(this))
            {
                nIndexInParent = i;
                break;
            }
    }

    return nIndexInParent;
}

sal_Int16 SAL_CALL AccessibleSlideSorterView::getAccessibleRole (void)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    static sal_Int16 nRole = AccessibleRole::DOCUMENT;
    return nRole;
}

OUString SAL_CALL AccessibleSlideSorterView::getAccessibleDescription (void)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    SolarMutexGuard aGuard;

    return SD_RESSTR(SID_SD_A11Y_I_SLIDEVIEW_D);
}

OUString SAL_CALL AccessibleSlideSorterView::getAccessibleName (void)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    SolarMutexGuard aGuard;

    return SD_RESSTR(SID_SD_A11Y_I_SLIDEVIEW_N);
}

Reference<XAccessibleRelationSet> SAL_CALL
    AccessibleSlideSorterView::getAccessibleRelationSet (void)
    throw (uno::RuntimeException, std::exception)
{
    return Reference<XAccessibleRelationSet>();
}

Reference<XAccessibleStateSet > SAL_CALL
    AccessibleSlideSorterView::getAccessibleStateSet (void)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    ::utl::AccessibleStateSetHelper* pStateSet = new ::utl::AccessibleStateSetHelper();

    pStateSet->AddState(AccessibleStateType::FOCUSABLE);
    pStateSet->AddState(AccessibleStateType::SELECTABLE);
    pStateSet->AddState(AccessibleStateType::ENABLED);
    pStateSet->AddState(AccessibleStateType::ACTIVE);
    pStateSet->AddState(AccessibleStateType::MULTI_SELECTABLE);
    pStateSet->AddState(AccessibleStateType::OPAQUE);
    if (mpContentWindow!=NULL)
    {
        if (mpContentWindow->IsVisible())
            pStateSet->AddState(AccessibleStateType::VISIBLE);
        if (mpContentWindow->IsReallyVisible())
            pStateSet->AddState(AccessibleStateType::SHOWING);
    }

    return pStateSet;
}

lang::Locale SAL_CALL AccessibleSlideSorterView::getLocale (void)
    throw (IllegalAccessibleComponentStateException,
        RuntimeException, std::exception)
{
    ThrowIfDisposed ();
    Reference<XAccessibleContext> xParentContext;
    Reference<XAccessible> xParent (getAccessibleParent());
    if (xParent.is())
        xParentContext = xParent->getAccessibleContext();

    if (xParentContext.is())
        return xParentContext->getLocale();
    else
        // Strange, no parent!  Anyway, return the default locale.
        return Application::GetSettings().GetLanguageTag().getLocale();
}

void SAL_CALL AccessibleSlideSorterView::addAccessibleEventListener(
    const Reference<XAccessibleEventListener >& rxListener)
    throw (RuntimeException, std::exception)
{
    if (rxListener.is())
    {
        const osl::MutexGuard aGuard(maMutex);

        if (IsDisposed())
        {
            uno::Reference<uno::XInterface> x ((lang::XComponent *)this, uno::UNO_QUERY);
            rxListener->disposing (lang::EventObject (x));
        }
        else
        {
            if ( ! mnClientId)
                mnClientId = comphelper::AccessibleEventNotifier::registerClient();
            comphelper::AccessibleEventNotifier::addEventListener(mnClientId, rxListener);
        }
    }
}

void SAL_CALL AccessibleSlideSorterView::removeAccessibleEventListener(
    const Reference<XAccessibleEventListener >& rxListener)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (rxListener.is())
    {
        const osl::MutexGuard aGuard(maMutex);

        if (mnClientId != 0)
        {
            sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener(
                mnClientId, rxListener );
            if ( !nListenerCount )
            {
                // no listeners anymore -> revoke ourself. This may lead to
                // the notifier thread dying (if we were the last client),
                // and at least to us not firing any events anymore, in case
                // somebody calls NotifyAccessibleEvent, again
                comphelper::AccessibleEventNotifier::revokeClient( mnClientId );
                mnClientId = 0;
            }
        }
    }
}

//===== XAccessibleComponent ==================================================

sal_Bool SAL_CALL AccessibleSlideSorterView::containsPoint (const awt::Point& aPoint)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const awt::Rectangle aBBox (getBounds());
    return (aPoint.X >= 0)
        && (aPoint.X < aBBox.Width)
        && (aPoint.Y >= 0)
        && (aPoint.Y < aBBox.Height);
}

Reference<XAccessible> SAL_CALL
    AccessibleSlideSorterView::getAccessibleAtPoint (const awt::Point& aPoint)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    Reference<XAccessible> xAccessible;
    const SolarMutexGuard aSolarGuard;

    const Point aTestPoint (aPoint.X, aPoint.Y);
    ::sd::slidesorter::model::SharedPageDescriptor pHitDescriptor (
        mrSlideSorter.GetController().GetPageAt(aTestPoint));
    if (pHitDescriptor.get() != NULL)
        xAccessible = mpImpl->GetAccessibleChild(
            (pHitDescriptor->GetPage()->GetPageNum()-1)/2);

    return xAccessible;
}

awt::Rectangle SAL_CALL AccessibleSlideSorterView::getBounds (void)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    awt::Rectangle aBBox;

    if (mpContentWindow != NULL)
    {
        const Point aPosition (mpContentWindow->GetPosPixel());
        const Size aSize (mpContentWindow->GetOutputSizePixel());

        aBBox.X = aPosition.X();
        aBBox.Y = aPosition.Y();
        aBBox.Width = aSize.Width();
        aBBox.Height = aSize.Height();
    }

    return aBBox;
}

awt::Point SAL_CALL AccessibleSlideSorterView::getLocation (void)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    awt::Point aLocation;

    if (mpContentWindow != NULL)
    {
        const Point aPosition (mpContentWindow->GetPosPixel());
        aLocation.X = aPosition.X();
        aLocation.Y = aPosition.Y();
    }

    return aLocation;
}

/** Calculate the location on screen from the parent's location on screen
    and our own relative location.
*/
awt::Point SAL_CALL AccessibleSlideSorterView::getLocationOnScreen()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    awt::Point aParentLocationOnScreen;

    Reference<XAccessible> xParent (getAccessibleParent());
    if (xParent.is())
    {
        Reference<XAccessibleComponent> xParentComponent (
            xParent->getAccessibleContext(), uno::UNO_QUERY);
        if (xParentComponent.is())
            aParentLocationOnScreen = xParentComponent->getLocationOnScreen();
    }

    awt::Point aLocationOnScreen (getLocation());
    aLocationOnScreen.X += aParentLocationOnScreen.X;
    aLocationOnScreen.Y += aParentLocationOnScreen.Y;

    return aLocationOnScreen;
}

awt::Size SAL_CALL AccessibleSlideSorterView::getSize (void)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    awt::Size aSize;

    if (mpContentWindow != NULL)
    {
        const Size aOutputSize (mpContentWindow->GetOutputSizePixel());
        aSize.Width = aOutputSize.Width();
        aSize.Height = aOutputSize.Height();
    }

    return aSize;
}

void SAL_CALL AccessibleSlideSorterView::grabFocus (void)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if (mpContentWindow)
        mpContentWindow->GrabFocus();
}

sal_Int32 SAL_CALL AccessibleSlideSorterView::getForeground (void)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    svtools::ColorConfig aColorConfig;
    sal_uInt32 nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL AccessibleSlideSorterView::getBackground (void)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    sal_uInt32 nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}

//===== XAccessibleSelection ==================================================

void SAL_CALL AccessibleSlideSorterView::selectAccessibleChild (sal_Int32 nChildIndex)
    throw (lang::IndexOutOfBoundsException,
        RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    AccessibleSlideSorterObject* pChild = mpImpl->GetAccessibleChild(nChildIndex);
    if (pChild != NULL)
        mrSlideSorter.GetController().GetPageSelector().SelectPage(pChild->GetPageNumber());
    else
        throw lang::IndexOutOfBoundsException();
}

sal_Bool SAL_CALL AccessibleSlideSorterView::isAccessibleChildSelected (sal_Int32 nChildIndex)
    throw (lang::IndexOutOfBoundsException,
        RuntimeException, std::exception)
{
    ThrowIfDisposed();
    bool bIsSelected = false;
    const SolarMutexGuard aSolarGuard;

    AccessibleSlideSorterObject* pChild = mpImpl->GetAccessibleChild(nChildIndex);
    if (pChild != NULL)
        bIsSelected = mrSlideSorter.GetController().GetPageSelector().IsPageSelected(
            pChild->GetPageNumber());
    else
        throw lang::IndexOutOfBoundsException();

    return bIsSelected;
}

void SAL_CALL AccessibleSlideSorterView::clearAccessibleSelection (void)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    mrSlideSorter.GetController().GetPageSelector().DeselectAllPages();
}

void SAL_CALL AccessibleSlideSorterView::selectAllAccessibleChildren (void)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    mrSlideSorter.GetController().GetPageSelector().SelectAllPages();
}

sal_Int32 SAL_CALL AccessibleSlideSorterView::getSelectedAccessibleChildCount (void)
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed ();
    const SolarMutexGuard aSolarGuard;
    return mrSlideSorter.GetController().GetPageSelector().GetSelectedPageCount();
}

Reference<XAccessible > SAL_CALL
    AccessibleSlideSorterView::getSelectedAccessibleChild (sal_Int32 nSelectedChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException, std::exception)
{
    ThrowIfDisposed ();
    const SolarMutexGuard aSolarGuard;
    Reference<XAccessible> xChild;

    ::sd::slidesorter::controller::PageSelector& rSelector (
        mrSlideSorter.GetController().GetPageSelector());
    sal_Int32 nPageCount(rSelector.GetPageCount());
    sal_Int32 nSelectedCount = 0;
    for (sal_Int32 i=0; i<nPageCount; i++)
        if (rSelector.IsPageSelected(i))
        {
            if (nSelectedCount == nSelectedChildIndex)
            {
                xChild = mpImpl->GetAccessibleChild(i);
                break;
            }
            ++nSelectedCount;
        }

    if ( ! xChild.is() )
        throw lang::IndexOutOfBoundsException();

    return xChild;
}

void SAL_CALL AccessibleSlideSorterView::deselectAccessibleChild (sal_Int32 nChildIndex)
    throw (lang::IndexOutOfBoundsException,
        RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    AccessibleSlideSorterObject* pChild = mpImpl->GetAccessibleChild(nChildIndex);
    if (pChild != NULL)
        mrSlideSorter.GetController().GetPageSelector().DeselectPage(pChild->GetPageNumber());
    else
        throw lang::IndexOutOfBoundsException();
}

//=====  XServiceInfo  ========================================================
OUString SAL_CALL
       AccessibleSlideSorterView::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return OUString("AccessibleSlideSorterView");
}

sal_Bool SAL_CALL AccessibleSlideSorterView::supportsService (const OUString& sServiceName)
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, sServiceName);
}

uno::Sequence< OUString> SAL_CALL
       AccessibleSlideSorterView::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed ();

    static const OUString sServiceNames[3] = {
            OUString("com.sun.star.accessibility.Accessible"),
            OUString("com.sun.star.accessibility.AccessibleContext"),
            OUString("com.sun.star.drawing.AccessibleSlideSorterView")
    };
    return uno::Sequence<OUString> (sServiceNames, 3);
}

void AccessibleSlideSorterView::ThrowIfDisposed (void)
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        OSL_TRACE ("Calling disposed object. Throwing exception:");
        throw lang::DisposedException ("object has been already disposed",
            static_cast<uno::XWeak*>(this));
    }
}

bool AccessibleSlideSorterView::IsDisposed (void)
{
    return (rBHelper.bDisposed || rBHelper.bInDispose);
}

//===== AccessibleSlideSorterView::Implementation =============================

AccessibleSlideSorterView::Implementation::Implementation (
    AccessibleSlideSorterView& rAccessibleSlideSorter,
    ::sd::slidesorter::SlideSorter& rSlideSorter,
    vcl::Window* pWindow)
    : mrAccessibleSlideSorter(rAccessibleSlideSorter),
      mrSlideSorter(rSlideSorter),
      maPageObjects(),
      mnFirstVisibleChild(0),
      mnLastVisibleChild(-1),
      mbListeningToDocument(false),
      mpWindow(pWindow),
      mnFocusedIndex(-1),
      mbModelChangeLocked(false),
      mnUpdateChildrenUserEventId(0),
      mnSelectionChangeUserEventId(0)
{
    ConnectListeners();
    UpdateChildren();
}

AccessibleSlideSorterView::Implementation::~Implementation (void)
{
    if (mnUpdateChildrenUserEventId != 0)
        Application::RemoveUserEvent(mnUpdateChildrenUserEventId);
    if (mnSelectionChangeUserEventId != 0)
        Application::RemoveUserEvent(mnSelectionChangeUserEventId);
    ReleaseListeners();
    Clear();
}

void AccessibleSlideSorterView::Implementation::RequestUpdateChildren (void)
{
    if (mnUpdateChildrenUserEventId == 0)
        mnUpdateChildrenUserEventId = Application::PostUserEvent(
            LINK(this, AccessibleSlideSorterView::Implementation,
            UpdateChildrenCallback));
}

void AccessibleSlideSorterView::Implementation::UpdateChildren (void)
{
      //By default, all children should be accessible. So here workaround is to make all children visible.
      // MT: THis was in UpdateVisibility, which has some similarity, and hg merge automatically has put it here. Correct?!
      // In the IA2 CWS, also setting mnFirst/LastVisibleChild was commented out!
      mnLastVisibleChild = maPageObjects.size();

    if (mbModelChangeLocked)
    {
        // Do nothing right now.  When the flag is reset, this method is
        // called again.
        return;
    }

    const Pair aRange (mrSlideSorter.GetView().GetVisiblePageRange());
    mnFirstVisibleChild = aRange.A();
    mnLastVisibleChild = aRange.B();

    // Release all children.
    Clear();

    // Create new children for the modified visible range.
    maPageObjects.resize(mrSlideSorter.GetModel().GetPageCount());

    // No Visible children
    if (mnFirstVisibleChild == -1 && mnLastVisibleChild == -1)
        return;

    for (sal_Int32 nIndex(mnFirstVisibleChild); nIndex<=mnLastVisibleChild; ++nIndex)
        GetAccessibleChild(nIndex);
}

void AccessibleSlideSorterView::Implementation::Clear (void)
{
    PageObjectList::iterator iPageObject;
    PageObjectList::iterator iEnd = maPageObjects.end();
    for (iPageObject=maPageObjects.begin(); iPageObject!=iEnd; ++iPageObject)
        if (*iPageObject != NULL)
        {
            mrAccessibleSlideSorter.FireAccessibleEvent(
                AccessibleEventId::CHILD,
                Any(Reference<XAccessible>(iPageObject->get())),
                Any());

            Reference<XComponent> xComponent (Reference<XWeak>(iPageObject->get()), UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
            *iPageObject = NULL;
        }
    maPageObjects.clear();
}

sal_Int32 AccessibleSlideSorterView::Implementation::GetVisibleChildCount (void) const
{
    if (mnFirstVisibleChild<=mnLastVisibleChild && mnFirstVisibleChild>=0)
        return mnLastVisibleChild - mnFirstVisibleChild + 1;
    else
        return 0;
}

AccessibleSlideSorterObject* AccessibleSlideSorterView::Implementation::GetVisibleChild (
    sal_Int32 nIndex)
{
    assert(nIndex>=0 && nIndex<GetVisibleChildCount());

    return GetAccessibleChild(nIndex+mnFirstVisibleChild);
}

AccessibleSlideSorterObject* AccessibleSlideSorterView::Implementation::GetAccessibleChild (
    sal_Int32 nIndex)
{
    AccessibleSlideSorterObject* pChild = NULL;

    if (nIndex>=0 && (sal_uInt32)nIndex<maPageObjects.size())
    {
        if (maPageObjects[nIndex] == NULL)
        {
            ::sd::slidesorter::model::SharedPageDescriptor pDescriptor(
                mrSlideSorter.GetModel().GetPageDescriptor(nIndex));
            if (pDescriptor.get() != NULL)
            {
                maPageObjects[nIndex] = new AccessibleSlideSorterObject(
                    &mrAccessibleSlideSorter,
                    mrSlideSorter,
                    (pDescriptor->GetPage()->GetPageNum()-1)/2);

                mrAccessibleSlideSorter.FireAccessibleEvent(
                    AccessibleEventId::CHILD,
                    Any(),
                    Any(Reference<XAccessible>(maPageObjects[nIndex].get())));
            }

        }

        pChild = maPageObjects[nIndex].get();
    }
    else
    {
        OSL_ASSERT(nIndex>=0 && (sal_uInt32)nIndex<maPageObjects.size());
    }

    return pChild;
}

void AccessibleSlideSorterView::Implementation::ConnectListeners (void)
{
    StartListening (*mrSlideSorter.GetModel().GetDocument());
    if (mrSlideSorter.GetViewShell() != NULL)
        StartListening (*mrSlideSorter.GetViewShell());
    mbListeningToDocument = true;

    if (mpWindow != NULL)
        mpWindow->AddEventListener(
            LINK(this,AccessibleSlideSorterView::Implementation,WindowEventListener));

    mrSlideSorter.GetController().GetSelectionManager()->AddSelectionChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,SelectionChangeListener));
    mrSlideSorter.GetController().GetFocusManager().AddFocusChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,FocusChangeListener));
    mrSlideSorter.GetView().AddVisibilityChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,VisibilityChangeListener));
}

void AccessibleSlideSorterView::Implementation::ReleaseListeners (void)
{
    mrSlideSorter.GetController().GetFocusManager().RemoveFocusChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,FocusChangeListener));
    mrSlideSorter.GetController().GetSelectionManager()->RemoveSelectionChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,SelectionChangeListener));
    mrSlideSorter.GetView().RemoveVisibilityChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,VisibilityChangeListener));

    if (mpWindow != NULL)
        mpWindow->RemoveEventListener(
            LINK(this,AccessibleSlideSorterView::Implementation,WindowEventListener));

    if (mbListeningToDocument)
    {
        if (mrSlideSorter.GetViewShell() != NULL)
            StartListening(*mrSlideSorter.GetViewShell());
        EndListening (*mrSlideSorter.GetModel().GetDocument());
        mbListeningToDocument = false;
    }
}

void AccessibleSlideSorterView::Implementation::Notify (
    SfxBroadcaster&,
    const SfxHint& rHint)
{
    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint);
    if (pSdrHint)
    {
        switch (pSdrHint->GetKind())
        {
            case HINT_PAGEORDERCHG:
                RequestUpdateChildren();
                break;
            default:
                break;
        }
    }
    else if (dynamic_cast<const sd::ViewShellHint*>(&rHint))
    {
        const sd::ViewShellHint& rViewShellHint = static_cast<const sd::ViewShellHint&>(rHint);
        switch (rViewShellHint.GetHintId())
        {
            case sd::ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_START:
                mbModelChangeLocked = true;
                break;

            case sd::ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_END:
                mbModelChangeLocked = false;
                RequestUpdateChildren();
                break;
            default:
                break;
        }
    }
}

void AccessibleSlideSorterView::SwitchViewActivated (void)
{
    // Firstly, set focus to view
    this->FireAccessibleEvent(AccessibleEventId::STATE_CHANGED,
                    Any(),
                    Any(AccessibleStateType::FOCUSED));

    mpImpl->Activated();
}

void AccessibleSlideSorterView::Implementation::Activated()
{
    mrSlideSorter.GetController().GetFocusManager().ShowFocus();

}

IMPL_LINK(AccessibleSlideSorterView::Implementation, WindowEventListener, VclWindowEvent*, pEvent)
{
    switch (pEvent->GetId())
    {
        case VCLEVENT_WINDOW_MOVE:
        case VCLEVENT_WINDOW_RESIZE:
            RequestUpdateChildren();
            break;

        case VCLEVENT_WINDOW_GETFOCUS:
        case VCLEVENT_WINDOW_LOSEFOCUS:
            mrAccessibleSlideSorter.FireAccessibleEvent(
                AccessibleEventId::SELECTION_CHANGED,
                Any(),
                Any());
            break;
        default:
            break;
    }
    return 1;
}

IMPL_LINK_NOARG(AccessibleSlideSorterView::Implementation, SelectionChangeListener)
{
    if (mnSelectionChangeUserEventId == 0)
        mnSelectionChangeUserEventId = Application::PostUserEvent(
            LINK(this, AccessibleSlideSorterView::Implementation, BroadcastSelectionChange));
    return 1;
}

IMPL_LINK_NOARG(AccessibleSlideSorterView::Implementation, BroadcastSelectionChange)
{
    mnSelectionChangeUserEventId = 0;
    mrAccessibleSlideSorter.FireAccessibleEvent(
        AccessibleEventId::SELECTION_CHANGED,
        Any(),
        Any());
    return 1;
}

IMPL_LINK_NOARG(AccessibleSlideSorterView::Implementation, FocusChangeListener)
{
    sal_Int32 nNewFocusedIndex (
        mrSlideSorter.GetController().GetFocusManager().GetFocusedPageIndex());

    bool bHasFocus = mrSlideSorter.GetController().GetFocusManager().IsFocusShowing();
    if (!bHasFocus)
        nNewFocusedIndex = -1;

    // add a checker whether the focus event is sent out. Only after sent, the mnFocusedIndex should be updated.
    bool bSentFocus = false;
    if (nNewFocusedIndex != mnFocusedIndex)
    {
        if (mnFocusedIndex >= 0)
        {
            AccessibleSlideSorterObject* pObject = GetAccessibleChild(mnFocusedIndex);
            if (pObject != NULL)
            {
                pObject->FireAccessibleEvent(
                    AccessibleEventId::STATE_CHANGED,
                    Any(AccessibleStateType::FOCUSED),
                    Any());
                bSentFocus = true;
            }
        }
        if (nNewFocusedIndex >= 0)
        {
            AccessibleSlideSorterObject* pObject = GetAccessibleChild(nNewFocusedIndex);
            if (pObject != NULL)
            {
                pObject->FireAccessibleEvent(
                    AccessibleEventId::STATE_CHANGED,
                    Any(),
                    Any(AccessibleStateType::FOCUSED));
                bSentFocus = true;
            }
        }
        if (bSentFocus)
            mnFocusedIndex = nNewFocusedIndex;
    }
    return 1;
}

IMPL_LINK_NOARG(AccessibleSlideSorterView::Implementation, UpdateChildrenCallback)
{
    mnUpdateChildrenUserEventId = 0;
    UpdateChildren();

    return 1;
}

IMPL_LINK_NOARG(AccessibleSlideSorterView::Implementation, VisibilityChangeListener)
{
    UpdateChildren();
    return 1;
}

} // end of namespace ::accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
