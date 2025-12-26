/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vcl/dllapi.h>
#include <sal/types.h>

class BinaryDataContainer;
namespace com::sun::star::uno
{
template <class interface_type> class Reference;
}
namespace com::sun::star::util
{
class XBinaryDataContainer;
}

namespace vcl
{
VCL_DLLPUBLIC BinaryDataContainer convertUnoBinaryDataContainer(
    css::uno::Reference<css::util::XBinaryDataContainer> const& rxBinaryDataContainer);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
