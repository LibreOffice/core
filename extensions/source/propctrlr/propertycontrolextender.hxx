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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCONTROLEXTENDER_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCONTROLEXTENDER_HXX

#include <com/sun/star/awt/XKeyListener.hpp>
#include <com/sun/star/inspection/XPropertyControl.hpp>

#include <cppuhelper/implbase.hxx>

#include <memory>


namespace pcr
{



    //= PropertyControlExtender

    struct PropertyControlExtender_Data;
    typedef ::cppu::WeakImplHelper <   css::awt::XKeyListener
                                    >   PropertyControlExtender_Base;
    class PropertyControlExtender : public PropertyControlExtender_Base
    {
    public:
        PropertyControlExtender(
            const css::uno::Reference< css::inspection::XPropertyControl >& _rxObservedControl
        );

        // XKeyListener
        virtual void SAL_CALL keyPressed( const css::awt::KeyEvent& e ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL keyReleased( const css::awt::KeyEvent& e ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
        virtual ~PropertyControlExtender();

    private:
        ::std::unique_ptr< PropertyControlExtender_Data > m_pData;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PROPERTYCONTROLEXTENDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
