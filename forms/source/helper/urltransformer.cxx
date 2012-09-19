/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#include "urltransformer.hxx"

#include <com/sun/star/util/URLTransformer.hpp>
#include <tools/debug.hxx>
#include <comphelper/processfactory.hxx>

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;

    //====================================================================
    //= UrlTransformer
    //====================================================================
    //--------------------------------------------------------------------
    UrlTransformer::UrlTransformer( const Reference< XMultiServiceFactory >& _rxORB )
        :m_xORB( _rxORB )
        ,m_bTriedToCreateTransformer( false )
    {
        DBG_ASSERT( _rxORB.is(), "UrlTransformer::UrlTransformer: invalid service factory!" );
    }

    //--------------------------------------------------------------------
    bool UrlTransformer::implEnsureTransformer() const
    {
        // create the transformer, if not already attempted to do so
        if ( !m_xTransformer.is() && !m_bTriedToCreateTransformer )
        {
            if ( m_xORB.is() )
            {
                m_xTransformer.set(URLTransformer::create(comphelper::getComponentContext(m_xORB)));
            }

            m_bTriedToCreateTransformer = true;
        }
        return m_xTransformer.is();
    }

    //--------------------------------------------------------------------
    URL UrlTransformer::getStrictURL( const ::rtl::OUString& _rURL ) const
    {
        URL aReturn;
        aReturn.Complete = _rURL;
        if ( implEnsureTransformer() )
            m_xTransformer->parseStrict( aReturn );
        return aReturn;
    }

    //--------------------------------------------------------------------
    URL UrlTransformer::getStrictURLFromAscii( const sal_Char* _pAsciiURL ) const
    {
        return getStrictURL( ::rtl::OUString::createFromAscii( _pAsciiURL ) );
    }

    //--------------------------------------------------------------------
    void UrlTransformer::parseSmartWithAsciiProtocol( ::com::sun::star::util::URL& _rURL, const sal_Char* _pAsciiURL ) const
    {
        if ( implEnsureTransformer() )
            m_xTransformer->parseSmart( _rURL, ::rtl::OUString::createFromAscii( _pAsciiURL ) );
    }

//........................................................................
} // namespace frm
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
