/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>
#include <vcl/dllapi.h>
#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star::lang
{
class XInitialization;
}
namespace com::sun::star::uno
{
class XInterface;
}

namespace vcl
{
// Ole and X11 refer to the UNO DnD interface names and their expected XInitialization arguments.

enum class DragOrDrop
{
    Drag,
    Drop
};
VCL_DLLPUBLIC void OleDnDHelper(const css::uno::Reference<css::lang::XInitialization>&,
                                sal_IntPtr pWin, DragOrDrop);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
