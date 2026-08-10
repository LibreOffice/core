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

#include <com/sun/star/uno/Reference.hxx>
#include <sal/config.h>

namespace com::sun::star::frame
{
class XModel;
}
namespace scriptinterop
{
class XSpreadsheet;
}

namespace scriptinterop::detail
{
css::uno::Reference<XSpreadsheet>
createSpreadsheet(css::uno::Reference<css::frame::XModel> const& model);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
