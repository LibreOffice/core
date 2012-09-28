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


#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <ucbhelper/content.hxx>
#include <tools/resmgr.hxx>
#include <tools/urlobj.hxx>
#include <svl/solar.hrc>
#include <svl/urihelper.hxx>
#include <svtools/filter.hxx>
#include <svl/itempool.hxx>
#include <sfx2/docfile.hxx>
#include <avmedia/mediawindow.hxx>
#include <vcl/svapp.hxx>

#include "svtools/filter.hxx"
#include <svx/svdpage.hxx>
#include <svx/svdograf.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmview.hxx>
#include <svx/unomodel.hxx>
#include "codec.hxx"
#include "gallery.hrc"
#include "svx/gallery1.hxx"
#include "svx/galtheme.hxx"
#include "svx/galmisc.hxx"
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>

// --------------
// - Namespaces -
// --------------

using namespace ::rtl;
using namespace ::com::sun::star;

// ----------
// - ResMgr -
// ----------

ResMgr* GetGalleryResMgr()
{
    static ResMgr* pGalleryResMgr = NULL;

    if( !pGalleryResMgr )
    {
        pGalleryResMgr = ResMgr::CreateResMgr(
            "gal", Application::GetSettings().GetUILocale() );
    }

    return pGalleryResMgr;
}

// -------------------------
// - GalleryResGetBitmapEx -
// -------------------------

BitmapEx GalleryResGetBitmapEx( sal_uInt32 nId )
{
    BitmapEx aBmpEx( GAL_RES( nId ) );

    if( !aBmpEx.IsTransparent() )
            aBmpEx = BitmapEx( aBmpEx.GetBitmap(), COL_LIGHTMAGENTA );

    return aBmpEx;
}

// ----------------------
// - SgaUserDataFactory -
// ----------------------

IMPL_LINK( SgaUserDataFactory, MakeUserData, SdrObjFactory*, pObjFactory )
{
    if ( pObjFactory->nInventor == IV_IMAPINFO && pObjFactory->nIdentifier == ID_IMAPINFO )
        pObjFactory->pNewData = new SgaIMapInfo;

    return 0L;
}

// ------------------------
// - GalleryGraphicImport -
// ------------------------

sal_uInt16 GalleryGraphicImport( const INetURLObject& rURL, Graphic& rGraphic,
                             String& rFilterName, sal_Bool bShowProgress )
{
    sal_uInt16      nRet = SGA_IMPORT_NONE;
    SfxMedium   aMedium( rURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );
    String      aFilterName;

    aMedium.DownLoad();

    SvStream* pIStm = aMedium.GetInStream();

    if( pIStm )
    {
        GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
        GalleryProgress*    pProgress = bShowProgress ? new GalleryProgress( &rGraphicFilter ) : NULL;
        sal_uInt16              nFormat;

        if( !rGraphicFilter.ImportGraphic( rGraphic, rURL.GetMainURL( INetURLObject::NO_DECODE ), *pIStm, GRFILTER_FORMAT_DONTKNOW, &nFormat ) )
        {
            rFilterName = rGraphicFilter.GetImportFormatName( nFormat );
            nRet = SGA_IMPORT_FILE;
        }

        delete pProgress;
    }

    return nRet;
}

// -----------------------
// - GallerySvDrawImport -
// -----------------------

sal_Bool GallerySvDrawImport( SvStream& rIStm, SdrModel& rModel )
{
    sal_uInt32  nVersion;
    sal_Bool    bRet = sal_False;

    if( GalleryCodec::IsCoded( rIStm, nVersion ) )
    {
        SvMemoryStream  aMemStm( 65535, 65535 );
        GalleryCodec    aCodec( rIStm );

        aCodec.Read( aMemStm );
        aMemStm.Seek( 0UL );

        if( 1 == nVersion )
        {
            OSL_FAIL( "staroffice binary file formats are no longer supported inside the gallery!" );
            bRet = false;
        }
        else if( 2 == nVersion )
        {
            // recall to read as XML
            bRet = GallerySvDrawImport( aMemStm, rModel );
        }
     }
    else
    {
        // read as XML
        uno::Reference< io::XInputStream > xInputStream( new utl::OInputStreamWrapper( rIStm ) );

        rModel.GetItemPool().SetDefaultMetric( SFX_MAPUNIT_100TH_MM );
        uno::Reference< lang::XComponent > xComponent;

        bRet = SvxDrawingLayerImport( &rModel, xInputStream, xComponent, "com.sun.star.comp.Draw.XMLOasisImporter" );
        if( !bRet || (rModel.GetPageCount() == 0) )
        {
            rIStm.Seek(0);
            bRet = SvxDrawingLayerImport( &rModel, xInputStream, xComponent, "com.sun.star.comp.Draw.XMLImporter" );
        }

    }

    return bRet;
}

// ---------------------
// - CreateIMapGraphic -
// ---------------------

sal_Bool CreateIMapGraphic( const FmFormModel& rModel, Graphic& rGraphic, ImageMap& rImageMap )
{
    sal_Bool bRet = sal_False;

    if ( rModel.GetPageCount() )
    {
        const SdrPage*      pPage = rModel.GetPage( 0 );
        const SdrObject*    pObj = pPage->GetObj( 0 );

        if ( pPage->GetObjCount() == 1 && pObj->ISA( SdrGrafObj ) )
        {
            const sal_uInt16 nCount = pObj->GetUserDataCount();

            // gibt es in den User-Daten eine IMap-Information?
            for ( sal_uInt16 i = 0; i < nCount; i++ )
            {
                const SdrObjUserData* pUserData = pObj->GetUserData( i );

                if ( ( pUserData->GetInventor() == IV_IMAPINFO ) && ( pUserData->GetId() == ID_IMAPINFO ) )
                {
                    rGraphic = ( (SdrGrafObj*) pObj )->GetGraphic();
                    rImageMap = ( (SgaIMapInfo*) pUserData )->GetImageMap();
                    bRet = sal_True;
                    break;
                }
            }
        }
    }

    return bRet;
}

// --------------------
// - GetReducedString -
// --------------------

String GetReducedString( const INetURLObject& rURL, sal_uIntPtr nMaxLen )
{
    String aReduced( rURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) );

    aReduced = aReduced.GetToken( comphelper::string::getTokenCount(aReduced, '/') - 1, '/' );

    if( INET_PROT_PRIV_SOFFICE != rURL.GetProtocol() )
    {
        sal_Unicode     aDelimiter;
           const String    aPath( rURL.getFSysPath( INetURLObject::FSYS_DETECT, &aDelimiter ) );
        const String    aName( aReduced );

        if( aPath.Len() > nMaxLen )
        {
            aReduced = aPath.Copy( 0, (sal_uInt16)( nMaxLen - aName.Len() - 4 ) );
            aReduced += String( RTL_CONSTASCII_USTRINGPARAM( "..." ) );
            aReduced += aDelimiter;
            aReduced += aName;
        }
        else
            aReduced = aPath;
    }

    return aReduced;
}

// -----------------------------------------------------------------------------

String GetSvDrawStreamNameFromURL( const INetURLObject& rSvDrawObjURL )
{
    String aRet;

    if( rSvDrawObjURL.GetProtocol() == INET_PROT_PRIV_SOFFICE &&
        comphelper::string::getTokenCount(rSvDrawObjURL.GetMainURL( INetURLObject::NO_DECODE ), '/') == 3 )
    {
        aRet = String(rSvDrawObjURL.GetMainURL( INetURLObject::NO_DECODE )).GetToken( 2, '/' );
    }

    return aRet;
}

// -----------------------------------------------------------------------------

sal_Bool FileExists( const INetURLObject& rURL )
{
    sal_Bool bRet = sal_False;

    if( rURL.GetProtocol() != INET_PROT_NOT_VALID )
    {
        try
        {
            ::ucbhelper::Content        aCnt( rURL.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
            OUString    aTitle;

            aCnt.getPropertyValue( OUString("Title") ) >>= aTitle;
            bRet = ( !aTitle.isEmpty() );
        }
        catch( const ucb::ContentCreationException& )
        {
        }
        catch( const uno::RuntimeException& )
        {
        }
        catch( const uno::Exception& )
        {
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool CreateDir( const INetURLObject& rURL )
{
    sal_Bool bRet = FileExists( rURL );

    if( !bRet )
    {
        try
        {
            uno::Reference< ucb::XCommandEnvironment >  aCmdEnv;
            INetURLObject                           aNewFolderURL( rURL );
            INetURLObject                           aParentURL( aNewFolderURL ); aParentURL.removeSegment();
            ::ucbhelper::Content                    aParent( aParentURL.GetMainURL( INetURLObject::NO_DECODE ), aCmdEnv, comphelper::getProcessComponentContext() );
            uno::Sequence< OUString >               aProps( 1 );
            uno::Sequence< uno::Any >               aValues( 1 );

            aProps.getArray()[ 0 ] = OUString("Title");
            aValues.getArray()[ 0 ] = uno::makeAny( OUString( aNewFolderURL.GetName() ) );

        ::ucbhelper::Content aContent( aNewFolderURL.GetMainURL( INetURLObject::NO_DECODE ), aCmdEnv, comphelper::getProcessComponentContext() );
        bRet = aParent.insertNewContent( OUString("application/vnd.sun.staroffice.fsys-folder"), aProps, aValues, aContent );
        }
        catch( const ucb::ContentCreationException& )
        {
        }
        catch( const uno::RuntimeException& )
        {
        }
        catch( const uno::Exception& )
        {
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool CopyFile(  const INetURLObject& rSrcURL, const INetURLObject& rDstURL )
{
    sal_Bool bRet = sal_False;

    try
    {
        ::ucbhelper::Content aDestPath( rDstURL.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );

        aDestPath.executeCommand( OUString("transfer"),
                                  uno::makeAny( ucb::TransferInfo( sal_False, rSrcURL.GetMainURL( INetURLObject::NO_DECODE ),
                                                rDstURL.GetName(), ucb::NameClash::OVERWRITE ) ) );
        bRet = sal_True;
    }
    catch( const ucb::ContentCreationException& )
    {
    }
    catch( const uno::RuntimeException& )
    {
    }
    catch( const uno::Exception& )
    {
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool KillFile( const INetURLObject& rURL )
{
    sal_Bool bRet = FileExists( rURL );

    if( bRet )
    {
        try
        {
            ::ucbhelper::Content aCnt( rURL.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
            aCnt.executeCommand( OUString("delete"), uno::makeAny( sal_Bool( sal_True ) ) );
        }
        catch( const ucb::ContentCreationException& )
        {
            bRet = sal_False;
        }
        catch( const uno::RuntimeException& )
        {
            bRet = sal_False;
        }
        catch( const uno::Exception& )
        {
            bRet = sal_False;
        }
    }

    return bRet;
}

// -------------------
// - GalleryProgress -
// -------------------

GalleryProgress::GalleryProgress( GraphicFilter* pFilter ) :
    mpFilter( pFilter )
{
    uno::Reference< lang::XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );

    if( xMgr.is() )
    {
        uno::Reference< awt::XProgressMonitor > xMonitor( xMgr->createInstance(
                                                      ::rtl::OUString("com.sun.star.awt.XProgressMonitor") ),
                                                      uno::UNO_QUERY );

        if ( xMonitor.is() )
        {
            mxProgressBar = uno::Reference< awt::XProgressBar >( xMonitor, uno::UNO_QUERY );

            if( mxProgressBar.is() )
            {
                String aProgressText;

                if( mpFilter )
                {
                    aProgressText = GAL_RESSTR(RID_SVXSTR_GALLERY_FILTER);
//                  mpFilter->SetUpdatePercentHdl( LINK( this, GalleryProgress, Update ) );     // sj: progress wasn't working up from SO7 at all
//                                                                                              // so I am removing this. The gallery progress should
//                                                                                              // be changed to use the XStatusIndicator instead of XProgressMonitor
                }
                else
                    aProgressText = String( RTL_CONSTASCII_USTRINGPARAM( "Gallery" ) );

                xMonitor->addText( String( RTL_CONSTASCII_USTRINGPARAM( "Gallery" ) ), aProgressText, sal_False ) ;
                mxProgressBar->setRange( 0, GALLERY_PROGRESS_RANGE );
            }
        }
    }
}

// ------------------------------------------------------------------------

GalleryProgress::~GalleryProgress()
{
}

// ------------------------------------------------------------------------

void GalleryProgress::Update( sal_uIntPtr nVal, sal_uIntPtr nMaxVal )
{
    if( mxProgressBar.is() && nMaxVal )
        mxProgressBar->setValue( Min( (sal_uIntPtr)( (double) nVal / nMaxVal * GALLERY_PROGRESS_RANGE ), (sal_uIntPtr) GALLERY_PROGRESS_RANGE ) );
}

// -----------------------
// - GalleryTransferable -
// -----------------------

GalleryTransferable::GalleryTransferable( GalleryTheme* pTheme, sal_uIntPtr nObjectPos, bool bLazy ) :
    mpTheme( pTheme ),
    meObjectKind( mpTheme->GetObjectKind( nObjectPos ) ),
    mnObjectPos( nObjectPos ),
    mpGraphicObject( NULL ),
    mpImageMap( NULL ),
    mpURL( NULL )
{
    InitData( bLazy );
}

// ------------------------------------------------------------------------

GalleryTransferable::~GalleryTransferable()
{
}

// ------------------------------------------------------------------------

void GalleryTransferable::InitData( bool bLazy )
{
    switch( meObjectKind )
    {
        case( SGA_OBJ_SVDRAW ):
        {
            if( !bLazy )
            {
                if( !mpGraphicObject )
                {
                    Graphic aGraphic;

                    if( mpTheme->GetGraphic( mnObjectPos, aGraphic ) )
                        mpGraphicObject = new GraphicObject( aGraphic );
                }

                if( !mxModelStream.Is() )
                {
                    mxModelStream = new SotStorageStream( String() );
                    mxModelStream->SetBufferSize( 16348 );

                    if( !mpTheme->GetModelStream( mnObjectPos, mxModelStream ) )
                        mxModelStream.Clear();
                    else
                        mxModelStream->Seek( 0 );
                }
            }
        }
        break;

        case( SGA_OBJ_ANIM ):
        case( SGA_OBJ_BMP ):
        case( SGA_OBJ_INET ):
        case( SGA_OBJ_SOUND ):
        {
            if( !mpURL )
            {
                mpURL = new INetURLObject;

                if( !mpTheme->GetURL( mnObjectPos, *mpURL ) )
                    delete mpURL, mpURL = NULL;
            }

            if( ( SGA_OBJ_SOUND != meObjectKind ) && !mpGraphicObject )
            {
                Graphic aGraphic;

                if( mpTheme->GetGraphic( mnObjectPos, aGraphic ) )
                    mpGraphicObject = new GraphicObject( aGraphic );
            }
        }
        break;

        default:
            OSL_FAIL( "GalleryTransferable::GalleryTransferable: invalid object type" );
        break;
    }
}

// ------------------------------------------------------------------------

void GalleryTransferable::AddSupportedFormats()
{
    if( SGA_OBJ_SVDRAW == meObjectKind )
    {
        AddFormat( SOT_FORMATSTR_ID_DRAWING );
        AddFormat( SOT_FORMATSTR_ID_SVXB );
        AddFormat( FORMAT_GDIMETAFILE );
        AddFormat( FORMAT_BITMAP );
    }
    else
    {
        if( mpURL )
            AddFormat( FORMAT_FILE );

        if( mpGraphicObject )
        {
            AddFormat( SOT_FORMATSTR_ID_SVXB );

            if( mpGraphicObject->GetType() == GRAPHIC_GDIMETAFILE )
            {
                AddFormat( FORMAT_GDIMETAFILE );
                AddFormat( FORMAT_BITMAP );
            }
            else
            {
                AddFormat( FORMAT_BITMAP );
                AddFormat( FORMAT_GDIMETAFILE );
            }
        }
    }
}

// ------------------------------------------------------------------------

sal_Bool GalleryTransferable::GetData( const datatransfer::DataFlavor& rFlavor )
{
    sal_uInt32  nFormat = SotExchange::GetFormat( rFlavor );
    sal_Bool    bRet = sal_False;

    InitData( false );

    if( ( SOT_FORMATSTR_ID_DRAWING == nFormat ) && ( SGA_OBJ_SVDRAW == meObjectKind ) )
    {
        bRet = ( mxModelStream.Is() && SetObject( &mxModelStream, 0, rFlavor ) );
    }
    else if( ( SOT_FORMATSTR_ID_SVIM == nFormat ) && mpImageMap )
    {
        // TODO/MBA: do we need a BaseURL here?!
        bRet = SetImageMap( *mpImageMap, rFlavor );
    }
    else if( ( FORMAT_FILE == nFormat ) && mpURL )
    {
        bRet = SetString( mpURL->GetMainURL( INetURLObject::NO_DECODE ), rFlavor );
    }
    else if( ( SOT_FORMATSTR_ID_SVXB == nFormat ) && mpGraphicObject )
    {
        bRet = SetGraphic( mpGraphicObject->GetGraphic(), rFlavor );
    }
    else if( ( FORMAT_GDIMETAFILE == nFormat ) && mpGraphicObject )
    {
        bRet = SetGDIMetaFile( mpGraphicObject->GetGraphic().GetGDIMetaFile(), rFlavor );
    }
    else if( ( FORMAT_BITMAP == nFormat ) && mpGraphicObject )
    {
        bRet = SetBitmap( mpGraphicObject->GetGraphic().GetBitmap(), rFlavor );
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool GalleryTransferable::WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject,
                                           sal_uInt32, const datatransfer::DataFlavor& )
{
    sal_Bool bRet = sal_False;

    if( pUserObject )
    {
        *rxOStm << *static_cast< SotStorageStream* >( pUserObject );
        bRet = ( rxOStm->GetError() == ERRCODE_NONE );
    }

    return bRet;
}

// ------------------------------------------------------------------------

void GalleryTransferable::DragFinished( sal_Int8 nDropAction )
{
    mpTheme->SetDragging( sal_False );
    mpTheme->SetDragPos( 0 );
    if ( nDropAction )
    {
        Window *pFocusWindow = Application::GetFocusWindow();
        if ( pFocusWindow )
            pFocusWindow->GrabFocusToDocument();
    }
}

// ------------------------------------------------------------------------

void GalleryTransferable::ObjectReleased()
{
    mxModelStream.Clear();
    delete mpGraphicObject, mpGraphicObject = NULL;
    delete mpImageMap, mpImageMap = NULL;
    delete mpURL, mpURL = NULL;
}

// ------------------------------------------------------------------------

void GalleryTransferable::CopyToClipboard( Window* pWindow )
{
    TransferableHelper::CopyToClipboard( pWindow );
}

// ------------------------------------------------------------------------

void GalleryTransferable::StartDrag( Window* pWindow, sal_Int8 nDragSourceActions,
                                     sal_Int32 nDragPointer, sal_Int32 nDragImage )
{
    INetURLObject aURL;

    if( mpTheme->GetURL( mnObjectPos, aURL ) && ( aURL.GetProtocol() != INET_PROT_NOT_VALID ) )
    {
        mpTheme->SetDragging( sal_True );
        mpTheme->SetDragPos( mnObjectPos );
        TransferableHelper::StartDrag( pWindow, nDragSourceActions, nDragPointer, nDragImage );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
