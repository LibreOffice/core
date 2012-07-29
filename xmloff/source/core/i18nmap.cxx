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

#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include "xmloff/i18nmap.hxx"

bool SvI18NMap::Add( sal_uInt16 nKind, const rtl::OUString& rName,
                     const rtl::OUString& rNewName )
{
    SvI18NMapEntry_Key aKey(nKind, rName);
    bool bIsNewInsertion = m_aMap.insert(SvI18NMap_Impl::value_type(aKey, rNewName)).second;
    DBG_ASSERT( bIsNewInsertion, "SvI18NMap::Add: item registered already" );
    return bIsNewInsertion;
}

const rtl::OUString& SvI18NMap::Get( sal_uInt16 nKind, const rtl::OUString& rName ) const
{
    SvI18NMapEntry_Key aKey(nKind, rName);
    SvI18NMap_Impl::const_iterator aI = m_aMap.find(aKey);
    if (aI != m_aMap.end())
        return aI->second;
    return rName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
