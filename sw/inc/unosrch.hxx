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

namespace com{namespace sun{namespace star{namespace util{
    struct SearchOptions;
}}}}

class SwXTextSearch : public cppu::WeakImplHelper
<
    css::util::XPropertyReplace,
    css::lang::XServiceInfo,
    css::lang::XUnoTunnel
>
{
    friend class SwXTextDocument;

    OUString                sSearchText;
    OUString                sReplaceText;

    SwSearchProperties_Impl*    pSearchProperties;
    SwSearchProperties_Impl*    pReplaceProperties;

    const SfxItemPropertySet*   m_pPropSet;
    bool                    bAll  : 1;
    bool                    bWord : 1;
    bool                    bBack : 1;
    bool                    bExpr : 1;
    bool                    bCase : 1;
    bool                    bStyles:1;
    bool                    bSimilarity : 1;
    bool                    bLevRelax       :1;
    sal_Int16                   nLevExchange;
    sal_Int16                   nLevAdd;
    sal_Int16                   nLevRemove;

    bool                    bIsValueSearch :1;
protected:
    virtual ~SwXTextSearch();
public:
    SwXTextSearch();

    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

    //XSearchDescriptor
    virtual OUString SAL_CALL getSearchString(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSearchString( const OUString& aString ) throw(css::uno::RuntimeException, std::exception) override;

    //XReplaceDescriptor
    virtual OUString SAL_CALL getReplaceString() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setReplaceString(const OUString& aReplaceString) throw( css::uno::RuntimeException, std::exception ) override;

    //XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    //XPropertyReplace
    virtual sal_Bool SAL_CALL getValueSearch() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setValueSearch(sal_Bool ValueSearch_) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getSearchAttributes() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setSearchAttributes(const css::uno::Sequence< css::beans::PropertyValue >& aSearchAttribs) throw( css::beans::UnknownPropertyException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getReplaceAttributes() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setReplaceAttributes(const css::uno::Sequence< css::beans::PropertyValue >& aSearchAttribs) throw( css::beans::UnknownPropertyException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    void    FillSearchItemSet(SfxItemSet& rSet) const;
    void    FillReplaceItemSet(SfxItemSet& rSet) const;

    bool    HasSearchAttributes() const;
    bool    HasReplaceAttributes() const;

    void    FillSearchOptions( css::util::SearchOptions&
                                        rSearchOpt ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
