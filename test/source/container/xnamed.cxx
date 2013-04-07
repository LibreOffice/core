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
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
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

#include <com/sun/star/container/XNamed.hpp>
#include <test/container/xnamed.hxx>
#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star::uno;

namespace apitest {

void XNamed::testGetName()
{
    uno::Reference< container::XNamed > xNamed(init(), UNO_QUERY_THROW);
    CPPUNIT_ASSERT( maName == xNamed->getName() );
}

void XNamed::testSetName()
{
    uno::Reference< container::XNamed > xNamed(init(), UNO_QUERY_THROW);
    OUString aName("NewName");

    xNamed->setName(aName);
    CPPUNIT_ASSERT( aName == xNamed->getName() );

    // restore old name
    xNamed->setName(maName);
    CPPUNIT_ASSERT(maName == xNamed->getName());
}

XNamed::~XNamed()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
