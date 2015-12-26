/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/container/XNamed.hpp>
#include <test/container/xnamed.hxx>
#include "cppunit/extensions/HelperMacros.h"

using namespace css;
using namespace css::uno;

namespace apitest {

void XNamed::testGetName()
{
    uno::Reference< container::XNamed > xNamed(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT( maTestName == xNamed->getName() );
}

void XNamed::testSetName()
{
    uno::Reference< container::XNamed > xNamed(init(), UNO_QUERY_THROW);
    OUString aTestName("NewName");

    xNamed->setName(aTestName);
    CPPUNIT_ASSERT( aTestName == xNamed->getName() );

    // restore old name
    xNamed->setName(maTestName);
    CPPUNIT_ASSERT(maTestName == xNamed->getName());
}

XNamed::~XNamed()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
