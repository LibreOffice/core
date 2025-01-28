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

#pragma once

#include <svl/lstner.hxx>
#include <vcl/accessibility/vclxaccessiblecomponent.hxx>
#include <vcl/textdata.hxx>
#include <vcl/texteng.hxx>
#include <vcl/textview.hxx>
#include <vcl/window.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/AccessibleScrollType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleMultiLineText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <comphelper/accessibletexthelper.hxx>
#include <rtl/ref.hxx>
#include <unotools/weakref.hxx>

#include <unotools/accessiblerelationsethelper.hxx>
#include <queue>
#include <unordered_map>

class TextEngine;
class TextView;

class Document;

class SfxListenerGuard
{
public:
    SfxListenerGuard(::SfxListener & rListener):
        m_rListener(rListener), m_pNotifier(nullptr) {}

    ~SfxListenerGuard() { endListening(); }

    // Not thread safe:
    void startListening(::SfxBroadcaster & rNotifier);

    // Not thread safe:
    void endListening();

private:
    ::SfxListener & m_rListener;
    ::SfxBroadcaster * m_pNotifier;
};

class WindowListenerGuard
{
public:
    WindowListenerGuard(::Link<VclWindowEvent&,void> const & rListener):
        m_aListener(rListener), m_pNotifier(nullptr) {}

    ~WindowListenerGuard() { endListening(); }

    // Not thread safe:
    void startListening(vcl::Window & rNotifier);

    // Not thread safe:
    void endListening();

private:
    ::Link<VclWindowEvent&,void> m_aListener;
    VclPtr<vcl::Window> m_pNotifier;
};

class Paragraph;

class ParagraphInfo
{
public:
    ParagraphInfo(::sal_Int32 nHeight): m_nHeight(nHeight) {}

    unotools::WeakReference< Paragraph > const &
    getParagraph() const { return m_xParagraph; }

    ::sal_Int32 getHeight() const { return m_nHeight; }

    void setParagraph( rtl::Reference< Paragraph > const & rParagraph)
    { m_xParagraph = rParagraph.get(); }

    void changeHeight(::sal_Int32 nHeight) { m_nHeight = nHeight; }

private:
    unotools::WeakReference< Paragraph > m_xParagraph;
    ::sal_Int32 m_nHeight;
};

typedef std::vector< ParagraphInfo > Paragraphs;

typedef cppu::ImplInheritanceHelper<
    comphelper::OAccessibleComponentHelper,
    css::accessibility::XAccessible,
    css::accessibility::XAccessibleEditableText,
    css::accessibility::XAccessibleMultiLineText,
    css::accessibility::XAccessibleTextAttributes> ParagraphBase;

// The Paragraph's number is the absolute position within the text engine (from
// 0 to N - 1), whereas the Paragraph's index is the position within the text
// view/accessible parent (from 0 to M - 1).  Paragraphs outside the currently
// visible range have an index of -1.
class Paragraph final
    : public ParagraphBase, private ::comphelper::OCommonAccessibleText
{
public:
    Paragraph(::rtl::Reference< Document > xDocument,
                  Paragraphs::size_type nNumber);

    // Not thread-safe.
    Paragraphs::size_type getNumber() const { return m_nNumber; }

    // Not thread-safe.
    void numberChanged(bool bIncremented);

    // Not thread-safe.
    void textChanged();

    // Thread-safe.
    void notifyEvent(::sal_Int16 nEventId, css::uno::Any const & rOldValue,
                     css::uno::Any const & rNewValue);

private:
    // OAccessibleComponentHelper
    virtual css::awt::Rectangle implGetBounds() override;

    // OCommonAccessibleText
    virtual void implGetParagraphBoundary( const OUString& rText,
                                           css::i18n::Boundary& rBoundary,
                                           ::sal_Int32 nIndex ) override;
    virtual void implGetLineBoundary( const OUString& rText,
                                      css::i18n::Boundary& rBoundary,
                                      ::sal_Int32 nIndex ) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
    SAL_CALL getAccessibleContext() override;

    virtual sal_Int64 SAL_CALL getAccessibleChildCount() override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleChild(sal_Int64 i) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleParent() override;

    virtual sal_Int64 SAL_CALL getAccessibleIndexInParent() override;

    virtual ::sal_Int16 SAL_CALL getAccessibleRole() override;

    virtual OUString SAL_CALL getAccessibleDescription() override;

    virtual OUString SAL_CALL getAccessibleName() override;

    virtual
    css::uno::Reference< css::accessibility::XAccessibleRelationSet >
    SAL_CALL getAccessibleRelationSet() override;

    virtual sal_Int64 SAL_CALL getAccessibleStateSet() override;

    virtual css::lang::Locale SAL_CALL getLocale() override;

    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleAtPoint(css::awt::Point const & rPoint) override;

    virtual void SAL_CALL grabFocus() override;

    virtual sal_Int32 SAL_CALL getForeground() override;

    virtual sal_Int32 SAL_CALL getBackground() override;

    virtual ::sal_Int32 SAL_CALL getCaretPosition() override;

    virtual sal_Bool SAL_CALL setCaretPosition(::sal_Int32 nIndex) override;

    virtual ::sal_Unicode SAL_CALL getCharacter(::sal_Int32 nIndex) override;

    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
    getCharacterAttributes(::sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes ) override;

    virtual css::awt::Rectangle SAL_CALL
    getCharacterBounds(::sal_Int32 nIndex) override;

    virtual ::sal_Int32 SAL_CALL getCharacterCount() override;

    virtual ::sal_Int32 SAL_CALL
    getIndexAtPoint(css::awt::Point const & rPoint) override;

    virtual OUString SAL_CALL getSelectedText() override;

    virtual ::sal_Int32 SAL_CALL getSelectionStart() override;

    virtual ::sal_Int32 SAL_CALL getSelectionEnd() override;

    virtual sal_Bool SAL_CALL setSelection(::sal_Int32 nStartIndex,
                                             ::sal_Int32 nEndIndex) override;

    virtual sal_Bool SAL_CALL scrollSubstringTo( sal_Int32 nStartIndex, sal_Int32 nEndIndex, css::accessibility::AccessibleScrollType aScrollType) override;

    virtual OUString SAL_CALL getText() override;

    virtual OUString SAL_CALL getTextRange(::sal_Int32 nStartIndex,
                                                  ::sal_Int32 nEndIndex) override;

    virtual css::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;
    virtual css::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) override;

    virtual sal_Bool SAL_CALL copyText(::sal_Int32 nStartIndex,
                                         ::sal_Int32 nEndIndex) override;

    virtual sal_Bool SAL_CALL cutText(::sal_Int32 nStartIndex,
                                        ::sal_Int32 nEndIndex) override;

    virtual sal_Bool SAL_CALL pasteText(::sal_Int32 nIndex) override;

    virtual sal_Bool SAL_CALL deleteText(::sal_Int32 nStartIndex,
                                           ::sal_Int32 nEndIndex) override;

    virtual sal_Bool SAL_CALL insertText(OUString const & rText,
                                           ::sal_Int32 nIndex) override;

    virtual sal_Bool SAL_CALL replaceText(
        ::sal_Int32 nStartIndex, ::sal_Int32 nEndIndex,
        OUString const & rReplacement) override;

    virtual sal_Bool SAL_CALL setAttributes(
        ::sal_Int32 nStartIndex, ::sal_Int32 nEndIndex,
        css::uno::Sequence< css::beans::PropertyValue > const &
        rAttributeSet) override;

    virtual sal_Bool SAL_CALL setText(OUString const & rText) override;

    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
    getDefaultAttributes(const css::uno::Sequence< OUString >& RequestedAttributes) override;

    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
    getRunAttributes(::sal_Int32 Index, const css::uno::Sequence< OUString >& RequestedAttributes) override;

    virtual ::sal_Int32 SAL_CALL getLineNumberAtIndex( ::sal_Int32 nIndex ) override;

    virtual css::accessibility::TextSegment SAL_CALL getTextAtLineNumber( ::sal_Int32 nLineNo ) override;

    virtual css::accessibility::TextSegment SAL_CALL getTextAtLineWithCaret(  ) override;

    virtual ::sal_Int32 SAL_CALL getNumberOfLineWithCaret(  ) override;

    virtual OUString implGetText() override;

    virtual css::lang::Locale implGetLocale() override;

    virtual void implGetSelection(::sal_Int32 & rStartIndex,
                                  ::sal_Int32 & rEndIndex) override;

    ::rtl::Reference< Document > m_xDocument;
    Paragraphs::size_type m_nNumber;

    OUString m_aParagraphText;
};


typedef std::unordered_map< OUString,
                         css::beans::PropertyValue > tPropValMap;

class Document final : public ::VCLXAccessibleComponent, public ::SfxListener
{
public:
    Document(vcl::Window* pWindow, ::TextEngine & rEngine, ::TextView & rView);

    css::uno::Reference<css::accessibility::XAccessible> getAccessible() const;

    css::lang::Locale retrieveLocale();

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const *" instead of a
    // "::rtl::Reference< Paragraph > const &".
    ::sal_Int32 retrieveParagraphIndex(Paragraph const * pParagraph);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const *" instead of a
    // "::rtl::Reference< Paragraph > const &".
    ::sal_Int64 retrieveParagraphState(Paragraph const * pParagraph);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const &" instead of a
    // "::rtl::Reference< Paragraph > const &".
    css::awt::Rectangle
    retrieveParagraphBounds(Paragraph const * pParagraph, bool bAbsolute);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const &" instead of a
    // "::rtl::Reference< Paragraph > const &".
    OUString retrieveParagraphText(Paragraph const * pParagraph);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const &" instead of a
    // "::rtl::Reference< Paragraph > const &".
    void retrieveParagraphSelection(Paragraph const * pParagraph,
                                    ::sal_Int32 * pBegin, ::sal_Int32 * pEnd);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const *" instead of a
    // "::rtl::Reference< Paragraph > const &".
    ::sal_Int32 retrieveParagraphCaretPosition(Paragraph const * pParagraph);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const &" instead of a
    // "::rtl::Reference< Paragraph > const &".
    // Throws css::lang::IndexOutOfBoundsException.
    css::awt::Rectangle
    retrieveCharacterBounds(Paragraph const * pParagraph,
                            ::sal_Int32 nIndex);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const &" instead of a
    // "::rtl::Reference< Paragraph > const &".
    ::sal_Int32 retrieveCharacterIndex(Paragraph const * pParagraph,
                                       css::awt::Point const & rPoint);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const &" instead of a
    // "::rtl::Reference< Paragraph > const &".
    // Throws css::lang::IndexOutOfBoundsException.
    css::uno::Sequence< css::beans::PropertyValue > retrieveCharacterAttributes(
        Paragraph const * pParagraph, ::sal_Int32 nIndex,
        const css::uno::Sequence< OUString >& aRequestedAttributes);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const &" instead of a
    // "::rtl::Reference< Paragraph > const &".
    // Throws css::lang::IndexOutOfBoundsException.
    css::uno::Sequence< css::beans::PropertyValue > retrieveRunAttributes(
        Paragraph const * pParagraph, ::sal_Int32 Index,
        const css::uno::Sequence< OUString >& RequestedAttributes);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const &" instead of a
    // "::rtl::Reference< Paragraph > const &".
    void changeParagraphText(Paragraph const * pParagraph,
                             OUString const & rText);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const &" instead of a
    // "::rtl::Reference< Paragraph > const &".
    // Throws css::lang::IndexOutOfBoundsException.
    void changeParagraphText(Paragraph const * pParagraph, ::sal_Int32 nBegin,
                             ::sal_Int32 nEnd, bool bCut, bool bPaste,
                             OUString const & rText);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const &" instead of a
    // "::rtl::Reference< Paragraph > const &".
    // Throws css::lang::IndexOutOfBoundsException.
    void copyParagraphText(Paragraph const * pParagraph,
                           ::sal_Int32 nBegin, ::sal_Int32 nEnd);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const &" instead of a
    // "::rtl::Reference< Paragraph > const &".
    // Throws css::lang::IndexOutOfBoundsException.
    void changeParagraphAttributes(
        Paragraph const * pParagraph, ::sal_Int32 nBegin, ::sal_Int32 nEnd,
        css::uno::Sequence< css::beans::PropertyValue > const &
        rAttributeSet);

    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "Paragraph const &" instead of a
    // "::rtl::Reference< Paragraph > const &".
    // Throws css::lang::IndexOutOfBoundsException.
    void changeParagraphSelection(Paragraph const * pParagraph,
                                  ::sal_Int32 nBegin, ::sal_Int32 nEnd);

    css::i18n::Boundary
    retrieveParagraphLineBoundary( Paragraph const * pParagraph,
                                   ::sal_Int32 nIndex, ::sal_Int32 *pLineNo);

    css::i18n::Boundary
    retrieveParagraphBoundaryOfLine( Paragraph const * pParagraph,
                                     ::sal_Int32 nIndex );

    sal_Int32 retrieveParagraphLineWithCursor( Paragraph const * pParagraph );

    css::uno::Reference< css::accessibility::XAccessibleRelationSet >
    retrieveParagraphRelationSet( Paragraph const * pParagraph );

private:
    virtual sal_Int64 SAL_CALL getAccessibleChildCount() override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
    SAL_CALL getAccessibleChild(sal_Int64 i) override;

    virtual ::sal_Int16 SAL_CALL getAccessibleRole() override;

    virtual css::uno::Reference< css::accessibility::XAccessible >
    SAL_CALL getAccessibleAtPoint(css::awt::Point const & rPoint) override;

    virtual void FillAccessibleStateSet( sal_Int64& rStateSet ) override;
    virtual void FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet ) override;

    // ??? Will be called with both the external (Solar) and internal mutex
    // locked:
    virtual void SAL_CALL disposing() override;

    // ??? Will be called with the external (Solar) mutex locked.
    virtual void Notify(::SfxBroadcaster & rBC, ::SfxHint const & rHint) override;

    // Assuming that this will only be called with the external (Solar) mutex
    // locked.
    DECL_LINK(WindowEventHandler, VclWindowEvent&, void);

    // Must be called with both the external (Solar) and internal mutex
    // locked:
    static ::rtl::Reference< Paragraph >
    getParagraph(Paragraphs::iterator const & rIt);

    // Must be called with both the external (Solar) and internal mutex
    // locked:
    // Throws css::uno::RuntimeException.
    css::uno::Reference< css::accessibility::XAccessible >
    getAccessibleChild(Paragraphs::iterator const & rIt);

    // Must be called with both the external (Solar) and internal mutex
    // locked:
    void determineVisibleRange();

    // Must be called with both the external (Solar) and internal mutex
    // locked:
    void notifyVisibleRangeChanges(
        Paragraphs::iterator const & rOldVisibleBegin,
        Paragraphs::iterator const & rOldVisibleEnd,
        Paragraphs::iterator const & rInserted);

    // Must be called with both the external (Solar) and internal mutex
    // locked:
    void changeParagraphText(::sal_uInt32 nNumber, ::sal_uInt16 nBegin, ::sal_uInt16 nEnd,
                             bool bCut, bool bPaste,
                             OUString const & rText);

    void
    handleParagraphNotifications();

    void handleSelectionChangeNotification();

    void sendEvent(::sal_Int32 start, ::sal_Int32 end, ::sal_Int16 nEventId);

    void disposeParagraphs();

    static css::uno::Any mapFontColor(::Color const & rColor);

    static ::Color mapFontColor(css::uno::Any const & rColor);

    static css::uno::Any mapFontWeight(::FontWeight nWeight);

    static ::FontWeight mapFontWeight(css::uno::Any const & rWeight);

    void retrieveRunAttributesImpl(
        Paragraph const * pParagraph, ::sal_Int32 Index,
        const css::uno::Sequence< OUString >& RequestedAttributes,
        tPropValMap& rRunAttrSeq);

    Paragraphs::iterator getIter(Paragraphs::size_type n) { return m_aParagraphs.begin() + std::min(n, m_aParagraphs.size()); }
    Paragraphs::iterator visibleBegin() { return getIter(m_nVisibleBegin); }
    Paragraphs::iterator visibleEnd() { return getIter(m_nVisibleEnd); }
    Paragraphs::iterator focused() { return getIter(m_nFocused); }

    ::TextEngine & m_rEngine;
    ::TextView & m_rView;

    SfxListenerGuard m_aEngineListener;
    WindowListenerGuard m_aViewListener;

    Paragraphs m_aParagraphs;

    // m_nViewOffset is from the start of the document (0) to the start of the
    // current view, and m_nViewHeight is the height of the view:
    ::sal_Int32 m_nViewOffset;
    ::sal_Int32 m_nViewHeight;

    // m_nVisibleBegin points to the first Paragraph that is (partially)
    // contained in the view, and m_nVisibleEnd points past the last Paragraph
    // that is (partially) contained.  If no Paragraphs are (partially) in the
    // view, both m_nVisibleBegin and m_nVisibleEnd are set to
    // m_xParagraphs->size().  These values are only changed by
    // determineVisibleRange.
    Paragraphs::size_type m_nVisibleBegin = 0;
    Paragraphs::size_type m_nVisibleEnd = 0;

    // m_nVisibleBeginOffset is from m_nViewOffset back to the start of the
    // Paragraph pointed to by m_aVisibleBegin (and always has a non-negative
    // value).  If m_aVisibleBegin == m_xParagraphs->end(),
    // m_nVisibleBeginOffset is set to 0.  These values are only changed by
    // determineVisibleRange.
    ::sal_Int32 m_nVisibleBeginOffset;

    // If no selection has yet been set, all the following four variables are
    // set to -1.  m_nSelectionLastPara/Pos is also the cursor position.
    ::sal_Int32 m_nSelectionFirstPara;
    ::sal_Int32 m_nSelectionFirstPos;
    ::sal_Int32 m_nSelectionLastPara;
    ::sal_Int32 m_nSelectionLastPos;

    Paragraphs::size_type m_nFocused = 0;

    std::queue< ::TextHint > m_aParagraphNotifications;
    bool m_bSelectionChangedNotification;
    bool m_bInParagraphNotificationsHandler = false;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
