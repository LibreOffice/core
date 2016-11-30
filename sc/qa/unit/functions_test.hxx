/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <unotest/filters-test.hxx>
#include "scdll.hxx"
#include "helper/qahelper.hxx"

class SCQAHELPER_DLLPUBLIC FunctionsTest : public ScBootstrapFixture, public test::FiltersTest
{
public:

    FunctionsTest(const OUString& rPath);

    virtual void setUp() override;

    virtual bool load(
        const OUString &rFilter,
        const OUString &rURL,
        const OUString &rUserData,
        SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID,
        unsigned int nFilterVersion) override;

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
