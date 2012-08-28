/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/text/XText.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <rtl/ustrbuf.hxx>
#include <sot/storage.hxx>
#include <svl/itemprop.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlmetae.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlstyle.hxx>
#include "editsource.hxx"
#include <editeng/editeng.hxx>
#include <editeng/unotext.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unoipset.hxx>

using namespace com::sun::star;
using namespace com::sun::star::document;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::sax;
using namespace com::sun::star::text;
using namespace ::rtl;
using namespace cppu;
using namespace xmloff::token;


///////////////////////////////////////////////////////////////////////

class SvxXMLTextImportContext : public SvXMLImportContext
{
public:
    SvxXMLTextImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, const uno::Reference< XAttributeList >& xAttrList, const uno::Reference< XText >& xText );
    virtual ~SvxXMLTextImportContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList );

//  SvxXMLXTableImport& getImport() const { return *(SvxXMLXTableImport*)&GetImport(); }

private:
    const uno::Reference< XText > mxText;
};

///////////////////////////////////////////////////////////////////////

SvxXMLTextImportContext::SvxXMLTextImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, const uno::Reference< XAttributeList >&, const uno::Reference< XText >& xText )
: SvXMLImportContext( rImport, nPrfx, rLName ), mxText( xText )
{
}

SvxXMLTextImportContext::~SvxXMLTextImportContext()
{
}

SvXMLImportContext *SvxXMLTextImportContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = NULL;
    if(XML_NAMESPACE_OFFICE == nPrefix && IsXMLToken( rLocalName, XML_BODY ) )
    {
        pContext = new SvxXMLTextImportContext( GetImport(), nPrefix, rLocalName, xAttrList, mxText );
    }
    else if( XML_NAMESPACE_OFFICE == nPrefix && IsXMLToken( rLocalName, XML_AUTOMATIC_STYLES ) )
    {
        pContext = new SvXMLStylesContext( GetImport(), nPrefix, rLocalName, xAttrList );
        GetImport().GetTextImport()->SetAutoStyles( (SvXMLStylesContext*)pContext );

    }
    else
    {
        pContext = GetImport().GetTextImport()->CreateTextChildContext( GetImport(), nPrefix, rLocalName, xAttrList );
    }

    if( NULL == pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

///////////////////////////////////////////////////////////////////////

class SvxXMLXTextImportComponent : public SvXMLImport
{
public:
    SvxXMLXTextImportComponent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        const uno::Reference< XText > & xText );

    virtual ~SvxXMLXTextImportComponent() throw ();

    static sal_Bool load( const rtl::OUString& rUrl, const com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >& xTable ) throw();
protected:
    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList );

private:
    const uno::Reference< XText > mxText;
};

// --------------------------------------------------------------------

SvxXMLXTextImportComponent::SvxXMLXTextImportComponent(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const uno::Reference< XText > & xText )
:   SvXMLImport(xServiceFactory),
    mxText( xText )
{
    GetTextImport()->SetCursor( mxText->createTextCursor() );
}

SvxXMLXTextImportComponent::~SvxXMLXTextImportComponent() throw ()
{
}

void SvxReadXML( EditEngine& rEditEngine, SvStream& rStream, const ESelection& rSel )
{
    SvxEditEngineSource aEditSource( &rEditEngine );

    static const SfxItemPropertyMapEntry SvxXMLTextImportComponentPropertyMap[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
//      SVX_UNOEDIT_OUTLINER_PROPERTIES,
        SVX_UNOEDIT_PARA_PROPERTIES,
        {0,0,0,0,0,0}
    };
    static SvxItemPropertySet aSvxXMLTextImportComponentPropertySet( SvxXMLTextImportComponentPropertyMap, EditEngine::GetGlobalItemPool() );

    uno::Reference<text::XText > xParent;
    SvxUnoText* pUnoText = new SvxUnoText( &aEditSource, &aSvxXMLTextImportComponentPropertySet, xParent );
    pUnoText->SetSelection( rSel );
    uno::Reference<text::XText > xText( pUnoText );

    try
    {
        do
        {
            uno::Reference<lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );
            if( !xServiceFactory.is() )
            {
                OSL_FAIL( "SvxXMLXTableImport::load: got no service manager" );
                break;
            }

            uno::Reference< xml::sax::XParser > xParser( xServiceFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser" ) ) ), uno::UNO_QUERY );
            if( !xParser.is() )
            {
                OSL_FAIL( "com.sun.star.xml.sax.Parser service missing" );
                break;
            }

            uno::Reference<io::XInputStream> xInputStream = new utl::OInputStreamWrapper( rStream );

/* testcode
            const OUString aURL( RTL_CONSTASCII_USTRINGPARAM( "file:///e:/test.xml" ) );
            SfxMedium aMedium( aURL, STREAM_READ | STREAM_NOCREATE, sal_True );
            aMedium.IsRemote();
            uno::Reference<io::XOutputStream> xOut( new utl::OOutputStreamWrapper( *aMedium.GetOutStream() ) );

            aMedium.GetInStream()->Seek( 0 );
            uno::Reference< io::XActiveDataSource > xSource( aMedium.GetDataSource() );

            if( !xSource.is() )
            {
                OSL_FAIL( "got no data source from medium" );
                break;
            }

            uno::Reference< XInterface > xPipe( Pipe::create(comphelper::ComponentContext(xServiceFactory).getUNOContext()), UNO_QUERY );

            // connect pipe's output stream to the data source
            xSource->setOutputStream( uno::Reference< io::XOutputStream >::query( xPipe ) );

            xml::sax::InputSource aParserInput;
            aParserInput.aInputStream = uno::Reference< io::XInputStream >::query( xPipe );
            aParserInput.sSystemId = aMedium.GetName();


            if( xSource.is() )
            {
                uno::Reference< io::XActiveDataControl > xSourceControl( xSource, UNO_QUERY );
                xSourceControl->start();
            }

*/

            // uno::Reference< XDocumentHandler > xHandler( new SvxXMLXTextImportComponent( xText ) );
            uno::Reference< XDocumentHandler > xHandler( new SvxXMLXTextImportComponent( xServiceFactory, xText ) );

            xParser->setDocumentHandler( xHandler );

            xml::sax::InputSource aParserInput;
            aParserInput.aInputStream = xInputStream;
//          aParserInput.sSystemId = aMedium.GetName();
            xParser->parseStream( aParserInput );
        }
        while(0);
    }
    catch( const uno::Exception& )
    {
    }
}

SvXMLImportContext *SvxXMLXTextImportComponent::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext;
    if(XML_NAMESPACE_OFFICE == nPrefix && ( IsXMLToken( rLocalName, XML_DOCUMENT ) || IsXMLToken( rLocalName, XML_DOCUMENT_CONTENT ) ) )
    {
         pContext = new SvxXMLTextImportContext(*this, nPrefix, rLocalName, xAttrList, mxText );
    }
    else
    {
        pContext = SvXMLImport::CreateContext(nPrefix, rLocalName, xAttrList);
    }
    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
