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

#include <codemaker/global.hxx>

#include <sal/types.h>
#include <o3tl/string_view.hxx>


namespace codemaker::cppumaker {

bool dumpNamespaceOpen(
    FileStream & out, std::u16string_view entityName, bool fullModuleType)
{
    bool bOutput = false;
    bool bFirst = true;
    for (sal_Int32 i = 0; i >= 0;) {
        std::u16string_view id(o3tl::getToken(entityName, 0, '.', i));
        if (fullModuleType || i >= 0) {
            if (!bFirst) {
                out << " ";
            }
            out << "namespace " << id << " {";
            bFirst = false;
            bOutput = true;
        }
    }
    return bOutput;
}

bool dumpNamespaceClose(
    FileStream & out, std::u16string_view entityName, bool fullModuleType)
{
    bool bOutput = false;
    bool bFirst = true;
    for (size_t i = 0; i != std::u16string_view::npos;) {
        i = entityName.find('.', i);
        if (i != std::u16string_view::npos) {
            ++i;
        }
        if (fullModuleType || i != std::u16string_view::npos) {
            if (!bFirst) {
                out << " ";
            }
            out << "}";
            bFirst = false;
            bOutput = true;
        }
    }
    return bOutput;
}

void dumpTypeIdentifier(FileStream & out, std::u16string_view entityName) {
    std::u16string_view::size_type pos = entityName.rfind('.');
    out << entityName.substr((pos != std::u16string_view::npos) ? pos + 1 : 0);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
