/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "VCLKDEApplication.hxx"

#include <QClipboard>
#include <QEvent>

#include "KDESalDisplay.hxx"

VCLKDEApplication::VCLKDEApplication() :
    KApplication()
{
}

bool VCLKDEApplication::x11EventFilter(XEvent* ev)
{
    //if we have a display and the display consumes the event
    //do not process the event in qt
    if (SalKDEDisplay::self() && SalKDEDisplay::self()->Dispatch(ev))
    {
        return true;
    }

    return false;
}

// various hacks to be performed before re-entering Qt's event loop
// because of showing a Qt dialog
void VCLKDEApplication::preDialogSetup()
{
    // KFileDialog integration requires using event loop with QClipboard.
    // Opening the KDE file dialog here can lead to QClipboard
    // asking for clipboard contents. If LO core is the owner of the clipboard
    // content, without event loop use this will block for 5 seconds and timeout,
    // since the clipboard thread will not be able to acquire SolarMutex
    // and thus won't be able to respond. If the event loops
    // are properly integrated and QClipboard can use a nested event loop
    // (see the KDE VCL plug), then this won't happen.
    // We cannot simply release SolarMutex here, because the event loop started
    // by the file dialog would also call back to LO code.
    assert( QApplication::clipboard()->property( "useEventLoopWhenWaiting" ).toBool() );
}

// various hacks to be performed after a Qt dialog has been closed
void VCLKDEApplication::postDialogCleanup()
{
    // HACK: KFileDialog uses KConfig("kdeglobals") for saving some settings
    // (such as the auto-extension flag), but that doesn't update KGlobal::config()
    // (which is probably a KDE bug), so force reading the new configuration,
    // otherwise the next opening of the dialog would use the old settings.
    KGlobal::config()->reparseConfiguration();
    // HACK: If Qt owns clipboard or selection, give up on their ownership now. Otherwise
    // LO core might ask for the contents, but it would block while doing so (i.e. it
    // doesn't seem to have an equivalent of QClipboard's "useEventLoopWhenWaiting"),
    // therefore QClipboard wouldn't be able to respond, and whole LO would block until
    // a timeout. Given that Klipper is most probably running, giving up clipboard/selection
    // ownership will not only avoid the blocking, but even pasting that content in LO
    // will in fact work, if Klipper can handle it.
    // Technically proper solution would be of course to allow Qt to process QClipboard
    // events while LO waits for clipboard contents, or short-circuit to QClipboard somehow
    // (it's a mystery why LO's clipboard handling has its own thread when whole LO can
    // get blocked by both trying to send and receive clipboard contents anyway).
    QClipboard* clipboard = QApplication::clipboard();
    if( clipboard->ownsSelection())
        clipboard->clear( QClipboard::Selection );
    if( clipboard->ownsClipboard())
        clipboard->clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
