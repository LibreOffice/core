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

#ifndef _XMLOFF_I18NMAP_HXX
#define _XMLOFF_I18NMAP_HXX

#include <sal/config.h>
#include "xmloff/dllapi.h"
#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <map>

class SvI18NMapEntry_Key
{
    sal_uInt16      nKind;
    rtl::OUString   aName;
public:
    SvI18NMapEntry_Key( sal_uInt16 nKnd, const rtl::OUString& rName ) :
        nKind( nKnd ),
        aName( rName )
    {
    }

    sal_Bool operator==( const SvI18NMapEntry_Key& r ) const
    {
        return nKind == r.nKind &&
               aName == r.aName;
    }

    sal_Bool operator<( const SvI18NMapEntry_Key& r ) const
    {
        return nKind < r.nKind ||
               ( nKind == r.nKind &&
                 aName < r.aName);
    }
};

typedef std::map<SvI18NMapEntry_Key, rtl::OUString> SvI18NMap_Impl;

class XMLOFF_DLLPUBLIC SvI18NMap
{
    SvI18NMap_Impl m_aMap;

public:
    // Add a name mapping
    bool Add( sal_uInt16 nKind, const rtl::OUString& rName,
              const rtl::OUString& rNewName );

    // Return a mapped name. If the name could not be found, return the
    // original name.
    const rtl::OUString& Get( sal_uInt16 nKind,
                                const rtl::OUString& rName ) const;
};

#endif  //  _XMLOFF_I18NMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
