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

constexpr OUString aTrueStr = u"true"_ustr;
constexpr OUString aFalseStr = u"false"_ustr;

void
exportLibraryContainer(
    Reference< xml::sax::XWriter > const & xOut,
    const LibDescriptorArray* pLibArray )
{
    xOut->startDocument();

    xOut->unknown(
        u"<!DOCTYPE library:libraries PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
        " \"libraries.dtd\">"_ustr );
    xOut->ignorableWhitespace( OUString() );

    OUString aLibrariesName( u"" XMLNS_LIBRARY_PREFIX ":libraries"_ustr );
    rtl::Reference<XMLElement> pLibsElement = new XMLElement( aLibrariesName );

    pLibsElement->addAttribute( u"xmlns:" XMLNS_LIBRARY_PREFIX ""_ustr, XMLNS_LIBRARY_URI );
    pLibsElement->addAttribute( u"xmlns:" XMLNS_XLINK_PREFIX ""_ustr, XMLNS_XLINK_URI );

    xOut->ignorableWhitespace( OUString() );
    xOut->startElement( aLibrariesName, pLibsElement );

    OUString sTrueStr(aTrueStr);
    OUString sFalseStr(aFalseStr);

    int nLibCount = pLibArray->mnLibCount;
    for( sal_Int32 i = 0 ; i < nLibCount ; i++ )
    {
        LibDescriptor& rLib = pLibArray->mpLibs[i];

        rtl::Reference<XMLElement> pLibElement(new XMLElement( u"" XMLNS_LIBRARY_PREFIX ":library"_ustr ));

        pLibElement->addAttribute( u"" XMLNS_LIBRARY_PREFIX ":name"_ustr, rLib.aName );

        if( !rLib.aStorageURL.isEmpty() )
        {
            pLibElement->addAttribute( u"" XMLNS_XLINK_PREFIX ":href"_ustr, rLib.aStorageURL );
            pLibElement->addAttribute( u"" XMLNS_XLINK_PREFIX ":type"_ustr, u"simple"_ustr );
        }

        pLibElement->addAttribute(  u"" XMLNS_LIBRARY_PREFIX ":link"_ustr, rLib.bLink ? sTrueStr : sFalseStr );

        if( rLib.bLink )
        {
            pLibElement->addAttribute( u"" XMLNS_LIBRARY_PREFIX ":readonly"_ustr, rLib.bReadOnly ? sTrueStr : sFalseStr );
        }

        pLibElement->dump( xOut );
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
        u"<!DOCTYPE library:library PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
        " \"library.dtd\">"_ustr );
    xOut->ignorableWhitespace( OUString() );

    rtl::Reference<XMLElement> pLibElement = new XMLElement( u"" XMLNS_LIBRARY_PREFIX ":library"_ustr );

    pLibElement->addAttribute( u"xmlns:" XMLNS_LIBRARY_PREFIX ""_ustr, XMLNS_LIBRARY_URI );

    pLibElement->addAttribute( u"" XMLNS_LIBRARY_PREFIX ":name"_ustr, rLib.aName );

    OUString sTrueStr(aTrueStr);
    OUString sFalseStr(aFalseStr);

    pLibElement->addAttribute( u"" XMLNS_LIBRARY_PREFIX ":readonly"_ustr, rLib.bReadOnly ? sTrueStr : sFalseStr );

    pLibElement->addAttribute( u"" XMLNS_LIBRARY_PREFIX ":passwordprotected"_ustr, rLib.bPasswordProtected ? sTrueStr : sFalseStr );

    if( rLib.bPreload )
        pLibElement->addAttribute( u"" XMLNS_LIBRARY_PREFIX ":preload"_ustr, sTrueStr );

    for( const auto& rElementName : rLib.aElementNames )
    {
        rtl::Reference<XMLElement> pElement(new XMLElement( u"" XMLNS_LIBRARY_PREFIX ":element"_ustr ));

        pElement->addAttribute( u"" XMLNS_LIBRARY_PREFIX ":name"_ustr,
                                    rElementName );

        pLibElement->addSubElement( pElement );
    }

    pLibElement->dump( xOut );

    xOut->endDocument();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
