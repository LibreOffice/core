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

#include <tools/solar.h>

namespace rtl
{
    class OUString;
}
namespace binfilter {

class SvI18NMap_Impl;
class SvI18NMapEntry_Impl;

class SvI18NMap
{
    SvI18NMap_Impl		*pImpl;

    SvI18NMapEntry_Impl *_Find( USHORT nKind,
                                const ::rtl::OUString& rName ) const;

public:

    SvI18NMap();
    ~SvI18NMap();

    // Add a name mapping
    void Add( USHORT nKind, const ::rtl::OUString& rName,
              const ::rtl::OUString& rNewName );

    // Return a mapped name. If the name could not be found, return the
    // original name.
    const ::rtl::OUString& Get( USHORT nKind,
                                const ::rtl::OUString& rName ) const;
};

}//end of namespace binfilter
#endif	//  _XMLOFF_I18NMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
