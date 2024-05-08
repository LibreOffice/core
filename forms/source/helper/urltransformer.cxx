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

#include <urltransformer.hxx>

#include <com/sun/star/util/URLTransformer.hpp>
#include <tools/debug.hxx>


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;

    UrlTransformer::UrlTransformer( const Reference< XComponentContext >& _rxORB )
        :m_xORB( _rxORB )
        ,m_bTriedToCreateTransformer( false )
    {
        DBG_ASSERT( _rxORB.is(), "UrlTransformer::UrlTransformer: invalid service factory!" );
    }


    bool UrlTransformer::implEnsureTransformer() const
    {
        // create the transformer, if not already attempted to do so
        if ( !m_xTransformer.is() && !m_bTriedToCreateTransformer )
        {
            if ( m_xORB.is() )
            {
                m_xTransformer.set(URLTransformer::create(m_xORB));
            }

            m_bTriedToCreateTransformer = true;
        }
        return m_xTransformer.is();
    }


    URL UrlTransformer::getStrictURL( const OUString& _rURL ) const
    {
        URL aReturn;
        aReturn.Complete = _rURL;
        if ( implEnsureTransformer() )
            m_xTransformer->parseStrict( aReturn );
        return aReturn;
    }


    void UrlTransformer::parseSmartWithProtocol( css::util::URL& _rURL, const OUString& _rProtocol ) const
    {
        if ( implEnsureTransformer() )
            m_xTransformer->parseSmart( _rURL, _rProtocol );
    }

} // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
