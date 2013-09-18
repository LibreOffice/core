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
#ifndef _UNOSRCH_HXX
#define _UNOSRCH_HXX

#include <com/sun/star/util/XPropertyReplace.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase3.hxx> // helper for implementations
#include <tools/string.hxx>

class SfxItemPropertySet;
class SwXTextDocument;
class SwSearchProperties_Impl;
class SfxItemSet;

namespace com{namespace sun{namespace star{namespace util{
    struct SearchOptions;
}}}}


class SwXTextSearch : public cppu::WeakImplHelper3
<
    ::com::sun::star::util::XPropertyReplace,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XUnoTunnel
>
{
    friend class SwXTextDocument;

    OUString                sSearchText;
    OUString                sReplaceText;

    SwSearchProperties_Impl*    pSearchProperties;
    SwSearchProperties_Impl*    pReplaceProperties;


    const SfxItemPropertySet*   m_pPropSet;
    sal_Bool                    bAll  : 1;
    sal_Bool                    bWord : 1;
    sal_Bool                    bBack : 1;
    sal_Bool                    bExpr : 1;
    sal_Bool                    bCase : 1;
    sal_Bool                    bStyles:1;
    sal_Bool                    bSimilarity : 1;
    sal_Bool                    bLevRelax       :1;
    sal_Int16                   nLevExchange;
    sal_Int16                   nLevAdd;
    sal_Int16                   nLevRemove;

    sal_Bool                    bIsValueSearch :1;
protected:
    virtual ~SwXTextSearch();
public:
    SwXTextSearch();



    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId();

    //XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    //XSearchDescriptor
    virtual OUString SAL_CALL getSearchString(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSearchString( const OUString& aString ) throw(::com::sun::star::uno::RuntimeException);

    //XReplaceDescriptor
    virtual OUString SAL_CALL getReplaceString(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setReplaceString(const OUString& aReplaceString) throw( ::com::sun::star::uno::RuntimeException );

    //XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    //XPropertyReplace
    virtual sal_Bool SAL_CALL getValueSearch(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setValueSearch(sal_Bool ValueSearch_) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > SAL_CALL getSearchAttributes(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setSearchAttributes(const ::com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& aSearchAttribs) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > SAL_CALL getReplaceAttributes(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setReplaceAttributes(const ::com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& aSearchAttribs) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName(void) throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(void) throw( ::com::sun::star::uno::RuntimeException );

    void    FillSearchItemSet(SfxItemSet& rSet) const;
    void    FillReplaceItemSet(SfxItemSet& rSet) const;

    sal_Bool    HasSearchAttributes() const;
    sal_Bool    HasReplaceAttributes() const;

    void    FillSearchOptions( ::com::sun::star::util::SearchOptions&
                                        rSearchOpt ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
