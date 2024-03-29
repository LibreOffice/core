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

#include <com/sun/star/text/XText.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <svl/itemprop.hxx>
#include <utility>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlstyle.hxx>
#include "editsource.hxx"
#include <editxml.hxx>
#include <editdoc.hxx>
#include <editeng/editeng.hxx>
#include <editeng/unotext.hxx>
#include <editeng/unoprnms.hxx>
#include <editeng/unoipset.hxx>
#include <cassert>
#include <unomodel.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::sax;
using namespace com::sun::star::text;
using namespace cppu;
using namespace xmloff::token;

namespace {

class SvxXMLTextImportContext : public SvXMLImportContext
{
public:
    SvxXMLTextImportContext( SvXMLImport& rImport, uno::Reference< XText > xText );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement,
            const uno::Reference< xml::sax::XFastAttributeList >& xAttrList) override;

private:
    const uno::Reference< XText > mxText;
};

}

SvxXMLTextImportContext::SvxXMLTextImportContext( SvXMLImport& rImport, uno::Reference< XText > xText )
: SvXMLImportContext( rImport ), mxText(std::move( xText ))
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SvxXMLTextImportContext::createFastChildContext(
        sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList >& xAttrList)
{
    SvXMLImportContext* pContext = nullptr;
    if(nElement == XML_ELEMENT(OFFICE, XML_BODY ))
    {
        pContext = new SvxXMLTextImportContext( GetImport(), mxText );
    }
    else if( nElement == XML_ELEMENT(OFFICE, XML_AUTOMATIC_STYLES ) )
    {
        pContext = new SvXMLStylesContext( GetImport() );
        GetImport().GetTextImport()->SetAutoStyles( static_cast<SvXMLStylesContext*>(pContext) );
    }
    else
        pContext = GetImport().GetTextImport()->CreateTextChildContext( GetImport(), nElement, xAttrList );
    return pContext;
}

namespace {

class SvxXMLXTextImportComponent : public SvXMLImport
{
public:
    SvxXMLXTextImportComponent(
        const css::uno::Reference< css::uno::XComponentContext >& rContext,
        uno::Reference< XText > xText );

    virtual SvXMLImportContext* CreateFastContext(sal_Int32 nElement,
        const ::css::uno::Reference< ::css::xml::sax::XFastAttributeList >& xAttrList ) override;

private:
    const uno::Reference< XText > mxText;
};

}

SvXMLImportContext *SvxXMLXTextImportComponent::CreateFastContext(
        sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/)
{
    SvXMLImportContext* pContext = nullptr;

    if(nElement == XML_ELEMENT(OFFICE, XML_DOCUMENT_CONTENT ) )
    {
        pContext = new SvxXMLTextImportContext( *this, mxText );
    }

    return pContext;
}

SvxXMLXTextImportComponent::SvxXMLXTextImportComponent(
    const css::uno::Reference< css::uno::XComponentContext >& xContext,
    uno::Reference< XText > xText )
:   SvXMLImport(xContext, ""),
    mxText(std::move( xText ))
{
    GetTextImport()->SetCursor( mxText->createTextCursor() );
    SvXMLImport::setTargetDocument(new SvxSimpleUnoModel);
}

EditPaM SvxReadXML( EditEngine& rEditEngine, SvStream& rStream, const ESelection& rSel )
{
    SvxEditEngineSource aEditSource( &rEditEngine );

    static const SfxItemPropertyMapEntry SvxXMLTextImportComponentPropertyMap[] =
    {
        SVX_UNOEDIT_CHAR_PROPERTIES,
        SVX_UNOEDIT_FONT_PROPERTIES,
//      bullets & numbering props, tdf#128046
        { UNO_NAME_NUMBERING_RULES,        EE_PARA_NUMBULLET,  cppu::UnoType<css::container::XIndexReplace>::get(), 0, 0 },
        { UNO_NAME_NUMBERING,              EE_PARA_BULLETSTATE,cppu::UnoType<bool>::get(), 0, 0 },
        { UNO_NAME_NUMBERING_LEVEL,        EE_PARA_OUTLLEVEL,  ::cppu::UnoType<sal_Int16>::get(), 0, 0 },
        SVX_UNOEDIT_PARA_PROPERTIES,
    };
    static SvxItemPropertySet aSvxXMLTextImportComponentPropertySet( SvxXMLTextImportComponentPropertyMap, EditEngine::GetGlobalItemPool() );

    assert(!rSel.HasRange());
    //get the initial para count before paste
    sal_uInt32 initialParaCount = rEditEngine.GetEditDoc().Count();
    //insert para breaks before inserting the copied text
    rEditEngine.InsertParaBreak( rEditEngine.CreateSelection( rSel ).Max() );
    rEditEngine.InsertParaBreak( rEditEngine.CreateSelection( rSel ).Max() );

    // Init return PaM.
    EditPaM aPaM( rEditEngine.CreateSelection( rSel ).Max());

    ESelection aSel(rSel.nStartPara+1, 0, rSel.nEndPara+1, 0);
    uno::Reference<text::XText > xParent;
    rtl::Reference<SvxUnoText> pUnoText = new SvxUnoText( &aEditSource, &aSvxXMLTextImportComponentPropertySet, xParent );
    pUnoText->SetSelection( aSel );

    try
    {
        do
        {
            uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

            uno::Reference<io::XInputStream> xInputStream = new utl::OInputStreamWrapper( rStream );

/* testcode
            static constexpr OUStringLiteral aURL( u"file:///e:/test.xml" );
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
            rtl::Reference< SvxXMLXTextImportComponent > xImport( new SvxXMLXTextImportComponent( xContext, pUnoText ) );

            xml::sax::InputSource aParserInput;
            aParserInput.aInputStream = xInputStream;
//          aParserInput.sSystemId = aMedium.GetName();
            xImport->parseStream( aParserInput );
        }
        while(false);

        //remove the extra para breaks
        EditDoc& pDoc = rEditEngine.GetEditDoc();
        rEditEngine.ParaAttribsToCharAttribs( pDoc.GetObject( rSel.nEndPara ) );
        rEditEngine.ConnectParagraphs( pDoc.GetObject( rSel.nEndPara ),
            pDoc.GetObject( rSel.nEndPara + 1 ), true );
        rEditEngine.ParaAttribsToCharAttribs( pDoc.GetObject( pDoc.Count() - initialParaCount + aSel.nEndPara - 2 ) );
        rEditEngine.ConnectParagraphs( pDoc.GetObject( pDoc.Count() - initialParaCount + aSel.nEndPara - 2 ),
            pDoc.GetObject( pDoc.Count() - initialParaCount + aSel.nEndPara -1 ), true );

        // The final join is to be returned.
        aPaM = rEditEngine.ConnectParagraphs( pDoc.GetObject( pDoc.Count() - initialParaCount + aSel.nEndPara - 2 ),
            pDoc.GetObject( pDoc.Count() - initialParaCount + aSel.nEndPara -1 ), true );
    }
    catch( const uno::Exception& )
    {
    }

    return aPaM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
