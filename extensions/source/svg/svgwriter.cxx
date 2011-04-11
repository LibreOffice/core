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

#include "svgwriter.hxx"
#include "svgaction.hxx"
#include <uno/mapping.hxx>

// ----------------
// - SVGMtfExport -
// ----------------

class SVGMtfExport : public SvXMLExport
{
private:

                            SVGMtfExport();

protected:

    virtual void            _ExportMeta() {}
    virtual void            _ExportStyles( sal_Bool /*bUsed*/ ) {}
    virtual void            _ExportAutoStyles() {}
    virtual void            _ExportContent() {}
    virtual void            _ExportMasterStyles() {}
    virtual sal_uInt32      exportDoc( enum ::xmloff::token::XMLTokenEnum /*eClass*/ ) { return 0; }

public:

    SVGMtfExport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        const REF( NMSP_SAX::XDocumentHandler )& rxHandler );

    virtual                 ~SVGMtfExport();

    virtual void            writeMtf( const GDIMetaFile& rMtf );
};

// -----------------------------------------------------------------------------

SVGMtfExport::SVGMtfExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const REF( NMSP_SAX::XDocumentHandler )& rxHandler )
:   SvXMLExport( xServiceFactory, NMSP_RTL::OUString(), rxHandler )
{
    GetDocHandler()->startDocument();
}

// -----------------------------------------------------------------------------

SVGMtfExport::~SVGMtfExport()
{
    GetDocHandler()->endDocument();
}

// -----------------------------------------------------------------------------

void SVGMtfExport::writeMtf( const GDIMetaFile& rMtf )
{
    const Size                                  aSize( OutputDevice::LogicToLogic( rMtf.GetPrefSize(), rMtf.GetPrefMapMode(), MAP_MM ) );
    NMSP_RTL::OUString                          aAttr;
    REF( NMSP_SAX::XExtendedDocumentHandler )   xExtDocHandler( GetDocHandler(), NMSP_UNO::UNO_QUERY );

    if( xExtDocHandler.is() )
        xExtDocHandler->unknown( SVG_DTD_STRING );

    aAttr = NMSP_RTL::OUString::valueOf( aSize.Width() );
    aAttr += B2UCONST( "mm" );
    AddAttribute( XML_NAMESPACE_NONE, "width", aAttr );

    aAttr = NMSP_RTL::OUString::valueOf( aSize.Height() );
    aAttr += B2UCONST( "mm" );
    AddAttribute( XML_NAMESPACE_NONE, "height", aAttr );

    aAttr = B2UCONST( "0 0 " );
    aAttr += NMSP_RTL::OUString::valueOf( aSize.Width() * 100L );
    aAttr += B2UCONST( " " );
    aAttr += NMSP_RTL::OUString::valueOf( aSize.Height() * 100L );
    AddAttribute( XML_NAMESPACE_NONE, "viewBox", aAttr );

    {
        SvXMLElementExport  aSVG( *this, XML_NAMESPACE_NONE, "svg", sal_True, sal_True );
        SVGActionWriter*    pWriter = new SVGActionWriter( *this, rMtf );

        delete pWriter;
    }
}

// -------------
// - SVGWriter -
// -------------

SVGWriter::SVGWriter( const REF( NMSP_LANG::XMultiServiceFactory )& rxMgr ) :
    mxFact( rxMgr )
{
}

// -----------------------------------------------------------------------------

SVGWriter::~SVGWriter()
{
}

// -----------------------------------------------------------------------------


ANY SAL_CALL SVGWriter::queryInterface( const NMSP_UNO::Type & rType ) throw( NMSP_UNO::RuntimeException )
{
    const ANY aRet( NMSP_CPPU::queryInterface( rType, static_cast< NMSP_SVG::XSVGWriter* >( this ) ) );

    return( aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType ) );
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGWriter::acquire() throw()
{
    OWeakObject::acquire();
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGWriter::release() throw()
{
    OWeakObject::release();
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGWriter::write( const REF( NMSP_SAX::XDocumentHandler )& rxDocHandler,
                                const SEQ( sal_Int8 )& rMtfSeq ) throw( NMSP_UNO::RuntimeException )
{
    SvMemoryStream  aMemStm( (char*) rMtfSeq.getConstArray(), rMtfSeq.getLength(), STREAM_READ );
    GDIMetaFile     aMtf;

    aMemStm.SetCompressMode( COMPRESSMODE_FULL );
    aMemStm >> aMtf;

    const REF( NMSP_SAX::XDocumentHandler ) xDocumentHandler( rxDocHandler );

    SVGMtfExport* pWriter = new SVGMtfExport( mxFact, xDocumentHandler );

    pWriter->writeMtf( aMtf );
    delete pWriter;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
