/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_TEXT_BASEINDEXTEST_HXX
#define INCLUDED_TEST_TEXT_BASEINDEXTEST_HXX

#include <test/testdllapi.hxx>

namespace apitest
{
class OOO_DLLPUBLIC_TEST BaseIndex
{
public:
    virtual css::uno::Reference<css::uno::XInterface> init() = 0;
    virtual ~BaseIndex();
    void testBaseIndexProperties();
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
