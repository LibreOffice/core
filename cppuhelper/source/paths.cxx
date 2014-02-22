/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <config_features.h>
#include <config_folders.h>

#include "sal/config.h"

#include <cassert>

#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/file.hxx"
#include "osl/module.hxx"
#include "osl/mutex.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "paths.hxx"

namespace {

rtl::OUString get_this_libpath() {
    static rtl::OUString s_uri;
    if (s_uri.isEmpty()) {
        rtl::OUString uri;
        osl::Module::getUrlFromAddress(
            reinterpret_cast< oslGenericFunction >(get_this_libpath), uri);
        sal_Int32 i = uri.lastIndexOf('/');
        if (i == -1) {
            throw css::uno::DeploymentException(
                "URI " + uri + " is expected to contain a slash",
                css::uno::Reference< css::uno::XInterface >());
        }
        uri = uri.copy(0, i);
        osl::MutexGuard guard(osl::Mutex::getGlobalMutex());
        if (s_uri.isEmpty()) {
            s_uri = uri;
        }
    }
    return s_uri;
}

}

rtl::OUString cppu::getUnoIniUri() {
#if defined ANDROID
    
    
    
    
    
    
    
    rtl::OUString uri("file:
#else
    rtl::OUString uri(get_this_libpath());
#if HAVE_FEATURE_MACOSX_MACLIKE_APP_STRUCTURE
    
    
    
    
    
    
    
    if (uri.endsWith( "/" LIBO_LIB_FOLDER ) )
    {
        uri = uri.copy( 0, uri.getLength() - (sizeof(LIBO_LIB_FOLDER)-1) ) + LIBO_URE_ETC_FOLDER;
    }
#endif
#endif
    return uri + "/" SAL_CONFIGFILE("uno");
}

bool cppu::nextDirectoryItem(osl::Directory & directory, rtl::OUString * url) {
    assert(url != 0);
    for (;;) {
        osl::DirectoryItem i;
        switch (directory.getNextItem(i, SAL_MAX_UINT32)) {
        case osl::FileBase::E_None:
            break;
        case osl::FileBase::E_NOENT:
            return false;
        default:
            throw css::uno::DeploymentException(
                "Cannot iterate directory",
                css::uno::Reference< css::uno::XInterface >());
        }
        osl::FileStatus stat(
            osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileName |
            osl_FileStatus_Mask_FileURL);
        if (i.getFileStatus(stat) != osl::FileBase::E_None) {
            throw css::uno::DeploymentException(
                "Cannot stat in directory",
                css::uno::Reference< css::uno::XInterface >());
        }
        if (stat.getFileType() != osl::FileStatus::Directory) { 
            
            rtl::OUString name(stat.getFileName());
            if (!(name.match(".") || name.endsWith("~"))) {
                *url = stat.getFileURL();
                return true;
            }
        }
    }
}

void cppu::decodeRdbUri(rtl::OUString * uri, bool * optional, bool * directory)
{
    assert(uri != 0 && optional != 0 && directory != 0);
    *optional = (*uri)[0] == '?';
    if (*optional) {
        *uri = uri->copy(1);
    }
    *directory = uri->startsWith("<") && uri->endsWith(">*");
    if (*directory) {
        *uri = uri->copy(1, uri->getLength() - 3);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
