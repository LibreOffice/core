/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_DP_GUI_EXTENSIONCMDQUEUE_HXX
#define INCLUDED_DP_GUI_EXTENSIONCMDQUEUE_HXX

#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "rtl/ref.hxx"

#include <vector>

#include "dp_gui_updatedata.hxx"

/// @HTML

namespace com { namespace sun { namespace star {
    namespace task { class XInteractionRequest; }
    namespace uno { class XComponentContext; }
} } }

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
                       const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & rContext);

    ~ExtensionCmdQueue();

    /**
    */
    void addExtension( const ::rtl::OUString &rExtensionURL,
                       const ::rtl::OUString &rRepository,
                       const bool bWarnUser );
    void removeExtension( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &rPackage );
    void enableExtension( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &rPackage,
                          const bool bEnable );
    void checkForUpdates(const std::vector< ::com::sun::star::uno::Reference<
                         ::com::sun::star::deployment::XPackage > > &vList );
    void acceptLicense( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &rPackage );
    static void syncRepositories( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext );
    /**
       This call does not block. It signals the internal thread
       that it should install the remaining extensions and then terminate.
    */
    void stop();

    bool isBusy();
private:
    ExtensionCmdQueue(ExtensionCmdQueue &); // not defined
    void operator =(ExtensionCmdQueue &); // not defined

    class Thread;

    rtl::Reference< Thread > m_thread;
};

void handleInteractionRequest( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext,
                               const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest > & xRequest );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
