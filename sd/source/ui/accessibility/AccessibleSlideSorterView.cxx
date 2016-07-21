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
    virtual ~Implementation();

    void RequestUpdateChildren();
    void Clear();
    sal_Int32 GetVisibleChildCount() const;
    AccessibleSlideSorterObject* GetAccessibleChild (sal_Int32 nIndex);
    AccessibleSlideSorterObject* GetVisibleChild (sal_Int32 nIndex);

    void ConnectListeners();
    void ReleaseListeners();
    void Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint) override;
    DECL_LINK_TYPED(WindowEventListener, VclWindowEvent&, void);
    DECL_LINK_TYPED(SelectionChangeListener, LinkParamNone*, void);
    DECL_LINK_TYPED(BroadcastSelectionChange, void*, void);
    DECL_LINK_TYPED(FocusChangeListener, LinkParamNone*, void);
    DECL_LINK_TYPED(VisibilityChangeListener, LinkParamNone*, void);
    DECL_LINK_TYPED(UpdateChildrenCallback, void*, void);

    void Activated();
private:
    AccessibleSlideSorterView& mrAccessibleSlideSorter;
    ::sd::slidesorter::SlideSorter& mrSlideSorter;
    typedef ::std::vector<rtl::Reference<AccessibleSlideSorterObject> > PageObjectList;
    PageObjectList maPageObjects;
    sal_Int32 mnFirstVisibleChild;
    sal_Int32 mnLastVisibleChild;
    bool mbListeningToDocument;
    VclPtr<vcl::Window> mpWindow;
    sal_Int32 mnFocusedIndex;
    bool mbModelChangeLocked;
    ImplSVEvent * mnUpdateChildrenUserEventId;
    ImplSVEvent * mnSelectionChangeUserEventId;

    void UpdateChildren();
};

//===== AccessibleSlideSorterView =============================================

AccessibleSlideSorterView::AccessibleSlideSorterView(
    ::sd::slidesorter::SlideSorter& rSlideSorter,
    vcl::Window* pContentWindow)
    : AccessibleSlideSorterViewBase(MutexOwner::maMutex),
      mrSlideSorter(rSlideSorter),
      mnClientId(0),
      mpContentWindow(pContentWindow)
{
}

void AccessibleSlideSorterView::Init()
{
    mpImpl.reset(new Implementation(*this,mrSlideSorter,mpContentWindow));
}

AccessibleSlideSorterView::~AccessibleSlideSorterView()
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

void SAL_CALL AccessibleSlideSorterView::disposing()
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
    AccessibleSlideSorterObject* pResult = nullptr;
    ::osl::MutexGuard aGuard (maMutex);

    if (nIndex>=0 && nIndex<mpImpl->GetVisibleChildCount())
        pResult = mpImpl->GetVisibleChild(nIndex);

    return pResult;
}

void AccessibleSlideSorterView::Destroyed()
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
    AccessibleSlideSorterView::getAccessibleContext()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed ();
    return this;
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL AccessibleSlideSorterView::getAccessibleChildCount()
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

Reference<XAccessible > SAL_CALL AccessibleSlideSorterView::getAccessibleParent()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    Reference<XAccessible> xParent;

    if (mpContentWindow != nullptr)
    {
        vcl::Window* pParent = mpContentWindow->GetAccessibleParentWindow();
        if (pParent != nullptr)
            xParent = pParent->GetAccessible();
    }

    return xParent;
}

sal_Int32 SAL_CALL AccessibleSlideSorterView::getAccessibleIndexInParent()
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

sal_Int16 SAL_CALL AccessibleSlideSorterView::getAccessibleRole()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    static sal_Int16 nRole = AccessibleRole::DOCUMENT;
    return nRole;
}

OUString SAL_CALL AccessibleSlideSorterView::getAccessibleDescription()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    SolarMutexGuard aGuard;

    return SD_RESSTR(SID_SD_A11Y_I_SLIDEVIEW_D);
}

OUString SAL_CALL AccessibleSlideSorterView::getAccessibleName()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    SolarMutexGuard aGuard;

    return SD_RESSTR(SID_SD_A11Y_I_SLIDEVIEW_N);
}

Reference<XAccessibleRelationSet> SAL_CALL
    AccessibleSlideSorterView::getAccessibleRelationSet()
    throw (uno::RuntimeException, std::exception)
{
    return Reference<XAccessibleRelationSet>();
}

Reference<XAccessibleStateSet > SAL_CALL
    AccessibleSlideSorterView::getAccessibleStateSet()
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
    if (mpContentWindow!=nullptr)
    {
        if (mpContentWindow->IsVisible())
            pStateSet->AddState(AccessibleStateType::VISIBLE);
        if (mpContentWindow->IsReallyVisible())
            pStateSet->AddState(AccessibleStateType::SHOWING);
    }

    return pStateSet;
}

lang::Locale SAL_CALL AccessibleSlideSorterView::getLocale()
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
            uno::Reference<uno::XInterface> x (static_cast<lang::XComponent *>(this), uno::UNO_QUERY);
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
    if (pHitDescriptor.get() != nullptr)
        xAccessible = mpImpl->GetAccessibleChild(
            (pHitDescriptor->GetPage()->GetPageNum()-1)/2);

    return xAccessible;
}

awt::Rectangle SAL_CALL AccessibleSlideSorterView::getBounds()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;
    awt::Rectangle aBBox;

    if (mpContentWindow != nullptr)
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

awt::Point SAL_CALL AccessibleSlideSorterView::getLocation()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    awt::Point aLocation;

    if (mpContentWindow != nullptr)
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

awt::Size SAL_CALL AccessibleSlideSorterView::getSize()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    awt::Size aSize;

    if (mpContentWindow != nullptr)
    {
        const Size aOutputSize (mpContentWindow->GetOutputSizePixel());
        aSize.Width = aOutputSize.Width();
        aSize.Height = aOutputSize.Height();
    }

    return aSize;
}

void SAL_CALL AccessibleSlideSorterView::grabFocus()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    if (mpContentWindow)
        mpContentWindow->GrabFocus();
}

sal_Int32 SAL_CALL AccessibleSlideSorterView::getForeground()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    svtools::ColorConfig aColorConfig;
    sal_uInt32 nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL AccessibleSlideSorterView::getBackground()
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
    if (pChild != nullptr)
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
    if (pChild != nullptr)
        bIsSelected = mrSlideSorter.GetController().GetPageSelector().IsPageSelected(
            pChild->GetPageNumber());
    else
        throw lang::IndexOutOfBoundsException();

    return bIsSelected;
}

void SAL_CALL AccessibleSlideSorterView::clearAccessibleSelection()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    mrSlideSorter.GetController().GetPageSelector().DeselectAllPages();
}

void SAL_CALL AccessibleSlideSorterView::selectAllAccessibleChildren()
    throw (uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    const SolarMutexGuard aSolarGuard;

    mrSlideSorter.GetController().GetPageSelector().SelectAllPages();
}

sal_Int32 SAL_CALL AccessibleSlideSorterView::getSelectedAccessibleChildCount()
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
    if (pChild != nullptr)
        mrSlideSorter.GetController().GetPageSelector().DeselectPage(pChild->GetPageNumber());
    else
        throw lang::IndexOutOfBoundsException();
}

// XServiceInfo
OUString SAL_CALL
       AccessibleSlideSorterView::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return OUString("AccessibleSlideSorterView");
}

sal_Bool SAL_CALL AccessibleSlideSorterView::supportsService (const OUString& sServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, sServiceName);
}

uno::Sequence< OUString> SAL_CALL
       AccessibleSlideSorterView::getSupportedServiceNames()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed ();

    return uno::Sequence<OUString> {
            OUString("com.sun.star.accessibility.Accessible"),
            OUString("com.sun.star.accessibility.AccessibleContext"),
            OUString("com.sun.star.drawing.AccessibleSlideSorterView")
    };
}

void AccessibleSlideSorterView::ThrowIfDisposed()
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        OSL_TRACE ("Calling disposed object. Throwing exception:");
        throw lang::DisposedException ("object has been already disposed",
            static_cast<uno::XWeak*>(this));
    }
}

bool AccessibleSlideSorterView::IsDisposed()
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
      mnUpdateChildrenUserEventId(nullptr),
      mnSelectionChangeUserEventId(nullptr)
{
    ConnectListeners();
    UpdateChildren();
}

AccessibleSlideSorterView::Implementation::~Implementation()
{
    if (mnUpdateChildrenUserEventId != nullptr)
        Application::RemoveUserEvent(mnUpdateChildrenUserEventId);
    if (mnSelectionChangeUserEventId != nullptr)
        Application::RemoveUserEvent(mnSelectionChangeUserEventId);
    ReleaseListeners();
    Clear();
}

void AccessibleSlideSorterView::Implementation::RequestUpdateChildren()
{
    if (mnUpdateChildrenUserEventId == nullptr)
        mnUpdateChildrenUserEventId = Application::PostUserEvent(
            LINK(this, AccessibleSlideSorterView::Implementation,
                 UpdateChildrenCallback));
}

void AccessibleSlideSorterView::Implementation::UpdateChildren()
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

void AccessibleSlideSorterView::Implementation::Clear()
{
    PageObjectList::iterator iPageObject;
    PageObjectList::iterator iEnd = maPageObjects.end();
    for (iPageObject=maPageObjects.begin(); iPageObject!=iEnd; ++iPageObject)
        if (*iPageObject != nullptr)
        {
            mrAccessibleSlideSorter.FireAccessibleEvent(
                AccessibleEventId::CHILD,
                Any(Reference<XAccessible>(iPageObject->get())),
                Any());

            Reference<XComponent> xComponent (Reference<XWeak>(iPageObject->get()), UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
            *iPageObject = nullptr;
        }
    maPageObjects.clear();
}

sal_Int32 AccessibleSlideSorterView::Implementation::GetVisibleChildCount() const
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
    AccessibleSlideSorterObject* pChild = nullptr;

    if (nIndex>=0 && (sal_uInt32)nIndex<maPageObjects.size())
    {
        if (maPageObjects[nIndex] == nullptr)
        {
            ::sd::slidesorter::model::SharedPageDescriptor pDescriptor(
                mrSlideSorter.GetModel().GetPageDescriptor(nIndex));
            if (pDescriptor.get() != nullptr)
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

void AccessibleSlideSorterView::Implementation::ConnectListeners()
{
    StartListening (*mrSlideSorter.GetModel().GetDocument());
    if (mrSlideSorter.GetViewShell() != nullptr)
        StartListening (*mrSlideSorter.GetViewShell());
    mbListeningToDocument = true;

    if (mpWindow != nullptr)
        mpWindow->AddEventListener(
            LINK(this,AccessibleSlideSorterView::Implementation,WindowEventListener));

    mrSlideSorter.GetController().GetSelectionManager()->AddSelectionChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,SelectionChangeListener));
    mrSlideSorter.GetController().GetFocusManager().AddFocusChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,FocusChangeListener));
    mrSlideSorter.GetView().AddVisibilityChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,VisibilityChangeListener));
}

void AccessibleSlideSorterView::Implementation::ReleaseListeners()
{
    mrSlideSorter.GetController().GetFocusManager().RemoveFocusChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,FocusChangeListener));
    mrSlideSorter.GetController().GetSelectionManager()->RemoveSelectionChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,SelectionChangeListener));
    mrSlideSorter.GetView().RemoveVisibilityChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,VisibilityChangeListener));

    if (mpWindow != nullptr)
        mpWindow->RemoveEventListener(
            LINK(this,AccessibleSlideSorterView::Implementation,WindowEventListener));

    if (mbListeningToDocument)
    {
        if (mrSlideSorter.GetViewShell() != nullptr)
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

void AccessibleSlideSorterView::SwitchViewActivated()
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

IMPL_LINK_TYPED(AccessibleSlideSorterView::Implementation, WindowEventListener, VclWindowEvent&, rEvent, void)
{
    switch (rEvent.GetId())
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
}

IMPL_LINK_NOARG_TYPED(AccessibleSlideSorterView::Implementation, SelectionChangeListener, LinkParamNone*, void)
{
    if (mnSelectionChangeUserEventId == nullptr)
        mnSelectionChangeUserEventId = Application::PostUserEvent(
            LINK(this, AccessibleSlideSorterView::Implementation, BroadcastSelectionChange));
}

IMPL_LINK_NOARG_TYPED(AccessibleSlideSorterView::Implementation, BroadcastSelectionChange, void*, void)
{
    mnSelectionChangeUserEventId = nullptr;
    mrAccessibleSlideSorter.FireAccessibleEvent(
        AccessibleEventId::SELECTION_CHANGED,
        Any(),
        Any());
}

IMPL_LINK_NOARG_TYPED(AccessibleSlideSorterView::Implementation, FocusChangeListener, LinkParamNone*, void)
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
            if (pObject != nullptr)
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
            if (pObject != nullptr)
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
}

IMPL_LINK_NOARG_TYPED(AccessibleSlideSorterView::Implementation, UpdateChildrenCallback, void*, void)
{
    mnUpdateChildrenUserEventId = nullptr;
    UpdateChildren();
}

IMPL_LINK_NOARG_TYPED(AccessibleSlideSorterView::Implementation, VisibilityChangeListener, LinkParamNone*, void)
{
    UpdateChildren();
}

} // end of namespace ::accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
