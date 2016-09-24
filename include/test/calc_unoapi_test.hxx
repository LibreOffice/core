/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_CALC_UNOAPI_TEST_HXX
#define INCLUDED_TEST_CALC_UNOAPI_TEST_HXX

#include <test/unoapi_test.hxx>

// basic uno api test class for calc

class OOO_DLLPUBLIC_TEST CalcUnoApiTest : public UnoApiTest
{
public:
    CalcUnoApiTest(const OUString& path);

    virtual void setUp() override;
    virtual void tearDown() override;

private:
    css::uno::Reference<css::uno::XInterface> m_xCalcComponent;
};

#endif // INCLUDED_TEST_CALC_UNOAPI_TEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
