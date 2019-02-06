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

#include <config_folders.h>

#include <sal/config.h>

#include <cassert>

#include <com/sun/star/uno/DeploymentException.hpp>
#include <osl/file.hxx>
#include <osl/module.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include "paths.hxx"

namespace {

#ifndef ANDROID
OUString get_this_libpath() {
    static OUString s_uri = []() {
        OUString uri;
        osl::Module::getUrlFromAddress(reinterpret_cast<oslGenericFunction>(get_this_libpath), uri);
        sal_Int32 i = uri.lastIndexOf('/');
        if (i == -1)
        {
            throw css::uno::DeploymentException("URI " + uri + " is expected to contain a slash");
        }
        return uri.copy(0, i);
    }();

    return s_uri;
}
#endif
}

OUString cppu::getUnoIniUri() {
#if defined ANDROID
    // Wouldn't it be lovely to avoid this ugly hard-coding.
    // The problem is that the 'create_bootstrap_macro_expander_factory()'
    // required for bootstrapping services, calls cppu::get_unorc directly
    // instead of re-using the BootstrapHandle from:
    //     defaultBootstrap_InitialComponentContext
    // and since rtlBootstrapHandle is not ref-counted doing anything
    // clean here is hardish.
    OUString uri("file:///assets/program");
#else
    OUString uri(get_this_libpath());
#ifdef MACOSX
    // We keep both the LO and URE dylibs directly in "Frameworks"
    // (that is, LIBO_LIB_FOLDER) and rc files in "Resources"
    // (LIBO_ETC_FOLDER). Except for unorc, of which there are two,
    // the "LO" one (which is in "Resources") and the "URE" one (which
    // is in "Resources/ure/etc" (LIBO_URE_ETC_FOLDER)). As this code
    // goes into the cppuhelper library which is part of URE, we are
    // looking for the latter one here. I think...
    if (uri.endsWith( "/" LIBO_LIB_FOLDER ) )
    {
        uri = uri.copy( 0, uri.getLength() - (sizeof(LIBO_LIB_FOLDER)-1) ) + LIBO_URE_ETC_FOLDER;
    }
#endif
#endif
    return uri + "/" SAL_CONFIGFILE("uno");
}

bool cppu::nextDirectoryItem(osl::Directory & directory, OUString * url) {
    assert(url != nullptr);
    for (;;) {
        osl::DirectoryItem i;
        switch (directory.getNextItem(i, SAL_MAX_UINT32)) {
        case osl::FileBase::E_None:
            break;
        case osl::FileBase::E_NOENT:
            return false;
        default:
            throw css::uno::DeploymentException(
                "Cannot iterate directory");
        }
        osl::FileStatus stat(
            osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName |
            osl_FileStatus_Mask_FileURL);
        if (i.getFileStatus(stat) != osl::FileBase::E_None) {
            throw css::uno::DeploymentException(
                "Cannot stat in directory");
        }
        if (stat.getFileType() != osl::FileStatus::Directory) { //TODO: symlinks
            // Ignore backup files:
            OUString name(stat.getFileName());
            if (!(name.match(".") || name.endsWith("~"))) {
                *url = stat.getFileURL();
                return true;
            }
        }
    }
}

void cppu::decodeRdbUri(OUString * uri, bool * optional, bool * directory)
{
    assert(uri != nullptr && optional != nullptr && directory != nullptr);
    if(!(uri->isEmpty()))
    {
        *optional = (*uri)[0] == '?';
        if (*optional) {
            *uri = uri->copy(1);
        }
        *directory = uri->startsWith("<") && uri->endsWith(">*");
        if (*directory) {
            *uri = uri->copy(1, uri->getLength() - 3);
        }
    }
    else
    {
        *optional = false;
        *directory = false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
