/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_UNOAPI_TEST_HXX
#define INCLUDED_TEST_UNOAPI_TEST_HXX

#include <sal/config.h>

#include <string_view>

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <osl/file.hxx>

// basic uno api test class

class OOO_DLLPUBLIC_TEST UnoApiTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    UnoApiTest(const OUString& path);

    void createFileURL(std::u16string_view aFileBase, OUString& rFilePath);

    virtual void setUp() override;

protected:
    void closeDocument(css::uno::Reference<css::lang::XComponent> const& xDocument);

private:
    OUString m_aBaseString;
};

#endif // INCLUDED_TEST_UNOAPI_TEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
