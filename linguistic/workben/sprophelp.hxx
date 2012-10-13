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

#ifndef _LINGU2_PROPHELP_HXX_
#define _LINGU2_PROPHELP_HXX_

#include <tools/solar.h>

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase2.hxx> // helper for implementations
#include <cppuhelper/interfacecontainer.h>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <com/sun/star/linguistic2/XLinguServiceEventBroadcaster.hpp>

namespace com { namespace sun { namespace star { namespace beans {
    class   XPropertySet;
}}}};

namespace com { namespace sun { namespace star { namespace linguistic2 {
    struct  LinguServiceEvent;
}}}};


using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

// PropertyChgHelper
// virtual base class for all XPropertyChangeListener members of the
// various lingu services.
// Only propertyChange needs to be implemented.

class PropertyChgHelper :
    public cppu::WeakImplHelper2
    <
        XPropertyChangeListener,
        XLinguServiceEventBroadcaster
    >
{
    Sequence< OUString >                aPropNames;
    Reference< XInterface >             xMyEvtObj;
    ::cppu::OInterfaceContainerHelper   aLngSvcEvtListeners;
    Reference< XPropertySet >           xPropSet;

    // disallow use of copy-constructor and assignment-operator
    PropertyChgHelper( const PropertyChgHelper & );
    PropertyChgHelper & operator = ( const PropertyChgHelper & );

public:
    PropertyChgHelper(
            const Reference< XInterface > &rxSource,
            Reference< XPropertySet > &rxPropSet,
            const char *pPropNames[], sal_uInt16 nPropCount );
    virtual ~PropertyChgHelper();

    // XEventListener
    virtual void SAL_CALL
        disposing( const EventObject& rSource )
            throw(RuntimeException);

    // XPropertyChangeListener
    virtual void SAL_CALL
        propertyChange( const PropertyChangeEvent& rEvt )
            throw(RuntimeException) = 0;

    // XLinguServiceEventBroadcaster
    virtual sal_Bool SAL_CALL
        addLinguServiceEventListener(
                const Reference< XLinguServiceEventListener >& rxListener )
            throw(RuntimeException);
    virtual sal_Bool SAL_CALL
        removeLinguServiceEventListener(
                const Reference< XLinguServiceEventListener >& rxListener )
            throw(RuntimeException);

    // non UNO functions
    void    AddAsPropListener();
    void    RemoveAsPropListener();
    void    LaunchEvent( const LinguServiceEvent& rEvt );

    const Sequence< OUString > &
            GetPropNames() const    { return aPropNames; }
    const Reference< XPropertySet > &
            GetPropSet() const      { return xPropSet; }
    const Reference< XInterface > &
            GetEvtObj() const       { return xMyEvtObj; }
};




class PropertyHelper_Spell :
    public PropertyChgHelper
{
    // default values
    sal_Bool    bIsGermanPreReform;
    sal_Bool    bIsIgnoreControlCharacters;
    sal_Bool    bIsUseDictionaryList;
    sal_Bool    bIsSpellUpperCase;
    sal_Bool    bIsSpellWithDigits;
    sal_Bool    bIsSpellCapitalization;

    // return values, will be set to default value or current temporary value
    sal_Bool    bResIsGermanPreReform;
    sal_Bool    bResIsIgnoreControlCharacters;
    sal_Bool    bResIsUseDictionaryList;
    sal_Bool    bResIsSpellUpperCase;
    sal_Bool    bResIsSpellWithDigits;
    sal_Bool    bResIsSpellCapitalization;


    // disallow use of copy-constructor and assignment-operator
    PropertyHelper_Spell( const PropertyHelper_Spell & );
    PropertyHelper_Spell & operator = ( const PropertyHelper_Spell & );

    void    SetDefault();

public:
    PropertyHelper_Spell(
            const Reference< XInterface > &rxSource,
            Reference< XPropertySet > &rxPropSet );
    virtual ~PropertyHelper_Spell();

    // XPropertyChangeListener
    virtual void SAL_CALL
        propertyChange( const PropertyChangeEvent& rEvt )
            throw(RuntimeException);

    void    SetTmpPropVals( const PropertyValues &rPropVals );

    sal_Bool    IsGermanPreReform() const           { return bResIsGermanPreReform; }
    sal_Bool    IsIgnoreControlCharacters() const   { return bResIsIgnoreControlCharacters; }
    sal_Bool    IsUseDictionaryList() const         { return bResIsUseDictionaryList; }
    sal_Bool    IsSpellUpperCase() const            { return bResIsSpellUpperCase; }
    sal_Bool    IsSpellWithDigits() const           { return bResIsSpellWithDigits; }
    sal_Bool    IsSpellCapitalization() const       { return bResIsSpellCapitalization; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
