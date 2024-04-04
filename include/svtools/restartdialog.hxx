/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <svtools/svtdllapi.h>

namespace com :: sun :: star :: uno { template <typename > class Reference; }

namespace weld { class Window; }
namespace com::sun::star::uno {
    class XComponentContext;
}

namespace svtools {

enum RestartReason {
    // "For the selected Java runtime environment to work properly,
    // %PRODUCTNAME must be restarted.":
    RESTART_REASON_JAVA,
    // "For the bibliography to work properly,
    // %PRODUCTNAME must be restarted.":
    RESTART_REASON_BIBLIOGRAPHY_INSTALL,
    // "For the mail merge to work properly,
    // %PRODUCTNAME must be restarted.":
    RESTART_REASON_MAILMERGE_INSTALL,
    // "For the updated language settings to take effect,
    // %PRODUCTNAME must be restarted.":
    RESTART_REASON_LANGUAGE_CHANGE,
    // "For the added path to take effect,
    // %PRODUCTNAME must be restarted.":
    RESTART_REASON_ADDING_PATH,
    // "For the assigned java parameters to take effect,
    // %PRODUCTNAME must be restarted.":
    RESTART_REASON_ASSIGNING_JAVAPARAMETERS,
    // "For the assigned folders and archives to take effect,
    // %PRODUCTNAME must be restarted.":
    RESTART_REASON_ASSIGNING_FOLDERS,
    // "For the modified experimental features to take effect,
    // %PRODUCTNAME must be restarted.":
    RESTART_REASON_EXP_FEATURES,
    // "For the extension to work properly,
    // %PRODUCTNAME must be restarted.":
    RESTART_REASON_EXTENSION_INSTALL,
    // For the Skia changes to take effect,
    // %PRODUCTNAME must be restarted:
    RESTART_REASON_SKIA,
    // For the OpenCL changes to take effect,
    // %PRODUCTNAME must be restarted:
    RESTART_REASON_OPENCL,
    // For the multi-threaded calculation changes to take effect,
    // %PRODUCTNAME must be restarted:
    RESTART_REASON_THREADING,
    // To apply changes, %PRODUCTNAME,
    // %PRODUCTNAME must be restarted:
    RESTART_REASON_UI_CHANGE,
    // No particular reason:
    RESTART_REASON_NONE
};

// Must be called with the solar mutex locked; returns if restart was initiated:
SVT_DLLPUBLIC bool executeRestartDialog(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    weld::Window* parent, RestartReason reason);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
