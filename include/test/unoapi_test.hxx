/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <rtl/ustrbuf.hxx>
#include <osl/file.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

// basic uno api test class

class OOO_DLLPUBLIC_TEST UnoApiTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    UnoApiTest(const OUString& path);

    void createFileURL(const OUString& aFileBase, OUString& rFilePath);

    virtual void setUp();
    virtual void tearDown();

protected:
    void closeDocument( uno::Reference< lang::XComponent > xDocument );


private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
    OUString m_aBaseString;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
