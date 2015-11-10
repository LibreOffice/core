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

#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
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
#include "editxml.hxx"
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
using namespace cppu;
using namespace xmloff::token;




class SvxXMLTextImportContext : public SvXMLImportContext
{
public:
    SvxXMLTextImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, const uno::Reference< XAttributeList >& xAttrList, const uno::Reference< XText >& xText );
    virtual ~SvxXMLTextImportContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList ) override;

//  SvxXMLXTableImport& getImport() const { return *(SvxXMLXTableImport*)&GetImport(); }

private:
    const uno::Reference< XText > mxText;
};



SvxXMLTextImportContext::SvxXMLTextImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, const uno::Reference< XAttributeList >&, const uno::Reference< XText >& xText )
: SvXMLImportContext( rImport, nPrfx, rLName ), mxText( xText )
{
}

SvxXMLTextImportContext::~SvxXMLTextImportContext()
{
}

SvXMLImportContext *SvxXMLTextImportContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference< XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;
    if(XML_NAMESPACE_OFFICE == nPrefix && IsXMLToken( rLocalName, XML_BODY ) )
    {
        pContext = new SvxXMLTextImportContext( GetImport(), nPrefix, rLocalName, xAttrList, mxText );
    }
    else if( XML_NAMESPACE_OFFICE == nPrefix && IsXMLToken( rLocalName, XML_AUTOMATIC_STYLES ) )
    {
        pContext = new SvXMLStylesContext( GetImport(), nPrefix, rLocalName, xAttrList );
        GetImport().GetTextImport()->SetAutoStyles( static_cast<SvXMLStylesContext*>(pContext) );

    }
    else
    {
        pContext = GetImport().GetTextImport()->CreateTextChildContext( GetImport(), nPrefix, rLocalName, xAttrList );
    }

    if( nullptr == pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}



class SvxXMLXTextImportComponent : public SvXMLImport
{
public:
    SvxXMLXTextImportComponent(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        const uno::Reference< XText > & rText );

    virtual ~SvxXMLXTextImportComponent() throw ();

private:
    const uno::Reference< XText > mxText;
};



SvxXMLXTextImportComponent::SvxXMLXTextImportComponent(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    const uno::Reference< XText > & xText )
:   SvXMLImport(xContext, ""),
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
        { OUString(), 0, css::uno::Type(), 0, 0 }
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
            uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

            uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create( xContext );

            uno::Reference<io::XInputStream> xInputStream = new utl::OInputStreamWrapper( rStream );

/* testcode
            const OUString aURL( "file:///e:/test.xml" );
            SfxMedium aMedium( aURL, StreamMode::READ | STREAM_NOCREATE, sal_True );
            uno::Reference<io::XOutputStream> xOut( new utl::OOutputStreamWrapper( *aMedium.GetOutStream() ) );

            aMedium.GetInStream()->Seek( 0 );
            uno::Reference< io::XActiveDataSource > xSource( aMedium.GetDataSource() );

            if( !xSource.is() )
            {
                OSL_FAIL( "got no data source from medium" );
                break;
            }

            uno::Reference< XInterface > xPipe( Pipe::create(comphelper::getComponentContext(xServiceFactory)), UNO_QUERY );

            // connect pipe's output stream to the data source
            xSource->setOutputStream( uno::Reference< io::XOutputStream >::query( xPipe ) );

            xml::sax::InputSource aParserInput;
            aParserInput.aInputStream.set( xPipe, UNO_QUERY );
            aParserInput.sSystemId = aMedium.GetName();


            if( xSource.is() )
            {
                uno::Reference< io::XActiveDataControl > xSourceControl( xSource, UNO_QUERY );
                xSourceControl->start();
            }

*/

            // uno::Reference< XDocumentHandler > xHandler( new SvxXMLXTextImportComponent( xText ) );
            uno::Reference< XDocumentHandler > xHandler( new SvxXMLXTextImportComponent( xContext, xText ) );

            xParser->setDocumentHandler( xHandler );

            xml::sax::InputSource aParserInput;
            aParserInput.aInputStream = xInputStream;
//          aParserInput.sSystemId = aMedium.GetName();
            xParser->parseStream( aParserInput );
        }
        while(false);
    }
    catch( const uno::Exception& )
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
