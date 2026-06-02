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
#include <vcl/weld/Container.hxx>

namespace weld
{
class VCL_DLLPUBLIC Grid : virtual public Container
{
public:
    virtual void set_child_left_attach(weld::Widget& rWidget, int nAttach) = 0;
    virtual int get_child_left_attach(weld::Widget& rWidget) const = 0;
    virtual void set_child_column_span(weld::Widget& rWidget, int nCols) = 0;
    virtual void set_child_top_attach(weld::Widget& rWidget, int nAttach) = 0;
    virtual int get_child_top_attach(weld::Widget& rWidget) const = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
