/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/datatransfer/clipboard/SystemClipboard.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/color.hxx>
#include <vcl/unohelp.hxx>
#include <test/a11y/AccessibilityTools.hxx>
#include <test/a11y/XAccessibleTextTester.hxx>

#include <cppunit/TestAssert.h>

using namespace css;

/**
 * Calls the method and checks returned value is
 * -1 (no caret in this object) or within range
 * [0, character_count];
 */
void XAccessibleTextTester::testGetCaretPosition()
{
    // check that caret position is -1 (no caret in this object)
    // or within range [0, character_count]
    const sal_Int32 nCaretPos = m_xText->getCaretPosition();
    CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(-1), nCaretPos);
    CPPUNIT_ASSERT_LESSEQUAL(m_xText->getCharacterCount(), nCaretPos);
}

/**
 * Calls the method with a wrong index and with a correct index.
 * Has OK status if exception was thrown for wrong index and
 * if exception wasn't thrown for the correct index.
 */
void XAccessibleTextTester::testSetCaretPosition()
{
    // check that trying to set invalid caret position results in IndexOutOfBoundsException
    for (sal_Int32 nPos : { sal_Int32(-2), m_xText->getCharacterCount() + 1 })
    {
        try
        {
            m_xText->setCaretPosition(nPos);
            CPPUNIT_FAIL("No exception thrown on invalid index");
        }
        catch (const css::lang::IndexOutOfBoundsException&)
        {
            // OK; this is expected
        }
    }

    const sal_Int32 nNewPos = m_xText->getCharacterCount() - 1;
    // setting caret position may or may not be supported
    const bool bSet = m_xText->setCaretPosition(nNewPos);
    if (bSet)
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Caret position doesn't match the one that was just set",
                                     nNewPos, m_xText->getCaretPosition());
}

/**
 * Calls the method with wrong indices and with correct indices.
 * Checks every character in the text.
 * Succeeds if exception was thrown for wrong index,
 * if exception wasn't thrown for the correct index and
 * if every character is equal to corresponding character in the text.
 */
void XAccessibleTextTester::testGetCharacter()
{
    const sal_Int32 nCharCount = m_xText->getCharacterCount();

    // check that trying to get character at invalid index results in IndexOutOfBoundsException
    for (sal_Int32 nPos : { sal_Int32(-1), nCharCount })
    {
        try
        {
            m_xText->getCharacter(nPos);
            CPPUNIT_FAIL("No exception thrown on invalid index");
        }
        catch (const css::lang::IndexOutOfBoundsException&)
        {
            // OK; this is expected
        }
    }

    // check every character of the text
    const OUString sText = m_xText->getText();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Text lengths don't match", nCharCount, sText.getLength());
    for (sal_Int32 i = 0; i < nCharCount; ++i)
    {
        const bool bSame = sText[i] == m_xText->getCharacter(i);
        CPPUNIT_ASSERT_MESSAGE("Character doesn't match", bSame);
    }
}

/**
 * Calls the method with wrong indices and with a correct index.
 * Succeeds if exception was thrown for the wrong indices,
 * if exception wasn't thrown for the correct index.
 */
void XAccessibleTextTester::testGetCharacterAttributes()
{
    const sal_Int32 nCharCount = m_xText->getCharacterCount();
    const css::uno::Sequence<OUString> aAttrs;

    // check that trying to get character attribtes at invalid index results in IndexOutOfBoundsException
    for (sal_Int32 nPos : { sal_Int32(-1), nCharCount })
    {
        try
        {
            m_xText->getCharacterAttributes(nPos, aAttrs);
            CPPUNIT_FAIL("No exception thrown on invalid index");
        }
        catch (const css::lang::IndexOutOfBoundsException&)
        {
            // OK; this is expected
        }
    }

    // check that calling with valid index succeeds
    if (nCharCount > 0)
    {
        // just call the method to verify no exception is thrown
        m_xText->getCharacterAttributes(nCharCount - 1, aAttrs);
    }
}

/**
 * Calls the method with wrong indices and with a correct index,
 * checks whether character bounds are within compontent bounds.
 */
void XAccessibleTextTester::testGetCharacterBounds()
{
    const sal_Int32 nCharCount = m_xText->getCharacterCount();

    // check that trying to get character bounds at invalid index results in IndexOutOfBoundsException
    // nCharCount (i.e. past the last character index) may be supported, so use nCharCount + 1
    for (sal_Int32 nPos : { sal_Int32(-1), nCharCount + 1 })
    {
        try
        {
            m_xText->getCharacterBounds(nPos);
            CPPUNIT_FAIL("No exception thrown on invalid index");
        }
        catch (const css::lang::IndexOutOfBoundsException&)
        {
            // OK; this is expected
        }
    }

    // check whether all characters fit into component bounds
    css::uno::Reference<css::accessibility::XAccessibleComponent> xComponent(
        m_xContext, css::uno::UNO_QUERY_THROW);
    css::awt::Rectangle aBounds = xComponent->getBounds();
    for (int i = 0; i < nCharCount; ++i)
    {
        css::awt::Rectangle aCharBounds = m_xText->getCharacterBounds(i);
        const bool bContained = (aCharBounds.X >= 0 && aCharBounds.Y >= 0)
                                && (aCharBounds.X + aCharBounds.Width <= aBounds.Width)
                                && (aCharBounds.Y + aCharBounds.Height <= aBounds.Height);
        if (!bContained)
        {
            std::cout << "Character bounds outside component" << std::endl;
            std::cout << "index: " << i << ", character: " << m_xText->getTextRange(i, i + 1)
                      << std::endl;
            std::cout << "Character rect: " << aCharBounds.X << ", " << aCharBounds.Y << ", "
                      << aCharBounds.Width << ", " << aCharBounds.Height << std::endl;
            std::cout << "Component rect: " << aBounds.X << ", " << aBounds.Y << ", "
                      << aBounds.Width << ", " << aBounds.Height << std::endl;
        }
    }
}

/**
 * Calls the method and checks character count matches
 * length of the text returned by XAccessibleText::getText.
 */
void XAccessibleTextTester::testGetCharacterCount()
{
    const sal_Int32 nCharCount = m_xText->getCharacterCount();
    CPPUNIT_ASSERT_GREATEREQUAL(sal_Int32(0), nCharCount);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Character count doesn't match text length",
                                 m_xText->getText().getLength(), nCharCount);
}

/**
 * Calls the method for an invalid point and for the point of rectangle
 * returned by the method <code>getCharacterBounds()</code>.
 * Succeeds if returned value is equal to <code>-1</code> for an
 * invalid point and if returned value matches the index for a valid point.
 */
void XAccessibleTextTester::testGetIndexAtPoint()
{
    const css::awt::Point aInvalidPoint(-1, -1);
    const sal_Int32 nInvalidIndex = m_xText->getIndexAtPoint(aInvalidPoint);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Index at invalid point isn't -1", sal_Int32(-1), nInvalidIndex);

    const sal_Int32 nCharCount = m_xText->getCharacterCount();
    for (sal_Int32 i = 0; i < nCharCount; ++i)
    {
        css::awt::Rectangle aRect = m_xText->getCharacterBounds(i);

        sal_Int32 nX = aRect.X + (aRect.Width / 2);
        sal_Int32 nY = aRect.Y + (aRect.Height / 2);
        css::awt::Point aPoint(nX, nY);
        const int nIndex = m_xText->getIndexAtPoint(aPoint);

        nX = aRect.X;
        nY = aRect.Y + (aRect.Height / 2);
        aPoint = awt::Point(nX, nY);
        const int nIndexLeft = m_xText->getIndexAtPoint(aPoint);

        // for some letters the center of the rectangle isn't recognised
        // in this case we are happy if the left border of the rectangle
        // returns the correct value.
        CPPUNIT_ASSERT(nIndex == i || nIndexLeft == i);
    }
}

/**
 * Checks returned values after different calls of the method
 * <code>setSelection()</code>.
 */
void XAccessibleTextTester::testGetSelectedText()
{
    // only check selection matches the requested one if XAccessibleText::setSelection
    // returns true, i.e. setting selection is supported

    // empty selection
    if (m_xText->setSelection(0, 0))
        CPPUNIT_ASSERT(m_xText->getSelectedText().isEmpty());

    const sal_Int32 nCharCount = m_xText->getCharacterCount();
    const OUString sText = m_xText->getText();

    // select all text
    if (m_xText->setSelection(0, nCharCount))
        CPPUNIT_ASSERT_EQUAL(sText, m_xText->getSelectedText());

    if (nCharCount > 2)
    {
        if (m_xText->setSelection(1, nCharCount - 1))
            CPPUNIT_ASSERT_EQUAL(OUString(sText.subView(1, nCharCount - 2)),
                                 m_xText->getSelectedText());
    }
}

/**
 * Checks a returned values after different calls of the method
 * <code>setSelection()</code>.
 */
void XAccessibleTextTester::testGetSelectionStart()
{
    // only check selection matches the requested one if XAccessibleText::setSelection
    // returns true, i.e. setting selection is supported

    const sal_Int32 nCharCount = m_xText->getCharacterCount();

    if (m_xText->setSelection(0, nCharCount))
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), m_xText->getSelectionStart());

    if (nCharCount > 2)
    {
        if (m_xText->setSelection(1, nCharCount - 1))
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), m_xText->getSelectionStart());
    }
}

/**
 * Checks a returned values after different calls of the method
 * <code>setSelection()</code>.
 */
void XAccessibleTextTester::testGetSelectionEnd()
{
    // only check selection matches the requested one if XAccessibleText::setSelection
    // returns true, i.e. setting selection is supported

    const sal_Int32 nCharCount = m_xText->getCharacterCount();

    if (m_xText->setSelection(0, nCharCount))
        CPPUNIT_ASSERT_EQUAL(nCharCount, m_xText->getSelectionEnd());

    if (nCharCount > 2)
    {
        if (m_xText->setSelection(1, nCharCount - 1))
            CPPUNIT_ASSERT_EQUAL(nCharCount - 1, m_xText->getSelectionStart());
    }
}

/**
 * Calls the method with invalid parameters and with valid parameters.
 * Succeeds if exception was thrown for invalid parameters and
 * exception wasn't thrown for valid parameters.
 */
void XAccessibleTextTester::testSetSelection()
{
    const sal_Int32 nCharCount = m_xText->getCharacterCount();

    // check that trying to set invalid selection results in IndexOutOfBoundsException
    // if setting selection is supported
    try
    {
        if (m_xText->setSelection(-1, nCharCount - 1))
            CPPUNIT_FAIL("No exception thrown on invalid index");
    }
    catch (const css::lang::IndexOutOfBoundsException&)
    {
        // OK; this is expected
    }
    try
    {
        if (m_xText->setSelection(0, nCharCount + 1))
            CPPUNIT_FAIL("No exception thrown on invalid index");
    }
    catch (const css::lang::IndexOutOfBoundsException&)
    {
        // OK; this is expected
    }

    // check that setting selection using valid indeces does not trigger exception
    if (nCharCount > 2)
    {
        m_xText->setSelection(1, nCharCount - 1);

        m_xText->setSelection(nCharCount - 1, 1);
    }

    if (nCharCount > 0)
    {
        m_xText->setSelection(0, nCharCount - 1);
        m_xText->setSelection(nCharCount - 1, 0);
    }

    m_xText->setSelection(0, 0);
}

/**
 * Calls the method to check it doesn't throw an exception.
 */
void XAccessibleTextTester::testGetText() { m_xText->getText(); }

/**
 * Calls the method with invalid parameters and with valid parameters,
 * checks returned values.
 * Succeeds if exception was thrown for invalid parameters,
 * if exception wasn't thrown for valid parameters and the
 * returned string matches the substring of the
 * the string returned by XAccessibleText::getText for the
 * given start/end indices.
 */
void XAccessibleTextTester::testGetTextRange()
{
    const OUString sText = m_xText->getText();
    const sal_Int32 nCharCount = m_xText->getCharacterCount();

    if (nCharCount > 3)
    {
        CPPUNIT_ASSERT_EQUAL(OUString(sText.subView(1, nCharCount - 3)),
                             m_xText->getTextRange(1, nCharCount - 2));
    }

    if (nCharCount > 1)
    {
        CPPUNIT_ASSERT_EQUAL(OUString(sText.subView(0, nCharCount - 1)),
                             m_xText->getTextRange(0, nCharCount - 1));
    }

    CPPUNIT_ASSERT_EQUAL(sText, m_xText->getTextRange(nCharCount, 0));

    CPPUNIT_ASSERT_EQUAL(u""_ustr, m_xText->getTextRange(0, 0));

    // check that exception is thrown for invalid indices
    try
    {
        m_xText->getTextRange(-1, nCharCount - 1);
        CPPUNIT_FAIL("No exception thrown on invalid index");
    }
    catch (const css::lang::IndexOutOfBoundsException&)
    {
        // OK; this is expected
    }

    try
    {
        m_xText->getTextRange(0, nCharCount + 1);
        CPPUNIT_FAIL("No exception thrown on invalid index");
    }
    catch (const css::lang::IndexOutOfBoundsException&)
    {
        // OK; this is expected
    }

    try
    {
        m_xText->getTextRange(nCharCount + 1, -1);
        CPPUNIT_FAIL("No exception thrown on invalid index");
    }
    catch (const css::lang::IndexOutOfBoundsException&)
    {
        // OK; this is expected
    }
}

/**
 * Calls the method with invalid parameters and with valid parameters,
 * checks returned values.
 * Succeeds if exception was thrown for invalid parameters,
 * if exception wasn't thrown for valid parameters.
 */
void XAccessibleTextTester::testGetTextAtIndex()
{
    const sal_Int32 nCharCount = m_xText->getCharacterCount();

    for (sal_Int32 nIndex : { sal_Int32(-1), nCharCount + 1 })
    {
        try
        {
            m_xText->getTextAtIndex(nIndex, css::accessibility::AccessibleTextType::PARAGRAPH);
            CPPUNIT_FAIL("No exception thrown on invalid index");
        }
        catch (const css::lang::IndexOutOfBoundsException&)
        {
            // OK; this is expected
        }
    }

    css::accessibility::TextSegment aTextSegment
        = m_xText->getTextAtIndex(nCharCount, css::accessibility::AccessibleTextType::WORD);
    CPPUNIT_ASSERT(aTextSegment.SegmentText.isEmpty());
}

/**
 * Calls the method with invalid parameters and with valid parameters.
 * Succeeds if exception was thrown for invalid parameters,
 * if exception wasn't thrown for valid parameters and if returned values
 * are equal to corresponding substrings of the text received via
 * XAccessibleText::getText
 */
void XAccessibleTextTester::testGetTextBeforeIndex()
{
    const OUString sText = m_xText->getText();
    const sal_Int32 nCharCount = m_xText->getCharacterCount();

    for (sal_Int32 nIndex : { sal_Int32(-1), nCharCount + 1 })
    {
        try
        {
            m_xText->getTextBeforeIndex(nIndex, css::accessibility::AccessibleTextType::PARAGRAPH);
            CPPUNIT_FAIL("No exception thrown on invalid index");
        }
        catch (const css::lang::IndexOutOfBoundsException&)
        {
            // OK; this is expected
        }
    }

    css::accessibility::TextSegment aTextSegment
        = m_xText->getTextBeforeIndex(0, css::accessibility::AccessibleTextType::PARAGRAPH);
    CPPUNIT_ASSERT(aTextSegment.SegmentText.isEmpty());

    if (nCharCount >= 2)
    {
        aTextSegment = m_xText->getTextBeforeIndex(
            nCharCount - 1, css::accessibility::AccessibleTextType::CHARACTER);
        CPPUNIT_ASSERT_EQUAL(OUString(sText.subView(nCharCount - 2, 1)), aTextSegment.SegmentText);
    }

    if (nCharCount > 2)
    {
        aTextSegment
            = m_xText->getTextBeforeIndex(2, css::accessibility::AccessibleTextType::CHARACTER);
        CPPUNIT_ASSERT_EQUAL(OUString(sText.subView(1, 1)), aTextSegment.SegmentText);
    }
}

/**
 * Calls the method with invalid parameters and with valid parameters,
 * checks returned values.
 * Succeeds if exception was thrown for invalid parameters,
 * if exception wasn't thrown for valid parameters and if returned values
 * are equal to corresponding substrings of the text received
 * via XAccessibleText::getText.
 */
void XAccessibleTextTester::testGetTextBehindIndex()
{
    const OUString sText = m_xText->getText();
    const sal_Int32 nCharCount = m_xText->getCharacterCount();

    for (sal_Int32 nIndex : { sal_Int32(-1), nCharCount + 1 })
    {
        try
        {
            m_xText->getTextBehindIndex(nIndex, css::accessibility::AccessibleTextType::PARAGRAPH);
            CPPUNIT_FAIL("No exception thrown on invalid index");
        }
        catch (const css::lang::IndexOutOfBoundsException&)
        {
            // OK; this is expected
        }
    }

    css::accessibility::TextSegment aTextSegment = m_xText->getTextBehindIndex(
        nCharCount, css::accessibility::AccessibleTextType::PARAGRAPH);
    CPPUNIT_ASSERT(aTextSegment.SegmentText.isEmpty());

    aTextSegment = m_xText->getTextBehindIndex(nCharCount - 1,
                                               css::accessibility::AccessibleTextType::PARAGRAPH);
    CPPUNIT_ASSERT(aTextSegment.SegmentText.isEmpty());

    if (nCharCount >= 3)
    {
        aTextSegment
            = m_xText->getTextBehindIndex(1, css::accessibility::AccessibleTextType::CHARACTER);
        CPPUNIT_ASSERT_EQUAL(OUString(sText.subView(2, 1)), aTextSegment.SegmentText);

        aTextSegment = m_xText->getTextBehindIndex(
            nCharCount - 2, css::accessibility::AccessibleTextType::CHARACTER);
        CPPUNIT_ASSERT_EQUAL(OUString(sText.subView(nCharCount - 1, 1)), aTextSegment.SegmentText);
    }
}

/**
 * Calls the method with invalid parameters and with valid parameter,
 * checks returned values.
 * Succeeds if exception was thrown for invalid parameters,
 * if exception wasn't thrown for valid parameter, returned value for
 * valid parameter is equal to <code>true</code> and the text
 * was copied to the clipboard.
 */
void XAccessibleTextTester::testCopyText()
{
    const OUString sText = m_xText->getText();
    const sal_Int32 nCharCount = m_xText->getCharacterCount();

    try
    {
        m_xText->copyText(-1, nCharCount);
        CPPUNIT_FAIL("No exception thrown on invalid index");
    }
    catch (const css::lang::IndexOutOfBoundsException&)
    {
        // OK; this is expected
    }

    try
    {
        m_xText->copyText(0, nCharCount + 1);
        CPPUNIT_FAIL("No exception thrown on invalid index");
    }
    catch (const css::lang::IndexOutOfBoundsException&)
    {
        // OK; this is expected
    }

    CPPUNIT_ASSERT(m_xText->copyText(0, nCharCount));
    OUString sClipboardText = getSystemClipboardText();
    CPPUNIT_ASSERT_EQUAL(sText, sClipboardText);

    if (nCharCount > 2)
    {
        CPPUNIT_ASSERT(m_xText->copyText(1, nCharCount - 1));
        sClipboardText = getSystemClipboardText();
        CPPUNIT_ASSERT_EQUAL(OUString(sText.subView(1, nCharCount - 2)), sClipboardText);
    }
}

OUString XAccessibleTextTester::getSystemClipboardText()
{
    css::uno::Reference<css::datatransfer::clipboard::XSystemClipboard> xClipboard
        = css::datatransfer::clipboard::SystemClipboard::create(
            comphelper::getProcessComponentContext());

    css::uno::Reference<css::datatransfer::XTransferable> xTrans = xClipboard->getContents();

    css::uno::Sequence<css::datatransfer::DataFlavor> aDataFlavors
        = xTrans->getTransferDataFlavors();

    for (const css::datatransfer::DataFlavor& rDataFlavor : aDataFlavors)
    {
        if (rDataFlavor.MimeType.startsWith(u"text/plain"))
        {
            css::uno::Any aData = xTrans->getTransferData(rDataFlavor);
            OUString sText;
            if (aData >>= sText)
                return sText;
        }
    }

    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
