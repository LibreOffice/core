/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlscript.hxx"

#include <xmlscript/xmlmod_imexp.hxx>
#include <xmlscript/xml_helper.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star;
using namespace rtl;

namespace xmlscript
{

//==================================================================================================

void
SAL_CALL exportScriptModule(
    Reference< xml::sax::XExtendedDocumentHandler > const & xOut,
    const ModuleDescriptor& rMod )
        SAL_THROW( (Exception) )
{
    xOut->startDocument();

    OUString aDocTypeStr( RTL_CONSTASCII_USTRINGPARAM(
        "<!DOCTYPE script:module PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
        " \"module.dtd\">" ) );
    xOut->unknown( aDocTypeStr );
    xOut->ignorableWhitespace( OUString() );

    OUString aModuleName( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":module") );
    XMLElement* pModElement = new XMLElement( aModuleName );
    Reference< xml::sax::XAttributeList > xAttributes( pModElement );

    pModElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM("xmlns:" XMLNS_SCRIPT_PREFIX) ),
                                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_URI) ) );

    pModElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":name") ),
                                rMod.aName );
    pModElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":language") ),
                                rMod.aLanguage );
    if( rMod.aModuleType.getLength()>0 )
        pModElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":moduleType") ),
                                    rMod.aModuleType );

    xOut->ignorableWhitespace( OUString() );
    xOut->startElement( aModuleName, xAttributes );
    xOut->characters( rMod.aCode );
    xOut->endElement( aModuleName );
    xOut->endDocument();
}

}

