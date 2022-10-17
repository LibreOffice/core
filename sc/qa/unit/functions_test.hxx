/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/types.h>

#include <unotest/filters-test.hxx>
#include "helper/qahelper.hxx"

#include <com/sun/star/uno/Reference.hxx>

class SCQAHELPER_DLLPUBLIC FunctionsTest : public ScBootstrapFixture, public test::FiltersTest
{
public:

    FunctionsTest(const OUString& rPath);

    virtual bool load(
        const OUString &rFilter,
        const OUString &rURL,
        const OUString &rUserData,
        SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID,
        unsigned int nFilterVersion) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
