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
