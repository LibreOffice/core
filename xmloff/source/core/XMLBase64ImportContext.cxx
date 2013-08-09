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

#include <sax/tools/converter.hxx>

#include <xmloff/xmlimp.hxx>
#include <xmloff/XMLBase64ImportContext.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::io;

TYPEINIT1( XMLBase64ImportContext, SvXMLImportContext );

XMLBase64ImportContext::XMLBase64ImportContext(
        SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList >&,
        const Reference< XOutputStream >& rOut ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    xOut( rOut )
{
}

XMLBase64ImportContext::~XMLBase64ImportContext()
{
}

void XMLBase64ImportContext::EndElement()
{
    xOut->closeOutput();
}

void XMLBase64ImportContext::Characters( const OUString& rChars )
{
    OUString sTrimmedChars( rChars. trim() );
    if( !sTrimmedChars.isEmpty() )
    {
        OUString sChars;
        if( !sBase64CharsLeft.isEmpty() )
        {
            sChars = sBase64CharsLeft;
            sChars += sTrimmedChars;
            sBase64CharsLeft = OUString();
        }
        else
        {
            sChars = sTrimmedChars;
        }
        Sequence< sal_Int8 > aBuffer( (sChars.getLength() / 4) * 3 );
		sal_Int32 const nCharsDecoded =
				::sax::Converter::decodeBase64SomeChars( aBuffer, sChars );
        xOut->writeBytes( aBuffer );
        if( nCharsDecoded != sChars.getLength() )
            sBase64CharsLeft = sChars.copy( nCharsDecoded );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
