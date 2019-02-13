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

#ifndef INCLUDED_LINGUISTIC_LNGPROPHELP_HXX
#define INCLUDED_LINGUISTIC_LNGPROPHELP_HXX

#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>
#include <linguistic/lngdllapi.h>
#include <rtl/ref.hxx>

namespace com { namespace sun { namespace star { namespace beans {
    class   XPropertySet;
}}}}

namespace com { namespace sun { namespace star { namespace linguistic2 {
    struct  LinguServiceEvent;
}}}}

namespace com::sun::star::linguistic2 { class XLinguProperties; }


namespace linguistic
{

// PropertyChgHelper
// Base class for all XPropertyChangeListener members of the
// various lingu services.


// Flags for type of events allowed to be launched
#define AE_SPELLCHECKER 1
#define AE_HYPHENATOR   2

typedef cppu::WeakImplHelper
<
    css::beans::XPropertyChangeListener,
    css::linguistic2::XLinguServiceEventBroadcaster
> PropertyChgHelperBase;

class PropertyChgHelper :
    public PropertyChgHelperBase
{
    css::uno::Sequence< OUString >                      aPropNames;
    css::uno::Reference< css::uno::XInterface >         xMyEvtObj;
    ::comphelper::OInterfaceContainerHelper2                   aLngSvcEvtListeners;
    css::uno::Reference< css::beans::XPropertySet >     xPropSet;

    int     nEvtFlags;  // flags for event types allowed to be launched

    // default values
    bool    bIsIgnoreControlCharacters;
    bool    bIsUseDictionaryList;

    // return values, will be set to default value or current temporary value
    bool    bResIsIgnoreControlCharacters;
    bool    bResIsUseDictionaryList;

    PropertyChgHelper( const PropertyChgHelper & ) = delete;
    PropertyChgHelper & operator = ( const PropertyChgHelper & ) = delete;

protected:
    virtual void    SetDefaultValues();
    virtual void    GetCurrentValues();

    css::uno::Sequence< OUString > &
            GetPropNames()  { return aPropNames; }
    css::uno::Reference<
        css::beans::XPropertySet > &
            GetPropSet()    { return xPropSet; }

    void    AddPropNames( const char *pNewNames[], sal_Int32 nCount );

    virtual bool    propertyChange_Impl( const css::beans::PropertyChangeEvent& rEvt );

public:
    PropertyChgHelper(
            const css::uno::Reference< css::uno::XInterface > &rxSource,
            css::uno::Reference< css::linguistic2::XLinguProperties > const &rxPropSet,
            int nAllowedEvents );
    virtual ~PropertyChgHelper() override;

    virtual void    SetTmpPropVals( const css::beans::PropertyValues &rPropVals );

    // XEventListener
    virtual void SAL_CALL
        disposing( const css::lang::EventObject& rSource ) override;

    // XPropertyChangeListener
    virtual void SAL_CALL
        propertyChange( const css::beans::PropertyChangeEvent& rEvt ) override;

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL
        addLinguServiceEventListener(
                const css::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxListener ) override;
    virtual sal_Bool SAL_CALL
        removeLinguServiceEventListener(
                const css::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxListener ) override;

    // non-UNO functions
    void    LNG_DLLPUBLIC AddAsPropListener();
    void    LNG_DLLPUBLIC RemoveAsPropListener();
    void    LaunchEvent( const css::linguistic2::LinguServiceEvent& rEvt );

    const css::uno::Sequence< OUString > &
            GetPropNames() const    { return aPropNames; }
    const css::uno::Reference< css::beans::XPropertySet > &
            GetPropSet() const      { return xPropSet; }
    const css::uno::Reference< css::uno::XInterface > &
            GetEvtObj() const       { return xMyEvtObj; }
};


class PropertyHelper_Thes :
    public PropertyChgHelper
{
    PropertyHelper_Thes( const PropertyHelper_Thes & ) = delete;
    PropertyHelper_Thes & operator = ( const PropertyHelper_Thes & ) = delete;

public:
    PropertyHelper_Thes(
            const css::uno::Reference< css::uno::XInterface > &rxSource,
            css::uno::Reference< css::linguistic2::XLinguProperties > const &rxPropSet );
    virtual ~PropertyHelper_Thes() override;

    // XPropertyChangeListener
    virtual void SAL_CALL
        propertyChange( const css::beans::PropertyChangeEvent& rEvt ) override;
};

class LNG_DLLPUBLIC PropertyHelper_Thesaurus
{
    rtl::Reference< PropertyHelper_Thes >    mxPropHelper;

    // disallow use of copy-constructor and assignment-operator
    PropertyHelper_Thesaurus( const PropertyHelper_Thes & ) = delete;
    PropertyHelper_Thesaurus & operator = ( const PropertyHelper_Thes & ) = delete;

public:
    PropertyHelper_Thesaurus(
            const css::uno::Reference< css::uno::XInterface > &rxSource,
            css::uno::Reference< css::linguistic2::XLinguProperties > const &rxPropSet );
    ~PropertyHelper_Thesaurus();
    void    AddAsPropListener();
    void    RemoveAsPropListener();
    void SetTmpPropVals( const css::beans::PropertyValues &rPropVals );
};


class LNG_DLLPUBLIC PropertyHelper_Spell :
    public PropertyChgHelper
{
    // default values
    bool        bIsSpellUpperCase;
    bool        bIsSpellWithDigits;
    bool        bIsSpellCapitalization;

    // return values, will be set to default value or current temporary value
    bool        bResIsSpellUpperCase;
    bool        bResIsSpellWithDigits;
    bool        bResIsSpellCapitalization;

    PropertyHelper_Spell( const PropertyHelper_Spell & ) = delete;
    PropertyHelper_Spell & operator = ( const PropertyHelper_Spell & ) = delete;

protected:
    // PropertyChgHelper
    virtual void    SetDefaultValues() override;
    virtual void    GetCurrentValues() override;
    virtual bool    propertyChange_Impl(
                            const css::beans::PropertyChangeEvent& rEvt ) override;

public:
    PropertyHelper_Spell(
            const css::uno::Reference< css::uno::XInterface > &rxSource,
            css::uno::Reference< css::linguistic2::XLinguProperties > const &rxPropSet );
    virtual ~PropertyHelper_Spell() override;

    virtual void    SetTmpPropVals( const css::beans::PropertyValues &rPropVals ) override;

    // XPropertyChangeListener
    virtual void SAL_CALL
        propertyChange( const css::beans::PropertyChangeEvent& rEvt ) override;

    bool        IsSpellUpperCase() const            { return bResIsSpellUpperCase; }
    bool        IsSpellWithDigits() const           { return bResIsSpellWithDigits; }
    bool        IsSpellCapitalization() const       { return bResIsSpellCapitalization; }
};


class LNG_DLLPUBLIC PropertyHelper_Spelling
{
    rtl::Reference< PropertyHelper_Spell >    mxPropHelper;

    // disallow use of copy-constructor and assignment-operator
    PropertyHelper_Spelling( const PropertyHelper_Spell & ) = delete;
    PropertyHelper_Spelling & operator = ( const PropertyHelper_Spell & ) = delete;

public:
    PropertyHelper_Spelling(
            const css::uno::Reference< css::uno::XInterface > &rxSource,
            css::uno::Reference< css::linguistic2::XLinguProperties > const &rxPropSet );
    ~PropertyHelper_Spelling();

    void    AddAsPropListener();
    void    RemoveAsPropListener();
    void    SetTmpPropVals( const css::beans::PropertyValues &rPropVals );
    bool    IsSpellUpperCase() const;
    bool    IsSpellWithDigits() const;
    bool    IsSpellCapitalization() const;
    /// @throws css::uno::RuntimeException
    bool addLinguServiceEventListener(
                const css::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxListener );
    /// @throws css::uno::RuntimeException
    bool removeLinguServiceEventListener(
                const css::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxListener );
};


class PropertyHelper_Hyphen :
    public PropertyChgHelper
{
    // default values
    sal_Int16   nHyphMinLeading,
            nHyphMinTrailing,
            nHyphMinWordLength;

    // return values, will be set to default value or current temporary value
    sal_Int16   nResHyphMinLeading,
            nResHyphMinTrailing,
            nResHyphMinWordLength;

    PropertyHelper_Hyphen( const PropertyHelper_Hyphen & ) = delete;
    PropertyHelper_Hyphen & operator = ( const PropertyHelper_Hyphen & ) = delete;

protected:
    // PropertyChgHelper
    virtual void    SetDefaultValues() override;
    virtual void    GetCurrentValues() override;
    virtual bool    propertyChange_Impl(
                            const css::beans::PropertyChangeEvent& rEvt ) override;

public:
    PropertyHelper_Hyphen(
            const css::uno::Reference< css::uno::XInterface > &rxSource,
            css::uno::Reference< css::linguistic2::XLinguProperties > const &rxPropSet);
    virtual ~PropertyHelper_Hyphen() override;

    virtual void    SetTmpPropVals( const css::beans::PropertyValues &rPropVals ) override;

    // XPropertyChangeListener
    virtual void SAL_CALL
        propertyChange( const css::beans::PropertyChangeEvent& rEvt ) override;

    sal_Int16   GetMinLeading() const               { return nResHyphMinLeading; }
    sal_Int16   GetMinTrailing() const              { return nResHyphMinTrailing; }
    sal_Int16   GetMinWordLength() const            { return nResHyphMinWordLength; }
};

class LNG_DLLPUBLIC PropertyHelper_Hyphenation
{
    rtl::Reference< PropertyHelper_Hyphen >    mxPropHelper;

    // disallow use of copy-constructor and assignment-operator
    PropertyHelper_Hyphenation( const PropertyHelper_Hyphen & ) = delete;
    PropertyHelper_Hyphenation & operator = ( const PropertyHelper_Hyphen & ) = delete;

public:
    PropertyHelper_Hyphenation(
            const css::uno::Reference< css::uno::XInterface > &rxSource,
            css::uno::Reference< css::linguistic2::XLinguProperties > const &rxPropSet);
    ~PropertyHelper_Hyphenation();

    void    AddAsPropListener();
    void    RemoveAsPropListener();
    void SetTmpPropVals( const css::beans::PropertyValues &rPropVals );
    sal_Int16   GetMinLeading() const;
    sal_Int16   GetMinTrailing() const;
    sal_Int16   GetMinWordLength() const;
    /// @throws css::uno::RuntimeException
    bool addLinguServiceEventListener(
                const css::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxListener );
    /// @throws css::uno::RuntimeException
    bool removeLinguServiceEventListener(
                const css::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxListener );
};

}   // namespace linguistic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
