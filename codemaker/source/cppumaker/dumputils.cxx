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


#include "dumputils.hxx"

#include "codemaker/global.hxx"
#include "codemaker/commoncpp.hxx"

#include "rtl/ustring.hxx"
#include "sal/types.h"


namespace codemaker { namespace cppumaker {

bool dumpNamespaceOpen(
    FileStream & out, OString const & registryType, bool fullModuleType)
{
    bool output = false;
    if (registryType != "/") {
        bool first = true;
        for (sal_Int32 i = 0; i >= 0;) {
            OString id(registryType.getToken(0, '/', i));
            if (fullModuleType || i >= 0) {
                if (!first) {
                    out << " ";
                }
                out << "namespace " << id << " {";
                first = false;
                output = true;
            }
        }
    }
    return output;
}

bool dumpNamespaceClose(
    FileStream & out, OString const & registryType, bool fullModuleType)
{
    bool output = false;
    if (registryType != "/") {
        bool first = true;
        for (sal_Int32 i = 0; i >= 0;) {
            i = registryType.indexOf('/', i);
            if (i >= 0) {
                ++i;
            }
            if (fullModuleType || i >= 0) {
                if (!first) {
                    out << " ";
                }
                out << "}";
                first = false;
                output = true;
            }
        }
    }
    return output;
}

void dumpTypeIdentifier(FileStream & out, OString const & registryType) {
    out << registryType.copy(registryType.lastIndexOf('/') + 1);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
