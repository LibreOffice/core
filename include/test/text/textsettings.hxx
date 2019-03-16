/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_TEXT_TEXTSETTINGS_HXX
#define INCLUDED_TEST_TEXT_TEXTSETTINGS_HXX

namespace apitest
{
class OOO_DLLPUBLIC_TEST TextSettings
{
public:
    virtual css::uno::Reference<css::uno::XInterface> init() = 0;
    virtual ~TextSettings();
    void testSettingsProperties();
};
} // end namespace apitest

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
