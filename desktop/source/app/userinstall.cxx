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

#include "sal/config.h"

#include <cassert>

#include "boost/shared_ptr.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "comphelper/configuration.hxx"
#include "config_folders.h"
#include "officecfg/Setup.hxx"
#include "osl/file.h"
#include "osl/file.hxx"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "unotools/bootstrap.hxx"

#include "userinstall.hxx"

namespace desktop { namespace userinstall {

namespace {

#if !(defined ANDROID || defined IOS)
osl::FileBase::RC copyRecursive(
    OUString const & srcUri, OUString const & dstUri)
{
    osl::DirectoryItem item;
    osl::FileBase::RC e = osl::DirectoryItem::get(srcUri, item);
    if (e != osl::FileBase::E_None) {
        return e;
    }
    osl::FileStatus stat1(osl_FileStatus_Mask_Type);
    e = item.getFileStatus(stat1);
    if (e != osl::FileBase::E_None) {
        return e;
    }
    if (stat1.getFileType() == osl::FileStatus::Directory) {
        e = osl::Directory::create(dstUri);
        if (e != osl::FileBase::E_None && e != osl::FileBase::E_EXIST) {
            return e;
        }
        osl::Directory dir(srcUri);
        e = dir.open();
        if (e != osl::FileBase::E_None) {
            return e;
        }
        for (;;) {
            e = dir.getNextItem(item);
            if (e == osl::FileBase::E_NOENT) {
                break;
            }
            if (e != osl::FileBase::E_None) {
                return e;
            }
            osl::FileStatus stat2(
                osl_FileStatus_Mask_FileName | osl_FileStatus_Mask_FileURL);
            e = item.getFileStatus(stat2);
            if (e != osl::FileBase::E_None) {
                return e;
            }
            assert(!dstUri.endsWith("/"));
            e = copyRecursive(
                stat2.getFileURL(), dstUri + "/" + stat2.getFileName());
                
                
            if (e != osl::FileBase::E_None) {
                return e;
            }
        }
        e = dir.close();
    } else {
        e = osl::File::copy(srcUri, dstUri);
        if (e == osl::FileBase::E_EXIST) {
            
            e = osl::FileBase::E_None;
        }
    }
    return e;
}
#endif

Status create(OUString const & uri) {
    osl::FileBase::RC e = osl::Directory::createPath(uri);
    if (e != osl::FileBase::E_None && e != osl::FileBase::E_EXIST) {
        return ERROR_OTHER;
    }
#if !(defined ANDROID || defined IOS)
#if defined UNIX
    
    osl::File::setAttributes(
        uri,
        (osl_File_Attribute_OwnWrite | osl_File_Attribute_OwnRead
         | osl_File_Attribute_OwnExe));
#endif
    
    OUString baseUri;
    if (utl::Bootstrap::locateBaseInstallation(baseUri)
        != utl::Bootstrap::PATH_EXISTS)
    {
        return ERROR_OTHER;
    }
    switch (copyRecursive(
                baseUri + "/" LIBO_SHARE_PRESETS_FOLDER, uri + "/user"))
    {
    case osl::FileBase::E_None:
        break;
    case osl::FileBase::E_ACCES:
        return ERROR_CANT_WRITE;
    case osl::FileBase::E_NOSPC:
        return ERROR_NO_SPACE;
    default:
        return ERROR_OTHER;
    }
#endif
    boost::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    officecfg::Setup::Office::ooSetupInstCompleted::set(true, batch);
    batch->commit();
    return CREATED;
}

bool isCreated() {
    try {
        return officecfg::Setup::Office::ooSetupInstCompleted::get();
    } catch (css::uno::Exception & e) {
        SAL_WARN("desktop.app", "ignoring Exception \"" << e.Message << "\"");
        return false;
    }
}

}

Status finalize() {
    OUString uri;
    switch (utl::Bootstrap::locateUserInstallation(uri)) {
    case utl::Bootstrap::PATH_EXISTS:
        if (isCreated()) {
            return EXISTED;
        }
        
    case utl::Bootstrap::PATH_VALID:
        return create(uri);
    default:
        return ERROR_OTHER;
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
