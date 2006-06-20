/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmllib_export.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:13:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

