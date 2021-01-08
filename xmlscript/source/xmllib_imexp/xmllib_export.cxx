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

#include <rtl/ref.hxx>
#include <xmlscript/xmllib_imexp.hxx>
#include <xmlscript/xml_helper.hxx>
#include <xmlscript/xmlns.h>
#include <com/sun/star/xml/sax/XWriter.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star;

namespace xmlscript
{

constexpr OUStringLiteral aTrueStr = u"true";
constexpr OUStringLiteral aFalseStr = u"false";

void
exportLibraryContainer(
    Reference< xml::sax::XWriter > const & xOut,
    const LibDescriptorArray* pLibArray )
{
    xOut->startDocument();

    xOut->unknown(
        "<!DOCTYPE library:libraries PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
        " \"libraries.dtd\">" );
    xOut->ignorableWhitespace( OUString() );

    OUString aLibrariesName( XMLNS_LIBRARY_PREFIX ":libraries" );
    XMLElement* pLibsElement = new XMLElement( aLibrariesName );
    Reference< xml::sax::XAttributeList > xAttributes( pLibsElement );

    pLibsElement->addAttribute( "xmlns:" XMLNS_LIBRARY_PREFIX, XMLNS_LIBRARY_URI );
    pLibsElement->addAttribute( "xmlns:" XMLNS_XLINK_PREFIX, XMLNS_XLINK_URI );

    xOut->ignorableWhitespace( OUString() );
    xOut->startElement( aLibrariesName, xAttributes );

    OUString sTrueStr(aTrueStr);
    OUString sFalseStr(aFalseStr);

    int nLibCount = pLibArray->mnLibCount;
    for( sal_Int32 i = 0 ; i < nLibCount ; i++ )
    {
        LibDescriptor& rLib = pLibArray->mpLibs[i];

        rtl::Reference<XMLElement> pLibElement(new XMLElement( XMLNS_LIBRARY_PREFIX ":library" ));

        pLibElement->addAttribute( XMLNS_LIBRARY_PREFIX ":name", rLib.aName );

        if( !rLib.aStorageURL.isEmpty() )
        {
            pLibElement->addAttribute( XMLNS_XLINK_PREFIX ":href", rLib.aStorageURL );
            pLibElement->addAttribute( XMLNS_XLINK_PREFIX ":type", "simple" );
        }

        pLibElement->addAttribute(  XMLNS_LIBRARY_PREFIX ":link", rLib.bLink ? sTrueStr : sFalseStr );

        if( rLib.bLink )
        {
            pLibElement->addAttribute( XMLNS_LIBRARY_PREFIX ":readonly", rLib.bReadOnly ? sTrueStr : sFalseStr );
        }

        pLibElement->dump( xOut.get() );
    }

    xOut->ignorableWhitespace( OUString() );
    xOut->endElement( aLibrariesName );

    xOut->endDocument();
}

void
exportLibrary(
    css::uno::Reference< css::xml::sax::XWriter > const & xOut,
    const LibDescriptor& rLib )
{
    xOut->startDocument();

    xOut->unknown(
        "<!DOCTYPE library:library PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
        " \"library.dtd\">" );
    xOut->ignorableWhitespace( OUString() );

    rtl::Reference<XMLElement> pLibElement = new XMLElement( XMLNS_LIBRARY_PREFIX ":library" );

    pLibElement->addAttribute( "xmlns:" XMLNS_LIBRARY_PREFIX, XMLNS_LIBRARY_URI );

    pLibElement->addAttribute( XMLNS_LIBRARY_PREFIX ":name", rLib.aName );

    OUString sTrueStr(aTrueStr);
    OUString sFalseStr(aFalseStr);

    pLibElement->addAttribute( XMLNS_LIBRARY_PREFIX ":readonly", rLib.bReadOnly ? sTrueStr : sFalseStr );

    pLibElement->addAttribute( XMLNS_LIBRARY_PREFIX ":passwordprotected", rLib.bPasswordProtected ? sTrueStr : sFalseStr );

    if( rLib.bPreload )
        pLibElement->addAttribute( XMLNS_LIBRARY_PREFIX ":preload", sTrueStr );

    for( const auto& rElementName : rLib.aElementNames )
    {
        rtl::Reference<XMLElement> pElement(new XMLElement( XMLNS_LIBRARY_PREFIX ":element" ));

        pElement->addAttribute( XMLNS_LIBRARY_PREFIX ":name",
                                    rElementName );

        pLibElement->addSubElement( pElement.get() );
    }

    pLibElement->dump( xOut.get() );

    xOut->endDocument();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
