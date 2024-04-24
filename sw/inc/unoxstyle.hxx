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

#include <rtl/ref.hxx>
#include <svl/listener.hxx>
#include <svl/style.hxx>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <cppuhelper/implbase.hxx>
#include "coreframestyle.hxx"

class StyleFamilyEntry;
class SwStyleBase_Impl;
class SwStyleProperties_Impl;
class SvxSetItem;
struct SfxItemPropertyMapEntry;

class SAL_DLLPUBLIC_RTTI SwXStyle
    : public cppu::WeakImplHelper<css::style::XStyle, css::beans::XPropertySet,
                                  css::beans::XMultiPropertySet, css::lang::XServiceInfo,
                                  css::lang::XUnoTunnel, css::beans::XPropertyState,
                                  css::beans::XMultiPropertyStates>,
      public SfxListener,
      public SvtListener
{
    SwDoc* m_pDoc;
    OUString m_sStyleName;
    const StyleFamilyEntry& m_rEntry;
    bool m_bIsDescriptor;
    bool m_bIsConditional;
    OUString m_sParentStyleName;

protected:
    SfxStyleSheetBasePool* m_pBasePool;
    std::unique_ptr<SwStyleProperties_Impl> m_pPropertiesImpl;
    css::uno::Reference<css::container::XNameAccess> m_xStyleFamily;
    css::uno::Reference<css::beans::XPropertySet> m_xStyleData;

    template <sal_uInt16>
    void SetPropertyValue(const SfxItemPropertyMapEntry&, const SfxItemPropertySet&,
                          const css::uno::Any&, SwStyleBase_Impl&);
    void SetPropertyValues_Impl(const css::uno::Sequence<OUString>& aPropertyNames,
                                const css::uno::Sequence<css::uno::Any>& aValues);
    SfxStyleSheetBase* GetStyleSheetBase();
    void PrepareStyleBase(SwStyleBase_Impl& rBase);
    template <sal_uInt16>
    css::uno::Any GetStyleProperty(const SfxItemPropertyMapEntry& rEntry,
                                   const SfxItemPropertySet& rPropSet, SwStyleBase_Impl& rBase);
    css::uno::Any GetStyleProperty_Impl(const SfxItemPropertyMapEntry& rEntry,
                                        const SfxItemPropertySet& rPropSet,
                                        SwStyleBase_Impl& rBase);
    css::uno::Any GetPropertyValue_Impl(const SfxItemPropertySet* pPropSet, SwStyleBase_Impl& rBase,
                                        const OUString& rPropertyName);

public:
    SwXStyle(SwDoc* pDoc, SfxStyleFamily eFam, bool bConditional = false);
    SwXStyle(SfxStyleSheetBasePool* pPool, SfxStyleFamily eFamily, SwDoc* pDoc,
             const OUString& rStyleName);
    virtual ~SwXStyle() override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL
    getSomething(const css::uno::Sequence<sal_Int8>& aIdentifier) override;

    //XNamed
    virtual OUString SAL_CALL getName() override;
    virtual void SAL_CALL setName(const OUString& Name_) override;

    //XStyle
    virtual sal_Bool SAL_CALL isUserDefined() override;
    virtual sal_Bool SAL_CALL isInUse() override;
    virtual OUString SAL_CALL getParentStyle() override;
    virtual void SAL_CALL setParentStyle(const OUString& aParentStyle) override;

    //XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo>
        SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName,
                                           const css::uno::Any& aValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(
        const OUString&, const css::uno::Reference<css::beans::XPropertyChangeListener>&) override
    {
        OSL_FAIL("not implemented");
    };
    virtual void SAL_CALL removePropertyChangeListener(
        const OUString&, const css::uno::Reference<css::beans::XPropertyChangeListener>&) override
    {
        OSL_FAIL("not implemented");
    };
    virtual void SAL_CALL addVetoableChangeListener(
        const OUString&, const css::uno::Reference<css::beans::XVetoableChangeListener>&) override
    {
        OSL_FAIL("not implemented");
    };
    virtual void SAL_CALL removeVetoableChangeListener(
        const OUString&, const css::uno::Reference<css::beans::XVetoableChangeListener>&) override
    {
        OSL_FAIL("not implemented");
    };

    //XMultiPropertySet
    virtual void SAL_CALL
    setPropertyValues(const css::uno::Sequence<OUString>& aPropertyNames,
                      const css::uno::Sequence<css::uno::Any>& aValues) override;
    virtual css::uno::Sequence<css::uno::Any>
        SAL_CALL getPropertyValues(const css::uno::Sequence<OUString>& aPropertyNames) override;
    virtual void SAL_CALL addPropertiesChangeListener(
        const css::uno::Sequence<OUString>&,
        const css::uno::Reference<css::beans::XPropertiesChangeListener>&) override{};
    virtual void SAL_CALL removePropertiesChangeListener(
        const css::uno::Reference<css::beans::XPropertiesChangeListener>&) override{};
    virtual void SAL_CALL firePropertiesChangeEvent(
        const css::uno::Sequence<OUString>&,
        const css::uno::Reference<css::beans::XPropertiesChangeListener>&) override{};

    //XPropertyState
    virtual css::beans::PropertyState SAL_CALL
    getPropertyState(const OUString& PropertyName) override;
    virtual css::uno::Sequence<css::beans::PropertyState>
        SAL_CALL getPropertyStates(const css::uno::Sequence<OUString>& aPropertyName) override;
    virtual void SAL_CALL setPropertyToDefault(const OUString& PropertyName) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault(const OUString& aPropertyName) override;

    //XMultiPropertyStates
    virtual void SAL_CALL setAllPropertiesToDefault() override;
    virtual void SAL_CALL
    setPropertiesToDefault(const css::uno::Sequence<OUString>& aPropertyNames) override;
    virtual css::uno::Sequence<css::uno::Any>
        SAL_CALL getPropertyDefaults(const css::uno::Sequence<OUString>& aPropertyNames) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

    //SfxListener
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;
    //SvtListener
    virtual void Notify(const SfxHint&) override;
    const OUString& GetStyleName() const { return m_sStyleName; }
    SfxStyleFamily GetFamily() const;

    bool IsDescriptor() const { return m_bIsDescriptor; }
    bool IsConditional() const { return m_bIsConditional; }
    const OUString& GetParentStyleName() const { return m_sParentStyleName; }
    void SetDoc(SwDoc* pDc, SfxStyleSheetBasePool* pPool)
    {
        m_bIsDescriptor = false;
        m_pDoc = pDc;
        m_pBasePool = pPool;
        SfxListener::StartListening(*m_pBasePool);
    }
    SwDoc* GetDoc() const { return m_pDoc; }
    void Invalidate();
    void ApplyDescriptorProperties();
    void SetStyleName(const OUString& rSet) { m_sStyleName = rSet; }
    /// @throws beans::PropertyVetoException
    /// @throws lang::IllegalArgumentException
    /// @throws lang::WrappedTargetException
    /// @throws uno::RuntimeException
    void SetStyleProperty(const SfxItemPropertyMapEntry& rEntry, const SfxItemPropertySet& rPropSet,
                          const css::uno::Any& rValue, SwStyleBase_Impl& rBase);
    void PutItemToSet(const SvxSetItem* pSetItem, const SfxItemPropertySet& rPropSet,
                      const SfxItemPropertyMapEntry& rEntry, const css::uno::Any& rVal,
                      SwStyleBase_Impl& rBaseImpl);
};

typedef cppu::ImplInheritanceHelper<SwXStyle, css::document::XEventsSupplier> SwXFrameStyle_Base;
class SwXFrameStyle : public SwXFrameStyle_Base, public sw::ICoreFrameStyle
{
public:
    SwXFrameStyle(SfxStyleSheetBasePool& rPool, SwDoc* pDoc, const OUString& rStyleName)
        : SwXFrameStyle_Base(&rPool, SfxStyleFamily::Frame, pDoc, rStyleName)
    {
    }
    explicit SwXFrameStyle(SwDoc* pDoc);

    virtual css::uno::Reference<css::container::XNameReplace> SAL_CALL getEvents() override;

    //ICoreStyle
    virtual void SetItem(sal_uInt16 eAtr, const SfxPoolItem& rItem) override;
    virtual const SfxPoolItem* GetItem(sal_uInt16 eAtr) override;
    virtual css::document::XEventsSupplier& GetEventsSupplier() override { return *this; };
};

class SAL_DLLPUBLIC_RTTI SwXPageStyle : public SwXStyle
{
protected:
    void SetPropertyValues_Impl(const css::uno::Sequence<OUString>& aPropertyNames,
                                const css::uno::Sequence<css::uno::Any>& aValues);
    css::uno::Sequence<css::uno::Any>
    GetPropertyValues_Impl(const css::uno::Sequence<OUString>& aPropertyNames);

public:
    SwXPageStyle(SfxStyleSheetBasePool& rPool, SwDocShell* pDocSh, const OUString& rStyleName);
    explicit SwXPageStyle(SwDocShell* pDocSh);

    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName,
                                           const css::uno::Any& aValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;

    virtual void SAL_CALL
    setPropertyValues(const css::uno::Sequence<OUString>& aPropertyNames,
                      const css::uno::Sequence<css::uno::Any>& aValues) override;
    virtual css::uno::Sequence<css::uno::Any>
        SAL_CALL getPropertyValues(const css::uno::Sequence<OUString>& aPropertyNames) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
