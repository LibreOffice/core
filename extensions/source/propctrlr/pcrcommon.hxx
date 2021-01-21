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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PCRCOMMON_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PCRCOMMON_HXX

#define EDITOR_LIST_APPEND              (SAL_MAX_UINT16)
#define EDITOR_LIST_ENTRY_NOTFOUND      (SAL_MAX_UINT16)

#include <sal/config.h>

#include <string_view>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>

#include <comphelper/listenernotification.hxx>


namespace pcr
{


    #define OWN_PROPERTY_ID_INTROSPECTEDOBJECT      0x0010
    #define OWN_PROPERTY_ID_CURRENTPAGE             0x0011
    #define OWN_PROPERTY_ID_CONTROLCONTEXT          0x0012
    #define OWN_PROPERTY_ID_TABBINGMODEL            0x0013


    //= types

    typedef ::comphelper::OSimpleListenerContainer  <   css::beans::XPropertyChangeListener
                                                    ,   css::beans::PropertyChangeEvent
                                                    >   PropertyChangeListeners;


    //= helper

    // small helper to make the "swap" call on an STL container a single-line call, which
    // in its canonic form "aFoo.swap( Container() )" doesn't compile with GCC
    template< class CONTAINER >
    void clearContainer( CONTAINER& _rContainer )
    {
        CONTAINER().swap(_rContainer);
    }


    //= HelpIdUrl

    /// small helper to translate help ids into help urls
    class HelpIdUrl
    {
    public:
        static OString getHelpId( const OUString& _rHelpURL );
        static OUString getHelpURL( std::string_view );
    };


    //= StlSyntaxSequence

    template< class ELEMENT >
    class StlSyntaxSequence : public css::uno::Sequence< ELEMENT >
    {
    private:
        typedef css::uno::Sequence< ELEMENT >  UnoBase;

    public:
        StlSyntaxSequence() : UnoBase() { }
        explicit StlSyntaxSequence( const UnoBase& rSeq ) : UnoBase( rSeq ) { }
        explicit StlSyntaxSequence( sal_Int32 len ) : UnoBase( len ) { }

        typedef const ELEMENT* const_iterator;
        typedef       ELEMENT* iterator;

        const_iterator begin() const { return UnoBase::getConstArray(); }
        const_iterator end() const { return UnoBase::getConstArray() + UnoBase::getLength(); }

        iterator begin() { return UnoBase::getArray(); }
        iterator end() { return UnoBase::getArray() + UnoBase::getLength(); }

        sal_Int32 size() const { return UnoBase::getLength(); }
        bool empty() const { return !UnoBase::hasElements(); }
    };


    //= UNO helpers

#define DECLARE_XCOMPONENT() \
    virtual void SAL_CALL dispose(  ) override; \
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override; \
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

#define IMPLEMENT_FORWARD_XCOMPONENT( classname, baseclass ) \
    void SAL_CALL classname::dispose(  ) \
    { \
    baseclass::WeakComponentImplHelperBase::dispose(); \
    } \
    void SAL_CALL classname::addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener ) \
    { \
        baseclass::WeakComponentImplHelperBase::addEventListener( Listener ); \
    } \
    void SAL_CALL classname::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener ) \
    { \
        baseclass::WeakComponentImplHelperBase::removeEventListener( Listener ); \
    } \


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PCRCOMMON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
