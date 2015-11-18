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

#ifndef INCLUDED_VCL_INC_XCONNECTION_HXX
#define INCLUDED_VCL_INC_XCONNECTION_HXX

#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include "displayconnectiondispatch.hxx"

#include <list>

namespace vcl {

    class DisplayConnection :
        public DisplayConnectionDispatch
    {
        ::osl::Mutex                    m_aMutex;
        ::std::list< css::uno::Reference< css::awt::XEventHandler > >
                                        m_aHandlers;
        ::std::list< css::uno::Reference< css::awt::XEventHandler > >
                                        m_aErrorHandlers;
        css::uno::Any                   m_aAny;
    public:
        DisplayConnection();
        virtual ~DisplayConnection();

        void start();
        void terminate();

        virtual bool dispatchEvent( void* pData, int nBytes ) override;

        // XDisplayConnection
        virtual void SAL_CALL addEventHandler( const css::uno::Any& window, const css::uno::Reference< css::awt::XEventHandler >& handler, sal_Int32 eventMask ) throw(std::exception) override;
        virtual void SAL_CALL removeEventHandler( const css::uno::Any& window, const css::uno::Reference< css::awt::XEventHandler >& handler ) throw(std::exception) override;
        virtual void SAL_CALL addErrorHandler( const css::uno::Reference< css::awt::XEventHandler >& handler ) throw(std::exception) override;
        virtual void SAL_CALL removeErrorHandler( const css::uno::Reference< css::awt::XEventHandler >& handler ) throw(std::exception) override;
        virtual css::uno::Any SAL_CALL getIdentifier() throw(std::exception) override;

    };

}

#endif // INCLUDED_VCL_INC_XCONNECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
