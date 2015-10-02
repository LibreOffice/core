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
#define EDITOR_LIST_REPLACE_EXISTING    (SAL_MAX_UINT16)
#define EDITOR_LIST_ENTRY_NOTFOUND      (SAL_MAX_UINT16)

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>

#include <toolkit/helper/vclunohelper.hxx>
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
    // in it's canonic form "aFoo.swap( Container() )" doesn't compile with GCC
    template< class CONTAINER >
    void clearContainer( CONTAINER& _rContainer )
    {
        CONTAINER aEmpty;
        _rContainer.swap( aEmpty );
    }


    //= HelpIdUrl

    /// small helper to translate help ids into help urls
    class HelpIdUrl
    {
    public:
        static OString getHelpId( const OUString& _rHelpURL );
        static OUString getHelpURL( const OString& );
    };


    //= StlSyntaxSequence

    template< class ELEMENT >
    class StlSyntaxSequence : public css::uno::Sequence< ELEMENT >
    {
    private:
        typedef css::uno::Sequence< ELEMENT >  UnoBase;

    public:
        inline StlSyntaxSequence() : UnoBase() { }
        inline StlSyntaxSequence( const UnoBase& rSeq ) : UnoBase( rSeq ) { }
        inline StlSyntaxSequence( const ELEMENT* pElements, sal_Int32 len ) : UnoBase( pElements, len ) { }
        inline StlSyntaxSequence( sal_Int32 len ) : UnoBase( len ) { }

        operator const UnoBase&() const { return *this; }
        operator       UnoBase&()       { return *this; }

        typedef const ELEMENT* const_iterator;
        typedef       ELEMENT* iterator;

        inline const_iterator begin() const { return UnoBase::getConstArray(); }
        inline const_iterator end() const { return UnoBase::getConstArray() + UnoBase::getLength(); }

        inline iterator begin() { return UnoBase::getArray(); }
        inline iterator end() { return UnoBase::getArray() + UnoBase::getLength(); }

        inline sal_Int32 size() const { return UnoBase::getLength(); }
        inline bool empty() const { return UnoBase::getLength() == 0; }
    };


    //= UNO helpers

#define DECLARE_XCOMPONENT() \
    virtual void SAL_CALL dispose(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE; \
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE; \
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

#define IMPLEMENT_FORWARD_XCOMPONENT( classname, baseclass ) \
    void SAL_CALL classname::dispose(  ) throw (css::uno::RuntimeException, std::exception) \
    { \
    baseclass::WeakComponentImplHelperBase::dispose(); \
    } \
    void SAL_CALL classname::addEventListener( const css::uno::Reference< css::lang::XEventListener >& _Listener ) throw (css::uno::RuntimeException, std::exception) \
    { \
        baseclass::WeakComponentImplHelperBase::addEventListener( _Listener ); \
    } \
    void SAL_CALL classname::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& _Listener ) throw (css::uno::RuntimeException, std::exception) \
    { \
        baseclass::WeakComponentImplHelperBase::removeEventListener( _Listener ); \
    } \


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PCRCOMMON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
