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

#include <xmlscript/xmlmod_imexp.hxx>
#include <xmlscript/xml_helper.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star;

namespace xmlscript
{

void
SAL_CALL exportScriptModule(
    Reference< xml::sax::XWriter > const & xOut,
    const ModuleDescriptor& rMod )
        SAL_THROW( (Exception) )
{
    xOut->startDocument();

    OUString aDocTypeStr(
        "<!DOCTYPE script:module PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
        " \"module.dtd\">");
    xOut->unknown( aDocTypeStr );
    xOut->ignorableWhitespace( OUString() );

    OUString aModuleName( XMLNS_SCRIPT_PREFIX ":module" );
    XMLElement* pModElement = new XMLElement( aModuleName );
    Reference< xml::sax::XAttributeList > xAttributes( pModElement );

    pModElement->addAttribute( "xmlns:" XMLNS_SCRIPT_PREFIX, XMLNS_SCRIPT_URI );

    pModElement->addAttribute( XMLNS_SCRIPT_PREFIX ":name", rMod.aName );
    pModElement->addAttribute( XMLNS_SCRIPT_PREFIX ":language", rMod.aLanguage );
    if( !rMod.aModuleType.isEmpty() )
        pModElement->addAttribute( XMLNS_SCRIPT_PREFIX ":moduleType", rMod.aModuleType );

    xOut->ignorableWhitespace( OUString() );
    xOut->startElement( aModuleName, xAttributes );
    xOut->characters( rMod.aCode );
    xOut->endElement( aModuleName );
    xOut->endDocument();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
