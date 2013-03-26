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

#ifndef INCLUDED_CODEMAKER_TYPEMANAGER_HXX
#define INCLUDED_CODEMAKER_TYPEMANAGER_HXX

#include "codemaker/global.hxx"
#include "registry/registry.hxx"
#include "registry/types.h"
#include "salhelper/simplereferenceobject.hxx"

#include <boost/unordered_map.hpp>
#include <list>

namespace typereg { class Reader; }

//typedef ::std::list< Registry* >  RegistryList;
typedef ::std::vector< Registry* >  RegistryList;
typedef ::std::pair< RegistryKey, sal_Bool >    KeyPair;
typedef ::std::vector< KeyPair >    RegistryKeyList;

typedef ::boost::unordered_map
<
    ::rtl::OString, // Typename
    RTTypeClass,    // TypeClass
    HashString,
    EqualString
> T2TypeClassMap;

class TypeManager : public salhelper::SimpleReferenceObject
{
public:
    TypeManager();

    sal_Bool init(const StringVector& regFiles, const StringVector& extraFiles = StringVector() );

    ::rtl::OString getTypeName(RegistryKey& rTypeKey) const;

    sal_Bool    isValidType(const ::rtl::OString& name) const
        { return searchTypeKey(name, 0).isValid(); }
    RegistryKey getTypeKey(
        const ::rtl::OString& name, sal_Bool * pIsExtraType = 0 ) const
        { return searchTypeKey(name, pIsExtraType); }
    RegistryKeyList getTypeKeys(const ::rtl::OString& name) const;
    typereg::Reader getTypeReader(
        const ::rtl::OString& name, sal_Bool * pIsExtraType = 0 ) const;
    typereg::Reader getTypeReader(RegistryKey& rTypeKey) const;
    RTTypeClass getTypeClass(const ::rtl::OString& name) const;
    RTTypeClass getTypeClass(RegistryKey& rTypeKey) const;

    void setBase(const ::rtl::OString& base);
    ::rtl::OString getBase() const { return m_base; }

    sal_Int32 getSize() const { return m_t2TypeClass.size(); }

    static sal_Bool isBaseType(const ::rtl::OString& name);

private:
    virtual ~TypeManager();

    RegistryKey searchTypeKey(
        const ::rtl::OString& name, sal_Bool * pIsExtraType = 0 ) const;
    void        freeRegistries();

    mutable T2TypeClassMap m_t2TypeClass;
    RegistryList    m_registries;
    RegistryList    m_extra_registries;
    ::rtl::OString  m_base;
};

#endif // INCLUDED_CODEMAKER_TYPEMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
