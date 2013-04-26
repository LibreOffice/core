/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/container/xelementaccess.hxx>
#include <com/sun/star/container/XElementAccess.hpp>
#include "cppunit/extensions/HelperMacros.h"

using namespace css;
using namespace css::uno;

namespace apitest {

void XElementAccess::testGetElementType()
{
    uno::Reference< container::XElementAccess > xElementAccess(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xElementAccess->getElementType()==maType);
}

void XElementAccess::testHasElements()
{
    uno::Reference< container::XElementAccess > xElementAccess(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xElementAccess->hasElements());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
