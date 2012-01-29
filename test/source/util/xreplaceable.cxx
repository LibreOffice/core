/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/util/xreplaceable.hxx>

#include <com/sun/star/util/XReplaceable.hpp>
#include <com/sun/star/util/XReplaceDescriptor.hpp>
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include "cppunit/extensions/HelperMacros.h"

#include <iostream>

using namespace com::sun::star::uno;

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

    //check that at least one object is there that will be replaced
    uno::Reference< uno::XInterface > xElement = xReplaceable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(xElement.is());

    //check that there is none object with the replace string
    xSearchDescr->setSearchString(maReplaceString);
    xElement = xReplaceable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(!xElement.is());

    xReplaceDescr->setSearchString(maSearchString);
    xReplaceDescr->setReplaceString(maReplaceString);

    xReplaceable->replaceAll(uno::Reference< util::XSearchDescriptor >(xReplaceDescr, UNO_QUERY_THROW));

    //check that now at least one element is found
    xElement = xReplaceable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(xElement.is());

    xSearchDescr->setSearchString(maSearchString);
    xElement = xReplaceable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(!xElement.is());

    //redo the whole thing
    xReplaceDescr->setSearchString(maReplaceString);
    xReplaceDescr->setReplaceString(maSearchString);

    xReplaceable->replaceAll(uno::Reference< util::XSearchDescriptor >(xReplaceDescr, UNO_QUERY_THROW));

    //check that it works
    xElement = xReplaceable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(xElement.is());

    //check that there is none object with the replace string
    xSearchDescr->setSearchString(maReplaceString);
    xElement = xReplaceable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(!xElement.is());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
