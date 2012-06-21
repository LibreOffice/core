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

#ifndef _SPECIALTYPEMANAGER_HXX_
#define _SPECIALTYPEMANAGER_HXX_

#include    <codemaker/registry.hxx>
#include    <codemaker/typemanager.hxx>


struct SpecialTypeManagerImpl
{
    T2TypeClassMap  m_t2TypeClass;
};

class SpecialTypeManager : public TypeManager
{
public:
    SpecialTypeManager();
    ~SpecialTypeManager();

    SpecialTypeManager( const SpecialTypeManager& value )
        : TypeManager(value)
        , m_pImpl( value.m_pImpl )
    {
        acquire();
    }

    using TypeManager::init;
    sal_Bool init(const ::rtl::OString& registryName);

    sal_Bool    isValidType(const ::rtl::OString&)
        { return sal_True; }
    TypeReader  getTypeReader(const ::rtl::OString& name);
    RTTypeClass getTypeClass(const ::rtl::OString& name);

    sal_Int32 getSize() { return m_pImpl->m_t2TypeClass.size(); }

protected:
    void acquire();
    void release();

protected:
    SpecialTypeManagerImpl* m_pImpl;
};

#endif // _CODEMAKER_TYPEMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
