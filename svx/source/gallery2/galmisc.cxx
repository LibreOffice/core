/*************************************************************************
 *
 *  $RCSfile: galmisc.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: pb $ $Date: 2000-10-23 12:08:49 $
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

#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>
#include <tools/resmgr.hxx>
#include <tools/intn.hxx>
#include <tools/urlobj.hxx>
#include <svtools/solar.hrc>
#include <svtools/urihelper.hxx>
#include <svtools/filter.hxx>
#include <svtools/itempool.hxx>
#include <sfx2/docfile.hxx>
#include "impgrf.hxx"
#include "svdpage.hxx"
#include "svdograf.hxx"
#include "fmmodel.hxx"
#include "fmview.hxx"
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

// -----------
// - Defines -
// -----------

#define GALLERY_PROGRESS_RANGE 10000

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
            aResMgrName.GetBuffer(), Application::GetAppInternational().GetLanguage() );
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

// -------------
// - SGAImport -
// -------------

USHORT SGAImport( const String& rFile, Graphic& rGraphic,
                  String& rFilterName, BOOL bShowProgress, const USHORT nDepth )
{
    USHORT nRet = SGA_IMPORT_NONE;

    // Wir duerfen hier maximal 2-mal reinlaufen (1.INET => 2.reelles File );
    // alles andere fuehrt zu einer unendlichen Rekursion => kein gueltiges File
    if ( nDepth < 2 )
    {
        // INet-Download oder normales Laden
        if ( ( INetURLObject( ::URIHelper::SmartRelToAbs( rFile, FALSE,
                                                          INetURLObject::WAS_ENCODED,
                                                          INetURLObject::DECODE_UNAMBIGUOUS ) ).GetProtocol() != INET_PROT_FILE ) )
        {
            SfxMedium   aMed( rFile, STREAM_READ, TRUE );
            String      aFilterName;

            aMed.SetTransferPriority( SFX_TFPRIO_VISIBLE_HIGHRES_GRAPHIC | SFX_TFPRIO_SYNCHRON );
            aMed.DownLoad();

            // kleine Rekursion
            if ( SGAImport( aMed.GetPhysicalName(), rGraphic, rFilterName, bShowProgress, nDepth + 1 ) == SGA_IMPORT_FILE )
                nRet = SGA_IMPORT_INET;
        }
        else
        {
            GraphicFilter*      pGraphicFilter = GetGrfFilter();
            GalleryProgress*    pProgress = bShowProgress ? new GalleryProgress( pGraphicFilter ) : NULL;
            USHORT              nFormat;

            if( !pGraphicFilter->ImportGraphic( rGraphic, INetURLObject( rFile, INET_PROT_FILE ), GRFILTER_FORMAT_DONTKNOW, &nFormat ) )
            {
                rFilterName = pGraphicFilter->GetImportFormatName( nFormat );
                nRet = SGA_IMPORT_FILE;
            }

            delete pProgress;
        }
    }

    return nRet;
}

// -------------------
// - SGASvDrawImport -
// -------------------

BOOL SGASvDrawImport( SvStream& rIStm, FmFormModel& rModel )
{
    BOOL bRet;

    if ( RLECodec::IsRLECoded( rIStm ) )
    {
        SvMemoryStream  aMemStm;
        RLECodec        aCodec( rIStm );

        aCodec.Read( aMemStm );
        aMemStm.Seek( 0UL );
        bRet = SGASvDrawImport( aMemStm, rModel );
    }
    else
    {
        SgaUserDataFactory  aFactory;

        rIStm.SetVersion( SOFFICE_FILEFORMAT_NOW );
        rModel.SetStreamingSdrModel( TRUE );
        rModel.GetItemPool().Load( rIStm );
        rIStm >> rModel;
        rModel.SetStreamingSdrModel( FALSE );
        rModel.GetItemPool().LoadCompleted();
        bRet = ( rIStm.GetError() == 0 );
    }

    return bRet;
}

// ------------------
// - SGAIsSoundFile -
// ------------------

BOOL SGAIsSoundFile( const String& rFile )
{
    const String    aExt( INetURLObject( rFile, INET_PROT_FILE ).GetExtension().ToLowerAscii() );
    BOOL            bRet = FALSE;

    if( ( aExt == String( RTL_CONSTASCII_USTRINGPARAM( "wav" ) ) ) ||
        ( aExt == String( RTL_CONSTASCII_USTRINGPARAM( "aif" ) ) ) ||
        ( aExt == String( RTL_CONSTASCII_USTRINGPARAM( "au" ) ) ) )
        bRet = TRUE;
    else
    {
        // File anlesen
        SvFileStream aIStm( rFile, STREAM_READ );

        if ( aIStm.IsOpen() )
        {
            BYTE cVal1, cVal2, cVal3, cVal4;

            aIStm >> cVal1 >> cVal2 >> cVal3 >> cVal4;

            if ( ( cVal1 == 'R' && cVal2 == 'I' && cVal3 == 'F' && cVal4 == 'F' ) ||
                 ( cVal1 == '.' && cVal2 == 's' && cVal3 == 'n' && cVal4 == 'd' ) )
            {
                bRet = TRUE;
            }
        }
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

// ----------------
// - ReduceString -
// ----------------

String ReduceString( const String& rString )
{
    String aStr;

    if ( rString.Len() > 30 )
    {
        const String aName( INetURLObject( rString, INET_PROT_FILE ).GetName() );

        aStr = rString.Copy( 0, 30 - aName.Len() - 4 );
        aStr += String( RTL_CONSTASCII_USTRINGPARAM( "..." ) );
        aStr += '/';
        aStr += aName;
    }
    else
        aStr = rString;

    return aStr;
}

// ------------------------
// - CreateUniqueFileName -
// ------------------------

INetURLObject CreateUniqueFileName( Gallery* pGallery, SgaObjKind eObjKind )
{
    SvFileStream    aStm;
    INetURLObject   aPath( pGallery->GetUserPath(), INET_PROT_FILE );
    INetURLObject   aTmpPath( aPath );
    sal_uInt32      nNextNumber;

    aTmpPath.Append( String( RTL_CONSTASCII_USTRINGPARAM( "sdddndx1" ) ) );

    if( FileExists( aTmpPath ) )
    {
        aStm.Open( aTmpPath.PathToFileName(), STREAM_READ );
        aStm >> nNextNumber;
        aStm.Close();
    }
    else
        nNextNumber = 1999;

    String aNextFileName( RTL_CONSTASCII_USTRINGPARAM( "dd" ) );
    aNextFileName += String::CreateFromInt32( ++nNextNumber % ( eObjKind == SGA_OBJ_SVDRAW ? 99999999L : 999999L ) );

    aStm.Open( aTmpPath.PathToFileName(), STREAM_WRITE | STREAM_TRUNC );

    if( aStm.IsOpen() )
    {
        aStm << nNextNumber;
        aStm.Close();
    }

    aPath.Append( String( RTL_CONSTASCII_USTRINGPARAM( "dragdrop" ) ) );
    CreateDir( aPath );
    aPath.Append( aNextFileName += String( RTL_CONSTASCII_USTRINGPARAM( ".svm" ) ) );

    return aPath;
}

// -----------------------------------------------------------------------------

BOOL FileExists( const INetURLObject& rURL )
{
    BOOL bRet = FALSE;

    try
    {
        Content     aCnt( rURL.GetMainURL(), uno::Reference< XCommandEnvironment >() );
        OUString    aTitle;

        aCnt.getPropertyValue( OUString::createFromAscii( "Title" ) ) >>= aTitle;
        bRet = ( aTitle.getLength() > 0 );
    }
    catch( ... )
    {
        DBG_ERROR( "Gallery: FileExists: ucb error" );
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
            Content                                 aParent( aParentURL.GetMainURL(), aCmdEnv );
            uno::Sequence< OUString >               aProps( 1 );
            uno::Sequence< uno::Any >               aValues( 1 );

            aProps.getArray()[ 0 ] = OUString::createFromAscii( "Title" );
            aValues.getArray()[ 0 ] = uno::makeAny( OUString( aNewFolderURL.GetName() ) );

            bRet = aParent.insertNewContent( OUString::createFromAscii( "application/vnd.sun.staroffice.fsys-folder" ), aProps, aValues,
                                             Content( aNewFolderURL.GetMainURL(), aCmdEnv ) );
        }
        catch( ... )
        {
            DBG_ERROR( "Gallery: CreateDir: ucb error" );
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
        Content aDestPath( rDstURL.GetMainURL(), uno::Reference< XCommandEnvironment >() );

        aDestPath.executeCommand( OUString::createFromAscii( "transfer" ),
                                  uno::makeAny( TransferInfo( sal_False, rSrcURL.GetMainURL(),
                                                rDstURL.GetName(), NameClash::OVERWRITE ) ) );
        bRet = TRUE;
    }
    catch( ... )
    {
        DBG_ERROR( "Gallery: CopyFile: ucb error" );
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
            Content aCnt( rURL.GetMainURL(), uno::Reference< XCommandEnvironment >() );
            aCnt.executeCommand( OUString::createFromAscii( "delete" ), uno::makeAny( sal_Bool( sal_True ) ) );
        }
        catch( ... )
        {
            bRet = FALSE;
            DBG_ERROR( "Gallery: KillFile: ucb error" );
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
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );

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

// -----------------
// - SgaDataObject -
// -----------------

SgaDataObject::SgaDataObject( GalleryTheme* pTheme, ULONG nActualObjPos ) :
            mpTheme ( pTheme ),
            mnObjPos( nActualObjPos )
{
    mpTheme->SetDragging( TRUE );
    mpTheme->SetDragPos( mnObjPos );
}

// ------------------------------------------------------------------------

SgaDataObject::~SgaDataObject()
{
    mpTheme->SetDragging( FALSE );
    mpTheme->SetDragPos( 0 );
    maTypeList.Clear();
}

// ------------------------------------------------------------------------

BOOL SgaDataObject::GetData( SvData* pData )
{
    BOOL bRet = FALSE;

    if( GetTypeList().Get( *pData ) && mpTheme )
        bRet = mpTheme->GetDataXChgData( pData, pData->GetFormat(), mnObjPos );

    return bRet;
}

// ------------------------------------------------------------------------

const SvDataTypeList& SgaDataObject::GetTypeList() const
{
    if( !maTypeList.Count() )
        ( (SgaDataObject*) this )->maTypeList = mpTheme->GetDataXChgTypeList( maTypeList, mnObjPos );

    return maTypeList;
}
