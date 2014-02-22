/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <test/util/xreplaceable.hxx>

#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XReplaceDescriptor.hpp>
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include "cppunit/extensions/HelperMacros.h"

#include <iostream>

using namespace css;
using namespace css::uno;

namespace apitest {

void XReplaceable::testCreateReplaceDescriptor()
{
    uno::Reference< util::XReplaceable > xReplaceable(init(), UNO_QUERY_THROW);
    uno::Reference< util::XReplaceDescriptor> xReplaceDescr = xReplaceable->createReplaceDescriptor();
    CPPUNIT_ASSERT(xReplaceDescr.is());
}

void XReplaceable::testReplaceAll()
{
    std::cout << "testReplaceAll" << std::endl;
    uno::Reference< util::XReplaceable > xReplaceable(init(), UNO_QUERY_THROW);
    uno::Reference< util::XReplaceDescriptor> xReplaceDescr = xReplaceable->createReplaceDescriptor();
    CPPUNIT_ASSERT(xReplaceDescr.is());

    uno::Reference< util::XSearchDescriptor > xSearchDescr = xReplaceable->createSearchDescriptor();
    xSearchDescr->setSearchString(maSearchString);

    
    uno::Reference< uno::XInterface > xElement = xReplaceable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(xElement.is());

    
    xSearchDescr->setSearchString(maReplaceString);
    xElement = xReplaceable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(!xElement.is());

    xReplaceDescr->setSearchString(maSearchString);
    xReplaceDescr->setReplaceString(maReplaceString);

    xReplaceable->replaceAll(uno::Reference< util::XSearchDescriptor >(xReplaceDescr, UNO_QUERY_THROW));

    
    xElement = xReplaceable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(xElement.is());

    xSearchDescr->setSearchString(maSearchString);
    xElement = xReplaceable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(!xElement.is());

    
    xReplaceDescr->setSearchString(maReplaceString);
    xReplaceDescr->setReplaceString(maSearchString);

    xReplaceable->replaceAll(uno::Reference< util::XSearchDescriptor >(xReplaceDescr, UNO_QUERY_THROW));

    
    xElement = xReplaceable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(xElement.is());

    
    xSearchDescr->setSearchString(maReplaceString);
    xElement = xReplaceable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(!xElement.is());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
