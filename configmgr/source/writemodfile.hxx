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

#ifndef INCLUDED_CONFIGMGR_SOURCE_WRITEMODFILE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_WRITEMODFILE_HXX

#include <sal/config.h>

#include <string_view>

#include <rtl/strbuf.hxx>

namespace configmgr {

class Components;
struct Data;

struct TempFile {
    OUString url;
    oslFileHandle handle;
    bool closed;
    OStringBuffer buffer;

    TempFile(): handle(nullptr), closed(false) {}
    ~TempFile();
    void closeAndRename(const OUString &url);
    oslFileError flush();
#ifdef _WIN32
    oslFileError closeWithoutUnlink();
#endif
    void writeString(std::string_view text);

private:
    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;
};

void writeAttributeValue(TempFile &handle, std::u16string_view value);
void writeValueContent(TempFile &handle, std::u16string_view value);

void writeModFile(
    Components & components, OUString const & url, Data const & data);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
