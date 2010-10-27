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
 ***********************************************************************/

#ifndef USERINPUTINTERCEPTION_HXX
#define USERINPUTINTERCEPTION_HXX

#include "sfx2/dllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/awt/XUserInputInterception.hpp>
/** === end UNO includes === **/

#include <boost/noncopyable.hpp>
#include <memory>

class NotifyEvent;

namespace cppu { class OWeakObject; }

//........................................................................
namespace sfx2
{
//........................................................................

    //====================================================================
    //= UserInputInterception
    //====================================================================
    struct UserInputInterception_Data;
    /** helper class for implementing the XUserInputInterception interface
        for a controller implementation
    */
    class SFX2_DLLPUBLIC UserInputInterception : public ::boost::noncopyable
    {
    public:
        UserInputInterception( ::cppu::OWeakObject& _rControllerImpl, ::osl::Mutex& _rMutex );
        ~UserInputInterception();

        // delegator functions for your XUserInputInterception implementation
        void    addKeyHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);
        void    removeKeyHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XKeyHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);
        void    addMouseClickHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);
        void    removeMouseClickHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMouseClickHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);

        // state
        bool    hasKeyHandlers() const;
        bool    hasMouseClickListeners() const;

        // forwarding a NotifyEvent to the KeyListeners respectively MouseClickListeners
        bool    handleNotifyEvent( const NotifyEvent& _rEvent );

    private:
        ::std::auto_ptr< UserInputInterception_Data >   m_pData;
    };

//........................................................................
} // namespace sfx2
//........................................................................

#endif // USERINPUTINTERCEPTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
