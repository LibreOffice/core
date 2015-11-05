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
        <   css::text::XTextMarkup
        ,   css::text::XMultiTextMarkup
        >
{
public:
    SwXTextMarkup(SwTextNode *const rTextNode,
            const ModelToViewHelper& rConversionMap);
    virtual ~SwXTextMarkup();

    // css::text::XTextMarkup:
    virtual css::uno::Reference< css::container::XStringKeyMap > SAL_CALL getMarkupInfoContainer() throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL commitStringMarkup(::sal_Int32 nType, const OUString & aIdentifier, ::sal_Int32 nStart, ::sal_Int32 nLength,
                                           const css::uno::Reference< css::container::XStringKeyMap > & xMarkupInfoContainer) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL commitTextRangeMarkup(::sal_Int32 nType, const OUString & aIdentifier, const css::uno::Reference< css::text::XTextRange> & xRange,
                                                const css::uno::Reference< css::container::XStringKeyMap > & xMarkupInfoContainer) throw (css::uno::RuntimeException, std::exception) override;

    // css::text::XMultiTextMarkup:
    virtual void SAL_CALL commitMultiTextMarkup( const css::uno::Sequence< css::text::TextMarkupDescriptor >& aMarkups ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

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

/** Implementation of the css::container::XStringKeyMap interface
 */
class SwXStringKeyMap:
    public ::cppu::WeakImplHelper<
        css::container::XStringKeyMap>
{
public:
    SwXStringKeyMap();

    // css::container::XStringKeyMap:
    virtual css::uno::Any SAL_CALL getValue(const OUString & aKey) throw (css::uno::RuntimeException, css::container::NoSuchElementException, std::exception) override;
    virtual sal_Bool SAL_CALL hasValue(const OUString & aKey) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertValue(const OUString & aKey, const css::uno::Any & aValue) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::container::ElementExistException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getKeyByIndex(::sal_Int32 nIndex) throw (css::uno::RuntimeException, css::lang::IndexOutOfBoundsException, std::exception) override;
    virtual css::uno::Any SAL_CALL getValueByIndex(::sal_Int32 nIndex) throw (css::uno::RuntimeException, css::lang::IndexOutOfBoundsException, std::exception) override;

private:
    SwXStringKeyMap(SwXStringKeyMap &) = delete;
    void operator =(SwXStringKeyMap &) = delete;

    virtual ~SwXStringKeyMap() {}

    std::map< OUString, css::uno::Any > maMap;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
