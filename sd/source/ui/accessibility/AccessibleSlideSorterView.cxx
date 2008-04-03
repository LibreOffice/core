/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleSlideSorterView.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:24:14 $
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

#include "precompiled_sd.hxx"

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
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef COMPHELPER_ACCESSIBLE_EVENT_NOTIFIER
#include <comphelper/accessibleeventnotifier.hxx>
#endif
#include <unotools/accessiblestatesethelper.hxx>
#include <rtl/ref.hxx>

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

namespace accessibility {


class AccessibleSlideSorterView::Implementation
    : public SfxListener
{
public:
    Implementation (
        AccessibleSlideSorterView& rAccessibleSlideSorter,
        ::sd::slidesorter::SlideSorter& rSlideSorter,
        ::Window* pWindow);
    ~Implementation (void);

    void UpdateVisibility (void);
    void UpdateChildren (void);
    void Clear (void);
    sal_Int32 GetVisibleChildCount (void) const;
    AccessibleSlideSorterObject* GetAccessibleChild (sal_Int32 nIndex);
    AccessibleSlideSorterObject* GetVisibleChild (sal_Int32 nIndex);

    void ConnectListeners (void);
    void ReleaseListeners (void);
    void Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint);
    DECL_LINK(WindowEventListener, VclWindowEvent*);
    DECL_LINK(SelectionChangeListener, void*);
    DECL_LINK(FocusChangeListener, void*);

private:
    AccessibleSlideSorterView& mrAccessibleSlideSorter;
    ::sd::slidesorter::SlideSorter& mrSlideSorter;
    typedef ::std::vector<rtl::Reference<AccessibleSlideSorterObject> > PageObjectList;
    PageObjectList maPageObjects;
    sal_Int32 mnFirstVisibleChild;
    sal_Int32 mnLastVisibleChild;
    bool mbListeningToDocument;
    ::Window* mpWindow;
    sal_Int32 mnFocusedIndex;
    bool mbModelChangeLocked;
};




//===== AccessibleSlideSorterView =============================================

AccessibleSlideSorterView::AccessibleSlideSorterView(
    ::sd::slidesorter::SlideSorter& rSlideSorter,
    const Reference<XAccessible>& rxParent,
    ::Window* pContentWindow)
    : AccessibleSlideSorterViewBase(MutexOwner::maMutex),
      mpImpl(new Implementation(*this,rSlideSorter,pContentWindow)),
      mrSlideSorter(rSlideSorter),
      mxParent(rxParent),
      mnClientId(0),
      mpContentWindow(pContentWindow)
{
    OSL_TRACE("creating AccessibleSlideSorterView");
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
    mpImpl.reset(NULL);
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
    throw (uno::RuntimeException)
{
    return this;
}

//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL AccessibleSlideSorterView::getAccessibleChildCount (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (maMutex);
    return mpImpl->GetVisibleChildCount();
}

Reference<XAccessible > SAL_CALL
    AccessibleSlideSorterView::getAccessibleChild (sal_Int32 nIndex)
    throw (lang::IndexOutOfBoundsException, RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (maMutex);

    if (nIndex<0 || nIndex>=mpImpl->GetVisibleChildCount())
        throw lang::IndexOutOfBoundsException();

    return  mpImpl->GetVisibleChild(nIndex);
}

Reference<XAccessible > SAL_CALL AccessibleSlideSorterView::getAccessibleParent (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());
    Reference<XAccessible> xParent;

    if (mpContentWindow != NULL)
    {
        ::Window* pParent = mpContentWindow->GetAccessibleParentWindow();
        if (pParent != NULL)
            xParent = pParent->GetAccessible();
    }

    return xParent;
}

sal_Int32 SAL_CALL AccessibleSlideSorterView::getAccessibleIndexInParent (void)
    throw (uno::RuntimeException)
{
    OSL_ASSERT(getAccessibleParent().is());
    ThrowIfDisposed();
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());
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
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    static sal_Int16 nRole = AccessibleRole::PANEL;
    return nRole;
}




::rtl::OUString SAL_CALL AccessibleSlideSorterView::getAccessibleDescription (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());

    return String(SdResId(SID_SD_A11Y_I_SLIDEVIEW_D));
}




::rtl::OUString SAL_CALL AccessibleSlideSorterView::getAccessibleName (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    ::vos::OGuard aGuard (Application::GetSolarMutex());

    return String(SdResId(SID_SD_A11Y_I_SLIDEVIEW_N));
}




Reference<XAccessibleRelationSet> SAL_CALL
    AccessibleSlideSorterView::getAccessibleRelationSet (void)
    throw (uno::RuntimeException)
{
    return Reference<XAccessibleRelationSet>();
}




Reference<XAccessibleStateSet > SAL_CALL
    AccessibleSlideSorterView::getAccessibleStateSet (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());
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
        RuntimeException)
{
    Reference<XAccessibleContext> xParentContext;
    Reference<XAccessible> xParent (getAccessibleParent());
    if (xParent.is())
        xParentContext = xParent->getAccessibleContext();

    if (xParentContext.is())
        return xParentContext->getLocale();
    else
        // Strange, no parent!  Anyway, return the default locale.
        return Application::GetSettings().GetLocale();
}




void SAL_CALL AccessibleSlideSorterView::addEventListener(
    const Reference<XAccessibleEventListener >& rxListener)
    throw (RuntimeException)
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




void SAL_CALL AccessibleSlideSorterView::removeEventListener(
    const Reference<XAccessibleEventListener >& rxListener)
    throw (RuntimeException)
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
    throw (RuntimeException)
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
    throw (RuntimeException)
{
    ThrowIfDisposed();
    Reference<XAccessible> xAccessible;
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());

    const Point aTestPoint (aPoint.X, aPoint.Y);
    ::sd::slidesorter::model::SharedPageDescriptor pHitDescriptor (
        mrSlideSorter.GetController().GetPageAt(aTestPoint));
    if (pHitDescriptor.get() != NULL)
        xAccessible = mpImpl->GetAccessibleChild(
            (pHitDescriptor->GetPage()->GetPageNum()-1)/2);

    return xAccessible;
}




awt::Rectangle SAL_CALL AccessibleSlideSorterView::getBounds (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());
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
    throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard aSolarGuard( Application::GetSolarMutex() );
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
    throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());

    if (mpContentWindow)
        mpContentWindow->GrabFocus();
}




sal_Int32 SAL_CALL AccessibleSlideSorterView::getForeground (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    svtools::ColorConfig aColorConfig;
    UINT32 nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
    return static_cast<sal_Int32>(nColor);
}




sal_Int32 SAL_CALL AccessibleSlideSorterView::getBackground (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    UINT32 nColor = Application::GetSettings().GetStyleSettings().GetWindowColor().GetColor();
    return static_cast<sal_Int32>(nColor);
}




//===== XAccessibleSelection ==================================================

void SAL_CALL AccessibleSlideSorterView::selectAccessibleChild (sal_Int32 nChildIndex)
    throw (lang::IndexOutOfBoundsException,
        RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());

    AccessibleSlideSorterObject* pChild = mpImpl->GetAccessibleChild(nChildIndex);
    if (pChild != NULL)
        mrSlideSorter.GetController().GetPageSelector().SelectPage(pChild->GetPageNumber());
    else
        throw lang::IndexOutOfBoundsException();
}




sal_Bool SAL_CALL AccessibleSlideSorterView::isAccessibleChildSelected (sal_Int32 nChildIndex)
    throw (lang::IndexOutOfBoundsException,
        RuntimeException)
{
    ThrowIfDisposed();
    sal_Bool bIsSelected = sal_False;
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());

    AccessibleSlideSorterObject* pChild = mpImpl->GetAccessibleChild(nChildIndex);
    if (pChild != NULL)
        bIsSelected = mrSlideSorter.GetController().GetPageSelector().IsPageSelected(
            pChild->GetPageNumber());
    else
        throw lang::IndexOutOfBoundsException();

    return bIsSelected;
}




void SAL_CALL AccessibleSlideSorterView::clearAccessibleSelection (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());

    mrSlideSorter.GetController().GetPageSelector().DeselectAllPages();
}




void SAL_CALL AccessibleSlideSorterView::selectAllAccessibleChildren (void)
    throw (uno::RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());

    mrSlideSorter.GetController().GetPageSelector().SelectAllPages();
}




sal_Int32 SAL_CALL AccessibleSlideSorterView::getSelectedAccessibleChildCount (void)
    throw (uno::RuntimeException)
{
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());
    return mrSlideSorter.GetController().GetPageSelector().GetSelectedPageCount();
}




Reference<XAccessible > SAL_CALL
    AccessibleSlideSorterView::getSelectedAccessibleChild (sal_Int32 nSelectedChildIndex )
    throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());
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
        RuntimeException)
{
    ThrowIfDisposed();
    const vos::OGuard aSolarGuard (Application::GetSolarMutex());

    AccessibleSlideSorterObject* pChild = mpImpl->GetAccessibleChild(nChildIndex);
    if (pChild != NULL)
        mrSlideSorter.GetController().GetPageSelector().DeselectPage(pChild->GetPageNumber());
    else
        throw lang::IndexOutOfBoundsException();
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
       AccessibleSlideSorterView::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleSlideSorterView"));
}




sal_Bool SAL_CALL
     AccessibleSlideSorterView::supportsService (const OUString& sServiceName)
    throw (::com::sun::star::uno::RuntimeException)
{
    //  Iterate over all supported service names and return true if on of them
    //  matches the given name.
    uno::Sequence< ::rtl::OUString> aSupportedServices (
        getSupportedServiceNames ());
    for (int i=0; i<aSupportedServices.getLength(); i++)
        if (sServiceName == aSupportedServices[i])
            return sal_True;
    return sal_False;
}




uno::Sequence< ::rtl::OUString> SAL_CALL
       AccessibleSlideSorterView::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    static const OUString sServiceNames[3] = {
        OUString(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.accessibility.Accessible")),
        OUString(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.accessibility.AccessibleContext")),
        OUString(RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.drawing.AccessibleSlideSorterView"))
    };
    return uno::Sequence<OUString> (sServiceNames, 3);
}




void AccessibleSlideSorterView::ThrowIfDisposed (void)
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        OSL_TRACE ("Calling disposed object. Throwing exception:");
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM("object has been already disposed")),
            static_cast<uno::XWeak*>(this));
    }
}



sal_Bool AccessibleSlideSorterView::IsDisposed (void)
{
    return (rBHelper.bDisposed || rBHelper.bInDispose);
}

//===== AccessibleSlideSorterView::Implementation =============================

AccessibleSlideSorterView::Implementation::Implementation (
    AccessibleSlideSorterView& rAccessibleSlideSorter,
    ::sd::slidesorter::SlideSorter& rSlideSorter,
    ::Window* pWindow)
    : mrAccessibleSlideSorter(rAccessibleSlideSorter),
      mrSlideSorter(rSlideSorter),
      maPageObjects(),
      mnFirstVisibleChild(0),
      mnLastVisibleChild(-1),
      mbListeningToDocument(false),
      mpWindow(pWindow),
      mnFocusedIndex(-1),
      mbModelChangeLocked(false)
{
    ConnectListeners();
    UpdateChildren();
    UpdateVisibility();
}




AccessibleSlideSorterView::Implementation::~Implementation (void)
{
    ReleaseListeners();
    Clear();
}




void AccessibleSlideSorterView::Implementation::UpdateVisibility (void)
{
    ::sd::slidesorter::view::SlideSorterView::PageRange aRange (
        mrSlideSorter.GetView().GetVisiblePageRange());
    mnFirstVisibleChild = aRange.first;
    mnLastVisibleChild = aRange.second;
}




void AccessibleSlideSorterView::Implementation::UpdateChildren (void)
{
    // Clear the list of accessible children and adapt its size.  It is
    // refilled on demand when later the children are requested.
    Clear();
    maPageObjects.resize(mrSlideSorter.GetModel().GetPageCount());
    UpdateVisibility();
}




void AccessibleSlideSorterView::Implementation::Clear (void)
{
    PageObjectList::iterator iPageObject;
    PageObjectList::iterator iEnd = maPageObjects.end();
    for (iPageObject=maPageObjects.begin(); iPageObject!=iEnd; ++iPageObject)
        if (*iPageObject != NULL)
        {
            Reference<XComponent> xComponent (Reference<XWeak>(iPageObject->get()), UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
            *iPageObject = NULL;
        }
    maPageObjects.clear();
}




sal_Int32 AccessibleSlideSorterView::Implementation::GetVisibleChildCount (void) const
{
    if (mnFirstVisibleChild <= mnLastVisibleChild)
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
    OSL_ASSERT(nIndex>=0 && (sal_uInt32)nIndex<maPageObjects.size());

    AccessibleSlideSorterObject* pChild = NULL;

    if (nIndex>=0 && (sal_uInt32)nIndex<maPageObjects.size())
    {
        if (maPageObjects[nIndex] == NULL)
        {
            ::sd::slidesorter::model::SharedPageDescriptor pDescriptor(
                mrSlideSorter.GetModel().GetPageDescriptor(nIndex));
            if (pDescriptor.get() != NULL)
                maPageObjects[nIndex] = new AccessibleSlideSorterObject(
                    &mrAccessibleSlideSorter,
                    mrSlideSorter,
                    (pDescriptor->GetPage()->GetPageNum()-1)/2);
        }

        pChild = maPageObjects[nIndex].get();
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
}




void AccessibleSlideSorterView::Implementation::ReleaseListeners (void)
{
    mrSlideSorter.GetController().GetFocusManager().RemoveFocusChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,FocusChangeListener));

    mrSlideSorter.GetController().GetSelectionManager()->RemoveSelectionChangeListener(
        LINK(this,AccessibleSlideSorterView::Implementation,SelectionChangeListener));

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
    if (rHint.ISA(SdrHint))
    {
        SdrHint& rSdrHint (*PTR_CAST(SdrHint,&rHint));
        switch (rSdrHint.GetKind())
        {
            case HINT_PAGEORDERCHG:
                if ( ! mbModelChangeLocked)
                {
                    UpdateChildren();
                    mrAccessibleSlideSorter.FireAccessibleEvent(
                        AccessibleEventId::INVALIDATE_ALL_CHILDREN,
                        Any(),
                        Any());
                }
                break;
            default:
                break;
        }
    }
    else if (rHint.ISA(sd::ViewShellHint))
    {
        sd::ViewShellHint& rViewShellHint (*PTR_CAST(sd::ViewShellHint, &rHint));
        switch (rViewShellHint.GetHintId())
        {
            case sd::ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_START:
                mbModelChangeLocked = true;
                break;

            case sd::ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_END:
                mbModelChangeLocked = false;
                UpdateChildren();
                mrAccessibleSlideSorter.FireAccessibleEvent(
                    AccessibleEventId::INVALIDATE_ALL_CHILDREN,
                    Any(),
                    Any());
                break;
            default:
                break;
        }
    }
}




IMPL_LINK(AccessibleSlideSorterView::Implementation, WindowEventListener, VclWindowEvent*, pEvent)
{
    switch (pEvent->GetId())
    {
        case VCLEVENT_WINDOW_MOVE:
        case VCLEVENT_WINDOW_RESIZE:
            UpdateVisibility();
            mrAccessibleSlideSorter.FireAccessibleEvent(
                AccessibleEventId::INVALIDATE_ALL_CHILDREN,
                Any(),
                Any());
            break;

        case VCLEVENT_WINDOW_GETFOCUS:
        case VCLEVENT_WINDOW_LOSEFOCUS:
            mrAccessibleSlideSorter.FireAccessibleEvent(
                AccessibleEventId::SELECTION_CHANGED,
                Any(),
                Any());
            break;
    }
    return 1;
}




IMPL_LINK(AccessibleSlideSorterView::Implementation, SelectionChangeListener, void*, EMPTYARG )
{
    mrAccessibleSlideSorter.FireAccessibleEvent(
        AccessibleEventId::SELECTION_CHANGED,
        Any(),
        Any());
    return 1;
}




IMPL_LINK(AccessibleSlideSorterView::Implementation, FocusChangeListener, void*, EMPTYARG )
{
    sal_Int32 nNewFocusedIndex (
        mrSlideSorter.GetController().GetFocusManager().GetFocusedPageIndex());

    if (nNewFocusedIndex != mnFocusedIndex)
    {
        if (mnFocusedIndex >= 0)
        {
            AccessibleSlideSorterObject* pObject = GetAccessibleChild(mnFocusedIndex);
            if (pObject != NULL)
                pObject->FireAccessibleEvent(
                    AccessibleEventId::STATE_CHANGED,
                    Any(AccessibleStateType::FOCUSED),
                    Any());
        }
        if (nNewFocusedIndex >= 0)
        {
            AccessibleSlideSorterObject* pObject = GetAccessibleChild(nNewFocusedIndex);
            if (pObject != NULL)
                pObject->FireAccessibleEvent(
                    AccessibleEventId::STATE_CHANGED,
                    Any(),
                    Any(AccessibleStateType::FOCUSED));
        }
        mnFocusedIndex = nNewFocusedIndex;
    }
    return 1;
}


} // end of namespace ::accessibility
