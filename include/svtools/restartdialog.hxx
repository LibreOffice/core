/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVTOOLS_RESTARTDIALOG_HXX
#define INCLUDED_SVTOOLS_RESTARTDIALOG_HXX

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <svtools/svtdllapi.h>

namespace vcl { class Window; }
namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace svtools {

enum RestartReason {
    RESTART_REASON_JAVA,
        // "For the selected Java runtime environment to work properly,
        // %PRODUCTNAME must be restarted."
    RESTART_REASON_PDF_AS_STANDARD_JOB_FORMAT,
        // For the modified default print job format to take effect,
        // %PRODUCTNAME must be restarted.
    RESTART_REASON_BIBLIOGRAPHY_INSTALL,
        // "For the bibliography to work properly,
        // %PRODUCTNAME must be restarted."
    RESTART_REASON_MAILMERGE_INSTALL,
        // "For the mail merge to work properly,
        // %PRODUCTNAME must be restarted."
    RESTART_REASON_LANGUAGE_CHANGE,
        // "For the updated language settings to take effect,
        // %PRODUCTNAME must be restarted."
    RESTART_REASON_ADDING_PATH,
        // "For the added path to take effect,
        // %PRODUCTNAME must be restarted."
    RESTART_REASON_ASSIGNING_JAVAPARAMETERS,
        // "For the assigned java parameters to take effect,
        // %PRODUCTNAME must be restarted."
    RESTART_REASON_ASSIGNING_FOLDERS,
        // "For the assigned folders and archives to take effect,
        // %PRODUCTNAME must be restarted."
    RESTART_REASON_EXP_FEATURES,
        // "For the modified experimental features to take effect,
        // %PRODUCTNAME must be restarted."
    RESTART_REASON_EXTENSION_INSTALL,
        // "For the extension to work properly,
        // %PRODUCTNAME must be restarted."
    RESTART_REASON_OPENGL,
};

// Must be called with the solar mutex locked:
SVT_DLLPUBLIC void executeRestartDialog(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    vcl::Window * parent, RestartReason reason);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
