/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test/text/xtextcontent.hxx"
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include "cppunit/extensions/HelperMacros.h"

using namespace css;
using namespace css::uno;

namespace apitest {

void XTextContent::testGetAnchor()
{
    uno::Reference< text::XTextContent > xTextContent(init(), UNO_QUERY_THROW);
    uno::Reference< uno::XInterface > xAnchor(xTextContent->getAnchor());
    CPPUNIT_ASSERT(xAnchor.is());
}

void XTextContent::testAttach()
{
    uno::Reference< text::XTextContent > xTextContent(init(), UNO_QUERY_THROW);
    uno::Reference< text::XTextRange > xAnchor = xTextContent->getAnchor();

    uno::Reference< text::XTextContent > xContent(getTextContent(), UNO_QUERY_THROW);
    uno::Reference< text::XTextRange > xRange(getTextRange(), UNO_QUERY_THROW);

    try
    {
        if (xContent.is())
            xContent->attach(xRange);
        else
            xTextContent->attach(xRange);
    }
    catch (const lang::IllegalArgumentException&)
    {
        bool bAttachSupported = isAttachSupported();
        if (bAttachSupported)
            CPPUNIT_ASSERT(false);
    }
    catch (const RuntimeException&)
    {
        bool bAttachSupported = isAttachSupported();
        if (bAttachSupported)
            CPPUNIT_ASSERT(false);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
