/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/graph.hxx>

namespace weld
{
class Window;
}

namespace vcl
{
namespace graphic
{
Graphic VCL_DLLPUBLIC loadFromURL(OUString const& rURL, weld::Window* pParentWin = nullptr);
}
} // end vcl::graphic


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
