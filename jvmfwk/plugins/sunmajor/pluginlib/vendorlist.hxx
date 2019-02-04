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
#ifndef INCLUDED_JVMFWK_PLUGINS_SUNMAJOR_PLUGINLIB_VENDORLIST_HXX
#define INCLUDED_JVMFWK_PLUGINS_SUNMAJOR_PLUGINLIB_VENDORLIST_HXX

#include <rtl/ref.hxx>
#include <vendorbase.hxx>

namespace jfw_plugin
{

typedef char  const * const * (* getJavaExePaths_func)(int*);
typedef rtl::Reference<VendorBase> (* createInstance_func) ();

struct VendorSupportMapEntry
{
    char const * sVendorName;
    getJavaExePaths_func getJavaFunc;
    createInstance_func  createFunc;
};

extern VendorSupportMapEntry const gVendorMap[];

template<typename y> constexpr VendorSupportMapEntry VENDOR_MAP_ENTRY(char const * x) {
    return {x, & y::getJavaExePaths, & y::createInstance};
}

/* Examines if the vendor supplied in parameter sVendor is part of the
   list of supported vendors. That is the arry of VendorSupportMapEntry
   is search for an respective entry.
*/
bool isVendorSupported(const OUString & sVendor);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
