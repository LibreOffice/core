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

#include "unourl.hxx"
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>


namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;


    //= UnoURL

    UnoURL::UnoURL( const OUString& _rCompleteURL, const Reference< XMultiServiceFactory >& _rxORB )
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


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
