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

#include "test/text/xtextcontent.hxx"
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star::uno;

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
