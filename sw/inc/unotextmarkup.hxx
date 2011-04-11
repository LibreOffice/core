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

#ifndef _UNOTEXTMARKUP_HXX
#define _UNOTEXTMARKUP_HXX

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/text/XTextMarkup.hpp>
#include <com/sun/star/text/XMultiTextMarkup.hpp>
#include <calbck.hxx>
#include <modeltoviewhelper.hxx>

#include <map>

namespace com { namespace sun { namespace star { namespace container {
    class XStringKeyMap;
} } } }

class SwTxtNode;
class SfxPoolItem;

/** Implementation of the css::text::XTextMarkup interface
 */
class SwXTextMarkup:
    public ::cppu::WeakImplHelper2
    <
        ::com::sun::star::text::XTextMarkup,
        ::com::sun::star::text::XMultiTextMarkup
    >,
    public SwClient
{
public:
    SwXTextMarkup( SwTxtNode& rTxtNode, const ModelToViewHelper::ConversionMap* pConversionMap );
    virtual ~SwXTextMarkup();

    // ::com::sun::star::text::XTextMarkup:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XStringKeyMap > SAL_CALL getMarkupInfoContainer() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL commitTextMarkup(::sal_Int32 nType, const ::rtl::OUString & aIdentifier, ::sal_Int32 nStart, ::sal_Int32 nLength, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XStringKeyMap > & xMarkupInfoContainer) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::text::XMultiTextMarkup:
    virtual void SAL_CALL commitMultiTextMarkup( const ::com::sun::star::uno::Sequence< ::com::sun::star::text::TextMarkupDescriptor >& aMarkups ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

private:
    SwXTextMarkup( const SwXTextMarkup & ); // not defined
    SwXTextMarkup & operator =( const SwXTextMarkup & ); // not defined

protected:
    //SwClient
   virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew);

    SwTxtNode* mpTxtNode;
    const ModelToViewHelper::ConversionMap* mpConversionMap;

};


/** Implementation of the ::com::sun::star::container::XStringKeyMap interface
 */
class SwXStringKeyMap:
    public ::cppu::WeakImplHelper1<
        ::com::sun::star::container::XStringKeyMap>
{
public:
    SwXStringKeyMap();

    // ::com::sun::star::container::XStringKeyMap:
    virtual ::com::sun::star::uno::Any SAL_CALL getValue(const ::rtl::OUString & aKey) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::container::NoSuchElementException);
    virtual ::sal_Bool SAL_CALL hasValue(const ::rtl::OUString & aKey) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertValue(const ::rtl::OUString & aKey, const ::com::sun::star::uno::Any & aValue) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException);
    virtual ::sal_Int32 SAL_CALL getCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getKeyByIndex(::sal_Int32 nIndex) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException);
    virtual ::com::sun::star::uno::Any SAL_CALL getValueByIndex(::sal_Int32 nIndex) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException);

private:
    SwXStringKeyMap(SwXStringKeyMap &); // not defined
    void operator =(SwXStringKeyMap &); // not defined

    virtual ~SwXStringKeyMap() {}

    std::map< rtl::OUString, ::com::sun::star::uno::Any > maMap;
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
