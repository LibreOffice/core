/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <unx/gensys.h>

class Qt5System : public SalGenericSystem
{
public:
    virtual unsigned int GetDisplayScreenCount() override;
    virtual tools::Rectangle GetDisplayScreenPosSizePixel(unsigned int nScreen) override;
    virtual int ShowNativeDialog(const OUString& rTitle, const OUString& rMessage,
                                 const std::vector<OUString>& rButtons) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
