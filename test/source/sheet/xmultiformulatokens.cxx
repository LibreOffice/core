/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xmultiformulatokens.hxx>

#include <com/sun/star/sheet/FormulaToken.hpp>
#include <com/sun/star/sheet/XMultiFormulaTokens.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/extensions/HelperMacros.h>

using namespace css;

namespace apitest
{
void XMultiFormulaTokens::testGetCount()
{
    uno::Reference<sheet::XMultiFormulaTokens> xMFT(init(), uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), xMFT->getCount());
}

void XMultiFormulaTokens::testGetSetTokens()
{
    uno::Reference<sheet::XMultiFormulaTokens> xMFT(init(), uno::UNO_QUERY_THROW);

    uno::Sequence<sheet::FormulaToken> aTokens(1);
    aTokens[0].OpCode = 2;
    xMFT->setTokens(0, aTokens);

    CPPUNIT_ASSERT_EQUAL(aTokens[0].OpCode, xMFT->getTokens(0)[0].OpCode);
}

} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
