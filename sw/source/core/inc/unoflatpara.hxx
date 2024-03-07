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

#pragma once

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XFlatParagraph.hpp>
#include <com/sun/star/text/XFlatParagraphIterator.hpp>
#include <svl/listener.hxx>
#include "unotextmarkup.hxx"
#include <nodeoffset.hxx>

#include <set>

namespace com::sun::star::container { class XStringKeyMap; }
namespace com::sun::star::text { class XTextRange; }
class SwTextNode;
class SwDoc;
class ModelToViewHelper;

typedef ::cppu::ImplInheritanceHelper
<   SwXTextMarkup
,   css::beans::XPropertySet
,   css::text::XFlatParagraph
> SwXFlatParagraph_Base;

class SwXFlatParagraph final
    :   public SwXFlatParagraph_Base
{
public:
    SwXFlatParagraph( SwTextNode& rTextNode, OUString aExpandText, const ModelToViewHelper& rConversionMap );
    virtual ~SwXFlatParagraph() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const css::uno::Any& rValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener) override;
    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener) override;
    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener) override;

    // text::XTextMarkup:
    virtual css::uno::Reference< css::container::XStringKeyMap > SAL_CALL getMarkupInfoContainer() override;

    virtual void SAL_CALL commitStringMarkup(::sal_Int32 nType, const OUString & aIdentifier, ::sal_Int32 nStart, ::sal_Int32 nLength,
                                   const css::uno::Reference< css::container::XStringKeyMap > & xMarkupInfoContainer) override;

    virtual void SAL_CALL commitTextRangeMarkup(::sal_Int32 nType, const OUString & aIdentifier, const css::uno::Reference< css::text::XTextRange> & xRange,
                                                const css::uno::Reference< css::container::XStringKeyMap > & xMarkupInfoContainer) override;

    // text::XFlatParagraph:
    virtual OUString SAL_CALL getText() override;
    virtual sal_Bool SAL_CALL isModified() override;
    virtual void SAL_CALL setChecked(::sal_Int32 nType, sal_Bool bVal) override;
    virtual sal_Bool SAL_CALL isChecked(::sal_Int32 nType) override;
    virtual css::lang::Locale SAL_CALL getLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen) override;
    virtual css::lang::Locale SAL_CALL getPrimaryLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen) override;
    virtual void SAL_CALL changeText(::sal_Int32 nPos, ::sal_Int32 nLen, const OUString & aNewText, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes) override;
    virtual void SAL_CALL changeAttributes(::sal_Int32 nPos, ::sal_Int32 nLen, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes) override;
    virtual css::uno::Sequence< ::sal_Int32 > SAL_CALL getLanguagePortions() override;

    using SwXTextMarkup::GetTextNode;

private:
    SwXFlatParagraph( const SwXFlatParagraph & ) = delete;
    SwXFlatParagraph & operator = ( const SwXFlatParagraph & ) = delete;

    OUString maExpandText;
    OUString maOrigText;
};

class SwXFlatParagraphIterator final :
    public ::cppu::WeakImplHelper
    <
        css::text::XFlatParagraphIterator
    >,
    public SvtListener
{
public:
    SwXFlatParagraphIterator( SwDoc& rDoc, sal_Int32 nType, bool bAutomatic );
    virtual ~SwXFlatParagraphIterator() override;

    // text::XFlatParagraphIterator:
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getFirstPara() override;
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getNextPara() override;
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getLastPara() override;
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getParaBefore(const css::uno::Reference< css::text::XFlatParagraph > & xPara) override;
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getParaAfter(const css::uno::Reference< css::text::XFlatParagraph > & xPara) override;

    virtual void Notify( const SfxHint& ) override;

private:
    SwXFlatParagraphIterator( const SwXFlatParagraphIterator & ) = delete;
    SwXFlatParagraphIterator & operator =(const SwXFlatParagraphIterator & ) = delete;

    SwDoc* mpDoc;
    const sal_Int32 mnType;
    const bool mbAutomatic;

    SwNodeOffset mnCurrentNode;    // used for non-automatic mode
    SwNodeOffset mnEndNode;        // used for non-automatic mode
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
