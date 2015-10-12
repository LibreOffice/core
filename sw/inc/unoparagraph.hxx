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

#ifndef INCLUDED_SW_INC_UNOPARAGRAPH_HXX
#define INCLUDED_SW_INC_UNOPARAGRAPH_HXX

#include <memory>

#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XTolerantMultiPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextRange.hpp>

#include <cppuhelper/implbase.hxx>

#include <sfx2/Metadatable.hxx>

#include <unobaseclass.hxx>

struct SwPosition;
class SwPaM;
class SwUnoCrsr;
class SwStartNode;
class SwTextNode;
class SwTable;
class SwXText;

typedef ::cppu::ImplInheritanceHelper
<   ::sfx2::MetadatableMixin
,   ::com::sun::star::lang::XUnoTunnel
,   ::com::sun::star::lang::XServiceInfo
,   ::com::sun::star::beans::XPropertySet
,   ::com::sun::star::beans::XPropertyState
,   ::com::sun::star::beans::XMultiPropertySet
,   ::com::sun::star::beans::XTolerantMultiPropertySet
,   ::com::sun::star::container::XEnumerationAccess
,   ::com::sun::star::container::XContentEnumerationAccess
,   ::com::sun::star::text::XTextContent
,   ::com::sun::star::text::XTextRange
> SwXParagraph_Base;

class SwXParagraph
    : public SwXParagraph_Base
{

private:

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    virtual ~SwXParagraph();

    SwXParagraph(::com::sun::star::uno::Reference<
                    ::com::sun::star::text::XText > const & xParent,
            SwTextNode & rTextNode,
            const sal_Int32 nSelStart = -1, const sal_Int32 nSelEnd = - 1);

    /// descriptor
    SwXParagraph();

public:

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XTextContent>
        CreateXParagraph(SwDoc & rDoc, SwTextNode * pTextNode,
            ::com::sun::star::uno::Reference< ::com::sun::star::text::XText>
                const& xParentText = 0,
            const sal_Int32 nSelStart = -1, const sal_Int32 nSelEnd = - 1);

    const SwTextNode * GetTextNode() const;
    bool            IsDescriptor() const;
    /// make rPaM select the paragraph
    bool SelectPaM(SwPaM & rPaM);
    /// for SwXText
    void attachToText(SwXText & rParent, SwTextNode & rTextNode);

    // MetadatableMixin
    virtual ::sfx2::Metadatable* GetCoreObject() override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        GetModel() override;

    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const ::com::sun::star::uno::Sequence< sal_Int8 >& rIdentifier)
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
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
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

    // XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL
        getPropertyState(const OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyState > SAL_CALL
        getPropertyStates(
            const ::com::sun::star::uno::Sequence<
                OUString >& rPropertyNames)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault(
            const OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault(
            const OUString& rPropertyName)
        throw (::com::sun::star::beans::UnknownPropertyException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues(
            const ::com::sun::star::uno::Sequence< OUString >&
                rPropertyNames,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Any >& rValues)
        throw (::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
        SAL_CALL getPropertyValues(
            const ::com::sun::star::uno::Sequence< OUString >&
                rPropertyNames)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertiesChangeListener(
            const ::com::sun::star::uno::Sequence< OUString >&
                rPropertyNames,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertiesChangeListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertiesChangeListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertiesChangeListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL firePropertiesChangeEvent(
            const ::com::sun::star::uno::Sequence< OUString >&
                rPropertyNames,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertiesChangeListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTolerantMultiPropertySet
    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::SetPropertyTolerantFailed > SAL_CALL
        setPropertyValuesTolerant(
            const ::com::sun::star::uno::Sequence< OUString >&
                rPropertyNames,
            const ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Any >& rValues)
        throw (::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::GetPropertyTolerantResult > SAL_CALL
        getPropertyValuesTolerant(
            const ::com::sun::star::uno::Sequence< OUString >&
                rPropertyNames)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::GetDirectPropertyTolerantResult > SAL_CALL
        getDirectPropertyValuesTolerant(
            const ::com::sun::star::uno::Sequence< OUString >&
                rPropertyNames)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

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

    // XContentEnumerationAccess
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XEnumeration > SAL_CALL
        createContentEnumeration(const OUString& rServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getAvailableServiceNames()
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
        SAL_CALL getText()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
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

};


struct SwXParagraphEnumeration
    : public SwSimpleEnumeration_Base
{
    static SwXParagraphEnumeration* Create(
        ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > const & xParent,
        std::shared_ptr<SwUnoCrsr> pCursor,
        const CursorType eType,
        SwStartNode const*const pStartNode = 0,
        SwTable const*const pTable = 0);
};

#endif // INCLUDED_SW_INC_UNOPARAGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
