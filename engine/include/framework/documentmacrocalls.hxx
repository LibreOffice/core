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

#include <framework/fwkdllapi.h>
#include <com/sun/star/ui/XUIConfigurationManager2.hpp>

#include <string_view>

namespace framework
{
/** Whether a command URL runs a macro or a script, or names one in its arguments. A .uno:, slot:
    or findbar command is not a macro call, unless its arguments name a macro: or
    vnd.sun.star.script: URL or a Referer.
 */
FWK_DLLPUBLIC bool isMacroCallCommand(std::u16string_view rCommand);

/** Sets whether the macros of the document that owns rxManager may run. The macro call commands
    of the document's own UI configuration are left out of its settings and key bindings until
    this is set to true.
 */
FWK_DLLPUBLIC void
setDocumentMacroCallsAllowed(const cpo::uno::Reference<css::ui::XUIConfigurationManager2>& rxManager,
                             bool bAllowed);

/** Whether the document's own UI configuration held by rxManager has a macro call command in any
    of its elements or key bindings.
 */
FWK_DLLPUBLIC bool documentConfigurationHasMacroCalls(
    const cpo::uno::Reference<css::ui::XUIConfigurationManager2>& rxManager);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
