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

#ifndef INCLUDED_UNODEVTOOLS_TYPEMANAGER_HXX
#define INCLUDED_UNODEVTOOLS_TYPEMANAGER_HXX

#include <codemaker/typemanager.hxx>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

#include <boost/unordered_map.hpp>
#include <vector>

class RegistryKey;

namespace typereg { class Reader; }

typedef boost::unordered_map
<
    ::rtl::OString, // Typename
    RTTypeClass,    // TypeClass
    HashString,
    EqualString
> T2TypeClassMap;

namespace unodevtools {

struct UnoTypeManagerImpl
{
    UnoTypeManagerImpl() {}

    T2TypeClassMap  m_t2TypeClass;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XHierarchicalNameAccess> m_tdmgr;
};

class UnoTypeManager : public TypeManager
{
public:
    UnoTypeManager();
    ~UnoTypeManager();

    UnoTypeManager( const UnoTypeManager& value )
        : TypeManager(value)
        , m_pImpl( value.m_pImpl )
    {}

    sal_Bool init(const ::std::vector< ::rtl::OUString > registries);

    sal_Bool    isValidType(const ::rtl::OString& name) const;
    ::rtl::OString getTypeName(RegistryKey& rTypeKey) const;
    typereg::Reader getTypeReader(
        const ::rtl::OString& name, sal_Bool * pIsExtraType = 0 ) const;
    typereg::Reader getTypeReader(RegistryKey& rTypeKey) const;
    RTTypeClass getTypeClass(const ::rtl::OString& name) const;
    RTTypeClass getTypeClass(RegistryKey& rTypeKey) const;

protected:
    void release();

    UnoTypeManagerImpl* m_pImpl;
};

}

#endif // _UNODEVTOOLS_TYPEMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
