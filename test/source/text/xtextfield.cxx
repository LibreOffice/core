/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "test/text/xtextfield.hxx"
#include <com/sun/star/text/XTextField.hpp>
#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star::uno;

namespace apitest
{

void XTextField::testGetPresentation()
{
    uno::Reference< text::XTextField > xTextField(init(), UNO_QUERY_THROW);
    OUString aString = xTextField->getPresentation(true);
    CPPUNIT_ASSERT(!aString.isEmpty());
    aString = xTextField->getPresentation(false);
    CPPUNIT_ASSERT(!aString.isEmpty());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
