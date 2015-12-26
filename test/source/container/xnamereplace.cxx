/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/container/xnamereplace.hxx>
#include <com/sun/star/container/XNameReplace.hpp>
#include "cppunit/extensions/HelperMacros.h"

using namespace css;
using namespace css::uno;

namespace apitest {

void XNameReplace::testReplaceByName()
{
    uno::Reference< container::XNameReplace > xNameReplace(init(), UNO_QUERY_THROW);
    xNameReplace->replaceByName(maTestReplacementName, getAnyElementForNameReplace());
    Any aAny = xNameReplace->getByName( maTestReplacementName );
    uno::Reference< uno::XInterface > xElement(aAny, UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xElement.is());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
