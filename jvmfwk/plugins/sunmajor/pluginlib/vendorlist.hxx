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
#if !defined INCLUDED_JFW_PLUGIN_VENDORLIST_HXX
#define INCLUDED_JFW_PLUGIN_VENDORLIST_HXX

#include "rtl/ref.hxx"
#include "vendorbase.hxx"
#include "com/sun/star/uno/Sequence.hxx"

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

#define BEGIN_VENDOR_MAP() \
VendorSupportMapEntry gVendorMap[] ={

#define VENDOR_MAP_ENTRY(x,y) \
    {x, & y::getJavaExePaths, & y::createInstance},

#define END_VENDOR_MAP() \
    {NULL, NULL, NULL} };


com::sun::star::uno::Sequence<rtl::OUString> getVendorNames();

/* Examines if the vendor supplied in parameter sVendor is part of the
   list of supported vendors. That is the arry of VendorSupportMapEntry
   is search for an respective entry.
*/
bool isVendorSupported(const rtl::OUString & sVendor);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
