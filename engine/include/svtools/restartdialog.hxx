/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <svtools/svtdllapi.h>

namespace cpo::uno { template <typename > class Reference; }

namespace weld { class Window; }
namespace cpo::uno { class XComponentContext; }

namespace svtools {

enum RestartReason {
    // "For the bibliography to work properly,
    // %PRODUCTNAME must be restarted.":
    RESTART_REASON_BIBLIOGRAPHY_INSTALL,
    // "For the mail merge to work properly,
    // %PRODUCTNAME must be restarted.":
    RESTART_REASON_MAILMERGE_INSTALL,
    // To apply changes, %PRODUCTNAME,
    // %PRODUCTNAME must be restarted:
    RESTART_REASON_UI_CHANGE,
    // No particular reason:
    RESTART_REASON_NONE
};

// Must be called with the solar mutex locked; returns if restart was initiated:
SVT_DLLPUBLIC bool executeRestartDialog(
    cpo::uno::Reference< cpo::uno::XComponentContext > const & context,
    weld::Window* parent, RestartReason reason);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
