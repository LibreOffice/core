/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/weld/Dialog.hxx>

namespace weld
{
class VCL_DLLPUBLIC MessageDialog : virtual public Dialog
{
public:
    virtual void set_primary_text(const OUString& rText) = 0;
    virtual OUString get_primary_text() const = 0;
    virtual void set_secondary_text(const OUString& rText) = 0;
    virtual OUString get_secondary_text() const = 0;
    virtual std::unique_ptr<Container> weld_message_area() = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
