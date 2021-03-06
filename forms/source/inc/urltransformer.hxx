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

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/URL.hpp>


namespace frm
{

    class UrlTransformer
    {
    private:
        css::uno::Reference< css::uno::XComponentContext >
                        m_xORB;
        mutable css::uno::Reference< css::util::XURLTransformer >
                        m_xTransformer;
        mutable bool    m_bTriedToCreateTransformer;

    public:
        UrlTransformer( const css::uno::Reference< css::uno::XComponentContext >& _rxORB );

        /** returns a URL object for the given URL string
        */
        css::util::URL
            getStrictURL( const OUString& _rURL ) const;

        /** returns a URL object for the given URL ASCII string
        */
        css::util::URL
            getStrictURLFromAscii( const char* _pAsciiURL ) const;

        /** parses a given URL smartly, with a protocol given by ASCII string
        */
        void
            parseSmartWithAsciiProtocol( css::util::URL& _rURL, const char* _pAsciiURL ) const;

    private:
        /** ensures that we have a URLTransformer instance in <member>m_xTransformer</member>

            @return
                <TRUE/> if and only if m_xTransformer is not <NULL/>
        */
        bool    implEnsureTransformer() const;
    };


} // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
