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

#ifndef INCLUDED_FORMS_SOURCE_INC_URLTRANSFORMER_HXX
#define INCLUDED_FORMS_SOURCE_INC_URLTRANSFORMER_HXX

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/URL.hpp>


namespace frm
{

    class UrlTransformer
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        m_xORB;
        mutable ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >
                        m_xTransformer;
        mutable bool    m_bTriedToCreateTransformer;

    public:
        UrlTransformer( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB );

        /** returns an URL object for the given URL string
        */
        ::com::sun::star::util::URL
            getStrictURL( const OUString& _rURL ) const;

        /** returns an URL object for the given URL ASCII string
        */
        ::com::sun::star::util::URL
            getStrictURLFromAscii( const sal_Char* _pAsciiURL ) const;

        /** parses a given URL smartly, with a protocol given by ASCII string
        */
        void
            parseSmartWithAsciiProtocol( ::com::sun::star::util::URL& _rURL, const sal_Char* _pAsciiURL ) const;

    private:
        /** ensures that we have an URLTransformer instance in <member>m_xTransformer</member>

            @return
                <TRUE/> if and only if m_xTransformer is not <NULL/>
        */
        bool    implEnsureTransformer() const;
    };


}


#endif // INCLUDED_FORMS_SOURCE_INC_URLTRANSFORMER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
