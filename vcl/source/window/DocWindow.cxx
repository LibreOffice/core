/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <sal/log.hxx>
#include <vcl/DocWindow.hxx>
#include <vcl/ITiledRenderable.hxx>

namespace vcl
{
void DocWindow::SetPointer(PointerStyle nPointer)
{
    Window::SetPointer(nPointer);

    VclPtr<vcl::Window> pWin = GetParentWithLOKNotifier();
    if (!pWin)
        return;

    PointerStyle aPointer = GetPointer();
    // We don't map all possible pointers hence we need a default
    OString aPointerString = "default"_ostr;
    auto aIt = vcl::gaLOKPointerMap.find(aPointer);
    if (aIt != vcl::gaLOKPointerMap.end())
    {
        aPointerString = aIt->second;
    }

    pWin->GetLOKNotifier()->libreOfficeKitViewCallback(LOK_CALLBACK_MOUSE_POINTER, aPointerString);
}
} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
