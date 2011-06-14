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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_codemaker.hxx"

#include "dumputils.hxx"

#include "codemaker/global.hxx"
#include "codemaker/commoncpp.hxx"

#include "rtl/ustring.hxx"
#include "sal/types.h"


namespace codemaker { namespace cppumaker {

bool dumpNamespaceOpen(
    FileStream & out, rtl::OString const & registryType, bool fullModuleType)
{
    bool output = false;
    if (registryType != "/") {
        bool first = true;
        for (sal_Int32 i = 0; i >= 0;) {
            rtl::OString id(registryType.getToken(0, '/', i));
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
    FileStream & out, rtl::OString const & registryType, bool fullModuleType)
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

void dumpTypeIdentifier(FileStream & out, rtl::OString const & registryType) {
    out << registryType.copy(registryType.lastIndexOf('/') + 1);
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
