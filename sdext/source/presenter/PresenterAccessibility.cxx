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

#include "PresenterAccessibility.hxx"
#include "PresenterTextView.hxx"
#include "PresenterConfigurationAccess.hxx"
#include "PresenterNotesView.hxx"
#include "PresenterPaneBase.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterPaneFactory.hxx"
#include "PresenterViewFactory.hxx"

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

//===== PresenterAccessibleObject =============================================

namespace sdext { namespace presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::accessibility::XAccessible,
        css::accessibility::XAccessibleContext,
        css::accessibility::XAccessibleComponent,
        css::accessibility::XAccessibleEventBroadcaster,
        css::awt::XWindowListener
    > PresenterAccessibleObjectInterfaceBase;
}

class PresenterAccessible::AccessibleObject
    : public ::cppu::BaseMutex,
      public PresenterAccessibleObjectInterfaceBase
{
public:
    AccessibleObject (
        const css::lang::Locale& rLocale,
        const sal_Int16 nRole,
        const OUString& rsName);
    void LateInitialization();

    virtual ~AccessibleObject();

    virtual void SetWindow (
        const css::uno::Reference<css::awt::XWindow>& rxContentWindow,
        const css::uno::Reference<css::awt::XWindow>& rxBorderWindow);
    void SetAccessibleParent (const css::uno::Reference<css::accessibility::XAccessible>& rxAccessibleParent);

    virtual void SAL_CALL disposing() override;

    void AddChild (const ::rtl::Reference<AccessibleObject>& rpChild);
    void RemoveChild (const ::rtl::Reference<AccessibleObject>& rpChild);

    void SetIsFocused (const bool bIsFocused);
    void SetAccessibleName (const OUString& rsName);

    void FireAccessibleEvent (
        const sal_Int16 nEventId,
        const css::uno::Any& rOldValue,
        const css::uno::Any& rNewValue);

    void UpdateStateSet();

    //----- XAccessible -------------------------------------------------------

    virtual css::uno::Reference<css::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext()
        throw (css::uno::RuntimeException, std::exception) override;

    //-----  XAccessibleContext  ----------------------------------------------

    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL getAccessibleParent()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int16 SAL_CALL getAccessibleRole()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getAccessibleDescription()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getAccessibleName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::lang::Locale SAL_CALL getLocale()
        throw (css::uno::RuntimeException,
            css::accessibility::IllegalAccessibleComponentStateException, std::exception) override;

    //-----  XAccessibleComponent  --------------------------------------------

    virtual sal_Bool SAL_CALL containsPoint (
        const css::awt::Point& aPoint)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::accessibility::XAccessible> SAL_CALL
        getAccessibleAtPoint (
            const css::awt::Point& aPoint)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Rectangle SAL_CALL getBounds()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Point SAL_CALL getLocation()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Point SAL_CALL getLocationOnScreen()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Size SAL_CALL getSize()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL grabFocus()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getForeground()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getBackground()
        throw (css::uno::RuntimeException, std::exception) override;

    //-----  XAccessibleEventBroadcaster --------------------------------------

    virtual void SAL_CALL addAccessibleEventListener (
            const css::uno::Reference<css::accessibility::XAccessibleEventListener>& rxListener)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL removeAccessibleEventListener (
            const css::uno::Reference<css::accessibility::XAccessibleEventListener>& rxListener)
        throw (css::uno::RuntimeException, std::exception) override;

    //----- XWindowListener ---------------------------------------------------

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    //----- XEventListener ----------------------------------------------------

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    OUString msName;
    css::uno::Reference<css::awt::XWindow2> mxContentWindow;
    css::uno::Reference<css::awt::XWindow2> mxBorderWindow;
    const css::lang::Locale maLocale;
    const sal_Int16 mnRole;
    sal_uInt32 mnStateSet;
    bool mbIsFocused;
    css::uno::Reference<css::accessibility::XAccessible> mxParentAccessible;
    ::std::vector<rtl::Reference<AccessibleObject> > maChildren;
    ::std::vector<Reference<XAccessibleEventListener> > maListeners;

    virtual awt::Point GetRelativeLocation();
    virtual awt::Size GetSize();
    virtual awt::Point GetAbsoluteParentLocation();

    virtual bool GetWindowState (const sal_Int16 nType) const;

    void UpdateState (const sal_Int16 aState, const bool bValue);

    bool IsDisposed() const;

    void ThrowIfDisposed() const
        throw (css::lang::DisposedException);
};

//===== AccessibleStateSet ====================================================

namespace {
typedef ::cppu::WeakComponentImplHelper <
    css::accessibility::XAccessibleStateSet
    > AccessibleStateSetInterfaceBase;
}

class AccessibleStateSet
    : public ::cppu::BaseMutex,
      public AccessibleStateSetInterfaceBase
{
public:
    explicit AccessibleStateSet (const sal_Int32 nStateSet);
    virtual ~AccessibleStateSet();

    static sal_uInt32 GetStateMask (const sal_Int16 nType);

    //----- XAccessibleStateSet -----------------------------------------------

    virtual sal_Bool SAL_CALL isEmpty()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL contains (sal_Int16 nState)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL containsAll (const css::uno::Sequence<sal_Int16>& rStateSet)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence<sal_Int16> SAL_CALL getStates()
        throw (css::uno::RuntimeException, std::exception) override;

private:
    const sal_Int32 mnStateSet;
};

//===== AccessibleRelationSet =================================================

namespace {
typedef ::cppu::WeakComponentImplHelper <
    css::accessibility::XAccessibleRelationSet
    > AccessibleRelationSetInterfaceBase;
}

class AccessibleRelationSet
    : public ::cppu::BaseMutex,
      public AccessibleRelationSetInterfaceBase
{
public:
    AccessibleRelationSet();
    virtual ~AccessibleRelationSet();

    void AddRelation (
        const sal_Int16 nRelationType,
        const Reference<XInterface>& rxObject);

    //----- XAccessibleRelationSet --------------------------------------------

    virtual sal_Int32 SAL_CALL getRelationCount()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual AccessibleRelation SAL_CALL getRelation (sal_Int32 nIndex)
        throw (css::uno::RuntimeException, css::lang::IndexOutOfBoundsException, std::exception) override;

    virtual sal_Bool SAL_CALL containsRelation (sal_Int16 nRelationType)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual AccessibleRelation SAL_CALL getRelationByType (sal_Int16 nRelationType)
        throw (css::uno::RuntimeException, std::exception) override;

private:
    ::std::vector<AccessibleRelation> maRelations;
};

//===== PresenterAccessibleParagraph ==========================================

namespace {
typedef ::cppu::ImplInheritanceHelper <
    PresenterAccessible::AccessibleObject,
    css::accessibility::XAccessibleText
    > PresenterAccessibleParagraphInterfaceBase;
}

class PresenterAccessible::AccessibleParagraph
    : public PresenterAccessibleParagraphInterfaceBase
{
public:
    AccessibleParagraph (
        const css::lang::Locale& rLocale,
        const sal_Int16 nRole,
        const OUString& rsName,
        const SharedPresenterTextParagraph& rpParagraph,
        const sal_Int32 nParagraphIndex);

    virtual ~AccessibleParagraph();

    //----- XAccessibleContext ------------------------------------------------

    virtual css::uno::Reference<css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet()
        throw (css::uno::RuntimeException, std::exception) override;

    //----- XAccessibleText ---------------------------------------------------

    virtual sal_Int32 SAL_CALL getCaretPosition()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL setCaretPosition (sal_Int32 nIndex)
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual sal_Unicode SAL_CALL getCharacter (sal_Int32 nIndex)
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence<css::beans::PropertyValue> SAL_CALL
        getCharacterAttributes (
            ::sal_Int32 nIndex,
            const css::uno::Sequence<OUString>& rRequestedAttributes)
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual css::awt::Rectangle SAL_CALL getCharacterBounds (sal_Int32 nIndex)
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getCharacterCount()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getIndexAtPoint (const css::awt::Point& rPoint)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getSelectedText()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getSelectionStart()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getSelectionEnd()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL setSelection (sal_Int32 nStartIndex, sal_Int32 nEndIndex)
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getText()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getTextRange (
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex)
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

    virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex (
        sal_Int32 nIndex,
        sal_Int16 nTextType)
        throw (css::lang::IndexOutOfBoundsException,
            css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception) override;

    virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex (
        sal_Int32 nIndex,
        sal_Int16 nTextType)
        throw (css::lang::IndexOutOfBoundsException,
            css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception) override;

    virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex (
        sal_Int32 nIndex,
        sal_Int16 nTextType)
        throw (css::lang::IndexOutOfBoundsException,
            css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL copyText (sal_Int32 nStartIndex, sal_Int32 nEndIndex)
        throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;

protected:
    virtual awt::Point GetRelativeLocation() override;
    virtual awt::Size GetSize() override;
    virtual awt::Point GetAbsoluteParentLocation() override;
    virtual bool GetWindowState (const sal_Int16 nType) const override;

private:
    SharedPresenterTextParagraph mpParagraph;
    const sal_Int32 mnParagraphIndex;
};

//===== AccessibleConsole =====================================================

class AccessibleConsole
{
public:
    static rtl::Reference<PresenterAccessible::AccessibleObject> Create (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const lang::Locale& rLocale)
    {
        OUString sName ("Presenter Console");
        PresenterConfigurationAccess aConfiguration (
            rxContext,
            OUString("/org.openoffice.Office.PresenterScreen/"),
            PresenterConfigurationAccess::READ_ONLY);
        aConfiguration.GetConfigurationNode("Presenter/Accessibility/Console/String")
            >>= sName;

        rtl::Reference<PresenterAccessible::AccessibleObject> pObject (
            new PresenterAccessible::AccessibleObject(
                rLocale, AccessibleRole::PANEL, sName));
        pObject->LateInitialization();
        pObject->UpdateStateSet();

        return pObject;
    }
};

//===== AccessiblePreview =====================================================

class AccessiblePreview
{
public:
    static rtl::Reference<PresenterAccessible::AccessibleObject> Create (
        const Reference<css::uno::XComponentContext>& rxContext,
        const lang::Locale& rLocale,
        const Reference<awt::XWindow>& rxContentWindow,
        const Reference<awt::XWindow>& rxBorderWindow)
    {
        OUString sName ("Presenter Notes Window");
        {
            PresenterConfigurationAccess aConfiguration (
                rxContext,
                OUString("/org.openoffice.Office.PresenterScreen/"),
                PresenterConfigurationAccess::READ_ONLY);
            aConfiguration.GetConfigurationNode("Presenter/Accessibility/Preview/String")
                >>= sName;
        }

        rtl::Reference<PresenterAccessible::AccessibleObject> pObject (
            new PresenterAccessible::AccessibleObject(
                rLocale,
                AccessibleRole::LABEL,
                sName));
        pObject->LateInitialization();
        pObject->UpdateStateSet();
        pObject->SetWindow(rxContentWindow, rxBorderWindow);

        return pObject;
    }
};

//===== AccessibleNotes =======================================================

class AccessibleNotes : public PresenterAccessible::AccessibleObject
{
public:
    AccessibleNotes (
        const css::lang::Locale& rLocale,
        const sal_Int16 nRole,
        const OUString& rsName);

    static rtl::Reference<PresenterAccessible::AccessibleObject> Create (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const lang::Locale& rLocale,
        const Reference<awt::XWindow>& rxContentWindow,
        const Reference<awt::XWindow>& rxBorderWindow,
        const std::shared_ptr<PresenterTextView>& rpTextView);

    void SetTextView (const std::shared_ptr<PresenterTextView>& rpTextView);

    virtual void SetWindow (
        const css::uno::Reference<css::awt::XWindow>& rxContentWindow,
        const css::uno::Reference<css::awt::XWindow>& rxBorderWindow) override;

private:
    std::shared_ptr<PresenterTextView> mpTextView;

    void NotifyCaretChange (
        const sal_Int32 nOldParagraphIndex,
        const sal_Int32 nOldCharacterIndex,
        const sal_Int32 nNewParagraphIndex,
        const sal_Int32 nNewCharacterIndex);
    void HandleTextChange();
};

//===== AccessibleFocusManager ================================================

/** A singleton class that makes sure that only one accessibility object in
    the PresenterConsole hierarchy has the focus.
*/
class AccessibleFocusManager
{
public:
    static std::shared_ptr<AccessibleFocusManager> const & Instance();

    void AddFocusableObject (const ::rtl::Reference<PresenterAccessible::AccessibleObject>& rpObject);
    void RemoveFocusableObject (const ::rtl::Reference<PresenterAccessible::AccessibleObject>& rpObject);

    void FocusObject (const ::rtl::Reference<PresenterAccessible::AccessibleObject>& rpObject);

private:
    static std::shared_ptr<AccessibleFocusManager> mpInstance;
    ::std::vector<rtl::Reference<PresenterAccessible::AccessibleObject> > maFocusableObjects;

    AccessibleFocusManager();
};

//===== PresenterAccessible ===================================================

PresenterAccessible::PresenterAccessible (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    const ::rtl::Reference<PresenterController>& rpPresenterController,
    const Reference<drawing::framework::XPane>& rxMainPane)
    : PresenterAccessibleInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mpPresenterController(rpPresenterController),
      mxMainPane(rxMainPane, UNO_QUERY),
      mxMainWindow(),
      mxPreviewContentWindow(),
      mxPreviewBorderWindow(),
      mxNotesContentWindow(),
      mxNotesBorderWindow(),
      mpAccessibleConsole(),
      mpAccessiblePreview(),
      mpAccessibleNotes(),
      mxAccessibleParent()
{
    if (mxMainPane.is())
        mxMainPane->setAccessible(this);
}

PresenterAccessible::~PresenterAccessible()
{
}

PresenterPaneContainer::SharedPaneDescriptor PresenterAccessible::GetPreviewPane() const
{
    PresenterPaneContainer::SharedPaneDescriptor pPreviewPane;

    if ( ! mpPresenterController.is())
        return pPreviewPane;

    rtl::Reference<PresenterPaneContainer> pContainer (mpPresenterController->GetPaneContainer());
    if ( ! pContainer.is())
        return pPreviewPane;

    pPreviewPane = pContainer->FindPaneURL(PresenterPaneFactory::msCurrentSlidePreviewPaneURL);
    Reference<drawing::framework::XPane> xPreviewPane;
    if (pPreviewPane)
        xPreviewPane = pPreviewPane->mxPane.get();
    if ( ! xPreviewPane.is())
    {
        pPreviewPane = pContainer->FindPaneURL(PresenterPaneFactory::msSlideSorterPaneURL);
    }
    return pPreviewPane;
}

void PresenterAccessible::UpdateAccessibilityHierarchy()
{
    if ( ! mpPresenterController.is())
        return;

    Reference<drawing::framework::XConfigurationController> xConfigurationController(
        mpPresenterController->GetConfigurationController());
    if ( ! xConfigurationController.is())
        return;

    rtl::Reference<PresenterPaneContainer> pPaneContainer (
        mpPresenterController->GetPaneContainer());
    if ( ! pPaneContainer.is())
        return;

    if ( ! mpAccessibleConsole.is())
        return;

    // Get the preview pane (standard or notes view) or the slide overview
    // pane.
    PresenterPaneContainer::SharedPaneDescriptor pPreviewPane(GetPreviewPane());
    Reference<drawing::framework::XPane> xPreviewPane;
    if (pPreviewPane)
        xPreviewPane = pPreviewPane->mxPane.get();

    // Get the notes pane.
    PresenterPaneContainer::SharedPaneDescriptor pNotesPane(
        pPaneContainer->FindPaneURL(PresenterPaneFactory::msNotesPaneURL));
    Reference<drawing::framework::XPane> xNotesPane;
    if (pNotesPane)
        xNotesPane = pNotesPane->mxPane.get();

    // Get the notes view.
    Reference<drawing::framework::XView> xNotesView;
    if (pNotesPane)
        xNotesView = pNotesPane->mxView;
    rtl::Reference<PresenterNotesView> pNotesView (
        dynamic_cast<PresenterNotesView*>(xNotesView.get()));

    UpdateAccessibilityHierarchy(
        pPreviewPane ? pPreviewPane->mxContentWindow : Reference<awt::XWindow>(),
        pPreviewPane ? pPreviewPane->mxBorderWindow : Reference<awt::XWindow>(),
        (pPreviewPane&&pPreviewPane->mxPane.is()) ? pPreviewPane->mxPane->GetTitle() : OUString(),
        pNotesPane ? pNotesPane->mxContentWindow : Reference<awt::XWindow>(),
        pNotesPane ? pNotesPane->mxBorderWindow : Reference<awt::XWindow>(),
        pNotesView.is()
            ? pNotesView->GetTextView()
            : std::shared_ptr<PresenterTextView>());
}

void PresenterAccessible::UpdateAccessibilityHierarchy (
    const Reference<awt::XWindow>& rxPreviewContentWindow,
    const Reference<awt::XWindow>& rxPreviewBorderWindow,
    const OUString& rsTitle,
    const Reference<awt::XWindow>& rxNotesContentWindow,
    const Reference<awt::XWindow>& rxNotesBorderWindow,
    const std::shared_ptr<PresenterTextView>& rpNotesTextView)
{
    if ( ! mpAccessibleConsole.is())
        return;

    if (mxPreviewContentWindow != rxPreviewContentWindow)
    {
        if (mpAccessiblePreview.is())
        {
            mpAccessibleConsole->RemoveChild(mpAccessiblePreview);
            mpAccessiblePreview = nullptr;
        }

        mxPreviewContentWindow = rxPreviewContentWindow;
        mxPreviewBorderWindow = rxPreviewBorderWindow;

        if (mxPreviewContentWindow.is())
        {
            mpAccessiblePreview = AccessiblePreview::Create(
                mxComponentContext,
                lang::Locale(),
                mxPreviewContentWindow,
                mxPreviewBorderWindow);
            mpAccessibleConsole->AddChild(mpAccessiblePreview);
            mpAccessiblePreview->SetAccessibleName(rsTitle);
        }
    }

    if (mxNotesContentWindow != rxNotesContentWindow)
    {
        if (mpAccessibleNotes.is())
        {
            mpAccessibleConsole->RemoveChild(mpAccessibleNotes);
            mpAccessibleNotes = nullptr;
        }

        mxNotesContentWindow = rxNotesContentWindow;
        mxNotesBorderWindow = rxNotesBorderWindow;

        if (mxNotesContentWindow.is())
        {
            mpAccessibleNotes = AccessibleNotes::Create(
                mxComponentContext,
                lang::Locale(),
                mxNotesContentWindow,
                mxNotesBorderWindow,
                rpNotesTextView);
            mpAccessibleConsole->AddChild(mpAccessibleNotes.get());
        }
    }
}

void PresenterAccessible::NotifyCurrentSlideChange (
    const sal_Int32 nCurrentSlideIndex,
    const sal_Int32 nSlideCount)
{
    (void)nCurrentSlideIndex;
    (void)nSlideCount;

    if (mpAccessiblePreview.is())
    {
        PresenterPaneContainer::SharedPaneDescriptor pPreviewPane (GetPreviewPane());
        mpAccessiblePreview->SetAccessibleName(
            (pPreviewPane&&pPreviewPane->mxPane.is()
                ? pPreviewPane->mxPane->GetTitle()
                : OUString()));
    }

    // Play some focus ping-pong to trigger AT tools.
    //AccessibleFocusManager::Instance()->FocusObject(mpAccessibleConsole);
    AccessibleFocusManager::Instance()->FocusObject(mpAccessiblePreview);
}

void SAL_CALL PresenterAccessible::disposing()
{
    UpdateAccessibilityHierarchy(
        nullptr,
        nullptr,
        OUString(),
        nullptr,
        nullptr,
        std::shared_ptr<PresenterTextView>());

    if (mxMainWindow.is())
    {
        mxMainWindow->removeFocusListener(this);

        if (mxMainPane.is())
            mxMainPane->setAccessible(nullptr);
    }

    mpAccessiblePreview = nullptr;
    mpAccessibleNotes = nullptr;
    mpAccessibleConsole = nullptr;
}

//----- XAccessible -----------------------------------------------------------

Reference<XAccessibleContext> SAL_CALL PresenterAccessible::getAccessibleContext()
    throw (css::uno::RuntimeException, std::exception)
{
    if ( ! mpAccessibleConsole.is())
    {
        Reference<XPane> xMainPane (mxMainPane, UNO_QUERY);
        if (xMainPane.is())
        {
            mxMainWindow.set(xMainPane->getWindow(), UNO_QUERY);
            mxMainWindow->addFocusListener(this);
        }
        mpAccessibleConsole = AccessibleConsole::Create(
            mxComponentContext, css::lang::Locale());
        mpAccessibleConsole->SetWindow(mxMainWindow, nullptr);
        mpAccessibleConsole->SetAccessibleParent(mxAccessibleParent);
        UpdateAccessibilityHierarchy();
        if (mpPresenterController.is())
            mpPresenterController->SetAccessibilityActiveState(true);
    }
    return mpAccessibleConsole->getAccessibleContext();
}

//----- XFocusListener ----------------------------------------------------

void SAL_CALL PresenterAccessible::focusGained (const css::awt::FocusEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception)
{
    (void)rEvent;
    SAL_INFO("sdext.presenter", OSL_THIS_FUNC << ": PresenterAccessible::focusGained at " << this
        << " and window " << mxMainWindow.get());
    AccessibleFocusManager::Instance()->FocusObject(mpAccessibleConsole);
}

void SAL_CALL PresenterAccessible::focusLost (const css::awt::FocusEvent& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    (void)rEvent;
    SAL_INFO("sdext.presenter", OSL_THIS_FUNC << ": PresenterAccessible::focusLost at " << this);
    AccessibleFocusManager::Instance()->FocusObject(nullptr);
}

//----- XEventListener ----------------------------------------------------

void SAL_CALL PresenterAccessible::disposing (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    if (rEvent.Source == mxMainWindow)
        mxMainWindow = nullptr;
}

//----- XInitialize -----------------------------------------------------------

void SAL_CALL PresenterAccessible::initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
    throw (css::uno::RuntimeException, std::exception)
{
    if (rArguments.getLength() >= 1)
    {
        mxAccessibleParent.set(rArguments[0], UNO_QUERY);
        if (mpAccessibleConsole.is())
            mpAccessibleConsole->SetAccessibleParent(mxAccessibleParent);
    }
}

//===== PresenterAccessible::AccessibleObject =========================================

PresenterAccessible::AccessibleObject::AccessibleObject (
    const lang::Locale& rLocale,
    const sal_Int16 nRole,
    const OUString& rsName)
    : PresenterAccessibleObjectInterfaceBase(m_aMutex),
      msName(rsName),
      mxContentWindow(),
      mxBorderWindow(),
      maLocale(rLocale),
      mnRole(nRole),
      mnStateSet(0),
      mbIsFocused(false),
      mxParentAccessible(),
      maChildren(),
      maListeners()
{
}

void PresenterAccessible::AccessibleObject::LateInitialization()
{
    AccessibleFocusManager::Instance()->AddFocusableObject(this);
}

PresenterAccessible::AccessibleObject::~AccessibleObject()
{
}

void PresenterAccessible::AccessibleObject::SetWindow (
    const Reference<awt::XWindow>& rxContentWindow,
    const Reference<awt::XWindow>& rxBorderWindow)
{
    Reference<awt::XWindow2> xContentWindow (rxContentWindow, UNO_QUERY);

    if (mxContentWindow.get() != xContentWindow.get())
    {
        if (mxContentWindow.is())
        {
            mxContentWindow->removeWindowListener(this);
        }

        mxContentWindow = xContentWindow;
        mxBorderWindow.set(rxBorderWindow, UNO_QUERY);

        if (mxContentWindow.is())
        {
            mxContentWindow->addWindowListener(this);
        }

        UpdateStateSet();
    }
}

void PresenterAccessible::AccessibleObject::SetAccessibleParent (
    const Reference<XAccessible>& rxAccessibleParent)
{
    mxParentAccessible = rxAccessibleParent;
}

void SAL_CALL PresenterAccessible::AccessibleObject::disposing()
{
    AccessibleFocusManager::Instance()->RemoveFocusableObject(this);
    SetWindow(nullptr, nullptr);
}

//----- XAccessible -------------------------------------------------------

Reference<XAccessibleContext> SAL_CALL
    PresenterAccessible::AccessibleObject::getAccessibleContext()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return this;
}

//-----  XAccessibleContext  ----------------------------------------------

sal_Int32 SAL_CALL PresenterAccessible::AccessibleObject::getAccessibleChildCount()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    const sal_Int32 nChildCount (maChildren.size());

    return nChildCount;
}

Reference<XAccessible> SAL_CALL
    PresenterAccessible::AccessibleObject::getAccessibleChild (sal_Int32 nIndex)
    throw (lang::IndexOutOfBoundsException, RuntimeException, std::exception)
{
    ThrowIfDisposed();

    if (nIndex<0 || nIndex>=sal_Int32(maChildren.size()))
        throw lang::IndexOutOfBoundsException("invalid child index", static_cast<uno::XWeak*>(this));

    return Reference<XAccessible>(maChildren[nIndex].get());
}

Reference<XAccessible> SAL_CALL
    PresenterAccessible::AccessibleObject::getAccessibleParent()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return mxParentAccessible;
}

sal_Int32 SAL_CALL
    PresenterAccessible::AccessibleObject::getAccessibleIndexInParent()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    const Reference<XAccessible> xThis (this);
    if (mxParentAccessible.is())
    {
        const Reference<XAccessibleContext> xContext (mxParentAccessible->getAccessibleContext());
        for (sal_Int32 nIndex=0,nCount=xContext->getAccessibleChildCount();
             nIndex<nCount;
             ++nIndex)
        {
            if (xContext->getAccessibleChild(nIndex) == xThis)
                return nIndex;
        }
    }

    return 0;
}

sal_Int16 SAL_CALL
    PresenterAccessible::AccessibleObject::getAccessibleRole()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return mnRole;
}

OUString SAL_CALL
    PresenterAccessible::AccessibleObject::getAccessibleDescription()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return msName;
}

OUString SAL_CALL
    PresenterAccessible::AccessibleObject::getAccessibleName()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return msName;
}

Reference<XAccessibleRelationSet> SAL_CALL
    PresenterAccessible::AccessibleObject::getAccessibleRelationSet()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return nullptr;
}

Reference<XAccessibleStateSet> SAL_CALL
    PresenterAccessible::AccessibleObject::getAccessibleStateSet()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return Reference<XAccessibleStateSet>(new AccessibleStateSet(mnStateSet));
}

lang::Locale SAL_CALL
    PresenterAccessible::AccessibleObject::getLocale()
    throw (RuntimeException,
        IllegalAccessibleComponentStateException, std::exception)
{
    ThrowIfDisposed();

    if (mxParentAccessible.is())
    {
        Reference<XAccessibleContext> xParentContext (mxParentAccessible->getAccessibleContext());
        if (xParentContext.is())
            return xParentContext->getLocale();
    }
    return maLocale;
}

//-----  XAccessibleComponent  ------------------------------------------------

sal_Bool SAL_CALL PresenterAccessible::AccessibleObject::containsPoint (
    const awt::Point& rPoint)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    if (mxContentWindow.is())
    {
        const awt::Rectangle aBox (getBounds());
        return rPoint.X>=aBox.X
            && rPoint.Y>=aBox.Y
            && rPoint.X<aBox.X+aBox.Width
            && rPoint.Y<aBox.Y+aBox.Height;
    }
    else
        return false;
}

Reference<XAccessible> SAL_CALL
    PresenterAccessible::AccessibleObject::getAccessibleAtPoint (const awt::Point& rPoint)
    throw (RuntimeException, std::exception)
{
    (void)rPoint;
    ThrowIfDisposed();

    return Reference<XAccessible>();
}

awt::Rectangle SAL_CALL PresenterAccessible::AccessibleObject::getBounds()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    const awt::Point aLocation (GetRelativeLocation());
    const awt::Size aSize (GetSize());

    return awt::Rectangle (aLocation.X, aLocation.Y, aSize.Width, aSize.Height);
}

awt::Point SAL_CALL PresenterAccessible::AccessibleObject::getLocation()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    const awt::Point aLocation (GetRelativeLocation());

    return aLocation;
}

awt::Point SAL_CALL PresenterAccessible::AccessibleObject::getLocationOnScreen()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    awt::Point aRelativeLocation (GetRelativeLocation());
    awt::Point aParentLocationOnScreen (GetAbsoluteParentLocation());

    return awt::Point(
        aRelativeLocation.X + aParentLocationOnScreen.X,
        aRelativeLocation.Y + aParentLocationOnScreen.Y);
}

awt::Size SAL_CALL PresenterAccessible::AccessibleObject::getSize()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    const awt::Size aSize (GetSize());

    return aSize;
}

void SAL_CALL PresenterAccessible::AccessibleObject::grabFocus()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (mxBorderWindow.is())
        mxBorderWindow->setFocus();
    else if (mxContentWindow.is())
        mxContentWindow->setFocus();
}

sal_Int32 SAL_CALL PresenterAccessible::AccessibleObject::getForeground()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return 0x00ffffff;
}

sal_Int32 SAL_CALL PresenterAccessible::AccessibleObject::getBackground()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return 0x00000000;
}

//----- XAccessibleEventBroadcaster -------------------------------------------

void SAL_CALL PresenterAccessible::AccessibleObject::addAccessibleEventListener (
    const Reference<XAccessibleEventListener>& rxListener)
    throw (RuntimeException, std::exception)
{
    if (rxListener.is())
    {
        const osl::MutexGuard aGuard(m_aMutex);

        if (IsDisposed())
        {
            uno::Reference<uno::XInterface> xThis (static_cast<XWeak*>(this), UNO_QUERY);
            rxListener->disposing (lang::EventObject(xThis));
        }
        else
        {
            maListeners.push_back(rxListener);
        }
    }
}

void SAL_CALL PresenterAccessible::AccessibleObject::removeAccessibleEventListener (
    const Reference<XAccessibleEventListener>& rxListener)
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();
    if (rxListener.is())
    {
        const osl::MutexGuard aGuard(m_aMutex);

        maListeners.erase(std::remove(maListeners.begin(), maListeners.end(), rxListener));
    }
}

//----- XWindowListener ---------------------------------------------------

void SAL_CALL PresenterAccessible::AccessibleObject::windowResized (
    const css::awt::WindowEvent& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    (void)rEvent;

    FireAccessibleEvent(AccessibleEventId::BOUNDRECT_CHANGED, Any(), Any());
}

void SAL_CALL PresenterAccessible::AccessibleObject::windowMoved (
    const css::awt::WindowEvent& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    (void)rEvent;

    FireAccessibleEvent(AccessibleEventId::BOUNDRECT_CHANGED, Any(), Any());
}

void SAL_CALL PresenterAccessible::AccessibleObject::windowShown (
    const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    (void)rEvent;
    UpdateStateSet();
}

void SAL_CALL PresenterAccessible::AccessibleObject::windowHidden (
    const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    (void)rEvent;
    UpdateStateSet();
}

//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterAccessible::AccessibleObject::disposing (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    if (rEvent.Source == mxContentWindow)
    {
        mxContentWindow = nullptr;
        mxBorderWindow = nullptr;
    }
    else
    {
        SetWindow(nullptr, nullptr);
    }
}

//----- private ---------------------------------------------------------------

bool PresenterAccessible::AccessibleObject::GetWindowState (const sal_Int16 nType) const
{
    switch (nType)
    {
        case AccessibleStateType::ENABLED:
            return mxContentWindow.is() && mxContentWindow->isEnabled();

        case AccessibleStateType::FOCUSABLE:
            return true;

        case AccessibleStateType::FOCUSED:
            return mbIsFocused;

        case AccessibleStateType::SHOWING:
            return mxContentWindow.is() && mxContentWindow->isVisible();

        default:
            return false;
    }
}

void PresenterAccessible::AccessibleObject::UpdateStateSet()
{
    UpdateState(AccessibleStateType::FOCUSABLE, true);
    UpdateState(AccessibleStateType::VISIBLE, true);
    UpdateState(AccessibleStateType::ENABLED, true);
    UpdateState(AccessibleStateType::MULTI_LINE, true);
    UpdateState(AccessibleStateType::SENSITIVE, true);

    UpdateState(AccessibleStateType::ENABLED, GetWindowState(AccessibleStateType::ENABLED));
    UpdateState(AccessibleStateType::FOCUSED, GetWindowState(AccessibleStateType::FOCUSED));
    UpdateState(AccessibleStateType::SHOWING, GetWindowState(AccessibleStateType::SHOWING));
    //    UpdateState(AccessibleStateType::ACTIVE, GetWindowState(AccessibleStateType::ACTIVE));
}

void PresenterAccessible::AccessibleObject::UpdateState(
    const sal_Int16 nState,
    const bool bValue)
{
    const sal_uInt32 nStateMask (AccessibleStateSet::GetStateMask(nState));
    if (((mnStateSet & nStateMask)!=0) != bValue)
    {
        if (bValue)
        {
            mnStateSet |= nStateMask;
            FireAccessibleEvent(AccessibleEventId::STATE_CHANGED, Any(), Any(nState));
        }
        else
        {
            mnStateSet &= ~nStateMask;
            FireAccessibleEvent(AccessibleEventId::STATE_CHANGED, Any(nState), Any());
        }
    }
}

void PresenterAccessible::AccessibleObject::AddChild (
    const ::rtl::Reference<AccessibleObject>& rpChild)
{
    maChildren.push_back(rpChild);
    rpChild->SetAccessibleParent(this);
    FireAccessibleEvent(AccessibleEventId::INVALIDATE_ALL_CHILDREN, Any(), Any());
}

void PresenterAccessible::AccessibleObject::RemoveChild (
    const ::rtl::Reference<AccessibleObject>& rpChild)
{
    rpChild->SetAccessibleParent(Reference<XAccessible>());
    maChildren.erase(::std::find(maChildren.begin(), maChildren.end(), rpChild));
    FireAccessibleEvent(AccessibleEventId::INVALIDATE_ALL_CHILDREN, Any(), Any());
}

void PresenterAccessible::AccessibleObject::SetIsFocused (const bool bIsFocused)
{
    if (mbIsFocused != bIsFocused)
    {
        mbIsFocused = bIsFocused;
        UpdateStateSet();
    }
}

void PresenterAccessible::AccessibleObject::SetAccessibleName (const OUString& rsName)
{
    if (msName != rsName)
    {
        const OUString sOldName(msName);
        msName = rsName;
        FireAccessibleEvent(AccessibleEventId::NAME_CHANGED, Any(sOldName), Any(msName));
    }
}

void PresenterAccessible::AccessibleObject::FireAccessibleEvent (
    const sal_Int16 nEventId,
    const uno::Any& rOldValue,
    const uno::Any& rNewValue )
{
    AccessibleEventObject aEventObject;

    aEventObject.Source = Reference<XWeak>(this);
    aEventObject.EventId = nEventId;
    aEventObject.NewValue = rNewValue;
    aEventObject.OldValue = rOldValue;

    ::std::vector<Reference<XAccessibleEventListener> > aListenerCopy(maListeners);
    for (::std::vector<Reference<XAccessibleEventListener> >::const_iterator
             iListener(aListenerCopy.begin()),
             iEnd(aListenerCopy.end());
         iListener!=iEnd;
         ++iListener)
    {
        try
        {
            (*iListener)->notifyEvent(aEventObject);
        }
        catch (const lang::DisposedException&)
        {
            // Listener has been disposed and should have been removed
            // already.
            removeAccessibleEventListener(*iListener);
        }
        catch (const Exception&)
        {
            // Ignore all other exceptions and assume that they are
            // caused by a temporary problem.
        }
    }
}

awt::Point PresenterAccessible::AccessibleObject::GetRelativeLocation()
{
    awt::Point aLocation;
    if (mxContentWindow.is())
    {
        const awt::Rectangle aContentBox (mxContentWindow->getPosSize());
        aLocation.X = aContentBox.X;
        aLocation.Y = aContentBox.Y;
        if (mxBorderWindow.is())
        {
            const awt::Rectangle aBorderBox (mxBorderWindow->getPosSize());
            aLocation.X += aBorderBox.X;
            aLocation.Y += aBorderBox.Y;
        }
    }
    return aLocation;
}

awt::Size PresenterAccessible::AccessibleObject::GetSize()
{
    if (mxContentWindow.is())
    {
        const awt::Rectangle aBox (mxContentWindow->getPosSize());
        return awt::Size(aBox.Width, aBox.Height);
    }
    else
        return awt::Size();
}

awt::Point PresenterAccessible::AccessibleObject::GetAbsoluteParentLocation()
{
    Reference<XAccessibleComponent> xParentComponent;
    if (mxParentAccessible.is())
        xParentComponent.set( mxParentAccessible->getAccessibleContext(), UNO_QUERY);
    if (xParentComponent.is())
        return xParentComponent->getLocationOnScreen();
    else
        return awt::Point();
}

bool PresenterAccessible::AccessibleObject::IsDisposed() const
{
    return (rBHelper.bDisposed || rBHelper.bInDispose);
}

void PresenterAccessible::AccessibleObject::ThrowIfDisposed() const
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
        throw lang::DisposedException("object has already been disposed", uno::Reference<uno::XInterface>(const_cast<uno::XWeak*>(static_cast<uno::XWeak const *>(this))));
}

//===== AccessibleStateSet ====================================================

AccessibleStateSet::AccessibleStateSet (const sal_Int32 nStateSet)
    : AccessibleStateSetInterfaceBase(m_aMutex),
      mnStateSet (nStateSet)
{
}

AccessibleStateSet::~AccessibleStateSet()
{
}

sal_uInt32 AccessibleStateSet::GetStateMask (const sal_Int16 nState)
{
    if (nState<0 || nState>=sal_Int16(sizeof(sal_uInt32)*8))
    {
        throw RuntimeException("AccessibleStateSet::GetStateMask: invalid state");
    }

    return 1<<nState;
}

//----- XAccessibleStateSet ---------------------------------------------------

sal_Bool SAL_CALL AccessibleStateSet::isEmpty()
    throw (css::uno::RuntimeException, std::exception)
{
    return mnStateSet==0;
}

sal_Bool SAL_CALL AccessibleStateSet::contains (sal_Int16 nState)
    throw (css::uno::RuntimeException, std::exception)
{
    return (mnStateSet & GetStateMask(nState)) != 0;
}

sal_Bool SAL_CALL AccessibleStateSet::containsAll (const css::uno::Sequence<sal_Int16>& rStateSet)
    throw (css::uno::RuntimeException, std::exception)
{
    for (sal_Int32 nIndex=0,nCount=rStateSet.getLength(); nIndex<nCount; ++nIndex)
    {
        if ((mnStateSet & GetStateMask(rStateSet[nIndex])) == 0)
            return false;
    }
    return true;
}

css::uno::Sequence<sal_Int16> SAL_CALL AccessibleStateSet::getStates()
    throw (css::uno::RuntimeException, std::exception)
{
    ::std::vector<sal_Int16> aStates;
    aStates.reserve(sizeof(mnStateSet)*8);
    for (sal_uInt16 nIndex=0; nIndex<sizeof(mnStateSet)*8; ++nIndex)
        if ((mnStateSet & GetStateMask(nIndex)) != 0)
            aStates.push_back(nIndex);
    return Sequence<sal_Int16>(&aStates.front(), aStates.size());
}

//===== AccessibleRelationSet =================================================

AccessibleRelationSet::AccessibleRelationSet()
    : AccessibleRelationSetInterfaceBase(m_aMutex),
      maRelations()
{
}

AccessibleRelationSet::~AccessibleRelationSet()
{
}

void AccessibleRelationSet::AddRelation (
    const sal_Int16 nRelationType,
    const Reference<XInterface>& rxObject)
{
    maRelations.resize(maRelations.size()+1);
    maRelations.back().RelationType = nRelationType;
    maRelations.back().TargetSet.realloc(1);
    maRelations.back().TargetSet[0] = rxObject;
}

//----- XAccessibleRelationSet ------------------------------------------------

sal_Int32 SAL_CALL AccessibleRelationSet::getRelationCount()
    throw (css::uno::RuntimeException, std::exception)
{
    return maRelations.size();
}

AccessibleRelation SAL_CALL AccessibleRelationSet::getRelation (sal_Int32 nIndex)
    throw (css::uno::RuntimeException, css::lang::IndexOutOfBoundsException, std::exception)
{
    if (nIndex<0 && sal_uInt32(nIndex)>=maRelations.size())
        return AccessibleRelation();
    else
        return maRelations[nIndex];
}

sal_Bool SAL_CALL AccessibleRelationSet::containsRelation (sal_Int16 nRelationType)
    throw (css::uno::RuntimeException, std::exception)
{
    for (::std::vector<AccessibleRelation>::const_iterator iRelation(maRelations.begin());
         iRelation!=maRelations.end();
         ++iRelation)
    {
        if (iRelation->RelationType == nRelationType)
            return true;
    }
    return false;
}

AccessibleRelation SAL_CALL AccessibleRelationSet::getRelationByType (sal_Int16 nRelationType)
    throw (css::uno::RuntimeException, std::exception)
{
    for (::std::vector<AccessibleRelation>::const_iterator iRelation(maRelations.begin());
         iRelation!=maRelations.end();
         ++iRelation)
    {
        if (iRelation->RelationType == nRelationType)
            return *iRelation;
    }
    return AccessibleRelation();
}

//===== PresenterAccessible::AccessibleParagraph ==============================

PresenterAccessible::AccessibleParagraph::AccessibleParagraph (
    const lang::Locale& rLocale,
    const sal_Int16 nRole,
    const OUString& rsName,
    const SharedPresenterTextParagraph& rpParagraph,
    const sal_Int32 nParagraphIndex)
    : PresenterAccessibleParagraphInterfaceBase(rLocale, nRole, rsName),
      mpParagraph(rpParagraph),
      mnParagraphIndex(nParagraphIndex)
{
}

PresenterAccessible::AccessibleParagraph::~AccessibleParagraph()
{
}

//----- XAccessibleContext ----------------------------------------------------

Reference<XAccessibleRelationSet> SAL_CALL
    PresenterAccessible::AccessibleParagraph::getAccessibleRelationSet()
    throw (RuntimeException, std::exception)
{
    ThrowIfDisposed();

    rtl::Reference<AccessibleRelationSet> pSet (new AccessibleRelationSet);

    if (mxParentAccessible.is())
    {
        Reference<XAccessibleContext> xParentContext (mxParentAccessible->getAccessibleContext());
        if (xParentContext.is())
        {
            if (mnParagraphIndex>0)
                pSet->AddRelation(
                    AccessibleRelationType::CONTENT_FLOWS_FROM,
                    xParentContext->getAccessibleChild(mnParagraphIndex-1));

            if (mnParagraphIndex<xParentContext->getAccessibleChildCount()-1)
                pSet->AddRelation(
                    AccessibleRelationType::CONTENT_FLOWS_TO,
                    xParentContext->getAccessibleChild(mnParagraphIndex+1));
        }
    }

    return Reference<XAccessibleRelationSet>(pSet.get());
}

//----- XAccessibleText -------------------------------------------------------

sal_Int32 SAL_CALL PresenterAccessible::AccessibleParagraph::getCaretPosition()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    sal_Int32 nPosition (-1);
    if (mpParagraph)
        nPosition = mpParagraph->GetCaretPosition();

    return nPosition;
}

sal_Bool SAL_CALL PresenterAccessible::AccessibleParagraph::setCaretPosition (sal_Int32 nIndex)
    throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    if (mpParagraph)
    {
        mpParagraph->SetCaretPosition(nIndex);
        return true;
    }
    else
        return false;
}

sal_Unicode SAL_CALL PresenterAccessible::AccessibleParagraph::getCharacter (sal_Int32 nIndex)
    throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    if (!mpParagraph)
        throw lang::IndexOutOfBoundsException("no text support in current mode", static_cast<uno::XWeak*>(this));
    return mpParagraph->GetCharacter(nIndex);
}

Sequence<css::beans::PropertyValue> SAL_CALL
    PresenterAccessible::AccessibleParagraph::getCharacterAttributes (
        ::sal_Int32 nIndex,
        const css::uno::Sequence<OUString>& rRequestedAttributes)
    throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

#if OSL_DEBUG_LEVEL > 0
    SAL_INFO( "sdext.presenter", OSL_THIS_FUNC << " at " << this << ", " << nIndex << " returns empty set" );
    for (sal_Int32 nAttributeIndex(0), nAttributeCount(rRequestedAttributes.getLength());
         nAttributeIndex < nAttributeCount;
         ++nAttributeIndex)
    {
        SAL_INFO( "sdext.presenter",
                  "    requested attribute " << nAttributeIndex << " is " << rRequestedAttributes[nAttributeIndex] );
    }
#endif

    // Character properties are not supported.
    (void)nIndex;
    (void)rRequestedAttributes;
    return Sequence<css::beans::PropertyValue>();
}

awt::Rectangle SAL_CALL PresenterAccessible::AccessibleParagraph::getCharacterBounds (
    sal_Int32 nIndex)
    throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    awt::Rectangle aCharacterBox;
    if (nIndex < 0)
    {
        throw lang::IndexOutOfBoundsException("invalid text index", static_cast<uno::XWeak*>(this));
    }
    else if (mpParagraph)
    {
        aCharacterBox = mpParagraph->GetCharacterBounds(nIndex, false);
        // Convert coordinates relative to the window origin into absolute
        // screen coordinates.
        const awt::Point aWindowLocationOnScreen (getLocationOnScreen());
        aCharacterBox.X += aWindowLocationOnScreen.X;
        aCharacterBox.Y += aWindowLocationOnScreen.Y;
    }
    else
    {
        throw lang::IndexOutOfBoundsException("no text support in current mode", static_cast<uno::XWeak*>(this));
    }

    return aCharacterBox;
}

sal_Int32 SAL_CALL PresenterAccessible::AccessibleParagraph::getCharacterCount()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    sal_Int32 nCount (0);
    if (mpParagraph)
        nCount = mpParagraph->GetCharacterCount();

    return nCount;
}

sal_Int32 SAL_CALL PresenterAccessible::AccessibleParagraph::getIndexAtPoint (
    const css::awt::Point& )
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();
    sal_Int32 nIndex (-1);
    return nIndex;
}

OUString SAL_CALL PresenterAccessible::AccessibleParagraph::getSelectedText()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return getTextRange(getSelectionStart(), getSelectionEnd());
}

sal_Int32 SAL_CALL PresenterAccessible::AccessibleParagraph::getSelectionStart()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return getCaretPosition();
}

sal_Int32 SAL_CALL PresenterAccessible::AccessibleParagraph::getSelectionEnd()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    return getCaretPosition();
}

sal_Bool SAL_CALL PresenterAccessible::AccessibleParagraph::setSelection (
    sal_Int32 nStartIndex,
    sal_Int32 nEndIndex)
    throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    (void)nEndIndex;
    ThrowIfDisposed();

    return setCaretPosition(nStartIndex);
}

OUString SAL_CALL PresenterAccessible::AccessibleParagraph::getText()
    throw (css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    OUString sText;
    if (mpParagraph)
        sText = mpParagraph->GetText();

    return sText;
}

OUString SAL_CALL PresenterAccessible::AccessibleParagraph::getTextRange (
    sal_Int32 nLocalStartIndex,
    sal_Int32 nLocalEndIndex)
    throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    OUString sText;
    if (mpParagraph)
    {
        const TextSegment aSegment (
            mpParagraph->CreateTextSegment(nLocalStartIndex, nLocalEndIndex));
        sText = aSegment.SegmentText;
    }

    return sText;
}

TextSegment SAL_CALL PresenterAccessible::AccessibleParagraph::getTextAtIndex (
    sal_Int32 nLocalCharacterIndex,
    sal_Int16 nTextType)
    throw (css::lang::IndexOutOfBoundsException,
        css::lang::IllegalArgumentException,
        css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    TextSegment aSegment;
    if (mpParagraph)
        aSegment = mpParagraph->GetTextSegment(0, nLocalCharacterIndex, nTextType);

    return aSegment;
}

TextSegment SAL_CALL PresenterAccessible::AccessibleParagraph::getTextBeforeIndex (
    sal_Int32 nLocalCharacterIndex,
    sal_Int16 nTextType)
    throw (css::lang::IndexOutOfBoundsException,
        css::lang::IllegalArgumentException,
        css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    TextSegment aSegment;
    if (mpParagraph)
        aSegment = mpParagraph->GetTextSegment(-1, nLocalCharacterIndex, nTextType);

    return aSegment;
}

TextSegment SAL_CALL PresenterAccessible::AccessibleParagraph::getTextBehindIndex (
    sal_Int32 nLocalCharacterIndex,
    sal_Int16 nTextType)
    throw (css::lang::IndexOutOfBoundsException,
        css::lang::IllegalArgumentException,
        css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    TextSegment aSegment;
    if (mpParagraph)
        aSegment = mpParagraph->GetTextSegment(+1, nLocalCharacterIndex, nTextType);

    return aSegment;
}

sal_Bool SAL_CALL PresenterAccessible::AccessibleParagraph::copyText (
    sal_Int32 nStartIndex,
    sal_Int32 nEndIndex)
    throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception)
{
    ThrowIfDisposed();

    // Return false because copying to clipboard is not supported.
    // It IS supported in the notes view.  There is no need to duplicate
    // this here.
    (void)nStartIndex;
    (void)nEndIndex;
    return false;
}

//----- protected -------------------------------------------------------------

awt::Point PresenterAccessible::AccessibleParagraph::GetRelativeLocation()
{
    awt::Point aLocation (AccessibleObject::GetRelativeLocation());
    if (mpParagraph)
    {
        const awt::Point aParagraphLocation (mpParagraph->GetRelativeLocation());
        aLocation.X += aParagraphLocation.X;
        aLocation.Y += aParagraphLocation.Y;
    }

    return aLocation;
}

awt::Size PresenterAccessible::AccessibleParagraph::GetSize()
{
    if (mpParagraph)
        return mpParagraph->GetSize();
    else
        return AccessibleObject::GetSize();
}

awt::Point PresenterAccessible::AccessibleParagraph::GetAbsoluteParentLocation()
{
    if (mxParentAccessible.is())
    {
        Reference<XAccessibleContext> xParentContext(
            mxParentAccessible->getAccessibleContext(), UNO_QUERY);
        if (xParentContext.is())
        {
            Reference<XAccessibleComponent> xGrandParentComponent(
                xParentContext->getAccessibleParent(), UNO_QUERY);
            if (xGrandParentComponent.is())
                return xGrandParentComponent->getLocationOnScreen();
        }
    }

    return awt::Point();
}

bool PresenterAccessible::AccessibleParagraph::GetWindowState (const sal_Int16 nType) const
{
    switch (nType)
    {
        case AccessibleStateType::EDITABLE:
            return mpParagraph.get()!=nullptr;

        case AccessibleStateType::ACTIVE:
            return true;

        default:
            return AccessibleObject::GetWindowState(nType);
    }
}

//===== AccessibleNotes =======================================================

AccessibleNotes::AccessibleNotes (
    const css::lang::Locale& rLocale,
    const sal_Int16 nRole,
    const OUString& rsName)
    : AccessibleObject(rLocale,nRole,rsName),
      mpTextView()
{
}

rtl::Reference<PresenterAccessible::AccessibleObject> AccessibleNotes::Create (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    const lang::Locale& rLocale,
    const Reference<awt::XWindow>& rxContentWindow,
    const Reference<awt::XWindow>& rxBorderWindow,
    const std::shared_ptr<PresenterTextView>& rpTextView)
{
    OUString sName ("Presenter Notes Text");
    {
        PresenterConfigurationAccess aConfiguration (
            rxContext,
            OUString("/org.openoffice.Office.PresenterScreen/"),
            PresenterConfigurationAccess::READ_ONLY);
        aConfiguration.GetConfigurationNode("Presenter/Accessibility/Notes/String")
            >>= sName;
    }

    rtl::Reference<AccessibleNotes> pObject (
        new AccessibleNotes(
            rLocale,
            AccessibleRole::PANEL,
            sName));
    pObject->LateInitialization();
    pObject->SetTextView(rpTextView);
    pObject->UpdateStateSet();
    pObject->SetWindow(rxContentWindow, rxBorderWindow);

    return rtl::Reference<PresenterAccessible::AccessibleObject>(pObject.get());
}

void AccessibleNotes::SetTextView (
    const std::shared_ptr<PresenterTextView>& rpTextView)
{
    ::std::vector<rtl::Reference<PresenterAccessible::AccessibleObject> > aChildren;

    // Release any listeners to the current text view.
    if (mpTextView)
    {
        mpTextView->GetCaret()->SetCaretMotionBroadcaster(
            ::std::function<void (sal_Int32,sal_Int32,sal_Int32,sal_Int32)>());
        mpTextView->SetTextChangeBroadcaster(
            ::std::function<void ()>());
    }

    mpTextView = rpTextView;

    if (mpTextView)
    {
        // Create a new set of children, one for each paragraph.
        const sal_Int32 nParagraphCount (mpTextView->GetParagraphCount());
        for (sal_Int32 nIndex=0; nIndex<nParagraphCount; ++nIndex)
        {
            rtl::Reference<PresenterAccessible::AccessibleParagraph> pParagraph (
                new PresenterAccessible::AccessibleParagraph(
                    css::lang::Locale(),
                    AccessibleRole::PARAGRAPH,
                    "Paragraph"+OUString::number(nIndex),
                    rpTextView->GetParagraph(nIndex),
                    nIndex));
            pParagraph->LateInitialization();
            pParagraph->SetWindow(
                Reference<awt::XWindow>(mxContentWindow, UNO_QUERY),
                Reference<awt::XWindow>(mxBorderWindow, UNO_QUERY));
            pParagraph->SetAccessibleParent(this);
            aChildren.push_back(
                rtl::Reference<PresenterAccessible::AccessibleObject>(pParagraph.get()));
        }
        maChildren.swap(aChildren);
        FireAccessibleEvent(AccessibleEventId::INVALIDATE_ALL_CHILDREN, Any(), Any());

        // Dispose the old children. (This will remove them from the focus
        // manager).
        for (std::vector<rtl::Reference<AccessibleObject> >::const_iterator
                 iChild(aChildren.begin()), iEnd(aChildren.end());
             iChild!=iEnd;
             ++iChild)
        {
            Reference<lang::XComponent> xComponent (static_cast<XWeak*>(iChild->get()), UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
        }

        // This class acts as a controller of who broadcasts caret motion
        // events and handles text changes.  Register the corresponding
        // listeners here.
        mpTextView->GetCaret()->SetCaretMotionBroadcaster(
            [this](sal_Int32 a, sal_Int32 b, sal_Int32 c, sal_Int32 d)
                { return this->NotifyCaretChange(a, b, c, d); });
        mpTextView->SetTextChangeBroadcaster(
            [this]() { return this->HandleTextChange(); });
    }
}

void AccessibleNotes::SetWindow (
    const css::uno::Reference<css::awt::XWindow>& rxContentWindow,
    const css::uno::Reference<css::awt::XWindow>& rxBorderWindow)
{
    AccessibleObject::SetWindow(rxContentWindow, rxBorderWindow);

    // Set the windows at the children as well, so that every paragraph can
    // setup its geometry.
    for (::std::vector<rtl::Reference<AccessibleObject> >::const_iterator
             iChild(maChildren.begin()),
             iEnd(maChildren.end());
         iChild!=iEnd;
         ++iChild)
    {
        (*iChild)->SetWindow(rxContentWindow, rxBorderWindow);
    }
}

void AccessibleNotes::NotifyCaretChange (
    const sal_Int32 nOldParagraphIndex,
    const sal_Int32 nOldCharacterIndex,
    const sal_Int32 nNewParagraphIndex,
    const sal_Int32 nNewCharacterIndex)
{
    AccessibleFocusManager::Instance()->FocusObject(
        nNewParagraphIndex >= 0
            ? maChildren[nNewParagraphIndex]
            : this);

    if (nOldParagraphIndex != nNewParagraphIndex)
    {
        // Moved caret from one paragraph to another (or showed or
        // hid the caret).  Move focus from one accessible
        // paragraph to another.
        if (nOldParagraphIndex >= 0)
        {
            maChildren[nOldParagraphIndex]->FireAccessibleEvent(
                AccessibleEventId::CARET_CHANGED,
                Any(nOldCharacterIndex),
                Any(sal_Int32(-1)));
        }
        if (nNewParagraphIndex >= 0)
        {
            maChildren[nNewParagraphIndex]->FireAccessibleEvent(
                AccessibleEventId::CARET_CHANGED,
                Any(sal_Int32(-1)),
                Any(nNewCharacterIndex));
        }
    }
    else if (nNewParagraphIndex >= 0)
    {
        // Caret moved inside one paragraph.
        maChildren[nNewParagraphIndex]->FireAccessibleEvent(
            AccessibleEventId::CARET_CHANGED,
            Any(nOldCharacterIndex),
            Any(nNewCharacterIndex));
    }
}

void AccessibleNotes::HandleTextChange()
{
    SetTextView(mpTextView);
}

//===== AccessibleFocusManager ================================================

std::shared_ptr<AccessibleFocusManager> AccessibleFocusManager::mpInstance;

std::shared_ptr<AccessibleFocusManager> const & AccessibleFocusManager::Instance()
{
    if ( ! mpInstance)
    {
        mpInstance.reset(new AccessibleFocusManager());
    }
    return mpInstance;
}

AccessibleFocusManager::AccessibleFocusManager()
    : maFocusableObjects()
{
}

void AccessibleFocusManager::AddFocusableObject (
    const ::rtl::Reference<PresenterAccessible::AccessibleObject>& rpObject)
{
    OSL_ASSERT(rpObject.is());
    OSL_ASSERT(::std::find(maFocusableObjects.begin(),maFocusableObjects.end(), rpObject)==maFocusableObjects.end());

    maFocusableObjects.push_back(rpObject);
}

void AccessibleFocusManager::RemoveFocusableObject (
    const ::rtl::Reference<PresenterAccessible::AccessibleObject>& rpObject)
{
    ::std::vector<rtl::Reference<PresenterAccessible::AccessibleObject> >::iterator iObject (
        ::std::find(maFocusableObjects.begin(),maFocusableObjects.end(), rpObject));

    if (iObject != maFocusableObjects.end())
        maFocusableObjects.erase(iObject);
    else
    {
        OSL_ASSERT(iObject!=maFocusableObjects.end());
    }
}

void AccessibleFocusManager::FocusObject (
    const ::rtl::Reference<PresenterAccessible::AccessibleObject>& rpObject)
{
    // Remove the focus of any of the other focusable objects.
    for (::std::vector<rtl::Reference<PresenterAccessible::AccessibleObject> >::const_iterator
             iObject (maFocusableObjects.begin()),
             iEnd (maFocusableObjects.end());
         iObject != iEnd;
         ++iObject)
    {
        if (*iObject!=rpObject)
            (*iObject)->SetIsFocused(false);
    }

    if (rpObject.is())
        rpObject->SetIsFocused(true);
}

} } // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
