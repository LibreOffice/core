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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNOFLATPARA_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNOFLATPARA_HXX

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/text/XFlatParagraph.hpp>
#include <com/sun/star/text/XFlatParagraphIterator.hpp>
#include <calbck.hxx>
#include <modeltoviewhelper.hxx>
#include <unotextmarkup.hxx>

#include <set>

namespace com { namespace sun { namespace star { namespace container {
    class XStringKeyMap;
} } } }

namespace com { namespace sun { namespace star { namespace text {
    class XTextRange;
} } } }

class SwTextNode;
class SwDoc;

typedef ::cppu::ImplInheritanceHelper
<   SwXTextMarkup
,   css::beans::XPropertySet
,   css::text::XFlatParagraph
,   css::lang::XUnoTunnel
> SwXFlatParagraph_Base;

class SwXFlatParagraph
    :   public SwXFlatParagraph_Base
{
public:
    SwXFlatParagraph( SwTextNode& rTextNode, const OUString& aExpandText, const ModelToViewHelper& rConversionMap );
    virtual ~SwXFlatParagraph();

    // XPropertySet
    virtual css::uno::Reference<
                css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const css::uno::Any& rValue)
        throw (css::beans::UnknownPropertyException,
                css::beans::PropertyVetoException,
                css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<
                css::beans::XPropertyChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<
                css::beans::XPropertyChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<
                css::beans::XVetoableChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<
                css::beans::XVetoableChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    // text::XTextMarkup:
    virtual css::uno::Reference< css::container::XStringKeyMap > SAL_CALL getMarkupInfoContainer() throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL commitStringMarkup(::sal_Int32 nType, const OUString & aIdentifier, ::sal_Int32 nStart, ::sal_Int32 nLength,
                                   const css::uno::Reference< css::container::XStringKeyMap > & xMarkupInfoContainer) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL commitTextRangeMarkup(::sal_Int32 nType, const OUString & aIdentifier, const css::uno::Reference< css::text::XTextRange> & xRange,
                                                const css::uno::Reference< css::container::XStringKeyMap > & xMarkupInfoContainer) throw (css::uno::RuntimeException, std::exception) override;

    // text::XFlatParagraph:
    virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isModified() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setChecked(::sal_Int32 nType, sal_Bool bVal) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isChecked(::sal_Int32 nType) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) override;
    virtual css::lang::Locale SAL_CALL getPrimaryLanguageOfText(::sal_Int32 nPos, ::sal_Int32 nLen) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) override;
    virtual void SAL_CALL changeText(::sal_Int32 nPos, ::sal_Int32 nLen, const OUString & aNewText, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) override;
    virtual void SAL_CALL changeAttributes(::sal_Int32 nPos, ::sal_Int32 nLen, const css::uno::Sequence< css::beans::PropertyValue > & aAttributes) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) override;
    virtual css::uno::Sequence< ::sal_Int32 > SAL_CALL getLanguagePortions() throw (css::uno::RuntimeException, std::exception) override;

    using SwXTextMarkup::GetTextNode;

    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(const css::uno::Sequence< sal_Int8 >& rId)
        throw (css::uno::RuntimeException, std::exception) override;

private:
    SwXFlatParagraph( const SwXFlatParagraph & ) = delete;
    SwXFlatParagraph & operator = ( const SwXFlatParagraph & ) = delete;

    OUString maExpandText;
};

class SwXFlatParagraphIterator:
    public ::cppu::WeakImplHelper
    <
        css::text::XFlatParagraphIterator
    >,
    public SwClient     // to get notified when doc is closed...
{
public:
    SwXFlatParagraphIterator( SwDoc& rDoc, sal_Int32 nType, bool bAutomatic );
    virtual ~SwXFlatParagraphIterator();

    // text::XFlatParagraphIterator:
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getFirstPara() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getNextPara() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getLastPara() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getParaBefore(const css::uno::Reference< css::text::XFlatParagraph > & xPara) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) override;
    virtual css::uno::Reference< css::text::XFlatParagraph > SAL_CALL getParaAfter(const css::uno::Reference< css::text::XFlatParagraph > & xPara) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception) override;

protected:
    // SwClient
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;

private:
    SwXFlatParagraphIterator( const SwXFlatParagraphIterator & ) = delete;
    SwXFlatParagraphIterator & operator =(const SwXFlatParagraphIterator & ) = delete;

    // container to hold the 'hard' references as long as necessary and valid
    std::set< css::uno::Reference< css::text::XFlatParagraph > >    m_aFlatParaList;

    SwDoc* mpDoc;
    const sal_Int32 mnType;
    const bool mbAutomatic;

    sal_uLong mnCurrentNode;    // used for non-automatic mode
    sal_uLong mnStartNode;      // used for non-automatic mode
    sal_uLong mnEndNode;        // used for non-automatic mode
    bool mbWrapped;     // used for non-automatic mode
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
