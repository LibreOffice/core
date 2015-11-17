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

#ifndef INCLUDED_UNOTOOLS_EVENTLISTENERADAPTER_HXX
#define INCLUDED_UNOTOOLS_EVENTLISTENERADAPTER_HXX

#include <unotools/unotoolsdllapi.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <memory>

namespace utl
{

    struct OEventListenerAdapterImpl;

    //= OEventListenerAdapter

    /** base class for non-UNO dispose listeners
    */
    class UNOTOOLS_DLLPUBLIC OEventListenerAdapter
    {
        friend class OEventListenerImpl;

    private:
        OEventListenerAdapter( const OEventListenerAdapter& _rSource ) = delete;
        const OEventListenerAdapter& operator=( const OEventListenerAdapter& _rSource ) = delete;

    protected:
        std::unique_ptr<OEventListenerAdapterImpl>  m_pImpl;

    protected:
                OEventListenerAdapter();
        virtual ~OEventListenerAdapter();

        void startComponentListening( const css::uno::Reference< css::lang::XComponent >& _rxComp );
        void stopComponentListening( const css::uno::Reference< css::lang::XComponent >& _rxComp );
        void stopAllComponentListening(  );

        virtual void _disposing( const css::lang::EventObject& _rSource ) = 0;
    };

}   // namespace utl

#endif // INCLUDED_UNOTOOLS_EVENTLISTENERADAPTER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
