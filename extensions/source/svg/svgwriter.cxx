/*************************************************************************
 *
 *  $RCSfile: svgwriter.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    virtual void            _ExportStyles( BOOL bUsed ) {}
    virtual void            _ExportAutoStyles() {}
    virtual void            _ExportContent() {}
    virtual void            _ExportMasterStyles() {}
    virtual ULONG           exportDoc( const char *pClass = 0 ) { return 0; }

public:

#ifndef _SVG_UNO3
                            SVGMtfExport( XDocumentHandlerRef& rXHandler );
#else
                            SVGMtfExport( const REF( NMSP_SAX::XDocumentHandler )& rxHandler );
#endif
    virtual                 ~SVGMtfExport();

    virtual void            writeMtf( const GDIMetaFile& rMtf );
};

// -----------------------------------------------------------------------------

#ifndef _SVG_UNO3
SVGMtfExport::SVGMtfExport( XDocumentHandlerRef& rxHandler ) :
#else
SVGMtfExport::SVGMtfExport( const REF( NMSP_SAX::XDocumentHandler )& rxHandler ) :
#endif
        SvXMLExport( NMSP_RTL::OUString(), rxHandler )
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
#ifndef _SVG_UNO3
    XExtendedDocumentHandlerRef                 xExtDocHandler( GetDocHandler(), USR_QUERY );
#else
    REF( NMSP_SAX::XExtendedDocumentHandler )   xExtDocHandler( GetDocHandler(), NMSP_UNO::UNO_QUERY );
#endif

    if( xExtDocHandler.is() )
        xExtDocHandler->unknown( B2UCONST( "<!DOCTYPE svg SYSTEM \"svg-19990812.dtd\">" ) );

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
        SvXMLElementExport  aSVG( *this, XML_NAMESPACE_NONE, "svg", TRUE, TRUE );
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

void SAL_CALL SVGWriter::acquire() throw( NMSP_UNO::RuntimeException )
{
    OWeakObject::acquire();
}

// -----------------------------------------------------------------------------

void SAL_CALL SVGWriter::release() throw( NMSP_UNO::RuntimeException )
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

#ifndef _SVG_UNO3
    NMSP_UNO::Mapping ecpptosmart( CPPU_CURRENT_LANGUAGE_BINDING_NAME, "smart" );
    XDocumentHandlerRef xDocumentHandler( (XDocumentHandler*)ecpptosmart.mapInterface( rxDocHandler.get(), getCppuType(&rxDocHandler)) );
    xDocumentHandler->release();
#else
    const REF( NMSP_SAX::XDocumentHandler ) xDocumentHandler( rxDocHandler );
#endif

    SVGMtfExport* pWriter = new SVGMtfExport( xDocumentHandler );
    pWriter->writeMtf( aMtf );
    delete pWriter;
}
