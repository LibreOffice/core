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

#ifndef EXTENSIONS_PROPERTYCONTROLEXTENDER_HXX
#define EXTENSIONS_PROPERTYCONTROLEXTENDER_HXX

#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/inspection/XPropertyControl.hpp>

#include <cppuhelper/implbase1.hxx>

#include <memory>


namespace pcr
{



    //= PropertyControlExtender

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
        virtual void SAL_CALL keyPressed( const ::com::sun::star::awt::KeyEvent& e ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL keyReleased( const ::com::sun::star::awt::KeyEvent& e ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    protected:
        virtual ~PropertyControlExtender();

    private:
        ::std::auto_ptr< PropertyControlExtender_Data > m_pData;
    };


} // namespace pcr


#endif // EXTENSIONS_PROPERTYCONTROLEXTENDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
