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
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
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
#include <vcl/wmf.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/gdimetafiletools.hxx>
#include <memory>

#include "swfexporter.hxx"
#include "swfwriter.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::task;
using namespace ::std;
using namespace ::swf;

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



PageInfo::PageInfo()
        : mnBackgroundID( 0 )
        , mnObjectsID( 0)
        , mnForegroundID( 0)
{
}



PageInfo::~PageInfo()
{
    vector<ShapeInfo*>::iterator aIter( maShapesVector.begin() );
    const vector<ShapeInfo*>::iterator aEnd( maShapesVector.end() );
    while( aIter != aEnd )
    {
        delete (*aIter++);
    }
}


FlashExporter::FlashExporter(
    const Reference< XComponentContext > &rxContext,

    // #i56084# variables for selection export
    const Reference< XShapes >& rxSelectedShapes,
    const Reference< XDrawPage >& rxSelectedDrawPage,

    sal_Int32 nJPEGCompressMode,
    bool bExportOLEAsJPEG)
    : mxContext(rxContext)
    // #i56084# variables for selection export
    , mxSelectedShapes(rxSelectedShapes)
    , mxSelectedDrawPage(rxSelectedDrawPage)
    , mbExportSelection(false)

    , mpWriter(nullptr)
    , mnDocWidth(0)
    , mnDocHeight(0)
    , mnJPEGcompressMode(nJPEGCompressMode)
    , mbExportOLEAsJPEG(bExportOLEAsJPEG)
    , mbPresentation(true)
    , mnPageNumber(-1)
{
    if(mxSelectedDrawPage.is() && mxSelectedShapes.is() && mxSelectedShapes->getCount())
    {
        // #i56084# determine export selection
        mbExportSelection = true;
    }
}



FlashExporter::~FlashExporter()
{
    Flush();
}

void FlashExporter::Flush()
{
    delete mpWriter;
    mpWriter = nullptr;

    maPagesMap.clear();
}



const sal_uInt16 cBackgroundDepth = 2;
const sal_uInt16 cBackgroundObjectsDepth = 3;
const sal_uInt16 cPageObjectsDepth = 4;
const sal_uInt16 cWaitButtonDepth = 10;

bool FlashExporter::exportAll( Reference< XComponent > xDoc, Reference< XOutputStream > &xOutputStream, Reference< XStatusIndicator> &xStatusIndicator )
{
    Reference< XServiceInfo > xDocServInfo( xDoc, UNO_QUERY );
    if( xDocServInfo.is() )
        mbPresentation = xDocServInfo->supportsService( "com.sun.star.presentation.PresentationDocument" );

    Reference< XDrawPagesSupplier > xDrawPagesSupplier(xDoc, UNO_QUERY);
    if(!xDrawPagesSupplier.is())
        return false;

    Reference< XIndexAccess > xDrawPages( xDrawPagesSupplier->getDrawPages(), UNO_QUERY );
    if(!xDrawPages.is())
        return false;

    Reference< XDrawPage > xDrawPage;

    // #i56084# set xDrawPage directly when exporting selection
    if(mbExportSelection)
    {
        xDrawPage = mxSelectedDrawPage;
    }
    else
    {
        xDrawPages->getByIndex(0) >>= xDrawPage;
    }

    Reference< XPropertySet > xProp( xDrawPage, UNO_QUERY );
    try
    {
        xProp->getPropertyValue( "Width" ) >>= mnDocWidth;
        xProp->getPropertyValue( "Height" )  >>= mnDocHeight;

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

    // #i56084# nPageCount is 1 when exporting selection
    const sal_Int32 nPageCount = mbExportSelection ? 1 : xDrawPages->getCount();
    sal_uInt16 nPage;

    if ( xStatusIndicator.is() )
    {
        xStatusIndicator->start("Macromedia Flash (SWF)", nPageCount);
    }

    for( nPage = 0; nPage < nPageCount; nPage++)
    {
        // #i56084# keep PageNumber? We could determine the PageNumber of the single to-be-exported page
        // when exporting the selection, but this is only used for swf internal, so no need to do so (AFAIK)
        mnPageNumber = nPage + 1;

        if ( xStatusIndicator.is() )
            xStatusIndicator->setValue( nPage );

        // #i56084# get current xDrawPage when not exporting selection; else alraedy set above
        if(!mbExportSelection)
        {
            xDrawPages->getByIndex(nPage) >>= xDrawPage;
        }

        if( !xDrawPage.is())
            continue;

        Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );
        if( mbPresentation )
        {
            bool bVisible = false;
            xPropSet->getPropertyValue( "Visible" ) >>= bVisible;
            if( !bVisible )
                continue;
        }

        // #i56084# no background when exporting selection
        if(!mbExportSelection)
        {
            exportBackgrounds( xDrawPage, nPage, false );
            exportBackgrounds( xDrawPage, nPage, true );
        }

        maPagesMap[nPage].mnForegroundID = mpWriter->startSprite();

        // #i56084# directly export selection in export selection mode
        if(mbExportSelection)
        {
            exportShapes( mxSelectedShapes, false, false );
        }
        else
        {
            exportDrawPageContents( xDrawPage, false, false );
        }

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

    return true;
}


bool FlashExporter::exportSlides( Reference< XDrawPage > xDrawPage, Reference< XOutputStream > &xOutputStream, sal_uInt16 /* nPage */ )
{
    Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );
    if( !xDrawPage.is() || !xPropSet.is() )
        return false;

    try
    {
        if( nullptr == mpWriter )
        {
            xPropSet->getPropertyValue( "Width" ) >>= mnDocWidth;
            xPropSet->getPropertyValue( "Height" ) >>= mnDocHeight;

            mpWriter = new Writer( 14400, 10800, mnDocWidth, mnDocHeight, mnJPEGcompressMode );
        }

        if( mbPresentation )
        {
            bool bVisible = false;
            xPropSet->getPropertyValue( "Visible" ) >>= bVisible;
            if( !bVisible )
                return false;
        }
    }
    catch( const Exception& )
    {
        OSL_ASSERT( false );
    }

    exportDrawPageContents(xDrawPage, true, false);

    mpWriter->storeTo( xOutputStream );

    return true;
}

sal_uInt16 FlashExporter::exportBackgrounds( Reference< XDrawPage > xDrawPage, Reference< XOutputStream > &xOutputStream, sal_uInt16 nPage, bool bExportObjects )
{
    Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );
    if( !xDrawPage.is() || !xPropSet.is() )
        return 0;

    if( nullptr == mpWriter )
    {
        xPropSet->getPropertyValue( "Width" ) >>= mnDocWidth;
        xPropSet->getPropertyValue( "Height" ) >>= mnDocHeight;

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

sal_uInt16 FlashExporter::exportBackgrounds( Reference< XDrawPage > xDrawPage, sal_uInt16 nPage, bool bExportObjects )
{
    Reference< XPropertySet > xPropSet( xDrawPage, UNO_QUERY );
    if( !xDrawPage.is() || !xPropSet.is() )
        return 0;

    bool bBackgroundVisible = true;
    bool bBackgroundObjectsVisible = true;

    if( mbPresentation )
    {
        xPropSet->getPropertyValue( "IsBackgroundVisible" ) >>= bBackgroundVisible;
        xPropSet->getPropertyValue( "IsBackgroundObjectsVisible" ) >>= bBackgroundObjectsVisible;
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

    BitmapChecksum masterchecksum = aMtfMaster.GetChecksum();
    BitmapChecksum privatechecksum = aMtfPrivate.GetChecksum();

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

    BitmapChecksum shapesum = ActionSummer(xShapes);

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



/** export's the definition of the shapes inside this drawing page and adds the
    shape infos to the current PageInfo */
void FlashExporter::exportDrawPageContents( const Reference< XDrawPage >& xPage, bool bStream, bool bMaster )
{
    Reference< XShapes > xShapes( xPage, UNO_QUERY );
    exportShapes(xShapes, bStream, bMaster);
}



/** export's the definition of the shapes inside this XShapes container and adds the
    shape infos to the current PageInfo */
void FlashExporter::exportShapes( const Reference< XShapes >& xShapes, bool bStream, bool bMaster )
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



/** export this shape definition and adds it's info to the current PageInfo */
void FlashExporter::exportShape( const Reference< XShape >& xShape, bool bMaster )
{
    Reference< XPropertySet > xPropSet( xShape, UNO_QUERY );
    if( !xPropSet.is() )
        return;

    if( mbPresentation )
    {
        try
        {
            // skip empty presentation objects
            bool bEmpty = false;
            xPropSet->getPropertyValue( "IsEmptyPresentationObject" ) >>= bEmpty;
            if( bEmpty )
                return;

            // don't export presentation placeholders on masterpage
            // they can be non empty when user edits the default texts
            if( bMaster )
            {
                OUString aShapeType( xShape->getShapeType() );
                if( aShapeType == "com.sun.star.presentation.TitleTextShape" ||
                    aShapeType == "com.sun.star.presentation.OutlinerShape"  ||
                    aShapeType == "com.sun.star.presentation.HeaderShape"    ||
                    aShapeType == "com.sun.star.presentation.FooterShape"    ||
                    aShapeType == "com.sun.star.presentation.SlideNumberShape"  ||
                    aShapeType == "com.sun.star.presentation.DateTimeShape" )
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
            css::awt::Rectangle aBoundRect;
            xPropSet->getPropertyValue( "BoundRect" ) >>= aBoundRect;

            std::unique_ptr<ShapeInfo> pShapeInfo(new ShapeInfo());
            pShapeInfo->mnX = aBoundRect.X;
            pShapeInfo->mnY = aBoundRect.Y;
            pShapeInfo->mnWidth = aBoundRect.Width;
            pShapeInfo->mnHeight = aBoundRect.Height;

            if( mbPresentation )
            {
                xPropSet->getPropertyValue( "Bookmark" ) >>= pShapeInfo->maBookmark;
                xPropSet->getPropertyValue( "DimColor" ) >>= pShapeInfo->mnDimColor;
                xPropSet->getPropertyValue( "DimHide" ) >>= pShapeInfo->mbDimHide;
                xPropSet->getPropertyValue( "DimPrevious" ) >>= pShapeInfo->mbDimPrev;
                xPropSet->getPropertyValue( "Effect" ) >>= pShapeInfo->meEffect;
                xPropSet->getPropertyValue( "PlayFull" ) >>= pShapeInfo->mbPlayFull;
                xPropSet->getPropertyValue( "PresentationOrder" ) >>= pShapeInfo->mnPresOrder;
                xPropSet->getPropertyValue( "Sound" ) >>= pShapeInfo->maSoundURL;
                xPropSet->getPropertyValue( "SoundOn" ) >>= pShapeInfo->mbSoundOn;
                xPropSet->getPropertyValue( "Speed" )  >>= pShapeInfo->meEffectSpeed;
                xPropSet->getPropertyValue( "TextEffect" )  >>= pShapeInfo->meTextEffect;
                xPropSet->getPropertyValue( "TransparentColor" )  >>= pShapeInfo->mnBlueScreenColor;
            }

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
            BitmapChecksum checksum = aMtf.GetChecksum();

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
    }
    catch( const Exception& )
    {
        OSL_ASSERT(false);
    }
}



bool FlashExporter::getMetaFile( Reference< XComponent >&xComponent, GDIMetaFile& rMtf, bool bOnlyBackground /* = false */, bool bExportAsJPEG /* = false */)
{
    if( !mxGraphicExporter.is() )
        mxGraphicExporter = GraphicExportFilter::create( mxContext );

    utl::TempFile aFile;
    aFile.EnableKillingFile();

    Sequence< PropertyValue > aFilterData(bExportAsJPEG ? 3 : 2);
    aFilterData[0].Name = "Version";
    aFilterData[0].Value <<= (sal_Int32)6000;
    aFilterData[1].Name = "PageNumber";
    aFilterData[1].Value <<= mnPageNumber;

    if(bExportAsJPEG)
    {
        aFilterData[2].Name = "Translucent";
        aFilterData[2].Value <<= true;
    }

    Sequence< PropertyValue > aDescriptor( bOnlyBackground ? 4 : 3 );
    aDescriptor[0].Name = "FilterName";

    // AS: If we've been asked to export as an image, then use the BMP filter.
    //  Otherwise, use SVM.  This is useful for things that don't convert well as
    //  metafiles, like the occasional OLE object.
    aDescriptor[0].Value <<= bExportAsJPEG ? OUString("PNG") : OUString("SVM");

    aDescriptor[1].Name = "URL";
    aDescriptor[1].Value <<= OUString(aFile.GetURL());
    aDescriptor[2].Name = "FilterData";
    aDescriptor[2].Value <<= aFilterData;
    if( bOnlyBackground )
    {
        aDescriptor[3].Name = "ExportOnlyBackground";
        aDescriptor[3].Value <<= bOnlyBackground;
    }
    mxGraphicExporter->setSourceDocument( xComponent );
    mxGraphicExporter->filter( aDescriptor );

    if (bExportAsJPEG)
    {
        Graphic aGraphic;
        GraphicFilter aFilter(false);

        aFilter.ImportGraphic( aGraphic, aFile.GetURL(), *aFile.GetStream( StreamMode::READ ) );
        BitmapEx rBitmapEx( aGraphic.GetBitmap(), Color(255,255,255) );

        Rectangle clipRect;
        for( size_t i = 0, nCount = rMtf.GetActionSize(); i < nCount; i++ )
        {
            const MetaAction*    pAction = rMtf.GetAction( i );
            const MetaActionType nType = pAction->GetType();

            switch( nType )
            {
                case( MetaActionType::ISECTRECTCLIPREGION ):
                {
                    const MetaISectRectClipRegionAction* pA = static_cast<const MetaISectRectClipRegionAction*>(pAction);
                    clipRect = pA->GetRect();
                    i = nCount;
                    break;
                }
                default: break;
            }
        }
        MetaBmpExScaleAction *pmetaAct = new MetaBmpExScaleAction(Point(clipRect.Left(), clipRect.Top()), Size(clipRect.GetWidth(), clipRect.GetHeight()), rBitmapEx);

        rMtf.Clear();
        rMtf.AddAction(pmetaAct);

    }
    else
    {
        rMtf.Read( *aFile.GetStream( StreamMode::READ ) );

        if(usesClipActions(rMtf))
        {
            // #i121267# It is necessary to prepare the metafile since the export does *not* support
            // clip regions. This tooling method clips the geometry content of the metafile internally
            // against its own clip regions, so that the export is safe to ignore clip regions
            clipMetafileContentAgainstOwnRegions(rMtf);
        }
    }

    return rMtf.GetActionSize() != 0;
}

BitmapChecksum FlashExporter::ActionSummer(Reference< XShape >& xShape)
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

BitmapChecksum FlashExporter::ActionSummer(Reference< XShapes >& xShapes)
{
    sal_uInt32 nShapeCount = xShapes->getCount();
    BitmapChecksum shapecount = 0;

    Reference< XShape > xShape2;

    for( sal_uInt32 nShape = 0; nShape < nShapeCount; nShape++ )
    {
        xShapes->getByIndex( nShape ) >>= xShape2;

        shapecount += ActionSummer(xShape2);
    }

    return shapecount;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
