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

#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <xmloff/i18nmap.hxx>

bool SvI18NMap::Add( sal_uInt16 nKind, const OUString& rName,
                     const OUString& rNewName )
{
    SvI18NMapEntry_Key aKey(nKind, rName);
    bool bIsNewInsertion = m_aMap.insert(SvI18NMap_Impl::value_type(aKey, rNewName)).second;
    SAL_INFO_IF(!bIsNewInsertion, "xmloff.core", "SvI18NMap::Add: item with key \"" << rName << "\" registered already, likely invalid input file");
    return bIsNewInsertion;
}

const OUString& SvI18NMap::Get( sal_uInt16 nKind, const OUString& rName ) const
{
    SvI18NMapEntry_Key aKey(nKind, rName);
    SvI18NMap_Impl::const_iterator aI = m_aMap.find(aKey);
    if (aI != m_aMap.end())
        return aI->second;
    return rName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
