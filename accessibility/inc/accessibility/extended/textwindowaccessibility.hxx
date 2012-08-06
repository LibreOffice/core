/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_ACCESSIBILITY_TEXTWINDOWACCESSIBILITY_HXX
#define INCLUDED_ACCESSIBILITY_TEXTWINDOWACCESSIBILITY_HXX

#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <svl/lstner.hxx>
#include <vcl/textdata.hxx>
#include <vcl/texteng.hxx>
#include <vcl/textview.hxx>
#include <vcl/txtattr.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleMultiLineText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <toolkit/awt/vclxwindow.hxx>
#include <cppuhelper/compbase7.hxx>
#include <comphelper/accessiblecontexthelper.hxx>
#include <comphelper/accessibletexthelper.hxx>
#include <rtl/ref.hxx>

#include <memory>
#include <queue>
#include <boost/unordered_map.hpp>

class TextEngine;
class TextView;

namespace css = ::com::sun::star;

namespace accessibility
{

class Paragraph;
class Document;

class SfxListenerGuard
{
public:
    inline SfxListenerGuard(::SfxListener & rListener):
        m_rListener(rListener), m_pNotifier(0) {}

    inline ~SfxListenerGuard() { endListening(); }

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
    inline WindowListenerGuard(::Link const & rListener):
        m_aListener(rListener), m_pNotifier(0) {}

    inline ~WindowListenerGuard() { endListening(); }

    // Not thread safe:
    void startListening(::Window & rNotifier);

    // Not thread safe:
    void endListening();

private:
    ::Link m_aListener;
    ::Window * m_pNotifier;
};

class ParagraphInfo
{
public:
    inline ParagraphInfo(::sal_Int32 nHeight): m_nHeight(nHeight) {}

    inline
    ::css::uno::WeakReference< ::css::accessibility::XAccessible > const &
    getParagraph() const { return m_xParagraph; }

    inline ::sal_Int32 getHeight() const { return m_nHeight; }

    inline void setParagraph(
        ::css::uno::Reference< ::css::accessibility::XAccessible > const &
        rParagraph) { m_xParagraph = rParagraph; }

    inline void changeHeight(::sal_Int32 nHeight) { m_nHeight = nHeight; }

private:
    ::css::uno::WeakReference< ::css::accessibility::XAccessible >
    m_xParagraph;
    ::sal_Int32 m_nHeight;
};

typedef ::std::vector< ParagraphInfo > Paragraphs;

typedef ::cppu::WeakAggComponentImplHelper7<
    ::css::accessibility::XAccessible,
    ::css::accessibility::XAccessibleContext,
    ::css::accessibility::XAccessibleComponent,
    ::css::accessibility::XAccessibleEditableText,
    ::css::accessibility::XAccessibleMultiLineText,
    ::css::accessibility::XAccessibleTextAttributes,
    ::css::accessibility::XAccessibleEventBroadcaster > ParagraphBase;

// The Paragraph's number is the absolute position within the text engine (from
// 0 to N - 1), whereas the Paragraph's index is the position within the text
// view/accessible parent (from 0 to M - 1).  Paragraphs outside the currently
// visible range have an index of -1.
class ParagraphImpl:
    public ParagraphBase, private ::comphelper::OCommonAccessibleText
{
public:
    ParagraphImpl(::rtl::Reference< Document > const & rDocument,
                  Paragraphs::size_type nNumber, ::osl::Mutex & rMutex);

    // Not thread-safe.
    inline Paragraphs::size_type getNumber() const { return m_nNumber; }

    // Not thread-safe.
    void numberChanged(bool bIncremented);

    // Not thread-safe.
    void textChanged();

    // Thread-safe.
    void notifyEvent(::sal_Int16 nEventId, ::css::uno::Any const & rOldValue,
                     ::css::uno::Any const & rNewValue);

protected:
    // OCommonAccessibleText
    virtual void implGetParagraphBoundary( ::css::i18n::Boundary& rBoundary,
                                           ::sal_Int32 nIndex );
    virtual void implGetLineBoundary( ::css::i18n::Boundary& rBoundary,
                                      ::sal_Int32 nIndex );

private:
    virtual ::css::uno::Reference< ::css::accessibility::XAccessibleContext >
    SAL_CALL getAccessibleContext() throw (::css::uno::RuntimeException);

    virtual ::sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::css::uno::RuntimeException);

    virtual ::css::uno::Reference< ::css::accessibility::XAccessible > SAL_CALL
    getAccessibleChild(::sal_Int32 i)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::css::uno::Reference< ::css::accessibility::XAccessible > SAL_CALL
    getAccessibleParent() throw (::css::uno::RuntimeException);

    virtual ::sal_Int32 SAL_CALL getAccessibleIndexInParent()
        throw (::css::uno::RuntimeException);

    virtual ::sal_Int16 SAL_CALL getAccessibleRole()
        throw (::css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getAccessibleDescription()
        throw (::css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getAccessibleName()
        throw (::css::uno::RuntimeException);

    virtual
    ::css::uno::Reference< ::css::accessibility::XAccessibleRelationSet >
    SAL_CALL getAccessibleRelationSet() throw (::css::uno::RuntimeException);

    virtual
    ::css::uno::Reference< ::css::accessibility::XAccessibleStateSet > SAL_CALL
    getAccessibleStateSet() throw (::css::uno::RuntimeException);

    virtual ::css::lang::Locale SAL_CALL getLocale()
        throw (::css::accessibility::IllegalAccessibleComponentStateException,
               ::css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL containsPoint(::css::awt::Point const & rPoint)
        throw (::css::uno::RuntimeException);

    virtual ::css::uno::Reference< ::css::accessibility::XAccessible > SAL_CALL
    getAccessibleAtPoint(::css::awt::Point const & rPoint)
        throw (::css::uno::RuntimeException);

    virtual ::css::awt::Rectangle SAL_CALL getBounds()
        throw (::css::uno::RuntimeException);

    virtual ::css::awt::Point SAL_CALL getLocation()
        throw (::css::uno::RuntimeException);

    virtual ::css::awt::Point SAL_CALL getLocationOnScreen()
        throw (::css::uno::RuntimeException);

    virtual ::css::awt::Size SAL_CALL getSize()
        throw (::css::uno::RuntimeException);

    virtual void SAL_CALL grabFocus() throw (::css::uno::RuntimeException);

    virtual ::css::uno::Any SAL_CALL getAccessibleKeyBinding()
        throw (::css::uno::RuntimeException);

    virtual ::css::util::Color SAL_CALL getForeground()
        throw (::css::uno::RuntimeException);

    virtual ::css::util::Color SAL_CALL getBackground()
        throw (::css::uno::RuntimeException);

    virtual ::sal_Int32 SAL_CALL getCaretPosition()
        throw (::css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL setCaretPosition(::sal_Int32 nIndex)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::sal_Unicode SAL_CALL getCharacter(::sal_Int32 nIndex)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::css::uno::Sequence< ::css::beans::PropertyValue > SAL_CALL
    getCharacterAttributes(::sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aRequestedAttributes )
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::css::awt::Rectangle SAL_CALL
    getCharacterBounds(::sal_Int32 nIndex)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::sal_Int32 SAL_CALL getCharacterCount()
        throw (::css::uno::RuntimeException);

    virtual ::sal_Int32 SAL_CALL
    getIndexAtPoint(::css::awt::Point const & rPoint)
        throw (::css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getSelectedText()
        throw (::css::uno::RuntimeException);

    virtual ::sal_Int32 SAL_CALL getSelectionStart()
        throw (::css::uno::RuntimeException);

    virtual ::sal_Int32 SAL_CALL getSelectionEnd()
        throw (::css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL setSelection(::sal_Int32 nStartIndex,
                                             ::sal_Int32 nEndIndex)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getText()
        throw (::css::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getTextRange(::sal_Int32 nStartIndex,
                                                  ::sal_Int32 nEndIndex)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL copyText(::sal_Int32 nStartIndex,
                                         ::sal_Int32 nEndIndex)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL cutText(::sal_Int32 nStartIndex,
                                        ::sal_Int32 nEndIndex)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL pasteText(::sal_Int32 nIndex)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL deleteText(::sal_Int32 nStartIndex,
                                           ::sal_Int32 nEndIndex)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL insertText(::rtl::OUString const & rText,
                                           ::sal_Int32 nIndex)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL replaceText(
        ::sal_Int32 nStartIndex, ::sal_Int32 nEndIndex,
        ::rtl::OUString const & rReplacement)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL setAttributes(
        ::sal_Int32 nStartIndex, ::sal_Int32 nEndIndex,
        ::css::uno::Sequence< ::css::beans::PropertyValue > const &
        rAttributeSet)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::sal_Bool SAL_CALL setText(::rtl::OUString const & rText)
        throw (::css::uno::RuntimeException);

    virtual ::css::uno::Sequence< ::css::beans::PropertyValue > SAL_CALL
    getDefaultAttributes(const ::css::uno::Sequence< ::rtl::OUString >& RequestedAttributes)
        throw (::css::uno::RuntimeException);

    virtual ::css::uno::Sequence< ::css::beans::PropertyValue > SAL_CALL
    getRunAttributes(::sal_Int32 Index, const ::css::uno::Sequence< ::rtl::OUString >& RequestedAttributes)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::sal_Int32 SAL_CALL getLineNumberAtIndex( ::sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtLineNumber( ::sal_Int32 nLineNo )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::accessibility::TextSegment SAL_CALL getTextAtLineWithCaret(  )
        throw (::com::sun::star::uno::RuntimeException);

    virtual ::sal_Int32 SAL_CALL getNumberOfLineWithCaret(  )
        throw (::com::sun::star::uno::RuntimeException);

    using cppu::WeakAggComponentImplHelperBase::addEventListener;
    virtual void SAL_CALL addEventListener(
        ::css::uno::Reference<
        ::css::accessibility::XAccessibleEventListener > const & rListener)
        throw (::css::uno::RuntimeException);

    using cppu::WeakAggComponentImplHelperBase::removeEventListener;
    virtual void SAL_CALL removeEventListener(
        ::css::uno::Reference<
        ::css::accessibility::XAccessibleEventListener > const & rListener)
        throw (::css::uno::RuntimeException);

    virtual void SAL_CALL disposing();

    virtual ::rtl::OUString implGetText();

    virtual ::css::lang::Locale implGetLocale();

    virtual void implGetSelection(::sal_Int32 & rStartIndex,
                                  ::sal_Int32 & rEndIndex);

    // Throws ::css::lang::DisposedException:
    void checkDisposed();

    ::rtl::Reference< Document > m_xDocument;
    Paragraphs::size_type m_nNumber;

//    ::cppu::OInterfaceContainerHelper m_aListeners;
    /// client id in the AccessibleEventNotifier queue
    sal_uInt32 m_nClientId;

    ::rtl::OUString m_aParagraphText;
};


typedef ::boost::unordered_map< ::rtl::OUString,
                         ::css::beans::PropertyValue,
                         ::rtl::OUStringHash,
                         ::std::equal_to< ::rtl::OUString > > tPropValMap;

class Document: public ::VCLXAccessibleComponent, public ::SfxListener
{
public:
    Document(::VCLXWindow * pVclXWindow, ::TextEngine & rEngine,
             ::TextView & rView, bool bCompoundControlChild);

    inline ::css::uno::Reference< ::css::accessibility::XAccessible >
    getAccessible() { return m_xAccessible; }

    // Must be called only after init has been called.
    ::css::lang::Locale retrieveLocale();

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const *" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    ::sal_Int32 retrieveParagraphIndex(ParagraphImpl const * pParagraph);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const *" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    ::sal_Int64 retrieveParagraphState(ParagraphImpl const * pParagraph);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const &" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    ::css::awt::Rectangle
    retrieveParagraphBounds(ParagraphImpl const * pParagraph, bool bAbsolute);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const &" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    ::rtl::OUString retrieveParagraphText(ParagraphImpl const * pParagraph);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const &" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    void retrieveParagraphSelection(ParagraphImpl const * pParagraph,
                                    ::sal_Int32 * pBegin, ::sal_Int32 * pEnd);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const *" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    ::sal_Int32 retrieveParagraphCaretPosition(ParagraphImpl const * pParagraph);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const &" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    // Throws ::css::lang::IndexOutOfBoundsException.
    ::css::awt::Rectangle
    retrieveCharacterBounds(ParagraphImpl const * pParagraph,
                            ::sal_Int32 nIndex);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const &" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    ::sal_Int32 retrieveCharacterIndex(ParagraphImpl const * pParagraph,
                                       ::css::awt::Point const & rPoint);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const &" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    // Throws ::css::lang::IndexOutOfBoundsException.
    ::css::uno::Sequence< ::css::beans::PropertyValue > retrieveCharacterAttributes(
        ParagraphImpl const * pParagraph, ::sal_Int32 nIndex,
        const ::css::uno::Sequence< ::rtl::OUString >& aRequestedAttributes);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const &" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    ::css::uno::Sequence< ::css::beans::PropertyValue > retrieveDefaultAttributes(
        ParagraphImpl const * pParagraph,
        const ::css::uno::Sequence< ::rtl::OUString >& RequestedAttributes);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const &" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    // Throws ::css::lang::IndexOutOfBoundsException.
    ::css::uno::Sequence< ::css::beans::PropertyValue > retrieveRunAttributes(
        ParagraphImpl const * pParagraph, ::sal_Int32 Index,
        const ::css::uno::Sequence< ::rtl::OUString >& RequestedAttributes);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const &" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    void changeParagraphText(ParagraphImpl * pParagraph,
                             ::rtl::OUString const & rText);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const &" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    // Throws ::css::lang::IndexOutOfBoundsException.
    void changeParagraphText(ParagraphImpl * pParagraph, ::sal_Int32 nBegin,
                             ::sal_Int32 nEnd, bool bCut, bool bPaste,
                             ::rtl::OUString const & rText);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const &" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    // Throws ::css::lang::IndexOutOfBoundsException.
    void copyParagraphText(ParagraphImpl const * pParagraph,
                           ::sal_Int32 nBegin, ::sal_Int32 nEnd);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const &" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    // Throws ::css::lang::IndexOutOfBoundsException.
    void changeParagraphAttributes(
        ParagraphImpl * pParagraph, ::sal_Int32 nBegin, ::sal_Int32 nEnd,
        ::css::uno::Sequence< ::css::beans::PropertyValue > const &
        rAttributeSet);

    // Must be called only after init has been called.
    // To make it possible for this method to be (indirectly) called from
    // within Paragraph's constructor (i.e., when the Paragraph's ref count is
    // still zero), pass a "ParagraphImpl const &" instead of a
    // "::rtl::Reference< ParagraphImpl > const &".
    // Throws ::css::lang::IndexOutOfBoundsException.
    void changeParagraphSelection(ParagraphImpl * pParagraph,
                                  ::sal_Int32 nBegin, ::sal_Int32 nEnd);

    ::css::i18n::Boundary
    retrieveParagraphLineBoundary( ParagraphImpl const * pParagraph,
                                   ::sal_Int32 nIndex, ::sal_Int32 *pLineNo = NULL);

    ::css::i18n::Boundary
    retrieveParagraphBoundaryOfLine( ParagraphImpl const * pParagraph,
                                     ::sal_Int32 nIndex );

    sal_Int32 retrieveParagraphLineWithCursor( ParagraphImpl const * pParagraph );

    ::css::uno::Reference< ::css::accessibility::XAccessibleRelationSet >
    retrieveParagraphRelationSet( ParagraphImpl const * pParagraph );

protected:
    // window event listener from base class
    virtual void ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );

private:
    virtual ::sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::css::uno::RuntimeException);

    virtual ::css::uno::Reference< ::css::accessibility::XAccessible >
    SAL_CALL getAccessibleChild(::sal_Int32 i)
        throw (::css::lang::IndexOutOfBoundsException,
               ::css::uno::RuntimeException);

    virtual ::sal_Int16 SAL_CALL getAccessibleRole()
        throw (::css::uno::RuntimeException);

    virtual ::css::uno::Reference< ::css::accessibility::XAccessible >
    SAL_CALL getAccessibleAtPoint(::css::awt::Point const & rPoint)
        throw (::css::uno::RuntimeException);

    // ??? Will be called with both the external (Solar) and internal mutex
    // locked:
    virtual void SAL_CALL disposing();

    // ??? Will be called with the external (Solar) mutex locked.
    // init will already have been called.
    virtual void Notify(::SfxBroadcaster & rBC, ::SfxHint const & rHint);

    // Assuming that this will only be called with the external (Solar) mutex
    // locked.
    // init will already have been called.
    DECL_LINK(WindowEventHandler, VclSimpleEvent *);

    // Must be called with both the external (Solar) and internal mutex
    // locked.
    void init();

    // Must be called with both the external (Solar) and internal mutex
    // locked, and after init has been called:
    ::rtl::Reference< ParagraphImpl >
    getParagraph(Paragraphs::iterator const & rIt);

    // Must be called with both the external (Solar) and internal mutex
    // locked, and after init has been called.
    // Throws ::css::uno::RuntimeException.
    ::css::uno::Reference< ::css::accessibility::XAccessible >
    getAccessibleChild(Paragraphs::iterator const & rIt);

    // Must be called with both the external (Solar) and internal mutex
    // locked, and after init has been called:
    void determineVisibleRange();

    // Must be called with both the external (Solar) and internal mutex
    // locked, and after init has been called:
    void notifyVisibleRangeChanges(
        Paragraphs::iterator const & rOldVisibleBegin,
        Paragraphs::iterator const & rOldVisibleEnd,
        Paragraphs::iterator const & rInserted);

    // Must be called with both the external (Solar) and internal mutex
    // locked, and after init has been called:
    void changeParagraphText(::sal_uLong nNumber, ::sal_uInt16 nBegin, ::sal_uInt16 nEnd,
                             bool bCut, bool bPaste,
                             ::rtl::OUString const & rText);

    void
    handleParagraphNotifications();

    void handleSelectionChangeNotification();

    void notifySelectionChange( sal_Int32 nFirst, sal_Int32 nLast );

    void justifySelection( TextPaM& rTextStart, TextPaM& rTextEnd );

    void disposeParagraphs();

    static ::css::uno::Any mapFontColor(::Color const & rColor);

    static ::Color mapFontColor(::css::uno::Any const & rColor);

    static ::css::uno::Any mapFontWeight(::FontWeight nWeight);

    static ::FontWeight mapFontWeight(::css::uno::Any const & rWeight);

    void retrieveDefaultAttributesImpl(
        ParagraphImpl const * pParagraph,
        const ::css::uno::Sequence< ::rtl::OUString >& RequestedAttributes,
        tPropValMap& rDefAttrSeq);

    void retrieveRunAttributesImpl(
        ParagraphImpl const * pParagraph, ::sal_Int32 Index,
        const ::css::uno::Sequence< ::rtl::OUString >& RequestedAttributes,
        tPropValMap& rRunAttrSeq);

    static ::css::uno::Sequence< ::css::beans::PropertyValue >
    convertHashMapToSequence(tPropValMap& rAttrSeq);

    ::css::uno::Reference< ::css::accessibility::XAccessible > m_xAccessible;
    ::TextEngine & m_rEngine;
    ::TextView & m_rView;

    SfxListenerGuard m_aEngineListener;
    WindowListenerGuard m_aViewListener;

    // All the following members have valid values only after calling init:

    ::std::auto_ptr< Paragraphs > m_xParagraphs;

    // m_nViewOffset is from the start of the document (0) to the start of the
    // current view, and m_nViewHeight is the height of the view:
    ::sal_Int32 m_nViewOffset;
    ::sal_Int32 m_nViewHeight;

    // m_aVisibleBegin points to the first Paragraph that is (partially)
    // contained in the view, and m_aVisibleEnd points past the last Paragraph
    // that is (partially) contained.  If no Paragraphs are (partially) in the
    // view, both m_aVisibleBegin and m_aVisibleEnd are set to
    // m_xParagraphs->end().  These values are only changed by
    // determineVisibleRange.
    Paragraphs::iterator m_aVisibleBegin;
    Paragraphs::iterator m_aVisibleEnd;

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

    Paragraphs::iterator m_aFocused;

    ::std::queue< ::TextHint > m_aParagraphNotifications;
    bool m_bSelectionChangedNotification;
    bool m_bCompoundControlChild;
};

}

#endif // INCLUDED_ACCESSIBILITY_TEXTWINDOWACCESSIBILITY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
