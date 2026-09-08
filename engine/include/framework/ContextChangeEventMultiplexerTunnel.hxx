/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <framework/fwkdllapi.h>
#include <functional>

#include <cpo/uno/Reference.hxx>

namespace com::sun::star::ui { class XContextChangeEventListener; }
namespace cpo::uno { class XInterface; }
namespace cpo::uno { class XComponentContext; }

namespace framework {

// this is pretty horrible, don't use it!
FWK_DLLPUBLIC cpo::uno::Reference<css::ui::XContextChangeEventListener>
GetFirstListenerWith(
    cpo::uno::Reference<cpo::uno::XComponentContext> const & xComponentContext,
    cpo::uno::Reference<cpo::uno::XInterface> const& xEventFocus,
    std::function<bool (cpo::uno::Reference<css::ui::XContextChangeEventListener> const&)> const& rPredicate);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
