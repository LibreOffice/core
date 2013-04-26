/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/util/XSearchable.hpp>
#include <com/sun/star/util/XSearchDescriptor.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include "cppunit/extensions/HelperMacros.h"

#include <test/util/xsearchable.hxx>

using namespace css;
using namespace css::uno;

namespace apitest {

void XSearchable::testFindAll()
{
    uno::Reference< util::XSearchable > xSearchable(init(), UNO_QUERY_THROW);
    uno::Reference< util::XSearchDescriptor> xSearchDescr = xSearchable->createSearchDescriptor();
    xSearchDescr->setSearchString(maSearchString);

    uno::Reference< container::XIndexAccess > xIndex = xSearchable->findAll(xSearchDescr);
    CPPUNIT_ASSERT(xIndex.is());
    CPPUNIT_ASSERT(xIndex->getCount() == mnCount);
}

void XSearchable::testFindFirst()
{
    uno::Reference< util::XSearchable > xSearchable(init(), UNO_QUERY_THROW);
    uno::Reference< util::XSearchDescriptor> xSearchDescr = xSearchable->createSearchDescriptor();
    xSearchDescr->setSearchString(maSearchString);

    uno::Reference< uno::XInterface > xElement = xSearchable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(xElement.is());
}

void XSearchable::testFindNext()
{
    uno::Reference< util::XSearchable > xSearchable(init(), UNO_QUERY_THROW);
    uno::Reference< util::XSearchDescriptor> xSearchDescr = xSearchable->createSearchDescriptor();
    xSearchDescr->setSearchString(maSearchString);

    uno::Reference< uno::XInterface > xElement = xSearchable->findFirst(xSearchDescr);
    CPPUNIT_ASSERT(xElement.is());

    if (mnCount > 1)
    {
        uno::Reference< uno::XInterface > xElement2 = xSearchable->findNext(xElement, xSearchDescr);
        CPPUNIT_ASSERT(xElement2.is());
    }
}

XSearchable::~XSearchable()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
