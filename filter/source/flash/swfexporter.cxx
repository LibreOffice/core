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

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <vcl/gdimtf.hxx>
#include <unotools/tempfile.hxx>
#include <osl/diagnose.h>
#include <osl/file.hxx>
#include <vcl/metaact.hxx>
#include <svtools/wmf.hxx>
#include <svtools/filter.hxx>

#include "swfexporter.hxx"
#include "swfwriter.hxx"

using rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::task;
using namespace ::std;
using namespace ::swf;

using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::io::XOutputStream;
using com::sun::star::beans::PropertyValue;
using com::sun::star::container::XIndexAccess;
using com::sun::star::beans::XPropertySet;
using com::sun::star::lang::XComponent;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::document::XExporter;
using com::sun::star::document::XFilter;
using com::sun::star::frame::XModel;
using com::sun::star::lang::XServiceInfo;

// -----------------------------------------------------------------------------

PageInfo::PageInfo()
:       meFadeEffect( FadeEffect_NONE ),
        meFadeSpeed( AnimationSpeed_MEDIUM ),
        mnDuration( 0 ),
        mnChange( 0 )
{
}

// -----------------------------------------------------------------------------

PageInfo::~PageInfo()
{
    vector<ShapeInfo*>::iterator aIter( maShapesVector.begin() );
    const vector<ShapeInfo*>::iterator aEnd( maShapesVector.end() );
    while( aIter != aEnd )
    {
        delete (*aIter++);
    }
}

#ifdef THEFUTURE
// -----------------------------------------------------------------------------

void PageInfo::addShape( ShapeInfo* pShapeInfo )
{
    maShapesVector.push_back( pShapeInfo );
}
#endif

// -----------------------------------------------------------------------------

FlashExporter::FlashExporter(const Reference< XMultiServiceFactory > &rxMSF, sal_Int32 nJPEGCompressMode, sal_Bool bExportOLEAsJPEG)
:   mxMSF( rxMSF ),
    mpWriter( NULL ),
    mnJPEGcompressMode(nJPEGCompressMode),
    mbExportOLEAsJPEG(bExportOLEAsJPEG),
    mbPresentation(true),
    mnPageNumber( - 1 )
{
}

// -----------------------------------------------------------------------------

FlashExporter::~FlashExporter()
{
    Flush();
}

void FlashExporter::Flush()
{
    delete mpWriter;
    mpWriter = NULL;

    maPagesMap.clear();
}

// -----------------------------------------------------------------------------

const sal_uInt16 cBackgroundDepth = 2;
const sal_uInt16 cBackgroundObjectsDepth = 3;
const sal_uInt16 cPageObjectsDepth = 4;
const sal_uInt16 cWaitButtonDepth = 10;

sal_Bool FlashExporter::exportAll( Reference< XComponent > xDoc, Reference< XOutputStream > &xOutputStream, Reference< XStatusIndicator> &xStatusIndicator )
{
    Reference< XServiceInfo > xDocServInfo( xDoc, UNO_QUERY );
    if( xDocServInfo.is() )
        mbPresentation = xDocServInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.PresentationDocument"))) ;

    Reference< XDrawPagesSupplier > xDrawPagesSupplier(xDoc, UNO_QUERY);
    if(!xDrawPagesSupplier.is())
        return sal_False;

    Reference< XIndexAccess > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY );
    if(!xDrawPages.is())
        return sal_False;

    Reference< XDrawPage > xDrawPage;
    xDrawPages->getByIndex(0) >>= xDrawPage;

    Reference< XPropertySet > xProp( xDrawPage, UNO_QUERY );
    try
    {
        xProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Width") ) ) >>= mnDocWidth;
        xProp->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Height") ) ) >>= mnDocHeight;

        sal_Int32 nOutputWidth = 14400;
        sal_Int32 nOutputHeight = (nOutputWidth * mnDocHeight ) / mnDocWidth;
        delete mpWriter;
        mpWriter = new Writer( nOutputWidth, nOutputHeight, mnDocWidth, mnDocHeight, mnJPEGcompressMode  );
    }
    catch( const Exception& )
    {
        OSL_ASSERT( false );
        return false; // no writer, no cookies
    }

    const sal_Int32 nPageCount = xDrawPages->getCount();
    sal_uInt16 nPage;
    if ( xStatusIndicator.is() )
        xStatusIndicator->start(OUString( RTL_CONSTASCII_USTRINGPARAM( "Macromedia Flash (SWF)" )), nPageCount);
    for( nPage = 0; nPage < nPageCount; nPage++)
    {
        mnPageNumber = nPage + 1;

        if ( xStatusIndicator.is() )
            xStatusIndicator->setValue( nPage );
        xDrawPages->getByIndex(nPage) >>= xDrawPage;

        if( !xDrawPage.is())
            continue;

        Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );
        if( mbPresentation )
        {
            sal_Bool bVisible = sal_False;
            xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Visible") ) ) >>= bVisible;
            if( !bVisible )
                continue;
        }

        exportBackgrounds( xDrawPage, nPage, false );
        exportBackgrounds( xDrawPage, nPage, true );

        maPagesMap[nPage].mnForegroundID = mpWriter->startSprite();
        exportDrawPageContents( xDrawPage, false, false );
        mpWriter->endSprite();

        // AS: If the background is different than the previous slide,
        //  we have to remove the old one and place the new one.
        if (nPage)
        {
            if (maPagesMap[nPage].mnBackgroundID != maPagesMap[nPage-1].mnBackgroundID)
            {
                mpWriter->removeShape(cBackgroundDepth);
                mpWriter->placeShape( maPagesMap[nPage].mnBackgroundID, cBackgroundDepth, 0, 0 );
            }

            if (maPagesMap[nPage].mnObjectsID != maPagesMap[nPage-1].mnObjectsID)
            {
                mpWriter->removeShape(cBackgroundObjectsDepth);
                mpWriter->placeShape( maPagesMap[nPage].mnObjectsID, cBackgroundObjectsDepth, 0, 0 );
            }

            // AS: Remove the Foreground of the previous slide.
            mpWriter->removeShape(cPageObjectsDepth);
        }
        else
        {
            mpWriter->placeShape( maPagesMap[nPage].mnBackgroundID, cBackgroundDepth, 0, 0 );
            mpWriter->placeShape( maPagesMap[nPage].mnObjectsID, cBackgroundObjectsDepth, 0, 0 );
        }

        mpWriter->placeShape( maPagesMap[nPage].mnForegroundID, cPageObjectsDepth, 0, 0 );

        mpWriter->waitOnClick( cWaitButtonDepth );
        mpWriter->showFrame();
    }

    mpWriter->removeShape( cBackgroundDepth );
    mpWriter->removeShape( cBackgroundObjectsDepth );
    mpWriter->removeShape( cPageObjectsDepth );
    mpWriter->gotoFrame( 0 );
    mpWriter->showFrame();

    mpWriter->storeTo( xOutputStream );

    return sal_True;
}


sal_Bool FlashExporter::exportSlides( Reference< XDrawPage > xDrawPage, Reference< XOutputStream > &xOutputStream, sal_uInt16 /* nPage */ )
{
    Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );
    if( !xDrawPage.is() || !xPropSet.is() )
        return sal_False;

    try
    {
        if( NULL == mpWriter )
        {
            xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Width") ) ) >>= mnDocWidth;
            xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Height") ) ) >>= mnDocHeight;

            mpWriter = new Writer( 14400, 10800, mnDocWidth, mnDocHeight, mnJPEGcompressMode );
        }

        if( mbPresentation )
        {
            sal_Bool bVisible = sal_False;
            xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Visible") ) ) >>= bVisible;
            if( !bVisible )
                return sal_False;
        }
    }
    catch( const Exception& )
    {
        OSL_ASSERT( false );
    }

    exportDrawPageContents(xDrawPage, true, false);

    mpWriter->storeTo( xOutputStream );

    return sal_True;
}

sal_uInt16 FlashExporter::exportBackgrounds( Reference< XDrawPage > xDrawPage, Reference< XOutputStream > &xOutputStream, sal_uInt16 nPage, sal_Bool bExportObjects )
{
    Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );
    if( !xDrawPage.is() || !xPropSet.is() )
        return sal_False;

    if( NULL == mpWriter )
    {
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Width") ) ) >>= mnDocWidth;
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Height") ) ) >>= mnDocHeight;

        mpWriter = new Writer( 14400, 10800, mnDocWidth, mnDocHeight, mnJPEGcompressMode );
    }

    sal_uInt16 ret = exportBackgrounds(xDrawPage, nPage, bExportObjects);

    if (ret != nPage)
        return ret;

    if (bExportObjects)
        mpWriter->placeShape( maPagesMap[nPage].mnObjectsID, _uInt16(1), 0, 0 );
    else
        mpWriter->placeShape( maPagesMap[nPage].mnBackgroundID, _uInt16(0), 0, 0 );

    mpWriter->storeTo( xOutputStream );

    return nPage;
}

sal_uInt16 FlashExporter::exportBackgrounds( Reference< XDrawPage > xDrawPage, sal_uInt16 nPage, sal_Bool bExportObjects )
{
    Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );
    if( !xDrawPage.is() || !xPropSet.is() )
        return sal_False;

    sal_Bool bBackgroundVisible = true;
    sal_Bool bBackgroundObjectsVisible = true;

    if( mbPresentation )
    {
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("IsBackgroundVisible") ) ) >>= bBackgroundVisible;
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("IsBackgroundObjectsVisible") ) ) >>= bBackgroundObjectsVisible;
    }


    if (bExportObjects)
    {
        if (bBackgroundObjectsVisible)
        {
            Reference< XMasterPageTarget > xMasterPageTarget( xDrawPage, UNO_QUERY );
            if( !xMasterPageTarget.is() )
            {
                maPagesMap[nPage].mnObjectsID = 0xffff;
                return 0xffff;
            }
            Reference<XDrawPage> aTemp = xMasterPageTarget->getMasterPage();
            sal_uInt16 ret = exportMasterPageObjects(nPage, aTemp);
            if (ret != nPage)
                return ret;
        }
        else
        {
            maPagesMap[nPage].mnObjectsID = 0xffff;
            return 0xffff;
        }
    }
    else
    {
        if (bBackgroundVisible)
        {
            sal_uInt16 ret = exportDrawPageBackground(nPage, xDrawPage);

            if (ret != nPage)
                return ret;
        }
        else
        {
            maPagesMap[nPage].mnBackgroundID = 0xffff;
            return 0xffff;
        }
    }

    return nPage;
}

#ifdef AUGUSTUS
sal_Bool FlashExporter::exportSound( Reference< XOutputStream > &xOutputStream, const char* wavfilename )
{
    try
    {
        delete mpWriter;
        mpWriter = new Writer( 0, 0, 0, 0 );
    }
    catch( const Exception& )
    {
        OSL_ASSERT( false );
    }

    if (!mpWriter->streamSound(wavfilename))
        return sal_False;
    else
        mpWriter->storeTo( xOutputStream );

    return sal_True;
}
#endif // defined AUGUSTUS

// -----------------------------------------------------------------------------

sal_Int32 nPlaceDepth;
// AS: A Slide can have a private background or use its masterpage's background.
//  We use the checksums on the metafiles to tell if backgrounds are the same and
//  should be reused.  The return value indicates which slide's background to use.
//  If the return value != nPage, then there is no background (if == -1) or the
//  background has already been exported.
sal_uInt16 FlashExporter::exportDrawPageBackground(sal_uInt16 nPage, Reference< XDrawPage >& xPage)
{
    sal_uInt16 rBackgroundID;

    GDIMetaFile aMtfPrivate, aMtfMaster;
    Reference< XComponent > xComponent( xPage, UNO_QUERY );

    Reference< XMasterPageTarget > xMasterPageTarget( xPage, UNO_QUERY );
    if( !xMasterPageTarget.is() )
        return 0xffff;

    Reference< XDrawPage > xMasterPage = xMasterPageTarget->getMasterPage();
    if( !xMasterPage.is())
        return 0xffff;

    Reference< XComponent > xCompMaster( xMasterPage, UNO_QUERY );

    getMetaFile( xCompMaster, aMtfMaster, true );
    getMetaFile( xComponent, aMtfPrivate, true );

    sal_uInt32 masterchecksum = aMtfMaster.GetChecksum();
    sal_uInt32 privatechecksum = aMtfPrivate.GetChecksum();

    // AS: If the slide has its own background
    if (privatechecksum)
    {
        ChecksumCache::iterator it = gPrivateCache.find(privatechecksum);

        // AS: and we've previously encountered this background, just return
        //  the previous index.
        if (gPrivateCache.end() != it)
        {
            maPagesMap[nPage].mnBackgroundID =
                maPagesMap[it->second].mnBackgroundID;
            return it->second;
        }
        else
        {
            // AS: Otherwise, cache this checksum.
            gPrivateCache[privatechecksum] = nPage;

            rBackgroundID = mpWriter->defineShape( aMtfPrivate );

            maPagesMap[nPage].mnBackgroundID = rBackgroundID;
            return nPage;
        }
    }

    // AS: Ok, no private background.  Use the master page's.
    // AS: Have we already exported this master page?
    ChecksumCache::iterator it = gMasterCache.find(masterchecksum);

    if (gMasterCache.end() != it)
    {
        maPagesMap[nPage].mnBackgroundID =
            maPagesMap[it->second].mnBackgroundID;

        return it->second;                // AS: Yes, so don't export it again.
    }

    gMasterCache[masterchecksum] = nPage;

    rBackgroundID = mpWriter->defineShape( aMtfMaster );

    maPagesMap[nPage].mnBackgroundID = rBackgroundID;

    return nPage;
}

sal_uInt16 FlashExporter::exportMasterPageObjects(sal_uInt16 nPage, Reference< XDrawPage >& xMasterPage)
{
    Reference< XShapes > xShapes( xMasterPage, UNO_QUERY );

    sal_uInt32 shapesum = ActionSummer(xShapes);

    ChecksumCache::iterator it = gObjectCache.find(shapesum);

    if (gObjectCache.end() != it)
    {
        maPagesMap[nPage].mnObjectsID =
            maPagesMap[it->second].mnObjectsID;

        return it->second;                // AS: Yes, so don't export it again.
    }

    gObjectCache[shapesum] = nPage;

    sal_uInt16 rObjectsID = mpWriter->startSprite();
    exportDrawPageContents( xMasterPage, false, true );
    mpWriter->endSprite();

    maPagesMap[nPage].mnObjectsID = rObjectsID;

    return nPage;
}

// -----------------------------------------------------------------------------

/** export's the definition of the shapes inside this drawing page and adds the
    shape infos to the current PageInfo */
void FlashExporter::exportDrawPageContents( Reference< XDrawPage >& xPage, bool bStream, bool bMaster )
{
    Reference< XShapes > xShapes( xPage, UNO_QUERY );
    exportShapes(xShapes, bStream, bMaster);
}

// -----------------------------------------------------------------------------

/** export's the definition of the shapes inside this XShapes container and adds the
    shape infos to the current PageInfo */
void FlashExporter::exportShapes( Reference< XShapes >& xShapes, bool bStream, bool bMaster )
{
    OSL_ENSURE( (xShapes->getCount() <= 0xffff), "overflow in FlashExporter::exportDrawPageContents()" );

    sal_uInt16 nShapeCount = (sal_uInt16)min( xShapes->getCount(), (sal_Int32)0xffff );
    sal_uInt16 nShape;

    Reference< XShape > xShape;

    for( nShape = 0; nShape < nShapeCount; nShape++ )
    {
        xShapes->getByIndex( nShape ) >>= xShape;

        if( xShape.is() )
        {
            Reference< XShapes > xShapes2( xShape, UNO_QUERY );
            if( xShapes2.is() && xShape->getShapeType() == "com.sun.star.drawing.GroupShape" )
                // export the contents of group shapes, but we only ever stream at the top
                // recursive level anyway, so pass false for streaming.
                exportShapes( xShapes2, false, bMaster);
            else
                exportShape( xShape, bMaster);
        }

        if (bStream)
            mpWriter->showFrame();
    }
}

// -----------------------------------------------------------------------------

/** export this shape definition and adds it's info to the current PageInfo */
void FlashExporter::exportShape( Reference< XShape >& xShape, bool bMaster )
{
    Reference< XPropertySet > xPropSet( xShape, UNO_QUERY );
    if( !xPropSet.is() )
        return;

    if( mbPresentation )
    {
        try
        {
            // skip empty presentation objects
            sal_Bool bEmpty = sal_False;
            xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ) ) >>= bEmpty;
            if( bEmpty )
                return;

            // don't export presentation placeholders on masterpage
            // they can be non empty when user edits the default texts
            if( bMaster )
            {
                OUString aShapeType( xShape->getShapeType() );
                if( (0 == aShapeType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.presentation.TitleTextShape" ))) ||
                    (0 == aShapeType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.presentation.OutlinerShape" ))) ||
                    (0 == aShapeType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.presentation.HeaderShape" ))) ||
                    (0 == aShapeType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.presentation.FooterShape" ))) ||
                    (0 == aShapeType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.presentation.SlideNumberShape" ))) ||
                    (0 == aShapeType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.presentation.DateTimeShape" ))))
                    return;
            }
        }
        catch( const Exception& )
        {
            // TODO: If we are exporting a draw, this property is not available
        }
    }

    try
    {
            com::sun::star::awt::Rectangle aBoundRect;
            xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("BoundRect") ) ) >>= aBoundRect;

            ShapeInfo* pShapeInfo = new ShapeInfo();
            pShapeInfo->mnX = aBoundRect.X;
            pShapeInfo->mnY = aBoundRect.Y;
            pShapeInfo->mnWidth = aBoundRect.Width;
            pShapeInfo->mnHeight = aBoundRect.Height;

            if( mbPresentation )
            {
                xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Bookmark") ) ) >>= pShapeInfo->maBookmark;
                xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("DimColor") ) ) >>= pShapeInfo->mnDimColor;
                xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("DimHide") ) ) >>= pShapeInfo->mbDimHide;
                xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("DimPrevious") ) ) >>= pShapeInfo->mbDimPrev;
                xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Effect") ) ) >>= pShapeInfo->meEffect;
                xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("PlayFull") ) ) >>= pShapeInfo->mbPlayFull;
                xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("PresentationOrder") ) ) >>= pShapeInfo->mnPresOrder;
                xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Sound") ) ) >>= pShapeInfo->maSoundURL;
                xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("SoundOn") ) ) >>= pShapeInfo->mbSoundOn;
                xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Speed") ) ) >>= pShapeInfo->meEffectSpeed;
                xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("TextEffect") ) ) >>= pShapeInfo->meTextEffect;
                xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("TransparentColor") ) ) >>= pShapeInfo->mnBlueScreenColor;
            }

//          long ZOrder;
//          xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("ZOrder") ) ) >>= ZOrder;

            GDIMetaFile     aMtf;
            Reference< XComponent > xComponent( xShape, UNO_QUERY );

            bool bIsOleObject = xShape->getShapeType() == "com.sun.star.presentation.OLE2Shape" || xShape->getShapeType() == "com.sun.star.drawing.OLE2Shape";

            getMetaFile( xComponent, aMtf );

            // AS: If it's an OLE object, then export a JPEG if the user requested.
            //  In this case, we use the bounding rect info generated in the first getMetaFile
            //  call, and then clear the metafile and add a BMP action.  This may be turned into
            //  a JPEG, depending on what gives the best compression.
            if (bIsOleObject && mbExportOLEAsJPEG)
                getMetaFile( xComponent, aMtf, false, true );

            sal_uInt16 nID;
            sal_uInt32 checksum = aMtf.GetChecksum();

            ChecksumCache::iterator it = gMetafileCache.find(checksum);

            if (gMetafileCache.end() != it)
                nID = it->second;
            else
            {
                nID = mpWriter->defineShape( aMtf );
                gMetafileCache[checksum] = nID;
            }

            if (!nID)
                return;

            pShapeInfo->mnID = nID;

//          pPageInfo->addShape( pShapeInfo );

            mpWriter->placeShape( pShapeInfo->mnID, _uInt16(nPlaceDepth++), pShapeInfo->mnX, pShapeInfo->mnY );

            delete pShapeInfo;
    }
    catch( const Exception& )
    {
        OSL_ASSERT(false);
    }
}

// -----------------------------------------------------------------------------

bool FlashExporter::getMetaFile( Reference< XComponent >&xComponent, GDIMetaFile& rMtf, bool bOnlyBackground /* = false */, bool bExportAsJPEG /* = false */)
{
    if( !mxGraphicExporter.is() )
        mxGraphicExporter = Reference< XExporter >::query( mxMSF->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GraphicExportFilter") ) ) );

    Reference< XFilter > xFilter( mxGraphicExporter, UNO_QUERY );

    utl::TempFile aFile;
    aFile.EnableKillingFile();

    Sequence< PropertyValue > aFilterData(bExportAsJPEG ? 3 : 2);
    aFilterData[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("Version") );
    aFilterData[0].Value <<= (sal_Int32)6000;
    aFilterData[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("PageNumber") );
    aFilterData[1].Value <<= mnPageNumber;

    if(bExportAsJPEG)
    {
        aFilterData[2].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("Translucent") );
        aFilterData[2].Value <<= (sal_Bool)sal_True;
    }

    Sequence< PropertyValue > aDescriptor( bOnlyBackground ? 4 : 3 );
    aDescriptor[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("FilterName") );

    // AS: If we've been asked to export as an image, then use the BMP filter.
    //  Otherwise, use SVM.  This is useful for things that don't convert well as
    //  metafiles, like the occasional OLE object.
    aDescriptor[0].Value <<= OUString( RTL_CONSTASCII_USTRINGPARAM(bExportAsJPEG ? "PNG" : "SVM") );

    aDescriptor[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("URL") );
    aDescriptor[1].Value <<= OUString( aFile.GetURL() );
    aDescriptor[2].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("FilterData") );
    aDescriptor[2].Value <<= aFilterData;
    if( bOnlyBackground )
    {
        aDescriptor[3].Name = OUString( RTL_CONSTASCII_USTRINGPARAM("ExportOnlyBackground") );
        aDescriptor[3].Value <<= (sal_Bool)bOnlyBackground;
    }
    mxGraphicExporter->setSourceDocument( xComponent );
    xFilter->filter( aDescriptor );

    if (bExportAsJPEG)
    {
        Graphic aGraphic;
        GraphicFilter aFilter(false);

        aFilter.ImportGraphic( aGraphic, String(aFile.GetURL()), *aFile.GetStream( STREAM_READ ) );
        BitmapEx rBitmapEx( aGraphic.GetBitmap(), Color(255,255,255) );

        Rectangle clipRect;
        for( size_t i = 0, nCount = rMtf.GetActionSize(); i < nCount; i++ )
        {
            const MetaAction*   pAction = rMtf.GetAction( i );
            const sal_uInt16    nType = pAction->GetType();

            switch( nType )
            {
                case( META_ISECTRECTCLIPREGION_ACTION ):
                {
                    const MetaISectRectClipRegionAction* pA = (const MetaISectRectClipRegionAction*) pAction;
                    clipRect = pA->GetRect();
                    i = nCount;
                    break;
                }
            }
        }
        MetaBmpExScaleAction *pmetaAct = new MetaBmpExScaleAction(Point(clipRect.Left(), clipRect.Top()), Size(clipRect.GetWidth(), clipRect.GetHeight()), rBitmapEx);

        rMtf.Clear();
        rMtf.AddAction(pmetaAct);

    }
    else
        rMtf.Read( *aFile.GetStream( STREAM_READ ) );

    return rMtf.GetActionSize() != 0;
}

sal_uInt32 FlashExporter::ActionSummer(Reference< XShape >& xShape)
{
    Reference< XShapes > xShapes( xShape, UNO_QUERY );

    if( xShapes.is() )
    {
        return ActionSummer(xShapes);
    }
    else
    {
        Reference< XComponent > xComponentShape( xShape, UNO_QUERY );

        GDIMetaFile aMtf;
        getMetaFile( xComponentShape, aMtf);

        return aMtf.GetChecksum();
    }
}

sal_uInt32 FlashExporter::ActionSummer(Reference< XShapes >& xShapes)
{
    sal_uInt32 nShapeCount = xShapes->getCount();
    sal_uInt32 shapecount = 0;

    Reference< XShape > xShape2;

    for( sal_uInt16 nShape = 0; nShape < nShapeCount; nShape++ )
    {
        xShapes->getByIndex( nShape ) >>= xShape2;

        shapecount += ActionSummer(xShape2);
    }

    return shapecount;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
