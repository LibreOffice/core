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
#include "precompiled_filter.hxx"

#define ITEMID_FIELD 0
 
#include "svgwriter.hxx"
#include "svgfontexport.hxx"
#include "svgfilter.hxx"

#include <svx/unopage.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoutl.hxx>
#include <editeng/outliner.hxx>
#include <editeng/flditem.hxx>
#include <editeng/numitem.hxx>

using ::rtl::OUString;

// -------------
// - SVGExport -
// -------------

// #110680#
SVGExport::SVGExport( 
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const Reference< XDocumentHandler >& rxHandler ) 
:	SvXMLExport( xServiceFactory, OUString(), rxHandler )
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
    Reference< XMultiServiceFactory >	xServiceFactory( ::comphelper::getProcessServiceFactory() ) ;
    Reference< XOutputStream >          xOStm;
    SvStream*                           pOStm = NULL;
    sal_Int32							nLength = rDescriptor.getLength();
    sal_Int32                           nPageToExport = SVG_EXPORT_ALLPAGES;
    const PropertyValue*				pValue = rDescriptor.getConstArray();
    sal_Bool							bRet = sal_False;

    for ( sal_Int32 i = 0 ; i < nLength; ++i)
    {
        if( pValue[ i ].Name.equalsAscii( "OutputStream" ) )
            pValue[ i ].Value >>= xOStm;
        else if( pValue[ i ].Name.equalsAscii( "FileName" ) )
        {
            ::rtl::OUString aFileName;

            pValue[ i ].Value >>= aFileName;
            pOStm = ::utl::UcbStreamHelper::CreateStream( aFileName, STREAM_WRITE | STREAM_TRUNC );

            if( pOStm )
                xOStm = Reference< XOutputStream >( new ::utl::OOutputStreamWrapper ( *pOStm ) );
        }
        else if( pValue[ i ].Name.equalsAscii( "PagePos" ) )
            pValue[ i ].Value >>= nPageToExport;
   }

    if( xOStm.is() && xServiceFactory.is() )
    {
        Reference< XMasterPagesSupplier >	xMasterPagesSupplier( mxSrcDoc, UNO_QUERY );
        Reference< XDrawPagesSupplier >		xDrawPagesSupplier( mxSrcDoc, UNO_QUERY );
    
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

                    // #110680#
                    // mpSVGExport = new SVGExport( xDocHandler );
                    mpSVGExport = new SVGExport( xServiceFactory, xDocHandler );

                    if( nPageToExport < 0 || nPageToExport >= xDrawPages->getCount() )
                        nPageToExport = SVG_EXPORT_ALLPAGES;

                    try
                    {
                        const sal_Int32 nDefaultPage = ( ( SVG_EXPORT_ALLPAGES == nPageToExport ) ? 0 : nPageToExport );
                        
                        xDrawPages->getByIndex( nDefaultPage ) >>= mxDefaultPage;
                        
                        if( mxDefaultPage.is() )
                        {
                            SvxDrawPage* pSvxDrawPage = SvxDrawPage::getImplementation( mxDefaultPage );

                            if( pSvxDrawPage )
                            {
                                mpDefaultSdrPage = pSvxDrawPage->GetSdrPage();
                                mpSdrModel = mpDefaultSdrPage->GetModel();
                                
                                if( mpSdrModel )
                                {
                                    SdrOutliner& rOutl = mpSdrModel->GetDrawOutliner(NULL);
                                    
                                    maOldFieldHdl = rOutl.GetCalcFieldValueHdl();
                                    rOutl.SetCalcFieldValueHdl( LINK( this, SVGFilter, CalcFieldHdl) );
                                }
                            }
                            
                            if( implCreateObjects( xMasterPages, xDrawPages, nPageToExport ) )
                            {
                                ObjectMap::const_iterator				aIter( mpObjects->begin() );
                                ::std::vector< ObjectRepresentation >	aObjects( mpObjects->size() );
                                sal_uInt32								nPos = 0;
    
                                while( aIter != mpObjects->end() )
                                {
                                    aObjects[ nPos++ ] = (*aIter).second;
                                    ++aIter;
                                }
    
                                mpSVGFontExport = new SVGFontExport( *mpSVGExport, aObjects );
                                mpSVGWriter = new SVGActionWriter( *mpSVGExport, *mpSVGFontExport );
                                
                                bRet = implExportDocument( xMasterPages, xDrawPages, nPageToExport );
                            }
                        }
                    }
                    catch( ... )
                    {
                        delete mpSVGDoc, mpSVGDoc = NULL;
                        DBG_ERROR( "Exception caught" );
                    }
                    
                    if( mpSdrModel )
                        mpSdrModel->GetDrawOutliner( NULL ).SetCalcFieldValueHdl( maOldFieldHdl );
                    
                    delete mpSVGWriter, mpSVGWriter = NULL;
                    delete mpSVGExport, mpSVGExport = NULL;
                    delete mpSVGFontExport, mpSVGFontExport = NULL;
                    delete mpObjects, mpObjects = NULL;
                    mbPresentation = sal_False;
                }
            }
        }
    }

    delete pOStm;
    
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

sal_Bool SVGFilter::implExportDocument( const Reference< XDrawPages >& rxMasterPages,
                                        const Reference< XDrawPages >& rxDrawPages,
                                        sal_Int32 nPageToExport )
{
    DBG_ASSERT( rxMasterPages.is() && rxDrawPages.is(), 
                "SVGFilter::implExportDocument: invalid parameter" );

    OUString		aAttr;
    sal_Int32		nDocWidth = 0, nDocHeight = 0;
    sal_Int32		nVisible = -1, nVisibleMaster = -1;
    sal_Bool 		bRet = sal_False;
    const sal_Bool	bSinglePage = ( rxDrawPages->getCount() == 1 ) || ( SVG_EXPORT_ALLPAGES != nPageToExport );
    const sal_Int32 nFirstPage = ( ( SVG_EXPORT_ALLPAGES == nPageToExport ) ? 0 : nPageToExport );
    sal_Int32       nCurPage = nFirstPage, nLastPage = ( bSinglePage ? nFirstPage : ( rxDrawPages->getCount() - 1 ) );

    const Reference< XPropertySet > 			xDefaultPagePropertySet( mxDefaultPage, UNO_QUERY );
    const Reference< XExtendedDocumentHandler > xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );
    
    if( xDefaultPagePropertySet.is() )
    {
        xDefaultPagePropertySet->getPropertyValue( B2UCONST( "Width" ) ) >>= nDocWidth;
        xDefaultPagePropertySet->getPropertyValue( B2UCONST( "Height" ) ) >>= nDocHeight;
    }

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

    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "version", B2UCONST( "1.1" ) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "viewBox", aAttr );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "preserveAspectRatio", B2UCONST( "xMidYMid" ) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "fill-rule", B2UCONST( "evenodd" ) );

    if( !bSinglePage )
    {
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:ooo", B2UCONST( "http://xml.openoffice.org/svg/export" ) );
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "onclick", B2UCONST( "onClick(evt)" ) );
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "onkeypress", B2UCONST( "onKeyPress(evt)" ) );
    }

    
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns", B2UCONST( "http://www.w3.org/2000/svg" ) );
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "xmlns:xlink", B2UCONST( "http://www.w3.org/1999/xlink" ) );

    mpSVGDoc = new SvXMLElementExport( *mpSVGExport, XML_NAMESPACE_NONE, "svg", TRUE, TRUE );

    while( ( nCurPage <= nLastPage ) && ( -1 == nVisible ) )
    {
        Reference< XDrawPage > xDrawPage;

        rxDrawPages->getByIndex( nCurPage ) >>= xDrawPage;

        if( xDrawPage.is() )
        {
            Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );

            if( xPropSet.is() )
            {
                sal_Bool bVisible = sal_False;

                if( !mbPresentation || bSinglePage ||
                    ( ( xPropSet->getPropertyValue( B2UCONST( "Visible" ) ) >>= bVisible ) && bVisible ) )
                {
                    Reference< XMasterPageTarget > xMasterTarget( xDrawPage, UNO_QUERY );

                    if( xMasterTarget.is() )
                    {
                        Reference< XDrawPage > xMasterPage( xMasterTarget->getMasterPage() );

                        nVisible = nCurPage;

                        for( sal_Int32 nMaster = 0, nMasterCount = rxMasterPages->getCount(); 
                             ( nMaster < nMasterCount ) && ( -1 == nVisibleMaster ); 
                             ++nMaster )
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

        ++nCurPage;
    }

#ifdef _SVG_EMBED_FONTS
    mpSVGFontExport->EmbedFonts();
#endif

    if( -1 != nVisible )
    {
        if( bSinglePage )
            implExportPages( rxMasterPages, nVisibleMaster, nVisibleMaster, nVisibleMaster, sal_True );
        else
        {
            implGenerateMetaData( rxMasterPages, rxDrawPages );
            implGenerateScript( rxMasterPages, rxDrawPages );
            implExportPages( rxMasterPages, 0, rxMasterPages->getCount() - 1, nVisibleMaster, sal_True );
        }

        implExportPages( rxDrawPages, nFirstPage, nLastPage, nVisible, sal_False );

        delete mpSVGDoc, mpSVGDoc = NULL;
        bRet = sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implGenerateMetaData( const Reference< XDrawPages >& /* rxMasterPages */, 
                                          const Reference< XDrawPages >& rxDrawPages )
{
    sal_Bool bRet = sal_False;

    if( rxDrawPages->getCount() )
    {
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", B2UCONST( "meta_slides" ) );
        mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "numberOfSlides", OUString::valueOf( rxDrawPages->getCount() ) );

        {
            SvXMLElementExport	aExp( *mpSVGExport, XML_NAMESPACE_NONE, "ooo:slidesInfo", TRUE, TRUE );
            const OUString		aId( B2UCONST( "meta_slide" ) );

            for( sal_Int32 i = 0, nCount = rxDrawPages->getCount(); i < nCount; ++i )
            {
                OUString						aSlideId( aId );
                Reference< XDrawPage >			xDrawPage( rxDrawPages->getByIndex( i ), UNO_QUERY );
                Reference< XMasterPageTarget >	xMasterPageTarget( xDrawPage, UNO_QUERY );
                Reference< XDrawPage >			xMasterPage( xMasterPageTarget->getMasterPage(), UNO_QUERY );
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
                    SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "ooo:slideInfo", TRUE, TRUE );
                }
            }
        }
        

        bRet =sal_True;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implGenerateScript( const Reference< XDrawPages >& /* rxMasterPages */, 
                                        const Reference< XDrawPages >& /* rxDrawPages */ )
{
    mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "type", B2UCONST( "text/ecmascript" ) );

    {
        SvXMLElementExport	                    aExp( *mpSVGExport, XML_NAMESPACE_NONE, "script", TRUE, TRUE );
        Reference< XExtendedDocumentHandler >	xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

        if( xExtDocHandler.is() )
        {
            xExtDocHandler->unknown( OUString::createFromAscii( aSVGScript1 ) );
            xExtDocHandler->unknown( OUString::createFromAscii( aSVGScript2 ) );
        }
    }

    return sal_True;
}

// -----------------------------------------------------------------------------

sal_Bool SVGFilter::implExportPages( const Reference< XDrawPages >& rxPages,
                                     sal_Int32 nFirstPage, sal_Int32 nLastPage, 
                                     sal_Int32 nVisiblePage, sal_Bool bMaster )
{
    DBG_ASSERT( nFirstPage <= nLastPage,
                "SVGFilter::implExportPages: nFirstPage > nLastPage" );

    sal_Bool bRet = sal_False;

    for( sal_Int32 i = nFirstPage; i <= nLastPage; ++i )
    {
        Reference< XDrawPage > xDrawPage;

        rxPages->getByIndex( i ) >>= xDrawPage;

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
                    SvXMLElementExport	aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", TRUE, TRUE );
                    const Point			aNullPt;

                    {
                        Reference< XExtendedDocumentHandler > xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

                        if( xExtDocHandler.is() )
                        {
                            SvXMLElementExport	aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "desc", TRUE, TRUE );
                            OUString			aDesc;

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
    sal_Bool			bRet = sal_False;

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
    Reference< XPropertySet >   xShapePropSet( rxShape, UNO_QUERY );
    sal_Bool                    bRet = sal_False;

    if( xShapePropSet.is() )
    {
        const ::rtl::OUString   aShapeType( rxShape->getShapeType() );
        bool                    bHideObj = false;
        
        if( mbPresentation )
        {    
            xShapePropSet->getPropertyValue( B2UCONST( "IsEmptyPresentationObject" ) )  >>= bHideObj;
            
            if( !bHideObj )
            {
                const Reference< XPropertySet > xDefaultPagePropertySet( mxDefaultPage, UNO_QUERY );
                Reference< XPropertySetInfo > 	xPagePropSetInfo( xDefaultPagePropertySet->getPropertySetInfo() );
                
                if( xPagePropSetInfo.is() )
                {
                    static const ::rtl::OUString aHeaderString( B2UCONST( "IsHeaderVisible" ) );
                    static const ::rtl::OUString aFooterString( B2UCONST( "IsFooterVisible" ) );
                    static const ::rtl::OUString aDateTimeString( B2UCONST( "IsDateTimeVisible" ) );
                    static const ::rtl::OUString aPageNumberString( B2UCONST( "IsPageNumberVisible" ) );
                    
                    Any     aProperty;
                    bool    bValue = sal_False;
                
                    if( ( aShapeType.lastIndexOf( B2UCONST( "presentation.HeaderShape" ) ) != -1 ) &&
                        xPagePropSetInfo->hasPropertyByName( aHeaderString ) &&
                        ( ( aProperty = xDefaultPagePropertySet->getPropertyValue( aHeaderString ) ) >>= bValue ) &&
                        !bValue )
                    {
                        bHideObj = true;
                    }   
                    else if( ( aShapeType.lastIndexOf( B2UCONST( "presentation.FooterShape" ) ) != -1 ) &&
                                xPagePropSetInfo->hasPropertyByName( aFooterString ) &&
                                ( ( aProperty = xDefaultPagePropertySet->getPropertyValue( aFooterString ) ) >>= bValue ) &&
                            !bValue )
                    {
                        bHideObj = true;
                    }   
                    else if( ( aShapeType.lastIndexOf( B2UCONST( "presentation.DateTimeShape" ) ) != -1 ) &&
                                xPagePropSetInfo->hasPropertyByName( aDateTimeString ) &&
                                ( ( aProperty = xDefaultPagePropertySet->getPropertyValue( aDateTimeString ) ) >>= bValue ) &&
                            !bValue )
                    {
                        bHideObj = true;
                    }   
                    else if( ( aShapeType.lastIndexOf( B2UCONST( "presentation.SlideNumberShape" ) ) != -1 ) &&
                                xPagePropSetInfo->hasPropertyByName( aPageNumberString ) &&
                                ( ( aProperty = xDefaultPagePropertySet->getPropertyValue( aPageNumberString ) ) >>= bValue ) &&
                            !bValue )
                    {
                        bHideObj = true;
                    }   
                } 
            }
        }

        if( !bHideObj )
        {
            OUString aObjName( implGetValidIDFromInterface( rxShape ) ), aObjDesc;

            if( aObjName.getLength() )
                mpSVGExport->AddAttribute( XML_NAMESPACE_NONE, "id", aObjName );

            if( aShapeType.lastIndexOf( B2UCONST( "drawing.GroupShape" ) ) != -1 )
            {
                Reference< XShapes > xShapes( rxShape, UNO_QUERY );

                if( xShapes.is() )
                {
                    SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", TRUE, TRUE );

                    {
                        SvXMLElementExport						aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "desc", TRUE, TRUE );
                        Reference< XExtendedDocumentHandler >	xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

                        xExtDocHandler->unknown( B2UCONST( "Group" ) );
                    }

                    bRet = implExportShapes( xShapes );
                }
            }

            if( !bRet )
            {
                 Reference< XText >                  xText( rxShape, UNO_QUERY );
                ::com::sun::star::awt::Rectangle    aBoundRect;
                const GDIMetaFile&					rMtf = (*mpObjects)[ rxShape ].GetRepresentation();
            
                xShapePropSet->getPropertyValue( B2UCONST( "BoundRect" ) ) >>= aBoundRect;
                const Point aTopLeft( aBoundRect.X, aBoundRect.Y );
                const Size  aSize( aBoundRect.Width, aBoundRect.Height );

                {
                    SvXMLElementExport aExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", TRUE, TRUE );

                    {
                        SvXMLElementExport						aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "desc", TRUE, TRUE );
                        Reference< XExtendedDocumentHandler >	xExtDocHandler( mpSVGExport->GetDocHandler(), UNO_QUERY );

                        xExtDocHandler->unknown( implGetDescriptionFromShape( rxShape ) );
                    }

                    if( rMtf.GetActionCount() )
                    {
                        if( ( aShapeType.lastIndexOf( B2UCONST( "drawing.OLE2Shape" ) ) != -1 ) ||
                            ( aShapeType.lastIndexOf( B2UCONST( "drawing.GraphicObjectShape" ) ) != -1 ) )
                        {
                            SvXMLElementExport aExp2( *mpSVGExport, XML_NAMESPACE_NONE, "g", TRUE, TRUE );
                            mpSVGWriter->WriteMetaFile( aTopLeft, aSize, rMtf, SVGWRITER_WRITE_ALL);
                        }
                        else 
                        {
                            // write geometries
                            SvXMLElementExport aGeometryExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", TRUE, TRUE );
                            mpSVGWriter->WriteMetaFile( aTopLeft, aSize, rMtf, SVGWRITER_WRITE_FILL );

                            // write text separately
                            SvXMLElementExport aTextExp( *mpSVGExport, XML_NAMESPACE_NONE, "g", TRUE, TRUE );
                            mpSVGWriter->WriteMetaFile( aTopLeft, aSize, rMtf, SVGWRITER_WRITE_TEXT );
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
                                       const Reference< XDrawPages >& rxDrawPages,
                                       sal_Int32 nPageToExport )
{
    if( SVG_EXPORT_ALLPAGES == nPageToExport )
    {
        sal_Int32 i, nCount;

        for( i = 0, nCount = rxMasterPages->getCount(); i < nCount; ++i )
        {
            Reference< XDrawPage > xMasterPage;

            rxMasterPages->getByIndex( i ) >>= xMasterPage;

            if( xMasterPage.is() )
            {
                Reference< XShapes > xShapes( xMasterPage, UNO_QUERY );

                implCreateObjectsFromBackground( xMasterPage );

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
    }
    else
    {
        DBG_ASSERT( nPageToExport >= 0 && nPageToExport < rxDrawPages->getCount(),
                    "SVGFilter::implCreateObjects: invalid page number to export" );

        Reference< XDrawPage > xDrawPage;

          rxDrawPages->getByIndex( nPageToExport ) >>= xDrawPage;

          if( xDrawPage.is() )
          {
            Reference< XMasterPageTarget > xMasterTarget( xDrawPage, UNO_QUERY );

            if( xMasterTarget.is() )
            {
                Reference< XDrawPage > xMasterPage( xMasterTarget->getMasterPage() );

                if( xMasterPage.is() )
                {
                    Reference< XShapes > xShapes( xMasterPage, UNO_QUERY );

                    implCreateObjectsFromBackground( xMasterPage );

                    if( xShapes.is() )
                        implCreateObjectsFromShapes( xShapes );
                }
            }
       
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
    sal_Bool			bRet = sal_False;

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
                    GDIMetaFile	aMtf;
                    const Point	aNullPt;
                    const Size	aSize( pObj->GetCurrentBoundRect().GetSize() );

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
    Reference< XExporter >	xExporter( mxMSF->createInstance( B2UCONST( "com.sun.star.drawing.GraphicExportFilter" ) ), UNO_QUERY );
    sal_Bool				bRet = sal_False;

    if( xExporter.is() )
    {
        GDIMetaFile				aMtf;
        Reference< XFilter >	xFilter( xExporter, UNO_QUERY );

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
    OUString			aRet;
    const OUString      aShapeType( rxShape->getShapeType() );

    if( aShapeType.lastIndexOf( B2UCONST( "drawing.GroupShape" ) ) != -1 )
        aRet = B2UCONST( "Group" );
    else if( aShapeType.lastIndexOf( B2UCONST( "drawing.GraphicObjectShape" ) ) != -1 )
        aRet = B2UCONST( "Graphic" );
    else if( aShapeType.lastIndexOf( B2UCONST( "drawing.OLE2Shape" ) ) != -1 )
        aRet = B2UCONST( "OLE2" );
    else if( aShapeType.lastIndexOf( B2UCONST( "presentation.HeaderShape" ) ) != -1 )
        aRet = B2UCONST( "Header" );
    else if( aShapeType.lastIndexOf( B2UCONST( "presentation.FooterShape" ) ) != -1 )
        aRet = B2UCONST( "Footer" );
    else if( aShapeType.lastIndexOf( B2UCONST( "presentation.DateTimeShape" ) ) != -1 )
        aRet = B2UCONST( "Date/Time" );
    else if( aShapeType.lastIndexOf( B2UCONST( "presentation.SlideNumberShape" ) ) != -1 )
        aRet = B2UCONST( "Slide Number" );
    else
        aRet = B2UCONST( "Drawing" );
        
    return aRet;
}

// -----------------------------------------------------------------------------

OUString SVGFilter::implGetValidIDFromInterface( const Reference< XInterface >& rxIf )
{
    Reference< XNamed > xNamed( rxIf, UNO_QUERY );
    OUString			aRet;

    if( xNamed.is() )
        aRet = xNamed->getName().replace( ' ', '_' );

    return aRet;
}

// -----------------------------------------------------------------------------

IMPL_LINK( SVGFilter, CalcFieldHdl, EditFieldInfo*, pInfo )
{
    OUString   aRepresentation;
    bool       bFieldProcessed = false;

    if( pInfo )
    {
        static const ::rtl::OUString aHeaderText( B2UCONST( "HeaderText" ) );
        static const ::rtl::OUString aFooterText( B2UCONST( "FooterText" ) );
        static const ::rtl::OUString aDateTimeText( B2UCONST( "DateTimeText" ) );
        static const ::rtl::OUString aPageNumberText( B2UCONST( "Number" ) );
        
        const Reference< XPropertySet >	xDefaultPagePropertySet( mxDefaultPage, UNO_QUERY );
        Reference< XPropertySetInfo > 	xDefaultPagePropSetInfo( xDefaultPagePropertySet->getPropertySetInfo() );
        
        if( xDefaultPagePropSetInfo.is() )
        {
            const SvxFieldData* pField = pInfo->GetField().GetField();
            Any     			aProperty;
        
            if( pField->ISA( SvxHeaderField ) &&
                xDefaultPagePropSetInfo->hasPropertyByName( aHeaderText ) )
            {
                xDefaultPagePropertySet->getPropertyValue( aHeaderText ) >>= aRepresentation;
                bFieldProcessed = true;
            }
            else if( pField->ISA( SvxFooterField ) &&
                     xDefaultPagePropSetInfo->hasPropertyByName( aFooterText ) )
            {
                xDefaultPagePropertySet->getPropertyValue( aFooterText ) >>= aRepresentation;
                bFieldProcessed = true;
            }
            else if( pField->ISA( SvxDateTimeField ) &&
                     xDefaultPagePropSetInfo->hasPropertyByName( aDateTimeText ) )
            {
                xDefaultPagePropertySet->getPropertyValue( aDateTimeText ) >>= aRepresentation;
                bFieldProcessed = true;
            }
            else if( pField->ISA( SvxPageField ) &&
                     xDefaultPagePropSetInfo->hasPropertyByName( aPageNumberText ) )
            {
                String     aPageNumValue;
                sal_Int16  nPageNumber = 0;
                
                xDefaultPagePropertySet->getPropertyValue( aPageNumberText ) >>= nPageNumber;
                
                if( mpSdrModel )
                {  
                    bool bUpper = false;
                
                    switch( mpSdrModel->GetPageNumType() )
                    {
                        case SVX_CHARS_UPPER_LETTER:
                            aPageNumValue += (sal_Unicode)(char)( ( nPageNumber - 1 ) % 26 + 'A' );
                            break;
                        case SVX_CHARS_LOWER_LETTER:
                            aPageNumValue += (sal_Unicode)(char)( ( nPageNumber- 1 ) % 26 + 'a' );
                            break;
                        case SVX_ROMAN_UPPER:
                            bUpper = true;
                        case SVX_ROMAN_LOWER:
                            aPageNumValue += SvxNumberFormat::CreateRomanString( nPageNumber, bUpper );
                            break;
                        case SVX_NUMBER_NONE:
                            aPageNumValue.Erase();
                            aPageNumValue += sal_Unicode(' ');
                            break;
                        default : break;
                    }
                   }
                
                if( !aPageNumValue.Len() )
                    aPageNumValue += String::CreateFromInt32( nPageNumber );
                
                aRepresentation = aPageNumValue;
                bFieldProcessed = true;
            }
        }
    
        pInfo->SetRepresentation( aRepresentation );
    }
    
    return( bFieldProcessed ? 0 : maOldFieldHdl.Call( pInfo ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
