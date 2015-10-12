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

#ifndef INCLUDED_SAX_SOURCE_EXPATWRAP_ATTRLISTIMPL_HXX
#define INCLUDED_SAX_SOURCE_EXPATWRAP_ATTRLISTIMPL_HXX

#include "sal/config.h"
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>

namespace sax_expatwrap
{

struct AttributeList_impl;

//FIXME
class AttributeList :
    public ::cppu::WeakImplHelper<
                ::com::sun::star::xml::sax::XAttributeList,
                ::com::sun::star::util::XCloneable >
{
public:
    AttributeList();
    AttributeList( const AttributeList & );
    virtual ~AttributeList();

    void addAttribute( const OUString &sName ,
        const OUString &sType , const OUString &sValue );
    void clear();
public:
    // XAttributeList
    virtual sal_Int16 SAL_CALL getLength()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getNameByIndex(sal_Int16 i)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTypeByIndex(sal_Int16 i)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTypeByName(const OUString& aName)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getValueByIndex(sal_Int16 i)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getValueByName(const OUString& aName)
        throw( ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XCloneable
    virtual ::com::sun::star::uno::Reference< XCloneable > SAL_CALL
        createClone()   throw(::com::sun::star::uno::RuntimeException, std::exception) override;

private:
    struct AttributeList_impl *m_pImpl;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
