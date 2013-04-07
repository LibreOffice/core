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

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>

#include <vector>
#include "updateinfo.hxx"

// Returns 'true' if successfully connected to the update server
bool checkForUpdates(
    UpdateInfo& o_rUpdateInfo,
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& rxInteractionHandler,
    const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XUpdateInformationProvider >& rxProvider
);

// The same as above, that does not read the info from bootstrap
SAL_DLLPUBLIC_EXPORT bool
checkForUpdates(
    UpdateInfo& o_rUpdateInfo,
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & rxContext,
    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > & rxInteractionHandler,
    const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XUpdateInformationProvider >& rUpdateInfoProvider,
    const OUString &rOS,
    const OUString &rArch,
    const ::com::sun::star::uno::Sequence< OUString > &rRepositoryList,
    const OUString &rGitID,
    const OUString &rInstallID
);

// Returns 'true' if there are updates for any extension
bool checkForExtensionUpdates(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
);

bool checkForPendingUpdates(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
);

bool storeExtensionUpdateInfos(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< OUString > > &rUpdateInfos
);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
