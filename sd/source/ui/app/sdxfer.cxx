/*************************************************************************
 *
 *  $RCSfile: sdxfer.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:57:44 $
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

#define ITEMID_FIELD EE_FEATURE_FIELD

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
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
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
#ifndef _SVDETC_HXX //autogen
#include <svx/svdetc.hxx>
#endif

#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#include <so3/outplace.hxx>
#include <svx/unomodel.hxx>
#include <svx/svditer.hxx>

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

// --------------
// - Namespaces -
// --------------

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

SdTransferable::SdTransferable( SdDrawDocument* pSrcDoc, ::sd::View* pWorkView, BOOL bInitOnGetData ) :
    pObjDesc( NULL ),
    pPageDocShell( NULL ),
    pSourceDoc( pSrcDoc ),
    pSdViewIntern( pWorkView ),
    pSdView( pWorkView ),
    pSdDrawDocumentIntern( NULL ),
    pSdDrawDocument( NULL ),
    pVDev( NULL ),
    bInternalMove( FALSE ),
    bOwnView( FALSE ),
    bOwnDocument( FALSE ),
    pOLEDataHelper( NULL ),
    pBookmark( NULL ),
    pGraphic( NULL ),
    pImageMap( NULL ),
    bLateInit( bInitOnGetData ),
    bPageTransferable( FALSE ),
    bPageTransferablePersistent( FALSE ),
    mbIsUnoObj( false )
{
    if( !bLateInit )
        CreateData();
}

// -----------------------------------------------------------------------------

SdTransferable::~SdTransferable()
{
    Application::GetSolarMutex().acquire();

    ObjectReleased();

    for( void* p = aPageBookmarks.First(); p; p = aPageBookmarks.Next() )
        delete static_cast< String* >( p );

    if( bOwnView )
        delete pSdViewIntern;

    if( bOwnDocument )
        delete pSdDrawDocumentIntern;

    delete pOLEDataHelper;

    if( aDocShellRef.Is() )
    {
        SvEmbeddedObject* pObj = aDocShellRef;
        ::sd::DrawDocShell* pDocSh = static_cast< ::sd::DrawDocShell*>(pObj);
        pDocSh->DoClose();
    }

    aDocShellRef.Clear();

    delete pGraphic;
    delete pBookmark;
    delete pImageMap;

    delete pVDev;
    delete pObjDesc;

    Application::GetSolarMutex().release();
}

// -----------------------------------------------------------------------------

void SdTransferable::CreateObjectReplacement( SdrObject* pObj )
{
    if( pObj )
    {
        UINT32 nInv = pObj->GetObjInventor();
        UINT16 nIdent = pObj->GetObjIdentifier();

        delete pOLEDataHelper, pOLEDataHelper = NULL;
        delete pGraphic, pGraphic = NULL;
        delete pBookmark, pBookmark = NULL;
        delete pImageMap, pImageMap = NULL;

        if( pObj->ISA( SdrOle2Obj ) )
        {
            const SvInPlaceObjectRef& rOldObjRef = static_cast< SdrOle2Obj* >( pObj )->GetObjRef();

            if( rOldObjRef.Is() )
                pOLEDataHelper = new TransferableDataHelper( rOldObjRef->CreateTransferableSnapshot() );
        }
        else if( pObj->ISA( SdrGrafObj ) && !pSourceDoc->GetAnimationInfo( pObj ) )
        {
            pGraphic = new Graphic( static_cast< SdrGrafObj* >( pObj )->GetTransformedGraphic() );
        }
        else if( pObj->IsUnoObj() && FmFormInventor == pObj->GetObjInventor() && ( nIdent == (UINT16) OBJ_FM_BUTTON ) )
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

                    pBookmark = new INetBookmark( String( aURL ), String( aLabel ) );
                }
            }
        }
        else if( pObj->ISA( SdrTextObj ) )
        {
            const OutlinerParaObject* pPara;

            if( pPara = static_cast< SdrTextObj* >( pObj )->GetOutlinerParaObject() )
            {
                const SvxFieldItem* pField;

                if( pField = pPara->GetTextObject().GetField() )
                {
                    const SvxFieldData* pData = pField->GetField();

                    if( pData && pData->ISA( SvxURLField ) )
                    {
                        const SvxURLField* pURL = (SvxURLField*) pData;

                        pBookmark = new INetBookmark( pURL->GetURL(), pURL->GetRepresentation() );
                    }
                }
            }
        }

        SdIMapInfo* pInfo = static_cast< SdDrawDocument* >( pObj->GetModel() )->GetIMapInfo( static_cast< SdrObject* >( pObj ) );

        if( pInfo )
            pImageMap = new ImageMap( pInfo->GetImageMap() );

        mbIsUnoObj = pObj && pObj->IsUnoObj();
    }
}

// -----------------------------------------------------------------------------

void SdTransferable::CreateData()
{
    if( pSdDrawDocument && !pSdViewIntern )
    {
        bOwnView = TRUE;

        SdPage* pPage = pSdDrawDocument->GetSdPage(0, PK_STANDARD);

        if( 1 == pPage->GetObjCount() )
            CreateObjectReplacement( pPage->GetObj( 0 ) );

        pVDev = new VirtualDevice( *Application::GetDefaultDevice() );
        pVDev->SetMapMode( MapMode( pSdDrawDocumentIntern->GetScaleUnit(), Point(), pSdDrawDocumentIntern->GetScaleFraction(), pSdDrawDocumentIntern->GetScaleFraction() ) );
        pSdViewIntern = new ::sd::View( pSdDrawDocumentIntern, pVDev );
        pSdViewIntern->EndListening(*pSdDrawDocumentIntern );
        pSdViewIntern->SetMarkHdlHidden( TRUE );
        SdrPageView* pPageView = pSdViewIntern->ShowPage(pPage, Point());
        ((SdrMarkView*)pSdViewIntern)->MarkAll(pPageView);
    }
    else if( pSdView && !pSdDrawDocumentIntern )
    {
        bOwnDocument = TRUE;

        const SdrMarkList& rMarkList = pSdView->GetMarkedObjectList();

        if( rMarkList.GetMarkCount() == 1 )
            CreateObjectReplacement( rMarkList.GetMark( 0 )->GetObj() );

        pSdDrawDocumentIntern = (SdDrawDocument*) pSdView->GetAllMarkedModel();

        if( !aDocShellRef.Is() && pSdDrawDocumentIntern->GetDocSh() )
        {
            // DocShell schon vorhanden ( AllocModel() )
            aDocShellRef = pSdDrawDocumentIntern->GetDocSh();
            bOwnDocument = FALSE;
        }

        // Groesse der Source-Seite uebernehmen
        SdrPageView*        pPgView = pSdView->GetPageViewPvNum( 0 );
        SdPage*             pOldPage = (SdPage*) pPgView->GetPage();
        SdrModel*           pOldModel = pSdView->GetModel();
        SdStyleSheetPool*   pOldStylePool = (SdStyleSheetPool*) pOldModel->GetStyleSheetPool();
        SdStyleSheetPool*   pNewStylePool = (SdStyleSheetPool*) pSdDrawDocumentIntern->GetStyleSheetPool();
        SdPage*             pPage = pSdDrawDocumentIntern->GetSdPage( 0, PK_STANDARD );
        String              aOldLayoutName( pOldPage->GetLayoutName() );

        pPage->SetSize( pOldPage->GetSize() );
        pPage->SetLayoutName( aOldLayoutName );
        pNewStylePool->CopyGraphicSheets( *pOldStylePool );
        aOldLayoutName.Erase( aOldLayoutName.SearchAscii( SD_LT_SEPARATOR ) );
        pNewStylePool->CopyLayoutSheets( aOldLayoutName, *pOldStylePool );
    }

    // set VisArea and adjust objects if neccessary
    if( aVisArea.IsEmpty() &&
        pSdDrawDocumentIntern && pSdViewIntern &&
        pSdDrawDocumentIntern->GetPageCount() )
    {
        SdPage* pPage = pSdDrawDocumentIntern->GetSdPage( 0, PK_STANDARD );

        if( 1 == pSdDrawDocumentIntern->GetPageCount() )
        {
            Point   aOrigin( ( aVisArea = pSdViewIntern->GetAllMarkedRect() ).TopLeft() );
            Size    aVector( -aOrigin.X(), -aOrigin.Y() );

            for( ULONG nObj = 0, nObjCount = pPage->GetObjCount(); nObj < nObjCount; nObj++ )
            {
                SdrObject* pObj = pPage->GetObj( nObj );
                pObj->NbcMove( aVector );
            }
        }
        else
            aVisArea.SetSize( pPage->GetSize() );

        // Die Ausgabe soll am Nullpunkt erfolgen
        aVisArea.SetPos( Point() );
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

void SdTransferable::AddSupportedFormats()
{
    if( !bPageTransferable || bPageTransferablePersistent )
    {
        if( !bLateInit )
            CreateData();

        if( pObjDesc )
            AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );

        if( pOLEDataHelper )
        {
            AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );

            DataFlavorExVector              aVector( pOLEDataHelper->GetDataFlavorExVector() );
            DataFlavorExVector::iterator    aIter( aVector.begin() ), aEnd( aVector.end() );

            while( aIter != aEnd )
                AddFormat( *aIter++ );
        }
        else if( pGraphic )
        {
            AddFormat( SOT_FORMATSTR_ID_SVXB );

            if( pGraphic->GetType() == GRAPHIC_BITMAP )
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
        else if( pBookmark )
        {
            AddFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
            AddFormat( FORMAT_STRING );
        }
        else
        {
            AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
            AddFormat( SOT_FORMATSTR_ID_DRAWING );
            if( !pSdDrawDocument || !lcl_HasOnlyControls( pSdDrawDocument ) )
            {
                AddFormat( SOT_FORMAT_GDIMETAFILE );
                AddFormat( SOT_FORMAT_BITMAP );
            }
        }

        if( pImageMap )
            AddFormat( SOT_FORMATSTR_ID_SVIM );
    }
}

// -----------------------------------------------------------------------------

sal_Bool SdTransferable::GetData( const DataFlavor& rFlavor )
{
    sal_uInt32  nFormat = SotExchange::GetFormat( rFlavor );
    sal_Bool    bOK = sal_False;

    CreateData();

    if( pOLEDataHelper && pOLEDataHelper->HasFormat( rFlavor ) )
    {
        ULONG nOldSwapMode;

        if( pSdDrawDocumentIntern )
        {
            nOldSwapMode = pSdDrawDocumentIntern->GetSwapGraphicsMode();
            pSdDrawDocumentIntern->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_PURGE );
        }

        bOK = SetAny( pOLEDataHelper->GetAny( rFlavor ), rFlavor );

        if( pSdDrawDocumentIntern )
            pSdDrawDocumentIntern->SetSwapGraphicsMode( nOldSwapMode );
    }
    else if( HasFormat( nFormat ) )
    {
        if( ( nFormat == SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR || nFormat == SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) && pObjDesc )
        {
            bOK = SetTransferableObjectDescriptor( *pObjDesc, rFlavor );
        }
        else if( nFormat == SOT_FORMATSTR_ID_DRAWING )
        {
            SdDrawDocument* pDoc = (SdDrawDocument*) pSdViewIntern->GetAllMarkedModel();
            bOK = SetObject( pDoc, SDTRANSFER_OBJECTTYPE_DRAWMODEL, rFlavor );
            delete pDoc;
        }
        else if( nFormat == FORMAT_GDIMETAFILE )
        {
            bOK = SetGDIMetaFile( pSdViewIntern->GetAllMarkedMetaFile( TRUE ), rFlavor );
        }
        else if( nFormat == FORMAT_BITMAP )
        {
            bOK = SetBitmap( pSdViewIntern->GetAllMarkedBitmap( TRUE ), rFlavor );
        }
        else if( ( nFormat == FORMAT_STRING ) && pBookmark )
        {
            bOK = SetString( pBookmark->GetURL(), rFlavor );
        }
        else if( ( nFormat == SOT_FORMATSTR_ID_SVXB ) && pGraphic )
        {
            bOK = SetGraphic( *pGraphic, rFlavor );
        }
        else if( ( nFormat == SOT_FORMATSTR_ID_SVIM ) && pImageMap )
        {
            bOK = SetImageMap( *pImageMap, rFlavor );
        }
        else if( pBookmark )
        {
            bOK = SetINetBookmark( *pBookmark, rFlavor );
        }
        else if( nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE )
        {
            ULONG nOldSwapMode;

            if( pSdDrawDocumentIntern )
            {
                nOldSwapMode = pSdDrawDocumentIntern->GetSwapGraphicsMode();
                pSdDrawDocumentIntern->SetSwapGraphicsMode( SDR_SWAPGRAPHICSMODE_PURGE );
            }

            if( !aDocShellRef.Is() )
            {
                aDocShellRef = new ::sd::DrawDocShell(
                    pSdDrawDocumentIntern,
                    SFX_CREATE_MODE_EMBEDDED,
                    TRUE,
                    pSdDrawDocumentIntern->GetDocumentType());
                bOwnDocument = FALSE;
                aDocShellRef->DoInitNew( NULL );
            }

            aDocShellRef->SetVisArea( aVisArea );
            bOK = SetObject( &aDocShellRef, SDTRANSFER_OBJECTTYPE_DRAWOLE, rFlavor );

            if( pSdDrawDocumentIntern )
                pSdDrawDocumentIntern->SetSwapGraphicsMode( nOldSwapMode );
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

sal_Bool SdTransferable::WriteObject( SotStorageStreamRef& rxOStm, void* pObject, sal_uInt32 nObjectType, const DataFlavor& rFlavor )
{
    sal_Bool bRet = sal_False;

    switch( nObjectType )
    {
        case( SDTRANSFER_OBJECTTYPE_DRAWMODEL ):
        {
            try
            {
                SdDrawDocument* pDoc = (SdDrawDocument*) pObject;
                pDoc->BurnInStyleSheetAttributes();
                pDoc->SetStreamingSdrModel( TRUE );
                pDoc->RemoveNotPersistentObjects( TRUE );
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

                pDoc->SetStreamingSdrModel( FALSE );

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
            SvEmbeddedObject*   pEmbObj = (SvEmbeddedObject*) pObject;
            ::utl::TempFile     aTempFile;
            SvStorageRef        xWorkStore( new SvStorage( TRUE, aTempFile.GetURL() ) );

            aTempFile.EnableKillingFile();

            // write document storage
            xWorkStore->SetVersion( SOFFICE_FILEFORMAT_60 );
            pEmbObj->SetupStorage( xWorkStore );
            bRet = pEmbObj->DoSaveAs( xWorkStore );
            pEmbObj->DoSaveCompleted();
            xWorkStore->Commit();
            xWorkStore.Clear();

            SvStream* pSrcStm = ::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), STREAM_READ );

            if( pSrcStm )
            {
                rxOStm->SetBufferSize( 0xff00 );
                *rxOStm << *pSrcStm;
                rxOStm->Commit();
                delete pSrcStm;
            }

            bRet = ( rxOStm->GetError() == ERRCODE_NONE );
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
    if( pSdView )
        ( (::sd::View*) pSdView )->DragFinished( nDropAction );
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
    delete pObjDesc;
    pObjDesc = new TransferableObjectDescriptor( rObjDesc );
}

// -----------------------------------------------------------------------------

void SdTransferable::SetPageBookmarks( const List& rPageBookmarks, BOOL bPersistent )
{
    if( pSourceDoc )
    {
        if( pSdViewIntern )
            pSdViewIntern->HideAllPages();

        // #116168#
        pSdDrawDocument->ClearModel(sal_False);

        pPageDocShell = NULL;

        for( void* p = aPageBookmarks.First(); p; p = aPageBookmarks.Next() )
            delete static_cast< String* >( p );

        if( bPersistent )
        {
            pSdDrawDocument->CreateFirstPages();
            pSdDrawDocument->InsertBookmarkAsPage( const_cast< List* >( &rPageBookmarks ), NULL, FALSE, TRUE, 1, TRUE, pSourceDoc->GetDocSh(), TRUE, TRUE, FALSE );
        }
        else
        {
            pPageDocShell = pSourceDoc->GetDocSh();

            for( ULONG i = 0; i < rPageBookmarks.Count(); i++ )
                aPageBookmarks.Insert( new String( *static_cast< String* >( rPageBookmarks.GetObject( i ) ) ), LIST_APPEND );
        }

        if( pSdViewIntern && pSdDrawDocument )
        {
            SdPage* pPage = pSdDrawDocument->GetSdPage( 0, PK_STANDARD );

            if( pPage )
            {
                ( (SdrMarkView*) pSdViewIntern )->MarkAll( (SdrPageView*) pSdViewIntern->ShowPage( pPage, Point() ) );
            }
        }

        // set flags for page transferable; if ( bPageTransferablePersistent == FALSE ),
        // don't offer any formats => it's just for internal puposes
        bPageTransferable = TRUE;
        bPageTransferablePersistent = bPersistent;
    }
}

// -----------------------------------------------------------------------------

sal_Int64 SAL_CALL SdTransferable::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException )
{
    sal_Int64 nRet;

    if( ( rId.getLength() == 16 ) &&
        ( 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
        nRet = (sal_Int64) this;
    }
    else
        nRet = 0;

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
        Reference< ::com::sun::star::lang::XUnoTunnel > xUnoTunnel( rxData, UNO_QUERY );

        return( xUnoTunnel.is() ?
                ( (SdTransferable*)(void*) xUnoTunnel->getSomething( SdTransferable::getUnoTunnelId() ) ) :
                NULL );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        return NULL;
    }
}
