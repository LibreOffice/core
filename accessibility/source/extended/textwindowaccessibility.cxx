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


#include <accessibility/extended/textwindowaccessibility.hxx>
#include "comphelper/accessibleeventnotifier.hxx"
#include "unotools/accessiblerelationsethelper.hxx"
#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/window.hxx>
#include <toolkit/helper/convert.hxx>

#include <algorithm>
#include <vector>
#include <boost/unordered_map.hpp>

namespace css = ::com::sun::star;

namespace accessibility
{

// Both ::osl::Mutex and ParagraphBase implement acquire and release, and thus
// ::rtl::Reference< Paragraph > does not work.  So ParagraphImpl was factored
// out and ::rtl::Reference< ParagraphImpl > is used instead.
class Paragraph: private ::osl::Mutex, public ParagraphImpl
{
public:
    inline Paragraph(::rtl::Reference< Document > const & rDocument,
                     Paragraphs::size_type nNumber):
        ParagraphImpl(rDocument, nNumber, *this) {}
};

void SfxListenerGuard::startListening(::SfxBroadcaster & rNotifier)
{
    OSL_ENSURE(m_pNotifier == 0, "called more than once");
    m_pNotifier = &rNotifier;
    m_rListener.StartListening(*m_pNotifier, true);
}

void SfxListenerGuard::endListening()
{
    if (m_pNotifier != 0)
    {
        m_rListener.EndListening(*m_pNotifier);
        m_pNotifier = 0;
    }
}

void WindowListenerGuard::startListening(::Window & rNotifier)
{
    OSL_ENSURE(m_pNotifier == 0, "called more than once");
    m_pNotifier = &rNotifier;
    m_pNotifier->AddEventListener(m_aListener);
}

void WindowListenerGuard::endListening()
{
    if (m_pNotifier != 0)
    {
        m_pNotifier->RemoveEventListener(m_aListener);
        m_pNotifier = 0;
    }
}

ParagraphImpl::ParagraphImpl(::rtl::Reference< Document > const & rDocument,
                             Paragraphs::size_type nNumber,
                             ::osl::Mutex & rMutex):
    ParagraphBase(rMutex),
    m_xDocument(rDocument),
    m_nNumber(nNumber),
    m_nClientId(0)
{
    m_aParagraphText = m_xDocument->retrieveParagraphText(this);
}

void
ParagraphImpl::numberChanged(bool bIncremented)
{
    if (bIncremented)
        ++m_nNumber;
    else
        --m_nNumber;
}

void ParagraphImpl::textChanged()
{
    OUString aParagraphText = implGetText();
    ::css::uno::Any aOldValue, aNewValue;
    if ( implInitTextChangedEvent( m_aParagraphText, aParagraphText, aOldValue, aNewValue ) )
    {
        m_aParagraphText = aParagraphText;
        notifyEvent(::css::accessibility::AccessibleEventId::
                    TEXT_CHANGED,
                    aOldValue, aNewValue);
    }
}

void ParagraphImpl::notifyEvent(::sal_Int16 nEventId,
                                ::css::uno::Any const & rOldValue,
                                ::css::uno::Any const & rNewValue)
{
    if (m_nClientId)
        comphelper::AccessibleEventNotifier::addEvent( m_nClientId, ::css::accessibility::AccessibleEventObject(
                             static_cast< ::cppu::OWeakObject * >(this),
                             nEventId, rNewValue, rOldValue) );
}

// virtual
::css::uno::Reference< ::css::accessibility::XAccessibleContext > SAL_CALL
ParagraphImpl::getAccessibleContext() throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return this;
}

// virtual
::sal_Int32 SAL_CALL ParagraphImpl::getAccessibleChildCount()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return 0;
}

// virtual
::css::uno::Reference< ::css::accessibility::XAccessible > SAL_CALL
ParagraphImpl::getAccessibleChild(::sal_Int32)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    throw ::css::lang::IndexOutOfBoundsException(
        "textwindowaccessibility.cxx:"
        " ParagraphImpl::getAccessibleChild",
        static_cast< ::css::uno::XWeak * >(this));
}

// virtual
::css::uno::Reference< ::css::accessibility::XAccessible > SAL_CALL
ParagraphImpl::getAccessibleParent()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return m_xDocument->getAccessible();
}

// virtual
::sal_Int32 SAL_CALL ParagraphImpl::getAccessibleIndexInParent()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return m_xDocument->retrieveParagraphIndex(this);
}

// virtual
::sal_Int16 SAL_CALL ParagraphImpl::getAccessibleRole()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return ::css::accessibility::AccessibleRole::PARAGRAPH;
}

// virtual
OUString SAL_CALL ParagraphImpl::getAccessibleDescription()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return OUString();
}

// virtual
OUString SAL_CALL ParagraphImpl::getAccessibleName()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return OUString();
}

// virtual
::css::uno::Reference< ::css::accessibility::XAccessibleRelationSet >
SAL_CALL ParagraphImpl::getAccessibleRelationSet()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return m_xDocument->retrieveParagraphRelationSet( this );
}

// virtual
::css::uno::Reference< ::css::accessibility::XAccessibleStateSet >
SAL_CALL ParagraphImpl::getAccessibleStateSet()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();

    // FIXME  Notification of changes (STATE_CHANGED) missing when
    // m_rView.IsReadOnly() changes:
    return new ::utl::AccessibleStateSetHelper(
        m_xDocument->retrieveParagraphState(this));
}

// virtual
::css::lang::Locale SAL_CALL ParagraphImpl::getLocale()
    throw (::css::accessibility::IllegalAccessibleComponentStateException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    return m_xDocument->retrieveLocale();
}

// virtual
::sal_Bool SAL_CALL ParagraphImpl::containsPoint(::css::awt::Point const & rPoint)
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    ::css::awt::Rectangle aRect(m_xDocument->retrieveParagraphBounds(this,
                                                                     false));
    return rPoint.X >= 0 && rPoint.X < aRect.Width
        && rPoint.Y >= 0 && rPoint.Y < aRect.Height;
}

// virtual
::css::uno::Reference< ::css::accessibility::XAccessible > SAL_CALL
ParagraphImpl::getAccessibleAtPoint(::css::awt::Point const &)
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return 0;
}

// virtual
::css::awt::Rectangle SAL_CALL ParagraphImpl::getBounds()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return m_xDocument->retrieveParagraphBounds(this, false);
}

// virtual
::css::awt::Point SAL_CALL ParagraphImpl::getLocation()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    ::css::awt::Rectangle aRect(m_xDocument->retrieveParagraphBounds(this,
                                                                     false));
    return ::css::awt::Point(aRect.X, aRect.Y);
}

// virtual
::css::awt::Point SAL_CALL ParagraphImpl::getLocationOnScreen()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    ::css::awt::Rectangle aRect(m_xDocument->retrieveParagraphBounds(this,
                                                                     true));
    return ::css::awt::Point(aRect.X, aRect.Y);
}

// virtual
::css::awt::Size SAL_CALL ParagraphImpl::getSize()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    ::css::awt::Rectangle aRect(m_xDocument->retrieveParagraphBounds(this,
                                                                     false));
    return ::css::awt::Size(aRect.Width, aRect.Height);
}

// virtual
void SAL_CALL ParagraphImpl::grabFocus() throw (::css::uno::RuntimeException)
{
    checkDisposed();
    Window* pWindow = m_xDocument->GetWindow();
    if ( pWindow )
    {
        pWindow->GrabFocus();
    }
    try
    {
        m_xDocument->changeParagraphSelection(this, 0, 0);
    }
    catch (const ::css::lang::IndexOutOfBoundsException & rEx)
    {
        OSL_TRACE(
            "textwindowaccessibility.cxx: ParagraphImpl::grabFocus:"
            " caught unexpected %s\n",
            OUStringToOString(rEx.Message, RTL_TEXTENCODING_UTF8).
            getStr());
    }
}

// virtual
::css::uno::Any SAL_CALL ParagraphImpl::getAccessibleKeyBinding()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return ::css::uno::Any();
}

// virtual
::css::util::Color SAL_CALL ParagraphImpl::getForeground()
    throw (::css::uno::RuntimeException)
{
    return 0; // TODO
}

// virtual
::css::util::Color SAL_CALL ParagraphImpl::getBackground()
    throw (::css::uno::RuntimeException)
{
    return 0; // TODO
}

// virtual
::sal_Int32 SAL_CALL ParagraphImpl::getCaretPosition()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return m_xDocument->retrieveParagraphCaretPosition(this);
}

// virtual
::sal_Bool SAL_CALL ParagraphImpl::setCaretPosition(::sal_Int32 nIndex)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    m_xDocument->changeParagraphSelection(this, nIndex, nIndex);
    return true;
}

// virtual
::sal_Unicode SAL_CALL ParagraphImpl::getCharacter(::sal_Int32 nIndex)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    return OCommonAccessibleText::getCharacter(nIndex);
}

// virtual
::css::uno::Sequence< ::css::beans::PropertyValue > SAL_CALL
ParagraphImpl::getCharacterAttributes(::sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< OUString >& aRequestedAttributes)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    return m_xDocument->retrieveCharacterAttributes( this, nIndex, aRequestedAttributes );
}

// virtual
::css::awt::Rectangle SAL_CALL
ParagraphImpl::getCharacterBounds(::sal_Int32 nIndex)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    ::css::awt::Rectangle aBounds(m_xDocument->retrieveCharacterBounds(this, nIndex));
    ::css::awt::Rectangle aParaBounds(m_xDocument->retrieveParagraphBounds(this, false));
    aBounds.X -= aParaBounds.X;
    aBounds.Y -= aParaBounds.Y;
    return aBounds;
}

// virtual
::sal_Int32 SAL_CALL ParagraphImpl::getCharacterCount()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return OCommonAccessibleText::getCharacterCount();
}

// virtual
::sal_Int32 SAL_CALL
ParagraphImpl::getIndexAtPoint(::css::awt::Point const & rPoint)
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    ::css::awt::Point aPoint(rPoint);
    ::css::awt::Rectangle aParaBounds(m_xDocument->retrieveParagraphBounds(this, false));
    aPoint.X += aParaBounds.X;
    aPoint.Y += aParaBounds.Y;
    return m_xDocument->retrieveCharacterIndex(this, aPoint);
}

// virtual
OUString SAL_CALL ParagraphImpl::getSelectedText()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();

    return OCommonAccessibleText::getSelectedText();
}

// virtual
::sal_Int32 SAL_CALL ParagraphImpl::getSelectionStart()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return OCommonAccessibleText::getSelectionStart();
}

// virtual
::sal_Int32 SAL_CALL ParagraphImpl::getSelectionEnd()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return OCommonAccessibleText::getSelectionEnd();
}

// virtual
::sal_Bool SAL_CALL ParagraphImpl::setSelection(::sal_Int32 nStartIndex,
                                                ::sal_Int32 nEndIndex)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    m_xDocument->changeParagraphSelection(this, nStartIndex, nEndIndex);
    return true;
}

// virtual
OUString SAL_CALL ParagraphImpl::getText()
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return OCommonAccessibleText::getText();
}

// virtual
OUString SAL_CALL ParagraphImpl::getTextRange(::sal_Int32 nStartIndex,
                                                     ::sal_Int32 nEndIndex)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    return OCommonAccessibleText::getTextRange(nStartIndex, nEndIndex);
}

// virtual
::com::sun::star::accessibility::TextSegment SAL_CALL ParagraphImpl::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    checkDisposed();
    return OCommonAccessibleText::getTextAtIndex(nIndex, aTextType);
}

// virtual
::com::sun::star::accessibility::TextSegment SAL_CALL ParagraphImpl::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    checkDisposed();
    return OCommonAccessibleText::getTextBeforeIndex(nIndex, aTextType);
}

// virtual
::com::sun::star::accessibility::TextSegment SAL_CALL ParagraphImpl::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    checkDisposed();
    return OCommonAccessibleText::getTextBehindIndex(nIndex, aTextType);
}

// virtual
::sal_Bool SAL_CALL ParagraphImpl::copyText(::sal_Int32 nStartIndex,
                                            ::sal_Int32 nEndIndex)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    m_xDocument->copyParagraphText(this, nStartIndex, nEndIndex);
    return true;
}

// virtual
::sal_Bool SAL_CALL ParagraphImpl::cutText(::sal_Int32 nStartIndex,
                                           ::sal_Int32 nEndIndex)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    m_xDocument->changeParagraphText(this, nStartIndex, nEndIndex, true, false,
                                     OUString());
    return true;
}

// virtual
::sal_Bool SAL_CALL ParagraphImpl::pasteText(::sal_Int32 nIndex)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    m_xDocument->changeParagraphText(this, nIndex, nIndex, false, true,
                                     OUString());
    return true;
}

// virtual
::sal_Bool SAL_CALL ParagraphImpl::deleteText(::sal_Int32 nStartIndex,
                                          ::sal_Int32 nEndIndex)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    m_xDocument->changeParagraphText(this, nStartIndex, nEndIndex, false, false,
                                     OUString());
    return true;
}

// virtual
::sal_Bool SAL_CALL ParagraphImpl::insertText(OUString const & rText,
                                              ::sal_Int32 nIndex)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    m_xDocument->changeParagraphText(this, nIndex, nIndex, false, false, rText);
    return true;
}

// virtual
::sal_Bool SAL_CALL
ParagraphImpl::replaceText(::sal_Int32 nStartIndex, ::sal_Int32 nEndIndex,
                           OUString const & rReplacement)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    m_xDocument->changeParagraphText(this, nStartIndex, nEndIndex, false, false,
                                     rReplacement);
    return true;
}

// virtual
::sal_Bool SAL_CALL ParagraphImpl::setAttributes(
    ::sal_Int32 nStartIndex, ::sal_Int32 nEndIndex,
    ::css::uno::Sequence< ::css::beans::PropertyValue > const & rAttributeSet)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    m_xDocument->changeParagraphAttributes(this, nStartIndex, nEndIndex,
                                           rAttributeSet);
    return true;
}

// virtual
::sal_Bool SAL_CALL ParagraphImpl::setText(OUString const & rText)
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    m_xDocument->changeParagraphText(this, rText);
    return true;
}

// virtual
::css::uno::Sequence< ::css::beans::PropertyValue > SAL_CALL
ParagraphImpl::getDefaultAttributes(const ::css::uno::Sequence< OUString >& RequestedAttributes)
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return m_xDocument->retrieveDefaultAttributes( this, RequestedAttributes );
}

// virtual
::css::uno::Sequence< ::css::beans::PropertyValue > SAL_CALL
ParagraphImpl::getRunAttributes(::sal_Int32 Index, const ::css::uno::Sequence< OUString >& RequestedAttributes)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();
    return m_xDocument->retrieveRunAttributes( this, Index, RequestedAttributes );
}

// virtual
::sal_Int32 SAL_CALL ParagraphImpl::getLineNumberAtIndex( ::sal_Int32 nIndex )
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();

    ::sal_Int32 nLineNo = -1;
    m_xDocument->retrieveParagraphLineBoundary( this, nIndex, &nLineNo );

    return nLineNo;
}

// virtual
::css::accessibility::TextSegment SAL_CALL ParagraphImpl::getTextAtLineNumber( ::sal_Int32 nLineNo )
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    checkDisposed();

    ::css::i18n::Boundary aBoundary =
        m_xDocument->retrieveParagraphBoundaryOfLine( this, nLineNo );

    return ::css::accessibility::TextSegment( getTextRange(aBoundary.startPos, aBoundary.endPos),
        aBoundary.startPos, aBoundary.endPos);
}

// virtual
::css::accessibility::TextSegment SAL_CALL ParagraphImpl::getTextAtLineWithCaret(  )
    throw (::css::uno::RuntimeException)
{
    checkDisposed();

    sal_Int32 nLineNo = getNumberOfLineWithCaret();

    try {
        return ( nLineNo >= 0 ) ?
            getTextAtLineNumber( nLineNo ) :
            ::css::accessibility::TextSegment();
    } catch (const ::css::lang::IndexOutOfBoundsException&) {
        throw ::css::uno::RuntimeException(
            "textwindowaccessibility.cxx:"
            " ParagraphImpl::getTextAtLineWithCaret",
            static_cast< ::css::uno::XWeak * >( this ) );
    }
}

// virtual
::sal_Int32 SAL_CALL ParagraphImpl::getNumberOfLineWithCaret(  )
    throw (::css::uno::RuntimeException)
{
    checkDisposed();
    return m_xDocument->retrieveParagraphLineWithCursor(this);
}


// virtual
void SAL_CALL ParagraphImpl::addEventListener(
    ::css::uno::Reference<
    ::css::accessibility::XAccessibleEventListener > const & rListener)
    throw (::css::uno::RuntimeException)
{
    if (rListener.is())
    {
        ::osl::ClearableMutexGuard aGuard(rBHelper.rMutex);
        if (rBHelper.bDisposed || rBHelper.bInDispose)
        {
            aGuard.clear();
            rListener->disposing(::css::lang::EventObject(
                                    static_cast< ::cppu::OWeakObject * >(this)));
        }
        else
        {
            if (!m_nClientId)
                m_nClientId = comphelper::AccessibleEventNotifier::registerClient( );
            comphelper::AccessibleEventNotifier::addEventListener( m_nClientId, rListener );
        }
    }
}

// virtual
void SAL_CALL ParagraphImpl::removeEventListener(
    ::css::uno::Reference<
    ::css::accessibility::XAccessibleEventListener > const & rListener)
    throw (::css::uno::RuntimeException)
{
    comphelper::AccessibleEventNotifier::TClientId nId = 0;
    {
        ::osl::ClearableMutexGuard aGuard(rBHelper.rMutex);
        if (rListener.is() && m_nClientId != 0
            && comphelper::AccessibleEventNotifier::removeEventListener( m_nClientId, rListener ) == 0)
        {
            nId = m_nClientId;
            m_nClientId = 0;
        }
    }
    if (nId != 0)
    {
        // no listeners anymore
        // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
        // and at least to us not firing any events anymore, in case somebody calls
        // NotifyAccessibleEvent, again
        comphelper::AccessibleEventNotifier::revokeClient(nId);
    }
}

// virtual
void SAL_CALL ParagraphImpl::disposing()
{
    comphelper::AccessibleEventNotifier::TClientId nId = 0;
    {
        ::osl::ClearableMutexGuard aGuard(rBHelper.rMutex);
        nId = m_nClientId;
        m_nClientId = 0;
    }
    if (nId != 0)
        comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing(nId, *this);
}

// virtual
OUString ParagraphImpl::implGetText()
{
    return m_xDocument->retrieveParagraphText(this);
}

// virtual
::css::lang::Locale ParagraphImpl::implGetLocale()
{
    return m_xDocument->retrieveLocale();
}

// virtual
void ParagraphImpl::implGetSelection(::sal_Int32 & rStartIndex,
                                     ::sal_Int32 & rEndIndex)
{
    m_xDocument->retrieveParagraphSelection(this, &rStartIndex, &rEndIndex);
}

// virtual
void ParagraphImpl::implGetParagraphBoundary( ::css::i18n::Boundary& rBoundary,
                                              ::sal_Int32 nIndex )
{
    OUString sText( implGetText() );
    ::sal_Int32 nLength = sText.getLength();

    if ( implIsValidIndex( nIndex, nLength ) )
    {
        rBoundary.startPos = 0;
        rBoundary.endPos = nLength;
    }
    else
    {
        rBoundary.startPos = nIndex;
        rBoundary.endPos = nIndex;
    }
}

// virtual
void ParagraphImpl::implGetLineBoundary( ::css::i18n::Boundary& rBoundary,
                                         ::sal_Int32 nIndex )
{
    OUString sText( implGetText() );
    ::sal_Int32 nLength = sText.getLength();

    if ( implIsValidIndex( nIndex, nLength ) || nIndex == nLength )
    {
        ::css::i18n::Boundary aBoundary =
            m_xDocument->retrieveParagraphLineBoundary( this, nIndex );
        rBoundary.startPos = aBoundary.startPos;
        rBoundary.endPos = aBoundary.endPos;
    }
    else
    {
        rBoundary.startPos = nIndex;
        rBoundary.endPos = nIndex;
    }
}


void ParagraphImpl::checkDisposed()
{
    ::osl::MutexGuard aGuard(rBHelper.rMutex);
    if (!(rBHelper.bDisposed || rBHelper.bInDispose))
        return;
    throw ::css::lang::DisposedException(
        OUString(), static_cast< ::css::uno::XWeak * >(this));
}

Document::Document(::VCLXWindow * pVclXWindow, ::TextEngine & rEngine,
                   ::TextView & rView, bool bCompoundControlChild):
    VCLXAccessibleComponent(pVclXWindow),
    m_xAccessible(pVclXWindow),
    m_rEngine(rEngine),
    m_rView(rView),
    m_aEngineListener(*this),
    m_aViewListener(LINK(this, Document, WindowEventHandler)),
    m_bCompoundControlChild(bCompoundControlChild)
{}

::css::lang::Locale Document::retrieveLocale()
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    return m_rEngine.GetLocale();
}

::sal_Int32 Document::retrieveParagraphIndex(ParagraphImpl const * pParagraph)
{
    ::osl::MutexGuard aInternalGuard(GetMutex());

    // If a client holds on to a Paragraph that is no longer visible, it can
    // happen that this Paragraph lies outside the range from m_aVisibleBegin
    // to m_aVisibleEnd.  In that case, return -1 instead of a valid index:
    Paragraphs::iterator aPara(m_xParagraphs->begin()
                               + pParagraph->getNumber());
    return aPara < m_aVisibleBegin || aPara >= m_aVisibleEnd
        ? -1 : static_cast< ::sal_Int32 >(aPara - m_aVisibleBegin);
        // XXX  numeric overflow
}

::sal_Int64 Document::retrieveParagraphState(ParagraphImpl const * pParagraph)
{
    ::osl::MutexGuard aInternalGuard(GetMutex());

    // If a client holds on to a Paragraph that is no longer visible, it can
    // happen that this Paragraph lies outside the range from m_aVisibleBegin
    // to m_aVisibleEnd.  In that case, it is neither VISIBLE nor SHOWING:
    ::sal_Int64 nState
          = (static_cast< ::sal_Int64 >(1)
             << ::css::accessibility::AccessibleStateType::ENABLED)
          | (static_cast< ::sal_Int64 >(1)
             << ::css::accessibility::AccessibleStateType::SENSITIVE)
          | (static_cast< ::sal_Int64 >(1)
             << ::css::accessibility::AccessibleStateType::FOCUSABLE)
          | (static_cast< ::sal_Int64 >(1)
             << ::css::accessibility::AccessibleStateType::MULTI_LINE);
    if (!m_rView.IsReadOnly())
        nState |= (static_cast< ::sal_Int64 >(1)
                   << ::css::accessibility::AccessibleStateType::EDITABLE);
    Paragraphs::iterator aPara(m_xParagraphs->begin()
                               + pParagraph->getNumber());
    if (aPara >= m_aVisibleBegin && aPara < m_aVisibleEnd)
    {
        nState
            |= (static_cast< ::sal_Int64 >(1)
                << ::css::accessibility::AccessibleStateType::VISIBLE)
            | (static_cast< ::sal_Int64 >(1)
               << ::css::accessibility::AccessibleStateType::SHOWING);
        if (aPara == m_aFocused)
            nState |= (static_cast< ::sal_Int64 >(1)
                       << ::css::accessibility::AccessibleStateType::FOCUSED);
    }
    return nState;
};

::css::awt::Rectangle
Document::retrieveParagraphBounds(ParagraphImpl const * pParagraph,
                                  bool bAbsolute)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    ::osl::MutexGuard aInternalGuard(GetMutex());

    // If a client holds on to a Paragraph that is no longer visible (as it
    // scrolled out the top of the view), it can happen that this Paragraph
    // lies before m_aVisibleBegin.  In that case, calculate the vertical
    // position of the Paragraph starting at paragraph 0, otherwise optimize
    // and start at m_aVisibleBegin:
    Paragraphs::iterator aPara(m_xParagraphs->begin()
                               + pParagraph->getNumber());
    ::sal_Int32 nPos;
    Paragraphs::iterator aIt;
    if (aPara < m_aVisibleBegin)
    {
        nPos = 0;
        aIt = m_xParagraphs->begin();
    }
    else
    {
        nPos = m_nViewOffset - m_nVisibleBeginOffset;
        aIt = m_aVisibleBegin;
    }
    for (; aIt != aPara; ++aIt)
        nPos += aIt->getHeight();

    Point aOrig(0, 0);
    if (bAbsolute)
        aOrig = m_rView.GetWindow()->OutputToAbsoluteScreenPixel(aOrig);

    return ::css::awt::Rectangle(
        static_cast< ::sal_Int32 >(aOrig.X()),
        static_cast< ::sal_Int32 >(aOrig.Y()) + nPos - m_nViewOffset,
        m_rView.GetWindow()->GetOutputSizePixel().Width(), aPara->getHeight());
        // XXX  numeric overflow (3x)
}

OUString
Document::retrieveParagraphText(ParagraphImpl const * pParagraph)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    ::osl::MutexGuard aInternalGuard(GetMutex());
    return m_rEngine.GetText(static_cast< ::sal_uLong >(pParagraph->getNumber()));
        // numeric overflow cannot happen here
}

void Document::retrieveParagraphSelection(ParagraphImpl const * pParagraph,
                                          ::sal_Int32 * pBegin,
                                          ::sal_Int32 * pEnd)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    ::osl::MutexGuard aInternalGuard(GetMutex());
    ::TextSelection const & rSelection = m_rView.GetSelection();
    Paragraphs::size_type nNumber = pParagraph->getNumber();
    TextPaM aStartPaM( rSelection.GetStart() );
    TextPaM aEndPaM( rSelection.GetEnd() );
    TextPaM aMinPaM( ::std::min( aStartPaM, aEndPaM ) );
    TextPaM aMaxPaM( ::std::max( aStartPaM, aEndPaM ) );

    if ( nNumber >= aMinPaM.GetPara() && nNumber <= aMaxPaM.GetPara() )
    {
        *pBegin = nNumber > aMinPaM.GetPara()
            ? 0
            : static_cast< ::sal_Int32 >( aMinPaM.GetIndex() );
            // XXX numeric overflow
        *pEnd = nNumber < aMaxPaM.GetPara()
            ? static_cast< ::sal_Int32 >( m_rEngine.GetText(static_cast< ::sal_uLong >(nNumber)).Len() )
            : static_cast< ::sal_Int32 >( aMaxPaM.GetIndex() );
            // XXX  numeric overflow (3x)

        if ( aStartPaM > aEndPaM )
            ::std::swap( *pBegin, *pEnd );
    }
    else
    {
        *pBegin = 0;
        *pEnd = 0;
    }
}

::sal_Int32 Document::retrieveParagraphCaretPosition(ParagraphImpl const * pParagraph)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    ::osl::MutexGuard aInternalGuard(GetMutex());
    ::TextSelection const & rSelection = m_rView.GetSelection();
    Paragraphs::size_type nNumber = pParagraph->getNumber();
    TextPaM aEndPaM( rSelection.GetEnd() );

    return aEndPaM.GetPara() == nNumber
        ? static_cast< ::sal_Int32 >(aEndPaM.GetIndex()) : -1;
}

::css::awt::Rectangle
Document::retrieveCharacterBounds(ParagraphImpl const * pParagraph,
                                  ::sal_Int32 nIndex)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    ::osl::MutexGuard aInternalGuard(GetMutex());
    ::sal_uLong nNumber = static_cast< ::sal_uLong >(pParagraph->getNumber());
    sal_Int32 nLength = m_rEngine.GetText(nNumber).Len();
        // XXX  numeric overflow
    if (nIndex < 0 || nIndex > nLength)
        throw ::css::lang::IndexOutOfBoundsException(
            "textwindowaccessibility.cxx:"
            " Document::retrieveCharacterAttributes",
            static_cast< ::css::uno::XWeak * >(this));
    ::css::awt::Rectangle aBounds( 0, 0, 0, 0 );
    if ( nIndex == nLength )
    {
        aBounds = AWTRectangle(
            m_rEngine.PaMtoEditCursor(::TextPaM(nNumber,
                                                static_cast< ::sal_uInt16 >(nIndex))));
    }
    else
    {
        ::Rectangle aLeft(
            m_rEngine.PaMtoEditCursor(::TextPaM(nNumber,
                                                static_cast< ::sal_uInt16 >(nIndex))));
            // XXX  numeric overflow
        ::Rectangle aRight(
            m_rEngine.PaMtoEditCursor(::TextPaM(nNumber,
                                                static_cast< ::sal_uInt16 >(nIndex)
                                                + 1)));
            // XXX  numeric overflow (2x)
        // FIXME  If the vertical extends of the two cursors do not match, assume
        // nIndex is the last character on the line; the bounding box will then
        // extend to m_rEnginge.GetMaxTextWidth():
        ::sal_Int32 nWidth = (aLeft.Top() == aRight.Top()
                            && aLeft.Bottom() == aRight.Bottom())
            ? static_cast< ::sal_Int32 >(aRight.Left() - aLeft.Left())
            : static_cast< ::sal_Int32 >(m_rEngine.GetMaxTextWidth()
                                        - aLeft.Left());
            // XXX  numeric overflow (4x)
        aBounds = ::css::awt::Rectangle(static_cast< ::sal_Int32 >(aLeft.Left()),
                                        static_cast< ::sal_Int32 >(aLeft.Top() - m_nViewOffset),
                                        nWidth,
                                        static_cast< ::sal_Int32 >(aLeft.Bottom()
                                                                    - aLeft.Top()));
            // XXX  numeric overflow (4x)
    }
    return aBounds;
}

::sal_Int32 Document::retrieveCharacterIndex(ParagraphImpl const * pParagraph,
                                             ::css::awt::Point const & rPoint)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    ::osl::MutexGuard aInternalGuard(GetMutex());
    ::sal_uLong nNumber = static_cast< ::sal_uLong >(pParagraph->getNumber());
        // XXX  numeric overflow
    ::TextPaM aPaM(m_rEngine.GetPaM(::Point(static_cast< long >(rPoint.X),
                                            static_cast< long >(rPoint.Y))));
        // XXX  numeric overflow (2x)
    return aPaM.GetPara() == nNumber
        ? static_cast< ::sal_Int32 >(aPaM.GetIndex()) : -1;
        // XXX  numeric overflow
}

::css::uno::Sequence< ::css::beans::PropertyValue >
Document::retrieveCharacterAttributes(
    ParagraphImpl const * pParagraph, ::sal_Int32 nIndex,
    const ::css::uno::Sequence< OUString >& aRequestedAttributes)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    ::osl::MutexGuard aInternalGuard(GetMutex());
    ::sal_uLong nNumber = static_cast< ::sal_uLong >(pParagraph->getNumber());
        // XXX  numeric overflow
    if (nIndex < 0 || nIndex >= m_rEngine.GetText(nNumber).Len())
        throw ::css::lang::IndexOutOfBoundsException(
            "textwindowaccessibility.cxx:"
            " Document::retrieveCharacterAttributes",
            static_cast< ::css::uno::XWeak * >(this));

    // retrieve default attributes
    tPropValMap aCharAttrSeq;
    retrieveDefaultAttributesImpl( pParagraph, aRequestedAttributes, aCharAttrSeq );

    // retrieve run attributes
    tPropValMap aRunAttrSeq;
    retrieveRunAttributesImpl( pParagraph, nIndex, aRequestedAttributes, aRunAttrSeq );

    // merge default and run attributes
    for ( tPropValMap::const_iterator aRunIter  = aRunAttrSeq.begin();
          aRunIter != aRunAttrSeq.end();
          ++aRunIter )
    {
        aCharAttrSeq[ aRunIter->first ] = aRunIter->second;
    }

    return convertHashMapToSequence( aCharAttrSeq );
}

void Document::retrieveDefaultAttributesImpl(
    ParagraphImpl const * pParagraph,
    const ::css::uno::Sequence< OUString >& RequestedAttributes,
    tPropValMap& rDefAttrSeq)
{
    // default attributes are not supported by text engine
    (void) pParagraph;
    (void) RequestedAttributes;
    (void) rDefAttrSeq;
}

::css::uno::Sequence< ::css::beans::PropertyValue >
Document::retrieveDefaultAttributes(
    ParagraphImpl const * pParagraph,
    const ::css::uno::Sequence< OUString >& RequestedAttributes)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard( getExternalLock() );
    ::osl::MutexGuard aInternalGuard( GetMutex() );

    tPropValMap aDefAttrSeq;
    retrieveDefaultAttributesImpl( pParagraph, RequestedAttributes, aDefAttrSeq );
    return convertHashMapToSequence( aDefAttrSeq );
}

// static
::css::uno::Sequence< ::css::beans::PropertyValue >
Document::convertHashMapToSequence(tPropValMap& rAttrSeq)
{
    ::css::uno::Sequence< ::css::beans::PropertyValue > aValues( rAttrSeq.size() );
    ::css::beans::PropertyValue* pValues = aValues.getArray();
    ::sal_Int32 i = 0;
    for ( tPropValMap::const_iterator aIter  = rAttrSeq.begin();
          aIter != rAttrSeq.end();
          ++aIter )
    {
        pValues[i] = aIter->second;
        ++i;
    }
    return aValues;
}

void Document::retrieveRunAttributesImpl(
    ParagraphImpl const * pParagraph, ::sal_Int32 Index,
    const ::css::uno::Sequence< OUString >& RequestedAttributes,
    tPropValMap& rRunAttrSeq)
{
    ::sal_uLong nNumber = static_cast< ::sal_uLong >( pParagraph->getNumber() );
    ::TextPaM aPaM( nNumber, static_cast< ::sal_uInt16 >( Index ) );
        // XXX  numeric overflow
    // FIXME  TEXTATTR_HYPERLINK ignored:
    ::TextAttribFontColor const * pColor
          = static_cast< ::TextAttribFontColor const * >(
              m_rEngine.FindAttrib( aPaM, TEXTATTR_FONTCOLOR ) );
    ::TextAttribFontWeight const * pWeight
          = static_cast< ::TextAttribFontWeight const * >(
              m_rEngine.FindAttrib( aPaM, TEXTATTR_FONTWEIGHT ) );
    tPropValMap aRunAttrSeq;
    if ( pColor )
    {
        ::css::beans::PropertyValue aPropVal;
        aPropVal.Name = "CharColor";
        aPropVal.Handle = -1;
        aPropVal.Value = mapFontColor( pColor->GetColor() );
        aPropVal.State = ::css::beans::PropertyState_DIRECT_VALUE;
        aRunAttrSeq[ aPropVal.Name ] = aPropVal;
    }
    if ( pWeight )
    {
        ::css::beans::PropertyValue aPropVal;
        aPropVal.Name = "CharWeight";
        aPropVal.Handle = -1;
        aPropVal.Value = mapFontWeight( pWeight->getFontWeight() );
        aPropVal.State = ::css::beans::PropertyState_DIRECT_VALUE;
        aRunAttrSeq[ aPropVal.Name ] = aPropVal;
    }
    if ( RequestedAttributes.getLength() == 0 )
    {
        rRunAttrSeq = aRunAttrSeq;
    }
    else
    {
        const OUString* pReqAttrs = RequestedAttributes.getConstArray();
        const ::sal_Int32 nLength = RequestedAttributes.getLength();
        for ( ::sal_Int32 i = 0; i < nLength; ++i )
        {
            tPropValMap::iterator aIter = aRunAttrSeq.find( pReqAttrs[i] );
            if ( aIter != aRunAttrSeq.end() )
            {
                rRunAttrSeq[ (*aIter).first ] = (*aIter).second;
            }
        }
    }
}

::css::uno::Sequence< ::css::beans::PropertyValue >
Document::retrieveRunAttributes(
    ParagraphImpl const * pParagraph, ::sal_Int32 Index,
    const ::css::uno::Sequence< OUString >& RequestedAttributes)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard( getExternalLock() );
    ::osl::MutexGuard aInternalGuard( GetMutex() );
    ::sal_uLong nNumber = static_cast< ::sal_uLong >( pParagraph->getNumber() );
        // XXX  numeric overflow
    if ( Index < 0 || Index >= m_rEngine.GetText(nNumber).Len() )
        throw ::css::lang::IndexOutOfBoundsException(
            "textwindowaccessibility.cxx:"
            " Document::retrieveRunAttributes",
            static_cast< ::css::uno::XWeak * >( this ) );

    tPropValMap aRunAttrSeq;
    retrieveRunAttributesImpl( pParagraph, Index, RequestedAttributes, aRunAttrSeq );
    return convertHashMapToSequence( aRunAttrSeq );
}

void Document::changeParagraphText(ParagraphImpl * pParagraph,
                                   OUString const & rText)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    {
        ::osl::MutexGuard aInternalGuard(GetMutex());
        ::sal_uLong nNumber = static_cast< ::sal_uLong >(pParagraph->getNumber());
            // XXX  numeric overflow
        changeParagraphText(nNumber, 0, m_rEngine.GetTextLen(nNumber), false,
                            false, rText);
    }
}

void Document::changeParagraphText(ParagraphImpl * pParagraph,
                                   ::sal_Int32 nBegin, ::sal_Int32 nEnd,
                                   bool bCut, bool bPaste,
                                   OUString const & rText)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    {
        ::osl::MutexGuard aInternalGuard(GetMutex());
        ::sal_uLong nNumber = static_cast< ::sal_uLong >(pParagraph->getNumber());
            // XXX  numeric overflow
        if (nBegin < 0 || nBegin > nEnd
            || nEnd > m_rEngine.GetText(nNumber).Len())
            throw ::css::lang::IndexOutOfBoundsException(
                "textwindowaccessibility.cxx:"
                " Document::changeParagraphText",
                static_cast< ::css::uno::XWeak * >(this));
        changeParagraphText(nNumber, static_cast< ::sal_uInt16 >(nBegin),
                            static_cast< ::sal_uInt16 >(nEnd), bCut, bPaste, rText);
            // XXX  numeric overflow (2x)
    }
}

void Document::copyParagraphText(ParagraphImpl const * pParagraph,
                                 ::sal_Int32 nBegin, ::sal_Int32 nEnd)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    {
        ::osl::MutexGuard aInternalGuard(GetMutex());
        ::sal_uLong nNumber = static_cast< ::sal_uLong >(pParagraph->getNumber());
            // XXX  numeric overflow
        if (nBegin < 0 || nBegin > nEnd
            || nEnd > m_rEngine.GetText(nNumber).Len())
            throw ::css::lang::IndexOutOfBoundsException(
                "textwindowaccessibility.cxx:"
                " Document::copyParagraphText",
                static_cast< ::css::uno::XWeak * >(this));
        m_rView.SetSelection(
            ::TextSelection(::TextPaM(nNumber, static_cast< ::sal_uInt16 >(nBegin)),
                            ::TextPaM(nNumber, static_cast< ::sal_uInt16 >(nEnd))));
            // XXX  numeric overflow (2x)
        m_rView.Copy();
    }
}

void Document::changeParagraphAttributes(
    ParagraphImpl * pParagraph, ::sal_Int32 nBegin, ::sal_Int32 nEnd,
    ::css::uno::Sequence< ::css::beans::PropertyValue > const & rAttributeSet)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    {
        ::osl::MutexGuard aInternalGuard(GetMutex());
        ::sal_uLong nNumber = static_cast< ::sal_uLong >(pParagraph->getNumber());
        // XXX  numeric overflow
        if (nBegin < 0 || nBegin > nEnd
            || nEnd > m_rEngine.GetText(nNumber).Len())
            throw ::css::lang::IndexOutOfBoundsException(
                "textwindowaccessibility.cxx:"
                " Document::changeParagraphAttributes",
                static_cast< ::css::uno::XWeak * >(this));

        // FIXME  The new attributes are added to any attributes already set,
        // they do not replace the old attributes as required by
        // XAccessibleEditableText.setAttributes:
        for (::sal_Int32 i = 0; i < rAttributeSet.getLength(); ++i)
            if ( rAttributeSet[i].Name == "CharColor" )
                m_rEngine.SetAttrib(::TextAttribFontColor(
                                        mapFontColor(rAttributeSet[i].Value)),
                                    nNumber, static_cast< ::sal_uInt16 >(nBegin),
                                    static_cast< ::sal_uInt16 >(nEnd));
                    // XXX  numeric overflow (2x)
            else if ( rAttributeSet[i].Name == "CharWeight" )
                m_rEngine.SetAttrib(::TextAttribFontWeight(
                                        mapFontWeight(rAttributeSet[i].Value)),
                                    nNumber, static_cast< ::sal_uInt16 >(nBegin),
                                    static_cast< ::sal_uInt16 >(nEnd));
                    // XXX  numeric overflow (2x)
    }
}

void Document::changeParagraphSelection(ParagraphImpl * pParagraph,
                                        ::sal_Int32 nBegin, ::sal_Int32 nEnd)
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    {
        ::osl::MutexGuard aInternalGuard(GetMutex());
        ::sal_uLong nNumber = static_cast< ::sal_uLong >(pParagraph->getNumber());
            // XXX  numeric overflow
        if (nBegin < 0 || nBegin > nEnd
            || nEnd > m_rEngine.GetText(nNumber).Len())
            throw ::css::lang::IndexOutOfBoundsException(
                "textwindowaccessibility.cxx:"
                " Document::changeParagraphSelection",
                static_cast< ::css::uno::XWeak * >(this));
        m_rView.SetSelection(
            ::TextSelection(::TextPaM(nNumber, static_cast< ::sal_uInt16 >(nBegin)),
                            ::TextPaM(nNumber, static_cast< ::sal_uInt16 >(nEnd))));
            // XXX  numeric overflow (2x)
    }
}

::css::i18n::Boundary
Document::retrieveParagraphLineBoundary( ParagraphImpl const * pParagraph,
                                         ::sal_Int32 nIndex, ::sal_Int32 *pLineNo )
{
    ::css::i18n::Boundary aBoundary;
    aBoundary.startPos = nIndex;
    aBoundary.endPos = nIndex;

    ::osl::Guard< ::comphelper::IMutex > aExternalGuard( getExternalLock() );
    {
        ::osl::MutexGuard aInternalGuard( GetMutex() );
        ::sal_uLong nNumber = static_cast< ::sal_uLong >( pParagraph->getNumber() );
        if ( nIndex < 0 || nIndex > m_rEngine.GetText( nNumber ).Len() )
            throw ::css::lang::IndexOutOfBoundsException(
                "textwindowaccessibility.cxx:"
                " Document::retrieveParagraphLineBoundary",
                static_cast< ::css::uno::XWeak * >( this ) );
        ::sal_Int32 nLineStart = 0;
        ::sal_Int32 nLineEnd = 0;
        ::sal_uInt16 nLineCount = m_rEngine.GetLineCount( nNumber );
        for ( ::sal_uInt16 nLine = 0; nLine < nLineCount; ++nLine )
        {
            ::sal_Int32 nLineLength = static_cast< ::sal_Int32 >(
                m_rEngine.GetLineLen( nNumber, nLine ) );
            nLineStart = nLineEnd;
            nLineEnd += nLineLength;
            if ( nIndex >= nLineStart && ( ( nLine == nLineCount - 1 ) ? nIndex <= nLineEnd : nIndex < nLineEnd ) )
            {
                aBoundary.startPos = nLineStart;
                aBoundary.endPos = nLineEnd;
                if( pLineNo )
                    pLineNo[0] = nLine;
                break;
            }
        }
    }

    return aBoundary;
}

::css::i18n::Boundary
Document::retrieveParagraphBoundaryOfLine( ParagraphImpl const * pParagraph,
                                           ::sal_Int32 nLineNo )
{
    ::css::i18n::Boundary aBoundary;
    aBoundary.startPos = 0;
    aBoundary.endPos = 0;

    ::osl::Guard< ::comphelper::IMutex > aExternalGuard( getExternalLock() );
    {
        ::osl::MutexGuard aInternalGuard( GetMutex() );
        ::sal_uLong nNumber = static_cast< ::sal_uLong >( pParagraph->getNumber() );
        if ( nLineNo >= m_rEngine.GetLineCount( nNumber ) )
            throw ::css::lang::IndexOutOfBoundsException(
                "textwindowaccessibility.cxx:"
                " Document::retrieveParagraphBoundaryOfLine",
                static_cast< ::css::uno::XWeak * >( this ) );
        ::sal_Int32 nLineStart = 0;
        ::sal_Int32 nLineEnd = 0;
        for ( ::sal_uInt16 nLine = 0; nLine <= nLineNo; ++nLine )
        {
            ::sal_Int32 nLineLength = static_cast< ::sal_Int32 >(
                m_rEngine.GetLineLen( nNumber, nLine ) );
            nLineStart = nLineEnd;
            nLineEnd += nLineLength;
        }

        aBoundary.startPos = nLineStart;
        aBoundary.endPos = nLineEnd;
    }

    return aBoundary;
}

sal_Int32 Document::retrieveParagraphLineWithCursor( ParagraphImpl const * pParagraph )
{
    ::osl::Guard< ::comphelper::IMutex > aExternalGuard(getExternalLock());
    ::osl::MutexGuard aInternalGuard(GetMutex());
    ::TextSelection const & rSelection = m_rView.GetSelection();
    Paragraphs::size_type nNumber = pParagraph->getNumber();
    TextPaM aEndPaM( rSelection.GetEnd() );

    return aEndPaM.GetPara() == nNumber
        ? m_rView.GetLineNumberOfCursorInSelection() : -1;
}


::css::uno::Reference< ::css::accessibility::XAccessibleRelationSet >
Document::retrieveParagraphRelationSet( ParagraphImpl const * pParagraph )
{
    ::osl::MutexGuard aInternalGuard( GetMutex() );

    ::utl::AccessibleRelationSetHelper* pRelationSetHelper = new ::utl::AccessibleRelationSetHelper();
    ::css::uno::Reference< ::css::accessibility::XAccessibleRelationSet > xSet = pRelationSetHelper;

    Paragraphs::iterator aPara( m_xParagraphs->begin() + pParagraph->getNumber() );

    if ( aPara > m_aVisibleBegin && aPara < m_aVisibleEnd )
    {
        ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XInterface > > aSequence(1);
        aSequence[0] = getAccessibleChild( aPara - 1 );
        ::css::accessibility::AccessibleRelation aRelation( ::css::accessibility::AccessibleRelationType::CONTENT_FLOWS_FROM, aSequence );
        pRelationSetHelper->AddRelation( aRelation );
    }

    if ( aPara >= m_aVisibleBegin && aPara < m_aVisibleEnd -1 )
    {
        ::css::uno::Sequence< ::css::uno::Reference< ::css::uno::XInterface > > aSequence(1);
        aSequence[0] = getAccessibleChild( aPara + 1 );
        ::css::accessibility::AccessibleRelation aRelation( ::css::accessibility::AccessibleRelationType::CONTENT_FLOWS_TO, aSequence );
        pRelationSetHelper->AddRelation( aRelation );
    }

    return xSet;
}

void Document::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_GETFOCUS:
        case VCLEVENT_WINDOW_LOSEFOCUS:
        {
            // #107179# if our parent is a compound control (e.g. MultiLineEdit),
            // suppress the window focus events here
            if ( !m_bCompoundControlChild )
                VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
        }
        break;
        default:
            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
    }
}

// virtual
::sal_Int32 SAL_CALL Document::getAccessibleChildCount()
    throw (::css::uno::RuntimeException)
{
    ::comphelper::OExternalLockGuard aGuard(this);
    init();
    return m_aVisibleEnd - m_aVisibleBegin;
}

// virtual
::css::uno::Reference< ::css::accessibility::XAccessible > SAL_CALL
Document::getAccessibleChild(::sal_Int32 i)
    throw (::css::lang::IndexOutOfBoundsException,
           ::css::uno::RuntimeException)
{
    ::comphelper::OExternalLockGuard aGuard(this);
    init();
    if (i < 0 || i >= m_aVisibleEnd - m_aVisibleBegin)
        throw ::css::lang::IndexOutOfBoundsException(
            "textwindowaccessibility.cxx:"
            " Document::getAccessibleChild",
            static_cast< ::css::uno::XWeak * >(this));
    return getAccessibleChild(m_aVisibleBegin
                              + static_cast< Paragraphs::size_type >(i));
}

// virtual
::sal_Int16 SAL_CALL Document::getAccessibleRole()
    throw (::css::uno::RuntimeException)
{
    return ::css::accessibility::AccessibleRole::TEXT_FRAME;
}

// virtual
::css::uno::Reference< ::css::accessibility::XAccessible > SAL_CALL
Document::getAccessibleAtPoint(::css::awt::Point const & rPoint)
    throw (::css::uno::RuntimeException)
{
    ::comphelper::OExternalLockGuard aGuard(this);
    init();
    if (rPoint.X >= 0
        && rPoint.X < m_rView.GetWindow()->GetOutputSizePixel().Width()
        && rPoint.Y >= 0 && rPoint.Y < m_nViewHeight)
    {
        ::sal_Int32 nOffset = m_nViewOffset + rPoint.Y; // XXX  numeric overflow
        ::sal_Int32 nPos = m_nViewOffset - m_nVisibleBeginOffset;
        for (Paragraphs::iterator aIt(m_aVisibleBegin); aIt != m_aVisibleEnd;
             ++aIt)
        {
            nPos += aIt->getHeight(); // XXX  numeric overflow
            if (nOffset < nPos)
                return getAccessibleChild(aIt);
        }
    }
    return 0;
}

// virtual
void SAL_CALL Document::disposing()
{
    m_aEngineListener.endListening();
    m_aViewListener.endListening();
    if (m_xParagraphs.get() != 0)
        disposeParagraphs();
    VCLXAccessibleComponent::disposing();
}

// virtual
void Document::Notify(::SfxBroadcaster &, ::SfxHint const & rHint)
{
    if (rHint.ISA(::TextHint))
    {
        ::TextHint const & rTextHint
              = static_cast< ::TextHint const & >(rHint);
        switch (rTextHint.GetId())
        {
        case TEXT_HINT_PARAINSERTED:
        case TEXT_HINT_PARAREMOVED:
            // TEXT_HINT_PARAINSERTED and TEXT_HINT_PARAREMOVED are sent at
            // "unsafe" times (when the text engine has not yet re-formatted its
            // content), so that for example calling ::TextEngine::GetTextHeight
            // from within the code that handles TEXT_HINT_PARAINSERTED causes
            // trouble within the text engine.  Therefore, these hints are just
            // buffered until a following ::TextEngine::FormatDoc causes a
            // TEXT_HINT_TEXTFORMATTED to come in:
        case TEXT_HINT_FORMATPARA:
            // ::TextEngine::FormatDoc sends a sequence of
            // TEXT_HINT_FORMATPARAs, followed by an optional
            // TEXT_HINT_TEXTHEIGHTCHANGED, followed in all cases by one
            // TEXT_HINT_TEXTFORMATTED.  Only the TEXT_HINT_FORMATPARAs contain
            // the the numbers of the affected paragraphs, but they are sent
            // before the changes are applied.  Therefore, TEXT_HINT_FORMATPARAs
            // are just buffered until another hint comes in:
            {
                ::osl::MutexGuard aInternalGuard(GetMutex());
                if (!isAlive())
                    break;

                m_aParagraphNotifications.push(rTextHint);
                break;
            }
        case TEXT_HINT_TEXTFORMATTED:
        case TEXT_HINT_TEXTHEIGHTCHANGED:
        case TEXT_HINT_MODIFIED:
            {
                ::osl::MutexGuard aInternalGuard(GetMutex());
                if (!isAlive())
                    break;
                handleParagraphNotifications();
                break;
            }
        case TEXT_HINT_VIEWSCROLLED:
            {
                ::osl::MutexGuard aInternalGuard(GetMutex());
                if (!isAlive())
                    break;
                handleParagraphNotifications();

                ::sal_Int32 nOffset = static_cast< ::sal_Int32 >(
                    m_rView.GetStartDocPos().Y());
                    // XXX  numeric overflow
                if (nOffset != m_nViewOffset)
                {
                    m_nViewOffset = nOffset;

                    Paragraphs::iterator aOldVisibleBegin(
                        m_aVisibleBegin);
                    Paragraphs::iterator aOldVisibleEnd(m_aVisibleEnd);

                    determineVisibleRange();

                    notifyVisibleRangeChanges(aOldVisibleBegin,
                                                aOldVisibleEnd,
                                                m_xParagraphs->end());
                }
                break;
            }
        case TEXT_HINT_VIEWSELECTIONCHANGED:
            {
                ::osl::MutexGuard aInternalGuard(GetMutex());
                if (!isAlive())
                    break;

                if (m_aParagraphNotifications.empty())
                {
                    handleSelectionChangeNotification();
                }
                else
                {
                    // TEXT_HINT_VIEWSELECTIONCHANGED is sometimes sent at
                    // "unsafe" times (when the text engine has not yet re-
                    // formatted its content), so that for example calling
                    // ::TextEngine::GetTextHeight from within the code that
                    // handles a previous TEXT_HINT_PARAINSERTED causes
                    // trouble within the text engine.  Therefore, these
                    // hints are just buffered (along with
                    // TEXT_HINT_PARAINSERTED/REMOVED/FORMATPARA) until a
                    // following ::TextEngine::FormatDoc causes a
                    // TEXT_HINT_TEXTFORMATTED to come in:
                    m_bSelectionChangedNotification = true;
                }
                break;
            }
        }
    }
}

IMPL_LINK(Document, WindowEventHandler, ::VclSimpleEvent *, pEvent)
{
    switch (pEvent->GetId())
    {
    case VCLEVENT_WINDOW_RESIZE:
        {
            ::osl::MutexGuard aInternalGuard(GetMutex());
            if (!isAlive())
                break;

            ::sal_Int32 nHeight = static_cast< ::sal_Int32 >(
                m_rView.GetWindow()->GetOutputSizePixel().Height());
                // XXX  numeric overflow
            if (nHeight != m_nViewHeight)
            {
                m_nViewHeight = nHeight;

                Paragraphs::iterator aOldVisibleBegin(m_aVisibleBegin);
                Paragraphs::iterator aOldVisibleEnd(m_aVisibleEnd);

                determineVisibleRange();

                notifyVisibleRangeChanges(aOldVisibleBegin, aOldVisibleEnd,
                                            m_xParagraphs->end());
            }
            break;
        }
    case VCLEVENT_WINDOW_GETFOCUS:
        {
            ::osl::MutexGuard aInternalGuard(GetMutex());
            if (!isAlive())
                break;

            if (m_aFocused >= m_aVisibleBegin && m_aFocused < m_aVisibleEnd)
            {
                ::rtl::Reference< ParagraphImpl > xParagraph(
                    getParagraph(m_aFocused));
                if (xParagraph.is())
                    xParagraph->notifyEvent(
                        ::css::accessibility::AccessibleEventId::
                        STATE_CHANGED,
                        ::css::uno::Any(),
                        ::css::uno::makeAny(
                            ::css::accessibility::AccessibleStateType::
                            FOCUSED));
            }
            break;
        }
    case VCLEVENT_WINDOW_LOSEFOCUS:
        {
            ::osl::MutexGuard aInternalGuard(GetMutex());
            if (!isAlive())
                break;

            if (m_aFocused >= m_aVisibleBegin && m_aFocused < m_aVisibleEnd)
            {
                ::rtl::Reference< ParagraphImpl > xParagraph(
                    getParagraph(m_aFocused));
                if (xParagraph.is())
                    xParagraph->notifyEvent(
                        ::css::accessibility::AccessibleEventId::
                        STATE_CHANGED,
                        ::css::uno::makeAny(
                            ::css::accessibility::AccessibleStateType::
                            FOCUSED),
                        ::css::uno::Any());
            }
            break;
        }
    }
    return 0;
}

void Document::init()
{
    if (m_xParagraphs.get() == 0)
    {
        ::sal_uLong nCount = m_rEngine.GetParagraphCount();
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr< Paragraphs > p(new Paragraphs);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        p->reserve(static_cast< Paragraphs::size_type >(nCount));
            // numeric overflow is harmless here
        for (::sal_uLong i = 0; i < nCount; ++i)
            p->push_back(ParagraphInfo(static_cast< ::sal_Int32 >(
                                           m_rEngine.GetTextHeight(i))));
                // XXX  numeric overflow
        m_nViewOffset = static_cast< ::sal_Int32 >(
            m_rView.GetStartDocPos().Y()); // XXX  numeric overflow
        m_nViewHeight = static_cast< ::sal_Int32 >(
            m_rView.GetWindow()->GetOutputSizePixel().Height());
            // XXX  numeric overflow
        m_xParagraphs = p;
        determineVisibleRange();
        m_nSelectionFirstPara = -1;
        m_nSelectionFirstPos = -1;
        m_nSelectionLastPara = -1;
        m_nSelectionLastPos = -1;
        m_aFocused = m_xParagraphs->end();
        m_bSelectionChangedNotification = false;
        m_aEngineListener.startListening(m_rEngine);
        m_aViewListener.startListening(*m_rView.GetWindow());
    }
}

::rtl::Reference< ParagraphImpl >
Document::getParagraph(Paragraphs::iterator const & rIt)
{
    return static_cast< ParagraphImpl * >(
        ::css::uno::Reference< ::css::accessibility::XAccessible >(
            rIt->getParagraph()).get());
}

::css::uno::Reference< ::css::accessibility::XAccessible >
Document::getAccessibleChild(Paragraphs::iterator const & rIt)
{
    ::css::uno::Reference< ::css::accessibility::XAccessible > xParagraph(
        rIt->getParagraph());
    if (!xParagraph.is())
    {
        xParagraph = new Paragraph(this, rIt - m_xParagraphs->begin());
        rIt->setParagraph(xParagraph);
    }
    return xParagraph;
}

void Document::determineVisibleRange()
{
    Paragraphs::iterator const aEnd = m_xParagraphs->end();

    m_aVisibleBegin = aEnd;
    m_aVisibleEnd = aEnd;
    m_nVisibleBeginOffset = 0;

    ::sal_Int32 nPos = 0;
    for (Paragraphs::iterator aIt = m_xParagraphs->begin(); m_aVisibleEnd == aEnd && aIt != aEnd; ++aIt)
    {
        ::sal_Int32 const nOldPos = nPos;
        nPos += aIt->getHeight(); // XXX  numeric overflow
        if (m_aVisibleBegin == aEnd)
        {
            if (nPos >= m_nViewOffset)
            {
                m_aVisibleBegin = aIt;
                m_nVisibleBeginOffset = m_nViewOffset - nOldPos;
            }
        }
        else
        {
            if (nPos >= m_nViewOffset + m_nViewHeight) // XXX  numeric overflow
            {
                m_aVisibleEnd = aIt;
            }
        }
    }

    OSL_POSTCOND(
            (m_aVisibleBegin == m_xParagraphs->end() && m_aVisibleEnd == m_xParagraphs->end() && m_nVisibleBeginOffset == 0)
            || (m_aVisibleBegin < m_aVisibleEnd && m_nVisibleBeginOffset >= 0),
            "invalid visible range");
}

void Document::notifyVisibleRangeChanges(
    Paragraphs::iterator const & rOldVisibleBegin,
    Paragraphs::iterator const & rOldVisibleEnd,
    Paragraphs::iterator const & rInserted)
{
    // XXX  Replace this code that determines which paragraphs have changed from
    // invisible to visible or vice versa with a better algorithm.
    {for (Paragraphs::iterator aIt(rOldVisibleBegin); aIt != rOldVisibleEnd;
          ++aIt)
        if (aIt != rInserted
            && (aIt < m_aVisibleBegin || aIt >= m_aVisibleEnd))
            NotifyAccessibleEvent(
                ::css::accessibility::AccessibleEventId::
                CHILD,
                ::css::uno::makeAny(getAccessibleChild(aIt)),
                ::css::uno::Any());
    }
    {for (Paragraphs::iterator aIt(m_aVisibleBegin); aIt != m_aVisibleEnd;
          ++aIt)
        if (aIt == rInserted
            || aIt < rOldVisibleBegin || aIt >= rOldVisibleEnd)
            NotifyAccessibleEvent(
                ::css::accessibility::AccessibleEventId::
                CHILD,
                ::css::uno::Any(),
                ::css::uno::makeAny(getAccessibleChild(aIt)));
    }
}

void
Document::changeParagraphText(::sal_uLong nNumber, ::sal_uInt16 nBegin, ::sal_uInt16 nEnd,
                              bool bCut, bool bPaste,
                              OUString const & rText)
{
    m_rView.SetSelection(::TextSelection(::TextPaM(nNumber, nBegin),
                                         ::TextPaM(nNumber, nEnd)));
    if (bCut)
        m_rView.Cut();
    else if (nBegin != nEnd)
        m_rView.DeleteSelected();
    if (bPaste)
        m_rView.Paste();
    else if (!rText.isEmpty())
        m_rView.InsertText(rText);
}

void Document::handleParagraphNotifications()
{
    while (!m_aParagraphNotifications.empty())
    {
        ::TextHint aHint(m_aParagraphNotifications.front());
        m_aParagraphNotifications.pop();
        switch (aHint.GetId())
        {
        case TEXT_HINT_PARAINSERTED:
            {
                ::sal_uLong n = aHint.GetValue();
                OSL_ENSURE(n <= m_xParagraphs->size(),
                           "bad TEXT_HINT_PARAINSERTED event");

                // Save the values of old iterators (the iterators themselves
                // will get invalidated), and adjust the old values so that they
                // reflect the insertion of the new paragraph:
                Paragraphs::size_type nOldVisibleBegin
                    = m_aVisibleBegin - m_xParagraphs->begin();
                Paragraphs::size_type nOldVisibleEnd
                    = m_aVisibleEnd - m_xParagraphs->begin();
                Paragraphs::size_type nOldFocused
                    = m_aFocused - m_xParagraphs->begin();
                if (n <= nOldVisibleBegin)
                    ++nOldVisibleBegin; // XXX  numeric overflow
                if (n <= nOldVisibleEnd)
                    ++nOldVisibleEnd; // XXX  numeric overflow
                if (n <= nOldFocused)
                    ++nOldFocused; // XXX  numeric overflow
                if (sal::static_int_cast<sal_Int32>(n) <= m_nSelectionFirstPara)
                    ++m_nSelectionFirstPara; // XXX  numeric overflow
                if (sal::static_int_cast<sal_Int32>(n) <= m_nSelectionLastPara)
                    ++m_nSelectionLastPara; // XXX  numeric overflow

                Paragraphs::iterator aIns(
                    m_xParagraphs->insert(
                        m_xParagraphs->begin() + n,
                        ParagraphInfo(static_cast< ::sal_Int32 >(
                                          m_rEngine.GetTextHeight(n)))));
                    // XXX  numeric overflow (2x)

                determineVisibleRange();
                m_aFocused = m_xParagraphs->begin() + nOldFocused;

                for (Paragraphs::iterator aIt(aIns);;)
                {
                    ++aIt;
                    if (aIt == m_xParagraphs->end())
                        break;
                    ::rtl::Reference< ParagraphImpl > xParagraph(
                        getParagraph(aIt));
                    if (xParagraph.is())
                        xParagraph->numberChanged(true);
                }

                notifyVisibleRangeChanges(
                    m_xParagraphs->begin() + nOldVisibleBegin,
                    m_xParagraphs->begin() + nOldVisibleEnd, aIns);
                break;
            }
        case TEXT_HINT_PARAREMOVED:
            {
                ::sal_uLong n = aHint.GetValue();
                if (n == TEXT_PARA_ALL)
                {
                    {for (Paragraphs::iterator aIt(m_aVisibleBegin);
                          aIt != m_aVisibleEnd; ++aIt)
                        NotifyAccessibleEvent(
                            ::css::accessibility::AccessibleEventId::
                            CHILD,
                            ::css::uno::makeAny(getAccessibleChild(aIt)),
                            ::css::uno::Any());
                    }
                    disposeParagraphs();
                    m_xParagraphs->clear();
                    determineVisibleRange();
                    m_nSelectionFirstPara = -1;
                    m_nSelectionFirstPos = -1;
                    m_nSelectionLastPara = -1;
                    m_nSelectionLastPos = -1;
                    m_aFocused = m_xParagraphs->end();
                }
                else
                {
                    OSL_ENSURE(n < m_xParagraphs->size(),
                               "Bad TEXT_HINT_PARAREMOVED event");

                    Paragraphs::iterator aIt(m_xParagraphs->begin() + n);
                        // numeric overflow cannot occur

                    // Save the values of old iterators (the iterators
                    // themselves will get invalidated), and adjust the old
                    // values so that they reflect the removal of the paragraph:
                    Paragraphs::size_type nOldVisibleBegin
                        = m_aVisibleBegin - m_xParagraphs->begin();
                    Paragraphs::size_type nOldVisibleEnd
                        = m_aVisibleEnd - m_xParagraphs->begin();
                    bool bWasVisible
                        = nOldVisibleBegin <= n && n < nOldVisibleEnd;
                    Paragraphs::size_type nOldFocused
                        = m_aFocused - m_xParagraphs->begin();
                    bool bWasFocused = aIt == m_aFocused;
                    if (n < nOldVisibleBegin)
                        --nOldVisibleBegin;
                    if (n < nOldVisibleEnd)
                        --nOldVisibleEnd;
                    if (n < nOldFocused)
                        --nOldFocused;
                    if (sal::static_int_cast<sal_Int32>(n) < m_nSelectionFirstPara)
                        --m_nSelectionFirstPara;
                    else if (sal::static_int_cast<sal_Int32>(n) == m_nSelectionFirstPara)
                    {
                        if (m_nSelectionFirstPara == m_nSelectionLastPara)
                        {
                            m_nSelectionFirstPara = -1;
                            m_nSelectionFirstPos = -1;
                            m_nSelectionLastPara = -1;
                            m_nSelectionLastPos = -1;
                        }
                        else
                        {
                            ++m_nSelectionFirstPara;
                            m_nSelectionFirstPos = 0;
                        }
                    }
                    if (sal::static_int_cast<sal_Int32>(n) < m_nSelectionLastPara)
                        --m_nSelectionLastPara;
                    else if (sal::static_int_cast<sal_Int32>(n) == m_nSelectionLastPara)
                    {
                        OSL_ENSURE(m_nSelectionFirstPara < m_nSelectionLastPara,
                                   "logic error");
                        --m_nSelectionLastPara;
                        m_nSelectionLastPos = 0x7FFFFFFF;
                    }

                    ::css::uno::Reference< ::css::accessibility::XAccessible >
                          xStrong;
                    if (bWasVisible)
                        xStrong = getAccessibleChild(aIt);
                    ::css::uno::WeakReference<
                          ::css::accessibility::XAccessible > xWeak(
                              aIt->getParagraph());
                    aIt = m_xParagraphs->erase(aIt);

                    determineVisibleRange();
                    m_aFocused = bWasFocused ? m_xParagraphs->end()
                        : m_xParagraphs->begin() + nOldFocused;

                    for (; aIt != m_xParagraphs->end(); ++aIt)
                    {
                        ::rtl::Reference< ParagraphImpl > xParagraph(
                            getParagraph(aIt));
                        if (xParagraph.is())
                            xParagraph->numberChanged(false);
                    }

                    if (bWasVisible)
                        NotifyAccessibleEvent(
                            ::css::accessibility::AccessibleEventId::
                            CHILD,
                            ::css::uno::makeAny(xStrong),
                            ::css::uno::Any());

                    ::css::uno::Reference< ::css::lang::XComponent > xComponent(
                        xWeak.get(), ::css::uno::UNO_QUERY);
                    if (xComponent.is())
                        xComponent->dispose();

                    notifyVisibleRangeChanges(
                        m_xParagraphs->begin() + nOldVisibleBegin,
                        m_xParagraphs->begin() + nOldVisibleEnd,
                        m_xParagraphs->end());
                }
                break;
            }
        case TEXT_HINT_FORMATPARA:
            {
                ::sal_uLong n = aHint.GetValue();
                OSL_ENSURE(n < m_xParagraphs->size(),
                           "Bad TEXT_HINT_FORMATPARA event");

                (*m_xParagraphs)[static_cast< Paragraphs::size_type >(n)].
                    changeHeight(static_cast< ::sal_Int32 >(
                                     m_rEngine.GetTextHeight(n)));
                    // XXX  numeric overflow
                Paragraphs::iterator aOldVisibleBegin(m_aVisibleBegin);
                Paragraphs::iterator aOldVisibleEnd(m_aVisibleEnd);
                determineVisibleRange();
                notifyVisibleRangeChanges(aOldVisibleBegin, aOldVisibleEnd,
                                          m_xParagraphs->end());

                if (n < m_xParagraphs->size())
                {
                    Paragraphs::iterator aIt(m_xParagraphs->begin() + n);
                    ::rtl::Reference< ParagraphImpl > xParagraph(getParagraph(aIt));
                    if (xParagraph.is())
                        xParagraph->textChanged();
                }
                break;
            }
        default:
            OSL_FAIL( "bad buffered hint");
            break;
        }
    }
    if (m_bSelectionChangedNotification)
    {
        m_bSelectionChangedNotification = false;
        handleSelectionChangeNotification();
    }
}

void Document::handleSelectionChangeNotification()
{
    ::TextSelection const & rSelection = m_rView.GetSelection();
    OSL_ENSURE(rSelection.GetStart().GetPara() < m_xParagraphs->size()
               && rSelection.GetEnd().GetPara() < m_xParagraphs->size(),
               "bad TEXT_HINT_VIEWSELECTIONCHANGED event");
    ::sal_Int32 nNewFirstPara
          = static_cast< ::sal_Int32 >(rSelection.GetStart().GetPara());
    ::sal_Int32 nNewFirstPos
          = static_cast< ::sal_Int32 >(rSelection.GetStart().GetIndex());
        // XXX  numeric overflow
    ::sal_Int32 nNewLastPara
          = static_cast< ::sal_Int32 >(rSelection.GetEnd().GetPara());
    ::sal_Int32 nNewLastPos
          = static_cast< ::sal_Int32 >(rSelection.GetEnd().GetIndex());
        // XXX  numeric overflow

    // Lose focus:
    Paragraphs::iterator aIt(m_xParagraphs->begin() + nNewLastPara);
    if (m_aFocused != m_xParagraphs->end() && m_aFocused != aIt
        && m_aFocused >= m_aVisibleBegin && m_aFocused < m_aVisibleEnd)
    {
        ::rtl::Reference< ParagraphImpl > xParagraph(getParagraph(m_aFocused));
        if (xParagraph.is())
            xParagraph->notifyEvent(
                ::css::accessibility::AccessibleEventId::
                STATE_CHANGED,
                ::css::uno::makeAny(
                    ::css::accessibility::AccessibleStateType::FOCUSED),
                ::css::uno::Any());
    }

    // Gain focus and update cursor position:
    if (aIt >= m_aVisibleBegin && aIt < m_aVisibleEnd
        && (aIt != m_aFocused
            || nNewLastPara != m_nSelectionLastPara
            || nNewLastPos != m_nSelectionLastPos))
    {
        ::rtl::Reference< ParagraphImpl > xParagraph(getParagraph(aIt));
        if (xParagraph.is())
        {
            if (aIt != m_aFocused)
                xParagraph->notifyEvent(
                    ::css::accessibility::AccessibleEventId::
                    STATE_CHANGED,
                    ::css::uno::Any(),
                    ::css::uno::makeAny(
                        ::css::accessibility::AccessibleStateType::FOCUSED));
            if (nNewLastPara != m_nSelectionLastPara
                || nNewLastPos != m_nSelectionLastPos)
                xParagraph->notifyEvent(
                    ::css::accessibility::AccessibleEventId::
                    CARET_CHANGED,
                    ::css::uno::makeAny< ::sal_Int32 >(
                        nNewLastPara == m_nSelectionLastPara
                        ? m_nSelectionLastPos : 0),
                    ::css::uno::makeAny(nNewLastPos));
        }
    }
    m_aFocused = aIt;

    // Update both old and new selection.  (Regardless of how the two selections
    // look like, there will always be two ranges to the left and right of the
    // overlap---the overlap and/or the range to the right of it possibly being
    // empty.  Only for these two ranges notifications have to be sent.)

    TextPaM aOldTextStart( static_cast< sal_uLong >( m_nSelectionFirstPara ), static_cast< sal_uInt16 >( m_nSelectionFirstPos ) );
    TextPaM aOldTextEnd( static_cast< sal_uLong >( m_nSelectionLastPara ), static_cast< sal_uInt16 >( m_nSelectionLastPos ) );
    TextPaM aNewTextStart( static_cast< sal_uLong >( nNewFirstPara ), static_cast< sal_uInt16 >( nNewFirstPos ) );
    TextPaM aNewTextEnd( static_cast< sal_uLong >( nNewLastPara ), static_cast< sal_uInt16 >( nNewLastPos ) );

    // justify selections
    justifySelection( aOldTextStart, aOldTextEnd );
    justifySelection( aNewTextStart, aNewTextEnd );

    sal_Int32 nFirst1;
    sal_Int32 nLast1;
    sal_Int32 nFirst2;
    sal_Int32 nLast2;

    if ( m_nSelectionFirstPara == -1 )
    {
        // old selection not initialized yet => notify events only for new selection (if not empty)
        nFirst1 = aNewTextStart.GetPara();
        nLast1 = aNewTextEnd.GetPara() + ( aNewTextStart != aNewTextEnd ? 1 : 0 );
        nFirst2 = 0;
        nLast2 = 0;
    }
    else if ( aOldTextStart == aOldTextEnd && aNewTextStart == aNewTextEnd )
    {
        // old and new selection empty => no events
        nFirst1 = 0;
        nLast1 = 0;
        nFirst2 = 0;
        nLast2 = 0;
    }
    else if ( aOldTextStart != aOldTextEnd && aNewTextStart == aNewTextEnd )
    {
        // old selection not empty + new selection empty => notify events only for old selection
        nFirst1 = aOldTextStart.GetPara();
        nLast1 = aOldTextEnd.GetPara() + 1;
        nFirst2 = 0;
        nLast2 = 0;
    }
    else if ( aOldTextStart == aOldTextEnd && aNewTextStart != aNewTextEnd )
    {
        // old selection empty + new selection not empty => notify events only for new selection
        nFirst1 = aNewTextStart.GetPara();
        nLast1 = aNewTextEnd.GetPara() + 1;
        nFirst2 = 0;
        nLast2 = 0;
    }
    else
    {
        // old and new selection not empty => notify events for the two ranges left and right of the overlap
        ::std::vector< TextPaM > aTextPaMs(4);
        aTextPaMs[0] = aOldTextStart;
        aTextPaMs[1] = aOldTextEnd;
        aTextPaMs[2] = aNewTextStart;
        aTextPaMs[3] = aNewTextEnd;
        ::std::sort( aTextPaMs.begin(), aTextPaMs.end() );

        nFirst1 = aTextPaMs[0].GetPara();
        nLast1 = aTextPaMs[1].GetPara() + ( aTextPaMs[0] != aTextPaMs[1] ? 1 : 0 );

        nFirst2 = aTextPaMs[2].GetPara();
        nLast2 = aTextPaMs[3].GetPara() + ( aTextPaMs[2] != aTextPaMs[3] ? 1 : 0 );

        // adjust overlapping ranges
        if ( nLast1 > nFirst2 )
            nLast1 = nFirst2;
    }

    // notify selection changes
    notifySelectionChange( nFirst1, nLast1 );
    notifySelectionChange( nFirst2, nLast2 );

    m_nSelectionFirstPara = nNewFirstPara;
    m_nSelectionFirstPos = nNewFirstPos;
    m_nSelectionLastPara = nNewLastPara;
    m_nSelectionLastPos = nNewLastPos;
}

void Document::notifySelectionChange( sal_Int32 nFirst, sal_Int32 nLast )
{
    if ( nFirst < nLast )
    {
        Paragraphs::iterator aItBound1 = m_xParagraphs->begin();
        for (sal_Int32 i = 0; i < nLast  && aItBound1 !=  m_xParagraphs->end() ; ++aItBound1);
        Paragraphs::iterator aEnd( ::std::min( aItBound1, m_aVisibleEnd ) );

        Paragraphs::iterator aItBound2 = m_xParagraphs->begin();
        for (sal_Int32 i = 0; i < nFirst && aItBound2 !=  m_xParagraphs->end() ; ++aItBound2);

        for ( Paragraphs::iterator aIt = ::std::max( aItBound2, m_aVisibleBegin ); aIt != aEnd; ++aIt )
        {
            ::rtl::Reference< ParagraphImpl > xParagraph( getParagraph( aIt ) );
            if ( xParagraph.is() )
            {
                xParagraph->notifyEvent(
                    ::css::accessibility::AccessibleEventId::SELECTION_CHANGED,
                    ::css::uno::Any(), ::css::uno::Any() );
                xParagraph->notifyEvent(
                    ::css::accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED,
                    ::css::uno::Any(), ::css::uno::Any() );
            }
        }
    }
}

void Document::justifySelection( TextPaM& rTextStart, TextPaM& rTextEnd )
{
    if ( rTextStart > rTextEnd )
    {
        TextPaM aTextPaM( rTextStart );
        rTextStart = rTextEnd;
        rTextEnd = aTextPaM;
    }
}

void Document::disposeParagraphs()
{
    for (Paragraphs::iterator aIt(m_xParagraphs->begin());
         aIt != m_xParagraphs->end(); ++aIt)
    {
        ::css::uno::Reference< ::css::lang::XComponent > xComponent(
            aIt->getParagraph().get(), ::css::uno::UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
    }
}

// static
::css::uno::Any Document::mapFontColor(::Color const & rColor)
{
    return ::css::uno::makeAny(
        static_cast< ::sal_Int32 >(COLORDATA_RGB(rColor.GetColor())));
        // FIXME  keep transparency?
}

// static
::Color Document::mapFontColor(::css::uno::Any const & rColor)
{
    ::sal_Int32 nColor = 0;
    rColor >>= nColor;
    return ::Color(static_cast< ::ColorData >(nColor));
}

// static
::css::uno::Any Document::mapFontWeight(::FontWeight nWeight)
{
    // Map from ::FontWeight to ::css:awt::FontWeight, depends on order of
    // elements in ::FontWeight (vcl/vclenum.hxx):
    static float const aWeight[]
        = { ::css::awt::FontWeight::DONTKNOW, // WEIGHT_DONTKNOW
            ::css::awt::FontWeight::THIN, // WEIGHT_THIN
            ::css::awt::FontWeight::ULTRALIGHT, // WEIGHT_ULTRALIGHT
            ::css::awt::FontWeight::LIGHT, // WEIGHT_LIGHT
            ::css::awt::FontWeight::SEMILIGHT, // WEIGHT_SEMILIGHT
            ::css::awt::FontWeight::NORMAL, // WEIGHT_NORMAL
            ::css::awt::FontWeight::NORMAL, // WEIGHT_MEDIUM
            ::css::awt::FontWeight::SEMIBOLD, // WEIGHT_SEMIBOLD
            ::css::awt::FontWeight::BOLD, // WEIGHT_BOLD
            ::css::awt::FontWeight::ULTRABOLD, // WEIGHT_ULTRABOLD
            ::css::awt::FontWeight::BLACK }; // WEIGHT_BLACK
    return ::css::uno::makeAny(aWeight[nWeight]);
}

// static
::FontWeight Document::mapFontWeight(::css::uno::Any const & rWeight)
{
    float nWeight = ::css::awt::FontWeight::NORMAL;
    rWeight >>= nWeight;
    return nWeight <= ::css::awt::FontWeight::DONTKNOW ? WEIGHT_DONTKNOW
        : nWeight <= ::css::awt::FontWeight::THIN ? WEIGHT_THIN
        : nWeight <= ::css::awt::FontWeight::ULTRALIGHT ? WEIGHT_ULTRALIGHT
        : nWeight <= ::css::awt::FontWeight::LIGHT ? WEIGHT_LIGHT
        : nWeight <= ::css::awt::FontWeight::SEMILIGHT ? WEIGHT_SEMILIGHT
        : nWeight <= ::css::awt::FontWeight::NORMAL ? WEIGHT_NORMAL
        : nWeight <= ::css::awt::FontWeight::SEMIBOLD ? WEIGHT_SEMIBOLD
        : nWeight <= ::css::awt::FontWeight::BOLD ? WEIGHT_BOLD
        : nWeight <= ::css::awt::FontWeight::ULTRABOLD ? WEIGHT_ULTRABOLD
        : WEIGHT_BLACK;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
