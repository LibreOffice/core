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
        const rtl::OUString & rModuleName,
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
    rtl::OUString m_aModuleName;
    com::sun::star::uno::Sequence< com::sun::star::uno::TypeClass > m_aTypes;
    com::sun::star::reflection::TypeDescriptionSearchDepth m_eDepth;
    TDEnumerationAccessStack m_aChildren;
    com::sun::star::uno::Reference<
        com::sun::star::reflection::XTypeDescriptionEnumeration > m_xEnum;
};

} // namespace stoc_tdmgr

#endif /* _STOC_TDMGR_TDENUMERATION_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
