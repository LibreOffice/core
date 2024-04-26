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
#include <o3tl/string_view.hxx>

#include "paths.hxx"

namespace {

#if !(defined ANDROID || defined EMSCRIPTEN)
OUString get_this_libpath() {
    static OUString s_uri = []() {
        OUString uri;
        sal_Int32 i = -1;
        if (osl::Module::getUrlFromAddress(reinterpret_cast<oslGenericFunction>(get_this_libpath), uri))
            i = uri.lastIndexOf('/');
        if (i == -1)
            throw css::uno::DeploymentException("URI " + uri + " is expected to contain a slash");
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
#elif defined(EMSCRIPTEN)
    OUString uri("file:///instdir/program");
#else
    OUString uri(get_this_libpath());
#ifdef MACOSX
    // We keep the URE dylibs directly in "Frameworks" (that is, LIBO_LIB_FOLDER) and unorc in
    // "Resources/ure/etc" (LIBO_URE_ETC_FOLDER).
    if (uri.endsWith( "/" LIBO_LIB_FOLDER ) )
    {
        uri = OUString::Concat(uri.subView( 0, uri.getLength() - (sizeof(LIBO_LIB_FOLDER)-1) )) + LIBO_URE_ETC_FOLDER;
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
                u"Cannot iterate directory"_ustr);
        }
        osl::FileStatus stat(
            osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName |
            osl_FileStatus_Mask_FileURL);
        if (i.getFileStatus(stat) != osl::FileBase::E_None) {
            throw css::uno::DeploymentException(
                u"Cannot stat in directory"_ustr);
        }
        if (stat.getFileType() != osl::FileStatus::Directory) { //TODO: symlinks
            // Ignore backup and spurious junk files:
            OUString name(stat.getFileName());
            if (name.startsWith(".") || !name.endsWithIgnoreAsciiCase(u".rdb")) {
                SAL_WARN("cppuhelper", "ignoring <" << stat.getFileURL() << ">");
            } else {
                *url = stat.getFileURL();
                return true;
            }
        }
    }
}

void cppu::decodeRdbUri(std::u16string_view * uri, bool * optional, bool * directory)
{
    assert(uri != nullptr && optional != nullptr && directory != nullptr);
    if(!(uri->empty()))
    {
        *optional = (*uri)[0] == '?';
        if (*optional) {
            *uri = uri->substr(1);
        }
        *directory = o3tl::starts_with(*uri, u"<") && o3tl::ends_with(*uri, u">*");
        if (*directory) {
            *uri = uri->substr(1, uri->size() - 3);
        }
    }
    else
    {
        *optional = false;
        *directory = false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
