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

#include <core_resource.hxx>

#include <unotools/resmgr.hxx>

// ---- needed as long as we have no contexts for components ---
#include <rtl/instance.hxx>

namespace dbaccess
{
    // ResourceManager
    namespace
    {
        // access safety
        struct theResourceManagerMutex : public rtl::Static< osl::Mutex, theResourceManagerMutex > {};
    }

    OUString ResourceManager::loadString(TranslateId pResId)
    {
        return Translate::get(pResId, Translate::Create("dba"));
    }

    OUString ResourceManager::loadString(TranslateId pResId, const char* _pPlaceholderAscii, std::u16string_view _rReplace)
    {
        OUString sString(loadString(pResId));
        return sString.replaceFirst( OUString::createFromAscii(_pPlaceholderAscii), _rReplace );
    }

    OUString ResourceManager::loadString(TranslateId pResId, const char* _pPlaceholderAscii1, std::u16string_view _rReplace1,
        const char* _pPlaceholderAscii2, std::u16string_view _rReplace2)
    {
        OUString sString(loadString(pResId));
        sString = sString.replaceFirst( OUString::createFromAscii(_pPlaceholderAscii1), _rReplace1 );
        sString = sString.replaceFirst( OUString::createFromAscii(_pPlaceholderAscii2), _rReplace2 );
        return sString;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
