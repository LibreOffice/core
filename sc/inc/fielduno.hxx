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

#include "address.hxx"

#include <svl/lstner.hxx>
#include <editeng/editdata.hxx>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <comphelper/interfacecontainer4.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/compbase.hxx>

#include <memory>

class ScEditSource;
class SvxFieldItem;
class SvxFieldData;
class ScDocShell;
class ScHeaderFooterTextData;
class SfxItemPropertySet;

class ScCellFieldsObj final : public cppu::WeakImplHelper<
                            css::container::XEnumerationAccess,
                            css::container::XIndexAccess,
                            css::container::XContainer,
                            css::util::XRefreshable,
                            css::lang::XServiceInfo >,
                        public SfxListener
{
private:
    css::uno::Reference<css::text::XTextRange> mxContent;
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    std::unique_ptr<ScEditSource> mpEditSource;
    /// List of refresh listeners.
    comphelper::OInterfaceContainerHelper4<css::util::XRefreshListener> maRefreshListeners;
    /// mutex to lock the InterfaceContainerHelper
    std::mutex              aMutex;

    css::uno::Reference<css::text::XTextField>
            GetObjectByIndex_Impl(sal_Int32 Index) const;

public:
    ScCellFieldsObj(
        css::uno::Reference<css::text::XTextRange> xContent,
        ScDocShell* pDocSh, const ScAddress& rPos);
    virtual ~ScCellFieldsObj() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XContainer
    virtual void SAL_CALL   addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
    virtual void SAL_CALL   removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;

                            // XRefreshable
    virtual void SAL_CALL refresh(  ) override;
    virtual void SAL_CALL addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l ) override;
    virtual void SAL_CALL removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

class ScHeaderFieldsObj final : public cppu::WeakImplHelper<
                            css::container::XEnumerationAccess,
                            css::container::XIndexAccess,
                            css::container::XContainer,
                            css::util::XRefreshable,
                            css::lang::XServiceInfo >
{
private:
    ScHeaderFooterTextData& mrData;
    std::unique_ptr<ScEditSource> mpEditSource;

    /// List of refresh listeners.
    comphelper::OInterfaceContainerHelper4<css::util::XRefreshListener> maRefreshListeners;
    /// mutex to lock the InterfaceContainerHelper
    std::mutex                  aMutex;

    css::uno::Reference< css::text::XTextField>
            GetObjectByIndex_Impl(sal_Int32 Index) const;

public:
    ScHeaderFieldsObj(ScHeaderFooterTextData& rData);
    virtual                 ~ScHeaderFieldsObj() override;

                            // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

                            // XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL
                            createEnumeration() override;

                            // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

                            // XContainer
    virtual void SAL_CALL   addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
    virtual void SAL_CALL   removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;

                            // XRefreshable
    virtual void SAL_CALL refresh(  ) override;
    virtual void SAL_CALL addRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l ) override;
    virtual void SAL_CALL removeRefreshListener( const css::uno::Reference< css::util::XRefreshListener >& l ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

/**
 * Generic UNO wrapper for edit engine's field item in cells, headers, and
 * footers.
 */
typedef comphelper::WeakComponentImplHelper<
                            css::text::XTextField,
                            css::beans::XPropertySet,
                            css::lang::XServiceInfo> ScEditFieldObj_Base;
class ScEditFieldObj final : public ScEditFieldObj_Base
{
    ScEditFieldObj() = delete;
    ScEditFieldObj(const ScEditFieldObj&) = delete;
    const ScEditFieldObj& operator=(const ScEditFieldObj&) = delete;

    const SfxItemPropertySet* pPropSet;
    std::unique_ptr<ScEditSource> mpEditSource;
    ESelection aSelection;

    sal_Int32 meType;
    std::unique_ptr<SvxFieldData> mpData;
    css::uno::Reference<css::text::XTextRange> mpContent;

    css::util::DateTime maDateTime;
    sal_Int32 mnNumFormat;
    bool mbIsDate:1;
    bool mbIsFixed:1;

private:
    SvxFieldData& getData();

    void setPropertyValueURL(const OUString& rName, const css::uno::Any& rVal);
    css::uno::Any getPropertyValueURL(const OUString& rName);

    void setPropertyValueFile(const OUString& rName, const css::uno::Any& rVal);
    css::uno::Any getPropertyValueFile(const OUString& rName);

    void setPropertyValueDateTime(const OUString& rName, const css::uno::Any& rVal);
    css::uno::Any getPropertyValueDateTime(const OUString& rName);

    void setPropertyValueSheet(const OUString& rName, const css::uno::Any& rVal);

public:
    ScEditFieldObj(
        css::uno::Reference<css::text::XTextRange> xContent,
        std::unique_ptr<ScEditSource> pEditSrc, sal_Int32 eType, const ESelection& rSel);
    virtual ~ScEditFieldObj() override;

    sal_Int32 GetFieldType() const { return meType;}
    void DeleteField();
    bool IsInserted() const;
    SvxFieldItem CreateFieldItem();
    void InitDoc(
        const css::uno::Reference<css::text::XTextRange>& rContent,
        std::unique_ptr<ScEditSource> pEditSrc, const ESelection& rSel);

                            // XTextField
    virtual OUString SAL_CALL getPresentation( sal_Bool bShowCommand ) override;

                            // XTextContent
    virtual void SAL_CALL attach( const css::uno::Reference< css::text::XTextRange >& xTextRange ) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL
                            getAnchor() override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

                            // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
