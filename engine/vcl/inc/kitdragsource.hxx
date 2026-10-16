/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <cpo/uno/Reference.hxx>

namespace com::sun::star::datatransfer
{
class XTransferable;
}

namespace vcl
{
/// A drag source whose drag runs inside a kit process, where no system drag-and-drop loop shows
/// the drag to a drop target or reports how it ended. It hands out the data being dragged and
/// takes the outcome of the drag.
class SAL_LOPLUGIN_ANNOTATE("crosscast") KitDragSource
{
public:
    /// The data being dragged, or empty while no drag runs.
    virtual cpo::uno::Reference<css::datatransfer::XTransferable> getKitDragTransferable() = 0;

    /// Ends the running drag and tells its listener the outcome. nDropAction is the DNDConstants
    /// action the drop carried out, or ACTION_NONE when the drag ended without a drop.
    virtual void endKitDrag(sal_Int8 nDropAction) = 0;

protected:
    ~KitDragSource() = default;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
