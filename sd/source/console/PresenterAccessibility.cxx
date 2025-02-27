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

#include "AccessibleFocusManager.hxx"
#include "PresenterAccessibility.hxx"
#include "PresenterTextView.hxx"
#include "PresenterConfigurationAccess.hxx"
#include "PresenterNotesView.hxx"
#include "PresenterPaneBase.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterPaneFactory.hxx"

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleScrollType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>

#include <algorithm>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sdext::presenter {

namespace {

//===== AccessibleRelationSet =================================================

typedef ::cppu::WeakComponentImplHelper <
    css::accessibility::XAccessibleRelationSet
    > AccessibleRelationSetInterfaceBase;

class AccessibleRelationSet
    : public ::cppu::BaseMutex,
      public AccessibleRelationSetInterfaceBase
{
public:
    AccessibleRelationSet();

    void AddRelation (
        const AccessibleRelationType eRelationType,
        const Reference<XAccessible>& rxObject);

    //----- XAccessibleRelationSet --------------------------------------------

    virtual sal_Int32 SAL_CALL getRelationCount() override;

    virtual AccessibleRelation SAL_CALL getRelation (sal_Int32 nIndex) override;

    virtual sal_Bool SAL_CALL containsRelation(css::accessibility::AccessibleRelationType eRelationType) override;

    virtual AccessibleRelation SAL_CALL getRelationByType(AccessibleRelationType eRelationType) override;

private:
    ::std::vector<AccessibleRelation> maRelations;
};

//===== PresenterAccessibleParagraph ==========================================

typedef ::cppu::ImplInheritanceHelper <
    AccessibleObject,
    css::accessibility::XAccessibleText
    > PresenterAccessibleParagraphInterfaceBase;
}

class PresenterAccessible::AccessibleParagraph
    : public PresenterAccessibleParagraphInterfaceBase
{
public:
    AccessibleParagraph(
        const OUString& rsName,
        SharedPresenterTextParagraph pParagraph,
        const sal_Int32 nParagraphIndex);

    //----- XAccessibleContext ------------------------------------------------

    virtual css::uno::Reference<css::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet() override;

    //----- XAccessibleText ---------------------------------------------------

    virtual sal_Int32 SAL_CALL getCaretPosition() override;

    virtual sal_Bool SAL_CALL setCaretPosition (sal_Int32 nIndex) override;

    virtual sal_Unicode SAL_CALL getCharacter (sal_Int32 nIndex) override;

    virtual css::uno::Sequence<css::beans::PropertyValue> SAL_CALL
        getCharacterAttributes (
            ::sal_Int32 nIndex,
            const css::uno::Sequence<OUString>& rRequestedAttributes) override;

    virtual css::awt::Rectangle SAL_CALL getCharacterBounds (sal_Int32 nIndex) override;

    virtual sal_Int32 SAL_CALL getCharacterCount() override;

    virtual sal_Int32 SAL_CALL getIndexAtPoint (const css::awt::Point& rPoint) override;

    virtual OUString SAL_CALL getSelectedText() override;

    virtual sal_Int32 SAL_CALL getSelectionStart() override;

    virtual sal_Int32 SAL_CALL getSelectionEnd() override;

    virtual sal_Bool SAL_CALL setSelection (sal_Int32 nStartIndex, sal_Int32 nEndIndex) override;

    virtual OUString SAL_CALL getText() override;

    virtual OUString SAL_CALL getTextRange (
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex) override;

    virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex (
        sal_Int32 nIndex,
        sal_Int16 nTextType) override;

    virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex (
        sal_Int32 nIndex,
        sal_Int16 nTextType) override;

    virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex (
        sal_Int32 nIndex,
        sal_Int16 nTextType) override;

    virtual sal_Bool SAL_CALL copyText (sal_Int32 nStartIndex, sal_Int32 nEndIndex) override;

    virtual sal_Bool SAL_CALL scrollSubstringTo(
        sal_Int32 nStartIndex,
        sal_Int32 nEndIndex,
        css::accessibility::AccessibleScrollType aScrollType) override;

protected:
    virtual awt::Point GetRelativeLocation() override;
    virtual awt::Size GetSize() override;
    virtual awt::Point GetAbsoluteParentLocation() override;
    virtual bool GetWindowState (const sal_Int64 nType) const override;

private:
    SharedPresenterTextParagraph mpParagraph;
    const sal_Int32 mnParagraphIndex;
};

//===== AccessibleConsole =====================================================

namespace {

class AccessibleConsole
{
public:
    static rtl::Reference<AccessibleObject> Create (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext)
    {
        OUString sName (u"Presenter Console"_ustr);
        PresenterConfigurationAccess aConfiguration (
            rxContext,
            u"/org.openoffice.Office.PresenterScreen/"_ustr,
            PresenterConfigurationAccess::READ_ONLY);
        aConfiguration.GetConfigurationNode(u"Presenter/Accessibility/Console/String"_ustr)
            >>= sName;

        rtl::Reference<AccessibleObject> pObject (
            new AccessibleObject(AccessibleRole::PANEL, sName));
        pObject->LateInitialization();
        pObject->UpdateStateSet();

        return pObject;
    }
};

//===== AccessiblePreview =====================================================

class AccessiblePreview
{
public:
    static rtl::Reference<AccessibleObject> Create (
        const Reference<css::uno::XComponentContext>& rxContext,
        const Reference<awt::XWindow>& rxContentWindow,
        const Reference<awt::XWindow>& rxBorderWindow)
    {
        OUString sName (u"Presenter Notes Window"_ustr);
        {
            PresenterConfigurationAccess aConfiguration (
                rxContext,
                u"/org.openoffice.Office.PresenterScreen/"_ustr,
                PresenterConfigurationAccess::READ_ONLY);
            aConfiguration.GetConfigurationNode(u"Presenter/Accessibility/Preview/String"_ustr)
                >>= sName;
        }

        rtl::Reference<AccessibleObject> pObject (
            new AccessibleObject(
                AccessibleRole::LABEL,
                sName));
        pObject->LateInitialization();
        pObject->UpdateStateSet();
        pObject->SetWindow(rxContentWindow, rxBorderWindow);

        return pObject;
    }
};

//===== AccessibleNotes =======================================================

class AccessibleNotes : public AccessibleObject
{
public:
    AccessibleNotes(const OUString& rsName);

    static rtl::Reference<AccessibleObject> Create (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
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
};

}

//===== PresenterAccessible ===================================================

PresenterAccessible::PresenterAccessible (
    css::uno::Reference<css::uno::XComponentContext> xContext,
    ::rtl::Reference<PresenterController> xPresenterController,
    const Reference<drawing::framework::XPane>& rxMainPane)
    : PresenterAccessibleInterfaceBase(m_aMutex),
      mxComponentContext(std::move(xContext)),
      mpPresenterController(std::move(xPresenterController)),
      mxMainPane(rxMainPane, UNO_QUERY)
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
                mxPreviewContentWindow,
                mxPreviewBorderWindow);
            mpAccessibleConsole->AddChild(mpAccessiblePreview);
            mpAccessiblePreview->SetAccessibleName(rsTitle);
        }
    }

    if (mxNotesContentWindow == rxNotesContentWindow)
        return;

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
            mxNotesContentWindow,
            mxNotesBorderWindow,
            rpNotesTextView);
        mpAccessibleConsole->AddChild(mpAccessibleNotes);
    }
}

void PresenterAccessible::NotifyCurrentSlideChange ()
{
    if (mpAccessiblePreview.is())
    {
        PresenterPaneContainer::SharedPaneDescriptor pPreviewPane (GetPreviewPane());
        mpAccessiblePreview->SetAccessibleName(
            pPreviewPane&&pPreviewPane->mxPane.is()
                ? pPreviewPane->mxPane->GetTitle()
                : OUString());
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
{
    if ( ! mpAccessibleConsole.is())
    {
        Reference<XPane> xMainPane (mxMainPane, UNO_QUERY);
        if (xMainPane.is())
        {
            mxMainWindow = xMainPane->getWindow();
            mxMainWindow->addFocusListener(this);
        }
        mpAccessibleConsole = AccessibleConsole::Create(
            mxComponentContext);
        mpAccessibleConsole->SetWindow(mxMainWindow, nullptr);
        mpAccessibleConsole->SetAccessibleParent(mxAccessibleParent);
        UpdateAccessibilityHierarchy();
        if (mpPresenterController.is())
            mpPresenterController->SetAccessibilityActiveState(true);
    }
    return mpAccessibleConsole->getAccessibleContext();
}

//----- XFocusListener ----------------------------------------------------

void SAL_CALL PresenterAccessible::focusGained (const css::awt::FocusEvent&)
{
    SAL_INFO("sdext.presenter", __func__ << ": PresenterAccessible::focusGained at " << this
        << " and window " << mxMainWindow.get());
    AccessibleFocusManager::Instance()->FocusObject(mpAccessibleConsole);
}

void SAL_CALL PresenterAccessible::focusLost (const css::awt::FocusEvent&)
{
    SAL_INFO("sdext.presenter", __func__ << ": PresenterAccessible::focusLost at " << this);
    AccessibleFocusManager::Instance()->FocusObject(nullptr);
}

//----- XEventListener ----------------------------------------------------

void SAL_CALL PresenterAccessible::disposing (const css::lang::EventObject& rEvent)
{
    if (rEvent.Source == mxMainWindow)
        mxMainWindow = nullptr;
}

//----- XInitialize -----------------------------------------------------------

void SAL_CALL PresenterAccessible::initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
{
    if (rArguments.hasElements())
    {
        mxAccessibleParent.set(rArguments[0], UNO_QUERY);
        if (mpAccessibleConsole.is())
            mpAccessibleConsole->SetAccessibleParent(mxAccessibleParent);
    }
}


//===== AccessibleRelationSet =================================================

AccessibleRelationSet::AccessibleRelationSet()
    : AccessibleRelationSetInterfaceBase(m_aMutex)
{
}

void AccessibleRelationSet::AddRelation (
    const AccessibleRelationType eRelationType,
    const Reference<XAccessible>& rxObject)
{
    maRelations.emplace_back();
    maRelations.back().RelationType = eRelationType;
    maRelations.back().TargetSet = { rxObject };
}

//----- XAccessibleRelationSet ------------------------------------------------

sal_Int32 SAL_CALL AccessibleRelationSet::getRelationCount()
{
    return maRelations.size();
}

AccessibleRelation SAL_CALL AccessibleRelationSet::getRelation (sal_Int32 nIndex)
{
    if (nIndex<0 && o3tl::make_unsigned(nIndex)>=maRelations.size())
        return AccessibleRelation();
    else
        return maRelations[nIndex];
}

sal_Bool SAL_CALL AccessibleRelationSet::containsRelation(AccessibleRelationType eRelationType)
{
    return std::any_of(maRelations.begin(), maRelations.end(),
        [eRelationType](const AccessibleRelation& rRelation) { return rRelation.RelationType == eRelationType; });
}

AccessibleRelation SAL_CALL AccessibleRelationSet::getRelationByType(AccessibleRelationType eRelationType)
{
    auto iRelation = std::find_if(maRelations.begin(), maRelations.end(),
        [eRelationType](const AccessibleRelation& rRelation) { return rRelation.RelationType == eRelationType; });
    if (iRelation != maRelations.end())
        return *iRelation;
    return AccessibleRelation();
}

//===== PresenterAccessible::AccessibleParagraph ==============================

PresenterAccessible::AccessibleParagraph::AccessibleParagraph(
    const OUString& rsName,
    SharedPresenterTextParagraph xParagraph,
    const sal_Int32 nParagraphIndex)
    : PresenterAccessibleParagraphInterfaceBase(AccessibleRole::PARAGRAPH, rsName),
      mpParagraph(std::move(xParagraph)),
      mnParagraphIndex(nParagraphIndex)
{
}

//----- XAccessibleContext ----------------------------------------------------

Reference<XAccessibleRelationSet> SAL_CALL
    PresenterAccessible::AccessibleParagraph::getAccessibleRelationSet()
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
                    AccessibleRelationType_CONTENT_FLOWS_FROM,
                    xParentContext->getAccessibleChild(mnParagraphIndex-1));

            if (mnParagraphIndex<xParentContext->getAccessibleChildCount()-1)
                pSet->AddRelation(
                    AccessibleRelationType_CONTENT_FLOWS_TO,
                    xParentContext->getAccessibleChild(mnParagraphIndex+1));
        }
    }

    return pSet;
}

//----- XAccessibleText -------------------------------------------------------

sal_Int32 SAL_CALL PresenterAccessible::AccessibleParagraph::getCaretPosition()
{
    ThrowIfDisposed();

    sal_Int32 nPosition (-1);
    if (mpParagraph)
        nPosition = mpParagraph->GetCaretPosition();

    return nPosition;
}

sal_Bool SAL_CALL PresenterAccessible::AccessibleParagraph::setCaretPosition (sal_Int32 nIndex)
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
{
    ThrowIfDisposed();

    if (!mpParagraph)
        throw lang::IndexOutOfBoundsException(u"no text support in current mode"_ustr, static_cast<uno::XWeak*>(this));
    return mpParagraph->GetCharacter(nIndex);
}

Sequence<css::beans::PropertyValue> SAL_CALL
    PresenterAccessible::AccessibleParagraph::getCharacterAttributes (
        ::sal_Int32 nIndex,
        const css::uno::Sequence<OUString>& rRequestedAttributes)
{
    ThrowIfDisposed();

#if OSL_DEBUG_LEVEL > 0
    SAL_INFO( "sdext.presenter", __func__ << " at " << this << ", " << nIndex << " returns empty set" );
    for (sal_Int32 nAttributeIndex(0), nAttributeCount(rRequestedAttributes.getLength());
         nAttributeIndex < nAttributeCount;
         ++nAttributeIndex)
    {
        SAL_INFO( "sdext.presenter",
                  "    requested attribute " << nAttributeIndex << " is " << rRequestedAttributes[nAttributeIndex] );
    }
#else
    (void)nIndex;
    (void)rRequestedAttributes;
#endif

    // Character properties are not supported.
    return Sequence<css::beans::PropertyValue>();
}

awt::Rectangle SAL_CALL PresenterAccessible::AccessibleParagraph::getCharacterBounds (
    sal_Int32 nIndex)
{
    ThrowIfDisposed();

    awt::Rectangle aCharacterBox;
    if (nIndex < 0)
    {
        throw lang::IndexOutOfBoundsException(u"invalid text index"_ustr, static_cast<uno::XWeak*>(this));
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
        throw lang::IndexOutOfBoundsException(u"no text support in current mode"_ustr, static_cast<uno::XWeak*>(this));
    }

    return aCharacterBox;
}

sal_Int32 SAL_CALL PresenterAccessible::AccessibleParagraph::getCharacterCount()
{
    ThrowIfDisposed();

    sal_Int32 nCount (0);
    if (mpParagraph)
        nCount = mpParagraph->GetCharacterCount();

    return nCount;
}

sal_Int32 SAL_CALL PresenterAccessible::AccessibleParagraph::getIndexAtPoint (
    const css::awt::Point& )
{
    ThrowIfDisposed();
    return -1;
}

OUString SAL_CALL PresenterAccessible::AccessibleParagraph::getSelectedText()
{
    ThrowIfDisposed();

    return getTextRange(getSelectionStart(), getSelectionEnd());
}

sal_Int32 SAL_CALL PresenterAccessible::AccessibleParagraph::getSelectionStart()
{
    ThrowIfDisposed();

    return getCaretPosition();
}

sal_Int32 SAL_CALL PresenterAccessible::AccessibleParagraph::getSelectionEnd()
{
    ThrowIfDisposed();

    return getCaretPosition();
}

sal_Bool SAL_CALL PresenterAccessible::AccessibleParagraph::setSelection (
    sal_Int32 nStartIndex,
    sal_Int32)
{
    ThrowIfDisposed();

    return setCaretPosition(nStartIndex);
}

OUString SAL_CALL PresenterAccessible::AccessibleParagraph::getText()
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
{
    ThrowIfDisposed();

    TextSegment aSegment;
    if (mpParagraph)
        aSegment = mpParagraph->GetTextSegment(+1, nLocalCharacterIndex, nTextType);

    return aSegment;
}

sal_Bool SAL_CALL PresenterAccessible::AccessibleParagraph::copyText (
    sal_Int32,
    sal_Int32)
{
    ThrowIfDisposed();

    // Return false because copying to clipboard is not supported.
    // It IS supported in the notes view.  There is no need to duplicate
    // this here.
    return false;
}

sal_Bool SAL_CALL PresenterAccessible::AccessibleParagraph::scrollSubstringTo(
    sal_Int32,
    sal_Int32,
    AccessibleScrollType)
{
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
        Reference<XAccessibleContext> xParentContext =
            mxParentAccessible->getAccessibleContext();
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

bool PresenterAccessible::AccessibleParagraph::GetWindowState (const sal_Int64 nType) const
{
    switch (nType)
    {
        case AccessibleStateType::EDITABLE:
            return bool(mpParagraph);

        case AccessibleStateType::ACTIVE:
            return true;

        default:
            return AccessibleObject::GetWindowState(nType);
    }
}

//===== AccessibleNotes =======================================================

AccessibleNotes::AccessibleNotes(const OUString& rsName)
    : AccessibleObject(AccessibleRole::PANEL, rsName)
{
}

rtl::Reference<AccessibleObject> AccessibleNotes::Create (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    const Reference<awt::XWindow>& rxContentWindow,
    const Reference<awt::XWindow>& rxBorderWindow,
    const std::shared_ptr<PresenterTextView>& rpTextView)
{
    OUString sName (u"Presenter Notes Text"_ustr);
    {
        PresenterConfigurationAccess aConfiguration (
            rxContext,
            u"/org.openoffice.Office.PresenterScreen/"_ustr,
            PresenterConfigurationAccess::READ_ONLY);
        aConfiguration.GetConfigurationNode(u"Presenter/Accessibility/Notes/String"_ustr)
            >>= sName;
    }

    rtl::Reference<AccessibleNotes> pObject (
        new AccessibleNotes(sName));
    pObject->LateInitialization();
    pObject->SetTextView(rpTextView);
    pObject->UpdateStateSet();
    pObject->SetWindow(rxContentWindow, rxBorderWindow);

    return pObject;
}

void AccessibleNotes::SetTextView (
    const std::shared_ptr<PresenterTextView>& rpTextView)
{
    ::std::vector<rtl::Reference<AccessibleObject> > aChildren;

    // Release any listeners to the current text view.
    if (mpTextView)
    {
        mpTextView->GetCaret()->SetCaretMotionBroadcaster(
            ::std::function<void (sal_Int32,sal_Int32,sal_Int32,sal_Int32)>());
        mpTextView->SetTextChangeBroadcaster(
            ::std::function<void ()>());
    }

    mpTextView = rpTextView;

    if (!mpTextView)
        return;

    // Create a new set of children, one for each paragraph.
    const sal_Int32 nParagraphCount (mpTextView->GetParagraphCount());
    for (sal_Int32 nIndex=0; nIndex<nParagraphCount; ++nIndex)
    {
        rtl::Reference<PresenterAccessible::AccessibleParagraph> pParagraph (
            new PresenterAccessible::AccessibleParagraph(
                "Paragraph"+OUString::number(nIndex),
                rpTextView->GetParagraph(nIndex),
                nIndex));
        pParagraph->LateInitialization();
        pParagraph->SetWindow(mxContentWindow, mxBorderWindow);
        pParagraph->SetAccessibleParent(this);
        aChildren.emplace_back(pParagraph.get());
    }
    maChildren.swap(aChildren);
    FireAccessibleEvent(AccessibleEventId::INVALIDATE_ALL_CHILDREN, Any(), Any());

    // Dispose the old children. (This will remove them from the focus
    // manager).
    for (const auto& rxChild : aChildren)
    {
        if (rxChild.is())
            rxChild->dispose();
    }

    // This class acts as a controller of who broadcasts caret motion
    // events and handles text changes.  Register the corresponding
    // listeners here.
    mpTextView->GetCaret()->SetCaretMotionBroadcaster(
        [this](sal_Int32 a, sal_Int32 b, sal_Int32 c, sal_Int32 d)
            { return this->NotifyCaretChange(a, b, c, d); });
    mpTextView->SetTextChangeBroadcaster(
        [this]() { return SetTextView(mpTextView); });
}

void AccessibleNotes::SetWindow (
    const css::uno::Reference<css::awt::XWindow>& rxContentWindow,
    const css::uno::Reference<css::awt::XWindow>& rxBorderWindow)
{
    AccessibleObject::SetWindow(rxContentWindow, rxBorderWindow);

    // Set the windows at the children as well, so that every paragraph can
    // setup its geometry.
    for (auto& rxChild : maChildren)
    {
        rxChild->SetWindow(rxContentWindow, rxBorderWindow);
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

} // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
