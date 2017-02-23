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
#ifndef INCLUDED_SW_INC_UNOSRCH_HXX
#define INCLUDED_SW_INC_UNOSRCH_HXX

#include <com/sun/star/util/XPropertyReplace.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>

class SfxItemPropertySet;
class SwXTextDocument;
class SwSearchProperties_Impl;
class SfxItemSet;

namespace i18nutil {
    struct SearchOptions2;
}

class SwXTextSearch : public cppu::WeakImplHelper
<
    css::util::XPropertyReplace,
    css::lang::XServiceInfo,
    css::lang::XUnoTunnel
>
{
    friend class SwXTextDocument;

    OUString                m_sSearchText;
    OUString                m_sReplaceText;

    SwSearchProperties_Impl*    m_pSearchProperties;
    SwSearchProperties_Impl*    m_pReplaceProperties;

    const SfxItemPropertySet*   m_pPropSet;
    bool                    m_bAll  : 1;
    bool                    m_bWord : 1;
    bool                    m_bBack : 1;
    bool                    m_bExpr : 1;
    bool                    m_bCase : 1;
    bool                    m_bStyles:1;
    bool                    m_bSimilarity : 1;
    bool                    m_bLevRelax       :1;
    sal_Int16                   m_nLevExchange;
    sal_Int16                   m_nLevAdd;
    sal_Int16                   m_nLevRemove;

    bool                    m_bIsValueSearch :1;
protected:
    virtual ~SwXTextSearch() override;
public:
    SwXTextSearch();

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    //XSearchDescriptor
    virtual OUString SAL_CALL getSearchString(  ) override;
    virtual void SAL_CALL setSearchString( const OUString& aString ) override;

    //XReplaceDescriptor
    virtual OUString SAL_CALL getReplaceString() override;
    virtual void SAL_CALL setReplaceString(const OUString& aReplaceString) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    //XPropertyReplace
    virtual sal_Bool SAL_CALL getValueSearch() override;
    virtual void SAL_CALL setValueSearch(sal_Bool ValueSearch_) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getSearchAttributes() override;
    virtual void SAL_CALL setSearchAttributes(const css::uno::Sequence< css::beans::PropertyValue >& aSearchAttribs) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getReplaceAttributes() override;
    virtual void SAL_CALL setReplaceAttributes(const css::uno::Sequence< css::beans::PropertyValue >& aSearchAttribs) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    void    FillSearchItemSet(SfxItemSet& rSet) const;
    void    FillReplaceItemSet(SfxItemSet& rSet) const;

    bool    HasSearchAttributes() const;
    bool    HasReplaceAttributes() const;

    void    FillSearchOptions( i18nutil::SearchOptions2& rSearchOpt ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
