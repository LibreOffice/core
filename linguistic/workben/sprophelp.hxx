/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
