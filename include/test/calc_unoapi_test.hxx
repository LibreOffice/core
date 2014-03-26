/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

// basic uno api test class for calc

class OOO_DLLPUBLIC_TEST CalcUnoApiTest : public UnoApiTest
{
public:
    CalcUnoApiTest(const OUString& path);

    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

private:
    css::uno::Reference<css::uno::XInterface> m_xCalcComponent;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
