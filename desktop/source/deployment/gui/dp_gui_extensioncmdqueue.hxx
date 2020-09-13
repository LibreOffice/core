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

#pragma once

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>

#include <vector>

#include "dp_gui_updatedata.hxx"

/// @HTML

namespace com::sun::star {
    namespace task { class XInteractionRequest; }
    namespace uno { class XComponentContext; }
}

namespace dp_gui {

class DialogHelper;
class TheExtensionManager;

/**
   Manages installing of extensions in the GUI mode. Requests for installing
   Extensions can be asynchronous. For example, the Extension Manager is running
   in an office process and someone uses the system integration to install an Extension.
   That is, the user double clicks an extension symbol in a file browser, which then
   causes an invocation of "unopkg gui ext". When at that time the Extension Manager
   already performs a task, triggered by the user (for example, add, update, disable,
   enable) then adding of the extension will be postponed until the user has finished
   the task.

   This class also ensures that the extensions are not installed in the main thread.
   Doing so would cause a deadlock because of the progress bar which needs to be constantly
   updated.
*/
class ExtensionCmdQueue {

public:
    /**
       Create an instance.
    */
    ExtensionCmdQueue( DialogHelper * pDialogHelper,
                       TheExtensionManager *pManager,
                       const css::uno::Reference< css::uno::XComponentContext > & rContext);

    ~ExtensionCmdQueue();

    void addExtension( const OUString &rExtensionURL,
                       const OUString &rRepository,
                       const bool bWarnUser );
    void removeExtension( const css::uno::Reference< css::deployment::XPackage > &rPackage );
    void enableExtension( const css::uno::Reference< css::deployment::XPackage > &rPackage,
                          const bool bEnable );
    void checkForUpdates(const std::vector< css::uno::Reference<
                         css::deployment::XPackage > > &vList );
    void acceptLicense( const css::uno::Reference< css::deployment::XPackage > &rPackage );
    static void syncRepositories( const css::uno::Reference< css::uno::XComponentContext > & xContext );
    /**
       This call does not block. It signals the internal thread
       that it should install the remaining extensions and then terminate.
    */
    void stop();

    bool isBusy();
private:
    ExtensionCmdQueue(ExtensionCmdQueue const &) = delete;
    ExtensionCmdQueue& operator =(ExtensionCmdQueue const &) = delete;

    class Thread;

    rtl::Reference< Thread > m_thread;
};

void handleInteractionRequest( const css::uno::Reference< css::uno::XComponentContext > & xContext,
                               const css::uno::Reference< css::task::XInteractionRequest > & xRequest );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
