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


#ifndef INCLUDED_HWPFILTER_SOURCE_ATTRIBUTES_HXX
#define INCLUDED_HWPFILTER_SOURCE_ATTRIBUTES_HXX

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <cppuhelper/implbase.hxx>
#include <memory>

using namespace ::cppu;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;

struct AttributeListImpl_impl;
class AttributeListImpl : public WeakImplHelper< XAttributeList >
{
protected:
    virtual ~AttributeListImpl() override;

public:
    AttributeListImpl();
    AttributeListImpl( const AttributeListImpl & );

public:
    virtual sal_Int16 SAL_CALL getLength() override;
    virtual OUString  SAL_CALL getNameByIndex(sal_Int16 i) override;
    virtual OUString  SAL_CALL getTypeByIndex(sal_Int16 i) override;
    virtual OUString  SAL_CALL getTypeByName(const OUString& aName) override;
    virtual OUString  SAL_CALL getValueByIndex(sal_Int16 i) override;
    virtual OUString  SAL_CALL getValueByName(const OUString& aName) override;

public:
    void addAttribute( const OUString &sName , const OUString &sType , const OUString &sValue );
    void clear();

private:
    std::unique_ptr<AttributeListImpl_impl> m_pImpl;
};

#endif // INCLUDED_HWPFILTER_SOURCE_ATTRIBUTES_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
