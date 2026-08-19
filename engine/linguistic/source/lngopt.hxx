/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <cpo/uno/Any.h>

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

    const cpo::uno::Sequence< OUString >&
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
        { setPropertyValue( aPropertyName, cpo::uno::Any(p1) ); }
    /// @throws css::uno::RuntimeException
    void setProperty(const OUString& aPropertyName, sal_Int16 p1)
        { setPropertyValue( aPropertyName, cpo::uno::Any(p1) ); }
    /// @throws css::uno::RuntimeException
    void setProperty(const OUString& aPropertyName, css::lang::Locale p1)
        { setPropertyValue( aPropertyName, cpo::uno::Any(p1) ); }

public:
    LinguProps();

    virtual bool getIsUseDictionaryList() override
        { return getPropertyBool(UPN_IS_USE_DICTIONARY_LIST); }
    virtual void setIsUseDictionaryList(bool p1) override
        { setProperty(UPN_IS_USE_DICTIONARY_LIST, p1); }
    virtual bool getIsIgnoreControlCharacters() override
        { return getPropertyBool(UPN_IS_IGNORE_CONTROL_CHARACTERS); }
    virtual void setIsIgnoreControlCharacters(bool p1) override
        { setProperty(UPN_IS_IGNORE_CONTROL_CHARACTERS, p1); }
    virtual bool getIsSpellUpperCase() override
        { return getPropertyBool(UPN_IS_SPELL_UPPER_CASE); }
    virtual void setIsSpellUpperCase(bool p1) override
        { setProperty(UPN_IS_SPELL_UPPER_CASE, p1); }
    virtual bool getIsSpellWithDigits() override
        { return getPropertyBool(UPN_IS_SPELL_WITH_DIGITS); }
    virtual void setIsSpellWithDigits(bool p1) override
        { setProperty(UPN_IS_SPELL_WITH_DIGITS, p1); }
    virtual bool getIsSpellCapitalization() override
        { return false; }
    virtual void setIsSpellCapitalization(bool /*p1*/) override
        { throw new css::beans::UnknownPropertyException( u"this deprecated property has been removed"_ustr); }
    virtual sal_Int16 getHyphMinLeading() override
        { return getPropertyInt16(UPN_HYPH_MIN_LEADING); }
    virtual void setHyphMinLeading(sal_Int16 p1) override
        { setProperty(UPN_HYPH_MIN_LEADING, p1); }
    virtual sal_Int16 getHyphMinTrailing() override
        { return getPropertyInt16(UPN_HYPH_MIN_TRAILING); }
    virtual void setHyphMinTrailing(sal_Int16 p1) override
        { setProperty(UPN_HYPH_MIN_TRAILING, p1); }
    virtual sal_Int16 getHyphMinWordLength() override
        { return getPropertyInt16(UPN_HYPH_MIN_WORD_LENGTH); }
    virtual void setHyphMinWordLength(sal_Int16 p1) override
        { setProperty(UPN_HYPH_MIN_WORD_LENGTH, p1); }
    virtual css::lang::Locale getDefaultLocale() override
        { return getPropertyLocale(UPN_DEFAULT_LOCALE); }
    virtual void setDefaultLocale(const css::lang::Locale& p1) override
        { setProperty(UPN_DEFAULT_LOCALE, p1); }
    virtual bool getIsHyphAuto() override
        { return getPropertyBool(UPN_IS_HYPH_AUTO); }
    virtual void setIsHyphAuto(bool p1) override
        { setProperty(UPN_IS_HYPH_AUTO, p1); }
    virtual bool getIsHyphSpecial() override
        { return getPropertyBool(UPN_IS_HYPH_SPECIAL); }
    virtual void setIsHyphSpecial(bool p1) override
        { setProperty(UPN_IS_HYPH_SPECIAL, p1); }
    virtual bool getIsSpellAuto() override
        { return getPropertyBool(UPN_IS_SPELL_AUTO); }
    virtual void setIsSpellAuto(bool p1) override
        { setProperty(UPN_IS_SPELL_AUTO, p1); }
    virtual bool getIsSpellSpecial() override
        { return getPropertyBool(UPN_IS_SPELL_SPECIAL); }
    virtual void setIsSpellSpecial(bool p1) override
        { setProperty(UPN_IS_SPELL_SPECIAL, p1); }
    virtual bool getIsWrapReverse() override
        { return getPropertyBool(UPN_IS_WRAP_REVERSE); }
    virtual void setIsWrapReverse(bool p1) override
        { setProperty(UPN_IS_WRAP_REVERSE, p1); }
    virtual css::lang::Locale getDefaultLocale_CJK() override
        { return getPropertyLocale(UPN_DEFAULT_LOCALE_CJK); }
    virtual void setDefaultLocale_CJK(const css::lang::Locale& p1) override
        { setProperty(UPN_DEFAULT_LOCALE_CJK, p1); }
    virtual css::lang::Locale getDefaultLocale_CTL() override
        { return getPropertyLocale(UPN_DEFAULT_LOCALE_CTL); }
    virtual void setDefaultLocale_CTL(const css::lang::Locale& p1) override
        { setProperty(UPN_DEFAULT_LOCALE_CTL, p1); }

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() override;
    virtual void setPropertyValue( const OUString& aPropertyName, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getPropertyValue( const OUString& PropertyName ) override;
    virtual void addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override;
    virtual void removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& rxListener ) override;
    virtual void addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override;
    virtual void removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& rxListener ) override;

    // XFastPropertySet
    virtual void setFastPropertyValue( sal_Int32 nHandle, const cpo::uno::Any& aValue ) override;
    virtual cpo::uno::Any getFastPropertyValue( sal_Int32 nHandle ) override;

    // XPropertyAccess
    virtual cpo::uno::Sequence< css::beans::PropertyValue > getPropertyValues() override;
    virtual void setPropertyValues( const cpo::uno::Sequence< css::beans::PropertyValue >& aProps ) override;

    // XComponent
    virtual void dispose() override;
    virtual void addEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;
    virtual void removeEventListener( const css::uno::Reference< css::lang::XEventListener >& rxListener ) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;
    virtual bool supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
