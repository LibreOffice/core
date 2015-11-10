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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNOMETA_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNOMETA_HXX

#include <deque>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextField.hpp>

#include <cppuhelper/implbase.hxx>

#include <sfx2/Metadatable.hxx>
#include <boost/noncopyable.hpp>

#include <unobaseclass.hxx>

typedef ::std::deque<
    css::uno::Reference< css::text::XTextRange > >
    TextRangeList_t;

class SwPaM;
class SwTextNode;

namespace sw {
    class Meta;
}

typedef ::cppu::ImplInheritanceHelper
<   ::sfx2::MetadatableMixin
,   css::lang::XUnoTunnel
,   css::lang::XServiceInfo
,   css::container::XChild
,   css::container::XEnumerationAccess
,   css::text::XTextContent
,   css::text::XText
> SwXMeta_Base;

class SwXMeta
    : public SwXMeta_Base
    , private ::boost::noncopyable
{

public:

    class Impl;

protected:

    ::sw::UnoImplPtr<Impl> m_pImpl;

    void SAL_CALL AttachImpl(
            const css::uno::Reference< css::text::XTextRange > & xTextRange,
            const sal_uInt16 nWhich)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException);

    virtual ~SwXMeta();

    /// @param pDoc and pMeta != 0, but not & because of ImplInheritanceHelper
    SwXMeta(SwDoc *const pDoc, ::sw::Meta *const pMeta,
        css::uno::Reference< css::text::XText> const&  xParentText,
        TextRangeList_t const*const pPortions);

    SwXMeta(SwDoc *const pDoc);

public:

    static css::uno::Reference< css::rdf::XMetadatable >
        CreateXMeta(
            ::sw::Meta & rMeta,
            css::uno::Reference< css::text::XText> const& xParentText = nullptr,
            ::std::unique_ptr<TextRangeList_t const> && pPortions = ::std::unique_ptr<TextRangeList_t const>());

    static css::uno::Reference<css::rdf::XMetadatable>
        CreateXMeta(SwDoc & rDoc, bool isField);

    /// init params with position of the attribute content (w/out CH_TXTATR)
    bool SetContentRange( SwTextNode *& rpNode, sal_Int32 & rStart, sal_Int32 & rEnd) const;
    css::uno::Reference< css::text::XText > GetParentText() const;

    bool CheckForOwnMemberMeta(const SwPaM & rPam, const bool bAbsorb)
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException);

    // MetadatableMixin
    virtual ::sfx2::Metadatable * GetCoreObject() override;
    virtual css::uno::Reference< css::frame::XModel >
        GetModel() override;

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const css::uno::Sequence< sal_Int8 >& Identifier )
        throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener(
            const css::uno::Reference< css::lang::XEventListener > & xListener)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener(
            const css::uno::Reference< css::lang::XEventListener > & xListener)
        throw (css::uno::RuntimeException, std::exception) override;

    // XChild
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL
        getParent()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setParent(
            css::uno::Reference< css::uno::XInterface> const& xParent)
        throw (css::uno::RuntimeException,
            css::lang::NoSupportException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements()
        throw (css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL
        createEnumeration()
        throw (css::uno::RuntimeException, std::exception) override;

    // XTextContent
    virtual void SAL_CALL attach(
            const css::uno::Reference< css::text::XTextRange > & xTextRange)
        throw (css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getAnchor()
        throw (css::uno::RuntimeException, std::exception) override;

    // XTextRange
    virtual css::uno::Reference< css::text::XText >
        SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
                css::text::XTextRange > SAL_CALL getStart()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference<
                css::text::XTextRange > SAL_CALL getEnd()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setString(const OUString& rString)
        throw (css::uno::RuntimeException, std::exception) override;

    // XSimpleText
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL
        createTextCursor()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextCursor >  SAL_CALL
        createTextCursorByRange(
            const css::uno::Reference< css::text::XTextRange > & xTextPosition)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertString(
            const css::uno::Reference< css::text::XTextRange > & xRange,
            const OUString& aString, sal_Bool bAbsorb)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertControlCharacter(
            const css::uno::Reference< css::text::XTextRange > & xRange,
            sal_Int16 nControlCharacter, sal_Bool bAbsorb)
        throw (css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception) override;

    // XText
    virtual void SAL_CALL insertTextContent(
            const css::uno::Reference< css::text::XTextRange > & xRange,
            const css::uno::Reference< css::text::XTextContent > & xContent,
            sal_Bool bAbsorb)
        throw (css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTextContent(
            const css::uno::Reference< css::text::XTextContent > & xContent)
        throw (css::container::NoSuchElementException,
            css::uno::RuntimeException, std::exception) override;

};

typedef ::cppu::ImplInheritanceHelper
<   SwXMeta
,   css::beans::XPropertySet
,   css::text::XTextField
> SwXMetaField_Base;

class SwXMetaField
    : public SwXMetaField_Base
{

private:

    virtual ~SwXMetaField();

    friend css::uno::Reference< css::rdf::XMetadatable >
        SwXMeta::CreateXMeta(::sw::Meta &,
            css::uno::Reference< css::text::XText> const&,
            ::std::unique_ptr<TextRangeList_t const> && pPortions);

    SwXMetaField(SwDoc *const pDoc, ::sw::Meta *const pMeta,
        css::uno::Reference< css::text::XText> const& xParentText,
        TextRangeList_t const*const pPortions);

    friend css::uno::Reference<css::rdf::XMetadatable>
        SwXMeta::CreateXMeta(SwDoc &, bool);

    SwXMetaField(SwDoc *const pDoc);

public:

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames( )
        throw (css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener(
            const css::uno::Reference< css::lang::XEventListener > & xListener)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener(
            const css::uno::Reference< css::lang::XEventListener > & xListener)
        throw (css::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
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
    virtual css::uno::Any SAL_CALL
        getPropertyValue(const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
        throw (css::beans::UnknownPropertyException,
            css::lang::WrappedTargetException,
            css::uno::RuntimeException, std::exception) override;

    // XTextContent
    virtual void SAL_CALL attach(
            const css::uno::Reference< css::text::XTextRange > & xTextRange)
        throw ( css::lang::IllegalArgumentException,
            css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getAnchor()
        throw (css::uno::RuntimeException, std::exception) override;

    // XTextField
    virtual OUString SAL_CALL getPresentation(sal_Bool bShowCommand)
        throw (css::uno::RuntimeException, std::exception) override;

};

/// get prefix/suffix from the RDF repository. @throws RuntimeException
void getPrefixAndSuffix(
        const css::uno::Reference< css::frame::XModel>& xModel,
        const css::uno::Reference< css::rdf::XMetadatable>& xMetaField,
        OUString *const o_pPrefix, OUString *const o_pSuffix);

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNOMETA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
