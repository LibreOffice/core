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


#include <rtl/ustrbuf.hxx>

#include <com/sun/star/io/XInputStream.hpp>

#include <comphelper/base64.hxx>

#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <XMLBase64Export.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;

#define INPUT_BUFFER_SIZE 54
#define OUTPUT_BUFFER_SIZE 72

XMLBase64Export::XMLBase64Export( SvXMLExport& rExp ) :
    rExport( rExp ){
}

bool XMLBase64Export::exportXML( const Reference < XInputStream> & rIn )
{
    bool bRet = true;
    try
    {
        Sequence < sal_Int8 > aInBuff( INPUT_BUFFER_SIZE );
        OUStringBuffer aOutBuff( OUTPUT_BUFFER_SIZE );
        sal_Int32 nRead;
        do
        {
            nRead = rIn->readBytes( aInBuff, INPUT_BUFFER_SIZE );
            if( nRead > 0 )
            {
                ::comphelper::Base64::encode( aOutBuff, aInBuff );
                GetExport().Characters( aOutBuff.makeStringAndClear() );
                if( nRead == INPUT_BUFFER_SIZE )
                    GetExport().IgnorableWhitespace();
            }
        }
        while( nRead == INPUT_BUFFER_SIZE );
    }
    catch( ... )
    {
        bRet = false;
    }

    return bRet;
}

bool XMLBase64Export::exportElement(
            const Reference < XInputStream > & rIn,
            enum ::xmloff::token::XMLTokenEnum eName )
{
    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_OFFICE, eName, true, true );
    return exportXML( rIn );
}

bool XMLBase64Export::exportOfficeBinaryDataElement(
            const Reference < XInputStream > & rIn )
{
    return exportElement( rIn, ::xmloff::token::XML_BINARY_DATA );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
