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
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>
#include <linguistic/lngdllapi.h>

namespace com { namespace sun { namespace star { namespace beans {
    class   XPropertySet;
}}}}

namespace com { namespace sun { namespace star { namespace linguistic2 {
    struct  LinguServiceEvent;
}}}}


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
    ::com::sun::star::beans::XPropertyChangeListener,
    ::com::sun::star::linguistic2::XLinguServiceEventBroadcaster
> PropertyChgHelperBase;

class PropertyChgHelper :
    public PropertyChgHelperBase
{
    ::com::sun::star::uno::Sequence< OUString >  aPropNames;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface >             xMyEvtObj;
    ::cppu::OInterfaceContainerHelper                   aLngSvcEvtListeners;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >         xPropSet;

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

    ::com::sun::star::uno::Sequence< OUString > &
            GetPropNames()  { return aPropNames; }
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > &
            GetPropSet()    { return xPropSet; }

    void    AddPropNames( const char *pNewNames[], sal_Int32 nCount );

    virtual bool    propertyChange_Impl(
                            const ::com::sun::star::beans::PropertyChangeEvent& rEvt );

public:
    PropertyChgHelper(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > &rxSource,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XLinguProperties > &rxPropSet,
            int nAllowedEvents );
    virtual ~PropertyChgHelper();

    virtual void    SetTmpPropVals( const com::sun::star::beans::PropertyValues &rPropVals );

    // XEventListener
    virtual void SAL_CALL
        disposing( const ::com::sun::star::lang::EventObject& rSource )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertyChangeListener
    virtual void SAL_CALL
        propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& rEvt )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL
        addLinguServiceEventListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventListener >& rxListener )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL
        removeLinguServiceEventListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventListener >& rxListener )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // non-UNO functions
    void    LNG_DLLPUBLIC AddAsPropListener();
    void    LNG_DLLPUBLIC RemoveAsPropListener();
    void    LaunchEvent(
                const ::com::sun::star::linguistic2::LinguServiceEvent& rEvt );

    const ::com::sun::star::uno::Sequence< OUString > &
            GetPropNames() const    { return aPropNames; }
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > &
            GetPropSet() const      { return xPropSet; }
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > &
            GetEvtObj() const       { return xMyEvtObj; }
};


class PropertyHelper_Thes :
    public PropertyChgHelper
{
    PropertyHelper_Thes( const PropertyHelper_Thes & ) = delete;
    PropertyHelper_Thes & operator = ( const PropertyHelper_Thes & ) = delete;

public:
    PropertyHelper_Thes(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > &rxSource,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XLinguProperties > &rxPropSet );
    virtual ~PropertyHelper_Thes();

    // XPropertyChangeListener
    virtual void SAL_CALL
        propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& rEvt )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

class LNG_DLLPUBLIC PropertyHelper_Thesaurus
{
    PropertyHelper_Thes* pInst;
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >    xPropHelper;

    // disallow use of copy-constructor and assignment-operator
    PropertyHelper_Thesaurus( const PropertyHelper_Thes & );
    PropertyHelper_Thesaurus & operator = ( const PropertyHelper_Thes & );

public:
    PropertyHelper_Thesaurus(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > &rxSource,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XLinguProperties > &rxPropSet );
    ~PropertyHelper_Thesaurus();
    void    AddAsPropListener();
    void    RemoveAsPropListener();
    void SetTmpPropVals( const com::sun::star::beans::PropertyValues &rPropVals );
};


class LNG_DLLPUBLIC PropertyHelper_Spell :
    public PropertyChgHelper
{
    // default values
    bool        bIsSpellUpperCase;
    bool        bIsSpellWithDigits;
    bool        bIsSpellCapitalization;

    // return values, will be set to default value or current temporary value
    sal_Int16   nResMaxNumberOfSuggestions; // special value that is not part of the property set and thus needs to be handled differently
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
                            const ::com::sun::star::beans::PropertyChangeEvent& rEvt ) override;

public:
    PropertyHelper_Spell(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > &rxSource,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XLinguProperties > &rxPropSet );
    virtual ~PropertyHelper_Spell();

    virtual void    SetTmpPropVals( const com::sun::star::beans::PropertyValues &rPropVals ) override;

    // XPropertyChangeListener
    virtual void SAL_CALL
        propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& rEvt )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    static sal_Int16 GetDefaultNumberOfSuggestions() { return 16; }

    bool        IsSpellUpperCase() const            { return bResIsSpellUpperCase; }
    bool        IsSpellWithDigits() const           { return bResIsSpellWithDigits; }
    bool        IsSpellCapitalization() const       { return bResIsSpellCapitalization; }
};


class LNG_DLLPUBLIC PropertyHelper_Spelling
{
    PropertyHelper_Spell* pInst;
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >    xPropHelper;

    // disallow use of copy-constructor and assignment-operator
    PropertyHelper_Spelling( const PropertyHelper_Spell & );
    PropertyHelper_Spelling & operator = ( const PropertyHelper_Spell & );

public:
    PropertyHelper_Spelling(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > &rxSource,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XLinguProperties > &rxPropSet );
    ~PropertyHelper_Spelling();

    void    AddAsPropListener();
    void    RemoveAsPropListener();
    void    SetTmpPropVals( const com::sun::star::beans::PropertyValues &rPropVals );
    bool    IsSpellUpperCase() const;
    bool    IsSpellWithDigits() const;
    bool    IsSpellCapitalization() const;
    bool addLinguServiceEventListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventListener >& rxListener )
            throw(::com::sun::star::uno::RuntimeException);
    bool removeLinguServiceEventListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventListener >& rxListener )
            throw(::com::sun::star::uno::RuntimeException);
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
                            const ::com::sun::star::beans::PropertyChangeEvent& rEvt ) override;

public:
    PropertyHelper_Hyphen(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > &rxSource,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XLinguProperties > &rxPropSet);
    virtual ~PropertyHelper_Hyphen();

    virtual void    SetTmpPropVals( const com::sun::star::beans::PropertyValues &rPropVals ) override;

    // XPropertyChangeListener
    virtual void SAL_CALL
        propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& rEvt )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    sal_Int16   GetMinLeading() const               { return nResHyphMinLeading; }
    sal_Int16   GetMinTrailing() const              { return nResHyphMinTrailing; }
    sal_Int16   GetMinWordLength() const            { return nResHyphMinWordLength; }
};

class LNG_DLLPUBLIC PropertyHelper_Hyphenation
{
    PropertyHelper_Hyphen* pInst;
    com::sun::star::uno::Reference< com::sun::star::beans::XPropertyChangeListener >    xPropHelper;

    // disallow use of copy-constructor and assignment-operator
    PropertyHelper_Hyphenation( const PropertyHelper_Hyphen & );
    PropertyHelper_Hyphenation & operator = ( const PropertyHelper_Hyphen & );

public:
    PropertyHelper_Hyphenation(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > &rxSource,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XLinguProperties > &rxPropSet);
    ~PropertyHelper_Hyphenation();

    void    AddAsPropListener();
    void    RemoveAsPropListener();
    void SetTmpPropVals( const com::sun::star::beans::PropertyValues &rPropVals );
    sal_Int16   GetMinLeading() const;
    sal_Int16   GetMinTrailing() const;
    sal_Int16   GetMinWordLength() const;
    bool addLinguServiceEventListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventListener >& rxListener )
            throw(::com::sun::star::uno::RuntimeException);
    bool removeLinguServiceEventListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventListener >& rxListener )
            throw(::com::sun::star::uno::RuntimeException);
};

}   // namespace linguistic

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
