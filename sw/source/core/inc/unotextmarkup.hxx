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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNOTEXTMARKUP_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNOTEXTMARKUP_HXX

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/text/XTextMarkup.hpp>
#include <com/sun/star/text/XMultiTextMarkup.hpp>

#include <unobaseclass.hxx>

#include <map>

namespace com { namespace sun { namespace star { namespace container {
    class XStringKeyMap;
} } } }

namespace com { namespace sun { namespace star { namespace text {
    class XTextRange;
} } } }

class SwTextNode;
class ModelToViewHelper;
class SfxPoolItem;

/** Implementation of the css::text::XTextMarkup interface
 */
class SwXTextMarkup
    : public ::cppu::WeakImplHelper
        <   ::com::sun::star::text::XTextMarkup
        ,   ::com::sun::star::text::XMultiTextMarkup
        >
{
public:
    SwXTextMarkup(SwTextNode *const rTextNode,
            const ModelToViewHelper& rConversionMap);
    virtual ~SwXTextMarkup();

    // ::com::sun::star::text::XTextMarkup:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XStringKeyMap > SAL_CALL getMarkupInfoContainer() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL commitStringMarkup(::sal_Int32 nType, const OUString & aIdentifier, ::sal_Int32 nStart, ::sal_Int32 nLength,
                                           const ::com::sun::star::uno::Reference< ::com::sun::star::container::XStringKeyMap > & xMarkupInfoContainer) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL commitTextRangeMarkup(::sal_Int32 nType, const OUString & aIdentifier, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange> & xRange,
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::container::XStringKeyMap > & xMarkupInfoContainer) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::text::XMultiTextMarkup:
    virtual void SAL_CALL commitMultiTextMarkup( const ::com::sun::star::uno::Sequence< ::com::sun::star::text::TextMarkupDescriptor >& aMarkups ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    SwXTextMarkup( const SwXTextMarkup & ) = delete;
    SwXTextMarkup & operator =( const SwXTextMarkup & ) = delete;

    struct Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

protected:
    SwTextNode* GetTextNode();
    void ClearTextNode();
    const ModelToViewHelper& GetConversionMap();
};

/** Implementation of the ::com::sun::star::container::XStringKeyMap interface
 */
class SwXStringKeyMap:
    public ::cppu::WeakImplHelper<
        ::com::sun::star::container::XStringKeyMap>
{
public:
    SwXStringKeyMap();

    // ::com::sun::star::container::XStringKeyMap:
    virtual ::com::sun::star::uno::Any SAL_CALL getValue(const OUString & aKey) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::container::NoSuchElementException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL hasValue(const OUString & aKey) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL insertValue(const OUString & aKey, const ::com::sun::star::uno::Any & aValue) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, std::exception) SAL_OVERRIDE;
    virtual ::sal_Int32 SAL_CALL getCount() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getKeyByIndex(::sal_Int32 nIndex) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Any SAL_CALL getValueByIndex(::sal_Int32 nIndex) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IndexOutOfBoundsException, std::exception) SAL_OVERRIDE;

private:
    SwXStringKeyMap(SwXStringKeyMap &) = delete;
    void operator =(SwXStringKeyMap &) = delete;

    virtual ~SwXStringKeyMap() {}

    std::map< OUString, ::com::sun::star::uno::Any > maMap;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
