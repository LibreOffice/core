/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdxfer.cxx,v $
 *
 *  $Revision: 1.57 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:35:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _COM_SUN_STAR_EMBED_XTRANSACTEDOBJECT_HPP_
#include <com/sun/star/embed/XTransactedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDPERSIST_HPP_
#include <com/sun/star/embed/XEmbedPersist.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _UNTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SVX_FLDITEM_HXX //autogen
#include <svx/flditem.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVDOGRAF_HXX //autogen
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef _OUTLOBJ_HXX //autogen
#include <svx/outlobj.hxx>
#endif
#include <sot/storage.hxx>
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif
#ifndef _EDITOBJ_HXX //autogen
#include <svx/editobj.hxx>
#endif
#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif
#ifndef _SVDOUNO_HXX //autogen
#include <svx/svdouno.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SOT_FORMATS_HXX //autogen
#include <sot/formats.hxx>
#endif
#ifndef _URLBMK_HXX //autogen
#include <svtools/urlbmk.hxx>
#endif
#ifndef _OUTLINER_HXX //autogen
#include <svx/outliner.hxx>
#endif

//#ifndef _SVDETC_HXX //autogen
//#include <svx/svdetc.hxx>
//#endif

#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#include <svx/svdotable.hxx>
#include <svx/unomodel.hxx>
#include <svx/svditer.hxx>
#include <sfx2/docfile.hxx>
#include <comphelper/storagehelper.hxx>
#include <svtools/embedtransfer.hxx>

#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#include "sdpage.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "drawdoc.hxx"
#include "stlpool.hxx"
#include "strings.hrc"
#include "sdresid.hxx"
#include "imapinfo.hxx"
#include "sdxfer.hxx"
#include "unomodel.hxx"

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;

// -----------
// - Defines -
// -----------

#define SDTRANSFER_OBJECTTYPE_DRAWMODEL         0x00000001
#define SDTRANSFER_OBJECTTYPE_DRAWOLE           0x00000002

// ------------------
// - SdTransferable -
// ------------------

SdTransferable::SdTransferable( SdDrawDocument* pSrcDoc, ::sd::View* pWorkView, BOOL bInitOnGetData )
:   mpPageDocShell( NULL )
,   mpOLEDataHelper( NULL )
,   mpObjDesc( NULL )
,   mpSdView( pWorkView )
,   mpSdViewIntern( pWorkView )
,   mpSdDrawDocument( NULL )
,   mpSdDrawDocumentIntern( NULL )
,   mpSourceDoc( pSrcDoc )
,   mpVDev( NULL )
,   mpBookmark( NULL )
,   mpGraphic( NULL )
,   mpImageMap( NULL )
,   mbInternalMove( FALSE )
,   mbOwnDocument( FALSE )
,   mbOwnView( FALSE )
,   mbLateInit( bInitOnGetData )
,   mbPageTransferable( FALSE )
,   mbPageTransferablePersistent( FALSE )
,   mbIsUnoObj( false )
{
    if( mpSourceDoc )
        StartListening( *mpSourceDoc );

    if( pWorkView )
        StartListening( *pWorkView );

    if( !mbLateInit )
        CreateData();
}

// -----------------------------------------------------------------------------

SdTransferable::~SdTransferable()
{
    if( mpSourceDoc )
        EndListening( *mpSourceDoc );

    if( mpSdView )
        EndListening( *const_cast< sd::View *>( mpSdView) );

    Application::GetSolarMutex().acquire();

    ObjectReleased();

    for( void* p = maPageBookmarks.First(); p; p = maPageBookmarks.Next() )
        delete static_cast< String* >( p );

    if( mbOwnView )
        delete mpSdViewIntern;

    delete mpOLEDataHelper;

    if( maDocShellRef.Is() )
    {
        SfxObjectShell* pObj = maDocShellRef;
        ::sd::DrawDocShell* pDocSh = static_cast< ::sd::DrawDocShell*>(pObj);
        pDocSh->DoClose();
    }

    maDocShellRef.Clear();

    if( mbOwnDocument )
        delete mpSdDrawDocumentIntern;

    delete mpGraphic;
    delete mpBookmark;
    delete mpImageMap;

    delete mpVDev;
    delete mpObjDesc;

    Application::GetSolarMutex().release();
}

// -----------------------------------------------------------------------------

void SdTransferable::CreateObjectReplacement( SdrObject* pObj )
{
    if( pObj )
    {
        delete mpOLEDataHelper, mpOLEDataHelper = NULL;
        delete mpGraphic, mpGraphic = NULL;
        delete mpBookmark, mpBookmark = NULL;
        delete mpImageMap, mpImageMap = NULL;

        if( pObj->ISA( SdrOle2Obj ) )
        {
            try
            {
                uno::Reference < embed::XEmbeddedObject > xObj = static_cast< SdrOle2Obj* >( pObj )->GetObjRef();
                uno::Reference < embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
                if( xObj.is() && xPersist.is() && xPersist->hasEntry() )
                {
                    mpOLEDataHelper = new TransferableDataHelper( new SvEmbedTransferHelper( xObj, static_cast< SdrOle2Obj* >( pObj )->GetGraphic(), static_cast< SdrOle2Obj* >( pObj )->GetAspect() ) );

                    // TODO/LATER: the standalone handling of the graphic should not be used any more in future
                    // The EmbedDataHelper should bring the graphic in future
                    Graphic* pObjGr = static_cast< SdrOle2Obj* >( pObj )->GetGraphic();
                    if ( pObjGr )
                        mpGraphic = new Graphic( *pObjGr );
                }
            }
            catch( uno::Exception& )
            {}
        }
        else if( pObj->ISA( SdrGrafObj ) && (mpSourceDoc && !mpSourceDoc->GetAnimationInfo( pObj )) )
        {
            mpGraphic = new Graphic( static_cast< SdrGrafObj* >( pObj )->GetTransformedGraphic() );
        }
        else if( pObj->IsUnoObj() && FmFormInventor == pObj->GetObjInventor() && ( pObj->GetObjIdentifier() == (UINT16) OBJ_FM_BUTTON ) )
        {
            SdrUnoObj* pUnoCtrl = static_cast< SdrUnoObj* >( pObj );

            if (pUnoCtrl && FmFormInventor == pUnoCtrl->GetObjInventor())
            {
                Reference< ::com::sun::star::awt::XControlModel > xControlModel( pUnoCtrl->GetUnoControlModel() );

                if( !xControlModel.is() )
                    return;

                Reference< ::com::sun::star::beans::XPropertySet > xPropSet( xControlModel, UNO_QUERY );

                if( !xPropSet.is() )
                    return;

                ::com::sun::star::form::FormButtonType  eButtonType;
                Any                                     aTmp( xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ButtonType" ) ) ) );

                if( aTmp >>= eButtonType )
                {
                    ::rtl::OUString aLabel, aURL;

                    xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Label" ) ) ) >>= aLabel;
                    xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("TargetURL") ) ) >>= aURL;

                    mpBookmark = new INetBookmark( String( aURL ), String( aLabel ) );
                }
            }
        }
        else if( pObj->ISA( SdrTextObj ) )
        {
            const OutlinerParaObject* pPara;

            if( (pPara = static_cast< SdrTextObj* >( pObj )->GetOutlinerParaObject()) != 0 )
            {
                const SvxFieldItem* pField;

                if( (pField = pPara->GetTextObject().GetField()) != 0 )
                {
                    const SvxFieldData* pData = pField->GetField();

                    if( pData && pData->ISA( SvxURLField ) )
                    {
                        const SvxURLField* pURL = (SvxURLField*) pData;

                        mpBookmark = new INetBookmark( pURL->GetURL(), pURL->GetRepresentation() );
                    }
                }
            }
        }

        SdIMapInfo* pInfo = static_cast< SdDrawDocument* >( pObj->GetModel() )->GetIMapInfo( static_cast< SdrObject* >( pObj ) );

        if( pInfo )
            mpImageMap = new ImageMap( pInfo->GetImageMap() );

        mbIsUnoObj = pObj && pObj->IsUnoObj();
    }
}

// -----------------------------------------------------------------------------

void SdTransferable::CreateData()
{
    if( mpSdDrawDocument && !mpSdViewIntern )
    {
        mbOwnView = TRUE;

        SdPage* pPage = mpSdDrawDocument->GetSdPage(0, PK_STANDARD);

        if( 1 == pPage->GetObjCount() )
            CreateObjectReplacement( pPage->GetObj( 0 ) );

        mpVDev = new VirtualDevice( *Application::GetDefaultDevice() );
        mpVDev->SetMapMode( MapMode( mpSdDrawDocumentIntern->GetScaleUnit(), Point(), mpSdDrawDocumentIntern->GetScaleFraction(), mpSdDrawDocumentIntern->GetScaleFraction() ) );
        mpSdViewIntern = new ::sd::View( mpSdDrawDocumentIntern, mpVDev );
        mpSdViewIntern->EndListening(*mpSdDrawDocumentIntern );
        mpSdViewIntern->hideMarkHandles();
        SdrPageView* pPageView = mpSdViewIntern->ShowSdrPage(pPage);
        ((SdrMarkView*)mpSdViewIntern)->MarkAllObj(pPageView);
    }
    else if( mpSdView && !mpSdDrawDocumentIntern )
    {
        const SdrMarkList& rMarkList = mpSdView->GetMarkedObjectList();

        if( rMarkList.GetMarkCount() == 1 )
            CreateObjectReplacement( rMarkList.GetMark( 0 )->GetMarkedSdrObj() );

        if( mpSourceDoc )
            mpSourceDoc->CreatingDataObj(this);
        mpSdDrawDocumentIntern = (SdDrawDocument*) mpSdView->GetAllMarkedModel();
        if( mpSourceDoc )
            mpSourceDoc->CreatingDataObj(0);

        if( !maDocShellRef.Is() && mpSdDrawDocumentIntern->GetDocSh() )
            maDocShellRef = mpSdDrawDocumentIntern->GetDocSh();

        if( !maDocShellRef.Is() )
        {
            DBG_ERROR( "SdTransferable::CreateData(), failed to create a model with persist, clipboard operation will fail for OLE objects!" );
            mbOwnDocument = TRUE;
        }

        // Groesse der Source-Seite uebernehmen
        SdrPageView*        pPgView = mpSdView->GetSdrPageView();
        SdPage*             pOldPage = (SdPage*) pPgView->GetPage();
        SdrModel*           pOldModel = mpSdView->GetModel();
        SdStyleSheetPool*   pOldStylePool = (SdStyleSheetPool*) pOldModel->GetStyleSheetPool();
        SdStyleSheetPool*   pNewStylePool = (SdStyleSheetPool*) mpSdDrawDocumentIntern->GetStyleSheetPool();
        SdPage*             pPage = mpSdDrawDocumentIntern->GetSdPage( 0, PK_STANDARD );
        String              aOldLayoutName( pOldPage->GetLayoutName() );

        pPage->SetSize( pOldPage->GetSize() );
        pPage->SetLayoutName( aOldLayoutName );
        pNewStylePool->CopyGraphicSheets( *pOldStylePool );
        pNewStylePool->CopyCellSheets( *pOldStylePool );
        pNewStylePool->CopyTableStyles( *pOldStylePool );
        aOldLayoutName.Erase( aOldLayoutName.SearchAscii( SD_LT_SEPARATOR ) );
        SdStyleSheetVector aCreatedSheets;
        pNewStylePool->CopyLayoutSheets( aOldLayoutName, *pOldStylePool, aCreatedSheets );
    }

    // set VisArea and adjust objects if neccessary
    if( maVisArea.IsEmpty() &&
        mpSdDrawDocumentIntern && mpSdViewIntern &&
        mpSdDrawDocumentIntern->GetPageCount() )
    {
        SdPage* pPage = mpSdDrawDocumentIntern->GetSdPage( 0, PK_STANDARD );

        if( 1 == mpSdDrawDocumentIntern->GetPageCount() )
        {
            Point   aOrigin( ( maVisArea = mpSdViewIntern->GetAllMarkedRect() ).TopLeft() );
            Size    aVector( -aOrigin.X(), -aOrigin.Y() );

            for( ULONG nObj = 0, nObjCount = pPage->GetObjCount(); nObj < nObjCount; nObj++ )
            {
                SdrObject* pObj = pPage->GetObj( nObj );
                pObj->NbcMove( aVector );
            }
        }
        else
            maVisArea.SetSize( pPage->GetSize() );

        // Die Ausgabe soll am Nullpunkt erfolgen
        maVisArea.SetPos( Point() );
    }
}

// -----------------------------------------------------------------------------

BOOL lcl_HasOnlyControls( SdrModel* pModel )
{
    BOOL bOnlyControls = FALSE;         // default if there are no objects

    if ( pModel )
    {
        SdrPage* pPage = pModel->GetPage(0);
        if (pPage)
        {
            SdrObjListIter aIter( *pPage, IM_DEEPNOGROUPS );
            SdrObject* pObj = aIter.Next();
            if ( pObj )
            {
                bOnlyControls = TRUE;   // only set if there are any objects at all
                while ( pObj )
                {
                    if (!pObj->ISA(SdrUnoObj))
                    {
                        bOnlyControls = FALSE;
                        break;
                    }
                    pObj = aIter.Next();
                }
            }
        }
    }

    return bOnlyControls;
}

// -----------------------------------------------------------------------------

bool lcl_HasOnlyOneTable( SdrModel* pModel )
{
    if ( pModel )
    {
        SdrPage* pPage = pModel->GetPage(0);
        if (pPage && pPage->GetObjCount() == 1 )
        {
            if( dynamic_cast< sdr::table::SdrTableObj* >( pPage->GetObj(0) ) != 0 )
                return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------

void SdTransferable::AddSupportedFormats()
{
    if( !mbPageTransferable || mbPageTransferablePersistent )
    {
        if( !mbLateInit )
            CreateData();

        if( mpObjDesc )
            AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );

        if( mpOLEDataHelper )
        {
            AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );

            DataFlavorExVector              aVector( mpOLEDataHelper->GetDataFlavorExVector() );
            DataFlavorExVector::iterator    aIter( aVector.begin() ), aEnd( aVector.end() );

            while( aIter != aEnd )
                AddFormat( *aIter++ );
        }
        else if( mpGraphic )
        {
            // #i25616#
            AddFormat( SOT_FORMATSTR_ID_DRAWING );

            AddFormat( SOT_FORMATSTR_ID_SVXB );

            if( mpGraphic->GetType() == GRAPHIC_BITMAP )
            {
                AddFormat( SOT_FORMAT_BITMAP );
                AddFormat( SOT_FORMAT_GDIMETAFILE );
            }
            else
            {
                AddFormat( SOT_FORMAT_GDIMETAFILE );
                AddFormat( SOT_FORMAT_BITMAP );
            }
        }
        else if( mpBookmark )
        {
            AddFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
            AddFormat( FORMAT_STRING );
        }
        else
        {
            AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
            AddFormat( SOT_FORMATSTR_ID_DRAWING );
            if( !mpSdDrawDocument || !lcl_HasOnlyControls( mpSdDrawDocument ) )
            {
                AddFormat( SOT_FORMAT_GDIMETAFILE );
                AddFormat( SOT_FORMAT_BITMAP );
            }

            if( lcl_HasOnlyOneTable( mpSdDrawDocument ) )
                AddFormat( SOT_FORMAT_RTF );
        }

        if( mpImageMap )
            AddFormat( SOT_FORMATSTR_ID_SVIM );
    }
}

// -----------------------------------------------------------------------------

sal_Bool SdTransferable::GetData( const DataFlavor& rFlavor )
{
    sal_uInt32  nFormat = SotExchange::GetFormat( rFlavor );
    sal_Bool    bOK = sal_False;

    CreateData();

    if( nFormat == SOT_FORMAT_RTF && lcl_HasOnlyOneTable( mpSdDrawDocument ) )
    {
        bOK = SetTableRTF( mpSdDrawDocument, rFlavor );
    }
    else if( mpOLEDataHelper && mpOLEDataHelper->HasFormat( rFlavor ) )
    {
        ULONG nOldSwapMode = 0;

        if( mpSdDrawDocumentIntern )
        {
            nOldSwapMode = mpSdDrawDocumentIntern->GetSwapGraphicsMode();
            mpSdDrawDocumentIntern->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_PURGE );
        }

        // TODO/LATER: support all the graphical formats, the embedded object scenario should not have separated handling
        if( nFormat == FORMAT_GDIMETAFILE && mpGraphic )
            bOK = SetGDIMetaFile( mpGraphic->GetGDIMetaFile(), rFlavor );
        else
            bOK = SetAny( mpOLEDataHelper->GetAny( rFlavor ), rFlavor );

        if( mpSdDrawDocumentIntern )
            mpSdDrawDocumentIntern->SetSwapGraphicsMode( nOldSwapMode );
    }
    else if( HasFormat( nFormat ) )
    {
        if( ( nFormat == SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR || nFormat == SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) && mpObjDesc )
        {
            bOK = SetTransferableObjectDescriptor( *mpObjDesc, rFlavor );
        }
        else if( nFormat == SOT_FORMATSTR_ID_DRAWING )
        {
            SfxObjectShellRef aOldRef( maDocShellRef );

            maDocShellRef.Clear();

            if( mpSdViewIntern )
            {
                SdDrawDocument* pInternDoc = mpSdViewIntern->GetDoc();
                if( pInternDoc )
                    pInternDoc->CreatingDataObj(this);
                SdDrawDocument* pDoc = dynamic_cast< SdDrawDocument* >( mpSdViewIntern->GetAllMarkedModel() );
                if( pInternDoc )
                    pInternDoc->CreatingDataObj(0);

                bOK = SetObject( pDoc, SDTRANSFER_OBJECTTYPE_DRAWMODEL, rFlavor );

                if( maDocShellRef.Is() )
                {
                    maDocShellRef->DoClose();
                }
                else
                {
                    delete pDoc;
                }
            }

            maDocShellRef = aOldRef;
        }
        else if( nFormat == FORMAT_GDIMETAFILE )
        {
            if( mpSdViewIntern )
                bOK = SetGDIMetaFile( mpSdViewIntern->GetAllMarkedMetaFile( TRUE ), rFlavor );
        }
        else if( nFormat == FORMAT_BITMAP )
        {
            if( mpSdViewIntern )
                bOK = SetBitmap( mpSdViewIntern->GetAllMarkedBitmap( TRUE ), rFlavor );
        }
        else if( ( nFormat == FORMAT_STRING ) && mpBookmark )
        {
            bOK = SetString( mpBookmark->GetURL(), rFlavor );
        }
        else if( ( nFormat == SOT_FORMATSTR_ID_SVXB ) && mpGraphic )
        {
            bOK = SetGraphic( *mpGraphic, rFlavor );
        }
        else if( ( nFormat == SOT_FORMATSTR_ID_SVIM ) && mpImageMap )
        {
            bOK = SetImageMap( *mpImageMap, rFlavor );
        }
        else if( mpBookmark )
        {
            bOK = SetINetBookmark( *mpBookmark, rFlavor );
        }
        else if( nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE )
        {
            ULONG nOldSwapMode = 0;

            if( mpSdDrawDocumentIntern )
            {
                nOldSwapMode = mpSdDrawDocumentIntern->GetSwapGraphicsMode();
                mpSdDrawDocumentIntern->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_PURGE );
            }

            if( !maDocShellRef.Is() )
            {
                maDocShellRef = new ::sd::DrawDocShell(
                    mpSdDrawDocumentIntern,
                    SFX_CREATE_MODE_EMBEDDED,
                    TRUE,
                    mpSdDrawDocumentIntern->GetDocumentType());
                mbOwnDocument = FALSE;
                maDocShellRef->DoInitNew( NULL );
            }

            maDocShellRef->SetVisArea( maVisArea );
            bOK = SetObject( &maDocShellRef, SDTRANSFER_OBJECTTYPE_DRAWOLE, rFlavor );

            if( mpSdDrawDocumentIntern )
                mpSdDrawDocumentIntern->SetSwapGraphicsMode( nOldSwapMode );
        }
    }

    return bOK;
}

// -----------------------------------------------------------------------------

/* testcode
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
*/

sal_Bool SdTransferable::WriteObject( SotStorageStreamRef& rxOStm, void* pObject, sal_uInt32 nObjectType, const DataFlavor& )
{
    sal_Bool bRet = sal_False;

    switch( nObjectType )
    {
        case( SDTRANSFER_OBJECTTYPE_DRAWMODEL ):
        {
            try
            {
                static const BOOL bDontBurnInStyleSheet = ( getenv( "AVOID_BURN_IN_FOR_GALLERY_THEME" ) != NULL );
                SdDrawDocument* pDoc = (SdDrawDocument*) pObject;
                if ( !bDontBurnInStyleSheet )
                    pDoc->BurnInStyleSheetAttributes();
                rxOStm->SetBufferSize( 16348 );

                Reference< XComponent > xComponent( new SdXImpressDocument( pDoc, sal_True ) );
                pDoc->setUnoModel( Reference< XInterface >::query( xComponent ) );

                {
                    com::sun::star::uno::Reference<com::sun::star::io::XOutputStream> xDocOut( new utl::OOutputStreamWrapper( *rxOStm ) );
                    if( SvxDrawingLayerExport( pDoc, xDocOut, xComponent, (pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) ? "com.sun.star.comp.Impress.XMLClipboardExporter" : "com.sun.star.comp.DrawingLayer.XMLExporter" ) )
                        rxOStm->Commit();
                }

    /* testcode
                {
                    const rtl::OUString aURL( RTL_CONSTASCII_USTRINGPARAM( "file:///e:/test.xml" ) );
                    SfxMedium aMedium( aURL, STREAM_WRITE | STREAM_TRUNC, TRUE );
                    aMedium.IsRemote();
                    com::sun::star::uno::Reference<com::sun::star::io::XOutputStream> xDocOut( new utl::OOutputStreamWrapper( *aMedium.GetOutStream() ) );
                    if( SvxDrawingLayerExport( pDoc, xDocOut, xComponent, (pDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) ? "com.sun.star.comp.Impress.XMLClipboardExporter" : "com.sun.star.comp.DrawingLayer.XMLExporter" ) )
                        aMedium.Commit();
                }
    */

                xComponent->dispose();
                bRet = ( rxOStm->GetError() == ERRCODE_NONE );
            }
            catch( Exception& )
            {
                DBG_ERROR( "sd::SdTransferable::WriteObject(), exception catched!" );
                bRet = FALSE;
            }
        }
        break;

        case( SDTRANSFER_OBJECTTYPE_DRAWOLE ):
        {
            SfxObjectShell*   pEmbObj = (SfxObjectShell*) pObject;
            ::utl::TempFile     aTempFile;
            aTempFile.EnableKillingFile();

            try
            {
                uno::Reference< embed::XStorage > xWorkStore =
                    ::comphelper::OStorageHelper::GetStorageFromURL( aTempFile.GetURL(), embed::ElementModes::READWRITE );

                // write document storage
                pEmbObj->SetupStorage( xWorkStore, SOFFICE_FILEFORMAT_CURRENT, sal_False );
                // mba: no relative ULRs for clipboard!
                SfxMedium aMedium( xWorkStore, String() );
                bRet = pEmbObj->DoSaveObjectAs( aMedium, FALSE );
                pEmbObj->DoSaveCompleted();

                uno::Reference< embed::XTransactedObject > xTransact( xWorkStore, uno::UNO_QUERY );
                if ( xTransact.is() )
                    xTransact->commit();

                SvStream* pSrcStm = ::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), STREAM_READ );
                if( pSrcStm )
                {
                    rxOStm->SetBufferSize( 0xff00 );
                    *rxOStm << *pSrcStm;
                    delete pSrcStm;
                }

                bRet = TRUE;
                rxOStm->Commit();
            }
            catch ( Exception& )
            {}
        }

        break;

        default:
        break;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void SdTransferable::DragFinished( sal_Int8 nDropAction )
{
    if( mpSdView )
        ( (::sd::View*) mpSdView )->DragFinished( nDropAction );
}

// -----------------------------------------------------------------------------

void SdTransferable::ObjectReleased()
{
    if( this == SD_MOD()->pTransferClip )
        SD_MOD()->pTransferClip = NULL;

    if( this == SD_MOD()->pTransferDrag )
        SD_MOD()->pTransferDrag = NULL;

    if( this == SD_MOD()->pTransferSelection )
        SD_MOD()->pTransferSelection = NULL;
}

// -----------------------------------------------------------------------------

void SdTransferable::SetObjectDescriptor( const TransferableObjectDescriptor& rObjDesc )
{
    delete mpObjDesc;
    mpObjDesc = new TransferableObjectDescriptor( rObjDesc );
}

// -----------------------------------------------------------------------------

void SdTransferable::SetPageBookmarks( const List& rPageBookmarks, BOOL bPersistent )
{
    if( mpSourceDoc )
    {
        if( mpSdViewIntern )
            mpSdViewIntern->HideSdrPage();

        // #116168#
        mpSdDrawDocument->ClearModel(sal_False);

        mpPageDocShell = NULL;

        for( void* p = maPageBookmarks.First(); p; p = maPageBookmarks.Next() )
            delete static_cast< String* >( p );

        if( bPersistent )
        {
            mpSdDrawDocument->CreateFirstPages(mpSourceDoc);
            mpSdDrawDocument->InsertBookmarkAsPage( const_cast< List* >( &rPageBookmarks ), NULL, FALSE, TRUE, 1, TRUE, mpSourceDoc->GetDocSh(), TRUE, TRUE, FALSE );
        }
        else
        {
            mpPageDocShell = mpSourceDoc->GetDocSh();

            for( ULONG i = 0; i < rPageBookmarks.Count(); i++ )
                maPageBookmarks.Insert( new String( *static_cast< String* >( rPageBookmarks.GetObject( i ) ) ), LIST_APPEND );
        }

        if( mpSdViewIntern && mpSdDrawDocument )
        {
            SdPage* pPage = mpSdDrawDocument->GetSdPage( 0, PK_STANDARD );

            if( pPage )
            {
                ( (SdrMarkView*) mpSdViewIntern )->MarkAllObj( (SdrPageView*) mpSdViewIntern->ShowSdrPage( pPage ) );
            }
        }

        // set flags for page transferable; if ( mbPageTransferablePersistent == FALSE ),
        // don't offer any formats => it's just for internal puposes
        mbPageTransferable = TRUE;
        mbPageTransferablePersistent = bPersistent;
    }
}

// -----------------------------------------------------------------------------

sal_Int64 SAL_CALL SdTransferable::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) &&
        ( 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
        nRet = sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    else
    {
        nRet = 0;
    }

    return nRet;
}

// -----------------------------------------------------------------------------

const ::com::sun::star::uno::Sequence< sal_Int8 >& SdTransferable::getUnoTunnelId()
{
    static ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;

    if( !aSeq.getLength() )
    {
        static osl::Mutex   aCreateMutex;
        osl::MutexGuard     aGuard( aCreateMutex );

        aSeq.realloc( 16 );
        rtl_createUuid( reinterpret_cast< sal_uInt8* >( aSeq.getArray() ), 0, sal_True );
    }

    return aSeq;
}

// -----------------------------------------------------------------------------

SdTransferable* SdTransferable::getImplementation( const Reference< XInterface >& rxData ) throw()
{
    try
    {
        Reference< ::com::sun::star::lang::XUnoTunnel > xUnoTunnel( rxData, UNO_QUERY_THROW );
        return reinterpret_cast<SdTransferable*>(sal::static_int_cast<sal_uIntPtr>(xUnoTunnel->getSomething( SdTransferable::getUnoTunnelId()) ) );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
    return NULL;
}

// -----------------------------------------------------------------------------

// SfxListener
void SdTransferable::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SdrHint* pSdrHint = dynamic_cast< const SdrHint* >( &rHint );
    if( pSdrHint )
    {
        if( HINT_MODELCLEARED == pSdrHint->GetKind() )
        {
            EndListening(*mpSourceDoc);
            mpSourceDoc = 0;
        }
    }
    else
    {
        const SfxSimpleHint* pSimpleHint = dynamic_cast< const SfxSimpleHint * >(&rHint);
        if(pSimpleHint && (pSimpleHint->GetId() == SFX_HINT_DYING) )
        {
            if( &rBC == mpSourceDoc )
                mpSourceDoc = 0;
            if( &rBC == mpSdViewIntern )
                mpSdViewIntern = 0;
            if( &rBC == mpSdView )
                mpSdView = 0;
        }
    }
}

sal_Bool SdTransferable::SetTableRTF( SdDrawDocument* pModel, const DataFlavor& rFlavor)
{
    if ( pModel )
    {
        SdrPage* pPage = pModel->GetPage(0);
        if (pPage && pPage->GetObjCount() == 1 )
        {
            sdr::table::SdrTableObj* pTableObj = dynamic_cast< sdr::table::SdrTableObj* >( pPage->GetObj(0) );
            if( pTableObj )
            {
                SvMemoryStream aMemStm( 65535, 65535 );
                sdr::table::SdrTableObj::ExportAsRTF( aMemStm, *pTableObj );
                return SetAny( Any( Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aMemStm.GetData() ), aMemStm.Seek( STREAM_SEEK_TO_END ) ) ), rFlavor );
            }
        }
    }

    return sal_False;
}
