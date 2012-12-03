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

#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "svtools/svtdllapi.h"

class Window;
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
    RESTART_REASON_BIBLIOGRAPHY_INSTALL
        // "For the bibliography to work properly,
        // %PRODUCTNAME must be restarted."
};

// Must be called with the solar mutex locked:
SVT_DLLPUBLIC void executeRestartDialog(
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
        const & context,
    Window * parent, RestartReason reason);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
