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

#include <com/sun/star/io/XOutputStream.hpp>

#include <comphelper/base64.hxx>

#include <xmloff/xmlimp.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;


XMLBase64ImportContext::XMLBase64ImportContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList >&,
        const Reference< XOutputStream >& rOut ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xOut( rOut )
{
}

XMLBase64ImportContext::XMLBase64ImportContext(
        SvXMLImport& rImport,
        const Reference< XOutputStream >& rOut ) :
    SvXMLImportContext( rImport ),
    xOut( rOut )
{
}

XMLBase64ImportContext::~XMLBase64ImportContext()
{
}

void XMLBase64ImportContext::EndElement()
{
    endFastElement(0);
}

void XMLBase64ImportContext::endFastElement(sal_Int32 )
{
    OUString sChars = maCharBuffer.makeStringAndClear().trim();
    if( !sChars.isEmpty() )
    {
        Sequence< sal_Int8 > aBuffer( (sChars.getLength() / 4) * 3 );
        ::comphelper::Base64::decodeSomeChars( aBuffer, sChars );
        xOut->writeBytes( aBuffer );
    }
    xOut->closeOutput();
}

void XMLBase64ImportContext::Characters( const OUString& rChars )
{
    characters(rChars);
}

void XMLBase64ImportContext::characters( const OUString& rChars )
{
    maCharBuffer.append(rChars);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
