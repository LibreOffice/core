 /*************************************************************************
 *
 *  $RCSfile: svgexport.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 17:57:52 $
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
#include "svgfontexport.hxx"
#include "svgfilter.hxx"

// -------------
// - SVGExport -
// -------------

SVGExport::SVGExport( const Reference< XDocumentHandler >& rxHandler ) :
        SvXMLExport( OUString(), rxHandler )
{
    GetDocHandler()->startDocument();
}

// -----------------------------------------------------------------------------

SVGExport::~SVGExport()
{
    GetDocHandler()->endDocument();
}

// ------------------------
// - ObjectRepresentation -
// ------------------------

ObjectRepresentation::ObjectRepresentation() :
    mpMtf( NULL )
{
}

// -----------------------------------------------------------------------------

ObjectRepresentation::ObjectRepresentation( const Reference< XInterface >& rxObject,
                                            const GDIMetaFile& rMtf ) :
    mxObject( rxObject ),
    mpMtf( new GDIMetaFile( rMtf ) )
{
}

// -----------------------------------------------------------------------------

ObjectRepresentation::ObjectRepresentation( const ObjectRepresentation& rPresentation ) :
    mxObject( rPresentation.mxObject ),
    mpMtf( rPresentation.mpMtf ? new GDIMetaFile( *rPresentation.mpMtf ) : NULL )
{
}

// -----------------------------------------------------------------------------

ObjectRepresentation::~ObjectRepresentation()
{
    delete mpMtf;
}

// -----------------------------------------------------------------------------

ObjectRepresentation& ObjectRepresentation::operator=( const ObjectRepresentation& rPresentation )
{
    mxObject = rPresentation.mxObject;
    delete mpMtf, ( mpMtf = rPresentation.mpMtf ? new GDIMetaFile( *rPresentation.mpMtf ) : NULL );

    return *this;
}

// -----------------------------------------------------------------------------

bool ObjectRepresentation::operator==( const ObjectRepresentation& rPresentation ) const
{
    return( ( mxObject == rPresentation.mxObject ) &&
            ( *mpMtf == *rPresentation.mpMtf ) );
}

// -------------
// - SVGFilter -
// -------------

sal_Bool SVGFilter::implExport( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    Reference< XMultiServiceFactory >   xServiceFactory( ::comphelper::getProcessServiceFactory() ) ;
    Reference< XOutputStream >          xOStm;
    sal_Int32                           nLength = rDescriptor.getLength();
    const PropertyValue*                pValue = rDescriptor.getConstArray();
    sal_Bool                            bRet = sal_False;

    for ( sal_Int32 i = 0 ; ( i < nLength ) && !xOStm.is(); ++i)
        if( pValue[ i ].Name.equalsAscii( "OutputStream" ) )
            pValue[ i ].Value >>= xOStm;

    if( xOStm.is() && xServiceFactory.is() )
    {
        Reference< XMasterPagesSupplier >   xMasterPagesSupplier( mxSrcDoc, UNO_QUERY );
        Reference< XDrawPagesSupplier >     xDrawPagesSupplier( mxSrcDoc, UNO_QUERY );

        if( xMasterPagesSupplier.is() && xDrawPagesSupplier.is() )
        {
            Reference< XDrawPages >   xMasterPages( xMasterPagesSupplier->getMasterPages(), UNO_QUERY );
            Reference< XDrawPages >   xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY );

            if( xMasterPages.is() && xDrawPages->getCount() &&
                xDrawPages.is() && xDrawPages->getCount() )
            {
                Reference< XDocumentHandler > xDocHandler( implCreateExportDocumentHandler( xOStm ) );

                if( xDocHandler.is() )
                {
                    mbPresentation = Reference< XPresentationSupplier >( mxSrcDoc, UNO_QUERY ).is();
                    mpObjects = new ObjectMap;
                    mpSVGExport = new SVGExport( xDocHandler );

                    try
                    {
                        if( implCreateObjects( xMasterPages, xDrawPages ) )
                        {
                            ObjectMap::const_iterator               aIter( mpObjects->begin() );
                            ::std::vector< ObjectRepresentation >   aObjects( mpObjects->size() );
                            sal_uInt32                              nPos = 0;

                            while( aIter != mpObjects->end() )
                            {
                                aObjects[ nPos++ ] = (*aIter).second;
                                ++aIter;
                            }

                            mpSVGFontExport = new SVGFontExport( *mpSVGExport, aObjects );
                            mpSVGWriter = new SVGActionWriter( *mpSVGExport, *mpSVGFontExport );

                            bRet = implExportDocumemt( xMasterPages, xDrawPages );
                        }
                    }
                    catch( ... )
                    {
                        delete mpSVGDoc, mpSVGDoc = NULL;
                        DBG_ERROR( "Exception caught" );
                    }

                    delete mpSVGWriter, mpSVGWriter = NULL;
                    delete mpSVGExport, mpSVGExport = NULL;
                    delete mpSVGFontExport, mpSVGFontExport = NULL;
                    delete mpObjects, mpObjects = NULL;
                    mbPresentation = sal_False;
                }
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

Reference< XDocumentHandler > SVGFilter::implCreateExportDocumentHandler( const Reference< XOutputStream >& rxOStm )
{
    Reference< XMultiServiceFactory >   xMgr( ::comphelper::getProcessServiceFactory() );
    Reference< XDocumentHandler >       xSaxWriter;

    if( xMgr.is() && rxOStm.is() )
    {
        xSaxWriter = Reference< XDocumentHandler >( xMgr->createInstance( B2UCONST( "com.sun.star.xml.sax.Writer" ) ), UNO_QUERY );

        if( xSaxWriter.is() )
        {
            Reference< XActiveDataSource > xActiveDataSource( xSaxWriter, UNO_QUERY );

            if( xActiveDataSource.is() )
                xActiveDataSource->setOutputStream( rxOStm );
            else
                xSaxWriter = NULL;
        }
    }

    return xSaxWriter;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implExportDocumemt( const Reference< XDrawPages >& rxMasterPages,
                                        const Reference< XDrawPages >& rxDrawPages )
{
    sal_Bool                bSVGStarted = sal_False;
    sal_Bool                bRet = sal_False;

    if( rxDrawPages->getCount() )
    {
        Reference< XDrawPage > xDrawPage;

        rxDrawPages->getByIndex( 0 ) >>= xDrawPage;

        if( xDrawPage.is() )
        {
            Reference< XPropertySet > xPagePropSet( xDrawPage, UNO_QUERY );

            if( xPagePropSet.is() )
            {
                Reference< XExtendedDocumentHandler >   xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );
                OUString                                aAttr;
                sal_Int32                               nDocWidth = 0, nDocHeight = 0;
                sal_Int32                               nVisible = -1, nVisibleMaster = -1;
                const sal_Bool                          bMultiPage = ( rxDrawPages->getCount() > 1 );

                xPagePropSet->getPropertyValue( B2UCONST( "Width" ) ) >>= nDocWidth;
                xPagePropSet->getPropertyValue( B2UCONST( "Height" ) ) >>= nDocHeight;

                if( xExtDocHandler.is() )
                    xExtDocHandler->unknown( SVG_DTD_STRING );

#ifdef _SVG_WRITE_EXTENTS
                aAttr = OUString::valueOf( nDocWidth * 0.01 );
                aAttr += B2UCONST( "mm" );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "width", aAttr );

                aAttr = OUString::valueOf( nDocHeight * 0.01 );
                aAttr += B2UCONST( "mm" );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "height", aAttr );
#endif

                aAttr = B2UCONST( "0 0 " );
                aAttr += OUString::valueOf( nDocWidth );
                aAttr += B2UCONST( " " );
                aAttr += OUString::valueOf( nDocHeight );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "viewBox", aAttr );

                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "preserveAspectRatio", B2UCONST( "xMidYMid" ) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "fill-rule", B2UCONST( "evenodd" ) );

                if( bMultiPage )
                {
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:ooo", B2UCONST( "http://xml.openoffice.org/svg/export" ) );
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "onclick", B2UCONST( "onClick(evt)" ) );
                    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "onkeypress", B2UCONST( "onKeyPress(evt)" ) );
                }

                mpSVGDoc = new SvXMLElementExport( *mpSVGExport, XML_NAMESPACE_NONE, "svg", TRUE, TRUE );

                for( sal_Int32 i = 0, nCount = rxDrawPages->getCount(); ( i < nCount ) && ( -1 == nVisible ); ++i )
                {
                    Reference< XDrawPage > xDrawPage;
                    rxDrawPages->getByIndex( i ) >>= xDrawPage;

                    if( xDrawPage.is() )
                    {
                        Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );

                        if( xPropSet.is() )
                        {
                            sal_Bool bVisible;

                            if( !mbPresentation || ( ( xPropSet->getPropertyValue( B2UCONST( "Visible" ) ) >>= bVisible ) && bVisible ) )
                            {
                                Reference< XMasterPageTarget > xMasterTarget( xDrawPage, UNO_QUERY );

                                if( xMasterTarget.is() )
                                {
                                    Reference< XDrawPage > xMasterPage( xMasterTarget->getMasterPage() );

                                    nVisible = i;

                                    for( sal_Int32 nMaster = 0, nMasterCount = rxMasterPages->getCount(); ( nMaster < nMasterCount ) && ( -1 == nVisibleMaster ); ++nMaster )
                                    {
                                        Reference< XDrawPage > xMasterTestPage;
                                        rxMasterPages->getByIndex( nMaster ) >>= xMasterTestPage;

                                        if( xMasterTestPage == xMasterPage )
                                            nVisibleMaster = nMaster;
                                    }
                                }
                            }
                        }
                    }
                }

#ifdef _SVG_EMBED_FONTS
                mpSVGFontExport->EmbedFonts();
#endif

                if( bMultiPage )
                {
                    implGenerateMetaData( rxMasterPages, rxDrawPages );
                    implGenerateScript( rxMasterPages, rxDrawPages );
                }

                implExportPages( rxMasterPages, nVisibleMaster, sal_True );
                implExportPages( rxDrawPages, nVisible, sal_False );

                delete mpSVGDoc, mpSVGDoc = NULL;
                bRet = sal_True;
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implGenerateMetaData( const Reference< XDrawPages >& rxMasterPages,
                                          const Reference< XDrawPages >& rxDrawPages )
{
    sal_Bool bRet = sal_False;

    if( rxDrawPages->getCount() )
    {
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", B2UCONST( "meta_slides" ) );
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "numberOfSlides", OUString::valueOf( rxDrawPages->getCount() ) );

        {
            SvXMLElementExport  aExp( *mpSVGExport, XML_NAMESPACE_NONE, "ooo:slidesInfo", TRUE, TRUE );
            const OUString      aId( B2UCONST( "meta_slide" ) );

            for( sal_Int32 i = 0, nCount = rxDrawPages->getCount(); i < nCount; ++i )
            {
                OUString                        aSlideId( aId );
                Reference< XDrawPage >          xDrawPage( rxDrawPages->getByIndex( i ), UNO_QUERY );
                Reference< XMasterPageTarget >  xMasterPageTarget( xDrawPage, UNO_QUERY );
                Reference< XDrawPage >          xMasterPage( xMasterPageTarget->getMasterPage(), UNO_QUERY );
                sal_Bool                        bMasterVisible = sal_True;
                OUString                        aMasterVisibility;

                aSlideId += OUString::valueOf( i );

                if( mbPresentation )
                {
                    Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );

                    if( xPropSet.is() )
                        xPropSet->getPropertyValue( B2UCONST( "Background" ) )  >>= bMasterVisible;
                }

                if( bMasterVisible )
                    aMasterVisibility = B2UCONST( "visible" );
                else
                    aMasterVisibility = B2UCONST( "hidden" );

                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", aSlideId );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "slide", implGetValidIDFromInterface( xDrawPage ) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "master", implGetValidIDFromInterface( xMasterPage ) );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "master-visibility", aMasterVisibility );

                {
                    SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "ooo:slideInfo", TRUE, TRUE );
                }
            }
        }


        bRet =sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implGenerateScript( const Reference< XDrawPages >& rxMasterPages,
                                        const Reference< XDrawPages >& rxDrawPages )
{
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "type", B2UCONST( "text/ecmascript" ) );

    {
        SvXMLElementExport                      aExp( *mpSVGExport, XML_NAMESPACE_NONE, "script", TRUE, TRUE );
        Reference< XExtendedDocumentHandler >   xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

        if( xExtDocHandler.is() )
        {
            xExtDocHandler->unknown( OUString::createFromAscii( aSVGScript1 ) );
            xExtDocHandler->unknown( OUString::createFromAscii( aSVGScript2 ) );
        }
    }

    return sal_True;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implExportPages( const Reference< XDrawPages >& rxMasterPages,
                                     sal_Int32 nVisiblePage, sal_Bool bMaster )
{
    sal_Bool bRet = sal_False;

    for( sal_Int32 i = 0, nCount = rxMasterPages->getCount(); i < nCount; ++i )
    {
        Reference< XDrawPage > xDrawPage;

        rxMasterPages->getByIndex( i ) >>= xDrawPage;

        if( xDrawPage.is() )
        {
            Reference< XShapes > xShapes( xDrawPage, UNO_QUERY );

            if( xShapes.is() )
            {
                OUString aAttr;

                if( i == nVisiblePage )
                    aAttr = B2UCONST( "visible" );
                else
                    aAttr = B2UCONST( "hidden" );

                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "visibility", aAttr );
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", implGetValidIDFromInterface( xShapes ) );

                {
                    SvXMLElementExport  aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", TRUE, TRUE );
                    const Point         aNullPt;

                    {
                        Reference< XExtendedDocumentHandler > xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

                        if( xExtDocHandler.is() )
                        {
                            SvXMLElementExport  aExp( *mpSVGExport, XML_NAMESPACE_NONE, "desc", TRUE, TRUE );
                            OUString            aDesc;

                            if( bMaster )
                                aDesc = B2UCONST( "Master slide" );
                            else
                                aDesc = B2UCONST( "Slide" );

                            xExtDocHandler->unknown( aDesc );
                        }
                    }

                    if( bMaster )
                    {
                        const GDIMetaFile& rMtf = (*mpObjects)[ xDrawPage ].GetRepresentation();
                        mpSVGWriter->WriteMetaFile( aNullPt, rMtf.GetPrefSize(), rMtf, SVGWRITER_WRITE_FILL );
                    }

                    bRet = implExportShapes( xShapes ) || bRet;
                }
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implExportShapes( const Reference< XShapes >& rxShapes )
{
    Reference< XShape > xShape;
    sal_Bool            bRet = sal_False;

    for( sal_Int32 i = 0, nCount = rxShapes->getCount(); i < nCount; ++i )
    {
        if( ( rxShapes->getByIndex( i ) >>= xShape ) && xShape.is() )
            bRet = implExportShape( xShape ) || bRet;

        xShape = NULL;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implExportShape( const Reference< XShape >& rxShape )
{
    Reference< XPropertySet >   xPropSet( rxShape, UNO_QUERY );
    sal_Bool                    bRet = sal_False;

    if( xPropSet.is() )
    {
        sal_Bool bEmptyPres;

        if( mbPresentation )
            xPropSet->getPropertyValue( B2UCONST( "IsEmptyPresentationObject" ) )  >>= bEmptyPres;
        else
            bEmptyPres = sal_False;

        if( !bEmptyPres )
        {
            OUString aObjName( implGetValidIDFromInterface( rxShape ) ), aObjDesc;

            if( aObjName.getLength() )
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", aObjName );

            if( rxShape->getShapeType().lastIndexOf( B2UCONST( "drawing.GroupShape" ) ) != -1 )
            {
                Reference< XShapes > xShapes( rxShape, UNO_QUERY );

                if( xShapes.is() )
                {
                    SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", TRUE, TRUE );

                    {
                        SvXMLElementExport                      aExp( *mpSVGExport, XML_NAMESPACE_NONE, "desc", TRUE, TRUE );
                        Reference< XExtendedDocumentHandler >   xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

                        xExtDocHandler->unknown( B2UCONST( "Group" ) );
                    }

                    bRet = implExportShapes( xShapes );
                }
            }

            if( !bRet )
            {
                 Reference< XText >                  xText( rxShape, UNO_QUERY );
                ::com::sun::star::awt::Rectangle    aBoundRect;
                const GDIMetaFile&                  rMtf = (*mpObjects)[ rxShape ].GetRepresentation();

                xPropSet->getPropertyValue( B2UCONST( "BoundRect" ) ) >>= aBoundRect;
                const Point aTopLeft( aBoundRect.X, aBoundRect.Y );
                const Size  aSize( aBoundRect.Width, aBoundRect.Height );

                {
                    SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", TRUE, TRUE );

                    {
                        SvXMLElementExport                      aExp( *mpSVGExport, XML_NAMESPACE_NONE, "desc", TRUE, TRUE );
                        Reference< XExtendedDocumentHandler >   xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

                        xExtDocHandler->unknown( implGetDescriptionFromShape( rxShape ) );
                    }

                    if( rMtf.GetActionCount() )
                    {
                        if( ( rxShape->getShapeType().lastIndexOf( B2UCONST( "drawing.OLE2Shape" ) ) != -1 ) ||
                            ( rxShape->getShapeType().lastIndexOf( B2UCONST( "drawing.GraphicObjectShape" ) ) != -1 ) )
                        {
                            SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", TRUE, TRUE );
                            mpSVGWriter->WriteMetaFile( aTopLeft, aSize, rMtf, SVGWRITER_WRITE_ALL);
                        }
                        else
                        {
                            SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", TRUE, TRUE );
                            mpSVGWriter->WriteMetaFile( aTopLeft, aSize, rMtf, SVGWRITER_WRITE_FILL );

                            if( xText.is() && xText->getString().getLength() )
                            {
                                SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", TRUE, TRUE );
                                mpSVGWriter->WriteMetaFile( aTopLeft, aSize, rMtf, SVGWRITER_WRITE_TEXT );
                            }
                        }
                    }
                }

                bRet = sal_True;
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implCreateObjects( const Reference< XDrawPages >& rxMasterPages,
                                       const Reference< XDrawPages >& rxDrawPages )
{
    sal_Int32 i, nCount;

    for( i = 0, nCount = rxMasterPages->getCount(); i < nCount; ++i )
    {
        Reference< XDrawPage > xDrawPage;

        rxMasterPages->getByIndex( i ) >>= xDrawPage;

        if( xDrawPage.is() )
        {
            Reference< XShapes > xShapes( xDrawPage, UNO_QUERY );

            implCreateObjectsFromBackground( xDrawPage );

            if( xShapes.is() )
                implCreateObjectsFromShapes( xShapes );
        }
    }

    for( i = 0, nCount = rxDrawPages->getCount(); i < nCount; ++i )
    {
        Reference< XDrawPage > xDrawPage;

        rxDrawPages->getByIndex( i ) >>= xDrawPage;

        if( xDrawPage.is() )
        {
            Reference< XShapes > xShapes( xDrawPage, UNO_QUERY );

            if( xShapes.is() )
                implCreateObjectsFromShapes( xShapes );
        }
    }

    return sal_True;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implCreateObjectsFromShapes( const Reference< XShapes >& rxShapes )
{
    Reference< XShape > xShape;
    sal_Bool            bRet = sal_False;

    for( sal_Int32 i = 0, nCount = rxShapes->getCount(); i < nCount; ++i )
    {
        if( ( rxShapes->getByIndex( i ) >>= xShape ) && xShape.is() )
            bRet = implCreateObjectsFromShape( xShape ) || bRet;

        xShape = NULL;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implCreateObjectsFromShape( const Reference< XShape >& rxShape )
{
    sal_Bool bRet = sal_False;

    if( rxShape->getShapeType().lastIndexOf( B2UCONST( "drawing.GroupShape" ) ) != -1 )
    {
        Reference< XShapes > xShapes( rxShape, UNO_QUERY );

        if( xShapes.is() )
            bRet = implCreateObjectsFromShapes( xShapes );
    }
    else
    {
        SdrObject*  pObj = GetSdrObjectFromXShape( rxShape );

        if( pObj )
        {
            Graphic aGraphic( SdrExchangeView::GetObjGraphic( pObj->GetModel(), pObj ) );

            if( aGraphic.GetType() != GRAPHIC_NONE )
            {
                if( aGraphic.GetType() == GRAPHIC_BITMAP )
                {
                    GDIMetaFile aMtf;
                    const Point aNullPt;
                    const Size  aSize( pObj->GetBoundRect().GetSize() );

                    aMtf.AddAction( new MetaBmpExScaleAction( aNullPt, aSize, aGraphic.GetBitmapEx() ) );
                    aMtf.SetPrefSize( aSize );
                    aMtf.SetPrefMapMode( MAP_100TH_MM );

                    (*mpObjects)[ rxShape ] = ObjectRepresentation( rxShape, aMtf );
                }
                else
                    (*mpObjects)[ rxShape ] = ObjectRepresentation( rxShape, aGraphic.GetGDIMetaFile() );

                bRet = sal_True;
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implCreateObjectsFromBackground( const Reference< XDrawPage >& rxMasterPage )
{
    Reference< XExporter >  xExporter( mxMSF->createInstance( B2UCONST( "com.sun.star.drawing.GraphicExportFilter" ) ), UNO_QUERY );
    sal_Bool                bRet = sal_False;

    if( xExporter.is() )
    {
        GDIMetaFile             aMtf;
        Reference< XFilter >    xFilter( xExporter, UNO_QUERY );

        utl::TempFile aFile;
        aFile.EnableKillingFile();

        Sequence< PropertyValue > aDescriptor( 3 );
        aDescriptor[0].Name = B2UCONST( "FilterName" );
        aDescriptor[0].Value <<= B2UCONST( "SVM" );
        aDescriptor[1].Name = B2UCONST( "URL" );
        aDescriptor[1].Value <<= OUString( aFile.GetURL() );
        aDescriptor[2].Name = B2UCONST( "ExportOnlyBackground" );
        aDescriptor[2].Value <<= (sal_Bool) sal_True;

        xExporter->setSourceDocument( Reference< XComponent >( rxMasterPage, UNO_QUERY ) );
        xFilter->filter( aDescriptor );
        aMtf.Read( *aFile.GetStream( STREAM_READ ) );

        (*mpObjects)[ rxMasterPage ] = ObjectRepresentation( rxMasterPage, aMtf );

        bRet = sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

OUString SVGFilter::implGetDescriptionFromShape( const Reference< XShape >& rxShape )
{
    OUString            aRet;
    Reference< XText >  xText( rxShape, UNO_QUERY );

    if( rxShape->getShapeType().lastIndexOf( B2UCONST( "drawing.GroupShape" ) ) != -1 )
        aRet = B2UCONST( "Group" );
    else if( rxShape->getShapeType().lastIndexOf( B2UCONST( "drawing.GraphicObjectShape" ) ) != -1 )
        aRet = B2UCONST( "Graphic" );
    else if( rxShape->getShapeType().lastIndexOf( B2UCONST( "drawing.OLE2Shape" ) ) != -1 )
        aRet = B2UCONST( "OLE2" );
    else
        aRet = B2UCONST( "Drawing" );

    if( xText.is() && xText->getString().getLength() )
        aRet += B2UCONST( " with text" );

    return aRet;
}

// -----------------------------------------------------------------------------

OUString SVGFilter::implGetValidIDFromInterface( const Reference< XInterface >& rxIf )
{
    Reference< XNamed > xNamed( rxIf, UNO_QUERY );
    OUString            aRet;

    if( xNamed.is() )
        aRet = xNamed->getName().replace( ' ', '_' );

    return aRet;
}
