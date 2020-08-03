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

#pragma once

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>


namespace svt
{


    //= OFilePickerInteractionHandler

    typedef ::cppu::WeakImplHelper <   css::task::XInteractionHandler
                                    >   OFilePickerInteractionHandler_Base;

    /** an InteractionHandler implementation which extends another handler with some customizability
    */
    class OFilePickerInteractionHandler final : public OFilePickerInteractionHandler_Base
    {
    public:
        /** flags, which indicates special handled interactions
            These values will be used combined as flags - so they must
            in range [2^n]!
         */
        enum EInterceptedInteractions
        {
            E_NOINTERCEPTION = 0,
            E_DOESNOTEXIST   = 1
            // next values [2,4,8,16 ...]!
        };

    private:
        css::uno::Reference< css::task::XInteractionHandler > m_xMaster;    // our master handler
        css::uno::Any                                         m_aException; // the last handled request
        bool                                                  m_bUsed;      // indicates using of this interaction handler instance
        EInterceptedInteractions                              m_eInterceptions; // enable/disable interception of some special interactions

    public:
        explicit OFilePickerInteractionHandler( const css::uno::Reference< css::task::XInteractionHandler >& _rxMaster );

        // some generic functions
        void     enableInterceptions( EInterceptedInteractions eInterceptions );
        bool wasUsed            () const { return m_bUsed; }
        void     resetUseState      ();
        void     forgetRequest      ();

        // functions to analyze last cached request
        bool wasAccessDenied() const;

    private:
        // XInteractionHandler
        virtual void SAL_CALL handle( const css::uno::Reference< css::task::XInteractionRequest >& _rxRequest ) override;

        virtual ~OFilePickerInteractionHandler() override;
    };


}   // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
