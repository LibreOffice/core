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

#ifndef INCLUDED_COMPHELPER_ATTRIBUTELIST_HXX
#define INCLUDED_COMPHELPER_ATTRIBUTELIST_HXX

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/comphelperdllapi.h>


namespace comphelper
{

struct AttributeList_Impl;

class COMPHELPER_DLLPUBLIC AttributeList : public ::cppu::WeakImplHelper
<
    css::xml::sax::XAttributeList
>
{
    AttributeList_Impl *m_pImpl;
public:
    AttributeList();
    virtual ~AttributeList();

    // methods that are not contained in any interface
    void AddAttribute( const OUString &sName , const OUString &sType , const OUString &sValue );

    // css::xml::sax::XAttributeList
    virtual sal_Int16 SAL_CALL getLength()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getNameByIndex(sal_Int16 i)
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getTypeByIndex(sal_Int16 i)
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getTypeByName(const OUString& aName)
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getValueByIndex(sal_Int16 i)
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL getValueByName(const OUString& aName)
        throw( css::uno::RuntimeException, std::exception ) override;

};

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_ATTRIBUTELIST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
