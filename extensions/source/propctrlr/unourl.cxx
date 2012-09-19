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

#include "unourl.hxx"
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <comphelper/processfactory.hxx>

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;

    //====================================================================
    //= UnoURL
    //====================================================================
    UnoURL::UnoURL( const ::rtl::OUString& _rCompleteURL, const Reference< XMultiServiceFactory >& _rxORB )
    {
        m_aURL.Complete = _rCompleteURL;

        OSL_ENSURE( _rxORB.is(), "UnoURL::UnoURL: invalid ORB!" );
        Reference< XURLTransformer > xTransform;
        try
        {
            if ( _rxORB.is() )
            {
                xTransform.set( URLTransformer::create(comphelper::getComponentContext(_rxORB)) );
                OSL_ENSURE( xTransform.is(), "UnoURL::UnoURL: could not create an URL transformer!" );
                if ( xTransform.is() )
                    xTransform->parseStrict( m_aURL );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "UnoURL::UnoURL: caught an exception!" );
        }
    }

//........................................................................
}   // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
