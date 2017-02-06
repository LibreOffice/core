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

#include <sal/config.h>

#include <com/sun/star/io/IOException.hpp>

#include "connector.hxx"

using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::connection;


namespace stoc_connector {

    PipeConnection::PipeConnection( const OUString & sConnectionDescription ) :
        m_nStatus( 0 ),
        m_sDescription( sConnectionDescription )
    {
        // make it unique
        m_sDescription += ",uniqueValue=";
        m_sDescription += OUString::number(
            sal::static_int_cast< sal_Int64 >(
                reinterpret_cast< sal_IntPtr >(&m_pipe)) );
    }

    PipeConnection::~PipeConnection()
    {
    }

    sal_Int32 PipeConnection::read( Sequence < sal_Int8 > & aReadBytes , sal_Int32 nBytesToRead )
    {
        if( ! m_nStatus )
        {
            if( aReadBytes.getLength() != nBytesToRead )
            {
                aReadBytes.realloc( nBytesToRead );
            }
            return m_pipe.read( aReadBytes.getArray()  , aReadBytes.getLength() );
        }
        else {
            throw IOException();
        }
    }

    void PipeConnection::write( const Sequence < sal_Int8 > &seq )
    {
        if( ! m_nStatus )
        {
            if( m_pipe.write( seq.getConstArray() , seq.getLength() ) != seq.getLength() )
            {
                throw IOException();
            }
        }
        else {
            throw IOException();
        }
    }

    void PipeConnection::flush( )
    {

    }

    void PipeConnection::close()
    {
        // ensure that close is called only once
        if(1 == osl_atomic_increment( (&m_nStatus) ) )
        {
            m_pipe.close();
        }
    }

    OUString PipeConnection::getDescription()
    {
        return m_sDescription;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
