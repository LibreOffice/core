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

#include <sal/config.h>
#include <sal/log.hxx>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/i18n/Boundary.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <extended/textwindowaccessibility.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <vcl/window.hxx>
#include <toolkit/helper/convert.hxx>
#include <comphelper/sequence.hxx>

#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

namespace accessibility
{
void SfxListenerGuard::startListening(::SfxBroadcaster & rNotifier)
{
    assert(m_pNotifier == nullptr && "called more than once");
    m_pNotifier = &rNotifier;
    m_rListener.StartListening(*m_pNotifier, DuplicateHandling::Prevent);
}

void SfxListenerGuard::endListening()
{
    if (m_pNotifier != nullptr)
    {
        m_rListener.EndListening(*m_pNotifier);
        m_pNotifier = nullptr;
    }
}

void WindowListenerGuard::startListening(vcl::Window & rNotifier)
{
    assert(m_pNotifier == nullptr && "called more than once");
    m_pNotifier = &rNotifier;
    m_pNotifier->AddEventListener(m_aListener);
}

void WindowListenerGuard::endListening()
{
    if (m_pNotifier)
    {
        m_pNotifier->RemoveEventListener(m_aListener);
        m_pNotifier = nullptr;
    }
}

Paragraph::Paragraph(::rtl::Reference< Document > const & rDocument,
                             Paragraphs::size_type nNumber):
    ParagraphBase(m_aMutex),
    m_xDocument(rDocument),
    m_nNumber(nNumber),
    m_nClientId(0)
{
    m_aParagraphText = m_xDocument->retrieveParagraphText(this);
}

void
Paragraph::numberChanged(bool bIncremented)
{
    if (bIncremented)
        ++m_nNumber;
    else
        --m_nNumber;
}

void Paragraph::textChanged()
{
    OUString aParagraphText = implGetText();
    css::uno::Any aOldValue, aNewValue;
    if ( implInitTextChangedEvent( m_aParagraphText, aParagraphText, aOldValue, aNewValue ) )
    {
        m_aParagraphText = aParagraphText;
        notifyEvent(css::accessibility::AccessibleEventId::
                    TEXT_CHANGED,
                    aOldValue, aNewValue);
    }
}

void Paragraph::notifyEvent(::sal_Int16 nEventId,
                                css::uno::Any const & rOldValue,
                                css::uno::Any const & rNewValue)
{
    if (m_nClientId)
        comphelper::AccessibleEventNotifier::addEvent( m_nClientId, css::accessibility::AccessibleEventObject(
                             static_cast< ::cppu::OWeakObject * >(this),
                             nEventId, rNewValue, rOldValue) );
}

// virtual
css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL
Paragraph::getAccessibleContext()
{
    checkDisposed();
    return this;
}

// virtual
::sal_Int32 SAL_CALL Paragraph::getAccessibleChildCount()
{
    checkDisposed();
    return 0;
}

// virtual
css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
Paragraph::getAccessibleChild(::sal_Int32)
{
    checkDisposed();
    throw css::lang::IndexOutOfBoundsException(
        "textwindowaccessibility.cxx:"
        " Paragraph::getAccessibleChild",
        static_cast< css::uno::XWeak * >(this));
}

// virtual
css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
Paragraph::getAccessibleParent()
{
    checkDisposed();
    return m_xDocument->getAccessible();
}

// virtual
::sal_Int32 SAL_CALL Paragraph::getAccessibleIndexInParent()
{
    checkDisposed();
    return m_xDocument->retrieveParagraphIndex(this);
}

// virtual
::sal_Int16 SAL_CALL Paragraph::getAccessibleRole()
{
    checkDisposed();
    return css::accessibility::AccessibleRole::PARAGRAPH;
}

// virtual
OUString SAL_CALL Paragraph::getAccessibleDescription()
{
    checkDisposed();
    return OUString();
}

// virtual
OUString SAL_CALL Paragraph::getAccessibleName()
{
    checkDisposed();
    return OUString();
}

// virtual
css::uno::Reference< css::accessibility::XAccessibleRelationSet >
SAL_CALL Paragraph::getAccessibleRelationSet()
{
    checkDisposed();
    return m_xDocument->retrieveParagraphRelationSet( this );
}

// virtual
css::uno::Reference< css::accessibility::XAccessibleStateSet >
SAL_CALL Paragraph::getAccessibleStateSet()
{
    checkDisposed();

    // FIXME  Notification of changes (STATE_CHANGED) missing when
    // m_rView.IsReadOnly() changes:
    return new ::utl::AccessibleStateSetHelper(
        m_xDocument->retrieveParagraphState(this));
}

// virtual
css::lang::Locale SAL_CALL Paragraph::getLocale()
{
    checkDisposed();
    return m_xDocument->retrieveLocale();
}

// virtual
sal_Bool SAL_CALL Paragraph::containsPoint(css::awt::Point const & rPoint)
{
    checkDisposed();
    css::awt::Rectangle aRect(m_xDocument->retrieveParagraphBounds(this,
                                                                     false));
    return rPoint.X >= 0 && rPoint.X < aRect.Width
        && rPoint.Y >= 0 && rPoint.Y < aRect.Height;
}

// virtual
css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
Paragraph::getAccessibleAtPoint(css::awt::Point const &)
{
    checkDisposed();
    return nullptr;
}

// virtual
css::awt::Rectangle SAL_CALL Paragraph::getBounds()
{
    checkDisposed();
    return m_xDocument->retrieveParagraphBounds(this, false);
}

// virtual
css::awt::Point SAL_CALL Paragraph::getLocation()
{
    checkDisposed();
    css::awt::Rectangle aRect(m_xDocument->retrieveParagraphBounds(this,
                                                                     false));
    return css::awt::Point(aRect.X, aRect.Y);
}

// virtual
css::awt::Point SAL_CALL Paragraph::getLocationOnScreen()
{
    checkDisposed();
    css::awt::Rectangle aRect(m_xDocument->retrieveParagraphBounds(this,
                                                                     true));
    return css::awt::Point(aRect.X, aRect.Y);
}

// virtual
css::awt::Size SAL_CALL Paragraph::getSize()
{
    checkDisposed();
    css::awt::Rectangle aRect(m_xDocument->retrieveParagraphBounds(this,
                                                                     false));
    return css::awt::Size(aRect.Width, aRect.Height);
}

// virtual
void SAL_CALL Paragraph::grabFocus()
{
    checkDisposed();
    VclPtr<vcl::Window> pWindow = m_xDocument->GetWindow();
    if ( pWindow )
    {
        pWindow->GrabFocus();
    }
    try
    {
        m_xDocument->changeParagraphSelection(this, 0, 0);
    }
    catch (const css::lang::IndexOutOfBoundsException & rEx)
    {
        SAL_INFO("accessibility",
                 "textwindowaccessibility.cxx: Paragraph::grabFocus: caught unexpected "
                 << rEx);
    }
}

// virtual
sal_Int32 SAL_CALL Paragraph::getForeground()
{
    return 0; // TODO
}

// virtual
sal_Int32 SAL_CALL Paragraph::getBackground()
{
    return 0; // TODO
}

// virtual
::sal_Int32 SAL_CALL Paragraph::getCaretPosition()
{
    checkDisposed();
    return m_xDocument->retrieveParagraphCaretPosition(this);
}

// virtual
sal_Bool SAL_CALL Paragraph::setCaretPosition(::sal_Int32 nIndex)
{
    checkDisposed();
    m_xDocument->changeParagraphSelection(this, nIndex, nIndex);
    return true;
}

// virtual
::sal_Unicode SAL_CALL Paragraph::getCharacter(::sal_Int32 nIndex)
{
    checkDisposed();
    return OCommonAccessibleText::implGetCharacter(implGetText(), nIndex);
}

// virtual
css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
Paragraph::getCharacterAttributes(::sal_Int32 nIndex, const css::uno::Sequence< OUString >& aRequestedAttributes)
{
    checkDisposed();
    return m_xDocument->retrieveCharacterAttributes( this, nIndex, aRequestedAttributes );
}

// virtual
css::awt::Rectangle SAL_CALL
Paragraph::getCharacterBounds(::sal_Int32 nIndex)
{
    checkDisposed();
    css::awt::Rectangle aBounds(m_xDocument->retrieveCharacterBounds(this, nIndex));
    css::awt::Rectangle aParaBounds(m_xDocument->retrieveParagraphBounds(this, false));
    aBounds.X -= aParaBounds.X;
    aBounds.Y -= aParaBounds.Y;
    return aBounds;
}

// virtual
::sal_Int32 SAL_CALL Paragraph::getCharacterCount()
{
    checkDisposed();
    return implGetText().getLength();
}

// virtual
::sal_Int32 SAL_CALL
Paragraph::getIndexAtPoint(css::awt::Point const & rPoint)
{
    checkDisposed();
    css::awt::Point aPoint(rPoint);
    css::awt::Rectangle aParaBounds(m_xDocument->retrieveParagraphBounds(this, false));
    aPoint.X += aParaBounds.X;
    aPoint.Y += aParaBounds.Y;
    return m_xDocument->retrieveCharacterIndex(this, aPoint);
}

// virtual
OUString SAL_CALL Paragraph::getSelectedText()
{
    checkDisposed();

    return OCommonAccessibleText::getSelectedText();
}

// virtual
::sal_Int32 SAL_CALL Paragraph::getSelectionStart()
{
    checkDisposed();
    return OCommonAccessibleText::getSelectionStart();
}

// virtual
::sal_Int32 SAL_CALL Paragraph::getSelectionEnd()
{
    checkDisposed();
    return OCommonAccessibleText::getSelectionEnd();
}

// virtual
sal_Bool SAL_CALL Paragraph::setSelection(::sal_Int32 nStartIndex,
                                                ::sal_Int32 nEndIndex)
{
    checkDisposed();
    m_xDocument->changeParagraphSelection(this, nStartIndex, nEndIndex);
    return true;
}

// virtual
OUString SAL_CALL Paragraph::getText()
{
    checkDisposed();
    return implGetText();
}

// virtual
OUString SAL_CALL Paragraph::getTextRange(::sal_Int32 nStartIndex,
                                                     ::sal_Int32 nEndIndex)
{
    checkDisposed();
    return OCommonAccessibleText::implGetTextRange(implGetText(), nStartIndex, nEndIndex);
}

// virtual
css::accessibility::TextSegment SAL_CALL Paragraph::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType )
{
    checkDisposed();
    return OCommonAccessibleText::getTextAtIndex(nIndex, aTextType);
}

// virtual
css::accessibility::TextSegment SAL_CALL Paragraph::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType )
{
    checkDisposed();
    return OCommonAccessibleText::getTextBeforeIndex(nIndex, aTextType);
}

// virtual
css::accessibility::TextSegment SAL_CALL Paragraph::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType )
{
    checkDisposed();
    return OCommonAccessibleText::getTextBehindIndex(nIndex, aTextType);
}

// virtual
sal_Bool SAL_CALL Paragraph::copyText(::sal_Int32 nStartIndex,
                                            ::sal_Int32 nEndIndex)
{
    checkDisposed();
    m_xDocument->copyParagraphText(this, nStartIndex, nEndIndex);
    return true;
}

// virtual
sal_Bool SAL_CALL Paragraph::cutText(::sal_Int32 nStartIndex,
                                           ::sal_Int32 nEndIndex)
{
    checkDisposed();
    m_xDocument->changeParagraphText(this, nStartIndex, nEndIndex, true, false,
                                     OUString());
    return true;
}

// virtual
sal_Bool SAL_CALL Paragraph::pasteText(::sal_Int32 nIndex)
{
    checkDisposed();
    m_xDocument->changeParagraphText(this, nIndex, nIndex, false, true,
                                     OUString());
    return true;
}

// virtual
sal_Bool SAL_CALL Paragraph::deleteText(::sal_Int32 nStartIndex,
                                          ::sal_Int32 nEndIndex)
{
    checkDisposed();
    m_xDocument->changeParagraphText(this, nStartIndex, nEndIndex, false, false,
                                     OUString());
    return true;
}

// virtual
sal_Bool SAL_CALL Paragraph::insertText(OUString const & rText,
                                              ::sal_Int32 nIndex)
{
    checkDisposed();
    m_xDocument->changeParagraphText(this, nIndex, nIndex, false, false, rText);
    return true;
}

// virtual
sal_Bool SAL_CALL
Paragraph::replaceText(::sal_Int32 nStartIndex, ::sal_Int32 nEndIndex,
                           OUString const & rReplacement)
{
    checkDisposed();
    m_xDocument->changeParagraphText(this, nStartIndex, nEndIndex, false, false,
                                     rReplacement);
    return true;
}

// virtual
sal_Bool SAL_CALL Paragraph::setAttributes(
    ::sal_Int32 nStartIndex, ::sal_Int32 nEndIndex,
    css::uno::Sequence< css::beans::PropertyValue > const & rAttributeSet)
{
    checkDisposed();
    m_xDocument->changeParagraphAttributes(this, nStartIndex, nEndIndex,
                                           rAttributeSet);
    return true;
}

// virtual
sal_Bool SAL_CALL Paragraph::setText(OUString const & rText)
{
    checkDisposed();
    m_xDocument->changeParagraphText(this, rText);
    return true;
}

// virtual
css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
Paragraph::getDefaultAttributes(const css::uno::Sequence< OUString >&)
{
    checkDisposed();
    return {}; // default attributes are not supported by text engine
}

// virtual
css::uno::Sequence< css::beans::PropertyValue > SAL_CALL
Paragraph::getRunAttributes(::sal_Int32 Index, const css::uno::Sequence< OUString >& RequestedAttributes)
{
    checkDisposed();
    return m_xDocument->retrieveRunAttributes( this, Index, RequestedAttributes );
}

// virtual
::sal_Int32 SAL_CALL Paragraph::getLineNumberAtIndex( ::sal_Int32 nIndex )
{
    checkDisposed();

    ::sal_Int32 nLineNo = -1;
    m_xDocument->retrieveParagraphLineBoundary( this, nIndex, &nLineNo );

    return nLineNo;
}

// virtual
css::accessibility::TextSegment SAL_CALL Paragraph::getTextAtLineNumber( ::sal_Int32 nLineNo )
{
    checkDisposed();

    css::i18n::Boundary aBoundary =
        m_xDocument->retrieveParagraphBoundaryOfLine( this, nLineNo );

    return css::accessibility::TextSegment( getTextRange(aBoundary.startPos, aBoundary.endPos),
        aBoundary.startPos, aBoundary.endPos);
}

// virtual
css::accessibility::TextSegment SAL_CALL Paragraph::getTextAtLineWithCaret(  )
{
    checkDisposed();

    sal_Int32 nLineNo = getNumberOfLineWithCaret();

    try {
        return ( nLineNo >= 0 ) ?
            getTextAtLineNumber( nLineNo ) :
            css::accessibility::TextSegment();
    } catch (const css::lang::IndexOutOfBoundsException&) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
            "textwindowaccessibility.cxx:"
            " Paragraph::getTextAtLineWithCaret",
            static_cast< css::uno::XWeak * >( this ), anyEx );
    }
}

// virtual
::sal_Int32 SAL_CALL Paragraph::getNumberOfLineWithCaret(  )
{
    checkDisposed();
    return m_xDocument->retrieveParagraphLineWithCursor(this);
}


// virtual
void SAL_CALL Paragraph::addAccessibleEventListener(
    css::uno::Reference<
    css::accessibility::XAccessibleEventListener > const & rListener)
{
    if (rListener.is())
    {
        ::osl::ClearableMutexGuard aGuard(rBHelper.rMutex);
        if (rBHelper.bDisposed || rBHelper.bInDispose)
        {
            aGuard.clear();
            rListener->disposing(css::lang::EventObject(
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
void SAL_CALL Paragraph::removeAccessibleEventListener(
    css::uno::Reference<
    css::accessibility::XAccessibleEventListener > const & rListener)
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
void SAL_CALL Paragraph::disposing()
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
OUString Paragraph::implGetText()
{
    return m_xDocument->retrieveParagraphText(this);
}

// virtual
css::lang::Locale Paragraph::implGetLocale()
{
    return m_xDocument->retrieveLocale();
}

// virtual
void Paragraph::implGetSelection(::sal_Int32 & rStartIndex,
                                     ::sal_Int32 & rEndIndex)
{
    m_xDocument->retrieveParagraphSelection(this, &rStartIndex, &rEndIndex);
}

// virtual
void Paragraph::implGetParagraphBoundary( const OUString& rText,
                                          css::i18n::Boundary& rBoundary,
                                          ::sal_Int32 nIndex )
{
    ::sal_Int32 nLength = rText.getLength();

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
void Paragraph::implGetLineBoundary( const OUString& rText,
                                     css::i18n::Boundary& rBoundary,
                                     ::sal_Int32 nIndex )
{
    ::sal_Int32 nLength = rText.getLength();

    if ( implIsValidIndex( nIndex, nLength ) || nIndex == nLength )
    {
        css::i18n::Boundary aBoundary =
            m_xDocument->retrieveParagraphLineBoundary( this, nIndex, nullptr );
        rBoundary.startPos = aBoundary.startPos;
        rBoundary.endPos = aBoundary.endPos;
    }
    else
    {
        rBoundary.startPos = nIndex;
        rBoundary.endPos = nIndex;
    }
}


void Paragraph::checkDisposed()
{
    ::osl::MutexGuard aGuard(rBHelper.rMutex);
    if (!(rBHelper.bDisposed || rBHelper.bInDispose))
        return;
    throw css::lang::DisposedException(
        OUString(), static_cast< css::uno::XWeak * >(this));
}

Document::Document(::VCLXWindow * pVclXWindow, ::TextEngine & rEngine,
                   ::TextView & rView):
    VCLXAccessibleComponent(pVclXWindow),
    m_xAccessible(pVclXWindow),
    m_rEngine(rEngine),
    m_rView(rView),
    m_aEngineListener(*this),
    m_aViewListener(LINK(this, Document, WindowEventHandler)),
    m_nViewOffset(0),
    m_nViewHeight(0),
    m_nVisibleBeginOffset(0),
    m_nSelectionFirstPara(-1),
    m_nSelectionFirstPos(-1),
    m_nSelectionLastPara(-1),
    m_nSelectionLastPos(-1),
    m_bSelectionChangedNotification(false)
{}

css::lang::Locale Document::retrieveLocale()
{
    SolarMutexGuard aGuard;
    return m_rEngine.GetLocale();
}

::sal_Int32 Document::retrieveParagraphIndex(Paragraph const * pParagraph)
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

::sal_Int64 Document::retrieveParagraphState(Paragraph const * pParagraph)
{
    ::osl::MutexGuard aInternalGuard(GetMutex());

    // If a client holds on to a Paragraph that is no longer visible, it can
    // happen that this Paragraph lies outside the range from m_aVisibleBegin
    // to m_aVisibleEnd.  In that case, it is neither VISIBLE nor SHOWING:
    ::sal_Int64 nState
          = (static_cast< ::sal_Int64 >(1)
             << css::accessibility::AccessibleStateType::ENABLED)
          | (static_cast< ::sal_Int64 >(1)
             << css::accessibility::AccessibleStateType::SENSITIVE)
          | (static_cast< ::sal_Int64 >(1)
             << css::accessibility::AccessibleStateType::FOCUSABLE)
          | (static_cast< ::sal_Int64 >(1)
             << css::accessibility::AccessibleStateType::MULTI_LINE);
    if (!m_rView.IsReadOnly())
        nState |= (static_cast< ::sal_Int64 >(1)
                   << css::accessibility::AccessibleStateType::EDITABLE);
    Paragraphs::iterator aPara(m_xParagraphs->begin()
                               + pParagraph->getNumber());
    if (aPara >= m_aVisibleBegin && aPara < m_aVisibleEnd)
    {
        nState
            |= (static_cast< ::sal_Int64 >(1)
                << css::accessibility::AccessibleStateType::VISIBLE)
            | (static_cast< ::sal_Int64 >(1)
               << css::accessibility::AccessibleStateType::SHOWING);
        if (aPara == m_aFocused)
            nState |= (static_cast< ::sal_Int64 >(1)
                       << css::accessibility::AccessibleStateType::FOCUSED);
    }
    return nState;
};

css::awt::Rectangle
Document::retrieveParagraphBounds(Paragraph const * pParagraph,
                                  bool bAbsolute)
{
    SolarMutexGuard aGuard;
    ::osl::MutexGuard aInternalGuard(GetMutex());

    // If a client holds on to a Paragraph that is no longer visible (as it
    // scrolled out the top of the view), it can happen that this Paragraph
    // lies before m_aVisibleBegin.  In that case, calculate the vertical
    // position of the Paragraph starting at paragraph 0, otherwise optimize
    // and start at m_aVisibleBegin:
    Paragraphs::iterator aPara(m_xParagraphs->begin()
                               + pParagraph->getNumber());
    auto lAddHeight = [](const sal_Int32& rSum, const ParagraphInfo& rParagraph) {
        return rSum + rParagraph.getHeight(); };
    ::sal_Int32 nPos;
    if (aPara < m_aVisibleBegin)
        nPos = std::accumulate(m_xParagraphs->begin(), aPara, sal_Int32(0), lAddHeight);
    else
        nPos = std::accumulate(m_aVisibleBegin, aPara, m_nViewOffset - m_nVisibleBeginOffset, lAddHeight);

    Point aOrig(0, 0);
    if (bAbsolute)
        aOrig = m_rView.GetWindow()->OutputToAbsoluteScreenPixel(aOrig);

    return css::awt::Rectangle(
        static_cast< ::sal_Int32 >(aOrig.X()),
        static_cast< ::sal_Int32 >(aOrig.Y()) + nPos - m_nViewOffset,
        m_rView.GetWindow()->GetOutputSizePixel().Width(), aPara->getHeight());
        // XXX  numeric overflow (3x)
}

OUString
Document::retrieveParagraphText(Paragraph const * pParagraph)
{
    SolarMutexGuard aGuard;
    ::osl::MutexGuard aInternalGuard(GetMutex());
    return m_rEngine.GetText(static_cast< ::sal_uInt32 >(pParagraph->getNumber()));
        // numeric overflow cannot happen here
}

void Document::retrieveParagraphSelection(Paragraph const * pParagraph,
                                          ::sal_Int32 * pBegin,
                                          ::sal_Int32 * pEnd)
{
    SolarMutexGuard aGuard;
    ::osl::MutexGuard aInternalGuard(GetMutex());
    ::TextSelection const & rSelection = m_rView.GetSelection();
    Paragraphs::size_type nNumber = pParagraph->getNumber();
    TextPaM aStartPaM( rSelection.GetStart() );
    TextPaM aEndPaM( rSelection.GetEnd() );
    TextPaM aMinPaM( std::min( aStartPaM, aEndPaM ) );
    TextPaM aMaxPaM( std::max( aStartPaM, aEndPaM ) );

    if ( nNumber >= aMinPaM.GetPara() && nNumber <= aMaxPaM.GetPara() )
    {
        *pBegin = nNumber > aMinPaM.GetPara() ? 0 : aMinPaM.GetIndex();
            // XXX numeric overflow
        *pEnd = nNumber < aMaxPaM.GetPara()
            ? m_rEngine.GetText(static_cast< ::sal_uInt32 >(nNumber)).getLength()
            : aMaxPaM.GetIndex();
            // XXX  numeric overflow (3x)

        if ( aStartPaM > aEndPaM )
            std::swap( *pBegin, *pEnd );
    }
    else
    {
        *pBegin = 0;
        *pEnd = 0;
    }
}

::sal_Int32 Document::retrieveParagraphCaretPosition(Paragraph const * pParagraph)
{
    SolarMutexGuard aGuard;
    ::osl::MutexGuard aInternalGuard(GetMutex());
    ::TextSelection const & rSelection = m_rView.GetSelection();
    Paragraphs::size_type nNumber = pParagraph->getNumber();
    TextPaM aEndPaM( rSelection.GetEnd() );

    return aEndPaM.GetPara() == nNumber ? aEndPaM.GetIndex() : -1;
}

css::awt::Rectangle
Document::retrieveCharacterBounds(Paragraph const * pParagraph,
                                  ::sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    ::osl::MutexGuard aInternalGuard(GetMutex());
    ::sal_uInt32 nNumber = static_cast< ::sal_uInt32 >(pParagraph->getNumber());
    sal_Int32 nLength = m_rEngine.GetText(nNumber).getLength();
        // XXX  numeric overflow
    if (nIndex < 0 || nIndex > nLength)
        throw css::lang::IndexOutOfBoundsException(
            "textwindowaccessibility.cxx:"
            " Document::retrieveCharacterAttributes",
            static_cast< css::uno::XWeak * >(this));
    css::awt::Rectangle aBounds( 0, 0, 0, 0 );
    if ( nIndex == nLength )
    {
        aBounds = AWTRectangle(
            m_rEngine.PaMtoEditCursor(::TextPaM(nNumber, nIndex)));
    }
    else
    {
        ::tools::Rectangle aLeft(
            m_rEngine.PaMtoEditCursor(::TextPaM(nNumber, nIndex)));
            // XXX  numeric overflow
        ::tools::Rectangle aRight(
            m_rEngine.PaMtoEditCursor(::TextPaM(nNumber, nIndex + 1)));
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
        aBounds = css::awt::Rectangle(static_cast< ::sal_Int32 >(aLeft.Left()),
                                        static_cast< ::sal_Int32 >(aLeft.Top() - m_nViewOffset),
                                        nWidth,
                                        static_cast< ::sal_Int32 >(aLeft.Bottom()
                                                                    - aLeft.Top()));
            // XXX  numeric overflow (4x)
    }
    return aBounds;
}

::sal_Int32 Document::retrieveCharacterIndex(Paragraph const * pParagraph,
                                             css::awt::Point const & rPoint)
{
    SolarMutexGuard aGuard;
    ::osl::MutexGuard aInternalGuard(GetMutex());
    ::sal_uInt32 nNumber = static_cast< ::sal_uInt32 >(pParagraph->getNumber());
        // XXX  numeric overflow
    ::TextPaM aPaM(m_rEngine.GetPaM(::Point(static_cast< long >(rPoint.X),
                                            static_cast< long >(rPoint.Y))));
        // XXX  numeric overflow (2x)
    return aPaM.GetPara() == nNumber ? aPaM.GetIndex() : -1;
        // XXX  numeric overflow
}

struct IndexCompare
{
    const css::beans::PropertyValue* pValues;
    explicit IndexCompare(const css::beans::PropertyValue* pVals)
        : pValues(pVals)
    {
    }
    bool operator() ( sal_Int32 a, sal_Int32 b ) const
    {
        return pValues[a].Name < pValues[b].Name;
    }
};

css::uno::Sequence< css::beans::PropertyValue >
Document::retrieveCharacterAttributes(
    Paragraph const * pParagraph, ::sal_Int32 nIndex,
    const css::uno::Sequence< OUString >& aRequestedAttributes)
{
    SolarMutexGuard aGuard;

    vcl::Font aFont = m_rEngine.GetFont();
    const sal_Int32 AttributeCount = 9;
    sal_Int32 i = 0;
    css::uno::Sequence< css::beans::PropertyValue > aAttribs( AttributeCount );

    //character background color
    aAttribs[i].Name = "CharBackColor";
    aAttribs[i].Handle = -1;
    aAttribs[i].Value = mapFontColor( aFont.GetFillColor() );
    aAttribs[i].State = css::beans::PropertyState_DIRECT_VALUE;
    i++;

    //character color
    aAttribs[i].Name = "CharColor";
    aAttribs[i].Handle = -1;
    //aAttribs[i].Value = mapFontColor( aFont.GetColor() );
    aAttribs[i].Value = mapFontColor( m_rEngine.GetTextColor() );
    aAttribs[i].State = css::beans::PropertyState_DIRECT_VALUE;
    i++;

    //character font name
    aAttribs[i].Name = "CharFontName";
    aAttribs[i].Handle = -1;
    aAttribs[i].Value <<= aFont.GetFamilyName();
    aAttribs[i].State = css::beans::PropertyState_DIRECT_VALUE;
    i++;

    //character height
    aAttribs[i].Name = "CharHeight";
    aAttribs[i].Handle = -1;
    aAttribs[i].Value <<= static_cast<sal_Int16>(aFont.GetFontHeight());
    aAttribs[i].State = css::beans::PropertyState_DIRECT_VALUE;
    i++;

    //character posture
    aAttribs[i].Name = "CharPosture";
    aAttribs[i].Handle = -1;
    aAttribs[i].Value <<= static_cast<sal_Int16>(aFont.GetItalic());
    aAttribs[i].State = css::beans::PropertyState_DIRECT_VALUE;
    i++;

    //character relief
    /*
    aAttribs[i].Name = "CharRelief";
    aAttribs[i].Handle = -1;
    aAttribs[i].Value = css::uno::Any( (sal_Int16)aFont.GetRelief() );
    aAttribs[i].State = css::beans::PropertyState_DIRECT_VALUE;
    i++;
    */

    //character strikeout
    aAttribs[i].Name = "CharStrikeout";
    aAttribs[i].Handle = -1;
    aAttribs[i].Value <<= static_cast<sal_Int16>(aFont.GetStrikeout());
    aAttribs[i].State = css::beans::PropertyState_DIRECT_VALUE;
    i++;

    //character underline
    aAttribs[i].Name = "CharUnderline";
    aAttribs[i].Handle = -1;
    aAttribs[i].Value <<= static_cast<sal_Int16>(aFont.GetUnderline());
    aAttribs[i].State = css::beans::PropertyState_DIRECT_VALUE;
    i++;

    //character weight
    aAttribs[i].Name = "CharWeight";
    aAttribs[i].Handle = -1;
    aAttribs[i].Value <<= static_cast<float>(aFont.GetWeight());
    aAttribs[i].State = css::beans::PropertyState_DIRECT_VALUE;
    i++;

    //character alignment
    aAttribs[i].Name = "ParaAdjust";
    aAttribs[i].Handle = -1;
    aAttribs[i].Value <<= static_cast<sal_Int16>(m_rEngine.GetTextAlign());
    aAttribs[i].State = css::beans::PropertyState_DIRECT_VALUE;
    i++;

    ::osl::MutexGuard aInternalGuard(GetMutex());
    ::sal_uInt32 nNumber = static_cast< ::sal_uInt32 >(pParagraph->getNumber());
        // XXX  numeric overflow
        // nIndex can be equal to getLength();
    if (nIndex < 0 || nIndex > m_rEngine.GetText(nNumber).getLength())
        throw css::lang::IndexOutOfBoundsException(
            "textwindowaccessibility.cxx:"
            " Document::retrieveCharacterAttributes",
            static_cast< css::uno::XWeak * >(this));


    // retrieve run attributes
    tPropValMap aCharAttrSeq;
    retrieveRunAttributesImpl( pParagraph, nIndex, aRequestedAttributes, aCharAttrSeq );

    css::beans::PropertyValue* pValues = aAttribs.getArray();
    for (i = 0; i < AttributeCount; i++,pValues++)
    {
        aCharAttrSeq[ pValues->Name ] = *pValues;
    }

    css::uno::Sequence< css::beans::PropertyValue > aRes = comphelper::mapValuesToSequence( aCharAttrSeq );

    // sort the attributes
    sal_Int32 nLength = aRes.getLength();
    const css::beans::PropertyValue* pPairs = aRes.getConstArray();
    std::unique_ptr<sal_Int32[]> pIndices( new sal_Int32[nLength] );
    for( i = 0; i < nLength; i++ )
        pIndices[i] = i;
    std::sort( &pIndices[0], &pIndices[nLength], IndexCompare(pPairs) );
    // create sorted sequences according to index array
    css::uno::Sequence< css::beans::PropertyValue > aNewValues( nLength );
    css::beans::PropertyValue* pNewValues = aNewValues.getArray();
    for( i = 0; i < nLength; i++ )
    {
        pNewValues[i] = pPairs[pIndices[i]];
    }

    return aNewValues;
}

void Document::retrieveRunAttributesImpl(
    Paragraph const * pParagraph, ::sal_Int32 Index,
    const css::uno::Sequence< OUString >& RequestedAttributes,
    tPropValMap& rRunAttrSeq)
{
    ::sal_uInt32 nNumber = static_cast< ::sal_uInt32 >( pParagraph->getNumber() );
    ::TextPaM aPaM( nNumber, Index );
        // XXX  numeric overflow
    ::TextAttribFontColor const * pColor
          = static_cast< ::TextAttribFontColor const * >(
              m_rEngine.FindAttrib( aPaM, TEXTATTR_FONTCOLOR ) );
    ::TextAttribFontWeight const * pWeight
          = static_cast< ::TextAttribFontWeight const * >(
              m_rEngine.FindAttrib( aPaM, TEXTATTR_FONTWEIGHT ) );
    tPropValMap aRunAttrSeq;
    if ( pColor )
    {
        css::beans::PropertyValue aPropVal;
        aPropVal.Name = "CharColor";
        aPropVal.Handle = -1;
        aPropVal.Value = mapFontColor( pColor->GetColor() );
        aPropVal.State = css::beans::PropertyState_DIRECT_VALUE;
        aRunAttrSeq[ aPropVal.Name ] = aPropVal;
    }
    if ( pWeight )
    {
        css::beans::PropertyValue aPropVal;
        aPropVal.Name = "CharWeight";
        aPropVal.Handle = -1;
        aPropVal.Value = mapFontWeight( pWeight->getFontWeight() );
        aPropVal.State = css::beans::PropertyState_DIRECT_VALUE;
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

css::uno::Sequence< css::beans::PropertyValue >
Document::retrieveRunAttributes(
    Paragraph const * pParagraph, ::sal_Int32 Index,
    const css::uno::Sequence< OUString >& RequestedAttributes)
{
    SolarMutexGuard aGuard;
    ::osl::MutexGuard aInternalGuard( GetMutex() );
    ::sal_uInt32 nNumber = static_cast< ::sal_uInt32 >( pParagraph->getNumber() );
        // XXX  numeric overflow
    if ( Index < 0 || Index >= m_rEngine.GetText(nNumber).getLength() )
        throw css::lang::IndexOutOfBoundsException(
            "textwindowaccessibility.cxx:"
            " Document::retrieveRunAttributes",
            static_cast< css::uno::XWeak * >( this ) );

    tPropValMap aRunAttrSeq;
    retrieveRunAttributesImpl( pParagraph, Index, RequestedAttributes, aRunAttrSeq );
    return comphelper::mapValuesToSequence( aRunAttrSeq );
}

void Document::changeParagraphText(Paragraph const * pParagraph,
                                   OUString const & rText)
{
    SolarMutexGuard aGuard;
    {
        ::osl::MutexGuard aInternalGuard(GetMutex());
        ::sal_uInt32 nNumber = static_cast< ::sal_uInt32 >(pParagraph->getNumber());
            // XXX  numeric overflow
        changeParagraphText(nNumber, 0, m_rEngine.GetTextLen(nNumber), false,
                            false, rText);
    }
}

void Document::changeParagraphText(Paragraph const * pParagraph,
                                   ::sal_Int32 nBegin, ::sal_Int32 nEnd,
                                   bool bCut, bool bPaste,
                                   OUString const & rText)
{
    SolarMutexGuard aGuard;
    {
        ::osl::MutexGuard aInternalGuard(GetMutex());
        ::sal_uInt32 nNumber = static_cast< ::sal_uInt32 >(pParagraph->getNumber());
            // XXX  numeric overflow
        if (nBegin < 0 || nBegin > nEnd
            || nEnd > m_rEngine.GetText(nNumber).getLength())
            throw css::lang::IndexOutOfBoundsException(
                "textwindowaccessibility.cxx:"
                " Document::changeParagraphText",
                static_cast< css::uno::XWeak * >(this));
        changeParagraphText(nNumber, static_cast< ::sal_uInt16 >(nBegin),
                            static_cast< ::sal_uInt16 >(nEnd), bCut, bPaste, rText);
            // XXX  numeric overflow (2x)
    }
}

void Document::copyParagraphText(Paragraph const * pParagraph,
                                 ::sal_Int32 nBegin, ::sal_Int32 nEnd)
{
    SolarMutexGuard aGuard;
    {
        ::osl::MutexGuard aInternalGuard(GetMutex());
        ::sal_uInt32 nNumber = static_cast< ::sal_uInt32 >(pParagraph->getNumber());
            // XXX  numeric overflow
        if (nBegin < 0 || nBegin > nEnd
            || nEnd > m_rEngine.GetText(nNumber).getLength())
            throw css::lang::IndexOutOfBoundsException(
                "textwindowaccessibility.cxx:"
                " Document::copyParagraphText",
                static_cast< css::uno::XWeak * >(this));
        m_rView.SetSelection(
            ::TextSelection(::TextPaM(nNumber, nBegin),
                            ::TextPaM(nNumber, nEnd)));
            // XXX  numeric overflow (2x)
        m_rView.Copy();
    }
}

void Document::changeParagraphAttributes(
    Paragraph const * pParagraph, ::sal_Int32 nBegin, ::sal_Int32 nEnd,
    css::uno::Sequence< css::beans::PropertyValue > const & rAttributeSet)
{
    SolarMutexGuard aGuard;
    {
        ::osl::MutexGuard aInternalGuard(GetMutex());
        ::sal_uInt32 nNumber = static_cast< ::sal_uInt32 >(pParagraph->getNumber());
        // XXX  numeric overflow
        if (nBegin < 0 || nBegin > nEnd
            || nEnd > m_rEngine.GetText(nNumber).getLength())
            throw css::lang::IndexOutOfBoundsException(
                "textwindowaccessibility.cxx:"
                " Document::changeParagraphAttributes",
                static_cast< css::uno::XWeak * >(this));

        // FIXME  The new attributes are added to any attributes already set,
        // they do not replace the old attributes as required by
        // XAccessibleEditableText.setAttributes:
        for (::sal_Int32 i = 0; i < rAttributeSet.getLength(); ++i)
            if ( rAttributeSet[i].Name == "CharColor" )
                m_rEngine.SetAttrib(::TextAttribFontColor(
                                        mapFontColor(rAttributeSet[i].Value)),
                                        nNumber, nBegin, nEnd);
                    // XXX  numeric overflow (2x)
            else if ( rAttributeSet[i].Name == "CharWeight" )
                m_rEngine.SetAttrib(::TextAttribFontWeight(
                                        mapFontWeight(rAttributeSet[i].Value)),
                                        nNumber, nBegin, nEnd);
                    // XXX  numeric overflow (2x)
    }
}

void Document::changeParagraphSelection(Paragraph const * pParagraph,
                                        ::sal_Int32 nBegin, ::sal_Int32 nEnd)
{
    SolarMutexGuard aGuard;
    {
        ::osl::MutexGuard aInternalGuard(GetMutex());
        ::sal_uInt32 nNumber = static_cast< ::sal_uInt32 >(pParagraph->getNumber());
            // XXX  numeric overflow
        if (nBegin < 0 || nBegin > nEnd
            || nEnd > m_rEngine.GetText(nNumber).getLength())
            throw css::lang::IndexOutOfBoundsException(
                "textwindowaccessibility.cxx:"
                " Document::changeParagraphSelection",
                static_cast< css::uno::XWeak * >(this));
        m_rView.SetSelection(
            ::TextSelection(::TextPaM(nNumber, nBegin),
                            ::TextPaM(nNumber, nEnd)));
            // XXX  numeric overflow (2x)
    }
}

css::i18n::Boundary
Document::retrieveParagraphLineBoundary( Paragraph const * pParagraph,
                                         ::sal_Int32 nIndex, ::sal_Int32 *pLineNo )
{
    css::i18n::Boundary aBoundary;
    aBoundary.startPos = nIndex;
    aBoundary.endPos = nIndex;

    SolarMutexGuard aGuard;
    {
        ::osl::MutexGuard aInternalGuard( GetMutex() );
        ::sal_uInt32 nNumber = static_cast< ::sal_uInt32 >( pParagraph->getNumber() );
        if ( nIndex < 0 || nIndex > m_rEngine.GetText( nNumber ).getLength() )
            throw css::lang::IndexOutOfBoundsException(
                "textwindowaccessibility.cxx:"
                " Document::retrieveParagraphLineBoundary",
                static_cast< css::uno::XWeak * >( this ) );
        ::sal_Int32 nLineStart = 0;
        ::sal_Int32 nLineEnd = 0;
        ::sal_uInt16 nLineCount = m_rEngine.GetLineCount( nNumber );
        for ( ::sal_uInt16 nLine = 0; nLine < nLineCount; ++nLine )
        {
            nLineStart = nLineEnd;
            nLineEnd += m_rEngine.GetLineLen( nNumber, nLine );
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

css::i18n::Boundary
Document::retrieveParagraphBoundaryOfLine( Paragraph const * pParagraph,
                                           ::sal_Int32 nLineNo )
{
    css::i18n::Boundary aBoundary;
    aBoundary.startPos = 0;
    aBoundary.endPos = 0;

    SolarMutexGuard aGuard;
    {
        ::osl::MutexGuard aInternalGuard( GetMutex() );
        ::sal_uInt32 nNumber = static_cast< ::sal_uInt32 >( pParagraph->getNumber() );
        if ( nLineNo >= m_rEngine.GetLineCount( nNumber ) )
            throw css::lang::IndexOutOfBoundsException(
                "textwindowaccessibility.cxx:"
                " Document::retrieveParagraphBoundaryOfLine",
                static_cast< css::uno::XWeak * >( this ) );
        ::sal_Int32 nLineStart = 0;
        ::sal_Int32 nLineEnd = 0;
        for ( ::sal_Int32 nLine = 0; nLine <= nLineNo; ++nLine )
        {
            nLineStart = nLineEnd;
            nLineEnd += m_rEngine.GetLineLen( nNumber, nLine );
        }

        aBoundary.startPos = nLineStart;
        aBoundary.endPos = nLineEnd;
    }

    return aBoundary;
}

sal_Int32 Document::retrieveParagraphLineWithCursor( Paragraph const * pParagraph )
{
    SolarMutexGuard aGuard;
    ::osl::MutexGuard aInternalGuard(GetMutex());
    ::TextSelection const & rSelection = m_rView.GetSelection();
    Paragraphs::size_type nNumber = pParagraph->getNumber();
    TextPaM aEndPaM( rSelection.GetEnd() );

    return aEndPaM.GetPara() == nNumber
        ? m_rView.GetLineNumberOfCursorInSelection() : -1;
}


css::uno::Reference< css::accessibility::XAccessibleRelationSet >
Document::retrieveParagraphRelationSet( Paragraph const * pParagraph )
{
    ::osl::MutexGuard aInternalGuard( GetMutex() );

    ::utl::AccessibleRelationSetHelper* pRelationSetHelper = new ::utl::AccessibleRelationSetHelper();
    css::uno::Reference< css::accessibility::XAccessibleRelationSet > xSet = pRelationSetHelper;

    Paragraphs::iterator aPara( m_xParagraphs->begin() + pParagraph->getNumber() );

    if ( aPara > m_aVisibleBegin && aPara < m_aVisibleEnd )
    {
        css::uno::Sequence< css::uno::Reference< css::uno::XInterface > > aSequence { getAccessibleChild( aPara - 1 ) };
        css::accessibility::AccessibleRelation aRelation( css::accessibility::AccessibleRelationType::CONTENT_FLOWS_FROM, aSequence );
        pRelationSetHelper->AddRelation( aRelation );
    }

    if ( aPara >= m_aVisibleBegin && aPara < m_aVisibleEnd -1 )
    {
        css::uno::Sequence< css::uno::Reference< css::uno::XInterface > > aSequence { getAccessibleChild( aPara + 1 ) };
        css::accessibility::AccessibleRelation aRelation( css::accessibility::AccessibleRelationType::CONTENT_FLOWS_TO, aSequence );
        pRelationSetHelper->AddRelation( aRelation );
    }

    return xSet;
}

// virtual
::sal_Int32 SAL_CALL Document::getAccessibleChildCount()
{
    ::comphelper::OExternalLockGuard aGuard(this);
    init();
    return m_aVisibleEnd - m_aVisibleBegin;
}

// virtual
css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
Document::getAccessibleChild(::sal_Int32 i)
{
    ::comphelper::OExternalLockGuard aGuard(this);
    init();
    if (i < 0 || i >= m_aVisibleEnd - m_aVisibleBegin)
        throw css::lang::IndexOutOfBoundsException(
            "textwindowaccessibility.cxx:"
            " Document::getAccessibleChild",
            static_cast< css::uno::XWeak * >(this));
    return getAccessibleChild(m_aVisibleBegin
                              + static_cast< Paragraphs::size_type >(i));
}

// virtual
::sal_Int16 SAL_CALL Document::getAccessibleRole()
{
    return css::accessibility::AccessibleRole::TEXT_FRAME;
}

// virtual
css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
Document::getAccessibleAtPoint(css::awt::Point const & rPoint)
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
    return nullptr;
}
void Document::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleComponent::FillAccessibleStateSet( rStateSet );
    if (!m_rView.IsReadOnly())
        rStateSet.AddState( css::accessibility::AccessibleStateType::EDITABLE );
}

void    Document::FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet )
{
    if( getAccessibleParent()->getAccessibleContext()->getAccessibleRole() == css::accessibility::AccessibleRole::SCROLL_PANE )
    {
        css::uno::Sequence< css::uno::Reference< css::uno::XInterface > > aSequence {  getAccessibleParent() };
        rRelationSet.AddRelation( css::accessibility::AccessibleRelation( css::accessibility::AccessibleRelationType::MEMBER_OF, aSequence ) );
    }
    else
    {
         VCLXAccessibleComponent::FillAccessibleRelationSet(rRelationSet);
    }
}
// virtual
void SAL_CALL Document::disposing()
{
    m_aEngineListener.endListening();
    m_aViewListener.endListening();
    if (m_xParagraphs != nullptr)
        disposeParagraphs();
    VCLXAccessibleComponent::disposing();
}

// virtual
void Document::Notify(::SfxBroadcaster &, ::SfxHint const & rHint)
{
    const TextHint* pTextHint = dynamic_cast<const TextHint*>(&rHint);
    if (pTextHint)
    {
        ::TextHint const & rTextHint = *pTextHint;
        switch (rTextHint.GetId())
        {
        case SfxHintId::TextParaInserted:
        case SfxHintId::TextParaRemoved:
            // SfxHintId::TextParaInserted and SfxHintId::TextParaRemoved are sent at
            // "unsafe" times (when the text engine has not yet re-formatted its
            // content), so that for example calling ::TextEngine::GetTextHeight
            // from within the code that handles SfxHintId::TextParaInserted causes
            // trouble within the text engine.  Therefore, these hints are just
            // buffered until a following ::TextEngine::FormatDoc causes a
            // SfxHintId::TextFormatted to come in:
        case SfxHintId::TextFormatPara:
            // ::TextEngine::FormatDoc sends a sequence of
            // SfxHintId::TextFormatParas, followed by an optional
            // SfxHintId::TextHeightChanged, followed in all cases by one
            // SfxHintId::TextFormatted.  Only the SfxHintId::TextFormatParas contain
            // the numbers of the affected paragraphs, but they are sent
            // before the changes are applied.  Therefore, SfxHintId::TextFormatParas
            // are just buffered until another hint comes in:
            {
                ::osl::MutexGuard aInternalGuard(GetMutex());
                if (!isAlive())
                    break;

                m_aParagraphNotifications.push(rTextHint);
                break;
            }
        case SfxHintId::TextFormatted:
        case SfxHintId::TextHeightChanged:
        case SfxHintId::TextModified:
            {
                ::osl::MutexGuard aInternalGuard(GetMutex());
                if (!isAlive())
                    break;
                handleParagraphNotifications();
                break;
            }
        case SfxHintId::TextViewScrolled:
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
        case SfxHintId::TextViewSelectionChanged:
        case SfxHintId::TextViewCaretChanged:
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
                    // SfxHintId::TextViewSelectionChanged is sometimes sent at
                    // "unsafe" times (when the text engine has not yet re-
                    // formatted its content), so that for example calling
                    // ::TextEngine::GetTextHeight from within the code that
                    // handles a previous SfxHintId::TextParaInserted causes
                    // trouble within the text engine.  Therefore, these
                    // hints are just buffered (along with
                    // SfxHintId::TextParaInserted/REMOVED/FORMATPARA) until a
                    // following ::TextEngine::FormatDoc causes a
                    // SfxHintId::TextFormatted to come in:
                    m_bSelectionChangedNotification = true;
                }
                break;
            }
        default: break;
        }
    }
}

IMPL_LINK(Document, WindowEventHandler, ::VclWindowEvent&, rEvent, void)
{
    switch (rEvent.GetId())
    {
    case VclEventId::WindowResize:
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
    case VclEventId::WindowGetFocus:
        {
            ::osl::MutexGuard aInternalGuard(GetMutex());
            if (!isAlive())
                break;
            //to enable the PARAGRAPH to get focus for multiline edit
            ::sal_Int32 count = getAccessibleChildCount();
            bool bEmpty = m_aFocused == m_aVisibleEnd && count == 1;
            if ((m_aFocused >= m_aVisibleBegin && m_aFocused < m_aVisibleEnd) || bEmpty)
            {
                Paragraphs::iterator aTemp = bEmpty ? m_aVisibleBegin : m_aFocused;
                ::rtl::Reference< Paragraph > xParagraph(getParagraph(aTemp));
                if (xParagraph.is())
                {
                    xParagraph->notifyEvent(
                        css::accessibility::AccessibleEventId::
                        STATE_CHANGED,
                        css::uno::Any(),
                        css::uno::Any(
                            css::accessibility::AccessibleStateType::
                            FOCUSED));
                }
            }
            break;
        }
    case VclEventId::WindowLoseFocus:
        {
            ::osl::MutexGuard aInternalGuard(GetMutex());
            if (!isAlive())
                break;
            //to enable the PARAGRAPH to get focus for multiline edit
            ::sal_Int32 count = getAccessibleChildCount();
            bool bEmpty = m_aFocused == m_aVisibleEnd && count == 1;
            if ((m_aFocused >= m_aVisibleBegin && m_aFocused < m_aVisibleEnd) || bEmpty)
            {
                Paragraphs::iterator aTemp = bEmpty ? m_aVisibleBegin : m_aFocused;
                ::rtl::Reference< Paragraph > xParagraph(getParagraph(aTemp));
                if (xParagraph.is())
                    xParagraph->notifyEvent(
                        css::accessibility::AccessibleEventId::
                        STATE_CHANGED,
                        css::uno::Any(
                            css::accessibility::AccessibleStateType::
                            FOCUSED),
                        css::uno::Any());
            }
            break;
        }
    default: break;
    }
}

void Document::init()
{
    if (m_xParagraphs == nullptr)
    {
        const ::sal_uInt32 nCount = m_rEngine.GetParagraphCount();
        m_xParagraphs.reset(new Paragraphs);
        m_xParagraphs->reserve(static_cast< Paragraphs::size_type >(nCount));
            // numeric overflow is harmless here
        for (::sal_uInt32 i = 0; i < nCount; ++i)
            m_xParagraphs->push_back(ParagraphInfo(static_cast< ::sal_Int32 >(
                                           m_rEngine.GetTextHeight(i))));
                // XXX  numeric overflow
        m_nViewOffset = static_cast< ::sal_Int32 >(
            m_rView.GetStartDocPos().Y()); // XXX  numeric overflow
        m_nViewHeight = static_cast< ::sal_Int32 >(
            m_rView.GetWindow()->GetOutputSizePixel().Height());
            // XXX  numeric overflow
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

::rtl::Reference< Paragraph >
Document::getParagraph(Paragraphs::iterator const & rIt)
{
    return static_cast< Paragraph * >(
        css::uno::Reference< css::accessibility::XAccessible >(
            rIt->getParagraph()).get());
}

css::uno::Reference< css::accessibility::XAccessible >
Document::getAccessibleChild(Paragraphs::iterator const & rIt)
{
    css::uno::Reference< css::accessibility::XAccessible > xParagraph(
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

    SAL_WARN_IF(
            !((m_aVisibleBegin == m_xParagraphs->end() && m_aVisibleEnd == m_xParagraphs->end() && m_nVisibleBeginOffset == 0)
                || (m_aVisibleBegin < m_aVisibleEnd && m_nVisibleBeginOffset >= 0)),
            "accessibility",
            "invalid visible range");
}

void Document::notifyVisibleRangeChanges(
    Paragraphs::iterator const & rOldVisibleBegin,
    Paragraphs::iterator const & rOldVisibleEnd,
    Paragraphs::iterator const & rInserted)
{
    // XXX  Replace this code that determines which paragraphs have changed from
    // invisible to visible or vice versa with a better algorithm.
    for (Paragraphs::iterator aIt(rOldVisibleBegin); aIt != rOldVisibleEnd;
         ++aIt)
    {
        if (aIt != rInserted
            && (aIt < m_aVisibleBegin || aIt >= m_aVisibleEnd))
            NotifyAccessibleEvent(
                css::accessibility::AccessibleEventId::
                CHILD,
                css::uno::Any(getAccessibleChild(aIt)),
                css::uno::Any());
    }
    for (Paragraphs::iterator aIt(m_aVisibleBegin); aIt != m_aVisibleEnd;
         ++aIt)
    {
        if (aIt == rInserted
            || aIt < rOldVisibleBegin || aIt >= rOldVisibleEnd)
            NotifyAccessibleEvent(
                css::accessibility::AccessibleEventId::
                CHILD,
                css::uno::Any(),
                css::uno::Any(getAccessibleChild(aIt)));
    }
}

void
Document::changeParagraphText(::sal_uInt32 nNumber, ::sal_uInt16 nBegin, ::sal_uInt16 nEnd,
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
        case SfxHintId::TextParaInserted:
            {
                ::sal_uInt32 n = static_cast< ::sal_uInt32 >( aHint.GetValue() );
                assert(n <= m_xParagraphs->size() && "bad SfxHintId::TextParaInserted event");

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
                    ::rtl::Reference< Paragraph > xParagraph(
                        getParagraph(aIt));
                    if (xParagraph.is())
                        xParagraph->numberChanged(true);
                }

                notifyVisibleRangeChanges(
                    m_xParagraphs->begin() + nOldVisibleBegin,
                    m_xParagraphs->begin() + nOldVisibleEnd, aIns);
                break;
            }
        case SfxHintId::TextParaRemoved:
            {
                ::sal_uInt32 n = static_cast< ::sal_uInt32 >( aHint.GetValue() );
                if (n == TEXT_PARA_ALL)
                {
                    for (Paragraphs::iterator aIt(m_aVisibleBegin);
                         aIt != m_aVisibleEnd; ++aIt)
                    {
                        NotifyAccessibleEvent(
                            css::accessibility::AccessibleEventId::
                            CHILD,
                            css::uno::Any(getAccessibleChild(aIt)),
                            css::uno::Any());
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
                    assert(n < m_xParagraphs->size() && "Bad SfxHintId::TextParaRemoved event");

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
                        assert(m_nSelectionFirstPara < m_nSelectionLastPara && "logic error");
                        --m_nSelectionLastPara;
                        m_nSelectionLastPos = 0x7FFFFFFF;
                    }

                    css::uno::Reference< css::accessibility::XAccessible >
                          xStrong;
                    if (bWasVisible)
                        xStrong = getAccessibleChild(aIt);
                    css::uno::WeakReference<
                          css::accessibility::XAccessible > xWeak(
                              aIt->getParagraph());
                    aIt = m_xParagraphs->erase(aIt);

                    determineVisibleRange();
                    m_aFocused = bWasFocused ? m_xParagraphs->end()
                        : m_xParagraphs->begin() + nOldFocused;

                    for (; aIt != m_xParagraphs->end(); ++aIt)
                    {
                        ::rtl::Reference< Paragraph > xParagraph(
                            getParagraph(aIt));
                        if (xParagraph.is())
                            xParagraph->numberChanged(false);
                    }

                    if (bWasVisible)
                        NotifyAccessibleEvent(
                            css::accessibility::AccessibleEventId::
                            CHILD,
                            css::uno::Any(xStrong),
                            css::uno::Any());

                    css::uno::Reference< css::lang::XComponent > xComponent(
                        xWeak.get(), css::uno::UNO_QUERY);
                    if (xComponent.is())
                        xComponent->dispose();

                    notifyVisibleRangeChanges(
                        m_xParagraphs->begin() + nOldVisibleBegin,
                        m_xParagraphs->begin() + nOldVisibleEnd,
                        m_xParagraphs->end());
                }
                break;
            }
        case SfxHintId::TextFormatPara:
            {
                ::sal_uInt32 n = static_cast< ::sal_uInt32 >( aHint.GetValue() );
                assert(n < m_xParagraphs->size() && "Bad SfxHintId::TextFormatPara event");

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
                    ::rtl::Reference< Paragraph > xParagraph(getParagraph(aIt));
                    if (xParagraph.is())
                        xParagraph->textChanged();
                }
                break;
            }
        default:
            SAL_WARN("accessibility", "bad buffered hint");
            break;
        }
    }
    if (m_bSelectionChangedNotification)
    {
        m_bSelectionChangedNotification = false;
        handleSelectionChangeNotification();
    }
}

namespace
{

enum class SelChangeType
{
    None, // no change, or invalid
    CaretMove, // neither old nor new have selection, and they are different
    NoSelToSel, // old has no selection but new has selection
    SelToNoSel, // old has selection but new has no selection
    // both old and new have selections
    NoParaChange, // only end index changed inside end para
    EndParaNoMoreBehind, // end para was behind start, but now is same or ahead
    AddedFollowingPara, // selection extended to following paragraph(s)
    ExcludedPreviousPara, // selection shrunk excluding previous paragraph(s)
    ExcludedFollowingPara, // selection shrunk excluding following paragraph(s)
    AddedPreviousPara, // selection extended to previous paragraph(s)
    EndParaBecameBehind // end para was ahead of start, but now is behind
};

SelChangeType getSelChangeType(const TextPaM& Os, const TextPaM& Oe,
                               const TextPaM& Ns, const TextPaM& Ne)
{
    if (Os == Oe) // no old selection
    {
        if (Ns == Ne) // no new selection: only caret moves
            return Os != Ns ? SelChangeType::CaretMove : SelChangeType::None;
        else // old has no selection but new has selection
            return SelChangeType::NoSelToSel;
    }
    else if (Ns == Ne) // old has selection; no new selection
    {
        return SelChangeType::SelToNoSel;
    }
    else if (Os == Ns) // both old and new have selections, and their starts are same
    {
        const sal_Int32 Osp = Os.GetPara(), Oep = Oe.GetPara();
        const sal_Int32 Nsp = Ns.GetPara(), Nep = Ne.GetPara();
        if (Oep == Nep) // end of selection stays in the same paragraph
        {
            //Send text_selection_change event on Nep
            return Oe.GetIndex() != Ne.GetIndex() ? SelChangeType::NoParaChange
                                                  : SelChangeType::None;
        }
        else if (Oep < Nep) // end of selection moved to a following paragraph
        {
            //all the following examples like 1,2->1,3 means that old start select para is 1, old end select para is 2,
            // then press shift up, the new start select para is 1, new end select para is 3;
            //for example, 1, 2 -> 1, 3; 4,1 -> 4, 7; 4,1 -> 4, 2; 4,4->4,5
            if (Nep >= Nsp) // new end para not behind start
            {
                // 1, 2 -> 1, 3; 4, 1 -> 4, 7; 4,4->4,5;
                if (Oep < Osp) // old end was behind start
                {
                    // 4,1 -> 4,7; 4,1 -> 4,4
                    return SelChangeType::EndParaNoMoreBehind;
                }
                else // old end para wasn't behind start
                {
                    // 1, 2 -> 1, 3; 4,4->4,5;
                    return SelChangeType::AddedFollowingPara;
                }
            }
            else // new end para is still behind start
            {
                // 4,1 -> 4,2,
                return SelChangeType::ExcludedPreviousPara;
            }
        }
        else // Oep > Nep => end of selection moved to a previous paragraph
        {
            // 3,2 -> 3,1; 4,7 -> 4,1; 4, 7 -> 4,6; 4,4 -> 4,3
            if (Nep >= Nsp) // new end para is still not behind of start
            {
                // 4,7 ->4,6
                return SelChangeType::ExcludedFollowingPara;
            }
            else // new end para is behind start
            {
                // 3,2 -> 3,1, 4,7 -> 4,1; 4,4->4,3
                if (Oep <= Osp) // it was not ahead already
                {
                    // 3,2 -> 3,1; 4,4->4,3
                    return SelChangeType::AddedPreviousPara;
                }
                else // it was ahead previously
                {
                    // 4,7 -> 4,1
                    return SelChangeType::EndParaBecameBehind;
                }
            }
        }
    }
    return SelChangeType::None;
}

} // namespace

void Document::sendEvent(::sal_Int32 start, ::sal_Int32 end, ::sal_Int16 nEventId)
{
    size_t nAvailDistance = std::distance(m_xParagraphs->begin(), m_aVisibleEnd);

    Paragraphs::iterator aEnd(m_xParagraphs->begin());
    size_t nEndDistance = std::min<size_t>(end + 1, nAvailDistance);
    std::advance(aEnd, nEndDistance);

    Paragraphs::iterator aIt(m_xParagraphs->begin());
    size_t nStartDistance = std::min<size_t>(start, nAvailDistance);
    std::advance(aIt, nStartDistance);

    while (aIt < aEnd)
    {
        ::rtl::Reference< Paragraph > xParagraph(getParagraph(aIt));
        if (xParagraph.is())
            xParagraph->notifyEvent(
            nEventId,
                css::uno::Any(), css::uno::Any());
        ++aIt;
    }
}

void Document::handleSelectionChangeNotification()
{
    ::TextSelection const & rSelection = m_rView.GetSelection();
    assert(rSelection.GetStart().GetPara() < m_xParagraphs->size() &&
           rSelection.GetEnd().GetPara() < m_xParagraphs->size() &&
           "bad SfxHintId::TextViewSelectionChanged event");
    ::sal_Int32 nNewFirstPara
          = static_cast< ::sal_Int32 >(rSelection.GetStart().GetPara());
    ::sal_Int32 nNewFirstPos = rSelection.GetStart().GetIndex();
        // XXX  numeric overflow
    ::sal_Int32 nNewLastPara
          = static_cast< ::sal_Int32 >(rSelection.GetEnd().GetPara());
    ::sal_Int32 nNewLastPos = rSelection.GetEnd().GetIndex();
        // XXX  numeric overflow

    // Lose focus:
    Paragraphs::iterator aIt(m_xParagraphs->begin() + nNewLastPara);
    if (m_aFocused != m_xParagraphs->end() && m_aFocused != aIt
        && m_aFocused >= m_aVisibleBegin && m_aFocused < m_aVisibleEnd)
    {
        ::rtl::Reference< Paragraph > xParagraph(getParagraph(m_aFocused));
        if (xParagraph.is())
            xParagraph->notifyEvent(
                css::accessibility::AccessibleEventId::
                STATE_CHANGED,
                css::uno::Any(
                    css::accessibility::AccessibleStateType::FOCUSED),
                css::uno::Any());
    }

    // Gain focus and update cursor position:
    if (aIt >= m_aVisibleBegin && aIt < m_aVisibleEnd
        && (aIt != m_aFocused
            || nNewLastPara != m_nSelectionLastPara
            || nNewLastPos != m_nSelectionLastPos))
    {
        ::rtl::Reference< Paragraph > xParagraph(getParagraph(aIt));
        if (xParagraph.is())
        {
            //disable the first event when user types in empty field.
            ::sal_Int32 count = getAccessibleChildCount();
            bool bEmpty = count > 1;
            //if (aIt != m_aFocused)
            if (aIt != m_aFocused && bEmpty)
                xParagraph->notifyEvent(
                    css::accessibility::AccessibleEventId::
                    STATE_CHANGED,
                    css::uno::Any(),
                    css::uno::Any(
                        css::accessibility::AccessibleStateType::FOCUSED));
            if (nNewLastPara != m_nSelectionLastPara
                || nNewLastPos != m_nSelectionLastPos)
                xParagraph->notifyEvent(
                    css::accessibility::AccessibleEventId::
                    CARET_CHANGED,
                    css::uno::makeAny< ::sal_Int32 >(
                        nNewLastPara == m_nSelectionLastPara
                        ? m_nSelectionLastPos : 0),
                    css::uno::Any(nNewLastPos));
        }
    }
    m_aFocused = aIt;

    if (m_nSelectionFirstPara != -1)
    {
        sal_Int32 nMin;
        sal_Int32 nMax;
        SelChangeType ret = getSelChangeType(TextPaM(m_nSelectionFirstPara, m_nSelectionFirstPos),
                                             TextPaM(m_nSelectionLastPara, m_nSelectionLastPos),
                                             rSelection.GetStart(), rSelection.GetEnd());
        switch (ret)
        {
            case SelChangeType::None:
                //no event
                break;
            case SelChangeType::CaretMove:
                //only caret moved, already handled in above
                break;
            case SelChangeType::NoSelToSel:
                //old has no selection but new has selection
                nMin = std::min(nNewFirstPara, nNewLastPara);
                nMax = std::max(nNewFirstPara, nNewLastPara);
                sendEvent(nMin, nMax, css::accessibility::AccessibleEventId::SELECTION_CHANGED);
                sendEvent(nMin, nMax,
                          css::accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED);
                break;
            case SelChangeType::SelToNoSel:
                //old has selection but new has no selection.
                nMin = std::min(m_nSelectionFirstPara, m_nSelectionLastPara);
                nMax = std::max(m_nSelectionFirstPara, m_nSelectionLastPara);
                sendEvent(nMin, nMax, css::accessibility::AccessibleEventId::SELECTION_CHANGED);
                sendEvent(nMin, nMax,
                          css::accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED);
                break;
            case SelChangeType::NoParaChange:
                //Send text_selection_change event on Nep
                sendEvent(nNewLastPara, nNewLastPara,
                          css::accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED);
                break;
            case SelChangeType::EndParaNoMoreBehind:
                // 4, 1 -> 4, 7; 4,1 -> 4,4
                sendEvent(m_nSelectionLastPara, m_nSelectionFirstPara - 1,
                          css::accessibility::AccessibleEventId::SELECTION_CHANGED);
                sendEvent(nNewFirstPara + 1, nNewLastPara,
                          css::accessibility::AccessibleEventId::SELECTION_CHANGED);

                sendEvent(m_nSelectionLastPara, nNewLastPara,
                          css::accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED);
                break;
            case SelChangeType::AddedFollowingPara:
                // 1, 2 -> 1, 4; 4,4->4,5;
                sendEvent(m_nSelectionLastPara + 1, nNewLastPara,
                          css::accessibility::AccessibleEventId::SELECTION_CHANGED);

                sendEvent(m_nSelectionLastPara, nNewLastPara,
                          css::accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED);
                break;
            case SelChangeType::ExcludedPreviousPara:
                // 4,1 -> 4,3,
                sendEvent(m_nSelectionLastPara + 1, nNewLastPara,
                          css::accessibility::AccessibleEventId::SELECTION_CHANGED);

                sendEvent(m_nSelectionLastPara, nNewLastPara,
                          css::accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED);
                break;
            case SelChangeType::ExcludedFollowingPara:
                // 4,7 ->4,5;
                sendEvent(nNewLastPara + 1, m_nSelectionLastPara,
                          css::accessibility::AccessibleEventId::SELECTION_CHANGED);

                sendEvent(nNewLastPara, m_nSelectionLastPara,
                          css::accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED);
                break;
            case SelChangeType::AddedPreviousPara:
                // 3,2 -> 3,1; 4,4->4,3
                sendEvent(nNewLastPara, m_nSelectionLastPara - 1,
                          css::accessibility::AccessibleEventId::SELECTION_CHANGED);

                sendEvent(nNewLastPara, m_nSelectionLastPara,
                          css::accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED);
                break;
            case SelChangeType::EndParaBecameBehind:
                // 4,7 -> 4,1
                sendEvent(m_nSelectionFirstPara + 1, m_nSelectionLastPara,
                          css::accessibility::AccessibleEventId::SELECTION_CHANGED);
                sendEvent(nNewLastPara, nNewFirstPara - 1,
                          css::accessibility::AccessibleEventId::SELECTION_CHANGED);

                sendEvent(nNewLastPara, m_nSelectionLastPara,
                          css::accessibility::AccessibleEventId::TEXT_SELECTION_CHANGED);
                break;
        }
    }

    m_nSelectionFirstPara = nNewFirstPara;
    m_nSelectionFirstPos = nNewFirstPos;
    m_nSelectionLastPara = nNewLastPara;
    m_nSelectionLastPos = nNewLastPos;
}

void Document::disposeParagraphs()
{
    for (auto const& paragraph : *m_xParagraphs)
    {
        css::uno::Reference< css::lang::XComponent > xComponent(
            paragraph.getParagraph().get(), css::uno::UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();
    }
}

// static
css::uno::Any Document::mapFontColor(::Color const & rColor)
{
    return css::uno::makeAny(rColor.GetRGBColor());
        // FIXME  keep transparency?
}

// static
::Color Document::mapFontColor(css::uno::Any const & rColor)
{
    ::Color nColor;
    rColor >>= nColor;
    return nColor;
}

// static
css::uno::Any Document::mapFontWeight(::FontWeight nWeight)
{
    // Map from ::FontWeight to css::awt::FontWeight, depends on order of
    // elements in ::FontWeight (vcl/vclenum.hxx):
    static float const aWeight[]
        = { css::awt::FontWeight::DONTKNOW, // WEIGHT_DONTKNOW
            css::awt::FontWeight::THIN, // WEIGHT_THIN
            css::awt::FontWeight::ULTRALIGHT, // WEIGHT_ULTRALIGHT
            css::awt::FontWeight::LIGHT, // WEIGHT_LIGHT
            css::awt::FontWeight::SEMILIGHT, // WEIGHT_SEMILIGHT
            css::awt::FontWeight::NORMAL, // WEIGHT_NORMAL
            css::awt::FontWeight::NORMAL, // WEIGHT_MEDIUM
            css::awt::FontWeight::SEMIBOLD, // WEIGHT_SEMIBOLD
            css::awt::FontWeight::BOLD, // WEIGHT_BOLD
            css::awt::FontWeight::ULTRABOLD, // WEIGHT_ULTRABOLD
            css::awt::FontWeight::BLACK }; // WEIGHT_BLACK
    return css::uno::Any(aWeight[nWeight]);
}

// static
::FontWeight Document::mapFontWeight(css::uno::Any const & rWeight)
{
    float nWeight = css::awt::FontWeight::NORMAL;
    rWeight >>= nWeight;
    return nWeight <= css::awt::FontWeight::DONTKNOW ? WEIGHT_DONTKNOW
        : nWeight <= css::awt::FontWeight::THIN ? WEIGHT_THIN
        : nWeight <= css::awt::FontWeight::ULTRALIGHT ? WEIGHT_ULTRALIGHT
        : nWeight <= css::awt::FontWeight::LIGHT ? WEIGHT_LIGHT
        : nWeight <= css::awt::FontWeight::SEMILIGHT ? WEIGHT_SEMILIGHT
        : nWeight <= css::awt::FontWeight::NORMAL ? WEIGHT_NORMAL
        : nWeight <= css::awt::FontWeight::SEMIBOLD ? WEIGHT_SEMIBOLD
        : nWeight <= css::awt::FontWeight::BOLD ? WEIGHT_BOLD
        : nWeight <= css::awt::FontWeight::ULTRABOLD ? WEIGHT_ULTRABOLD
        : WEIGHT_BLACK;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
