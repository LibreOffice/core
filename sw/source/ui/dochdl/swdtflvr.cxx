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

#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XEmbedObjectClipboardCreator.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>

#include <svtools/embedtransfer.hxx>
#include <svtools/insdlg.hxx>
#include <unotools/tempfile.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/string.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <sot/filelist.hxx>
#include <svx/svxdlg.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <osl/endian.h>
#include <sfx2/linkmgr.hxx>
#include <tools/urlobj.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <svtools/imap.hxx>
#include <sot/storage.hxx>
#include <vcl/graph.hxx>
#include <svl/urihelper.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xexch.hxx>
#include <svx/xmlexchg.hxx>
#include <svx/dbaexchange.hxx>
#include <svx/clipfmtitem.hxx>
#include <sfx2/mieclip.hxx>
#include <svx/svdetc.hxx>
#include <svx/xoutbmp.hxx>
#include <svl/urlbmk.hxx>
#include <svtools/htmlout.hxx>
#include <svx/hlnkitem.hxx>
#include <svtools/inetimg.hxx>
#include <editeng/paperinf.hxx>
#include <svx/fmview.hxx>
#include <editeng/scripttypeitem.hxx>
#include <sfx2/docfilt.hxx>
#include <svtools/imapobj.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <unotools/streamwrap.hxx>
#include <svtools/filter.hxx>

#include <svx/unomodel.hxx>
#include <fmturl.hxx>
#include <fmtinfmt.hxx>
#include <fmtfsize.hxx>
#include <swdtflvr.hxx>
#include <shellio.hxx>
#include <ddefld.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <pagedesc.hxx>
#include <IMark.hxx>
#include <docary.hxx>
#include <section.hxx>
#include <ndtxt.hxx>
#include <edtwin.hxx>
#include <navicont.hxx>
#include <swcont.hxx>
#include <wrtsh.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <wdocsh.hxx>
#include <fldbas.hxx>       //DDE
#include <swundo.hxx>       // for Undo-Ids
#include <pam.hxx>
#include <ndole.hxx>
#include <swwait.hxx>
#include <viewopt.hxx>
#include <swerror.h>
#include <SwCapObjType.hxx>
#include <cmdid.h>
#include <dochdl.hrc>
#include <comcore.hrc> // #111827#
#include <sot/stg.hxx>
#include <svx/svditer.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/svdpage.hxx>
#include <avmedia/mediawindow.hxx>
#include <swcrsr.hxx>
#include <SwRewriter.hxx>
#include <globals.hrc>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <swserv.hxx>
#include <switerator.hxx>

extern sal_Bool bFrmDrag;
extern sal_Bool bDDINetAttr;
extern sal_Bool bExecuteDrag;


#define OLESIZE 11905 - 2 * lMinBorder, 6 * MM50

#define SWTRANSFER_OBJECTTYPE_DRAWMODEL         0x00000001
#define SWTRANSFER_OBJECTTYPE_HTML              0x00000002
#define SWTRANSFER_OBJECTTYPE_RTF               0x00000004
#define SWTRANSFER_OBJECTTYPE_STRING            0x00000008
#define SWTRANSFER_OBJECTTYPE_SWOLE             0x00000010
#define SWTRANSFER_OBJECTTYPE_DDE               0x00000020

#define SWTRANSFER_GRAPHIC_INSERTED             0x00000040

using namespace ::svx;
using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer;
using namespace nsTransferBufferType;

#define DDE_TXT_ENCODING    osl_getThreadTextEncoding()

class SwTrnsfrDdeLink : public ::sfx2::SvBaseLink
{
    String sName;
    ::sfx2::SvLinkSourceRef refObj;
    SwTransferable& rTrnsfr;
    SwDocShell* pDocShell;
    sal_uLong nOldTimeOut;
    sal_Bool bDelBookmrk : 1;
    sal_Bool bInDisconnect : 1;

    sal_Bool FindDocShell();

    using sfx2::SvBaseLink::Disconnect;

protected:
    virtual ~SwTrnsfrDdeLink();

public:
    SwTrnsfrDdeLink( SwTransferable& rTrans, SwWrtShell& rSh );

    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const String& rMimeType, const ::com::sun::star::uno::Any & rValue );
    virtual void Closed();

    sal_Bool WriteData( SvStream& rStrm );

    void Disconnect( sal_Bool bRemoveDataAdvise );
};

// helper class for Action and Undo enclosing
class SwTrnsfrActionAndUndo
{
    SwWrtShell *pSh;
    SwUndoId eUndoId;
public:
    SwTrnsfrActionAndUndo( SwWrtShell *pS, SwUndoId nId,
                           const SwRewriter * pRewriter = 0,
                           sal_Bool bDelSel = sal_False)
        : pSh( pS ), eUndoId( nId )
    {
        pSh->StartUndo( eUndoId, pRewriter );
        if( bDelSel )
            pSh->DelRight();
        pSh->StartAllAction();
    }
    ~SwTrnsfrActionAndUndo()
    {
        pSh->EndUndo();
        pSh->EndAllAction();
    }
};

SwTransferable::SwTransferable( SwWrtShell& rSh )
    : pWrtShell( &rSh ),
    pCreatorView( 0 ),
    pClpDocFac( 0 ),
    pClpGraphic( 0 ),
    pClpBitmap( 0 ),
    pOrigGrf( 0 ),
    pBkmk( 0 ),
    pImageMap( 0 ),
    pTargetURL( 0 ),
    eBufferType( TRNSFR_NONE )
{
    rSh.GetView().AddTransferable(*this);
    SwDocShell* pDShell = rSh.GetDoc()->GetDocShell();
    if( pDShell )
    {
        pDShell->FillTransferableObjectDescriptor( aObjDesc );
        if( pDShell->GetMedium() )
        {
            const INetURLObject& rURLObj = pDShell->GetMedium()->GetURLObject();
            aObjDesc.maDisplayName = URIHelper::removePassword(
                                rURLObj.GetMainURL( INetURLObject::NO_DECODE ),
                                INetURLObject::WAS_ENCODED,
                                   INetURLObject::DECODE_UNAMBIGUOUS );
        }

        PrepareOLE( aObjDesc );
    }
}

SwTransferable::~SwTransferable()
{
    Application::GetSolarMutex().acquire();

    // the DDELink still needs the WrtShell!
    if( refDdeLink.Is() )
    {
        ((SwTrnsfrDdeLink*)&refDdeLink)->Disconnect( sal_True );
        refDdeLink.Clear();
    }

    pWrtShell = 0;

    // release reference to the document so that aDocShellRef will delete
    // it (if aDocShellRef is set). Otherwise, the OLE nodes keep references
    // to their sub-storage when the storage is already dead.
    delete pClpDocFac;

    // first close, then the Ref. can be cleared as well, so that
    // the DocShell really gets deleted!
    if( aDocShellRef.Is() )
    {
        SfxObjectShell * pObj = aDocShellRef;
        SwDocShell* pDocSh = (SwDocShell*)pObj;
        pDocSh->DoClose();
    }
    aDocShellRef.Clear();

    SwModule* pMod = SW_MOD();
    if(pMod)
    {
        if ( pMod->pDragDrop == this )
            pMod->pDragDrop = 0;
        else if ( pMod->pXSelection == this )
            pMod->pXSelection = 0;
    }

    delete pClpGraphic;
    delete pClpBitmap;
    delete pImageMap;
    delete pTargetURL;
    delete pBkmk;


    eBufferType = TRNSFR_NONE;

    Application::GetSolarMutex().release();
}

static SwDoc * lcl_GetDoc(SwDocFac & rDocFac)
{
    SwDoc *const pDoc = rDocFac.GetDoc();
    OSL_ENSURE( pDoc, "Document not found" );
    if (pDoc)
    {
        pDoc->SetClipBoard( true );
    }
    return pDoc;
}

void SwTransferable::ObjectReleased()
{
    SwModule *pMod = SW_MOD();
    if( this == pMod->pDragDrop )
        pMod->pDragDrop = 0;
    else if( this == pMod->pXSelection )
        pMod->pXSelection = 0;
}

void SwTransferable::AddSupportedFormats()
{
    // only need if we are the current XSelection Object
    SwModule *pMod = SW_MOD();
    if( this == pMod->pXSelection )
    {
        SetDataForDragAndDrop( Point( 0,0) );
    }
}

void SwTransferable::InitOle( SfxObjectShell* pDoc, SwDoc& rDoc )
{
    //set OleVisArea. Upper left corner of the page and size of
    //RealSize in Twips.
    const Size aSz( OLESIZE );
    SwRect aVis( Point( DOCUMENTBORDER, DOCUMENTBORDER ), aSz );
    pDoc->SetVisArea( aVis.SVRect() );
    rDoc.set(IDocumentSettingAccess::BROWSE_MODE, true );
}

uno::Reference < embed::XEmbeddedObject > SwTransferable::FindOLEObj( sal_Int64& nAspect ) const
{
    uno::Reference < embed::XEmbeddedObject > xObj;
    if( pClpDocFac )
    {
        SwIterator<SwCntntNode,SwFmtColl> aIter( *pClpDocFac->GetDoc()->GetDfltGrfFmtColl() );
        for( SwCntntNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
            if( ND_OLENODE == pNd->GetNodeType() )
            {
                xObj = ((SwOLENode*)pNd)->GetOLEObj().GetOleRef();
                nAspect = ((SwOLENode*)pNd)->GetAspect();
                break;
            }
    }
    return xObj;
}

Graphic* SwTransferable::FindOLEReplacementGraphic() const
{
    if( pClpDocFac )
    {
        SwIterator<SwCntntNode,SwFmtColl> aIter( *pClpDocFac->GetDoc()->GetDfltGrfFmtColl() );
        for( SwCntntNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
            if( ND_OLENODE == pNd->GetNodeType() )
            {
                return ((SwOLENode*)pNd)->GetGraphic();
            }
    }

    return NULL;
}

void SwTransferable::RemoveDDELinkFormat( const Window& rWin )
{
    RemoveFormat( SOT_FORMATSTR_ID_LINK );
    CopyToClipboard( (Window*)&rWin );
}

namespace
{
    //Resolves: fdo#40717 surely when we create a clipboard document we should
    //overwrite the clipboard documents styles and settings with that of the
    //source, so that we can WYSIWYG paste. If we want that the destinations
    //styles are used over the source styles, that's a matter of the
    //destination paste code to handle, not the source paste code.
    void lclOverWriteDoc(SwWrtShell &rSrcWrtShell, SwDoc &rDest)
    {
        const SwDoc &rSrc = *rSrcWrtShell.GetDoc();

        rDest.ReplaceCompatabilityOptions(rSrc);
        rDest.ReplaceDefaults(rSrc);

        //It would probably make most sense here to only insert the styles used
        //by the selection, e.g. apply SwDoc::IsUsed on styles ?
        rDest.ReplaceStyles(rSrc, false);

        rSrcWrtShell.Copy(&rDest);
    }
}

sal_Bool SwTransferable::GetData( const DATA_FLAVOR& rFlavor )
{
    sal_uInt32  nFormat = SotExchange::GetFormat( rFlavor );

    // we can only fullfil the request if
    // 1) we have data for this format
    // 2) we have either a clipboard document (pClpDocFac), or
    //    we have a SwWrtShell (so we can generate a new clipboard document)
    if( !HasFormat( nFormat ) || ( pClpDocFac == NULL && pWrtShell == NULL ) )
        return sal_False;

    if( !pClpDocFac )
    {
        SelectionType nSelectionType = pWrtShell->GetSelectionType();

// SEL_GRF comes from ContentType of editsh
        if( (nsSelectionType::SEL_GRF | nsSelectionType::SEL_DRW_FORM) & nSelectionType )
        {
            pClpGraphic = new Graphic;
            if( !pWrtShell->GetDrawObjGraphic( FORMAT_GDIMETAFILE, *pClpGraphic ))
                pOrigGrf = pClpGraphic;
            pClpBitmap = new Graphic;
            if( !pWrtShell->GetDrawObjGraphic( FORMAT_BITMAP, *pClpBitmap ))
                pOrigGrf = pClpBitmap;

            // is it an URL-Button ?
            String sURL, sDesc;
            if( pWrtShell->GetURLFromButton( sURL, sDesc ) )
            {
                pBkmk = new INetBookmark( sURL, sDesc );
                eBufferType = TRNSFR_INETFLD;
            }
        }

        pClpDocFac = new SwDocFac;
        SwDoc *const pTmpDoc = lcl_GetDoc(*pClpDocFac);

        pTmpDoc->LockExpFlds();     // never update fields - leave text as it is
        lclOverWriteDoc(*pWrtShell, *pTmpDoc);

        // in CORE a new one was created (OLE-Objekte copied!)
        aDocShellRef = pTmpDoc->GetTmpDocShell();
        if( aDocShellRef.Is() )
            SwTransferable::InitOle( aDocShellRef, *pTmpDoc );
        pTmpDoc->SetTmpDocShell( (SfxObjectShell*)NULL );

        if( nSelectionType & nsSelectionType::SEL_TXT && !pWrtShell->HasMark() )
        {
            SwContentAtPos aCntntAtPos( SwContentAtPos::SW_INETATTR );

            Point aPos( SwEditWin::GetDDStartPosX(), SwEditWin::GetDDStartPosY());

            sal_Bool bSelect = bExecuteDrag &&
                            pWrtShell->GetView().GetDocShell() &&
                            !pWrtShell->GetView().GetDocShell()->IsReadOnly();
            if( pWrtShell->GetContentAtPos( aPos, aCntntAtPos, bSelect ) )
            {
                pBkmk = new INetBookmark(
                        ((SwFmtINetFmt*)aCntntAtPos.aFnd.pAttr)->GetValue(),
                        aCntntAtPos.sStr );
                eBufferType = TRNSFR_INETFLD;
                if( bSelect )
                    pWrtShell->SelectTxtAttr( RES_TXTATR_INETFMT );
            }
        }
        if( pWrtShell->IsFrmSelected() )
        {
             SfxItemSet aSet( pWrtShell->GetAttrPool(), RES_URL, RES_URL );
            pWrtShell->GetFlyFrmAttr( aSet );
            const SwFmtURL& rURL = (SwFmtURL&)aSet.Get( RES_URL );
            if( rURL.GetMap() )
                pImageMap = new ImageMap( *rURL.GetMap() );
            else if( rURL.GetURL().Len() )
                pTargetURL = new INetImage( aEmptyStr, rURL.GetURL(),
                                            rURL.GetTargetFrameName(),
                                            aEmptyStr, Size() );
        }
    }

    sal_Bool    bOK = sal_False;
    if( TRNSFR_OLE == eBufferType )
    {
        //TODO/MBA: testing - is this the "single OLE object" case?!
        // get OLE-Object from ClipDoc and get the data from that.
        sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT; // will be set in the next statement
        uno::Reference < embed::XEmbeddedObject > xObj = FindOLEObj( nAspect );
        Graphic* pOLEGraph = FindOLEReplacementGraphic();
        if( xObj.is() )
        {
            TransferableDataHelper aD( new SvEmbedTransferHelper( xObj, pOLEGraph, nAspect ) );
            uno::Any aAny( aD.GetAny( rFlavor ));
            if( aAny.hasValue() )
                bOK = SetAny( aAny, rFlavor );
        }

        // the following solution will be used in the case when the object can not generate the image
        // TODO/LATER: in future the transferhelper must probably be created based on object and the replacement stream
        if ( nFormat == SOT_FORMAT_GDIMETAFILE )
        {
            pOLEGraph = FindOLEReplacementGraphic();
            if ( pOLEGraph )
                bOK = SetGDIMetaFile( pOLEGraph->GetGDIMetaFile(), rFlavor );
        }
    }
    else
    {
        switch( nFormat )
        {
        case SOT_FORMATSTR_ID_LINK:
            if( refDdeLink.Is() )
                bOK = SetObject( &refDdeLink,
                                    SWTRANSFER_OBJECTTYPE_DDE, rFlavor );
            break;

        case SOT_FORMATSTR_ID_OBJECTDESCRIPTOR:
        case SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR:
            bOK = SetTransferableObjectDescriptor( aObjDesc, rFlavor );
            break;

        case SOT_FORMATSTR_ID_DRAWING:
            {
                SwDoc *const pDoc = lcl_GetDoc(*pClpDocFac);
                bOK = SetObject( pDoc->GetDrawModel(),
                                SWTRANSFER_OBJECTTYPE_DRAWMODEL, rFlavor );
            }
            break;

        case SOT_FORMAT_STRING:
        {
            SwDoc *const pDoc = lcl_GetDoc(*pClpDocFac);
            bOK = SetObject( pDoc, SWTRANSFER_OBJECTTYPE_STRING, rFlavor );
        }
        break;
        case SOT_FORMAT_RTF:
        {
            SwDoc *const pDoc = lcl_GetDoc(*pClpDocFac);
            bOK = SetObject( pDoc, SWTRANSFER_OBJECTTYPE_RTF, rFlavor );
        }
            break;

        case SOT_FORMATSTR_ID_HTML:
        {
            SwDoc *const pDoc = lcl_GetDoc(*pClpDocFac);
            bOK = SetObject( pDoc, SWTRANSFER_OBJECTTYPE_HTML, rFlavor );
        }
            break;

        case SOT_FORMATSTR_ID_SVXB:
            if( eBufferType & TRNSFR_GRAPHIC && pOrigGrf )
                bOK = SetGraphic( *pOrigGrf, rFlavor );
            break;

        case SOT_FORMAT_GDIMETAFILE:
            if( eBufferType & TRNSFR_GRAPHIC )
                bOK = SetGDIMetaFile( pClpGraphic->GetGDIMetaFile(), rFlavor );
            break;
        case SOT_FORMAT_BITMAP:
            // Neither pClpBitmap nor pClpGraphic are necessarily set
            if( (eBufferType & TRNSFR_GRAPHIC) && (pClpBitmap != 0 || pClpGraphic != 0))
                bOK = SetBitmap( (pClpBitmap ? pClpBitmap
                                             : pClpGraphic)->GetBitmap(),
                                 rFlavor );
            break;

        case SOT_FORMATSTR_ID_SVIM:
            if( pImageMap )
                bOK = SetImageMap( *pImageMap, rFlavor );
            break;

        case SOT_FORMATSTR_ID_INET_IMAGE:
            if( pTargetURL )
                bOK = SetINetImage( *pTargetURL, rFlavor );
            break;

        case SOT_FORMATSTR_ID_SOLK:
        case SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK:
        case SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR:
        case SOT_FORMATSTR_ID_FILECONTENT:
        case SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR:
        case SOT_FORMAT_FILE:
            if( (TRNSFR_INETFLD & eBufferType) && pBkmk )
                bOK = SetINetBookmark( *pBkmk, rFlavor );
            break;

        case SOT_FORMATSTR_ID_EMBED_SOURCE:
            if( !aDocShellRef.Is() )
            {
                SwDoc *const pDoc = lcl_GetDoc(*pClpDocFac);
                SwDocShell* pNewDocSh = new SwDocShell( pDoc,
                                         SFX_CREATE_MODE_EMBEDDED );
                aDocShellRef = pNewDocSh;
                aDocShellRef->DoInitNew( NULL );
                SwTransferable::InitOle( aDocShellRef, *pDoc );
            }
            bOK = SetObject( &aDocShellRef, SWTRANSFER_OBJECTTYPE_SWOLE,
                            rFlavor );
            break;
        }
    }
    return bOK;
}

sal_Bool SwTransferable::WriteObject( SotStorageStreamRef& xStream,
                                    void* pObject, sal_uInt32 nObjectType,
                                    const DATA_FLAVOR& /*rFlavor*/ )
{
    sal_Bool bRet = sal_False;
    WriterRef xWrt;

    switch( nObjectType )
    {
    case SWTRANSFER_OBJECTTYPE_DRAWMODEL:
        {
            // dont change the sequence of commands
            SdrModel *pModel = (SdrModel*)pObject;
            xStream->SetBufferSize( 16348 );

            // for the changed pool defaults from drawing layer pool set those
            // attributes as hard attributes to preserve them for saving
            const SfxItemPool& rItemPool = pModel->GetItemPool();
            const SvxFontHeightItem& rDefaultFontHeight = (const SvxFontHeightItem&)rItemPool.GetDefaultItem(EE_CHAR_FONTHEIGHT);

            // SW should have no MasterPages
            OSL_ENSURE(0L == pModel->GetMasterPageCount(), "SW with MasterPages (!)");

            for(sal_uInt16 a(0); a < pModel->GetPageCount(); a++)
            {
                const SdrPage* pPage = pModel->GetPage(a);
                SdrObjListIter aIter(*pPage, IM_DEEPNOGROUPS);

                while(aIter.IsMore())
                {
                    SdrObject* pObj = aIter.Next();
                    const SvxFontHeightItem& rItem = (const SvxFontHeightItem&)pObj->GetMergedItem(EE_CHAR_FONTHEIGHT);

                    if(rItem.GetHeight() == rDefaultFontHeight.GetHeight())
                    {
                        pObj->SetMergedItem(rDefaultFontHeight);
                    }
                }
            }

            {
                uno::Reference<io::XOutputStream> xDocOut( new utl::OOutputStreamWrapper( *xStream ) );
                if( SvxDrawingLayerExport( pModel, xDocOut ) )
                    xStream->Commit();
            }

            bRet = ERRCODE_NONE == xStream->GetError();
        }
        break;

    case SWTRANSFER_OBJECTTYPE_SWOLE:
        {
            SfxObjectShell*   pEmbObj = (SfxObjectShell*) pObject;
            try
            {
                ::utl::TempFile     aTempFile;
                aTempFile.EnableKillingFile();
                uno::Reference< embed::XStorage > xWorkStore =
                    ::comphelper::OStorageHelper::GetStorageFromURL( aTempFile.GetURL(), embed::ElementModes::READWRITE );

                // write document storage
                pEmbObj->SetupStorage( xWorkStore, SOFFICE_FILEFORMAT_CURRENT, sal_False );
                // mba: no BaseURL for clipboard
                SfxMedium aMedium( xWorkStore, String() );
                bRet = pEmbObj->DoSaveObjectAs( aMedium, sal_False );
                pEmbObj->DoSaveCompleted();

                uno::Reference< embed::XTransactedObject > xTransact( xWorkStore, uno::UNO_QUERY );
                if ( xTransact.is() )
                    xTransact->commit();

                SvStream* pSrcStm = ::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), STREAM_READ );
                if( pSrcStm )
                {
                    xStream->SetBufferSize( 0xff00 );
                    *xStream << *pSrcStm;
                    delete pSrcStm;
                }

                bRet = sal_True;

                xWorkStore->dispose();
                xWorkStore = uno::Reference < embed::XStorage >();
                xStream->Commit();
            }
            catch (const uno::Exception&)
            {
            }

            bRet = ( xStream->GetError() == ERRCODE_NONE );
        }
        break;


    case SWTRANSFER_OBJECTTYPE_DDE:
        {
            xStream->SetBufferSize( 1024 );
            SwTrnsfrDdeLink* pDdeLnk = (SwTrnsfrDdeLink*)pObject;
            if( pDdeLnk->WriteData( *xStream ) )
            {
                xStream->Commit();
                bRet = ERRCODE_NONE == xStream->GetError();
            }
        }
        break;

    case SWTRANSFER_OBJECTTYPE_HTML:
        GetHTMLWriter( aEmptyStr, String(), xWrt );
        break;

    case SWTRANSFER_OBJECTTYPE_RTF:
        GetRTFWriter( aEmptyStr, String(), xWrt );
        break;

    case SWTRANSFER_OBJECTTYPE_STRING:
        GetASCWriter( aEmptyStr, String(), xWrt );
        if( xWrt.Is() )
        {
            SwAsciiOptions aAOpt;
            aAOpt.SetCharSet( RTL_TEXTENCODING_UTF8 );
            xWrt->SetAsciiOptions( aAOpt );

            // no start char for clipboard
            xWrt->bUCS2_WithStartChar = sal_False;
        }
        break;
    }

    if( xWrt.Is() )
    {
        SwDoc* pDoc = (SwDoc*)pObject;
        xWrt->bWriteClipboardDoc = sal_True;
        xWrt->bWriteOnlyFirstTable = 0 != (TRNSFR_TABELLE & eBufferType);
        xWrt->SetShowProgress( sal_False );

#if defined(DEBUGPASTE)
        SvFileStream aPasteDebug(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "PASTEBUFFER.debug")), STREAM_WRITE|STREAM_TRUNC);
        SwWriter aDbgWrt( aPasteDebug, *pDoc );
        aDbgWrt.Write( xWrt );
#endif

        SwWriter aWrt( *xStream, *pDoc );
        if( !IsError( aWrt.Write( xWrt )) )
        {
            *xStream << '\0';               // terminate with a zero
            xStream->Commit();
            bRet = sal_True;
        }
    }

    return bRet;
}

int SwTransferable::Cut()
{
    int nRet = Copy( sal_True );
    if( nRet )
        DeleteSelection();
    return nRet;
}

void SwTransferable::DeleteSelection()
{
    if(!pWrtShell)
        return;
    // ask for type of selection before action-bracketing
    const int nSelection = pWrtShell->GetSelectionType();
    pWrtShell->StartUndo( UNDO_DELETE );
    if( ( nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL ) & nSelection )
        pWrtShell->IntelligentCut( nSelection );
    pWrtShell->DelRight();
    pWrtShell->EndUndo( UNDO_DELETE );
}

int SwTransferable::PrepareForCopy( sal_Bool bIsCut )
{
    int nRet = 1;
    if(!pWrtShell)
        return 0;

    String sGrfNm;
    const int nSelection = pWrtShell->GetSelectionType();
    if( nSelection == nsSelectionType::SEL_GRF )
    {
        pClpGraphic = new Graphic;
        if( !pWrtShell->GetDrawObjGraphic( FORMAT_GDIMETAFILE, *pClpGraphic ))
            pOrigGrf = pClpGraphic;
        pClpBitmap = new Graphic;
        if( !pWrtShell->GetDrawObjGraphic( FORMAT_BITMAP, *pClpBitmap ))
            pOrigGrf = pClpBitmap;

        pClpDocFac = new SwDocFac;
        SwDoc *const pDoc = lcl_GetDoc(*pClpDocFac);
        pWrtShell->Copy( pDoc );

        if (pOrigGrf && !pOrigGrf->GetBitmap().IsEmpty())
          AddFormat( SOT_FORMATSTR_ID_SVXB );

        PrepareOLE( aObjDesc );
        AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );

        const Graphic* pGrf = pWrtShell->GetGraphic();
        if( pGrf && pGrf->IsSupportedGraphic() )
        {
            AddFormat( FORMAT_GDIMETAFILE );
            AddFormat( FORMAT_BITMAP );
        }
        eBufferType = TRNSFR_GRAPHIC;
        pWrtShell->GetGrfNms( &sGrfNm, 0 );
    }
    else if ( nSelection == nsSelectionType::SEL_OLE )
    {
        pClpDocFac = new SwDocFac;
        SwDoc *const pDoc = lcl_GetDoc(*pClpDocFac);
        aDocShellRef = new SwDocShell( pDoc, SFX_CREATE_MODE_EMBEDDED);
        aDocShellRef->DoInitNew( NULL );
        pWrtShell->Copy( pDoc );

        AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );

        // --> OD #i98753#
        // set size of embedded object at the object description structure
        aObjDesc.maSize = OutputDevice::LogicToLogic( pWrtShell->GetObjSize(), MAP_TWIP, MAP_100TH_MM );
        // <--
        PrepareOLE( aObjDesc );
        AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );

        AddFormat( FORMAT_GDIMETAFILE );
        eBufferType = TRNSFR_OLE;
    }
    // Is there anything to provide anyway?
    else if ( pWrtShell->IsSelection() || pWrtShell->IsFrmSelected() ||
              pWrtShell->IsObjSelected() )
    {
        SwWait *pWait = 0;
        if( pWrtShell->ShouldWait() )
            pWait = new SwWait( *pWrtShell->GetView().GetDocShell(), sal_True );

        pClpDocFac = new SwDocFac;

        // create additional cursor so that equal treatment of keyboard
        // and mouse selection is possible.
        // In AddMode with keyboard selection, the new cursor is not created
        // before the cursor is moved after end of selection.
        if( pWrtShell->IsAddMode() && pWrtShell->SwCrsrShell::HasSelection() )
            pWrtShell->CreateCrsr();

        SwDoc *const pTmpDoc = lcl_GetDoc(*pClpDocFac);

        pTmpDoc->LockExpFlds();     // nie die Felder updaten - Text so belassen
        lclOverWriteDoc(*pWrtShell, *pTmpDoc);

        {
            IDocumentMarkAccess* const pMarkAccess = pTmpDoc->getIDocumentMarkAccess();
            ::std::vector< ::sw::mark::IMark* > vDdeMarks;
            // find all DDE-Bookmarks
            for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getMarksBegin();
                ppMark != pMarkAccess->getMarksEnd();
                ++ppMark)
            {
                if(IDocumentMarkAccess::DDE_BOOKMARK == IDocumentMarkAccess::GetType(**ppMark))
                    vDdeMarks.push_back(ppMark->get());
            }
            // remove all DDE-Bookmarks, they are invalid inside the clipdoc!
            for(::std::vector< ::sw::mark::IMark* >::iterator ppMark = vDdeMarks.begin();
                ppMark != vDdeMarks.end();
                ++ppMark)
                pMarkAccess->deleteMark(*ppMark);
        }

        // a new one was created in CORE (OLE-Objekte copied!)
        aDocShellRef = pTmpDoc->GetTmpDocShell();
        if( aDocShellRef.Is() )
            SwTransferable::InitOle( aDocShellRef, *pTmpDoc );
        pTmpDoc->SetTmpDocShell( (SfxObjectShell*)NULL );

        if( pWrtShell->IsObjSelected() )
            eBufferType = TRNSFR_DRAWING;
        else
        {
            eBufferType = TRNSFR_DOCUMENT;
            if (pWrtShell->IntelligentCut(nSelection, sal_False) != SwWrtShell::NO_WORD)
                eBufferType = (TransferBufferType)(TRNSFR_DOCUMENT_WORD | eBufferType);
        }

        int bDDELink = pWrtShell->IsSelection();
        if( nSelection & nsSelectionType::SEL_TBL_CELLS )
        {
            eBufferType = (TransferBufferType)(TRNSFR_TABELLE | eBufferType);
            bDDELink = pWrtShell->HasWholeTabSelection();
        }

        //When someone needs it, we 'OLE' him something
        AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );

        //put RTF ahead of  the OLE's Metafile to have less loss
        if( !pWrtShell->IsObjSelected() )
        {
            AddFormat( FORMAT_RTF );
            AddFormat( SOT_FORMATSTR_ID_HTML );
        }
        if( pWrtShell->IsSelection() )
            AddFormat( FORMAT_STRING );

        if( nSelection & ( nsSelectionType::SEL_DRW | nsSelectionType::SEL_DRW_FORM ))
        {
            AddFormat( SOT_FORMATSTR_ID_DRAWING );
            if ( nSelection & nsSelectionType::SEL_DRW )
            {
                AddFormat( FORMAT_GDIMETAFILE );
                AddFormat( FORMAT_BITMAP );
            }
            eBufferType = (TransferBufferType)( TRNSFR_GRAPHIC | eBufferType );

            pClpGraphic = new Graphic;
            if( !pWrtShell->GetDrawObjGraphic( FORMAT_GDIMETAFILE, *pClpGraphic ))
                pOrigGrf = pClpGraphic;
            pClpBitmap = new Graphic;
            if( !pWrtShell->GetDrawObjGraphic( FORMAT_BITMAP, *pClpBitmap ))
                pOrigGrf = pClpBitmap;

            // is it an URL-Button ?
            String sURL, sDesc;
            if( pWrtShell->GetURLFromButton( sURL, sDesc ) )
            {
                AddFormat( FORMAT_STRING );
                 AddFormat( SOT_FORMATSTR_ID_SOLK );
                 AddFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
                 AddFormat( SOT_FORMATSTR_ID_FILECONTENT );
                 AddFormat( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR );
                 AddFormat( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR );
                eBufferType = (TransferBufferType)( TRNSFR_INETFLD | eBufferType );
                nRet = sal_True;
            }
        }

        // at Cut, DDE-Link doesn't make sense!!
        SwDocShell* pDShell;
        if( !bIsCut && bDDELink &&
            0 != ( pDShell = pWrtShell->GetDoc()->GetDocShell()) &&
            SFX_CREATE_MODE_STANDARD == pDShell->GetCreateMode() )
        {
            AddFormat( SOT_FORMATSTR_ID_LINK );
            refDdeLink = new SwTrnsfrDdeLink( *this, *pWrtShell );
        }

        //ObjectDescriptor was already filly from the old DocShell.
        //Now adjust it. Thus in GetData the first query can still
        //be answered with delayed rendering.
        aObjDesc.mbCanLink = sal_False;
        Size aSz( OLESIZE );
        aObjDesc.maSize = OutputDevice::LogicToLogic( aSz, MAP_TWIP, MAP_100TH_MM );

        PrepareOLE( aObjDesc );
        AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );

        delete pWait;
    }
    else
        nRet = 0;

    if( pWrtShell->IsFrmSelected() )
    {
        SfxItemSet aSet( pWrtShell->GetAttrPool(), RES_URL, RES_URL );
        pWrtShell->GetFlyFrmAttr( aSet );
        const SwFmtURL& rURL = (SwFmtURL&)aSet.Get( RES_URL );
        if( rURL.GetMap() )
        {
            pImageMap = new ImageMap( *rURL.GetMap() );
            AddFormat( SOT_FORMATSTR_ID_SVIM );
        }
        else if( rURL.GetURL().Len() )
        {
            pTargetURL = new INetImage( sGrfNm, rURL.GetURL(),
                                        rURL.GetTargetFrameName(),
                                        aEmptyStr, Size() );
            AddFormat( SOT_FORMATSTR_ID_INET_IMAGE );
        }
    }

    return nRet;
}

int SwTransferable::Copy( sal_Bool bIsCut )
{
    int nRet = PrepareForCopy( bIsCut );
    if ( nRet )
    {
        CopyToClipboard( &pWrtShell->GetView().GetEditWin() );
    }
    return nRet;
}

int SwTransferable::CalculateAndCopy()
{
    if(!pWrtShell)
        return 0;
    SwWait aWait( *pWrtShell->GetView().GetDocShell(), sal_True );

    String aStr( pWrtShell->Calculate() );

    pClpDocFac = new SwDocFac;
    SwDoc *const pDoc = lcl_GetDoc(*pClpDocFac);
    pWrtShell->Copy(pDoc, & aStr);
    eBufferType = TRNSFR_DOCUMENT;
    AddFormat( FORMAT_STRING );

    CopyToClipboard( &pWrtShell->GetView().GetEditWin() );

    return 1;
}

int SwTransferable::CopyGlossary( SwTextBlocks& rGlossary,
                                    const String& rStr )
{
    if(!pWrtShell)
        return 0;
    SwWait aWait( *pWrtShell->GetView().GetDocShell(), sal_True );

    pClpDocFac = new SwDocFac;
    SwDoc *const pCDoc = lcl_GetDoc(*pClpDocFac);

    SwNodes& rNds = pCDoc->GetNodes();
    SwNodeIndex aNodeIdx( *rNds.GetEndOfContent().StartOfSectionNode() );
    SwCntntNode* pCNd = rNds.GoNext( &aNodeIdx ); // go to 1st ContentNode
    SwPaM aPam( *pCNd );

    pCDoc->LockExpFlds();   // never update fields - leave text as it is

    pCDoc->InsertGlossary( rGlossary, rStr, aPam, 0 );

    // a new one was created in CORE (OLE-Objects copied!)
    aDocShellRef = pCDoc->GetTmpDocShell();
    if( aDocShellRef.Is() )
        SwTransferable::InitOle( aDocShellRef, *pCDoc );
    pCDoc->SetTmpDocShell( (SfxObjectShell*)NULL );

    eBufferType = TRNSFR_DOCUMENT;

    //When someone needs it, we 'OLE' her something.
    AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
    AddFormat( FORMAT_RTF );
    AddFormat( SOT_FORMATSTR_ID_HTML );
    AddFormat( FORMAT_STRING );

    //ObjectDescriptor was already filled from the old DocShell.
    //Now adjust it. Thus in GetData the first query can still
    //be answered with delayed rendering.
    aObjDesc.mbCanLink = sal_False;
    Size aSz( OLESIZE );
    aObjDesc.maSize = OutputDevice::LogicToLogic( aSz, MAP_TWIP, MAP_100TH_MM );

    PrepareOLE( aObjDesc );
    AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );

    CopyToClipboard( &pWrtShell->GetView().GetEditWin() );

    return 1;
}

static inline uno::Reference < XTransferable > * lcl_getTransferPointer ( uno::Reference < XTransferable > &xRef )
{
    return &xRef;
}

sal_Bool SwTransferable::IsPaste( const SwWrtShell& rSh,
                              const TransferableDataHelper& rData )
{
    // Check the common case first: We can always paste our own data!
    // If _only_ the internal format can be pasted, this check will
    // yield 'true', while the one below would give a (wrong) result 'false'.

    bool bIsPaste = ( GetSwTransferable( rData ) != NULL );

    // if it's not our own data, we need to have a closer look:
    if( ! bIsPaste )
    {
        // determine the proper paste action, and return true if we find one
        uno::Reference<XTransferable> xTransferable( rData.GetXTransferable() );

        sal_uInt16 nDestination = SwTransferable::GetSotDestination( rSh );
        sal_uInt16 nSourceOptions =
                    (( EXCHG_DEST_DOC_TEXTFRAME == nDestination ||
                       EXCHG_DEST_SWDOC_FREE_AREA == nDestination ||
                       EXCHG_DEST_DOC_TEXTFRAME_WEB == nDestination ||
                       EXCHG_DEST_SWDOC_FREE_AREA_WEB == nDestination )
                                    ? EXCHG_IN_ACTION_COPY
                     : EXCHG_IN_ACTION_MOVE);

        sal_uLong nFormat;          // output param for GetExchangeAction
        sal_uInt16 nEventAction;    // output param for GetExchangeAction
        sal_uInt16 nAction = SotExchange::GetExchangeAction(
                                rData.GetDataFlavorExVector(),
                                nDestination,
                                nSourceOptions,             /* ?? */
                                EXCHG_IN_ACTION_DEFAULT,    /* ?? */
                                nFormat, nEventAction, 0,
                                lcl_getTransferPointer ( xTransferable ) );

        // if we find a suitable action, we can paste!
        bIsPaste = (EXCHG_INOUT_ACTION_NONE != nAction);
    }

    return bIsPaste;
}

int SwTransferable::Paste( SwWrtShell& rSh, TransferableDataHelper& rData )
{
    sal_uInt16 nEventAction, nAction=0,
           nDestination = SwTransferable::GetSotDestination( rSh );
    sal_uLong nFormat = 0;

    if( GetSwTransferable( rData ) )
    {
        nAction = EXCHG_OUT_ACTION_INSERT_PRIVATE;
    }
    else
    {
        sal_uInt16 nSourceOptions =
                    (( EXCHG_DEST_DOC_TEXTFRAME == nDestination ||
                    EXCHG_DEST_SWDOC_FREE_AREA == nDestination ||
                    EXCHG_DEST_DOC_TEXTFRAME_WEB == nDestination ||
                    EXCHG_DEST_SWDOC_FREE_AREA_WEB == nDestination )
                                    ? EXCHG_IN_ACTION_COPY
                                    : EXCHG_IN_ACTION_MOVE);
        uno::Reference<XTransferable> xTransferable( rData.GetXTransferable() );
        nAction = SotExchange::GetExchangeAction(
                                    rData.GetDataFlavorExVector(),
                                    nDestination,
                                    nSourceOptions,             /* ?? */
                                    EXCHG_IN_ACTION_DEFAULT,    /* ?? */
                                    nFormat, nEventAction, 0,
                                    lcl_getTransferPointer ( xTransferable ) );
    }

    // special case for tables from draw application
    if( EXCHG_OUT_ACTION_INSERT_DRAWOBJ == (nAction & EXCHG_ACTION_MASK) )
    {
        if( rData.HasFormat( SOT_FORMAT_RTF ) )
        {
            nAction = EXCHG_OUT_ACTION_INSERT_STRING | (nAction & !EXCHG_ACTION_MASK);
            nFormat = SOT_FORMAT_RTF;
        }
    }

    return EXCHG_INOUT_ACTION_NONE != nAction &&
            SwTransferable::PasteData( rData, rSh, nAction, nFormat,
                                        nDestination, sal_False, sal_False );
}

int SwTransferable::PasteData( TransferableDataHelper& rData,
                            SwWrtShell& rSh, sal_uInt16 nAction, sal_uLong nFormat,
                            sal_uInt16 nDestination, sal_Bool bIsPasteFmt,
                            sal_Bool bIsDefault,
                            const Point* pPt, sal_Int8 nDropAction,
                            sal_Bool bPasteSelection )
{
    SwWait aWait( *rSh.GetView().
        GetDocShell(), sal_False );
    SwTrnsfrActionAndUndo* pAction = 0;
    SwModule* pMod = SW_MOD();

    int nRet = 0;
    bool bCallAutoCaption = false;

    if( pPt )
    {
        // external Drop
        if( bPasteSelection ? !pMod->pXSelection : !pMod->pDragDrop )
        {
            switch( nDestination )
            {
            case EXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP:
            case EXCHG_DEST_DOC_LNKD_GRAPHOBJ:
            case EXCHG_DEST_DOC_GRAPH_W_IMAP:
            case EXCHG_DEST_DOC_GRAPHOBJ:
            case EXCHG_DEST_DOC_OLEOBJ:
            case EXCHG_DEST_DOC_DRAWOBJ:
            case EXCHG_DEST_DOC_URLBUTTON:
            case EXCHG_DEST_DOC_GROUPOBJ:
                // select frames/objects
                SwTransferable::SetSelInShell( rSh, sal_True, pPt );
                break;

            default:
                SwTransferable::SetSelInShell( rSh, sal_False, pPt );
                break;
            }
        }
    }
    else if( ( !GetSwTransferable( rData ) || bIsPasteFmt ) &&
            !rSh.IsTableMode() && rSh.HasSelection() )
    {
        // then delete the selections

        //don't delete selected content
        // - at table-selection
        // - at ReRead of a graphic/DDEData
        // - at D&D, for the right selection was taken care of
        //      in Drop-Handler
        sal_Bool bDelSel = sal_False;
        switch( nDestination )
        {
        case EXCHG_DEST_DOC_TEXTFRAME:
        case EXCHG_DEST_SWDOC_FREE_AREA:
        case EXCHG_DEST_DOC_TEXTFRAME_WEB:
        case EXCHG_DEST_SWDOC_FREE_AREA_WEB:
            bDelSel = sal_True;
            break;
        }

        if( bDelSel )
            // #i34830#
            pAction = new SwTrnsfrActionAndUndo( &rSh, UNDO_PASTE_CLIPBOARD, NULL,
                                                 sal_True );
    }

    SwTransferable *pTrans=0, *pTunneledTrans=GetSwTransferable( rData );

    if( pPt && ( bPasteSelection ? 0 != ( pTrans = pMod->pXSelection )
                                 : 0 != ( pTrans = pMod->pDragDrop) ))
    {
        // then internal Drag & Drop or XSelection
        nRet = pTrans->PrivateDrop( rSh, *pPt, DND_ACTION_MOVE == nDropAction,
                                    bPasteSelection );
    }
    else if( !pPt && pTunneledTrans &&
            EXCHG_OUT_ACTION_INSERT_PRIVATE == nAction )
    {
        // then internal paste
        nRet = pTunneledTrans->PrivatePaste( rSh );
    }
    else if( EXCHG_INOUT_ACTION_NONE != nAction )
    {
        if( !pAction )
        {
            pAction = new SwTrnsfrActionAndUndo( &rSh, UNDO_PASTE_CLIPBOARD);
        }

        // in Drag&Drop MessageBoxes must not be showed
        sal_Bool bMsg = 0 == pPt;
        sal_uInt8 nActionFlags = static_cast< sal_uInt8 >(( nAction >> 8 ) & 0xFF);

        sal_uInt16 nClearedAction = ( nAction & EXCHG_ACTION_MASK );
        // delete selections

        switch( nClearedAction )
        {
        case EXCHG_OUT_ACTION_INSERT_PRIVATE:
            OSL_ENSURE( pPt, "EXCHG_OUT_ACTION_INSERT_PRIVATE: what should happen here?" );
            break;

        case EXCHG_OUT_ACTION_MOVE_PRIVATE:
            OSL_ENSURE( pPt, "EXCHG_OUT_ACTION_MOVE_PRIVATE: what should happen here?" );
            break;


        case EXCHG_IN_ACTION_MOVE:
        case EXCHG_IN_ACTION_COPY:
        case EXCHG_IN_ACTION_LINK:
        case EXCHG_OUT_ACTION_INSERT_HTML:
        case EXCHG_OUT_ACTION_INSERT_STRING:
        case EXCHG_OUT_ACTION_INSERT_IMAGEMAP:
        case EXCHG_OUT_ACTION_REPLACE_IMAGEMAP:

            // then we have to use the format
            switch( nFormat )
            {
            case SOT_FORMATSTR_ID_DRAWING:
                nRet = SwTransferable::_PasteSdrFormat( rData, rSh,
                                                SW_PASTESDR_INSERT, pPt,
                                                nActionFlags );
                break;

            case SOT_FORMATSTR_ID_HTML:
            case SOT_FORMATSTR_ID_HTML_SIMPLE:
            case SOT_FORMATSTR_ID_HTML_NO_COMMENT:
            case SOT_FORMAT_RTF:
            case SOT_FORMAT_STRING:
                nRet = SwTransferable::_PasteFileContent( rData, rSh,
                                                            nFormat, bMsg );
                break;

            case SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK:
                {
                    INetBookmark aBkmk;
                    if( rData.GetINetBookmark( nFormat, aBkmk ) )
                    {
                        SwFmtINetFmt aFmt( aBkmk.GetURL(), aEmptyStr );
                        rSh.InsertURL( aFmt, aBkmk.GetDescription() );
                        nRet = 1;
                    }
                }
                break;

            case SOT_FORMATSTR_ID_SD_OLE:
                nRet = SwTransferable::_PasteOLE( rData, rSh, nFormat,
                                                    nActionFlags, bMsg );
                break;

            case SOT_FORMATSTR_ID_SVIM:
                nRet = SwTransferable::_PasteImageMap( rData, rSh );
                break;

            case SOT_FORMATSTR_ID_SVXB:
            case SOT_FORMAT_BITMAP:
            case SOT_FORMAT_GDIMETAFILE:
                nRet = SwTransferable::_PasteGrf( rData, rSh, nFormat,
                                                SW_PASTESDR_INSERT,pPt,
                                                nActionFlags, bMsg );
                break;

            case SOT_FORMATSTR_ID_XFORMS:
            case SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE:
            case SOT_FORMATSTR_ID_SBA_DATAEXCHANGE:
            case SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE:
                nRet = SwTransferable::_PasteDBData( rData, rSh, nFormat,
                                            EXCHG_IN_ACTION_LINK == nClearedAction,
                                            pPt, bMsg );
                break;

            case SOT_FORMAT_FILE:
                nRet = SwTransferable::_PasteFileName( rData, rSh, nFormat,
                                ( EXCHG_IN_ACTION_MOVE == nClearedAction
                                    ? SW_PASTESDR_REPLACE
                                    : EXCHG_IN_ACTION_LINK == nClearedAction
                                        ? SW_PASTESDR_SETATTR
                                        : SW_PASTESDR_INSERT),
                                pPt, nActionFlags, bMsg );
                break;

            case SOT_FORMAT_FILE_LIST:
                // then insert as graphics only
                nRet = SwTransferable::_PasteFileList( rData, rSh,
                                    EXCHG_IN_ACTION_LINK == nClearedAction,
                                    pPt, bMsg );
                break;

            case SOT_FORMATSTR_ID_SONLK:
                if( pPt )
                {
                    NaviContentBookmark aBkmk;
                    if( aBkmk.Paste( rData ) )
                    {
                        if(bIsDefault)
                        {
                            switch(aBkmk.GetDefaultDragType())
                            {
                                case REGION_MODE_NONE: nClearedAction = EXCHG_IN_ACTION_COPY; break;
                                case REGION_MODE_EMBEDDED: nClearedAction = EXCHG_IN_ACTION_MOVE; break;
                                case REGION_MODE_LINK: nClearedAction = EXCHG_IN_ACTION_LINK; break;
                            }
                        }
                        rSh.NavigatorPaste( aBkmk, nClearedAction );
                        nRet = 1;
                    }
                }
                break;

            case SOT_FORMATSTR_ID_INET_IMAGE:
            case SOT_FORMATSTR_ID_NETSCAPE_IMAGE:
                nRet = SwTransferable::_PasteTargetURL( rData, rSh,
                                                        SW_PASTESDR_INSERT,
                                                        pPt, sal_True );
                break;

            default:
                OSL_ENSURE( pPt, "unknown format" );
            }
            break;

        case EXCHG_OUT_ACTION_INSERT_FILE:
            nRet = SwTransferable::_PasteFileName( rData, rSh, nFormat,
                                        SW_PASTESDR_INSERT, pPt,
                                        nActionFlags, bMsg );
            if( nRet & SWTRANSFER_GRAPHIC_INSERTED )
                bCallAutoCaption = true;
            break;

        case EXCHG_OUT_ACTION_INSERT_OLE:
            nRet = SwTransferable::_PasteOLE( rData, rSh, nFormat,
                                                nActionFlags,bMsg );
            break;

        case EXCHG_OUT_ACTION_INSERT_DDE:
            {
                sal_Bool bReRead = 0 != CNT_HasGrf( rSh.GetCntType() );
                nRet = SwTransferable::_PasteDDE( rData, rSh, bReRead, bMsg );
            }
            break;

        case EXCHG_OUT_ACTION_INSERT_HYPERLINK:
            {
                String sURL, sDesc;
                if( SOT_FORMAT_FILE == nFormat )
                {
                    if( rData.GetString( nFormat, sURL ) && sURL.Len() )
                    {
                        SwTransferable::_CheckForURLOrLNKFile( rData, sURL, &sDesc );
                        if( !sDesc.Len() )
                            sDesc = sURL;
                        nRet = 1;
                    }
                }
                else
                {
                    INetBookmark aBkmk;
                    if( rData.GetINetBookmark( nFormat, aBkmk ) )
                    {
                        sURL = aBkmk.GetURL();
                        sDesc = aBkmk.GetDescription();
                        nRet = 1;
                    }
                }

                if( nRet )
                {
                    SwFmtINetFmt aFmt( sURL, aEmptyStr );
                    rSh.InsertURL( aFmt, sDesc );
                }
            }
            break;

        case EXCHG_OUT_ACTION_GET_ATTRIBUTES:
            switch( nFormat )
            {
            case SOT_FORMATSTR_ID_DRAWING:
                nRet = SwTransferable::_PasteSdrFormat( rData, rSh,
                                                SW_PASTESDR_SETATTR, pPt,
                                                nActionFlags );
                break;
            case SOT_FORMATSTR_ID_SVXB:
            case SOT_FORMAT_GDIMETAFILE:
            case SOT_FORMAT_BITMAP:
            case SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK:
            case SOT_FORMAT_FILE:
            case SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR:
            case SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR:
                nRet = SwTransferable::_PasteGrf( rData, rSh, nFormat,
                                                SW_PASTESDR_SETATTR, pPt,
                                                nActionFlags, bMsg );
                break;
            default:
                OSL_FAIL( "unknown format" );
            }

            break;

        case EXCHG_OUT_ACTION_INSERT_DRAWOBJ:
            nRet = SwTransferable::_PasteSdrFormat( rData, rSh,
                                                SW_PASTESDR_INSERT, pPt,
                                                nActionFlags );
            break;
        case EXCHG_OUT_ACTION_INSERT_SVXB:
        case EXCHG_OUT_ACTION_INSERT_GDIMETAFILE:
        case EXCHG_OUT_ACTION_INSERT_BITMAP:
        case EXCHG_OUT_ACTION_INSERT_GRAPH:
            nRet = SwTransferable::_PasteGrf( rData, rSh, nFormat,
                                                SW_PASTESDR_INSERT, pPt,
                                                nActionFlags, bMsg );
            break;

        case EXCHG_OUT_ACTION_REPLACE_DRAWOBJ:
            nRet = SwTransferable::_PasteSdrFormat( rData, rSh,
                                                SW_PASTESDR_REPLACE, pPt,
                                                nActionFlags );
            break;

        case EXCHG_OUT_ACTION_REPLACE_SVXB:
        case EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE:
        case EXCHG_OUT_ACTION_REPLACE_BITMAP:
        case EXCHG_OUT_ACTION_REPLACE_GRAPH:
            nRet = SwTransferable::_PasteGrf( rData, rSh, nFormat,
                                                SW_PASTESDR_REPLACE,pPt,
                                                nActionFlags, bMsg );
            break;

        case EXCHG_OUT_ACTION_INSERT_INTERACTIVE:
            nRet = SwTransferable::_PasteAsHyperlink( rData, rSh, nFormat );
            break;

        default:
            OSL_FAIL("unknown action" );
        }
    }

    if( !bPasteSelection && rSh.IsFrmSelected() )
    {
        rSh.EnterSelFrmMode();
        //force ::SelectShell
        rSh.GetView().StopShellTimer();
    }

    delete pAction;
    if( bCallAutoCaption )
        rSh.GetView().AutoCaption( GRAPHIC_CAP );

    return nRet;
}

sal_uInt16 SwTransferable::GetSotDestination( const SwWrtShell& rSh,
                                            const Point* pPt )
{
    sal_uInt16 nRet = EXCHG_INOUT_ACTION_NONE;

    ObjCntType eOType;
    if( pPt )
    {
        SdrObject *pObj = 0;
        eOType = rSh.GetObjCntType( *pPt, pObj );
    }
    else
        eOType = rSh.GetObjCntTypeOfSelection();

    switch( eOType )
    {
    case OBJCNT_GRF:
        {
            sal_Bool bIMap, bLink;
            if( pPt )
            {
                bIMap = 0 != rSh.GetFmtFromObj( *pPt )->GetURL().GetMap();
                String aDummy;
                rSh.GetGrfAtPos( *pPt, aDummy, bLink );
            }
            else
            {
                bIMap = 0 != rSh.GetFlyFrmFmt()->GetURL().GetMap();
                String aDummy;
                rSh.GetGrfNms( &aDummy, 0 );
                bLink = 0 != aDummy.Len();
            }

            if( bLink && bIMap )
                nRet = EXCHG_DEST_DOC_LNKD_GRAPH_W_IMAP;
            else if( bLink )
                nRet = EXCHG_DEST_DOC_LNKD_GRAPHOBJ;
            else if( bIMap )
                nRet = EXCHG_DEST_DOC_GRAPH_W_IMAP;
            else
                nRet = EXCHG_DEST_DOC_GRAPHOBJ;
        }
        break;

    case OBJCNT_FLY:
        if( rSh.GetView().GetDocShell()->ISA(SwWebDocShell) )
            nRet = EXCHG_DEST_DOC_TEXTFRAME_WEB;
        else
            nRet = EXCHG_DEST_DOC_TEXTFRAME;
        break;
    case OBJCNT_OLE:        nRet = EXCHG_DEST_DOC_OLEOBJ;       break;

    case OBJCNT_CONTROL:    /* no Action avail */
    case OBJCNT_SIMPLE:     nRet = EXCHG_DEST_DOC_DRAWOBJ;      break;
    case OBJCNT_URLBUTTON:  nRet = EXCHG_DEST_DOC_URLBUTTON;    break;
    case OBJCNT_GROUPOBJ:   nRet = EXCHG_DEST_DOC_GROUPOBJ;     break;

// what do we do at multiple selections???
    default:
        {
            if( rSh.GetView().GetDocShell()->ISA(SwWebDocShell) )
                nRet = EXCHG_DEST_SWDOC_FREE_AREA_WEB;
            else
                nRet = EXCHG_DEST_SWDOC_FREE_AREA;
        }
    }

    return nRet;
}

int SwTransferable::_PasteFileContent( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, sal_uLong nFmt, sal_Bool bMsg )
{
    sal_uInt16 nResId = MSG_CLPBRD_FORMAT_ERROR;
    int nRet = 0;

    MSE40HTMLClipFormatObj aMSE40ClpObj;

    SotStorageStreamRef xStrm;
    SvStream* pStream = 0;
    SwRead pRead = 0;
    rtl::OUString sData;
    switch( nFmt )
    {
    case SOT_FORMAT_STRING:
        {
            pRead = ReadAscii;
            if( rData.GetString( nFmt, sData ) )
            {
                pStream = new SvMemoryStream( (void*)sData.getStr(),
                            sData.getLength() * sizeof( sal_Unicode ),
                            STREAM_READ );
#ifdef OSL_BIGENDIAN
                pStream->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
#else
                pStream->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
#endif

                SwAsciiOptions aAOpt;
                aAOpt.SetCharSet( RTL_TEXTENCODING_UCS2 );
                pRead->GetReaderOpt().SetASCIIOpts( aAOpt );
                break;
            }
        }
        // no break - because then test if we get a stream

    default:
        if( rData.GetSotStorageStream( nFmt, xStrm ) )
        {
            if( ( SOT_FORMATSTR_ID_HTML_SIMPLE == nFmt ) ||
                ( SOT_FORMATSTR_ID_HTML_NO_COMMENT == nFmt ) )
            {
                pStream = aMSE40ClpObj.IsValid( *xStrm );
                pRead = ReadHTML;
                pRead->SetReadUTF8( sal_True );

                sal_Bool bNoComments =
                    ( nFmt == SOT_FORMATSTR_ID_HTML_NO_COMMENT );
                pRead->SetIgnoreHTMLComments( bNoComments );
            }
            else
            {
                pStream = &xStrm;
                if( SOT_FORMAT_RTF == nFmt )
                    pRead = SwReaderWriter::GetReader( READER_WRITER_RTF );
                else if( !pRead )
                {
                    pRead = ReadHTML;
                    pRead->SetReadUTF8( sal_True );
                }
            }
        }
        break;
    }

    if( pStream && pRead )
    {
        Link aOldLink( rSh.GetChgLnk() );
        rSh.SetChgLnk( Link() );

        const SwPosition& rInsPos = *rSh.GetCrsr()->Start();
        SwReader aReader( *pStream, aEmptyStr, String(), *rSh.GetCrsr() );
        rSh.SaveTblBoxCntnt( &rInsPos );
        if( IsError( aReader.Read( *pRead )) )
            nResId = ERR_CLPBRD_READ;
        else
            nResId = 0, nRet = 1;

        rSh.SetChgLnk( aOldLink );
        if( nRet )
            rSh.CallChgLnk();
    }
    else
        nResId = MSG_CLPBRD_FORMAT_ERROR;

    // Exist a SvMemoryStream? (data in the OUString and xStrm is empty)
    if( pStream && !xStrm.Is() )
        delete pStream;

    if( bMsg && nResId )
    {
        InfoBox( 0, SW_RES( nResId )).Execute();
    }
    return nRet;
}

int SwTransferable::_PasteOLE( TransferableDataHelper& rData, SwWrtShell& rSh,
                                sal_uLong nFmt, sal_uInt8 nActionFlags, sal_Bool bMsg )
{
    int nRet = 0;
    TransferableObjectDescriptor aObjDesc;
    uno::Reference < io::XInputStream > xStrm;
    uno::Reference < embed::XStorage > xStore;
    Reader* pRead = 0;

    // Get the preferred format
    SotFormatStringId nId;
    if( rData.HasFormat( SOT_FORMATSTR_ID_EMBEDDED_OBJ ) )
        nId = SOT_FORMATSTR_ID_EMBEDDED_OBJ;
    else if( rData.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE ) &&
             rData.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ))
        nId = SOT_FORMATSTR_ID_EMBED_SOURCE;
    else
        nId = 0;

    if( nId && rData.GetInputStream( nId, xStrm ) && xStrm.is() )
    {
        // if there is an embedded object, first try if it's a writer object
        // this will be inserted into the document by using a Reader
        try
        {
            xStore = comphelper::OStorageHelper::GetStorageFromInputStream( xStrm );
            switch( SotStorage::GetFormatID( xStore ) )
            {
                case SOT_FORMATSTR_ID_STARWRITER_60:
                case SOT_FORMATSTR_ID_STARWRITERWEB_60:
                case SOT_FORMATSTR_ID_STARWRITERGLOB_60:
                case SOT_FORMATSTR_ID_STARWRITER_8:
                case SOT_FORMATSTR_ID_STARWRITERWEB_8:
                case SOT_FORMATSTR_ID_STARWRITERGLOB_8:
                    pRead = ReadXML;
                    break;
                default:
                    try
                    {
                        uno::Reference < lang::XComponent > xComp( xStore, uno::UNO_QUERY );
                        xComp->dispose();
                        xStore = 0;
                    }
                    catch (const uno::Exception&)
                    {
                    }

                    break;
            }
        }
        catch (const uno::Exception&)
        {
            // it wasn't a storage, but maybe it's a useful stream
        }

        nFmt = nId;
    }

    if( pRead )
    {
        SwPaM &rPAM = *rSh.GetCrsr();
        SwReader aReader( xStore, aEmptyStr, rPAM );
        if( !IsError( aReader.Read( *pRead )) )
            nRet = 1;
        else if( bMsg )
            InfoBox( 0, SW_RES(ERR_CLPBRD_READ) ).Execute();
    }
    else
    {
           // temporary storage until the object is inserted
        uno::Reference< embed::XStorage > xTmpStor;
        uno::Reference < embed::XEmbeddedObject > xObj;
        ::rtl::OUString aName;
           comphelper::EmbeddedObjectContainer aCnt;

        if ( xStrm.is() )
        {
            if ( !rData.GetTransferableObjectDescriptor( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aObjDesc ) )
            {
                OSL_ENSURE( !xStrm.is(), "An object without descriptor in clipboard!");
            }
        }
        else
        {
            if( rData.HasFormat( nFmt = SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE ) && rData.GetTransferableObjectDescriptor( nFmt, aObjDesc ) )
             {
                if ( !rData.GetInputStream( SOT_FORMATSTR_ID_EMBED_SOURCE_OLE, xStrm ) )
                    rData.GetInputStream( SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE, xStrm );

                if ( !xStrm.is() )
                {
                    // This is MSOLE object that should be created by direct using of system clipboard
                    try
                    {
                        xTmpStor = ::comphelper::OStorageHelper::GetTemporaryStorage();
                        uno::Reference < embed::XEmbedObjectClipboardCreator > xClipboardCreator(
                            ::comphelper::getProcessServiceFactory()->createInstance( ::rtl::OUString(
                                            RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.MSOLEObjectSystemCreator")) ),
                            uno::UNO_QUERY_THROW );

                        embed::InsertedObjectInfo aInfo = xClipboardCreator->createInstanceInitFromClipboard(
                                                            xTmpStor,
                                                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "DummyName" ) ),
                                                            uno::Sequence< beans::PropertyValue >() );

                        // TODO/LATER: in future InsertedObjectInfo will be used to get container related information
                        // for example whether the object should be an iconified one
                        xObj = aInfo.Object;
                    }
                    catch (const uno::Exception&)
                    {
                    }
                }
            }
        }

        if ( xStrm.is() && !xObj.is() )
            xObj = aCnt.InsertEmbeddedObject( xStrm, aName );

        if( xObj.is() )
        {
            svt::EmbeddedObjectRef xObjRef( xObj, aObjDesc.mnViewAspect );

            // try to get the replacement image from the clipboard
            Graphic aGraphic;
            sal_uLong nGrFormat = 0;

            // insert replacement image ( if there is one ) into the object helper
            if ( nGrFormat )
            {
                datatransfer::DataFlavor aDataFlavor;
                SotExchange::GetFormatDataFlavor( nGrFormat, aDataFlavor );
                   xObjRef.SetGraphic( aGraphic, aDataFlavor.MimeType );
            }
            else if ( aObjDesc.mnViewAspect == embed::Aspects::MSOLE_ICON )
            {
                // it is important to have an icon, let an empty graphic be used
                // if no other graphic is provided
                // TODO/LATER: in future a default bitmap could be used
                ::rtl::OUString aMimeType;
                MapMode aMapMode( MAP_100TH_MM );
                aGraphic.SetPrefSize( Size( 2500, 2500 ) );
                aGraphic.SetPrefMapMode( aMapMode );
                   xObjRef.SetGraphic( aGraphic, aMimeType );
            }

            //set size. This is a hack because of handing over, size should be
            //passed to the InsertOle!!!!!!!!!!
            Size aSize;
            if ( aObjDesc.mnViewAspect == embed::Aspects::MSOLE_ICON )
            {
                   if( aObjDesc.maSize.Width() && aObjDesc.maSize.Height() )
                    aSize = aObjDesc.maSize;
                else
                {
                    MapMode aMapMode( MAP_100TH_MM );
                    aSize = xObjRef.GetSize( &aMapMode );
                }
            }
            else if( aObjDesc.maSize.Width() && aObjDesc.maSize.Height() )
            {
                aSize = Size( aObjDesc.maSize );    //always 100TH_MM
                MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xObj->getMapUnit( aObjDesc.mnViewAspect ) );
                aSize = OutputDevice::LogicToLogic( aSize, MAP_100TH_MM, aUnit );
                awt::Size aSz;
                try
                {
                    aSz = xObj->getVisualAreaSize( aObjDesc.mnViewAspect );
                }
                catch (const embed::NoVisualAreaSizeException&)
                {
                    // in this case the provided size is used
                }

                if ( aSz.Width != aSize.Width() || aSz.Height != aSize.Height() )
                {
                    aSz.Width = aSize.Width();
                    aSz.Height = aSize.Height();
                    xObj->setVisualAreaSize( aObjDesc.mnViewAspect, aSz );
                }
            }
            else
            {
                // the descriptor contains the wrong object size
                // the following call will let the MSOLE objects cache the size if it is possible
                // it should be done while the object is running
                try
                {
                    xObj->getVisualAreaSize( aObjDesc.mnViewAspect );
                }
                catch (const uno::Exception&)
                {
                }
            }
            //End of Hack!

            rSh.InsertOleObject( xObjRef );
            nRet = 1;

            if( nRet && ( nActionFlags &
                ( EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL >> 8) ))
                SwTransferable::_PasteTargetURL( rData, rSh, 0, 0, sal_False );

            // let the object be unloaded if possible
            SwOLEObj::UnloadObject( xObj, rSh.GetDoc(), embed::Aspects::MSOLE_CONTENT );
        }
    }
    return nRet;
}

int SwTransferable::_PasteTargetURL( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, sal_uInt16 nAction,
                                    const Point* pPt, sal_Bool bInsertGRF )
{
    int nRet = 0;
    INetImage aINetImg;
    if( ( rData.HasFormat( SOT_FORMATSTR_ID_INET_IMAGE ) &&
          rData.GetINetImage( SOT_FORMATSTR_ID_INET_IMAGE, aINetImg )) ||
        ( rData.HasFormat( SOT_FORMATSTR_ID_NETSCAPE_IMAGE ) &&
          rData.GetINetImage( SOT_FORMATSTR_ID_NETSCAPE_IMAGE, aINetImg )) )
    {
        if( aINetImg.GetImageURL().Len() && bInsertGRF )
        {
            String sURL( aINetImg.GetImageURL() );
            SwTransferable::_CheckForURLOrLNKFile( rData, sURL );

            //!!! check at FileSystem - only then it make sense to test graphics !!!
            Graphic aGrf;
            GraphicFilter &rFlt = GraphicFilter::GetGraphicFilter();
            nRet = GRFILTER_OK == GraphicFilter::LoadGraphic( sURL, aEmptyStr, aGrf, &rFlt );
            if( nRet )
            {
                switch( nAction )
                {
                case SW_PASTESDR_INSERT:
                    SwTransferable::SetSelInShell( rSh, sal_False, pPt );
                    rSh.Insert( sURL, aEmptyStr, aGrf );
                    break;

                case SW_PASTESDR_REPLACE:
                    if( rSh.IsObjSelected() )
                    {
                        rSh.ReplaceSdrObj( sURL, aEmptyStr, &aGrf );
                        Point aPt( pPt ? *pPt : rSh.GetCrsrDocPos() );
                        SwTransferable::SetSelInShell( rSh, sal_True, &aPt );
                    }
                    else
                        rSh.ReRead( sURL, aEmptyStr, &aGrf );
                    break;

                case SW_PASTESDR_SETATTR:
                    if( rSh.IsObjSelected() )
                        rSh.Paste( aGrf );
                    else if( OBJCNT_GRF == rSh.GetObjCntTypeOfSelection() )
                        rSh.ReRead( sURL, aEmptyStr, &aGrf );
                    else
                    {
                        SwTransferable::SetSelInShell( rSh, sal_False, pPt );
                        rSh.Insert( sURL, aEmptyStr, aGrf );
                    }
                    break;
                default:
                    nRet = 0;
                }
            }
        }
        else
            nRet = 1;
    }

    if( nRet )
    {
        SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
        rSh.GetFlyFrmAttr( aSet );
        SwFmtURL aURL( (SwFmtURL&)aSet.Get( RES_URL ) );

        if( aURL.GetURL() != aINetImg.GetTargetURL() ||
            aURL.GetTargetFrameName() != aINetImg.GetTargetFrame() )
        {
            aURL.SetURL( aINetImg.GetTargetURL(), sal_False );
            aURL.SetTargetFrameName( aINetImg.GetTargetFrame() );
            aSet.Put( aURL );
            rSh.SetFlyFrmAttr( aSet );
        }
    }
    return nRet;
}

void SwTransferable::SetSelInShell( SwWrtShell& rSh, sal_Bool bSelectFrm,
                                        const Point* pPt )
{
    if( bSelectFrm )
    {
        // select frames/objects
        if( pPt && !rSh.GetView().GetViewFrame()->GetDispatcher()->IsLocked() )
        {
            rSh.GetView().NoRotate();
            if( rSh.SelectObj( *pPt ))
            {
                rSh.HideCrsr();
                rSh.EnterSelFrmMode( pPt );
                bFrmDrag = sal_True;
            }
        }
    }
    else
    {
        if( rSh.IsFrmSelected() || rSh.IsObjSelected() )
        {
            rSh.UnSelectFrm();
            rSh.LeaveSelFrmMode();
            rSh.GetView().GetEditWin().StopInsFrm();
            bFrmDrag = sal_False;
        }
        else if( rSh.GetView().GetDrawFuncPtr() )
            rSh.GetView().GetEditWin().StopInsFrm();

        rSh.EnterStdMode();
        if( pPt )
            rSh.SwCrsrShell::SetCrsr( *pPt, sal_True );
    }
}

int SwTransferable::_PasteDDE( TransferableDataHelper& rData,
                                SwWrtShell& rWrtShell, sal_Bool bReReadGrf,
                                sal_Bool bMsg )
{
    // data from Clipboardformat
    String aApp, aTopic, aItem;

    {
        SotStorageStreamRef xStrm;
        if( !rData.GetSotStorageStream( SOT_FORMATSTR_ID_LINK, xStrm ))
        {
            OSL_ENSURE( !&rWrtShell, "DDE Data not found." );
            return 0;
        }   // report useful error!!

        rtl_TextEncoding eEncoding = DDE_TXT_ENCODING;
        aApp = read_zeroTerminated_uInt8s_ToOUString(*xStrm, eEncoding);
        aTopic = read_zeroTerminated_uInt8s_ToOUString(*xStrm, eEncoding);
        aItem = read_zeroTerminated_uInt8s_ToOUString(*xStrm, eEncoding);
    }

    String aCmd;
    sfx2::MakeLnkName( aCmd, &aApp, aTopic, aItem );

    // do we want to read in a graphic now?
    sal_uLong nFormat;
    if( !rData.HasFormat( FORMAT_RTF ) &&
        !rData.HasFormat( SOT_FORMATSTR_ID_HTML ) &&
        !rData.HasFormat( FORMAT_STRING ) &&
        (rData.HasFormat( nFormat = FORMAT_GDIMETAFILE ) ||
         rData.HasFormat( nFormat = FORMAT_BITMAP )) )
    {
        Graphic aGrf;
        int nRet = rData.GetGraphic( nFormat, aGrf );
        if( nRet )
        {
            rtl::OUString sLnkTyp("DDE");
            if ( bReReadGrf )
                rWrtShell.ReRead( aCmd, sLnkTyp, &aGrf );
            else
                rWrtShell.Insert( aCmd, sLnkTyp, aGrf );
        }
        return nRet;
    }

    SwFieldType* pTyp = 0;
    sal_uInt16 i = 1,j;
    String aName;
    sal_Bool bAlreadyThere = sal_False, bDoublePaste = sal_False;
    sal_uInt16 nSize = rWrtShell.GetFldTypeCount();
    const ::utl::TransliterationWrapper& rColl = ::GetAppCmpStrIgnore();

    do {
        aName = aApp;
        aName += String::CreateFromInt32( i );
        for( j = INIT_FLDTYPES; j < nSize; j++ )
        {
            pTyp = rWrtShell.GetFldType( j );
            if( RES_DDEFLD == pTyp->Which() )
            {
                String sTmp( ((SwDDEFieldType*)pTyp)->GetCmd() );
                if( rColl.isEqual( sTmp, aCmd ) &&
                    sfx2::LINKUPDATE_ALWAYS == ((SwDDEFieldType*)pTyp)->GetType() )
                {
                    aName = pTyp->GetName();
                    bDoublePaste = sal_True;
                    break;
                }
                else if( rColl.isEqual( aName, pTyp->GetName() ) )
                    break;
            }
        }
        if( j == nSize )
            bAlreadyThere = sal_False;
        else
        {
            bAlreadyThere = sal_True;
            i++;
        }
    }
    while( bAlreadyThere && !bDoublePaste );

    if( !bDoublePaste )
    {
        SwDDEFieldType aType( aName, aCmd, sfx2::LINKUPDATE_ALWAYS );
        pTyp = rWrtShell.InsertFldType( aType );
    }


    SwDDEFieldType* pDDETyp = (SwDDEFieldType*)pTyp;

    String aExpand;
    if( rData.GetString( FORMAT_STRING, aExpand ))
    {
        do {            // middle checked loop

            // When data comes from a spreadsheet, we add a DDE-table
            if( ( rData.HasFormat( SOT_FORMATSTR_ID_SYLK ) ||
                  rData.HasFormat( SOT_FORMATSTR_ID_SYLK_BIGCAPS ) ) &&
                aExpand.Len() &&
                 ( 1 < comphelper::string::getTokenCount(aExpand, '\n') ||
                       comphelper::string::getTokenCount(aExpand, '\t') ) )
            {
                String sTmp( aExpand );
                xub_StrLen nRows = comphelper::string::getTokenCount(sTmp, '\n');
                if( nRows )
                    --nRows;
                sTmp = sTmp.GetToken( 0, '\n' );
                xub_StrLen nCols = comphelper::string::getTokenCount(sTmp, '\t');

                // at least one column & row must be there
                if( !nRows || !nCols )
                {
                    if( bMsg )
                        InfoBox(0, SW_RESSTR(STR_NO_TABLE)).Execute();
                    pDDETyp = 0;
                    break;
                }

                rWrtShell.InsertDDETable(
                    SwInsertTableOptions( tabopts::SPLIT_LAYOUT, 1 ), // TODO MULTIHEADER
                    pDDETyp, nRows, nCols );
            }
            else if( 1 < comphelper::string::getTokenCount(aExpand, '\n') )
            {
                // multiple paragraphs -> insert a protected section
                if( rWrtShell.HasSelection() )
                    rWrtShell.DelRight();

                SwSectionData aSect( DDE_LINK_SECTION, aName );
                aSect.SetLinkFileName( aCmd );
                aSect.SetProtectFlag(true);
                rWrtShell.InsertSection( aSect );

                pDDETyp = 0;                // remove FieldTypes again
            }
            else
            {
                // Einfuegen
                SwDDEField aSwDDEField( pDDETyp );
                rWrtShell.Insert( aSwDDEField );
            }

        } while( sal_False );
    }
    else
        pDDETyp = 0;                        // remove FieldTypes again

    if( !pDDETyp && !bDoublePaste )
    {
        // remove FieldType again - error occured!
        for( j = nSize; j >= INIT_FLDTYPES; --j )
            if( pTyp == rWrtShell.GetFldType( j ) )
            {
                rWrtShell.RemoveFldType( j );
                break;
            }
    }

    return 1;
}

int SwTransferable::_PasteSdrFormat(  TransferableDataHelper& rData,
                                    SwWrtShell& rSh, sal_uInt16 nAction,
                                    const Point* pPt, sal_uInt8 nActionFlags )
{
    int nRet = 0;
    SotStorageStreamRef xStrm;
    if( rData.GetSotStorageStream( SOT_FORMATSTR_ID_DRAWING, xStrm ))
    {
        xStrm->SetVersion( SOFFICE_FILEFORMAT_50 );
        rSh.Paste( *xStrm, nAction, pPt );
        nRet = 1;

        if( nRet && ( nActionFlags &
            ( EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL >> 8) ))
            SwTransferable::_PasteTargetURL( rData, rSh, 0, 0, sal_False );
    }
    return nRet;
}

int SwTransferable::_PasteGrf( TransferableDataHelper& rData, SwWrtShell& rSh,
                                sal_uLong nFmt, sal_uInt16 nAction, const Point* pPt,
                                sal_uInt8 nActionFlags, sal_Bool /*bMsg*/ )
{
    int nRet = 0;

    Graphic aGrf;
    INetBookmark aBkmk;
    sal_Bool bCheckForGrf = sal_False, bCheckForImageMap = sal_False;

    switch( nFmt )
    {
    case SOT_FORMAT_BITMAP:
    case SOT_FORMAT_GDIMETAFILE:
        nRet = rData.GetGraphic( nFmt, aGrf );
        break;

    case SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK:
    case SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR:
    case SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR:
        if( 0 != ( nRet = rData.GetINetBookmark( nFmt, aBkmk ) ))
        {
            if( SW_PASTESDR_SETATTR == nAction )
                nFmt = SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK;
            else
                bCheckForGrf = sal_True;
        }
        break;

    case SOT_FORMAT_FILE:
        {
            String sTxt;
            if( 0 != ( nRet = rData.GetString( nFmt, sTxt ) ) )
            {
                String sDesc;
                SwTransferable::_CheckForURLOrLNKFile( rData, sTxt, &sDesc );

                aBkmk = INetBookmark(
                        URIHelper::SmartRel2Abs(INetURLObject(), sTxt, Link(), false ),
                        sDesc );
                bCheckForGrf = sal_True;
                bCheckForImageMap = SW_PASTESDR_REPLACE == nAction;
            }
        }
        break;

    default:
        nRet = rData.GetGraphic( nFmt, aGrf );
        break;
    }

    if( bCheckForGrf )
    {
        //!!! check at FileSystem - only then it makes sense to test the graphics !!!
        GraphicFilter &rFlt = GraphicFilter::GetGraphicFilter();
        nRet = GRFILTER_OK == GraphicFilter::LoadGraphic( aBkmk.GetURL(), aEmptyStr,
                                            aGrf, &rFlt );
        if( !nRet && SW_PASTESDR_SETATTR == nAction &&
            SOT_FORMAT_FILE == nFmt &&
            // only at frame selection
            rSh.IsFrmSelected() )
        {
            // then set as hyperlink after the graphic
            nFmt = SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK;
            nRet = sal_True;
        }
    }

    if( nRet )
    {
        String sURL;
        if( rSh.GetView().GetDocShell()->ISA(SwWebDocShell) )
            sURL = aBkmk.GetURL();

        switch( nAction )
        {
        case SW_PASTESDR_INSERT:
            SwTransferable::SetSelInShell( rSh, sal_False, pPt );
            rSh.Insert( sURL, aEmptyStr, aGrf );
        break;

        case SW_PASTESDR_REPLACE:
            if( rSh.IsObjSelected() )
            {
                rSh.ReplaceSdrObj( sURL, aEmptyStr, &aGrf );
                Point aPt( pPt ? *pPt : rSh.GetCrsrDocPos() );
                SwTransferable::SetSelInShell( rSh, sal_True, &aPt );
            }
            else
                rSh.ReRead( sURL, aEmptyStr, &aGrf );
            break;

        case SW_PASTESDR_SETATTR:
            if( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK == nFmt )
            {
                if( rSh.IsFrmSelected() )
                {
                    SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                    rSh.GetFlyFrmAttr( aSet );
                    SwFmtURL aURL( (SwFmtURL&)aSet.Get( RES_URL ) );
                    aURL.SetURL( aBkmk.GetURL(), sal_False );
                    aSet.Put( aURL );
                    rSh.SetFlyFrmAttr( aSet );
                }
            }
            else if( rSh.IsObjSelected() )
                rSh.Paste( aGrf );
            else if( OBJCNT_GRF == rSh.GetObjCntTypeOfSelection() )
                rSh.ReRead( sURL, aEmptyStr, &aGrf );
            else
            {
                SwTransferable::SetSelInShell( rSh, sal_False, pPt );
                rSh.Insert( aBkmk.GetURL(), aEmptyStr, aGrf );
            }
            break;
        default:
            nRet = 0;
        }
    }

    if( nRet )
    {

        if( nActionFlags &
            (( EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP |
                EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP ) >> 8) )
            SwTransferable::_PasteImageMap( rData, rSh );

        if( nActionFlags &
            ( EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL >> 8) )
            SwTransferable::_PasteTargetURL( rData, rSh, 0, 0, sal_False );
    }
    else if( bCheckForImageMap )
    {
        // or should the file be an ImageMap-File?
        ImageMap aMap;
        SfxMedium aMed( INetURLObject(aBkmk.GetURL()).GetFull(),
                            STREAM_STD_READ );
        SvStream* pStream = aMed.GetInStream();
        if( pStream != NULL  &&
            !pStream->GetError()  &&
            // mba: no BaseURL for clipboard functionality
            aMap.Read( *pStream, IMAP_FORMAT_DETECT, String() ) == IMAP_ERR_OK &&
            aMap.GetIMapObjectCount() )
        {
            SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
            rSh.GetFlyFrmAttr( aSet );
            SwFmtURL aURL( (SwFmtURL&)aSet.Get( RES_URL ) );
            aURL.SetMap( &aMap );
            aSet.Put( aURL );
            rSh.SetFlyFrmAttr( aSet );
            nRet = 1;
        }
    }

    return nRet;
}

int SwTransferable::_PasteImageMap( TransferableDataHelper& rData,
                                    SwWrtShell& rSh )
{
    int nRet = 0;
    if( rData.HasFormat( SOT_FORMATSTR_ID_SVIM ))
    {
        SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
        rSh.GetFlyFrmAttr( aSet );
        SwFmtURL aURL( (SwFmtURL&)aSet.Get( RES_URL ) );
        const ImageMap* pOld = aURL.GetMap();

        // set or replace, that is the question
        ImageMap aImageMap;
        if( rData.GetImageMap( SOT_FORMATSTR_ID_SVIM, aImageMap ) &&
            ( !pOld || aImageMap != *pOld ))
        {
            aURL.SetMap( &aImageMap );
            aSet.Put( aURL );
            rSh.SetFlyFrmAttr( aSet );
        }
        nRet = 1;
    }
    return nRet;
}

int SwTransferable::_PasteAsHyperlink( TransferableDataHelper& rData,
                                        SwWrtShell& rSh, sal_uLong nFmt )
{
    int nRet = 0;
    String sFile;
    if( rData.GetString( nFmt, sFile ) && sFile.Len() )
    {
        String sDesc;
        SwTransferable::_CheckForURLOrLNKFile( rData, sFile, &sDesc );

        // first, make the URL absolute
        INetURLObject aURL;
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( sFile );
        sFile = aURL.GetMainURL( INetURLObject::NO_DECODE );

        switch( rSh.GetObjCntTypeOfSelection() )
        {
        case OBJCNT_FLY:
        case OBJCNT_GRF:
        case OBJCNT_OLE:
            {
                SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                rSh.GetFlyFrmAttr( aSet );
                SwFmtURL aURL2( (SwFmtURL&)aSet.Get( RES_URL ) );
                aURL2.SetURL( sFile, sal_False );
                if( !aURL2.GetName().Len() )
                    aURL2.SetName( sFile );
                aSet.Put( aURL2 );
                rSh.SetFlyFrmAttr( aSet );
            }
            break;

        default:
            {
                rSh.InsertURL( SwFmtINetFmt( sFile, aEmptyStr ),
                                sDesc.Len() ? sDesc : sFile );
            }
        }
        nRet = sal_True;
    }
    return nRet;
}

int SwTransferable::_PasteFileName( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, sal_uLong nFmt,
                                    sal_uInt16 nAction, const Point* pPt,
                                    sal_uInt8 nActionFlags, sal_Bool bMsg )
{
    int nRet = SwTransferable::_PasteGrf( rData, rSh, nFmt, nAction,
                                            pPt, nActionFlags, bMsg );
    if( nRet )
        nRet |= SWTRANSFER_GRAPHIC_INSERTED;
    if( !nRet )
    {
        String sFile, sDesc;
        if( rData.GetString( nFmt, sFile ) && sFile.Len() )
        {
            INetURLObject aMediaURL;

            aMediaURL.SetSmartURL( sFile );
            const String aMediaURLStr( aMediaURL.GetMainURL( INetURLObject::NO_DECODE ) );

            if( ::avmedia::MediaWindow::isMediaURL( aMediaURLStr ) )
            {
                const SfxStringItem aMediaURLItem( SID_INSERT_AVMEDIA, aMediaURLStr );
                rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(
                                SID_INSERT_AVMEDIA, SFX_CALLMODE_SYNCHRON,
                                &aMediaURLItem, 0L );
            }
            else
            {
                sal_Bool bIsURLFile = SwTransferable::_CheckForURLOrLNKFile( rData, sFile, &sDesc );

                //Own FileFormat? --> insert, not for StarWriter/Web
                String sFileURL = URIHelper::SmartRel2Abs(INetURLObject(), sFile, Link(), false );
                const SfxFilter* pFlt = SW_PASTESDR_SETATTR == nAction
                        ? 0 : SwIoSystem::GetFileFilter(
                        sFileURL, aEmptyStr );
                if( pFlt && !rSh.GetView().GetDocShell()->ISA(SwWebDocShell) )
                {
    // and then pull up the insert-region-dialog by PostUser event
                    SwSectionData * pSect = new SwSectionData(
                                    FILE_LINK_SECTION,
                                    rSh.GetDoc()->GetUniqueSectionName() );
                    pSect->SetLinkFileName( sFileURL );
                    pSect->SetProtectFlag( true );

                    Application::PostUserEvent( STATIC_LINK( &rSh, SwWrtShell,
                                                InsertRegionDialog ), pSect );
                    nRet = 1;
                    }
                else if( SW_PASTESDR_SETATTR == nAction ||
                        ( bIsURLFile && SW_PASTESDR_INSERT == nAction ))
                {
                    //we can insert foreign files as links after all

                    // first, make the URL absolute
                    INetURLObject aURL;
                    aURL.SetSmartProtocol( INET_PROT_FILE );
                    aURL.SetSmartURL( sFile );
                    sFile = aURL.GetMainURL( INetURLObject::NO_DECODE );

                    switch( rSh.GetObjCntTypeOfSelection() )
                    {
                    case OBJCNT_FLY:
                    case OBJCNT_GRF:
                    case OBJCNT_OLE:
                        {
                            SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                            rSh.GetFlyFrmAttr( aSet );
                            SwFmtURL aURL2( (SwFmtURL&)aSet.Get( RES_URL ) );
                            aURL2.SetURL( sFile, sal_False );
                            if( !aURL2.GetName().Len() )
                                aURL2.SetName( sFile );
                            aSet.Put( aURL2 );
                            rSh.SetFlyFrmAttr( aSet );
                        }
                        break;

                    default:
                        {
                            rSh.InsertURL( SwFmtINetFmt( sFile, aEmptyStr ),
                                            sDesc.Len() ? sDesc : sFile );
                        }
                    }
                    nRet = sal_True;
                }
            }
        }
    }
    return nRet;
}

int SwTransferable::_PasteDBData( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, sal_uLong nFmt, sal_Bool bLink,
                                    const Point* pDragPt, sal_Bool bMsg )
{
    int nRet = 0;
    String sTxt;
    if( rData.GetString( nFmt, sTxt ) && sTxt.Len() )
    {
        sal_uInt16 nWh = SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE == nFmt
                    ? 0
                    : SOT_FORMATSTR_ID_SBA_DATAEXCHANGE == nFmt
                                ? (bLink
                                    ? FN_QRY_MERGE_FIELD
                                    : FN_QRY_INSERT)
                                : (bLink
                                    ? 0
                                    : FN_QRY_INSERT_FIELD );
        DataFlavorExVector& rVector = rData.GetDataFlavorExVector();
        sal_Bool bHaveColumnDescriptor = OColumnTransferable::canExtractColumnDescriptor(rVector, CTF_COLUMN_DESCRIPTOR | CTF_CONTROL_EXCHANGE);
        if ( SOT_FORMATSTR_ID_XFORMS == nFmt )
        {
            SdrObject* pObj;
            rSh.MakeDrawView();
            FmFormView* pFmView = PTR_CAST( FmFormView, rSh.GetDrawView() );
            if(pFmView) {
                const OXFormsDescriptor &rDesc = OXFormsTransferable::extractDescriptor(rData);
                if(0 != (pObj = pFmView->CreateXFormsControl(rDesc)))
                {
                    rSh.SwFEShell::InsertDrawObj( *pObj, *pDragPt );
                }
            }
        }
        else if( nWh )
        {
            SfxUsrAnyItem* pConnectionItem  = 0;
            SfxUsrAnyItem* pCursorItem      = 0;
            SfxUsrAnyItem* pColumnItem      = 0;
            SfxUsrAnyItem* pSourceItem      = 0;
            SfxUsrAnyItem* pCommandItem     = 0;
            SfxUsrAnyItem* pCommandTypeItem = 0;
            SfxUsrAnyItem* pColumnNameItem  = 0;
            SfxUsrAnyItem* pSelectionItem   = 0;

            sal_Bool bDataAvailable = sal_True;
            ODataAccessDescriptor aDesc;
            if(bHaveColumnDescriptor)
                aDesc = OColumnTransferable::extractColumnDescriptor(rData);
            else if(ODataAccessObjectTransferable::canExtractObjectDescriptor(rVector) )
                aDesc = ODataAccessObjectTransferable::extractObjectDescriptor(rData);
            else
                bDataAvailable = sal_False;

            if ( bDataAvailable )
            {
                pConnectionItem = new SfxUsrAnyItem(FN_DB_CONNECTION_ANY, aDesc[daConnection]);
                pColumnItem = new SfxUsrAnyItem(FN_DB_COLUMN_ANY, aDesc[daColumnObject]);
                pSourceItem = new SfxUsrAnyItem(FN_DB_DATA_SOURCE_ANY, makeAny(aDesc.getDataSource()));
                pCommandItem = new SfxUsrAnyItem(FN_DB_DATA_COMMAND_ANY, aDesc[daCommand]);
                pCommandTypeItem = new SfxUsrAnyItem(FN_DB_DATA_COMMAND_TYPE_ANY, aDesc[daCommandType]);
                pColumnNameItem = new SfxUsrAnyItem(FN_DB_DATA_COLUMN_NAME_ANY, aDesc[daColumnName]);
                pSelectionItem = new SfxUsrAnyItem(FN_DB_DATA_SELECTION_ANY, aDesc[daSelection]);
                pCursorItem = new SfxUsrAnyItem(FN_DB_DATA_CURSOR_ANY, aDesc[daCursor]);
            }

            SwView& rView = rSh.GetView();
            //force ::SelectShell
            rView.StopShellTimer();

            SfxStringItem aDataDesc( nWh, sTxt );
            rView.GetViewFrame()->GetDispatcher()->Execute(
                                nWh, SFX_CALLMODE_ASYNCHRON, &aDataDesc,
                                pConnectionItem, pColumnItem,
                                pSourceItem, pCommandItem, pCommandTypeItem,
                                pColumnNameItem, pSelectionItem, pCursorItem,0L);
            delete pConnectionItem;
            delete pColumnItem;
            delete pSourceItem;
            delete pCommandItem;
            delete pCommandTypeItem;
            delete pColumnNameItem;
            delete pCursorItem;
        }
        else
        {
            SdrObject* pObj;
            rSh.MakeDrawView();
            FmFormView* pFmView = PTR_CAST( FmFormView, rSh.GetDrawView() );
            if (pFmView && bHaveColumnDescriptor)
            {
                if ( 0 != (pObj = pFmView->CreateFieldControl( OColumnTransferable::extractColumnDescriptor(rData) ) ) )
                    rSh.SwFEShell::InsertDrawObj( *pObj, *pDragPt );
            }
        }
        nRet = 1;
    }
    else if( bMsg )
    {
        InfoBox( 0, SW_RES(MSG_CLPBRD_FORMAT_ERROR)).Execute();
    }
    return nRet;
}

int SwTransferable::_PasteFileList( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, sal_Bool bLink,
                                    const Point* pPt, sal_Bool bMsg )
{
    int nRet = 0;
    FileList aFileList;
    if( rData.GetFileList( SOT_FORMAT_FILE_LIST, aFileList ) &&
        aFileList.Count() )
    {
        sal_uInt16 nAct = bLink ? SW_PASTESDR_SETATTR : SW_PASTESDR_INSERT;
        String sFlyNm;
        // iterate over the filelist
        for( sal_uLong n = 0, nEnd = aFileList.Count(); n < nEnd; ++n )
        {
            TransferDataContainer* pHlp = new TransferDataContainer;
            pHlp->CopyString( FORMAT_FILE, aFileList.GetFile( n ));
            TransferableDataHelper aData( pHlp );

            if( SwTransferable::_PasteFileName( aData, rSh, SOT_FORMAT_FILE, nAct,
                                            pPt, sal_False, bMsg ))
            {
                if( bLink )
                {
                    sFlyNm = rSh.GetFlyName();
                    SwTransferable::SetSelInShell( rSh, sal_False, pPt );
                }
                nRet = 1;
            }
        }
        if( sFlyNm.Len() )
            rSh.GotoFly( sFlyNm );
    }
    else if( bMsg )
    {
        InfoBox( 0, SW_RES(MSG_CLPBRD_FORMAT_ERROR)).Execute();
    }
    return nRet;
}

sal_Bool SwTransferable::_CheckForURLOrLNKFile( TransferableDataHelper& rData,
                                        String& rFileName, String* pTitle )
{
    sal_Bool bIsURLFile = sal_False;
    INetBookmark aBkmk;
    if( rData.GetINetBookmark( SOT_FORMATSTR_ID_SOLK, aBkmk ) )
    {
        rFileName = aBkmk.GetURL();
        if( pTitle )
            *pTitle = aBkmk.GetDescription();
        bIsURLFile = sal_True;
    }
    else
    {
        xub_StrLen nLen = rFileName.Len();
        if( 4 < nLen && '.' == rFileName.GetChar( nLen - 4 ))
        {
            String sExt( rFileName.Copy( nLen - 3 ));
            if( sExt.EqualsIgnoreCaseAscii( "url" ))
            {
                OSL_ENSURE( !&rFileName, "how do we read today .URL - Files?" );
            }
        }
    }
    return bIsURLFile;
}

sal_Bool SwTransferable::IsPasteSpecial( const SwWrtShell& rWrtShell,
                                     const TransferableDataHelper& rData )
{
    // we can paste-special if there's an entry in the paste-special-format list
    SvxClipboardFmtItem aClipboardFmtItem(0);
    FillClipFmtItem( rWrtShell, rData, aClipboardFmtItem);
    return aClipboardFmtItem.Count() > 0;
}

int SwTransferable::PasteFormat( SwWrtShell& rSh,
                                    TransferableDataHelper& rData,
                                    sal_uLong nFormat )
{
    SwWait aWait( *rSh.GetView().GetDocShell(), sal_False );
    int nRet = 0;

    sal_uLong nPrivateFmt = FORMAT_PRIVATE;
    SwTransferable *pClipboard = GetSwTransferable( rData );
    if( pClipboard &&
        ((TRNSFR_DOCUMENT|TRNSFR_GRAPHIC|TRNSFR_OLE) & pClipboard->eBufferType ))
        nPrivateFmt = SOT_FORMATSTR_ID_EMBED_SOURCE;

    if( pClipboard && nPrivateFmt == nFormat )
        nRet = pClipboard->PrivatePaste( rSh );
    else if( rData.HasFormat( nFormat ) )
    {
        uno::Reference<XTransferable> xTransferable( rData.GetXTransferable() );
        sal_uInt16 nEventAction,
               nDestination = SwTransferable::GetSotDestination( rSh ),
               nSourceOptions =
                    (( EXCHG_DEST_DOC_TEXTFRAME == nDestination ||
                       EXCHG_DEST_SWDOC_FREE_AREA == nDestination ||
                       EXCHG_DEST_DOC_TEXTFRAME_WEB == nDestination ||
                       EXCHG_DEST_SWDOC_FREE_AREA_WEB == nDestination )
                                        ? EXCHG_IN_ACTION_COPY
                                        : EXCHG_IN_ACTION_MOVE),
               nAction = SotExchange::GetExchangeAction(
                                    rData.GetDataFlavorExVector(),
                                    nDestination,
                                    nSourceOptions,             /* ?? */
                                    EXCHG_IN_ACTION_DEFAULT,    /* ?? */
                                    nFormat, nEventAction, nFormat,
                                    lcl_getTransferPointer ( xTransferable ) );

        if( EXCHG_INOUT_ACTION_NONE != nAction )
            nRet = SwTransferable::PasteData( rData, rSh, nAction, nFormat,
                                                nDestination, sal_True, sal_False );
    }
    return nRet;
}

int SwTransferable::_TestAllowedFormat( const TransferableDataHelper& rData,
                                        sal_uLong nFormat, sal_uInt16 nDestination )
{
    sal_uInt16 nAction = EXCHG_INOUT_ACTION_NONE, nEventAction;
    if( rData.HasFormat( nFormat )) {
        uno::Reference<XTransferable> xTransferable( rData.GetXTransferable() );
        nAction = SotExchange::GetExchangeAction(
                        rData.GetDataFlavorExVector(),
                        nDestination, EXCHG_IN_ACTION_COPY,
                        EXCHG_IN_ACTION_COPY, nFormat,
                        nEventAction, nFormat,
                        lcl_getTransferPointer ( xTransferable ) );
    }
    return EXCHG_INOUT_ACTION_NONE != nAction;
}

/**
 * the list of formats which will be offered to the user in the 'Paste
 * Special...' dialog and the paste button menu
 */
static sal_uInt16 aPasteSpecialIds[] =
{
    SOT_FORMATSTR_ID_HTML,
    SOT_FORMATSTR_ID_HTML_SIMPLE,
    SOT_FORMATSTR_ID_HTML_NO_COMMENT,
    FORMAT_RTF,
    FORMAT_STRING,
    SOT_FORMATSTR_ID_SONLK,
    SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK,
    SOT_FORMATSTR_ID_DRAWING,
    SOT_FORMATSTR_ID_SVXB,
    FORMAT_GDIMETAFILE,
    FORMAT_BITMAP,
    SOT_FORMATSTR_ID_SVIM,
    SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR,
    0
};

int SwTransferable::PasteUnformatted( SwWrtShell& rSh, TransferableDataHelper& rData )
{
    // Plain text == unformatted
    return SwTransferable::PasteFormat( rSh, rData, SOT_FORMAT_STRING );
}

int SwTransferable::PasteSpecial( SwWrtShell& rSh, TransferableDataHelper& rData, sal_uLong& rFormatUsed )
{
    int nRet = 0;
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    SfxAbstractPasteDialog* pDlg = pFact->CreatePasteDialog( &rSh.GetView().GetEditWin() );

    DataFlavorExVector aFormats( rData.GetDataFlavorExVector() );
    TransferableObjectDescriptor aDesc;

    sal_uInt16 nDest = SwTransferable::GetSotDestination( rSh );

    SwTransferable *pClipboard = GetSwTransferable( rData );
    if( pClipboard )
    {
        aDesc = pClipboard->aObjDesc;
        sal_uInt16 nResId;
        if( pClipboard->eBufferType & TRNSFR_DOCUMENT )
            nResId = STR_PRIVATETEXT;
        else if( pClipboard->eBufferType & TRNSFR_GRAPHIC )
            nResId = STR_PRIVATEGRAPHIC;
        else if( pClipboard->eBufferType == TRNSFR_OLE )
            nResId = STR_PRIVATEOLE;
        else
            nResId = 0;

        if( nResId )
        {
            if( STR_PRIVATEOLE == nResId || STR_PRIVATEGRAPHIC == nResId )
            {
                // add SOT_FORMATSTR_ID_EMBED_SOURCE to the formats. This
                // format display then the private format name.
                DataFlavorEx aFlavorEx;
                aFlavorEx.mnSotId = SOT_FORMATSTR_ID_EMBED_SOURCE;
                aFormats.insert( aFormats.begin(), aFlavorEx );
            }
            pDlg->SetObjName( pClipboard->aObjDesc.maClassName,
                                SW_RES( nResId ) );
            pDlg->Insert( SOT_FORMATSTR_ID_EMBED_SOURCE, aEmptyStr );
        }
    }
    else
    {
        if( rData.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) )
            rData.GetTransferableObjectDescriptor(
                                SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aDesc );

        if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_EMBED_SOURCE, nDest ))
            pDlg->Insert( SOT_FORMATSTR_ID_EMBED_SOURCE, aEmptyStr );
        if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_LINK_SOURCE, nDest ))
            pDlg->Insert( SOT_FORMATSTR_ID_LINK_SOURCE, aEmptyStr );
    }

    if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_LINK, nDest ))
        pDlg->Insert( SOT_FORMATSTR_ID_LINK, SW_RES(STR_DDEFORMAT) );

    for( sal_uInt16* pIds = aPasteSpecialIds; *pIds; ++pIds )
        if( SwTransferable::_TestAllowedFormat( rData, *pIds, nDest ))
            pDlg->Insert( *pIds, aEmptyStr );

    sal_uLong nFormat = pDlg->GetFormat( rData.GetTransferable() );

    if( nFormat )
        nRet = SwTransferable::PasteFormat( rSh, rData, nFormat );

    if ( nRet )
        rFormatUsed = nFormat;

    delete pDlg;
    return nRet;
}

void SwTransferable::FillClipFmtItem( const SwWrtShell& rSh,
                                const TransferableDataHelper& rData,
                                SvxClipboardFmtItem & rToFill )
{
    sal_uInt16 nDest = SwTransferable::GetSotDestination( rSh );

    SwTransferable *pClipboard = GetSwTransferable( rData );
    if( pClipboard )
    {
        sal_uInt16 nResId;
        if( pClipboard->eBufferType & TRNSFR_DOCUMENT )
            nResId = STR_PRIVATETEXT;
        else if( pClipboard->eBufferType & TRNSFR_GRAPHIC )
            nResId = STR_PRIVATEGRAPHIC;
        else if( pClipboard->eBufferType == TRNSFR_OLE )
            nResId = STR_PRIVATEOLE;
        else
            nResId = 0;

        if( nResId )
            rToFill.AddClipbrdFormat( SOT_FORMATSTR_ID_EMBED_SOURCE,
                                        SW_RESSTR( nResId ) );
    }
    else
    {
        TransferableObjectDescriptor aDesc;
        if( rData.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) )
            ((TransferableDataHelper&)rData).GetTransferableObjectDescriptor(
                                SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aDesc );

        if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_EMBED_SOURCE, nDest ))
            rToFill.AddClipbrdFormat( SOT_FORMATSTR_ID_EMBED_SOURCE,
                                            aDesc.maTypeName );
        if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_LINK_SOURCE, nDest ))
            rToFill.AddClipbrdFormat( SOT_FORMATSTR_ID_LINK_SOURCE );

        SotFormatStringId nFormat;
        if ( rData.HasFormat(nFormat = SOT_FORMATSTR_ID_EMBED_SOURCE_OLE) || rData.HasFormat(nFormat = SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE) )
        {
            String sName,sSource;
            if ( SvPasteObjectHelper::GetEmbeddedName(rData,sName,sSource,nFormat) )
                rToFill.AddClipbrdFormat( nFormat, sName );
        }
    }

    if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_LINK, nDest ))
        rToFill.AddClipbrdFormat( SOT_FORMATSTR_ID_LINK, SW_RESSTR(STR_DDEFORMAT) );

    for( sal_uInt16* pIds = aPasteSpecialIds; *pIds; ++pIds )
        if( SwTransferable::_TestAllowedFormat( rData, *pIds, nDest ))
            rToFill.AddClipbrdFormat( *pIds, aEmptyStr );
}

void SwTransferable::SetDataForDragAndDrop( const Point& rSttPos )
{
    if(!pWrtShell)
        return;
    String sGrfNm;
    const int nSelection = pWrtShell->GetSelectionType();
    if( nsSelectionType::SEL_GRF == nSelection)
    {
        AddFormat( SOT_FORMATSTR_ID_SVXB );
        const Graphic* pGrf = pWrtShell->GetGraphic();
        if ( pGrf && pGrf->IsSupportedGraphic() )
        {
            AddFormat( FORMAT_GDIMETAFILE );
            AddFormat( FORMAT_BITMAP );
        }
        eBufferType = TRNSFR_GRAPHIC;
        pWrtShell->GetGrfNms( &sGrfNm, 0 );
    }
    else if( nsSelectionType::SEL_OLE == nSelection )
    {
        AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
        PrepareOLE( aObjDesc );
        AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
        AddFormat( FORMAT_GDIMETAFILE );
        eBufferType = TRNSFR_OLE;
    }
    //Is there anything to provide anyway?
    else if ( pWrtShell->IsSelection() || pWrtShell->IsFrmSelected() ||
              pWrtShell->IsObjSelected() )
    {
        if( pWrtShell->IsObjSelected() )
            eBufferType = TRNSFR_DRAWING;
        else
        {
            eBufferType = TRNSFR_DOCUMENT;
            if( SwWrtShell::NO_WORD !=
                pWrtShell->IntelligentCut( nSelection, sal_False ))
                eBufferType = TransferBufferType( TRNSFR_DOCUMENT_WORD
                                                    | eBufferType);
        }

        if( nSelection & nsSelectionType::SEL_TBL_CELLS )
            eBufferType = (TransferBufferType)(TRNSFR_TABELLE | eBufferType);

        AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );

        //put RTF ahead of the OLE's Metafile for less loss
        if( !pWrtShell->IsObjSelected() )
        {
            AddFormat( FORMAT_RTF );
            AddFormat( SOT_FORMATSTR_ID_HTML );
        }
        if( pWrtShell->IsSelection() )
            AddFormat( FORMAT_STRING );

        if( nSelection & ( nsSelectionType::SEL_DRW | nsSelectionType::SEL_DRW_FORM ))
        {
            AddFormat( SOT_FORMATSTR_ID_DRAWING );
            if ( nSelection & nsSelectionType::SEL_DRW )
            {
                AddFormat( FORMAT_GDIMETAFILE );
                AddFormat( FORMAT_BITMAP );
            }
            eBufferType = (TransferBufferType)( TRNSFR_GRAPHIC | eBufferType );

            pClpGraphic = new Graphic;
            if( !pWrtShell->GetDrawObjGraphic( FORMAT_GDIMETAFILE, *pClpGraphic ))
                pOrigGrf = pClpGraphic;
            pClpBitmap = new Graphic;
            if( !pWrtShell->GetDrawObjGraphic( FORMAT_BITMAP, *pClpBitmap ))
                pOrigGrf = pClpBitmap;

            // is it an URL-Button ?
            String sURL, sDesc;
            if( pWrtShell->GetURLFromButton( sURL, sDesc ) )
            {
                AddFormat( FORMAT_STRING );
                 AddFormat( SOT_FORMATSTR_ID_SOLK );
                 AddFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
                 AddFormat( SOT_FORMATSTR_ID_FILECONTENT );
                 AddFormat( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR );
                 AddFormat( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR );
                eBufferType = (TransferBufferType)( TRNSFR_INETFLD | eBufferType );
            }
        }

        //ObjectDescriptor was already filled from the old DocShell.
        //Now adjust it. Thus in GetData the first query can still
        //be answered with delayed rendering.
        aObjDesc.mbCanLink = sal_False;
        aObjDesc.maDragStartPos = rSttPos;
        aObjDesc.maSize = OutputDevice::LogicToLogic( Size( OLESIZE ),
                                                MAP_TWIP, MAP_100TH_MM );
        PrepareOLE( aObjDesc );
        AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
    }
    else if( nSelection & nsSelectionType::SEL_TXT && !pWrtShell->HasMark() )
    {
        // is only one field - selected?
        SwContentAtPos aCntntAtPos( SwContentAtPos::SW_INETATTR );
        Point aPos( SwEditWin::GetDDStartPosX(), SwEditWin::GetDDStartPosY());

        if( pWrtShell->GetContentAtPos( aPos, aCntntAtPos ) )
        {
            AddFormat( FORMAT_STRING );
             AddFormat( SOT_FORMATSTR_ID_SOLK );
             AddFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
             AddFormat( SOT_FORMATSTR_ID_FILECONTENT );
             AddFormat( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR );
             AddFormat( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR );
            eBufferType = TRNSFR_INETFLD;
        }
    }

    if( pWrtShell->IsFrmSelected() )
    {
        SfxItemSet aSet( pWrtShell->GetAttrPool(), RES_URL, RES_URL );
        pWrtShell->GetFlyFrmAttr( aSet );
        const SwFmtURL& rURL = (SwFmtURL&)aSet.Get( RES_URL );
        if( rURL.GetMap() )
        {
            pImageMap = new ImageMap( *rURL.GetMap() );
            AddFormat( SOT_FORMATSTR_ID_SVIM );
        }
        else if( rURL.GetURL().Len() )
        {
            pTargetURL = new INetImage( sGrfNm, rURL.GetURL(),
                                        rURL.GetTargetFrameName(),
                                        aEmptyStr, Size() );
            AddFormat( SOT_FORMATSTR_ID_INET_IMAGE );
        }
    }
}

void SwTransferable::StartDrag( Window* pWin, const Point& rPos )
{
    if(!pWrtShell)
        return;
    bOldIdle = pWrtShell->GetViewOptions()->IsIdle();
    bCleanUp = sal_True;

    ((SwViewOption *)pWrtShell->GetViewOptions())->SetIdle( sal_False );

    if( pWrtShell->IsSelFrmMode() )
        pWrtShell->ShowCrsr();

    SW_MOD()->pDragDrop = this;

    SetDataForDragAndDrop( rPos );

    sal_Int8 nDragOptions = DND_ACTION_COPYMOVE | DND_ACTION_LINK;
    SwDocShell* pDShell = pWrtShell->GetView().GetDocShell();
    if( ( pDShell && pDShell->IsReadOnly() ) || pWrtShell->HasReadonlySel() )
        nDragOptions &= ~DND_ACTION_MOVE;

    TransferableHelper::StartDrag( pWin, nDragOptions );
}

void SwTransferable::DragFinished( sal_Int8 nAction )
{
    //And the last finishing work so that all statuses are right
    if( DND_ACTION_MOVE == nAction  )
    {
        if( bCleanUp )
        {
            //It was dropped outside of Writer. We still have to
            //delete.

            pWrtShell->StartAllAction();
            pWrtShell->StartUndo( UNDO_UI_DRAG_AND_MOVE );
            if ( pWrtShell->IsTableMode() )
                pWrtShell->DeleteTblSel();
            else
            {
                if ( !(pWrtShell->IsSelFrmMode() || pWrtShell->IsObjSelected()) )
                    //SmartCut, take one of the blanks along
                    pWrtShell->IntelligentCut( pWrtShell->GetSelectionType(), sal_True );
                pWrtShell->DelRight();
            }
            pWrtShell->EndUndo( UNDO_UI_DRAG_AND_MOVE );
            pWrtShell->EndAllAction();
        }
        else
        {
            const int nSelection = pWrtShell->GetSelectionType();
            if( ( nsSelectionType::SEL_FRM | nsSelectionType::SEL_GRF |
                 nsSelectionType::SEL_OLE | nsSelectionType::SEL_DRW ) & nSelection )
            {
                pWrtShell->EnterSelFrmMode();
            }
        }
    }
    pWrtShell->GetView().GetEditWin().DragFinished();

    if( pWrtShell->IsSelFrmMode() )
        pWrtShell->HideCrsr();
    else
        pWrtShell->ShowCrsr();

    ((SwViewOption *)pWrtShell->GetViewOptions())->SetIdle( bOldIdle );
}

int SwTransferable::PrivatePaste( SwWrtShell& rShell )
{
    // first, ask for the SelectionType, then action-bracketing !!!!
    // (otherwise it's not pasted into a TableSelection!!!)
    OSL_ENSURE( !rShell.ActionPend(), "Paste darf nie eine Actionklammerung haben" );
    if ( !pClpDocFac )
        return sal_False; // the return value of the SwFEShell::Paste also is sal_Bool!

    const int nSelection = rShell.GetSelectionType();

    SwRewriter aRewriter;

    SwTrnsfrActionAndUndo aAction( &rShell, UNDO_PASTE_CLIPBOARD);

    bool bKillPaMs = false;

    //Delete selected content, not at table-selection and table in Clipboard
    if( rShell.HasSelection() && !( nSelection & nsSelectionType::SEL_TBL_CELLS))
    {
        bKillPaMs = true;
        rShell.SetRetainSelection( true );
        rShell.DelRight();
        // when a Fly was selected, a valid cursor position has to be found now
        // (parked Cursor!)
        if( ( nsSelectionType::SEL_FRM | nsSelectionType::SEL_GRF |
            nsSelectionType::SEL_OLE | nsSelectionType::SEL_DRW |
            nsSelectionType::SEL_DRW_FORM ) & nSelection )
        {
            // position the cursor again
            Point aPt( rShell.GetCharRect().Pos() );
            rShell.SwCrsrShell::SetCrsr( aPt, sal_True );
        }
        rShell.SetRetainSelection( false );
    }

    sal_Bool bInWrd = sal_False, bEndWrd = sal_False, bSttWrd = sal_False,
         bSmart = 0 != (TRNSFR_DOCUMENT_WORD & eBufferType);
    if( bSmart )
    {
// Why not for other Scripts? If TRNSFR_DOCUMENT_WORD is set, we have a word
// in the buffer, word in this context means 'something with spaces at beginning
// and end'. In this case we definitely want these spaces to be inserted here.
            bInWrd = rShell.IsInWrd();
             bEndWrd = rShell.IsEndWrd();
            bSmart = bInWrd || bEndWrd;
            if( bSmart )
            {
                 bSttWrd = rShell.IsSttWrd();
                if( bSmart && !bSttWrd && (bInWrd || bEndWrd) )
                    rShell.SwEditShell::Insert(' ');
            }
    }

    int nRet = rShell.Paste( pClpDocFac->GetDoc() );

    if( bKillPaMs )
        rShell.KillPams();

    // If Smart Paste then insert blank
    if( nRet && bSmart && ((bInWrd && !bEndWrd )|| bSttWrd) )
        rShell.SwEditShell::Insert(' ');

    return nRet;
}

int SwTransferable::PrivateDrop( SwWrtShell& rSh, const Point& rDragPt,
                                sal_Bool bMove, sal_Bool bIsXSelection )
{
    int cWord    = 0;
    sal_Bool bInWrd  = sal_False;
    sal_Bool bEndWrd = sal_False;
    sal_Bool bSttWrd = sal_False;
    bool bSttPara = false;
    sal_Bool bTblSel = sal_False;
    sal_Bool bFrmSel = sal_False;

    SwWrtShell& rSrcSh = *GetShell();

    rSh.UnSetVisCrsr();

    if( TRNSFR_INETFLD == eBufferType )
    {
        if( rSh.GetFmtFromObj( rDragPt ) )
        {
            INetBookmark aTmp;
            if( (TRNSFR_INETFLD & eBufferType) && pBkmk )
                aTmp = *pBkmk;

            // select target graphic
            if( rSh.SelectObj( rDragPt ) )
            {
                rSh.HideCrsr();
                rSh.EnterSelFrmMode( &rDragPt );
                bFrmDrag = sal_True;
            }

            const int nSelection = rSh.GetSelectionType();

            // not yet consider Draw objects
            if( nsSelectionType::SEL_GRF & nSelection )
            {
                SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                rSh.GetFlyFrmAttr( aSet );
                SwFmtURL aURL( (SwFmtURL&)aSet.Get( RES_URL ) );
                aURL.SetURL( aTmp.GetURL(), sal_False );
                aSet.Put( aURL );
                rSh.SetFlyFrmAttr( aSet );
                return 1;
            }

            if( nsSelectionType::SEL_DRW & nSelection )
            {
                rSh.LeaveSelFrmMode();
                rSh.UnSelectFrm();
                rSh.ShowCrsr();
                bFrmDrag = sal_False;
            }
        }
    }

    if( &rSh != &rSrcSh && (nsSelectionType::SEL_GRF & rSh.GetSelectionType()) &&
        TRNSFR_GRAPHIC == eBufferType )
    {
        // ReRead the graphic
        String sGrfNm, sFltNm;
        rSrcSh.GetGrfNms( &sGrfNm, &sFltNm );
        rSh.ReRead( sGrfNm, sFltNm, rSrcSh.GetGraphic() );
        return 1;
    }

    //not in selections or selected frames
    if( rSh.ChgCurrPam( rDragPt ) ||
        ( rSh.IsSelFrmMode() && rSh.IsInsideSelectedObj( rDragPt )) )
        return 0;

    if( rSrcSh.IsTableMode() )
        bTblSel = sal_True;
    else if( rSrcSh.IsSelFrmMode() || rSrcSh.IsObjSelected() )
    {
        // don't move position-protected objects!
        if( bMove && rSrcSh.IsSelObjProtected( FLYPROTECT_POS ) )
            return 0;

        bFrmSel = sal_True;
    }

    const int nSel = rSrcSh.GetSelectionType();

    SwUndoId eUndoId = bMove ? UNDO_UI_DRAG_AND_MOVE : UNDO_UI_DRAG_AND_COPY;

    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, rSrcSh.GetSelDescr());

    if(rSrcSh.GetDoc() != rSh.GetDoc())
        rSrcSh.StartUndo( eUndoId, &aRewriter );
    rSh.StartUndo( eUndoId, &aRewriter );

    rSh.StartAction();
    rSrcSh.StartAction();

    if( &rSrcSh != &rSh )
    {
        rSh.EnterStdMode();
        rSh.SwCrsrShell::SetCrsr( rDragPt, sal_True );
        cWord = rSrcSh.IntelligentCut( nSel, sal_False );
    }
    else if( !bTblSel && !bFrmSel )
    {
        if( !rSh.IsAddMode() )
        {
            // #i87233#
            if ( rSh.IsBlockMode() )
            {
                // preserve order of cursors for block mode
                rSh.GoPrevCrsr();
            }

            rSh.SwCrsrShell::CreateCrsr();
        }
        rSh.SwCrsrShell::SetCrsr( rDragPt, sal_True, false );
        rSh.GoPrevCrsr();
        cWord = rSh.IntelligentCut( rSh.GetSelectionType(), sal_False );
        rSh.GoNextCrsr();
    }

    bInWrd  = rSh.IsInWrd();
    bEndWrd = rSh.IsEndWrd();
    bSttWrd = !bEndWrd && rSh.IsSttWrd();
    bSttPara= rSh.IsSttPara();

    Point aSttPt( SwEditWin::GetDDStartPosX(), SwEditWin::GetDDStartPosY() );

    // at first, select INetFelder!
    if( TRNSFR_INETFLD == eBufferType )
    {
        if( &rSrcSh == &rSh )
        {
            rSh.GoPrevCrsr();
            rSh.SwCrsrShell::SetCrsr( aSttPt, sal_True );
            rSh.SelectTxtAttr( RES_TXTATR_INETFMT );
            if( rSh.ChgCurrPam( rDragPt ) )
            {
                // don't copy/move inside of yourself
                rSh.DestroyCrsr();
                rSh.EndUndo();
                rSh.EndAction();
                rSh.EndAction();
                return 0;
            }
            rSh.GoNextCrsr();
        }
        else
        {
            rSrcSh.SwCrsrShell::SetCrsr( aSttPt, sal_True );
            rSrcSh.SelectTxtAttr( RES_TXTATR_INETFMT );
        }

        // is there an URL attribute at the insert point? Then replace that,
        // so simply put up a selection?
        rSh.DelINetAttrWithText();
        bDDINetAttr = sal_True;
    }

    if ( rSrcSh.IsSelFrmMode() )
    {
        //Hack: fool the special treatment
        aSttPt -= aSttPt - rSrcSh.GetObjRect().Pos();
    }

    sal_Bool bRet = rSrcSh.SwFEShell::Copy( &rSh, aSttPt, rDragPt, bMove,
                                            !bIsXSelection );

    if( !bIsXSelection )
    {
        rSrcSh.Push();
        if ( bRet && bMove && !bFrmSel )
        {
            if ( bTblSel )
            {
                /* delete table contents not cells */
                rSrcSh.Delete();
            }
            else
            {
                //SmartCut, take one of the blanks along.
                rSh.SwCrsrShell::DestroyCrsr();
                if ( cWord == SwWrtShell::WORD_SPACE_BEFORE )
                    rSh.ExtendSelection( sal_False );
                else if ( cWord == SwWrtShell::WORD_SPACE_AFTER )
                    rSh.ExtendSelection();
                rSrcSh.DelRight();
            }
        }
        rSrcSh.KillPams();
        rSrcSh.Pop( sal_False );

        /* after dragging a table selection inside one shell
            set cursor to the drop position. */
        if( &rSh == &rSrcSh && ( bTblSel || rSh.IsBlockMode() ) )
        {
            rSrcSh.SwCrsrShell::SetCrsr(rDragPt);
            rSrcSh.GetSwCrsr()->SetMark();
        }
    }

    if( bRet && !bTblSel && !bFrmSel )
    {
        if( (bInWrd || bEndWrd) &&
            (cWord == SwWrtShell::WORD_SPACE_AFTER ||
                cWord == SwWrtShell::WORD_SPACE_BEFORE) )
        {
            if ( bSttWrd || (bInWrd && !bEndWrd))
                rSh.SwEditShell::Insert(' ', bIsXSelection);
            if ( !bSttWrd || (bInWrd && !bSttPara) )
            {
                rSh.SwapPam();
                if ( !bSttWrd )
                    rSh.SwEditShell::Insert(' ', bIsXSelection);
                rSh.SwapPam();
            }
        }

        if( bIsXSelection )
        {
            if( &rSrcSh == &rSh && !rSh.IsAddMode() )
            {
                rSh.SwCrsrShell::DestroyCrsr();
                rSh.GoPrevCrsr();
            }
            else
            {
                rSh.SwapPam();
                rSh.SwCrsrShell::ClearMark();
            }
        }
        else
        {
            if( rSh.IsAddMode() )
                rSh.SwCrsrShell::CreateCrsr();
            else
            {
                // turn on selection mode
                rSh.SttSelect();
                rSh.EndSelect();
            }
        }
    }

    if( bRet && bMove && bFrmSel )
        rSrcSh.LeaveSelFrmMode();

    if( rSrcSh.GetDoc() != rSh.GetDoc() )
        rSrcSh.EndUndo();
    rSh.EndUndo();

        // put the shell in the right state
    if( &rSrcSh != &rSh && ( rSh.IsFrmSelected() || rSh.IsObjSelected() ))
        rSh.EnterSelFrmMode();

    rSrcSh.EndAction();
    rSh.EndAction();
    return 1;
}

// Interfaces for Selection
void SwTransferable::CreateSelection( SwWrtShell& rSh,
                                      const ViewShell * _pCreatorView )
{
    SwModule *pMod = SW_MOD();
    SwTransferable* pNew = new SwTransferable( rSh );

     pNew->pCreatorView = _pCreatorView;

    uno::Reference<
            datatransfer::XTransferable > xRef( pNew );
    pMod->pXSelection = pNew;
    pNew->CopyToSelection( rSh.GetWin() );
}

void SwTransferable::ClearSelection( SwWrtShell& rSh,
                                     const ViewShell * _pCreatorView)
{
    SwModule *pMod = SW_MOD();
    if( pMod->pXSelection &&
        ((!pMod->pXSelection->pWrtShell) || (pMod->pXSelection->pWrtShell == &rSh)) &&
        (!_pCreatorView || (pMod->pXSelection->pCreatorView == _pCreatorView)) )
    {
        TransferableHelper::ClearSelection( rSh.GetWin() );
    }
}

namespace
{
    class theSwTransferableUnoTunnelId : public rtl::Static< UnoTunnelIdInit, SwTransferable > {};
}

const Sequence< sal_Int8 >& SwTransferable::getUnoTunnelId()
{
    return theSwTransferableUnoTunnelId::get().getSeq();
}

sal_Int64 SwTransferable::getSomething( const Sequence< sal_Int8 >& rId ) throw( RuntimeException )
{
    sal_Int64 nRet;
    if( ( rId.getLength() == 16 ) &&
        ( 0 == memcmp( getUnoTunnelId().getConstArray(), rId.getConstArray(), 16 ) ) )
    {
        nRet = sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ) );
    }
    else
        nRet = TransferableHelper::getSomething(rId);
    return nRet;
}

SwTransferable* SwTransferable::GetSwTransferable( const TransferableDataHelper& rData )
{
    SwTransferable* pSwTransferable = NULL;

    uno::Reference<XUnoTunnel> xTunnel( rData.GetTransferable(), UNO_QUERY );
    if ( xTunnel.is() )
    {
        sal_Int64 nHandle = xTunnel->getSomething( getUnoTunnelId() );
        if ( nHandle )
            pSwTransferable = (SwTransferable*) (sal_IntPtr) nHandle;
    }

    return pSwTransferable;

}

SwTrnsfrDdeLink::SwTrnsfrDdeLink( SwTransferable& rTrans, SwWrtShell& rSh )
    : rTrnsfr( rTrans ), pDocShell( 0 ),
    bDelBookmrk( sal_False ), bInDisconnect( sal_False )
{
    // we only end up here with table- or text selection
    if( nsSelectionType::SEL_TBL_CELLS & rSh.GetSelectionType() )
    {
        SwFrmFmt* pFmt = rSh.GetTableFmt();
        if( pFmt )
            sName = pFmt->GetName();
    }
    else
    {
        // creating a temp. bookmark without undo
        sal_Bool bUndo = rSh.DoesUndo();
        rSh.DoUndo( sal_False );
        sal_Bool bIsModified = rSh.IsModified();

        ::sw::mark::IMark* pMark = rSh.SetBookmark(
            KeyCode(),
            ::rtl::OUString(),
            ::rtl::OUString(),
            IDocumentMarkAccess::DDE_BOOKMARK);
        if(pMark)
        {
            sName = pMark->GetName();
            bDelBookmrk = sal_True;
            if( !bIsModified )
                rSh.ResetModified();
        }
        else
            sName.Erase();
        rSh.DoUndo( bUndo );
    }

    if( sName.Len() &&
        0 != ( pDocShell = rSh.GetDoc()->GetDocShell() ) )
    {
        // then we create our "server" and connect to it
        refObj = pDocShell->DdeCreateLinkSource( sName );
        if( refObj.Is() )
        {
            refObj->AddConnectAdvise( this );
            refObj->AddDataAdvise( this,
                            aEmptyStr,
                            ADVISEMODE_NODATA | ADVISEMODE_ONLYONCE );
            nOldTimeOut = refObj->GetUpdateTimeout();
            refObj->SetUpdateTimeout( 0 );
        }
    }
}

SwTrnsfrDdeLink::~SwTrnsfrDdeLink()
{
    if( refObj.Is() )
        Disconnect( sal_True );
}

::sfx2::SvBaseLink::UpdateResult SwTrnsfrDdeLink::DataChanged( const String& ,
                                    const uno::Any& )
{
    // well, that's it with the link
    if( !bInDisconnect )
    {
        if( FindDocShell() && pDocShell->GetView() )
            rTrnsfr.RemoveDDELinkFormat( pDocShell->GetView()->GetEditWin() );
        Disconnect( sal_False );
    }
    return SUCCESS;
}

sal_Bool SwTrnsfrDdeLink::WriteData( SvStream& rStrm )
{
    if( !refObj.Is() || !FindDocShell() )
        return sal_False;

    rtl_TextEncoding eEncoding = DDE_TXT_ENCODING;
    const rtl::OString aAppNm(rtl::OUStringToOString(
        GetpApp()->GetAppName(), eEncoding));
    const rtl::OString aTopic(rtl::OUStringToOString(
        pDocShell->GetTitle(SFX_TITLE_FULLNAME), eEncoding));
    const rtl::OString aName(rtl::OUStringToOString(sName, eEncoding));

    sal_Char* pMem = new sal_Char[ aAppNm.getLength() + aTopic.getLength() + aName.getLength() + 4 ];

    sal_Int32 nLen = aAppNm.getLength();
    memcpy( pMem, aAppNm.getStr(), nLen );
    pMem[ nLen++ ] = 0;
    memcpy( pMem + nLen, aTopic.getStr(), aTopic.getLength() );
    nLen = nLen + aTopic.getLength();
    pMem[ nLen++ ] = 0;
    memcpy( pMem + nLen, aName.getStr(), aName.getLength() );
    nLen = nLen + aName.getLength();
    pMem[ nLen++ ] = 0;
    pMem[ nLen++ ] = 0;

    rStrm.Write( pMem, nLen );
    delete[] pMem;

    IDocumentMarkAccess* const pMarkAccess = pDocShell->GetDoc()->getIDocumentMarkAccess();
    IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->findMark(sName);
    if(ppMark != pMarkAccess->getMarksEnd()
        && IDocumentMarkAccess::GetType(**ppMark) != IDocumentMarkAccess::BOOKMARK)
    {
        // the mark is still a DdeBookmark
        // we replace it with a Bookmark, so it will get saved etc.
        ::sw::mark::IMark* const pMark = ppMark->get();
        SwServerObject* const pServerObject = dynamic_cast<SwServerObject *>(&refObj);

        // collecting state of old mark
        SwPaM aPaM(pMark->GetMarkStart());
        *aPaM.GetPoint() = pMark->GetMarkStart();
        if(pMark->IsExpanded())
        {
            aPaM.SetMark();
            *aPaM.GetMark() = pMark->GetMarkEnd();
        }
        ::rtl::OUString sMarkName = pMark->GetName();

        // remove mark
        pServerObject->SetNoServer(); // this removes the connection between SwServerObject and mark
        // N.B. ppMark was not loaded from file and cannot have xml:id
        pMarkAccess->deleteMark(ppMark);

        // recreate as Bookmark
        ::sw::mark::IMark* const pNewMark = pMarkAccess->makeMark(
            aPaM,
            sMarkName,
            IDocumentMarkAccess::BOOKMARK);
        pServerObject->SetDdeBookmark(*pNewMark);
    }

    bDelBookmrk = false;
    return true;
}

void SwTrnsfrDdeLink::Disconnect( sal_Bool bRemoveDataAdvise )
{
    //  don't accept DataChanged anymore, when already in Disconnect!
    //  (DTOR from Bookmark sends a DataChanged!)
    sal_Bool bOldDisconnect = bInDisconnect;
    bInDisconnect = sal_True;

    // destroy the unused bookmark again (without Undo!)?
    if( bDelBookmrk && refObj.Is() && FindDocShell() )
    {
        SwDoc* pDoc = pDocShell->GetDoc();
        ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

        // #i58448#
        Link aSavedOle2Link( pDoc->GetOle2Link() );
        pDoc->SetOle2Link( Link() );

        sal_Bool bIsModified = pDoc->IsModified();

        IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
        pMarkAccess->deleteMark(pMarkAccess->findMark(sName));

        if( !bIsModified )
            pDoc->ResetModified();
        // #i58448#
        pDoc->SetOle2Link( aSavedOle2Link );

        bDelBookmrk = sal_False;
    }

    if( refObj.Is() )
    {
        refObj->SetUpdateTimeout( nOldTimeOut );
        refObj->RemoveConnectAdvise( this );
        if( bRemoveDataAdvise )
            // in a DataChanged the SelectionObject must NEVER be deleted
            // is already handled by the base class
            // (ADVISEMODE_ONLYONCE!!!!)
            // but always in normal Disconnect!
            refObj->RemoveAllDataAdvise( this );
        refObj.Clear();
    }
    bInDisconnect = bOldDisconnect;
}

sal_Bool SwTrnsfrDdeLink::FindDocShell()
{
    TypeId aType( TYPE( SwDocShell ) );
    SfxObjectShell* pTmpSh = SfxObjectShell::GetFirst( &aType );
    while( pTmpSh )
    {
        if( pTmpSh == pDocShell )       // that's what we want to have
        {
            if( pDocShell->GetDoc() )
                return sal_True;
            break;      // the Doc is not there anymore, so leave!
        }
        pTmpSh = SfxObjectShell::GetNext( *pTmpSh, &aType );
    }

    pDocShell = 0;
    return sal_False;
}

void SwTrnsfrDdeLink::Closed()
{
    if( !bInDisconnect && refObj.Is() )
    {
        refObj->RemoveAllDataAdvise( this );
        refObj->RemoveConnectAdvise( this );
        refObj.Clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
