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

#pragma once

#include <rtl/ustring.hxx>
#include "dbadllapi.hxx"
#include <unotools/resmgr.hxx>

#define DBA_RES( id )                       ::dbaccess::ResourceManager::loadString( id )
#define DBA_RES_PARAM( id, ascii, replace ) ::dbaccess::ResourceManager::loadString( id, ascii, replace )

namespace dbaccess
{
    // ResourceManager
    // handling resources within the DBA-Core library
    class UNLESS_MERGELIBS_MORE(OOO_DLLPUBLIC_DBA) ResourceManager
    {
    private:
        // no instantiation allowed
        ResourceManager() = delete;
        ~ResourceManager() { }

    public:
        /** loads the string with the specified resource id
        */
        static OUString  loadString(TranslateId pResId);

        /** loads a string from the resource file, substituting two placeholders with given strings

            @param  pResId
                the resource ID of the string to load
            @param  _pPlaceholderAscii1
                the ASCII representation of the first placeholder string
            @param  _rReplace1
                the string which should substitute the first placeholder
            @param  _pPlaceholderAscii2
                the ASCII representation of the second placeholder string
            @param  _rReplace2
                the string which should substitute the second placeholder
        */
        static OUString  loadString(
                TranslateId pResId,
                std::u16string_view _rPlaceholderAscii1,
                std::u16string_view _rReplace1,
                std::u16string_view _rPlaceholderAscii2,
                std::u16string_view _rReplace2
        );
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
