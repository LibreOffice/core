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
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > >
    TextRangeList_t;

class SwPaM;
class SwTextNode;

namespace sw {
    class Meta;
}

typedef ::cppu::ImplInheritanceHelper
<   ::sfx2::MetadatableMixin
,   ::com::sun::star::lang::XUnoTunnel
,   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::container::XChild
,   ::com::sun::star::container::XEnumerationAccess
,   ::com::sun::star::text::XTextContent
,   ::com::sun::star::text::XText
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
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange,
            const sal_uInt16 nWhich)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException);

    virtual ~SwXMeta();

    /// @param pDoc and pMeta != 0, but not & because of ImplInheritanceHelper
    SwXMeta(SwDoc *const pDoc, ::sw::Meta *const pMeta,
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XText> const&
            xParentText,
        TextRangeList_t const*const pPortions);

    SwXMeta(SwDoc *const pDoc);

public:

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::rdf::XMetadatable >
        CreateXMeta(
            ::sw::Meta & rMeta,
            ::com::sun::star::uno::Reference< ::com::sun::star::text::XText>
                const& xParentText = 0,
            ::std::unique_ptr<TextRangeList_t const> && pPortions =
                ::std::unique_ptr<TextRangeList_t const>());

    static css::uno::Reference<css::rdf::XMetadatable>
        CreateXMeta(SwDoc & rDoc, bool isField);

    /// init params with position of the attribute content (w/out CH_TXTATR)
    bool SetContentRange(
            SwTextNode *& rpNode, sal_Int32 & rStart, sal_Int32 & rEnd) const;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >
        GetParentText() const;

    bool CheckForOwnMemberMeta(const SwPaM & rPam, const bool bAbsorb)
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

    // MetadatableMixin
    virtual ::sfx2::Metadatable * GetCoreObject() override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        GetModel() override;

    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const ::com::sun::star::uno::Sequence< sal_Int8 >& Identifier )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XChild
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XInterface > SAL_CALL
        getParent()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setParent(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface> const& xParent)
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::lang::NoSupportException, std::exception) override;

    // XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XEnumeration >  SAL_CALL
        createEnumeration()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTextContent
    virtual void SAL_CALL attach(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange)
        throw (::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL getAnchor()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >
        SAL_CALL getText() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL getStart()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > SAL_CALL getEnd()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setString(const OUString& rString)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XSimpleText
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >  SAL_CALL
        createTextCursor()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextCursor >  SAL_CALL
        createTextCursorByRange(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextPosition)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertString(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xRange,
            const OUString& aString, sal_Bool bAbsorb)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertControlCharacter(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xRange,
            sal_Int16 nControlCharacter, sal_Bool bAbsorb)
        throw (::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XText
    virtual void SAL_CALL insertTextContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xRange,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent > & xContent,
            sal_Bool bAbsorb)
        throw (::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTextContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextContent > & xContent)
        throw (::com::sun::star::container::NoSuchElementException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;

};

typedef ::cppu::ImplInheritanceHelper
<   SwXMeta
,   ::com::sun::star::beans::XPropertySet
,   ::com::sun::star::text::XTextField
> SwXMetaField_Base;

class SwXMetaField
    : public SwXMetaField_Base
{

private:

    virtual ~SwXMetaField();

    friend ::com::sun::star::uno::Reference<
            ::com::sun::star::rdf::XMetadatable >
        SwXMeta::CreateXMeta(::sw::Meta &,
            ::com::sun::star::uno::Reference< ::com::sun::star::text::XText>
                const&,
            ::std::unique_ptr<TextRangeList_t const> && pPortions);

    SwXMetaField(SwDoc *const pDoc, ::sw::Meta *const pMeta,
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XText> const&
            xParentText,
        TextRangeList_t const*const pPortions);

    friend css::uno::Reference<css::rdf::XMetadatable>
        SwXMeta::CreateXMeta(SwDoc &, bool);

    SwXMetaField(SwDoc *const pDoc);

public:

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(
            const OUString& ServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames( )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::lang::XEventListener > & xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Any& rValue)
        throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::beans::PropertyVetoException,
            ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL
        getPropertyValue(const OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertyChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XVetoableChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XVetoableChangeListener >& xListener)
        throw (::com::sun::star::beans::UnknownPropertyException,
            ::com::sun::star::lang::WrappedTargetException,
            ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTextContent
    virtual void SAL_CALL attach(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextRange > & xTextRange)
        throw ( ::com::sun::star::lang::IllegalArgumentException,
            ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XTextRange > SAL_CALL getAnchor()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTextField
    virtual OUString SAL_CALL getPresentation(sal_Bool bShowCommand)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

};

/// get prefix/suffix from the RDF repository. @throws RuntimeException
void getPrefixAndSuffix(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel>& xModel,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::rdf::XMetadatable>& xMetaField,
        OUString *const o_pPrefix, OUString *const o_pSuffix);

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNOMETA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
