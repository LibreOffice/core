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

#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer3.hxx>
#include <comphelper/multiinterfacecontainer3.hxx>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <unotools/lingucfg.hxx>
#include <svl/itemprop.hxx>
#include <unotools/linguprops.hxx>
#include <com/sun/star/uno/Any.h>

namespace com::sun::star {
    namespace beans {
        struct PropertyChangeEvent;
    }
}


// LinguOptions
// This class represents all Linguistik relevant options.

class LinguOptions
{
    static SvtLinguOptions     *pData;
    static oslInterlockedCount  nRefCount;  // number of objects of this class

public:
    LinguOptions();
    LinguOptions(const LinguOptions &rOpt);
    ~LinguOptions();

    static OUString    GetName( sal_Int32 nWID );

    const css::uno::Sequence< OUString >&
            GetActiveDics() const   { return pData->aActiveDics; }
};

typedef comphelper::OMultiTypeInterfaceContainerHelperVar3<css::beans::XPropertyChangeListener, sal_Int32>
    OPropertyListenerContainerHelper;

class LinguProps :
    public cppu::WeakImplHelper
    <
        css::linguistic2::XLinguProperties,
        css::beans::XFastPropertySet,
        css::beans::XPropertyAccess,
        css::lang::XComponent,
        css::lang::XServiceInfo
    >
{
    ::comphelper::OInterfaceContainerHelper3<css::lang::XEventListener> aEvtListeners;
    OPropertyListenerContainerHelper            aPropListeners;

    SfxItemPropertyMap                          aPropertyMap;
    SvtLinguConfig                              aConfig;

    bool                                        bDisposing;

    LinguProps(const LinguProps &) = delete;
    LinguProps & operator = (const LinguProps &) = delete;

    void    launchEvent( const css::beans::PropertyChangeEvent &rEvt ) const;

    /// @throws css::uno::RuntimeException
    bool getPropertyBool(const OUString& aPropertyName);
    /// @throws css::uno::RuntimeException
    sal_Int16 getPropertyInt16(const OUString& aPropertyName);
    /// @throws css::uno::RuntimeException
    css::lang::Locale getPropertyLocale(const OUString& aPropertyName);
    /// @throws css::uno::RuntimeException
    void setProperty(const OUString& aPropertyName, bool p1)
        { setPropertyValue( aPropertyName, css::uno::Any(p1) ); }
    /// @throws css::uno::RuntimeException
    void setProperty(const OUString& aPropertyName, sal_Int16 p1)
        { setPropertyValue( aPropertyName, css::uno::Any(p1) ); }
    /// @throws css::uno::RuntimeException
    void setProperty(const OUString& aPropertyName, css::lang::Locale p1)
        { setPropertyValue( aPropertyName, css::uno::Any(p1) ); }

public:
    LinguProps();

    virtual sal_Bool SAL_CALL getIsUseDictionaryList() override
        { return getPropertyBool(UPN_IS_USE_DICTIONARY_LIST); }
    virtual void SAL_CALL setIsUseDictionaryList(sal_Bool p1) override
        { setProperty(UPN_IS_USE_DICTIONARY_LIST, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getIsIgnoreControlCharacters() override
        { return getPropertyBool(UPN_IS_IGNORE_CONTROL_CHARACTERS); }
    virtual void SAL_CALL setIsIgnoreControlCharacters(sal_Bool p1) override
        { setProperty(UPN_IS_IGNORE_CONTROL_CHARACTERS, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getIsSpellUpperCase() override
        { return getPropertyBool(UPN_IS_SPELL_UPPER_CASE); }
    virtual void SAL_CALL setIsSpellUpperCase(sal_Bool p1) override
        { setProperty(UPN_IS_SPELL_UPPER_CASE, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getIsSpellWithDigits() override
        { return getPropertyBool(UPN_IS_SPELL_WITH_DIGITS); }
    virtual void SAL_CALL setIsSpellWithDigits(sal_Bool p1) override
        { setProperty(UPN_IS_SPELL_WITH_DIGITS, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getIsSpellCapitalization() override
        { return false; }
    virtual void SAL_CALL setIsSpellCapitalization(sal_Bool /*p1*/) override
        { throw new css::beans::UnknownPropertyException( u"this deprecated property has been removed"_ustr); }
    virtual sal_Int16 SAL_CALL getHyphMinLeading() override
        { return getPropertyInt16(UPN_HYPH_MIN_LEADING); }
    virtual void SAL_CALL setHyphMinLeading(sal_Int16 p1) override
        { setProperty(UPN_HYPH_MIN_LEADING, p1); }
    virtual sal_Int16 SAL_CALL getHyphMinTrailing() override
        { return getPropertyInt16(UPN_HYPH_MIN_TRAILING); }
    virtual void SAL_CALL setHyphMinTrailing(sal_Int16 p1) override
        { setProperty(UPN_HYPH_MIN_TRAILING, p1); }
    virtual sal_Int16 SAL_CALL getHyphMinWordLength() override
        { return getPropertyInt16(UPN_HYPH_MIN_WORD_LENGTH); }
    virtual void SAL_CALL setHyphMinWordLength(sal_Int16 p1) override
        { setProperty(UPN_HYPH_MIN_WORD_LENGTH, p1); }
    virtual css::lang::Locale SAL_CALL getDefaultLocale() override
        { return getPropertyLocale(UPN_DEFAULT_LOCALE); }
    virtual void SAL_CALL setDefaultLocale(const css::lang::Locale& p1) override
        { setProperty(UPN_DEFAULT_LOCALE, p1); }
    virtual sal_Bool SAL_CALL getIsHyphAuto() override
        { return getPropertyBool(UPN_IS_HYPH_AUTO); }
    virtual void SAL_CALL setIsHyphAuto(sal_Bool p1) override
        { setProperty(UPN_IS_HYPH_AUTO, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getIsHyphSpecial() override
        { return getPropertyBool(UPN_IS_HYPH_SPECIAL); }
    virtual void SAL_CALL setIsHyphSpecial(sal_Bool p1) override
        { setProperty(UPN_IS_HYPH_SPECIAL, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getIsSpellAuto() override
        { return getPropertyBool(UPN_IS_SPELL_AUTO); }
    virtual void SAL_CALL setIsSpellAuto(sal_Bool p1) override
        { setProperty(UPN_IS_SPELL_AUTO, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getIsSpellSpecial() override
        { return getPropertyBool(UPN_IS_SPELL_SPECIAL); }
    virtual void SAL_CALL setIsSpellSpecial(sal_Bool p1) override
        { setProperty(UPN_IS_SPELL_SPECIAL, static_cast<bool>(p1)); }
    virtual sal_Bool SAL_CALL getIsWrapReverse() override
        { return getPropertyBool(UPN_IS_WRAP_REVERSE); }
    virtual void SAL_CALL setIsWrapReverse(sal_Bool p1) override
        { setProperty(UPN_IS_WRAP_REVERSE, static_cast<bool>(p1)); }
    virtual css::lang::Locale SAL_CALL getDefaultLocale_CJK() override
        { return getPropertyLocale(UPN_DEFAULT_LOCALE_CJK); }
    virtual void SAL_CALL setDefaultLocale_CJK(const css::lang::Locale& p1) override
        { setProperty(UPN_DEFAULT_LOCALE_CJK, p1); }
    virtual css::lang::Locale SAL_CALL getDefaultLocale_CTL() override
        { return getPropertyLocale(UPN_DEFAULT_LOCALE_CTL); }
    virtual void SAL_CALL setDefaultLocale_CTL(const css::lang::Locale& p1) override
        { setProperty(UPN_DEFAULT_LOCALE_CTL, p1); }

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override;

    // XFastPropertySet
    virtual void SAL_CALL setFastPropertyValue( sal_Int32 nHandle, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getFastPropertyValue( sal_Int32 nHandle ) override;

    // XPropertyAccess
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getPropertyValues() override;
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< css::beans::PropertyValue >& aProps ) override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
