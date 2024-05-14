/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/text/textcontent.hxx>

#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

using namespace css;

namespace apitest
{
void TextContent::testTextContentProperties()
{
    uno::Reference<beans::XPropertySet> xPS(init(), uno::UNO_QUERY_THROW);

    text::TextContentAnchorType aExpectedTCAT;
    CPPUNIT_ASSERT(xPS->getPropertyValue(u"AnchorType"_ustr) >>= aExpectedTCAT);
    CPPUNIT_ASSERT_EQUAL(m_aExpectedTCAT, aExpectedTCAT);

    try
    {
        xPS->setPropertyValue(u"AnchorType"_ustr, uno::Any(m_aNewTCAT));
        text::TextContentAnchorType aNewTCAT;
        CPPUNIT_ASSERT(xPS->getPropertyValue(u"AnchorType"_ustr) >>= aNewTCAT);
        CPPUNIT_ASSERT_EQUAL(m_aNewTCAT, aNewTCAT);
    }
    catch (const beans::UnknownPropertyException& /* ex */)
    {
        // ignore if property is optional
    }

    uno::Sequence<text::TextContentAnchorType> aAnchorTypes;
    CPPUNIT_ASSERT(xPS->getPropertyValue(u"AnchorTypes"_ustr) >>= aAnchorTypes);
    CPPUNIT_ASSERT(aAnchorTypes.hasElements());

    text::WrapTextMode aExpectedWTM;
    CPPUNIT_ASSERT(xPS->getPropertyValue(u"TextWrap"_ustr) >>= aExpectedWTM);
    CPPUNIT_ASSERT_EQUAL(m_aExpectedWTM, aExpectedWTM);

    try
    {
        xPS->setPropertyValue(u"TextWrap"_ustr, uno::Any(m_aNewWTM));
        text::WrapTextMode aNewWTM;
        CPPUNIT_ASSERT(xPS->getPropertyValue(u"TextWrap"_ustr) >>= aNewWTM);
        CPPUNIT_ASSERT_EQUAL(m_aNewWTM, aNewWTM);
    }
    catch (const beans::UnknownPropertyException& /* ex */)
    {
        // ignore if property is optional
    }
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
