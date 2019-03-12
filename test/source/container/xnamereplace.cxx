/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/container/xnamereplace.hxx>

#include <com/sun/star/container/XNameReplace.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;

namespace apitest
{
void XNameReplace::testReplaceByName()
{
    uno::Reference<container::XNameReplace> xNameReplace(init(), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_NO_THROW(xNameReplace->replaceByName(m_aReplacementName, m_aReplacementElement));
    uno::Any aAny = xNameReplace->getByName(m_aReplacementName);

    // we deliberately avoid uno::UNO_QUERY_THROW, so a test on .is() can be made
    uno::Reference<uno::XInterface> xElement(aAny, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xElement.is());
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
