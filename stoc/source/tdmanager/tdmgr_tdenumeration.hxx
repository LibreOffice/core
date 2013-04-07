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

#ifndef _STOC_TDMGR_TDENUMERATION_HXX
#define _STOC_TDMGR_TDENUMERATION_HXX

#include <stack>
#include <osl/mutex.hxx>
#include <com/sun/star/reflection/XTypeDescriptionEnumeration.hpp>
#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#include <cppuhelper/implbase1.hxx>

namespace stoc_tdmgr
{

typedef std::stack< com::sun::star::uno::Reference<
            com::sun::star::reflection::XTypeDescriptionEnumerationAccess > >
                TDEnumerationAccessStack;

class TypeDescriptionEnumerationImpl
    : public cppu::WeakImplHelper1<
                com::sun::star::reflection::XTypeDescriptionEnumeration >
{
public:
    TypeDescriptionEnumerationImpl(
        const OUString & rModuleName,
        const com::sun::star::uno::Sequence<
            com::sun::star::uno::TypeClass > & rTypes,
        com::sun::star::reflection::TypeDescriptionSearchDepth eDepth,
        const TDEnumerationAccessStack & rTDEAS );
    virtual ~TypeDescriptionEnumerationImpl();

    // XEnumeration (base of XTypeDescriptionEnumeration)
    virtual sal_Bool SAL_CALL hasMoreElements()
        throw ( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement()
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    // XTypeDescriptionEnumeration
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::reflection::XTypeDescription > SAL_CALL
    nextTypeDescription()
        throw ( ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::uno::RuntimeException );

private:
    com::sun::star::uno::Reference<
        com::sun::star::reflection::XTypeDescriptionEnumeration >
    queryCurrentChildEnumeration();

    osl::Mutex m_aMutex;
    OUString m_aModuleName;
    com::sun::star::uno::Sequence< com::sun::star::uno::TypeClass > m_aTypes;
    com::sun::star::reflection::TypeDescriptionSearchDepth m_eDepth;
    TDEnumerationAccessStack m_aChildren;
    com::sun::star::uno::Reference<
        com::sun::star::reflection::XTypeDescriptionEnumeration > m_xEnum;
};

} // namespace stoc_tdmgr

#endif /* _STOC_TDMGR_TDENUMERATION_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
