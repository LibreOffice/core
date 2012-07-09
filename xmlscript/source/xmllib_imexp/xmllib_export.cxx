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


#include <xmlscript/xmllib_imexp.hxx>
#include <xmlscript/xml_helper.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star;

using ::rtl::OUString;

namespace xmlscript
{

const char aTrueStr[] = "true";
const char aFalseStr[] = "false";

//##################################################################################################


//==================================================================================================

void
SAL_CALL exportLibraryContainer(
    Reference< xml::sax::XExtendedDocumentHandler > const & xOut,
    const LibDescriptorArray* pLibArray )
        SAL_THROW( (Exception) )
{
    xOut->startDocument();

    OUString aDocTypeStr( RTL_CONSTASCII_USTRINGPARAM(
        "<!DOCTYPE library:libraries PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
        " \"libraries.dtd\">" ) );
    xOut->unknown( aDocTypeStr );
    xOut->ignorableWhitespace( OUString() );


    OUString aLibrariesName( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":libraries") );
    XMLElement* pLibsElement = new XMLElement( aLibrariesName );
    Reference< xml::sax::XAttributeList > xAttributes( pLibsElement );

    pLibsElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM("xmlns:" XMLNS_LIBRARY_PREFIX) ),
                                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_URI) ) );
    pLibsElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM("xmlns:" XMLNS_XLINK_PREFIX) ),
                                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_XLINK_URI) ) );


    xOut->ignorableWhitespace( OUString() );
    xOut->startElement( aLibrariesName, xAttributes );

    rtl::OUString sTrueStr(aTrueStr);
    rtl::OUString sFalseStr(aFalseStr);

    int nLibCount = pLibArray->mnLibCount;
    for( sal_Int32 i = 0 ; i < nLibCount ; i++ )
    {
        LibDescriptor& rLib = pLibArray->mpLibs[i];

        OUString aLibraryName( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":library") );
        XMLElement* pLibElement = new XMLElement( aLibraryName );
        Reference< xml::sax::XAttributeList > xLibElementAttribs;
        xLibElementAttribs = static_cast< xml::sax::XAttributeList* >( pLibElement );

        pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":name") ),
                                    rLib.aName );


        if( !rLib.aStorageURL.isEmpty() )
        {
            pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_XLINK_PREFIX ":href") ),
                                        rLib.aStorageURL );
            pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_XLINK_PREFIX ":type") ),
                                        OUString( RTL_CONSTASCII_USTRINGPARAM("simple") ) );
        }

        pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":link") ),
                                    rLib.bLink ? sTrueStr : sFalseStr );

        if( rLib.bLink )
        {
            pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":readonly") ),
                                        rLib.bReadOnly ? sTrueStr : sFalseStr );
        }

        pLibElement->dump( xOut.get() );
    }

    xOut->ignorableWhitespace( OUString() );
    xOut->endElement( aLibrariesName );

    xOut->endDocument();
}

//==================================================================================================

void
SAL_CALL exportLibrary(
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XExtendedDocumentHandler > const & xOut,
    const LibDescriptor& rLib )
        SAL_THROW( (::com::sun::star::uno::Exception) )
{
    xOut->startDocument();

    OUString aDocTypeStr( RTL_CONSTASCII_USTRINGPARAM(
        "<!DOCTYPE library:library PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
        " \"library.dtd\">" ) );
    xOut->unknown( aDocTypeStr );
    xOut->ignorableWhitespace( OUString() );


    OUString aLibraryName( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":library") );
    XMLElement* pLibElement = new XMLElement( aLibraryName );
    Reference< xml::sax::XAttributeList > xAttributes( pLibElement );

    pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM("xmlns:" XMLNS_LIBRARY_PREFIX) ),
                                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_URI) ) );

    pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":name") ),
                                rLib.aName );

    rtl::OUString sTrueStr(aTrueStr);
    rtl::OUString sFalseStr(aFalseStr);

    pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":readonly") ),
                                rLib.bReadOnly ? sTrueStr : sFalseStr );

    pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":passwordprotected") ),
                                rLib.bPasswordProtected ? sTrueStr : sFalseStr );

    if( rLib.bPreload )
        pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":preload") ), sTrueStr );

    sal_Int32 nElementCount = rLib.aElementNames.getLength();
    if( nElementCount )
    {
        const OUString* pElementNames = rLib.aElementNames.getConstArray();
        for( sal_Int32 i = 0 ; i < nElementCount ; i++ )
        {
            XMLElement* pElement = new XMLElement( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":element" ) ) );
            Reference< xml::sax::XAttributeList > xElementAttribs;
            xElementAttribs = static_cast< xml::sax::XAttributeList* >( pElement );

            pElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":name") ),
                                        pElementNames[i] );

            pLibElement->addSubElement( pElement );
        }
    }

    pLibElement->dump( xOut.get() );

    xOut->endDocument();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
