/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestAssert.h>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <test/unoapi_property_testers.hxx>
#include <test/text/footnote.hxx>

namespace apitest
{
Footnote::~Footnote() {}

void Footnote::testFootnoteProperties()
{
    css::uno::Reference<css::beans::XPropertySet> xFootnote(init(), css::uno::UNO_QUERY_THROW);

    short aReferenceIdGet;
    CPPUNIT_ASSERT_MESSAGE("Unable to get PropertyValue ReferenceId",
                           xFootnote->getPropertyValue(u"ReferenceId"_ustr) >>= aReferenceIdGet);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
