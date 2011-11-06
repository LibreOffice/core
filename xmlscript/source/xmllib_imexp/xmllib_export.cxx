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

#include <xmlscript/xmllib_imexp.hxx>
#include <xmlscript/xml_helper.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star;
using namespace rtl;

namespace xmlscript
{

static OUString aTrueStr ( RTL_CONSTASCII_USTRINGPARAM("true") );
static OUString aFalseStr( RTL_CONSTASCII_USTRINGPARAM("false") );

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


        if( rLib.aStorageURL.getLength() )
        {
            pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_XLINK_PREFIX ":href") ),
                                        rLib.aStorageURL );
            pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_XLINK_PREFIX ":type") ),
                                        OUString( RTL_CONSTASCII_USTRINGPARAM("simple") ) );
        }

        pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":link") ),
                                    rLib.bLink ? aTrueStr : aFalseStr );

        if( rLib.bLink )
        {
            pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":readonly") ),
                                        rLib.bReadOnly ? aTrueStr : aFalseStr );
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

    pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":readonly") ),
                                rLib.bReadOnly ? aTrueStr : aFalseStr );

    pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":passwordprotected") ),
                                rLib.bPasswordProtected ? aTrueStr : aFalseStr );

    if( rLib.bPreload )
        pLibElement->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_LIBRARY_PREFIX ":preload") ), aTrueStr );

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

