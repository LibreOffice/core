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

#ifndef INCLUDED_XMLOFF_I18NMAP_HXX
#define INCLUDED_XMLOFF_I18NMAP_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <rtl/ustring.hxx>
#include <map>

class SvI18NMapEntry_Key
{
    sal_uInt16      nKind;
    OUString   aName;
public:
    SvI18NMapEntry_Key( sal_uInt16 nKnd, const OUString& rName ) :
        nKind( nKnd ),
        aName( rName )
    {
    }

    bool operator==( const SvI18NMapEntry_Key& r ) const
    {
        return nKind == r.nKind &&
               aName == r.aName;
    }

    bool operator<( const SvI18NMapEntry_Key& r ) const
    {
        return nKind < r.nKind ||
               ( nKind == r.nKind &&
                 aName < r.aName);
    }
};

typedef std::map<SvI18NMapEntry_Key, OUString> SvI18NMap_Impl;

class XMLOFF_DLLPUBLIC SvI18NMap
{
    SvI18NMap_Impl m_aMap;

public:
    // Add a name mapping
    bool Add( sal_uInt16 nKind, const OUString& rName,
              const OUString& rNewName );

    // Return a mapped name. If the name could not be found, return the
    // original name.
    const OUString& Get( sal_uInt16 nKind,
                                const OUString& rName ) const;
};

#endif // INCLUDED_XMLOFF_I18NMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
