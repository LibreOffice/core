/*************************************************************************
 *
 *  $RCSfile: galmisc.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:03:06 $
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

#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/processfactory.hxx>
#include <ucbhelper/content.hxx>
#include <tools/resmgr.hxx>
#include <tools/intn.hxx>
#include <tools/urlobj.hxx>
#include <svtools/solar.hrc>
#include <svtools/urihelper.hxx>
#include <svtools/filter.hxx>
#include <svtools/itempool.hxx>
#include <sfx2/docfile.hxx>
#include <avmedia/mediawindow.hxx>
#include "impgrf.hxx"
#include "svdpage.hxx"
#include "svdograf.hxx"
#include "fmmodel.hxx"
#include "fmview.hxx"
#include "unomodel.hxx"
#include "codec.hxx"
#include "gallery.hrc"
#include "gallery1.hxx"
#include "galtheme.hxx"
#include "galmisc.hxx"

#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif

// --------------
// - Namespaces -
// --------------

using namespace ::ucb;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::ucb;

// ----------
// - ResMgr -
// ----------

ResMgr* GetGalleryResMgr()
{
    static ResMgr* pGalleryResMgr = NULL;

    if( !pGalleryResMgr )
    {
        ByteString aResMgrName( "gal" );
        aResMgrName += ByteString::CreateFromInt32( SOLARUPD );
        pGalleryResMgr = ResMgr::CreateResMgr(
            aResMgrName.GetBuffer(), Application::GetSettings().GetUILocale() );
    }

    return pGalleryResMgr;
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

USHORT GalleryGraphicImport( const INetURLObject& rURL, Graphic& rGraphic,
                             String& rFilterName, BOOL bShowProgress )
{
    USHORT      nRet = SGA_IMPORT_NONE;
    SfxMedium   aMedium( rURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ, TRUE );
    String      aFilterName;

    aMedium.DownLoad();

    SvStream* pIStm = aMedium.GetInStream();

    if( pIStm )
    {
        GraphicFilter*      pGraphicFilter = GetGrfFilter();
        GalleryProgress*    pProgress = bShowProgress ? new GalleryProgress( pGraphicFilter ) : NULL;
        USHORT              nFormat;

        if( !pGraphicFilter->ImportGraphic( rGraphic, rURL.GetMainURL( INetURLObject::NO_DECODE ), *pIStm, GRFILTER_FORMAT_DONTKNOW, &nFormat ) )
        {
            rFilterName = pGraphicFilter->GetImportFormatName( nFormat );
            nRet = SGA_IMPORT_FILE;
        }

        delete pProgress;
    }

    return nRet;
}

// -----------------------
// - GallerySvDrawImport -
// -----------------------

BOOL GallerySvDrawImport( SvStream& rIStm, FmFormModel& rModel )
{
    UINT32  nVersion;
    BOOL    bRet;

    if( GalleryCodec::IsCoded( rIStm, nVersion ) )
    {
        SvMemoryStream  aMemStm( 65535, 65535 );
        GalleryCodec    aCodec( rIStm );

        aCodec.Read( aMemStm );
        aMemStm.Seek( 0UL );

        if( 1 == nVersion )
        {
            // read as binary
               SgaUserDataFactory   aFactory;

            aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
            rModel.SetStreamingSdrModel( TRUE );
            rModel.GetItemPool().Load( aMemStm );
            aMemStm >> rModel;
            rModel.SetStreamingSdrModel( FALSE );
            rModel.GetItemPool().LoadCompleted();
            bRet = ( rIStm.GetError() == 0 );
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
        rModel.SetStreamingSdrModel( TRUE );
        bRet = SvxDrawingLayerImport( &rModel, xInputStream );
        rModel.SetStreamingSdrModel( FALSE );
    }

    return bRet;
}

// ---------------------
// - CreateIMapGraphic -
// ---------------------

BOOL CreateIMapGraphic( const FmFormModel& rModel, Graphic& rGraphic, ImageMap& rImageMap )
{
    BOOL bRet = FALSE;

    if ( rModel.GetPageCount() )
    {
        const SdrPage*      pPage = rModel.GetPage( 0 );
        const SdrObject*    pObj = pPage->GetObj( 0 );

        if ( pPage->GetObjCount() == 1 && pObj->ISA( SdrGrafObj ) )
        {
            const USHORT nCount = pObj->GetUserDataCount();

            // gibt es in den User-Daten eine IMap-Information?
            for ( USHORT i = 0; i < nCount; i++ )
            {
                const SdrObjUserData* pUserData = pObj->GetUserData( i );

                if ( ( pUserData->GetInventor() == IV_IMAPINFO ) && ( pUserData->GetId() == ID_IMAPINFO ) )
                {
                    rGraphic = ( (SdrGrafObj*) pObj )->GetGraphic();
                    rImageMap = ( (SgaIMapInfo*) pUserData )->GetImageMap();
                    bRet = TRUE;
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

String GetReducedString( const INetURLObject& rURL, ULONG nMaxLen )
{
    String aReduced( rURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) );

    aReduced = aReduced.GetToken( aReduced.GetTokenCount( '/' ) - 1, '/' );

    if( INET_PROT_PRIV_SOFFICE != rURL.GetProtocol() )
    {
        sal_Unicode     aDelimiter;
           const String    aPath( rURL.getFSysPath( INetURLObject::FSYS_DETECT, &aDelimiter ) );
        const String    aName( aReduced );

        if( aPath.Len() > nMaxLen )
        {
            aReduced = aPath.Copy( 0, (USHORT)( nMaxLen - aName.Len() - 4 ) );
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
        rSvDrawObjURL.GetMainURL( INetURLObject::NO_DECODE ).GetTokenCount( '/' ) == 3 )
    {
        aRet = rSvDrawObjURL.GetMainURL( INetURLObject::NO_DECODE ).GetToken( 2, '/' );
    }

    return aRet;
}

// -----------------------------------------------------------------------------

BOOL FileExists( const INetURLObject& rURL )
{
    BOOL bRet = FALSE;

    if( rURL.GetProtocol() != INET_PROT_NOT_VALID )
    {
        try
        {
            Content     aCnt( rURL.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< XCommandEnvironment >() );
            OUString    aTitle;

            aCnt.getPropertyValue( OUString::createFromAscii( "Title" ) ) >>= aTitle;
            bRet = ( aTitle.getLength() > 0 );
        }
        catch( const ContentCreationException& )
        {
        }
        catch( const ::com::sun::star::uno::RuntimeException& )
        {
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL CreateDir( const INetURLObject& rURL )
{
    BOOL bRet = FileExists( rURL );

    if( !bRet )
    {
        try
        {
            uno::Reference< XCommandEnvironment >   aCmdEnv;
            INetURLObject                           aNewFolderURL( rURL );
            INetURLObject                           aParentURL( aNewFolderURL ); aParentURL.removeSegment();
            Content                                 aParent( aParentURL.GetMainURL( INetURLObject::NO_DECODE ), aCmdEnv );
            uno::Sequence< OUString >               aProps( 1 );
            uno::Sequence< uno::Any >               aValues( 1 );

            aProps.getArray()[ 0 ] = OUString::createFromAscii( "Title" );
            aValues.getArray()[ 0 ] = uno::makeAny( OUString( aNewFolderURL.GetName() ) );

        Content aContent( aNewFolderURL.GetMainURL( INetURLObject::NO_DECODE ), aCmdEnv );
        bRet = aParent.insertNewContent( OUString::createFromAscii( "application/vnd.sun.staroffice.fsys-folder" ), aProps, aValues, aContent );
        }
        catch( const ContentCreationException& )
        {
        }
        catch( const ::com::sun::star::uno::RuntimeException& )
        {
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL CopyFile(  const INetURLObject& rSrcURL, const INetURLObject& rDstURL )
{
    BOOL bRet = FALSE;

    try
    {
        Content aDestPath( rDstURL.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< XCommandEnvironment >() );

        aDestPath.executeCommand( OUString::createFromAscii( "transfer" ),
                                  uno::makeAny( TransferInfo( sal_False, rSrcURL.GetMainURL( INetURLObject::NO_DECODE ),
                                                rDstURL.GetName(), NameClash::OVERWRITE ) ) );
        bRet = TRUE;
    }
    catch( const ContentCreationException& )
    {
    }
    catch( const ::com::sun::star::uno::RuntimeException& )
    {
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL KillFile( const INetURLObject& rURL )
{
    BOOL bRet = FileExists( rURL );

    if( bRet )
    {
        try
        {
            Content aCnt( rURL.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< XCommandEnvironment >() );
            aCnt.executeCommand( OUString::createFromAscii( "delete" ), uno::makeAny( sal_Bool( sal_True ) ) );
        }
        catch( const ContentCreationException& )
        {
            bRet = FALSE;
        }
        catch( const ::com::sun::star::uno::RuntimeException& )
        {
            bRet = FALSE;
        }
        catch( const ::com::sun::star::uno::Exception& )
        {
            bRet = FALSE;
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
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMgr( ::utl::getProcessServiceFactory() );

    if( xMgr.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XProgressMonitor > xMonitor( xMgr->createInstance(
                                                      ::rtl::OUString::createFromAscii( "com.sun.star.awt.XProgressMonitor" ) ),
                                                      ::com::sun::star::uno::UNO_QUERY );

        if ( xMonitor.is() )
        {
            mxProgressBar = ::com::sun::star::uno::Reference< ::com::sun::star::awt::XProgressBar >( xMonitor,
                                                                                                     ::com::sun::star::uno::UNO_QUERY );

            if( mxProgressBar.is() )
            {
                String aProgressText;

                if( mpFilter )
                {
                    aProgressText = String( GAL_RESID( RID_SVXSTR_GALLERY_FILTER ) );
                    mpFilter->SetUpdatePercentHdl( LINK( this, GalleryProgress, Update ) );
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
    if( mpFilter )
        mpFilter->SetUpdatePercentHdl( Link() );
}

// ------------------------------------------------------------------------

void GalleryProgress::Update( ULONG nVal, ULONG nMaxVal )
{
    if( mxProgressBar.is() && nMaxVal )
        mxProgressBar->setValue( Min( (ULONG)( (double) nVal / nMaxVal * GALLERY_PROGRESS_RANGE ), (ULONG) GALLERY_PROGRESS_RANGE ) );
}

// ------------------------------------------------------------------------

IMPL_LINK( GalleryProgress, Update, GraphicFilter*, pFilter )
{
    Update( pFilter->GetPercent(), 100 );
    return 0L;
}

// -----------------------
// - GalleryTransferable -
// -----------------------

GalleryTransferable::GalleryTransferable( GalleryTheme* pTheme, ULONG nObjectPos ) :
    mpTheme( pTheme ),
    meObjectKind( mpTheme->GetObjectKind( nObjectPos ) ),
    mnObjectPos( nObjectPos ),
    mpGraphicObject( NULL ),
    mpImageMap( NULL ),
    mpURL( NULL ),
    mbInitialized( sal_False )
{
}

// ------------------------------------------------------------------------

GalleryTransferable::~GalleryTransferable()
{
}

// ------------------------------------------------------------------------

void GalleryTransferable::InitData()
{
    if( !mbInitialized )
    {
        switch( meObjectKind )
        {
            case( SGA_OBJ_ANIM ):
            case( SGA_OBJ_BMP ):
            case( SGA_OBJ_INET ):
            {
                Graphic aGraphic;

                if( mpTheme->GetGraphic( mnObjectPos, aGraphic ) )
                    mpGraphicObject = new GraphicObject( aGraphic );

                mpURL = new INetURLObject;

                if( !mpTheme->GetURL( mnObjectPos, *mpURL ) )
                    delete mpURL, mpURL = NULL;
            }
            break;

            case( SGA_OBJ_SOUND ):
            {
                mpURL = new INetURLObject;

                if( !mpTheme->GetURL( mnObjectPos, *mpURL ) )
                    delete mpURL, mpURL = NULL;
            }
            break;

            case( SGA_OBJ_SVDRAW ):
            {
                Graphic aGraphic;


                if( mpTheme->GetGraphic( mnObjectPos, aGraphic ) )
                    mpGraphicObject = new GraphicObject( aGraphic );

                mxModelStream = new SotStorageStream( String() );
                mxModelStream->SetBufferSize( 16348 );

                if( !mpTheme->GetModelStream( mnObjectPos, mxModelStream ) )
                    mxModelStream.Clear();
                else
                    mxModelStream->Seek( 0 );
            }
            break;

            default:
                DBG_ERROR( "GalleryTransferable::GalleryTransferable: invalid object type" );
            break;
        }

        mbInitialized = sal_True;
    }
}

// ------------------------------------------------------------------------

void GalleryTransferable::AddSupportedFormats()
{
    InitData();

    if( mpURL )
        AddFormat( FORMAT_FILE );

    if( mxModelStream.Is() )
    {
/*!!!
        Graphic     aGraphic;
        ImageMap    aImageMap;

        if( CreateIMapGraphic( *mpModel, aGraphic, aImageMap ) )
        {
            delete mpGraphicObject, mpGraphicObject = new GraphicObject( aGraphic );
            delete mpImageMap, mpImageMap = new ImageMap( aImageMap );

            AddFormat( SOT_FORMATSTR_ID_SVIM );
        }
        else
*/
            AddFormat( SOT_FORMATSTR_ID_DRAWING );
    }

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

// ------------------------------------------------------------------------

sal_Bool GalleryTransferable::GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    sal_uInt32  nFormat = SotExchange::GetFormat( rFlavor );
    sal_Bool    bRet = sal_False;

    InitData();

    if( ( SOT_FORMATSTR_ID_DRAWING == nFormat ) && mxModelStream.Is() )
    {
        bRet = SetObject( &mxModelStream, 0, rFlavor );
    }
    else if( ( SOT_FORMATSTR_ID_SVIM == nFormat ) && mpImageMap )
    {
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
                                           sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor )
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
    mpTheme->SetDragging( FALSE );
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
    InitData();
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
