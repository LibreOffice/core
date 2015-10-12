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

#ifndef INCLUDED_FPICKER_SOURCE_OFFICE_FPINTERACTION_HXX
#define INCLUDED_FPICKER_SOURCE_OFFICE_FPINTERACTION_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/task/XInteractionHandler.hpp>


namespace svt
{



    //= OFilePickerInteractionHandler

    typedef ::cppu::WeakImplHelper <   ::com::sun::star::task::XInteractionHandler
                                    >   OFilePickerInteractionHandler_Base;

    /** a InteractionHandler implementation which extends another handler with some customizability
    */
    class OFilePickerInteractionHandler : public OFilePickerInteractionHandler_Base
    {
    public:
        /** flags, which indicates special handled interactions
            These values will be used combained as flags - so they must
            in range [2^n]!
         */
        enum EInterceptedInteractions
        {
            E_NOINTERCEPTION = 0,
            E_DOESNOTEXIST   = 1
            // next values [2,4,8,16 ...]!
        };

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler > m_xMaster        ; // our master handler
        ::com::sun::star::uno::Any                                                      m_aException     ; // the last handled request
        bool                                                                        m_bUsed          ; // indicates using of this interaction handler instance
        EInterceptedInteractions                                                        m_eInterceptions ; // enable/disable interception of some special interactions

    public:
        OFilePickerInteractionHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxMaster );

        // some generic functions
        void     enableInterceptions( EInterceptedInteractions eInterceptions );
        bool wasUsed            () const { return m_bUsed; }
        void     resetUseState      ();
        void     forgetRequest      ();

        // functions to analyze last cached request
        bool wasAccessDenied() const;

    protected:
        // XInteractionHandler
        virtual void SAL_CALL handle( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionRequest >& _rxRequest ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    private:
        virtual ~OFilePickerInteractionHandler();
    };


}   // namespace svt


#endif // INCLUDED_FPICKER_SOURCE_OFFICE_FPINTERACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
