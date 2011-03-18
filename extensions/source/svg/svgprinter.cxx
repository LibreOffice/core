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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#define _RMPRINTER_EXT

#include "svgprinter.hxx"
#include "svgaction.hxx"
#include <uno/mapping.hxx>
#include <vcl/print.hxx>
#include <vcl/virdev.hxx>

static const char aXMLElemSVG[] = "svg";
static const char aXMLElemMetaSVG[] = "staroffice:svgElementMeta";
static const char aXMLElemDesc[] = "desc";
static const char aXMLElemMeta[] = "metadata";
static const char aXMLElemRect[] = "rect";

static const char aXMLAttrMetaSVGOuter[] = "isOuterElement";
static const char aXMLAttrMetaSVGPage[] = "isPageElement";
static const char aXMLAttrViewBox[] = "viewBox";
static const char aXMLAttrX[] = "x";
static const char aXMLAttrY[] = "y";
static const char aXMLAttrWidth[] = "width";
static const char aXMLAttrHeight[] = "height";

// ----------------
// - SVGMtfExport -
// ----------------

class SVGPrinterExport : public SvXMLExport
{
private:

    Printer                 maPrinter;
    VirtualDevice*          mpVDev;
    SvXMLElementExport*     mpOuterElement;
    sal_uInt32              mnPage;

                            SVGPrinterExport();

    SvXMLElementExport*     ImplCreateSVGElement( const JobSetup& rSetup, Size& rOutputSize );
    void                    ImplWriteMetaAttr( sal_Bool bOuter, sal_Bool bPage );

protected:

    virtual void            _ExportMeta() {}
    virtual void            _ExportStyles( sal_Bool /*bUsed*/ ) {}
    virtual void            _ExportAutoStyles() {}
    virtual void            _ExportContent() {}
    virtual void            _ExportMasterStyles() {}
    virtual sal_uInt32      exportDoc( enum ::xmloff::token::XMLTokenEnum eClass = ::xmloff::token::XML_TOKEN_INVALID ) { (void)eClass; return 0; }

public:

    // #110680#
    SVGPrinterExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        const REF( NMSP_SAX::XDocumentHandler )& rxHandler,
        const JobSetup& rSetup,
        const NMSP_RTL::OUString& rJobName,
        sal_uInt32 nCopies,
        sal_Bool bCollate );

    virtual                 ~SVGPrinterExport();

    virtual void            writePage( const JobSetup& rJobSetup, const GDIMetaFile& rMtf );
};

// -----------------------------------------------------------------------------

// #110680#
SVGPrinterExport::SVGPrinterExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const REF( NMSP_SAX::XDocumentHandler )& rxHandler,
    const JobSetup& rSetup,
    const NMSP_RTL::OUString& rJobName,
    sal_uInt32 /*nCopies*/,
    sal_Bool /*bCollate*/ )
:   SvXMLExport( xServiceFactory, NMSP_RTL::OUString(), rxHandler ),
    mpVDev( NULL ),
    mnPage( 0 )
{
    maPrinter.SetJobSetup( rSetup );

    GetDocHandler()->startDocument();

    REF( NMSP_SAX::XExtendedDocumentHandler ) xExtDocHandler( GetDocHandler(), NMSP_UNO::UNO_QUERY );

    if( xExtDocHandler.is() )
    {
        NMSP_RTL::OUString          aString;
        const NMSP_RTL::OUString    aLineFeed( NMSP_RTL::OUString::valueOf( (sal_Unicode) 0x0a ) );

        // intro
        xExtDocHandler->unknown( ( aString = SVG_DTD_STRING ) += aLineFeed );
        xExtDocHandler->unknown( ( aString = B2UCONST( "<!ELEMENT metadata (#PCDATA | staroffice:svgElementMeta)*> " ) += aLineFeed ) );
        xExtDocHandler->unknown( ( aString = B2UCONST( "<!ELEMENT staroffice:svgElementMeta ANY> " ) += aLineFeed ) );
        xExtDocHandler->unknown( ( aString = B2UCONST( "<!ATTLIST staroffice:svgElementMeta " ) += aLineFeed ) );

        // ATTLIST
        xExtDocHandler->unknown( ( aString = B2UCONST( "isOuterElement (true | false) \"false\" " ) += aLineFeed ) );
        xExtDocHandler->unknown( ( aString = B2UCONST( "isPageElement (true | false) \"true\"" ) += aLineFeed ) );

        // end of intro
        xExtDocHandler->unknown( ( aString = B2UCONST( ">" ) += aLineFeed ) );
        xExtDocHandler->unknown( ( aString = B2UCONST( "]>" ) ) );
    }

    // create outer element
    Size aOutputSize;

    mpOuterElement = ImplCreateSVGElement( rSetup, aOutputSize );

    // write description
    SvXMLElementExport* pDescElem = new SvXMLElementExport( *this, XML_NAMESPACE_NONE, aXMLElemDesc, sal_True, sal_True );
    NMSP_RTL::OUString  aDesc( B2UCONST( "document name: " ) );

    GetDocHandler()->characters( aDesc += rJobName );
    delete pDescElem;

    // write meta attributes
    ImplWriteMetaAttr( sal_True, sal_False );
}

// -----------------------------------------------------------------------------

SVGPrinterExport::~SVGPrinterExport()
{
    delete mpOuterElement;
    GetDocHandler()->endDocument();
    delete mpVDev;
}

// -----------------------------------------------------------------------------

SvXMLElementExport* SVGPrinterExport::ImplCreateSVGElement( const JobSetup& rSetup, Size& rOutputSize )
{
    NMSP_RTL::OUString aAttr;

    delete mpVDev;
    mpVDev = new VirtualDevice;
    mpVDev->EnableOutput( sal_False );
    mpVDev->SetMapMode( MAP_100TH_MM );
    maPrinter.SetJobSetup( rSetup );

    rOutputSize = maPrinter.PixelToLogic( maPrinter.GetOutputSizePixel(), mpVDev->GetMapMode() );

    aAttr = SVGActionWriter::GetValueString( rOutputSize.Width(), sal_True );
    AddAttribute( XML_NAMESPACE_NONE, aXMLAttrWidth, aAttr );

    aAttr = SVGActionWriter::GetValueString( rOutputSize.Height(), sal_True );
    AddAttribute( XML_NAMESPACE_NONE, aXMLAttrHeight, aAttr );

    aAttr = B2UCONST( "0.0 0.0 " );
    aAttr += SVGActionWriter::GetValueString( rOutputSize.Width(), sal_True );
    aAttr += B2UCONST( " " );
    aAttr += SVGActionWriter::GetValueString( rOutputSize.Height(), sal_True );
    AddAttribute( XML_NAMESPACE_NONE, aXMLAttrViewBox, aAttr );

    return( new SvXMLElementExport( *this, XML_NAMESPACE_NONE, aXMLElemSVG, sal_True, sal_True ) );
}

// -----------------------------------------------------------------------------

void SVGPrinterExport::ImplWriteMetaAttr( sal_Bool bOuter, sal_Bool bPage )
{
    SvXMLElementExport  aMetaData( *this, XML_NAMESPACE_NONE, aXMLElemMeta, sal_True, sal_True );
    NMSP_RTL::OUString  aAttr;

    aAttr = bOuter ? B2UCONST( "true" ) : B2UCONST( "false" );
    AddAttribute( XML_NAMESPACE_NONE, aXMLAttrMetaSVGOuter, aAttr );

    aAttr = bPage ? B2UCONST( "true" ) : B2UCONST( "false" );
    AddAttribute( XML_NAMESPACE_NONE, aXMLAttrMetaSVGPage, aAttr );

    {
        delete( new SvXMLElementExport( *this, XML_NAMESPACE_NONE, aXMLElemMetaSVG, sal_True, sal_True ) );
    }
}

// -----------------------------------------------------------------------------

void SVGPrinterExport::writePage( const JobSetup& rSetup, const GDIMetaFile& rMtf )
{
    Size                aOutputSize;
    NMSP_RTL::OUString  aAttr;
    SvXMLElementExport* pPageElem = ImplCreateSVGElement( rSetup, aOutputSize );

    // write description
    SvXMLElementExport* pDescElem = new SvXMLElementExport( *this, XML_NAMESPACE_NONE, aXMLElemDesc, sal_True, sal_True );
    NMSP_RTL::OUString  aDesc( B2UCONST( "page: " ) );

    GetDocHandler()->characters( aDesc += NMSP_RTL::OUString::valueOf( (sal_Int32) ++mnPage ) );
    delete pDescElem;

    // write meta attributes
    ImplWriteMetaAttr( sal_False, sal_True );

    // write dummy rect element
    aAttr = B2UCONST( "0.0" );
    AddAttribute( XML_NAMESPACE_NONE, aXMLAttrX, aAttr );
    AddAttribute( XML_NAMESPACE_NONE, aXMLAttrY, aAttr );

    aAttr = SVGActionWriter::GetValueString( aOutputSize.Width(), sal_True );
    AddAttribute( XML_NAMESPACE_NONE, aXMLAttrWidth, aAttr );

    aAttr = SVGActionWriter::GetValueString( aOutputSize.Height(), sal_True );
    AddAttribute( XML_NAMESPACE_NONE, aXMLAttrHeight, aAttr );

    delete( new SvXMLElementExport( *this, XML_NAMESPACE_NONE, aXMLElemRect, sal_True, sal_True ) );
    delete( new SVGActionWriter( *this, rMtf, mpVDev, sal_True ) );

    delete pPageElem;
}

// --------------
// - SVGPrinter -
// --------------

SVGPrinter::SVGPrinter( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr ) :
    mxFact( rxMgr ),
    mpWriter( NULL )
{
}

// -----------------------------------------------------------------------------

SVGPrinter::~SVGPrinter()
{
    delete mpWriter;
}

// -----------------------------------------------------------------------------


ANY SAL_CALL SVGPrinter::queryInterface( const NMSP_UNO::Type & rType ) throw( NMSP_UNO::RuntimeException )
{
    const ANY aRet( NMSP_CPPU::queryInterface( rType, static_cast< NMSP_SVG::XSVGPrinter* >( this ) ) );

    return( aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ) );
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGPrinter::acquire() throw()
{
    OWeakObject::acquire();
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGPrinter::release() throw()
{
    OWeakObject::release();
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL SVGPrinter::startJob( const REF( NMSP_SAX::XDocumentHandler )& rxHandler,
                                        const SEQ( sal_Int8 )& rJobSetupSeq,
                                        const NMSP_RTL::OUString& rJobName,
                                        sal_uInt32 nCopies, sal_Bool bCollate ) throw( NMSP_UNO::RuntimeException )
{
    const sal_Bool bRet = ( mpWriter == NULL );

    if( bRet )
    {
        SvMemoryStream  aMemStm( (char*) rJobSetupSeq.getConstArray(), rJobSetupSeq.getLength(), STREAM_READ );
        JobSetup        aJobSetup;

        aMemStm.SetCompressMode( COMPRESSMODE_FULL );
        aMemStm >> aJobSetup;

        const REF( NMSP_SAX::XDocumentHandler ) xDocumentHandler( rxHandler );

        // #110680#
        // mpWriter = new SVGPrinterExport( xDocumentHandler, aJobSetup, rJobName, nCopies, bCollate );
        mpWriter = new SVGPrinterExport( mxFact, xDocumentHandler, aJobSetup, rJobName, nCopies, bCollate );
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGPrinter::printPage( const SEQ( sal_Int8 )& rPrintPage ) throw( NMSP_UNO::RuntimeException )
{
    SvMemoryStream  aMemStm( (char*) rPrintPage.getConstArray(), rPrintPage.getLength(), STREAM_READ );
    PrinterPage     aPage;

    aMemStm.SetCompressMode( COMPRESSMODE_FULL );
    aMemStm >> aPage;
    mpWriter->writePage( aPage.GetJobSetup(), *aPage.GetGDIMetaFile() );
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGPrinter::endJob() throw( NMSP_UNO::RuntimeException )
{
    delete mpWriter, mpWriter = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
