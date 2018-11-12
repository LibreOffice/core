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
#ifndef INCLUDED_JVMFWK_SOURCE_FWKUTIL_HXX
#define INCLUDED_JVMFWK_SOURCE_FWKUTIL_HXX

#include <config_features.h>
#include <config_folders.h>

#include <sal/config.h>
#include <sal/log.hxx>
#include <osl/mutex.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/instance.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/byteseq.hxx>
#include <osl/thread.hxx>

namespace jfw
{

/** Returns the file URL of the directory where the framework library
    (this library) resides.
*/
OUString getLibraryLocation();

/** provides a bootstrap class which already knows the values from the
    jvmfkwrc file.
*/
struct Bootstrap :
    public ::rtl::StaticWithInit< const rtl::Bootstrap *, Bootstrap > {
        const rtl::Bootstrap * operator () () {
            OUStringBuffer buf(256);
            buf.append(getLibraryLocation());
#ifdef MACOSX
            // For some reason the jvmfwk3rc file is traditionally in
            // LIBO_URE_ETC_FOLDER
            buf.append( "/../" LIBO_URE_ETC_FOLDER );
#endif
            buf.append(SAL_CONFIGFILE("/jvmfwk3"));
            OUString sIni = buf.makeStringAndClear();
            ::rtl::Bootstrap *  bootstrap = new ::rtl::Bootstrap(sIni);
            SAL_INFO("jfw.level2", "Using configuration file " << sIni);
            return bootstrap;
        }
};

struct FwkMutex: public ::rtl::Static<osl::Mutex, FwkMutex> {};

rtl::ByteSequence encodeBase16(const rtl::ByteSequence& rawData);
rtl::ByteSequence decodeBase16(const rtl::ByteSequence& data);

OUString getDirFromFile(const OUString& usFilePath);

enum FileStatus
{
    FILE_OK,
    FILE_DOES_NOT_EXIST,
    FILE_INVALID
};

/** checks if the URL is a file.

    If it is a link to a file than
    it is resolved. Assuming that the argument
    represents a relative URL then FILE_INVALID
    is returned.


    @return
    one of the values of FileStatus.

    @exception
    Errors occurred during determining if the file exists
 */
FileStatus checkFileURL(const OUString & path);

bool isAccessibilitySupportDesired();

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
