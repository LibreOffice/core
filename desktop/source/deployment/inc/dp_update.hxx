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


#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/deployment/XExtensionManager.hpp>
#include <com/sun/star/deployment/XUpdateInformationProvider.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>

#include "dp_misc_api.hxx"

#include <map>
#include <vector>

namespace dp_misc {

/** returns the default update URL (for the update information) which
    is used when an extension does not provide its own URL.
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
OUString getExtensionDefaultUpdateURL();

enum UPDATE_SOURCE
{
    UPDATE_SOURCE_NONE,
    UPDATE_SOURCE_SHARED,
    UPDATE_SOURCE_BUNDLED,
    UPDATE_SOURCE_ONLINE
};

/* determine if an update is available which is installed in the
   user repository.

   If the return value is UPDATE_SOURCE_NONE, then no update is
   available, otherwise the return value determine from which the
   repository the update is used.
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
UPDATE_SOURCE isUpdateUserExtension(
    bool bReadOnlyShared,
    OUString const & userVersion,
    OUString const & sharedVersion,
    OUString const & bundledVersion,
    OUString const & onlineVersion);

/* determine if an update is available which is installed in the
   shared repository.

   If the return value is UPDATE_SOURCE_NONE, then no update is
   available, otherwise the return value determine from which the
   repository the update is used.
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
UPDATE_SOURCE isUpdateSharedExtension(
    bool bReadOnlyShared,
    OUString const & sharedVersion,
    OUString const & bundledVersion,
    OUString const & onlineVersion);

/* determines the extension with the highest identifier and returns it

 */
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
css::uno::Reference< css::deployment::XPackage>
getExtensionWithHighestVersion(
    css::uno::Sequence<
    css::uno::Reference<
    css::deployment::XPackage> > const & seqExtensionsWithSameId);


struct UpdateInfo
{
    UpdateInfo( css::uno::Reference< css::deployment::XPackage> const & ext);

    css::uno::Reference<  css::deployment::XPackage> extension;
    //version of the update
    OUString version;
    css::uno::Reference< css::xml::dom::XNode > info;
};

typedef std::map< OUString, UpdateInfo > UpdateInfoMap;

/*
  @param extensionList
  List of extension for which online update information is to be obtained. If NULL, then
  for update information is obtained for all installed extension. There may be only one extension
  with a particular identifier contained in the list. If one extension is installed
  in several repositories, then the one with the highest version must be used, because it contains
  the more recent URLs for getting the update information (if at all).
  @param out_errors
  the first member of the pair is the extension  and the second the exception that was produced
  when processing the extension.

  @return
  A map of UpdateInfo instances. If the parameter extensionList was given, then the map contains
  at only information for those extensions.
 */
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
UpdateInfoMap getOnlineUpdateInfos(
    css::uno::Reference< css::uno::XComponentContext> const &xContext,
    css::uno::Reference< css::deployment::XExtensionManager> const & xExtMgr,
    css::uno::Reference< css::deployment::XUpdateInformationProvider > const & updateInformation,
    std::vector< css::uno::Reference< css::deployment::XPackage > > const * extensionList,
    std::vector< std::pair< css::uno::Reference<
    css::deployment::XPackage>, css::uno::Any> > & out_errors);

/* returns the highest version from the provided arguments.
*/
DESKTOP_DEPLOYMENTMISC_DLLPUBLIC
OUString getHighestVersion(
    OUString const & sharedVersion,
    OUString const & bundledVersion,
    OUString const & onlineVersion);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
