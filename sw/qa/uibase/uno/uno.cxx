/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/util/XCloseable.hpp>

/// Covers sw/source/uibase/uno/ fixes.
class SwUibaseUnoTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwUibaseUnoTest, testLockControllers)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    {
        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY_THROW);
        xModel->lockControllers();
    }
    {
        uno::Reference<util::XCloseable> xCloseable(mxComponent, uno::UNO_QUERY_THROW);
        xCloseable->close(false);
    }
    // Without the accompanying fix in place, this test would have crashed.
    mxComponent.clear();
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
