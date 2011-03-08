/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

#ifndef EXTENSIONS_PROPERTYCONTROLEXTENDER_HXX
#define EXTENSIONS_PROPERTYCONTROLEXTENDER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/inspection/XPropertyControl.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

#include <memory>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= PropertyControlExtender
    //====================================================================
    struct PropertyControlExtender_Data;
    typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::awt::XKeyListener
                                    >   PropertyControlExtender_Base;
    class PropertyControlExtender : public PropertyControlExtender_Base
    {
    public:
        PropertyControlExtender(
            const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl >& _rxObservedControl
        );

        // XKeyListener
        virtual void SAL_CALL keyPressed( const ::com::sun::star::awt::KeyEvent& e ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL keyReleased( const ::com::sun::star::awt::KeyEvent& e ) throw (::com::sun::star::uno::RuntimeException);
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~PropertyControlExtender();

    private:
        ::std::auto_ptr< PropertyControlExtender_Data > m_pData;
    };

//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_PROPERTYCONTROLEXTENDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
