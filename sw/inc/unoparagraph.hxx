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
,   css::lang::XUnoTunnel
,   css::lang::XServiceInfo
,   css::beans::XPropertySet
,   css::beans::XPropertyState
,   css::beans::XMultiPropertySet
,   css::beans::XTolerantMultiPropertySet
,   css::container::XEnumerationAccess
,   css::container::XContentEnumerationAccess
,   css::text::XTextContent
,   css::text::XTextRange
> SwXParagraph_Base;

class SwXParagraph
    : public SwXParagraph_Base
{

private:

    class Impl;
    ::sw::UnoImplPtr<Impl> m_pImpl;

    virtual ~SwXParagraph();

    SwXParagraph(css::uno::Reference< css::text::XText > const & xParent,
            SwTextNode & rTextNode,
            const sal_Int32 nSelStart = -1, const sal_Int32 nSelEnd = - 1);

    /// descriptor
    SwXParagraph();

public:

    static css::uno::Reference<css::text::XTextContent>
        CreateXParagraph(SwDoc & rDoc, SwTextNode * pTextNode,
            css::uno::Reference< css::text::XText>
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
    virtual css::uno::Reference< css::frame::XModel >
        GetModel() override;

    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(
            const css::uno::Sequence< sal_Int8 >& rIdentifier)
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
    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName)
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

    // XPropertyState
    virtual css::beans::PropertyState SAL_CALL
        getPropertyState(const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL
        getPropertyStates(
            const css::uno::Sequence< OUString >& rPropertyNames)
        throw (css::beans::UnknownPropertyException,
                css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault(
            const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault(
            const OUString& rPropertyName)
        throw (css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues(
            const css::uno::Sequence< OUString >&      rPropertyNames,
            const css::uno::Sequence< css::uno::Any >& rValues)
        throw (css::beans::PropertyVetoException,
                css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any >
        SAL_CALL getPropertyValues(
            const css::uno::Sequence< OUString >&  rPropertyNames)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertiesChangeListener(
            const css::uno::Sequence< OUString >&         rPropertyNames,
            const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertiesChangeListener(
            const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL firePropertiesChangeEvent(
            const css::uno::Sequence< OUString >&        rPropertyNames,
            const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener)
        throw (css::uno::RuntimeException, std::exception) override;

    // XTolerantMultiPropertySet
    virtual css::uno::Sequence< css::beans::SetPropertyTolerantFailed > SAL_CALL
        setPropertyValuesTolerant(
            const css::uno::Sequence< OUString >&      rPropertyNames,
            const css::uno::Sequence< css::uno::Any >& rValues)
        throw (css::lang::IllegalArgumentException,
                css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::beans::GetPropertyTolerantResult > SAL_CALL
        getPropertyValuesTolerant(
            const css::uno::Sequence< OUString >& rPropertyNames)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence<
            css::beans::GetDirectPropertyTolerantResult > SAL_CALL
        getDirectPropertyValuesTolerant(
            const css::uno::Sequence< OUString >& rPropertyNames)
        throw (css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements()
        throw (css::uno::RuntimeException, std::exception) override;

    // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL
        createEnumeration()
        throw (css::uno::RuntimeException, std::exception) override;

    // XContentEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
        createContentEnumeration(const OUString& rServiceName)
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getAvailableServiceNames()
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
        SAL_CALL getText()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getStart()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getEnd()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setString(const OUString& rString)
        throw (css::uno::RuntimeException, std::exception) override;

};


struct SwXParagraphEnumeration
    : public SwSimpleEnumeration_Base
{
    static SwXParagraphEnumeration* Create(
        css::uno::Reference< css::text::XText > const & xParent,
        std::shared_ptr<SwUnoCrsr> pCursor,
        const CursorType eType,
        SwStartNode const*const pStartNode = 0,
        SwTable const*const pTable = 0);
};

#endif // INCLUDED_SW_INC_UNOPARAGRAPH_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
