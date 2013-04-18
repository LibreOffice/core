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
 * [ Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer) ]
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
