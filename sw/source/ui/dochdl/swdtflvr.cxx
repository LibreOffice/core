/*************************************************************************
 *
 *  $RCSfile: swdtflvr.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: tl $ $Date: 2001-03-30 15:08:54 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _LINKMGR_HXX
#include <so3/linkmgr.hxx>
#endif
#ifndef _INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _CLIP_HXX
#include <vcl/clip.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _PASTEDLG_HXX
#include <so3/pastedlg.hxx>
#endif
#ifndef _WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _IMAP_HXX
#include <svtools/imap.hxx>
#endif
#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVX_XEXCH_HXX
#include <svx/xexch.hxx>
#endif
#ifndef _SVX_CLIPFMTITEM_HXX
#include <svx/clipfmtitem.hxx>
#endif

#ifndef _MIECLIP_HXX
#include <sfx2/mieclip.hxx>
#endif
#ifndef _OFF_APP_HXX
#include <offmgr/app.hxx>
#endif
#ifndef _SVDETC_HXX
#include <svx/svdetc.hxx>
#endif
#ifndef _XOUTBMP_HXX
#include <svx/xoutbmp.hxx>
#endif
#ifndef _URLBMK_HXX
#include <svtools/urlbmk.hxx>
#endif
#ifndef _HTMLOUT_HXX
#include <svtools/htmlout.hxx>
#endif
#ifndef _INETIMG_HXX
#include <svtools/inetimg.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX
#include <svx/paperinf.hxx>
#endif
#ifndef _SVX_FMVIEW_HXX
#include <svx/fmview.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX
#include <svx/impgrf.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef _FILELIST_HXX
#include <so3/filelist.hxx>
#endif
#ifndef _LINKSRC_HXX
#include <so3/linksrc.hxx>
#endif
#ifndef _GOODIES_IMAPOBJ_HXX
#include <svtools/imapobj.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif


#ifndef _FMTURL_HXX
#include <fmturl.hxx>
#endif
#ifndef _FMTINFMT_HXX
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTFSIZE_HXX
#include <fmtfsize.hxx>
#endif
#ifndef _SWDTFLVR_HXX
#include <swdtflvr.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _EDTWIN_HXX
#include <edtwin.hxx>
#endif
#ifndef _NAVICONT_HXX
#include <navicont.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>       //DDE
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>       // fuer Undo-Ids
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _SWWAIT_HXX
#include <swwait.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _DOCHDL_HRC
#include <dochdl.hrc>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif

extern BOOL bFrmDrag;
extern BOOL bDDINetAttr;
extern BOOL bExecuteDrag;


#define OLESIZE lA4Width - 2 * lMinBorder, 6 * MM50

#define SWTRANSFER_OBJECTTYPE_DRAWMODEL         0x00000001
#define SWTRANSFER_OBJECTTYPE_HTML              0x00000002
#define SWTRANSFER_OBJECTTYPE_RTF               0x00000004
#define SWTRANSFER_OBJECTTYPE_STRING            0x00000008
#define SWTRANSFER_OBJECTTYPE_SWOLE             0x00000010
#define SWTRANSFER_OBJECTTYPE_DDE               0x00000020



#ifdef DDE_AVAILABLE

#ifdef PM2
#define DDE_TXT_ENCODING    RTL_TEXTENCODING_IBM_850
#else
#define DDE_TXT_ENCODING    RTL_TEXTENCODING_MS_1252
#endif

class SwTrnsfrDdeLink : public ::so3::SvBaseLink
{
    String sName;
    ::so3::SvLinkSourceRef refObj;
    SwTransferable& rTrnsfr;
    SwDocShell* pDocShell;
    ULONG nOldTimeOut;
    BOOL bDelBookmrk : 1;
    BOOL bInDisconnect : 1;

    BOOL FindDocShell();

protected:
    virtual ~SwTrnsfrDdeLink();

public:
    SwTrnsfrDdeLink( SwTransferable& rTrans, SwWrtShell& rSh );

    virtual void DataChanged( const String& rMimeType,
                              const ::com::sun::star::uno::Any & rValue );
    virtual void Closed();

    BOOL WriteData( SvStream& rStrm );
    void Disconnect( BOOL bRemoveDataAdvise = TRUE );
};

#endif

// helper class for Action and Undo enclosing
class SwTrnsfrActionAndUndo
{
    SwWrtShell *pSh;
    USHORT nUndoId;
public:
    SwTrnsfrActionAndUndo( SwWrtShell *pS, USHORT nId, BOOL bDelSel = FALSE )
        : pSh( pS ), nUndoId( nId )
    {
        pSh->StartUndo( nUndoId );
        if( bDelSel )
            pSh->DelRight();
        pSh->StartAllAction();
    }
    ~SwTrnsfrActionAndUndo()
    {
        pSh->EndUndo( nUndoId );
        pSh->EndAllAction();
    }
};


// -----------------------------------------------------------------------

SwTransferable::SwTransferable( SwWrtShell& rSh )
    : pWrtShell( &rSh ),
    eBufferType( TRNSFR_NONE ),
    pClpDocFac( 0 ),
    pClpGraphic( 0 ),
    pClpBitmap( 0 ),
    pOrigGrf( 0 ),
    pBkmk( 0 ),
    pImageMap( 0 ),
    pTargetURL( 0 )
{
    SwDocShell* pDShell = rSh.GetDoc()->GetDocShell();
    if( pDShell )
    {
        pDShell->FillTransferableObjectDescriptor( aObjDesc );
        if( pDShell->GetMedium() )
        {
            const INetURLObject& rURLObj = pDShell->GetMedium()->GetURLObject();
            aObjDesc.maDisplayName =
                    URIHelper::removePassword( rURLObj.GetMainURL(),
                                        INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_UNAMBIGUOUS );
        }
    }
}


// -----------------------------------------------------------------------
#if 0
// used this anybody or is it used for the CreateTransferableSnapshot?
SwTransferable::SwTransferable( SwDoc& rDoc )
    : pWrtShell( 0 ),
    eBufferType( TRNSFR_NONE ),
    pClpDocFac( 0 ),
    pClpGraphic( 0 ),
    pClpBitmap( 0 ),
    pOrigGrf( 0 ),
    pBkmk( 0 ),
    pImageMap( 0 ),
    pTargetURL( 0 )
{
    // interface for CreateSnapShot
    BOOL bIsModified = rDoc.IsModified();

    //#52960#/#66044#
    SfxObjectShell* pDShell = rDoc.GetDocShell();
    if( pDShell && pDShell->GetMedium() )
    {
        const INetURLObject& rURLObj = pDShell->GetMedium()->GetURLObject();
        aObjDesc.maDisplayName =
                    URIHelper::removePassword( rURLObj.GetMainURL(),
                                        INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_UNAMBIGUOUS );
    }

    SvStorageRef aStg( new SvStorage( aEmptyStr ));
    rDoc.GetDocShell()->DoSaveAs( aStg );
    rDoc.GetDocShell()->DoSaveCompleted();

    if( bIsModified )
        rDoc.SetModified();

    SwDocShell* pNewDocSh = new SwDocShell( SFX_CREATE_MODE_INTERNAL );
    aDocShellRef = pNewDocSh;
    pNewDocSh->DoLoad( aStg );

    SwDoc* pDoc = pNewDocSh->GetDoc();

    pClpDocFac = new SwDocFac( pDoc );

    AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
    AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
    AddFormat( FORMAT_GDIMETAFILE );
    AddFormat( FORMAT_RTF );
    AddFormat( FORMAT_STRING );

    pNewDocSh->SetVisArea( rDoc.GetDocShell()->GetVisArea( ASPECT_CONTENT ) );

    //ObjectDescriptor wurde bereits aus der alten DocShell gefuellt.
    //Jetzt noch anpassen. Dadurch kann im GetData die erste Anfrage
    //auch noch mit delayed rendering beantwortet werden.
    aObjDesc.mbCanLink = FALSE;
}
#endif
// -----------------------------------------------------------------------

SwTransferable::~SwTransferable()
{
#ifdef DDE_AVAILABLE
    // der DDELink braucht noch die WrtShell!
    if( refDdeLink.Is() )
    {
        ((SwTrnsfrDdeLink*)&refDdeLink)->Disconnect();
        refDdeLink.Clear();
    }
#endif

    pWrtShell = 0;

    //JP 22.04.95: erst schliessen, dann kann die Ref. auch gecleared werden,
    //              so das die DocShell auch tatsaechlich geloescht wird!
    if( aDocShellRef.Is() )
    {
        SvEmbeddedObject * pObj = aDocShellRef;
        SwDocShell* pDocSh = (SwDocShell*)pObj;
        pDocSh->DoClose();
    }

    aDocShellRef.Clear();

    SwModule* pMod = SW_MOD();
    if ( pMod->pClipboard == this )
        pMod->pClipboard = 0;
    if ( pMod->pDragDrop == this )
        pMod->pDragDrop = 0;

    delete pClpGraphic;
    delete pClpBitmap;
    delete pClpDocFac;
    delete pImageMap;
    delete pTargetURL;
    delete pBkmk;

    eBufferType = TRNSFR_NONE;
}

// -----------------------------------------------------------------------

void SwTransferable::ObjectReleased()
{
    SwModule *pMod = SW_MOD();
    if( this == pMod->pClipboard )
        pMod->pClipboard = 0;
    if( this == pMod->pDragDrop )
        pMod->pDragDrop = 0;
}

// -----------------------------------------------------------------------

void SwTransferable::AddSupportedFormats()
{
    // we have add all formats!
}

// -----------------------------------------------------------------------

void SwTransferable::InitOle( SvEmbeddedObjectRef rRef, SwDoc& rDoc )
{
    //OleVisArea einstellen. Linke obere Ecke der Seite und Groesse
    //der RealSize in Twips.
    const Size aSz( OLESIZE );
    SwRect aVis( Point( DOCUMENTBORDER, DOCUMENTBORDER ), aSz );
    rRef->SetVisArea( aVis.SVRect() );
    rDoc.SetBrowseMode( TRUE );
}

// -----------------------------------------------------------------------

SvEmbeddedObject* SwTransferable::FindOLEObj() const
{
    SvEmbeddedObject* pRet = 0;
    if( pClpDocFac )
    {
        SwClientIter aIter( *(SwModify*)pClpDocFac->GetDoc()->
                            GetDfltGrfFmtColl() );
        for( SwCntntNode* pNd = (SwCntntNode*)aIter.First( TYPE( SwCntntNode ) );
                pNd; pNd = (SwCntntNode*)aIter.Next() )
            if( ND_OLENODE == pNd->GetNodeType() )
            {
                pRet = &((SwOLENode*)pNd)->GetOLEObj().GetOleRef();
                break;
            }
    }
    return pRet;
}

// -----------------------------------------------------------------------

void SwTransferable::RemoveDDELinkFormat()
{
    RemoveFormat( SOT_FORMATSTR_ID_LINK );
    CopyToClipboard();
}

// -----------------------------------------------------------------------

sal_Bool SwTransferable::GetData( const DATA_FLAVOR& rFlavor )
{
    sal_uInt32  nFormat = SotExchange::GetFormat( rFlavor );

    if( !HasFormat( nFormat ) )
        return sal_False;

    sal_Bool    bOK = sal_False;
    if( TRNSFR_OLE == eBufferType )
    {
        // aus dem ClipDoc das OLE-Object besorgen und von dem die Daten
        // besorgen.
        SvEmbeddedObject* pObj = FindOLEObj();
        if( pObj )
        {
            TransferableDataHelper aD( pObj->CreateTransferableSnapshot() );
            ::com::sun::star::uno::Any aAny( aD.GetAny( rFlavor ));
            if( aAny.hasValue() )
                bOK = SetAny( aAny, rFlavor );
        }
    }
    else
    {
        switch( nFormat )
        {
#ifdef DDE_AVAILABLE
        case SOT_FORMATSTR_ID_LINK:
            if( refDdeLink.Is() )
                bOK = SetObject( &refDdeLink,
                                    SWTRANSFER_OBJECTTYPE_DDE, rFlavor );
            break;
#endif
        case SOT_FORMATSTR_ID_OBJECTDESCRIPTOR:
        case SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR:
            bOK = SetTransferableObjectDescriptor( aObjDesc, rFlavor );
            break;

        case SOT_FORMATSTR_ID_DRAWING:
            {
                SwDoc *pDoc = pClpDocFac->GetDoc();
                ASSERT( pDoc, "Document not found" );
                bOK = SetObject( pDoc->GetDrawModel(),
                                SWTRANSFER_OBJECTTYPE_DRAWMODEL, rFlavor );
            }
            break;

        case SOT_FORMAT_STRING:
            bOK = SetObject( pClpDocFac->GetDoc(),
                                SWTRANSFER_OBJECTTYPE_STRING, rFlavor );
            break;

        case SOT_FORMAT_RTF:
            bOK = SetObject( pClpDocFac->GetDoc(),
                                SWTRANSFER_OBJECTTYPE_RTF, rFlavor );
            break;

        case SOT_FORMATSTR_ID_HTML:
            bOK = SetObject( pClpDocFac->GetDoc(),
                                SWTRANSFER_OBJECTTYPE_HTML, rFlavor );
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
            if( eBufferType & TRNSFR_GRAPHIC )
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

        default:
            if( !aDocShellRef.Is() )
            {
                SwDoc *pDoc = pClpDocFac->GetDoc();
                SwDocShell* pNewDocSh = new SwDocShell( pDoc,
                                         SFX_CREATE_MODE_EMBEDDED );
                aDocShellRef = pNewDocSh;
                aDocShellRef->DoInitNew( NULL );
                SwTransferable::InitOle( aDocShellRef, *pDoc );
            }
            bOK = SetObject( &aDocShellRef, SWTRANSFER_OBJECTTYPE_SWOLE,
                            rFlavor );
        }
    }
    return bOK;
}

// -----------------------------------------------------------------------

sal_Bool SwTransferable::WriteObject( SotStorageStreamRef& xStream,
                                    void* pObject, sal_uInt32 nObjectType,
                                    const DATA_FLAVOR& rFlavor )
{
    sal_Bool bRet = sal_False;
    WriterRef xWrt;

    switch( nObjectType )
    {
    case SWTRANSFER_OBJECTTYPE_DRAWMODEL:
        {
            //JP 28.02.2001: dont change the sequence of commands - Bug 8
            SdrModel *pModel = (SdrModel*)pObject;
            pModel->SetStreamingSdrModel( TRUE );
            xStream->SetBufferSize( 16348 );
            xStream->SetVersion( SOFFICE_FILEFORMAT_50 );
            pModel->GetItemPool().SetFileFormatVersion( SOFFICE_FILEFORMAT_50 );
            pModel->PreSave();
            pModel->GetItemPool().GetSecondaryPool()->Store( *xStream );
            *xStream << *pModel;
            xStream->Commit();
            pModel->SetStreamingSdrModel( FALSE );
            pModel->PostSave();

            bRet = ERRCODE_NONE == xStream->GetError();
        }
        break;

    case SWTRANSFER_OBJECTTYPE_SWOLE:
        {
            SvEmbeddedObject* pEmbObj = (SvEmbeddedObject*) pObject;
            SvStorageRef xWorkStore( new SvStorage( TRUE, *xStream ) );

            xStream->SetBufferSize( 0xff00 );

            // write document storage
            pEmbObj->SetupStorage( xWorkStore );
            bRet = pEmbObj->DoSaveAs( xWorkStore );
            pEmbObj->DoSaveCompleted();
            xWorkStore->Commit();
            xStream->Commit();

            bRet = ERRCODE_NONE == xStream->GetError();
        }
        break;


    case SWTRANSFER_OBJECTTYPE_DDE:
#ifdef DDE_AVAILABLE
        {
            xStream->SetBufferSize( 1024 );
            SwTrnsfrDdeLink* pDdeLnk = (SwTrnsfrDdeLink*)pObject;
            if( pDdeLnk->WriteData( *xStream ) )
            {
                xStream->Commit();
                bRet = ERRCODE_NONE == xStream->GetError();
            }
            else
            {
                // then we need the link anymore
                ::com::sun::star::uno::Any aValue;
                pDdeLnk->DataChanged( aEmptyStr, aValue );
            }
        }
#endif
        break;

    case SWTRANSFER_OBJECTTYPE_HTML:
        GetHTMLWriter( aEmptyStr, xWrt );
        break;

    case SWTRANSFER_OBJECTTYPE_RTF:
        GetRTFWriter( aEmptyStr, xWrt );
        break;

    case SWTRANSFER_OBJECTTYPE_STRING:
        GetASCWriter( aEmptyStr, xWrt );
        break;
    }

    if( xWrt.Is() )
    {
        //JP 12.04.96: fuer die absoluten/relative Namen -> absolut lassen!
        SwDoc* pDoc = (SwDoc*)pObject;
        String sSave( INetURLObject::GetBaseURL() );
        INetURLObject::SetBaseURL( aEmptyStr );

        xWrt->bWriteClipboardDoc = TRUE;
        xWrt->bWriteOnlyFirstTable = 0 != (TRNSFR_TABELLE & eBufferType);
        xWrt->SetShowProgress( FALSE );
        SwWriter aWrt( *xStream, *pDoc );
        if( !IsError( aWrt.Write( xWrt )) )
        {
            *xStream << '\0';               // terminate with a zero
            xStream->Commit();
            bRet = sal_True;
        }
        INetURLObject::SetBaseURL( sSave );
    }

    return bRet;
}

// -----------------------------------------------------------------------

int SwTransferable::Cut()
{
    int nRet = Copy( TRUE );
    if( nRet )
        DeleteSelection();
    return nRet;
}

// -----------------------------------------------------------------------

void SwTransferable::DeleteSelection()
{
    // Selektionsart vor Action-Klammerung erfragen
    const int nSelection = pWrtShell->GetSelectionType();
    pWrtShell->StartUndo( UNDO_DELETE );
    if( ( SwWrtShell::SEL_TXT | SwWrtShell::SEL_TBL ) & nSelection )
        pWrtShell->IntelligentCut( nSelection );
    pWrtShell->DelRight();
    pWrtShell->EndUndo( UNDO_DELETE );
}

// -----------------------------------------------------------------------

int SwTransferable::Copy( BOOL bIsCut )
{
    int nRet = 1;

    String sGrfNm;
    const int nSelection = pWrtShell->GetSelectionType();
    if( nSelection == SwWrtShell::SEL_GRF )
    {
        pClpGraphic = new Graphic;
        if( !pWrtShell->GetDrawObjGraphic( FORMAT_GDIMETAFILE, *pClpGraphic ))
            pOrigGrf = pClpGraphic;
        pClpBitmap = new Graphic;
        if( !pWrtShell->GetDrawObjGraphic( FORMAT_BITMAP, *pClpBitmap ))
            pOrigGrf = pClpBitmap;

        pClpDocFac = new SwDocFac;
        pWrtShell->Copy( pClpDocFac->GetDoc() );

        AddFormat( SOT_FORMATSTR_ID_SVXB );
        const Graphic &rGrf = pWrtShell->GetGraphic();
        if( rGrf.IsSupportedGraphic() )
        {
            AddFormat( FORMAT_GDIMETAFILE );
            AddFormat( FORMAT_BITMAP );
        }
        eBufferType = TRNSFR_GRAPHIC;
        pWrtShell->GetGrfNms( &sGrfNm, 0 );
    }
    else if ( nSelection == SwWrtShell::SEL_OLE )
    {
        pClpDocFac = new SwDocFac;
        SwDoc *pDoc = pClpDocFac->GetDoc();
        aDocShellRef = new SwDocShell( pDoc, SFX_CREATE_MODE_EMBEDDED);
        aDocShellRef->DoInitNew( NULL );
        pWrtShell->Copy( pDoc );

        AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
        AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
        AddFormat( FORMAT_GDIMETAFILE );
        eBufferType = TRNSFR_OLE;
    }
    //Gibt es ueberhaupt etwas zum bereitstellen?
    else if ( pWrtShell->IsSelection() || pWrtShell->IsFrmSelected() ||
              pWrtShell->IsObjSelected() )
    {
        SwWait *pWait = 0;
        if( pWrtShell->ShouldWait() )
            pWait = new SwWait( *pWrtShell->GetView().GetDocShell(), TRUE );

        pClpDocFac = new SwDocFac;

        // zusaetzlichen Cursor erzeugen, damit eine Gleichbehandlung
        // von Tastatur- und Mausselektion moeglich ist.
        // Im AddMode wird bei Tastaturselektion der neue Cursor erst
        // beim Bewegen des Cursors nach Selektionsende erzeugt.
        if( pWrtShell->IsAddMode() && pWrtShell->SwCrsrShell::HasSelection() )
            pWrtShell->CreateCrsr();
        SwDoc* pTmpDoc = pClpDocFac->GetDoc();
        pTmpDoc->SetRefForDocShell( (SvEmbeddedObjectRef*)&(long&)aDocShellRef );
        pTmpDoc->LockExpFlds();     // nie die Felder updaten - Text so belassen
        pWrtShell->Copy( pTmpDoc );

#ifdef DDE_AVAILABLE
        {
            // remove all DDE-Bookmarks, they are invalid inside the clipdoc!
            const SwBookmarks& rBkmk = pTmpDoc->GetBookmarks();
            for( USHORT n = rBkmk.Count(); n; )
                if( DDE_BOOKMARK == rBkmk[ --n ]->GetType() )
                    pTmpDoc->DelBookmark( n );
        }
#endif

        // es wurde in der CORE eine neu angelegt (OLE-Objekte kopiert!)
        if( aDocShellRef.Is() )
            SwTransferable::InitOle( aDocShellRef, *pTmpDoc );
        pTmpDoc->SetRefForDocShell( 0 );

        if( pWrtShell->IsObjSelected() )
            eBufferType = TRNSFR_DRAWING;
        else
        {
            eBufferType = TRNSFR_DOCUMENT;
            if (pWrtShell->IntelligentCut(nSelection, FALSE) != SwWrtShell::NO_WORD)
                eBufferType = (TransferBufferType)(TRNSFR_DOCUMENT_WORD | eBufferType);
        }

        int bDDELink = pWrtShell->IsSelection();
        if( nSelection & SwWrtShell::SEL_TBL_CELLS )
        {
            eBufferType = (TransferBufferType)(TRNSFR_TABELLE | eBufferType);
            bDDELink = pWrtShell->HasWholeTabSelection();
        }

        //Wenn's einer braucht OLE'n wir ihm was.
        AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
        AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );

        //RTF vor das Metafile von OLE stellen, weil mit weniger verlusten
        //behaftet.
        if( !pWrtShell->IsObjSelected() )
        {
            AddFormat( FORMAT_RTF );
            AddFormat( SOT_FORMATSTR_ID_HTML );
        }
        if( pWrtShell->IsSelection() )
            AddFormat( FORMAT_STRING );

        if( nSelection & ( SwWrtShell::SEL_DRW | SwWrtShell::SEL_DRW_FORM ))
        {
            AddFormat( SOT_FORMATSTR_ID_DRAWING );
            AddFormat( FORMAT_GDIMETAFILE );
            AddFormat( FORMAT_BITMAP );
            eBufferType = (TransferBufferType)( TRNSFR_GRAPHIC | eBufferType );

            pClpGraphic = new Graphic;
            if( !pWrtShell->GetDrawObjGraphic( FORMAT_GDIMETAFILE, *pClpGraphic ))
                pOrigGrf = pClpGraphic;
            pClpBitmap = new Graphic;
            if( !pWrtShell->GetDrawObjGraphic( FORMAT_BITMAP, *pClpBitmap ))
                pOrigGrf = pClpBitmap;

            // ist es ein URL-Button ?
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

#ifdef DDE_AVAILABLE
        // beim Cut hat DDE-Link keinen Sinn!!
        if( !bIsCut && bDDELink )
        {
            AddFormat( SOT_FORMATSTR_ID_LINK );
            refDdeLink = new SwTrnsfrDdeLink( *this, *pWrtShell );
        }
#endif
        //ObjectDescriptor wurde bereits aus der alten DocShell gefuellt.
        //Jetzt noch anpassen. Dadurch kann im GetData die erste Anfrage
        //auch noch mit delayed rendering beantwortet werden.
        aObjDesc.mbCanLink = FALSE;
        Size aSz( OLESIZE );
        aObjDesc.maSize = OutputDevice::LogicToLogic( aSz, MAP_TWIP, MAP_100TH_MM );

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

    SW_MOD()->pClipboard = this;
    CopyToClipboard();

    return nRet;
}

// -----------------------------------------------------------------------

int SwTransferable::CalculateAndCopy()
{
    SwWait aWait( *pWrtShell->GetView().GetDocShell(), TRUE );

    String aStr( pWrtShell->Calculate() );

    pClpDocFac = new SwDocFac;
    pWrtShell->Copy( pClpDocFac->GetDoc(), &aStr);
    eBufferType = TRNSFR_DOCUMENT;
    AddFormat( FORMAT_STRING );

    SW_MOD()->pClipboard = this;
    CopyToClipboard();

    return 1;
}

// -----------------------------------------------------------------------

int SwTransferable::CopyGlossary( SwTextBlocks& rGlossary,
                                    const String& rStr )
{
    SwWait aWait( *pWrtShell->GetView().GetDocShell(), TRUE );

    pClpDocFac = new SwDocFac;
    SwDoc* pCDoc = pClpDocFac->GetDoc();

    SwNodes& rNds = pCDoc->GetNodes();
    SwNodeIndex aNodeIdx( *rNds.GetEndOfContent().StartOfSectionNode() );
    SwCntntNode* pCNd = rNds.GoNext( &aNodeIdx ); // gehe zum 1. ContentNode
    SwPaM aPam( *pCNd );

    pCDoc->SetRefForDocShell( (SvEmbeddedObjectRef*)&(long&)aDocShellRef );
    pCDoc->LockExpFlds();   // nie die Felder updaten - Text so belassen

    pCDoc->InsertGlossary( rGlossary, rStr, aPam, 0 );

    // es wurde in der CORE eine neu angelegt (OLE-Objekte kopiert!)
    if( aDocShellRef.Is() )
        SwTransferable::InitOle( aDocShellRef, *pCDoc );
    pCDoc->SetRefForDocShell( 0 );

    eBufferType = TRNSFR_DOCUMENT;

    //Wenn's einer braucht OLE'n wir ihm was.
    AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
    AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
    AddFormat( FORMAT_RTF );
    AddFormat( SOT_FORMATSTR_ID_HTML );
    AddFormat( FORMAT_STRING );

    //ObjectDescriptor wurde bereits aus der alten DocShell gefuellt.
    //Jetzt noch anpassen. Dadurch kann im GetData die erste Anfrage
    //auch noch mit delayed rendering beantwortet werden.
    aObjDesc.mbCanLink = FALSE;
    Size aSz( OLESIZE );
    aObjDesc.maSize = OutputDevice::LogicToLogic( aSz, MAP_TWIP, MAP_100TH_MM );

    SW_MOD()->pClipboard = this;
    CopyToClipboard();

    return 1;
}

// -----------------------------------------------------------------------

BOOL SwTransferable::IsPaste( const SwWrtShell& rSh,
                              const TransferableDataHelper& rData )
{
    ULONG nFormat;
    USHORT nEventAction,
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
                                nFormat, nEventAction, 0,
                                &rData.GetTransferable() );

    return EXCHG_INOUT_ACTION_NONE != nAction;
}

// -----------------------------------------------------------------------

int SwTransferable::Paste( SwWrtShell& rSh, TransferableDataHelper& rData )
{
    USHORT nEventAction, nAction,
           nDestination = SwTransferable::GetSotDestination( rSh );
    ULONG nFormat;

    if( SW_MOD()->pClipboard )
        nAction = EXCHG_OUT_ACTION_INSERT_PRIVATE;
    else
    {
        USHORT nSourceOptions =
                    (( EXCHG_DEST_DOC_TEXTFRAME == nDestination ||
                       EXCHG_DEST_SWDOC_FREE_AREA == nDestination ||
                       EXCHG_DEST_DOC_TEXTFRAME_WEB == nDestination ||
                       EXCHG_DEST_SWDOC_FREE_AREA_WEB == nDestination )
                                    ? EXCHG_IN_ACTION_COPY
                                    : EXCHG_IN_ACTION_MOVE);
        nAction = SotExchange::GetExchangeAction(
                                    rData.GetDataFlavorExVector(),
                                    nDestination,
                                    nSourceOptions,             /* ?? */
                                    EXCHG_IN_ACTION_DEFAULT,    /* ?? */
                                    nFormat, nEventAction, 0,
                                    &rData.GetTransferable() );
    }

    return EXCHG_INOUT_ACTION_NONE != nAction &&
            SwTransferable::PasteData( rData, rSh, nAction, nFormat,
                                        nDestination, FALSE );
}

// -----------------------------------------------------------------------

int SwTransferable::PasteData( TransferableDataHelper& rData,
                            SwWrtShell& rSh, USHORT nAction, ULONG nFormat,
                            USHORT nDestination, BOOL bIsPasteFmt,
                            const Point* pPt, sal_Int8 nDropAction )
{
    SwWait aWait( *rSh.GetView().GetDocShell(), FALSE );
    SwTrnsfrActionAndUndo* pAction = 0;
    SwModule* pMod = SW_MOD();

    int nRet = 0;

    if( pPt )
    {
        // external Drop
        if( !pMod->pDragDrop )
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
                // Rahmen/Objecte selektieren
                SwTransferable::SetSelInShell( rSh, TRUE, pPt );
                break;

            // case EXCHG_DEST_DOC_TEXTFRAME:
            // case EXCHG_DEST_SWDOC_FREE_AREA:
            // case EXCHG_DEST_DOC_URLFIELD:
            default:
                SwTransferable::SetSelInShell( rSh, FALSE, pPt );
                break;
            }
        }
    }
    else if( ( !pMod->pClipboard || bIsPasteFmt ) &&
            !rSh.IsTableMode() && rSh.HasSelection() )
    {
        // dann die Selektionen loeschen

        //Selektierten Inhalt loeschen,
        // - nicht bei Tabellen-Selektion
        // - nicht bei ReRead einer Grafik/DDEDaten
        // - nicht bei D&D, fuer die richtige Selektion wurde im
        //      Drop-Handler gesorgt
        BOOL bDelSel = FALSE;
        switch( nDestination )
        {
        case EXCHG_DEST_DOC_TEXTFRAME:
        case EXCHG_DEST_SWDOC_FREE_AREA:
        case EXCHG_DEST_DOC_TEXTFRAME_WEB:
        case EXCHG_DEST_SWDOC_FREE_AREA_WEB:
            bDelSel = TRUE;
            break;
        }

        if( bDelSel )
            pAction = new SwTrnsfrActionAndUndo( &rSh, UNDO_INSERT, TRUE );
    }

    if( pPt && pMod->pDragDrop )
    {
        // dann internes Drag & Drop
        nRet = pMod->pDragDrop->PrivateDrop( rSh, *pPt,
                                        DND_ACTION_MOVE == nDropAction );
    }
    else if( !pPt && pMod->pClipboard &&
            EXCHG_OUT_ACTION_INSERT_PRIVATE == nAction )
    {
        // dann internes Pasten
        nRet = pMod->pClipboard->PrivatePaste( rSh );
    }
    else if( EXCHG_INOUT_ACTION_NONE != nAction )
    {
        if( !pAction )
            pAction = new SwTrnsfrActionAndUndo( &rSh, UNDO_INSERT );

        // im Drag&Drop duerfen keine MessageBoxen angezeigt werden
        BOOL bMsg = 0 == pPt;
        BYTE nActionFlags = ( nAction >> 8 ) & 0xFF;

        USHORT nClearedAction = ( nAction & EXCHG_ACTION_MASK );
        // Selektionen loeschen

        switch( nClearedAction )
        {
        case EXCHG_OUT_ACTION_INSERT_PRIVATE:
ASSERT( FALSE, "EXCHG_OUT_ACTION_INSERT_PRIVATE: was soll hier passieren?" );
            break;

        case EXCHG_OUT_ACTION_MOVE_PRIVATE:
ASSERT( FALSE, "EXCHG_OUT_ACTION_MOVE_PRIVATE: was soll hier passieren?" );
            break;


        case EXCHG_IN_ACTION_MOVE:
        case EXCHG_IN_ACTION_COPY:
        case EXCHG_IN_ACTION_LINK:
        case EXCHG_OUT_ACTION_INSERT_HTML:
        case EXCHG_OUT_ACTION_INSERT_STRING:
        case EXCHG_OUT_ACTION_INSERT_IMAGEMAP:
        case EXCHG_OUT_ACTION_REPLACE_IMAGEMAP:

            // dann muss ueber das Format gegangen werden
            switch( nFormat )
            {
            case SOT_FORMATSTR_ID_DRAWING:
                nRet = SwTransferable::_PasteSdrFormat( rData, rSh,
                                                SW_PASTESDR_INSERT, pPt,
                                                nActionFlags );
                break;

            case SOT_FORMATSTR_ID_HTML:
            case SOT_FORMATSTR_ID_HTML_SIMPLE:
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
                // dann nur als Grafiken einfuegen
                nRet = SwTransferable::_PasteFileList( rData, rSh, pPt, bMsg );
                break;

            case SOT_FORMATSTR_ID_INET_IMAGE:
            case SOT_FORMATSTR_ID_NETSCAPE_IMAGE:
                nRet = SwTransferable::_PasteTargetURL( rData, rSh,
                                                        SW_PASTESDR_INSERT,
                                                        pPt, TRUE );
                break;

            default:
                ASSERT( FALSE, "unbekanntes Format" );
            }
            break;

        case EXCHG_OUT_ACTION_INSERT_FILE:
            nRet = SwTransferable::_PasteFileName( rData, rSh, nFormat,
                                        SW_PASTESDR_INSERT, pPt,
                                        nActionFlags, bMsg );
            break;

        case EXCHG_OUT_ACTION_INSERT_OLE:
            nRet = SwTransferable::_PasteOLE( rData, rSh, nFormat,
                                                nActionFlags,bMsg );
            break;

        case EXCHG_OUT_ACTION_INSERT_DDE:
#ifdef DDE_AVAILABLE
            {
                FASTBOOL bReRead = 0 != CNT_HasGrf( rSh.GetCntType() );
                nRet = SwTransferable::_PasteDDE( rData, rSh, bReRead, bMsg );
            }
#endif
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
                ASSERT( FALSE, "unbekanntes Format" );
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
            ASSERT( FALSE, "unbekannte Action" );
        }
    }

    if( rSh.IsFrmSelected() )
        rSh.EnterSelFrmMode();

    if( pAction )
        delete pAction;

    return nRet;
}

// -----------------------------------------------------------------------

USHORT SwTransferable::GetSotDestination( const SwWrtShell& rSh,
                                            const Point* pPt )
{
    USHORT nRet = EXCHG_INOUT_ACTION_NONE;

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
            BOOL bIMap, bLink;
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

// was mmchen wir bei Mehrfachselektion???
//  case OBJCNT_DONTCARE:
    default:
        {
/*
JP 13.07.98: Bug 52637: es wird ein URL-Feld erkannt also werden nur die
                        Inhalte zugelassen. Das ist aber bestimmt nicht das
                        gewollte.
            SwContentAtPos aCntntAtPos( SwContentAtPos::SW_INETATTR );
            SfxItemSet aSet( (SfxItemPool&)rSh.GetAttrPool(),
                            RES_TXTATR_INETFMT, RES_TXTATR_INETFMT );
            if( pPt ? ((SwWrtShell&)rSh).GetContentAtPos( *pPt, aCntntAtPos, FALSE )
                     : (rSh.GetAttr( aSet ) && aSet.Count()) )
                nRet = EXCHG_DEST_DOC_URLFIELD;
            else
*/
            if( rSh.GetView().GetDocShell()->ISA(SwWebDocShell) )
                nRet = EXCHG_DEST_SWDOC_FREE_AREA_WEB;
            else
                nRet = EXCHG_DEST_SWDOC_FREE_AREA;
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------

int SwTransferable::_PasteFileContent( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, ULONG nFmt, BOOL bMsg )
{
    USHORT nResId = MSG_CLPBRD_FORMAT_ERROR;
    int nRet = 0;

    MSE40HTMLClipFormatObj aMSE40ClpObj;
    String sTmpBaseURL( INetURLObject::GetBaseURL() );

    SotStorageStreamRef xStrm;
    SvStream* pStream = 0;
    SwRead pRead = 0;
    switch( nFmt )
    {
    case SOT_FORMAT_STRING:
        {
            String sData;
            pRead = ReadAscii;
            if( rData.GetString( nFmt, sData ) )
            {
                xStrm = new SotStorageStream( aEmptyStr );
#ifdef __BIGENDIAN
                xStrm->SetNumberFormatInt( NUMBERFORMAT_INT_BIGENDIAN );
#else
                xStrm->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
#endif
                xStrm->Write( (void*)sData.GetBuffer(),
                              ( sData.Len() + 1 ) * sizeof( sal_Unicode ));
                xStrm->Seek( 0 );
                pStream = &xStrm;

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
            if( SOT_FORMATSTR_ID_HTML_SIMPLE == nFmt )
            {
                pStream = aMSE40ClpObj.IsValid( *xStrm );
                pRead = ReadHTML;
                pRead->SetReadUTF8( TRUE );
                INetURLObject::SetBaseURL( aMSE40ClpObj.GetBaseURL() );
            }
            else
            {
                pStream = &xStrm;
                if( SOT_FORMAT_RTF == nFmt )
                    pRead = ReadRtf;
                else if( !pRead )
                    pRead = ReadHTML;
            }
        }
        break;
    }

    if( pStream && pRead )
    {
        Link aOldLink( rSh.GetChgLnk() );
        rSh.SetChgLnk( Link() );

        SwReader aReader( *pStream, aEmptyStr, *rSh.GetCrsr() );
        if( IsError( aReader.Read( *pRead )) )
            nResId = ERR_CLPBRD_READ;
        else
            nResId = 0, nRet = 1;

        INetURLObject::SetBaseURL( sTmpBaseURL );

        rSh.SetChgLnk( aOldLink );
        if( nRet )
            rSh.CallChgLnk();
    }
    else
        nResId = MSG_CLPBRD_FORMAT_ERROR;

    if( bMsg && nResId )
    {
        InfoBox( 0, SW_RES( nResId )).Execute();
    }
    return nRet;
}

// -----------------------------------------------------------------------

int SwTransferable::_PasteOLE( TransferableDataHelper& rData, SwWrtShell& rSh,
                                ULONG nFmt, BYTE nActionFlags, BOOL bMsg )
{
    int nRet = 0;
    TransferableObjectDescriptor aObjDesc;
    SotStorageStreamRef xStrm;
    SvStorageRef xStore;
    Reader* pRead = 0;
    SotFormatStringId nId;
    if( rData.HasFormat( SOT_FORMATSTR_ID_EMBEDDED_OBJ ) )
        nId = SOT_FORMATSTR_ID_EMBEDDED_OBJ;
    else if( rData.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE ) &&
             rData.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ))
        nId = SOT_FORMATSTR_ID_EMBED_SOURCE;
    else
        nId = 0;

    if( nId && rData.GetSotStorageStream( nId, xStrm ) && xStrm.Is() )
    {
        xStore = new SvStorage( *xStrm );
        switch( xStore->GetFormat() )
        {
        case SOT_FORMATSTR_ID_STARWRITER_50:
        case SOT_FORMATSTR_ID_STARWRITER_40:
        case SOT_FORMATSTR_ID_STARWRITER_30:
        case SOT_FORMATSTR_ID_STARWRITERWEB_50:
        case SOT_FORMATSTR_ID_STARWRITERWEB_40:
        case SOT_FORMATSTR_ID_STARWRITERGLOB_50:
        case SOT_FORMATSTR_ID_STARWRITERGLOB_40:
            pRead = ReadSw3;
            break;

        case SOT_FORMATSTR_ID_STARWRITER_60:
        case SOT_FORMATSTR_ID_STARWRITERWEB_60:
        case SOT_FORMATSTR_ID_STARWRITERGLOB_60:
            pRead = ReadXML;
            break;
        }
    }

    if( pRead )
    {
        SwReader aReader( *xStore, aEmptyStr, *rSh.GetCrsr() );
        if( !IsError( aReader.Read( *pRead )) )
            nRet = 1;
        else if( bMsg )
            InfoBox( 0, SW_RES(ERR_CLPBRD_READ) ).Execute();
    }
    else if( rData.GetTransferableObjectDescriptor(
                SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aObjDesc ) &&
            ( nFmt == nId ? xStore.Is()
                          : ( xStore.Clear(),
                              rData.GetSotStorageStream( nFmt, xStrm )) ) )
    {
        if( !xStore.Is() )
            xStore = new SvStorage( *xStrm );
        SvInPlaceObjectRef xIPObj = &( (SvFactory*)SvInPlaceObject::
                            ClassFactory() )->CreateAndLoad( xStore );

        if( xIPObj.Is() )
        {
            //Size einstellen. Ist ein Hack wg. Auslieferung, die Size sollte
            //an das InsertOle uebergeben werden!!!!!!!!!!
            const Size aSize( aObjDesc.maSize );    //immer 100TH_MM
            if( aSize.Width() && aSize.Height() )
            {
                xIPObj->SetVisAreaSize( OutputDevice::LogicToLogic( aSize,
                                    MAP_100TH_MM, xIPObj->GetMapUnit() ) );
            }
            //Ende mit Hack!

            rSh.InsertOle( xIPObj );
            nRet = 1;

            if( nRet && ( nActionFlags &
                ( EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL >> 8) ))
                SwTransferable::_PasteTargetURL( rData, rSh, 0, 0, FALSE );

        }
    }
    return nRet;
}

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------

int SwTransferable::_PasteTargetURL( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, USHORT nAction,
                                    const Point* pPt, BOOL bInsertGRF )
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

            //!!! auf FileSystem abpruefen - nur dann ist es sinnvoll die
            // Grafiken zu testen !!!!
            Graphic aGrf;
            GraphicFilter *pFlt = ::GetGrfFilter();
            nRet = GRFILTER_OK == ::LoadGraphic( sURL, aEmptyStr, aGrf, pFlt );
            if( nRet )
            {
                switch( nAction )
                {
                case SW_PASTESDR_INSERT:
                    SwTransferable::SetSelInShell( rSh, FALSE, pPt );
                    rSh.Insert( sURL, aEmptyStr, aGrf );
                    break;

                case SW_PASTESDR_REPLACE:
                    if( rSh.IsObjSelected() )
                    {
                        rSh.ReplaceSdrObj( sURL, aEmptyStr, &aGrf );
                        Point aPt( pPt ? *pPt : rSh.GetCrsrDocPos() );
                        SwTransferable::SetSelInShell( rSh, TRUE, &aPt );
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
                        SwTransferable::SetSelInShell( rSh, FALSE, pPt );
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
            aURL.SetURL( aINetImg.GetTargetURL(), FALSE );
            aURL.SetTargetFrameName( aINetImg.GetTargetFrame() );
            aSet.Put( aURL );
            rSh.SetFlyFrmAttr( aSet );
        }
    }
    return nRet;
}


// -----------------------------------------------------------------------

void SwTransferable::SetSelInShell( SwWrtShell& rSh, BOOL bSelectFrm,
                                        const Point* pPt )
{
    if( bSelectFrm )
    {
        // Rahmen/Objecte selektieren
        if( pPt && !SFX_APP()->IsDispatcherLocked() )
        {
            rSh.GetView().NoRotate();
            if( rSh.SelectObj( *pPt ))
            {
                rSh.HideCrsr();
                rSh.EnterSelFrmMode( pPt );
                bFrmDrag = TRUE;
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
            bFrmDrag = FALSE;
        }
        else if( rSh.GetView().GetDrawFuncPtr() )
            rSh.GetView().GetEditWin().StopInsFrm();

        rSh.EnterStdMode();
        if( pPt )
            rSh.SwCrsrShell::SetCrsr( *pPt, TRUE );
    }
}

// -----------------------------------------------------------------------

int SwTransferable::_PasteDDE( TransferableDataHelper& rData,
                                SwWrtShell& rWrtShell, FASTBOOL bReReadGrf,
                                BOOL bMsg )
{
#ifdef DDE_AVAILABLE
    // Daten aus dem Clipboardformat
    String aApp, aTopic, aItem;

    {
        SotStorageStreamRef xStrm;
        if( !rData.GetSotStorageStream( SOT_FORMATSTR_ID_LINK, xStrm ))
        {
            ASSERT( !&rWrtShell, "DDE Data not found." );
            return 0;
        }   //sinnvollen Fehler melden!!

        xStrm->ReadCString( aApp, DDE_TXT_ENCODING );
        xStrm->ReadCString( aTopic, DDE_TXT_ENCODING );
        xStrm->ReadCString( aItem, DDE_TXT_ENCODING );
    }

    String aCmd;
    ::MakeLnkName( aCmd, &aApp, aTopic, aItem );

    // wollen wir jetzt eine Grafik einlesen ?
    ULONG nFormat;
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
            String sLnkTyp( String::CreateFromAscii(
                        RTL_CONSTASCII_STRINGPARAM( "DDE" )));
            if ( bReReadGrf )
                rWrtShell.ReRead( aCmd, sLnkTyp, &aGrf );
            else
                rWrtShell.Insert( aCmd, sLnkTyp, aGrf );
        }
        return nRet;
    }

    SwFieldType* pTyp;
    USHORT i = 1,j;
    String aName;
    BOOL bAlreadyThere = FALSE, bDoublePaste = FALSE;
    USHORT nSize = rWrtShell.GetFldTypeCount();
    const International& rInt = Application::GetAppInternational();

    do {
        aName = aApp;
        aName += String::CreateFromInt32( i );
        for( j = INIT_FLDTYPES; j < nSize; j++ )
        {
            pTyp = rWrtShell.GetFldType( j );
            if( RES_DDEFLD == pTyp->Which() )
            {
                String sTmp( ((SwDDEFieldType*)pTyp)->GetCmd() );
                if( rInt.CompareEqual( sTmp, aCmd, INTN_COMPARE_IGNORECASE ) &&
                    LINKUPDATE_ALWAYS == ((SwDDEFieldType*)pTyp)->GetType() )
                {
                    aName = pTyp->GetName();
                    bDoublePaste = TRUE;
                    break;
                }
                else if( rInt.CompareEqual( aName, pTyp->GetName(),
                                            INTN_COMPARE_IGNORECASE ) )
                    break;
            }
        }
        if( j == nSize )
            bAlreadyThere = FALSE;
        else
        {
            bAlreadyThere = TRUE;
            i++;
        }
    }
    while( bAlreadyThere && !bDoublePaste );

    if( !bDoublePaste )
    {
        SwDDEFieldType aType( aName, aCmd, LINKUPDATE_ALWAYS );
        pTyp = rWrtShell.InsertFldType( aType );
    }


    SwDDEFieldType* pDDETyp = (SwDDEFieldType*)pTyp;

    String aExpand;
    if( rData.GetString( FORMAT_STRING, aExpand ))
    {
        do {            // middle checked loop

            // Wenn die Daten von einer Tabellenkalkulation kommen
            // fuegen wir eine DDE-Tabelle ein
            if( ( rData.HasFormat( SOT_FORMATSTR_ID_SYLK ) ||
                  rData.HasFormat( SOT_FORMATSTR_ID_SYLK_BIGCAPS ) ) &&
                aExpand.Len() &&
                 ( 1 < aExpand.GetTokenCount( '\n' ) ||
                       aExpand.GetTokenCount( '\t' )) )
            {
                // wo stehen wir ueberhaupt ??
                // Wir duerfen keine Tabelle in eine Tabelle einfuegen !!
                if( rWrtShell.IsCrsrInTbl() )
                {
                    if(bMsg)
                        InfoBox(0, String(SW_RES(STR_NO_TABLE))).Execute();
                    pDDETyp = 0;
                    break;
                }

                String sTmp( aExpand );
                xub_StrLen nRows = sTmp.GetTokenCount( '\n' );
                if( nRows )
                    --nRows;
                sTmp = sTmp.GetToken( 0, '\n' );
                xub_StrLen nCols = sTmp.GetTokenCount( '\t' );

                // mindestens eine Spalte & Zeile muss vorhanden sein
                if( !nRows || !nCols )
                {
                    if( bMsg )
                        InfoBox(0, SW_RESSTR(STR_NO_TABLE)).Execute();
                    pDDETyp = 0;
                    break;
                }

                rWrtShell.InsertDDETable( pDDETyp, nRows, nCols );
            }
            else if( 1 < aExpand.GetTokenCount( '\n' ) )
            {
                // mehrere Absaetze -> eine geschuetzte Section einfuegen
                if( rWrtShell.HasSelection() )
                    rWrtShell.DelRight();

                SwSection aSect( DDE_LINK_SECTION, aName );
                aSect.SetLinkFileName( aCmd );
                aSect.SetProtect();
                rWrtShell.InsertSection( aSect );

                pDDETyp = 0;                // FeldTypen wieder entfernen
            }
            else
            {
                // Einfuegen
                SwDDEField aSwDDEField( pDDETyp );
                rWrtShell.Insert( aSwDDEField );
            }

        } while( FALSE );
    }
    else
        pDDETyp = 0;                        // FeldTypen wieder entfernen

    if( !pDDETyp && !bDoublePaste )
    {
        // FeldTyp wieder entfernen - Fehler aufgetreten!
        for( j = nSize; j >= INIT_FLDTYPES; --j )
            if( pTyp == rWrtShell.GetFldType( j ) )
            {
                rWrtShell.RemoveFldType( j );
                break;
            }
    }

    return 1;
#else
    return 0;
#endif
}

// -----------------------------------------------------------------------

int SwTransferable::_PasteSdrFormat(  TransferableDataHelper& rData,
                                    SwWrtShell& rSh, USHORT nAction,
                                    const Point* pPt, BYTE nActionFlags )
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
            SwTransferable::_PasteTargetURL( rData, rSh, 0, 0, FALSE );
    }
    return nRet;
}

// -----------------------------------------------------------------------

int SwTransferable::_PasteGrf( TransferableDataHelper& rData, SwWrtShell& rSh,
                                ULONG nFmt, USHORT nAction, const Point* pPt,
                                BYTE nActionFlags, BOOL bMsg )
{
    int nRet = 0;

    Graphic aGrf;
    INetBookmark aBkmk;
    BOOL bCheckForGrf = FALSE, bCheckForImageMap = FALSE;

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
/*              if( SW_PASTESDR_SETATTR != nAction )
            {
                INetURLObject aURL( aBkmk.GetURL() );
                bCheckForGrf = INET_PROT_FILE == aURL.GetProtocol();
                nRet = 0 != bCheckForGrf;
            }
*/
            if( SW_PASTESDR_SETATTR == nAction )
                nFmt = SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK;
            else
                bCheckForGrf = TRUE;
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
                    URIHelper::SmartRelToAbs( sTxt ), sDesc );
                bCheckForGrf = TRUE;
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
        //!!! auf FileSystem abpruefen - nur dann ist es sinnvoll die
        // Grafiken zu testen !!!!
        GraphicFilter *pFlt = ::GetGrfFilter();
        nRet = GRFILTER_OK == ::LoadGraphic( aBkmk.GetURL(), aEmptyStr,
                                            aGrf, pFlt );
        if( !nRet && SW_PASTESDR_SETATTR == nAction &&
            SOT_FORMAT_FILE == nFmt &&
            // Bug 63031 - nur bei Rahmenselektion
            rSh.IsFrmSelected() )
        {
            // dann als Hyperlink hinter die Grafik setzen
            nFmt = SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK;
            nRet = TRUE;
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
            SwTransferable::SetSelInShell( rSh, FALSE, pPt );
            rSh.Insert( sURL, aEmptyStr, aGrf );
            break;

        case SW_PASTESDR_REPLACE:
            if( rSh.IsObjSelected() )
            {
                rSh.ReplaceSdrObj( sURL, aEmptyStr, &aGrf );
                Point aPt( pPt ? *pPt : rSh.GetCrsrDocPos() );
                SwTransferable::SetSelInShell( rSh, TRUE, &aPt );
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
                    aURL.SetURL( aBkmk.GetURL(), FALSE );
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
                SwTransferable::SetSelInShell( rSh, FALSE, pPt );
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
            SwTransferable::_PasteTargetURL( rData, rSh, 0, 0, FALSE );
    }
    else if( bCheckForImageMap )
    {
        // oder sollte das File ein ImageMap-File sein?
        ImageMap aMap;
        SfxMedium aMed( INetURLObject(aBkmk.GetURL()).GetFull(),
                            STREAM_STD_READ, FALSE );
        if( !aMed.GetInStream()->GetError() && IMAP_ERR_OK ==
            aMap.Read( *aMed.GetInStream(), IMAP_FORMAT_DETECT ) &&
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

    if( !nRet )
        InfoBox( 0, SW_RES(MSG_CLPBRD_FORMAT_ERROR)).Execute();

    return nRet;
}

// -----------------------------------------------------------------------

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

        // setzen oder ersetzen ist hier die Frage
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

// -----------------------------------------------------------------------

int SwTransferable::_PasteAsHyperlink( TransferableDataHelper& rData,
                                        SwWrtShell& rSh, ULONG nFmt )
{
    int nRet = 0;
    String sFile;
    if( rData.GetString( nFmt, sFile ) && sFile.Len() )
    {
        String sDesc;
        SwTransferable::_CheckForURLOrLNKFile( rData, sFile, &sDesc );

        //#41801# ersteinmal die URL absolut machen
        INetURLObject aURL;
        aURL.SetSmartProtocol( INET_PROT_FILE );
        aURL.SetSmartURL( sFile );
        sFile = aURL.GetMainURL();

        switch( rSh.GetObjCntTypeOfSelection() )
        {
        case OBJCNT_FLY:
        case OBJCNT_GRF:
        case OBJCNT_OLE:
            {
                SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                rSh.GetFlyFrmAttr( aSet );
                SwFmtURL aURL( (SwFmtURL&)aSet.Get( RES_URL ) );
                aURL.SetURL( sFile, FALSE );
                if( !aURL.GetName().Len() )
                    aURL.SetName( sFile );
                aSet.Put( aURL );
                rSh.SetFlyFrmAttr( aSet );
            }
            break;

        default:
            {
                rSh.InsertURL( SwFmtINetFmt( sFile, aEmptyStr ),
                                sDesc.Len() ? sDesc : sFile );
            }
        }
        nRet = TRUE;
    }
    return nRet;
}

// -----------------------------------------------------------------------

int SwTransferable::_PasteFileName( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, ULONG nFmt,
                                    USHORT nAction, const Point* pPt,
                                    BYTE nActionFlags, BOOL bMsg )
{
    int nRet = SwTransferable::_PasteGrf( rData, rSh, nFmt, nAction,
                                            pPt, nActionFlags, bMsg );

        // oder doch als Dokument einfuegen!
    if( !nRet )
    {
        String sFile, sDesc;
        if( rData.GetString( nFmt, sFile ) && sFile.Len() )
        {
            BOOL bIsURLFile = SwTransferable::_CheckForURLOrLNKFile( rData, sFile, &sDesc );

            //Eigenes FileFormat? -->Einfuegen, nicht fuer StarWriter/Web
            const SfxFilter* pFlt = SW_PASTESDR_SETATTR == nAction
                    ? 0 : SwIoSystem::GetFileFilter( sFile, aEmptyStr );
            if( pFlt && !rSh.GetView().GetDocShell()->ISA(SwWebDocShell)
/*
JP 02.07.98: warum nur fuer die Formate ??
                && ( pFlt->GetUserData() == FILTER_SW5 ||
                pFlt->GetUserData() == FILTER_SW4 ||
                pFlt->GetUserData() == FILTER_SW3 ||
                pFlt->GetUserData() == FILTER_SWG )
*/
                )
            {
// und dann per PostUser Event den Bereich-Einfuegen-Dialog hochreissen
                SwSection* pSect = new SwSection( FILE_LINK_SECTION,
                                rSh.GetDoc()->GetUniqueSectionName() );
                pSect->SetLinkFileName( URIHelper::SmartRelToAbs( sFile ) );
                pSect->SetProtect( TRUE );

                Application::PostUserEvent( STATIC_LINK( &rSh, SwWrtShell,
                                            InsertRegionDialog ), pSect );
                nRet = 1;
#if 0
                if( rSh.InsertSection( aSect ) )
                {
                    if( SW_PASTESDR_SETATTR != nAction )
                    {
                        aSect.SetType( CONTENT_SECTION );
                        aSect.SetProtect( FALSE );
                        for( USHORT i = rSh.GetSectionFmtCount(); i; )
                            if( aNm == rSh.GetSectionFmt( --i ).
                                                GetSection()->GetName())
                            {
                                rSh.ChgSection( i, aSect );
                                break;
                            }
                    }
                    nRet = TRUE;
                }
#endif
            }
            else if( SW_PASTESDR_SETATTR == nAction ||
                    ( bIsURLFile && SW_PASTESDR_INSERT == nAction ))
            {
                //Fremde Files koennen wir immerhin noch als Links
                //Einfuegen.

                //#41801# ersteinmal die URL absolut machen
                INetURLObject aURL;
                aURL.SetSmartProtocol( INET_PROT_FILE );
                aURL.SetSmartURL( sFile );
                sFile = aURL.GetMainURL();

                switch( rSh.GetObjCntTypeOfSelection() )
                {
                case OBJCNT_FLY:
                case OBJCNT_GRF:
                case OBJCNT_OLE:
                    {
                        SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                        rSh.GetFlyFrmAttr( aSet );
                        SwFmtURL aURL( (SwFmtURL&)aSet.Get( RES_URL ) );
                        aURL.SetURL( sFile, FALSE );
                        if( !aURL.GetName().Len() )
                            aURL.SetName( sFile );
                        aSet.Put( aURL );
                        rSh.SetFlyFrmAttr( aSet );
                    }
                    break;

                default:
                    {
                        rSh.InsertURL( SwFmtINetFmt( sFile, aEmptyStr ),
                                        sDesc.Len() ? sDesc : sFile );
                    }
                }
                nRet = TRUE;
            }
        }
    }
    return nRet;
}

// -----------------------------------------------------------------------

int SwTransferable::_PasteDBData( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, ULONG nFmt, BOOL bLink,
                                    const Point* pDragPt, BOOL bMsg )
{
    int nRet = 0;
    String sTxt;
    if( rData.GetString( nFmt, sTxt ) && sTxt.Len() )
    {
        USHORT nWh = SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE == nFmt
                    ? 0
                    : SOT_FORMATSTR_ID_SBA_DATAEXCHANGE == nFmt
                                ? (bLink
                                    ? FN_QRY_MERGE_FIELD
                                    : FN_QRY_INSERT)
                                : (bLink
                                    ? 0
                                    : FN_QRY_INSERT_FIELD );
        if( nWh )
        {
            SwView& rView = rSh.GetView();
            //force ::SelectShell
            rView.StopShellTimer();

            SfxStringItem aDataDesc( nWh, sTxt );
            rView.GetViewFrame()->GetDispatcher()->Execute(
                                nWh, SFX_CALLMODE_SLOT, &aDataDesc, 0L);
        }
        else
        {
            SdrObject* pObj;
            rSh.MakeDrawView();
            FmFormView* pFmView = PTR_CAST( FmFormView, rSh.GetDrawView() );
            if( pFmView && 0 != (pObj = pFmView->CreateFieldControl(sTxt) ))
                rSh.SwFEShell::Insert( *pObj, 0, 0, pDragPt );
        }
        nRet = 1;
    }
    else if( bMsg )
    {
        InfoBox( 0, SW_RES(MSG_CLPBRD_FORMAT_ERROR)).Execute();
    }
    return nRet;
}

// -----------------------------------------------------------------------

int SwTransferable::_PasteFileList( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, const Point* pPt,
                                    BOOL bMsg )
{
    int nRet = 0;
    FileList aFileList;
    if( rData.GetFileList( SOT_FORMAT_FILE_LIST, aFileList ) &&
        aFileList.Count() )
    {
        // iterate over the filelist
        for( ULONG n = 0, nEnd = aFileList.Count(); n < nEnd; ++n )
        {

            DataFlavorEx aFlavorEx;
            if( SotExchange::GetFormatDataFlavor( FORMAT_FILE, aFlavorEx ) )
            {
                SwTransferable* pHlp = new SwTransferable( rSh );

                pHlp->AddFormat( aFlavorEx );
                pHlp->SetString( aFileList.GetFile( n ), aFlavorEx );
                TransferableDataHelper aData( pHlp );

                if( SwTransferable::_PasteGrf( aData, rSh, SOT_FORMAT_FILE,
                                                SW_PASTESDR_INSERT, pPt,
                                                FALSE, bMsg ))
                    nRet = 1;
            }
        }
    }
    else if( bMsg )
    {
        InfoBox( 0, SW_RES(MSG_CLPBRD_FORMAT_ERROR)).Execute();
    }
    return nRet;
}

// -----------------------------------------------------------------------

BOOL SwTransferable::_CheckForURLOrLNKFile( TransferableDataHelper& rData,
                                        String& rFileName, String* pTitle )
{
    BOOL bIsURLFile = FALSE;
    INetBookmark aBkmk;
    if( rData.GetINetBookmark( SOT_FORMATSTR_ID_SOLK, aBkmk ) )
    {
        rFileName = aBkmk.GetURL();
        if( pTitle )
            *pTitle = aBkmk.GetDescription();
        bIsURLFile = TRUE;
    }
    else
    {
        xub_StrLen nLen = rFileName.Len();
        if( 4 < nLen && '.' == rFileName.GetChar( nLen - 4 ))
        {
            String sExt( rFileName.Copy( nLen - 3 ));
            if( sExt.EqualsIgnoreCaseAscii( "url" ))
            {
ASSERT( !&rFileName, "how do we read today .URL - Files?" );
// JP 12.05.00 UNICODE - CHANGES
#if 0

                bIsURLFile = TRUE;
                Config aCfg( rFileName );
                aCfg.SetGroup( INTERNETSHORTCUT_ID_TAG );

                // Einlesung und Konvertierung des URL aus der Datei
                rFileName = aCfg.ReadKey( INTERNETSHORTCUT_URL_TAG,
                                            URLFILE_CHARSET );

                // dann teste doch mal auf ein lokales File
                INetURLObject aURL( rFileName );
                if( INET_PROT_FILE == aURL.GetProtocol() )
                    rFileName = aURL.PathToFileName();

                // Einlesung und Konvertierung des Titels aus der Datei
                if( pTitle )
                    *pTitle = aCfg.ReadKey( INTERNETSHORTCUT_TITLE_TAG,
                                            URLFILE_CHARSET  );
#endif
            }
        }
    }
    return bIsURLFile;
}

// -----------------------------------------------------------------------

/*  */

BOOL SwTransferable::IsPasteSpecial( const SwWrtShell& rWrtShell,
                                     const TransferableDataHelper& rData )
{
    return rData.GetTransferable().is() &&
        1 < rData.GetTransferable()->getTransferDataFlavors().getLength() &&
        IsPaste( rWrtShell, rData );
}

// -----------------------------------------------------------------------

int SwTransferable::PasteFormat( SwWrtShell& rSh,
                                    TransferableDataHelper& rData,
                                    ULONG nFormat )
{
    SwWait aWait( *rSh.GetView().GetDocShell(), FALSE );
    int nRet = 0;

    ULONG nPrivateFmt = FORMAT_PRIVATE;
    SwTransferable *pClipboard = SW_MOD()->pClipboard;
    if( pClipboard &&
        ((TRNSFR_DOCUMENT|TRNSFR_GRAPHIC|TRNSFR_OLE) & pClipboard->eBufferType ))
        nPrivateFmt = SOT_FORMATSTR_ID_EMBED_SOURCE;

    if( pClipboard && nPrivateFmt == nFormat )
        nRet = pClipboard->PrivatePaste( rSh );
    else if( rData.HasFormat( nFormat ) )
    {
        USHORT nEventAction,
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
                                    &rData.GetTransferable() );

        if( EXCHG_INOUT_ACTION_NONE != nAction )
            nRet = SwTransferable::PasteData( rData, rSh, nAction, nFormat,
                                                nDestination, TRUE );
    }
    return nRet;
}

// -----------------------------------------------------------------------

int SwTransferable::_TestAllowedFormat( const TransferableDataHelper& rData,
                                        ULONG nFormat, USHORT nDestination )
{
    USHORT nAction = EXCHG_INOUT_ACTION_NONE, nEventAction;
    if( rData.HasFormat( nFormat ))
        nAction = SotExchange::GetExchangeAction(
                        rData.GetDataFlavorExVector(),
                        nDestination, EXCHG_IN_ACTION_COPY,
                        EXCHG_IN_ACTION_COPY, nFormat,
                        nEventAction, nFormat,
                        &rData.GetTransferable() );
    return EXCHG_INOUT_ACTION_NONE != nAction;
}

// -----------------------------------------------------------------------

int SwTransferable::PasteSpecial( SwWrtShell& rSh,
                                    TransferableDataHelper& rData )
{
    int nRet = 0;
    SvPasteObjectDialog* pDlg = new SvPasteObjectDialog;

    DataFlavorExVector aFormats( rData.GetDataFlavorExVector() );
    TransferableObjectDescriptor aDesc;

    USHORT nDest = SwTransferable::GetSotDestination( rSh );

    // Vorhandene Formate in die ListBox eintragen
    if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_SVXB, nDest ))
        pDlg->Insert( SOT_FORMATSTR_ID_SVXB, aEmptyStr );

    SwTransferable *pClipboard = SW_MOD()->pClipboard;
    if( pClipboard )
    {
        aDesc = pClipboard->aObjDesc;
        USHORT nResId;
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
        rData.GetTransferableObjectDescriptor(
                                SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aDesc );

        if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_EMBED_SOURCE, nDest ))
            pDlg->Insert( SOT_FORMATSTR_ID_EMBED_SOURCE, aEmptyStr );
        if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_LINK_SOURCE, nDest ))
            pDlg->Insert( SOT_FORMATSTR_ID_LINK_SOURCE, aEmptyStr );
    }

#ifdef DDE_AVAILABLE

    if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_LINK, nDest ))
        pDlg->Insert( SOT_FORMATSTR_ID_LINK, SW_RES(STR_DDEFORMAT) );

#endif

    static USHORT aIds[] = {
            SOT_FORMATSTR_ID_HTML,
            SOT_FORMATSTR_ID_HTML_SIMPLE,
            FORMAT_RTF,
            FORMAT_STRING,
            SOT_FORMATSTR_ID_SONLK,
            SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK,
            SOT_FORMATSTR_ID_DRAWING,
            SOT_FORMATSTR_ID_SVXB,
            FORMAT_GDIMETAFILE,
            FORMAT_BITMAP,
            SOT_FORMATSTR_ID_SVIM,
            0 };

    for( USHORT* pIds = aIds; *pIds; ++pIds )
        if( SwTransferable::_TestAllowedFormat( rData, *pIds, nDest ))
        {
            String sStr;
            switch( *pIds )
            {
            case FORMAT_STRING: sStr = SW_RES( STR_TEXTFORMAT );    break;
            case FORMAT_RTF:    sStr = SW_RES( STR_RTFFORMAT );     break;
            }
            pDlg->Insert( *pIds, sStr );
        }

    ULONG nFormat = pDlg->Execute( &rSh.GetView().GetEditWin(), aFormats,
                                    aDesc );

    if( nFormat )
        nRet = SwTransferable::PasteFormat( rSh, rData, nFormat );

    delete pDlg;
    return nRet;
}


void SwTransferable::FillClipFmtItem( SwWrtShell& rSh,
                                const TransferableDataHelper& rData,
                                SvxClipboardFmtItem & rToFill )
{
    USHORT nDest = SwTransferable::GetSotDestination( rSh );
    // Vorhandene Formate in die ListBox eintragen
    if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_SVXB, nDest ))
        rToFill.AddClipbrdFormat( SOT_FORMATSTR_ID_SVXB );

/*
    if( pClipboard )
    {
        if( pClipboard->GetBufferType() & TRNSFR_DOCUMENT )
            nResId = STR_PRIVATETEXT;
        else if( pClipboard->GetBufferType() & TRNSFR_GRAPHIC )
            nResId = STR_PRIVATEGRAPHIC;
        else if( pClipboard->GetBufferType() == TRNSFR_OLE )
            nResId = STR_PRIVATEOLE;

        if( nResId )
        {
            nPrivateFmt = SOT_FORMATSTR_ID_EMBED_SOURCE;
            if( STR_PRIVATEOLE == nResId || STR_PRIVATEGRAPHIC == nResId )
                // fuer den dummen Dialog (privates Format anzeigen)
                rTypeLst.Insert( SvDataType( nPrivateFmt, MEDIUM_STORAGE ), 0 );

        }
    }
    else
*/
    {
        if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_EMBED_SOURCE, nDest ))
            rToFill.AddClipbrdFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
        if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_LINK_SOURCE, nDest ))
            rToFill.AddClipbrdFormat( SOT_FORMATSTR_ID_LINK_SOURCE );
    }

#ifdef DDE_AVAILABLE

    if( SwTransferable::_TestAllowedFormat( rData, SOT_FORMATSTR_ID_LINK, nDest ))
        rToFill.AddClipbrdFormat( SOT_FORMATSTR_ID_LINK, SW_RES(STR_DDEFORMAT) );

#endif

    static USHORT aIds[] = {
            SOT_FORMATSTR_ID_HTML,
            SOT_FORMATSTR_ID_HTML_SIMPLE,
            FORMAT_RTF,
            FORMAT_STRING,
            SOT_FORMATSTR_ID_SONLK,
            SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK,
            SOT_FORMATSTR_ID_DRAWING,
            SOT_FORMATSTR_ID_SVXB,
            FORMAT_GDIMETAFILE,
            FORMAT_BITMAP,
            SOT_FORMATSTR_ID_SVIM,
            0 };

    for( USHORT* pIds = aIds; *pIds; ++pIds )
        if( SwTransferable::_TestAllowedFormat( rData, *pIds, nDest ))
        {
            String sStr;
            switch( *pIds )
            {
            case FORMAT_STRING: sStr = SW_RES( STR_TEXTFORMAT );    break;
            case FORMAT_RTF:    sStr = SW_RES( STR_RTFFORMAT );     break;
            }
            rToFill.AddClipbrdFormat( *pIds, sStr );
        }
}

void SwTransferable::SetDataForDragAndDrop( const Point& rSttPos )
{
    String sGrfNm;
    const int nSelection = pWrtShell->GetSelectionType();
    if( SwWrtShell::SEL_GRF == nSelection)
    {
        AddFormat( SOT_FORMATSTR_ID_SVXB );
        const Graphic &rGrf = pWrtShell->GetGraphic();
        if ( rGrf.IsSupportedGraphic() )
        {
            AddFormat( FORMAT_GDIMETAFILE );
            AddFormat( FORMAT_BITMAP );
        }
        eBufferType = TRNSFR_GRAPHIC;
        pWrtShell->GetGrfNms( &sGrfNm, 0 );
    }
    else if( SwWrtShell::SEL_OLE == nSelection )
    {
        AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
        AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );
        AddFormat( FORMAT_GDIMETAFILE );
        eBufferType = TRNSFR_OLE;
    }
    //Gibt es ueberhaupt etwas zum bereitstellen?
    else if ( pWrtShell->IsSelection() || pWrtShell->IsFrmSelected() ||
              pWrtShell->IsObjSelected() )
    {
        if( pWrtShell->IsObjSelected() )
            eBufferType = TRNSFR_DRAWING;
        else
        {
            eBufferType = TRNSFR_DOCUMENT;
            if( SwWrtShell::NO_WORD !=
                pWrtShell->IntelligentCut( nSelection, FALSE ))
                eBufferType = TransferBufferType( TRNSFR_DOCUMENT_WORD
                                                    | eBufferType);
        }

        if( nSelection & SwWrtShell::SEL_TBL_CELLS )
            eBufferType = (TransferBufferType)(TRNSFR_TABELLE | eBufferType);

        AddFormat( SOT_FORMATSTR_ID_EMBED_SOURCE );
        AddFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR );

        //RTF vor das Metafile von OLE stellen, weil mit weniger verlusten
        //behaftet.
        if( !pWrtShell->IsObjSelected() )
        {
            AddFormat( FORMAT_RTF );
            AddFormat( SOT_FORMATSTR_ID_HTML );
        }
        if( pWrtShell->IsSelection() )
            AddFormat( FORMAT_STRING );

        if( nSelection & ( SwWrtShell::SEL_DRW | SwWrtShell::SEL_DRW_FORM ))
        {
            AddFormat( SOT_FORMATSTR_ID_DRAWING );
            AddFormat( FORMAT_GDIMETAFILE );
            AddFormat( FORMAT_BITMAP );
            eBufferType = (TransferBufferType)( TRNSFR_GRAPHIC | eBufferType );

            pClpGraphic = new Graphic;
            if( !pWrtShell->GetDrawObjGraphic( FORMAT_GDIMETAFILE, *pClpGraphic ))
                pOrigGrf = pClpGraphic;
            pClpBitmap = new Graphic;
            if( !pWrtShell->GetDrawObjGraphic( FORMAT_BITMAP, *pClpBitmap ))
                pOrigGrf = pClpBitmap;

            // ist es ein URL-Button ?
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

        //ObjectDescriptor wurde bereits aus der alten DocShell gefuellt.
        //Jetzt noch anpassen. Dadurch kann im GetData die erste Anfrage
        //auch noch mit delayed rendering beantwortet werden.
        aObjDesc.mbCanLink = FALSE;
        aObjDesc.maDragStartPos = rSttPos;
        aObjDesc.maSize = OutputDevice::LogicToLogic( Size( OLESIZE ),
                                                MAP_TWIP, MAP_100TH_MM );
    }
    else if( nSelection & SwWrtShell::SEL_TXT && !pWrtShell->HasMark() )
    {
        // ist nur ein Feld - Selektiert?
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
    bOldIdle = pWrtShell->GetViewOptions()->IsIdle();
    bCleanUp = TRUE;

    ((SwViewOption *)pWrtShell->GetViewOptions())->SetIdle( FALSE );

    if( pWrtShell->IsSelFrmMode() )
        pWrtShell->ShowCrsr();

#ifdef MAC
//!! Mac is able to show a outline of the drag source. The new interface not!
//!!    const Region aRegion( pWrtShell->GetCrsrRegion() );
//!!                    , &aRegion
#endif

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
    //Und noch die letzten Nacharbeiten damit alle Stati stimmen.
    if( DND_ACTION_MOVE == nAction  )
    {
        if( bCleanUp )
        {
            //Es wurde auserhalb des Writers gedroped. Wir muessen noch
            //loeschen.

            pWrtShell->StartAllAction();
            pWrtShell->StartUndo( UIUNDO_DRAG_AND_MOVE );
            if ( pWrtShell->IsTableMode() )
                pWrtShell->DeleteTblSel();
            else
            {
                if ( !(pWrtShell->IsSelFrmMode() || pWrtShell->IsObjSelected()) )
                    //SmartCut, eines der Blanks mitnehmen.
                    pWrtShell->IntelligentCut( pWrtShell->GetSelectionType(), TRUE );
                pWrtShell->DelRight();
            }
            pWrtShell->EndUndo( UIUNDO_DRAG_AND_MOVE );
            pWrtShell->EndAllAction();
        }
        else
        {
            const int nSelection = pWrtShell->GetSelectionType();
            if( ( SwWrtShell::SEL_FRM | SwWrtShell::SEL_GRF |
                 SwWrtShell::SEL_OLE | SwWrtShell::SEL_DRW ) & nSelection )
            {
                pWrtShell->EnterSelFrmMode();
            }
        }
    }
    if( pWrtShell->IsSelFrmMode() )
        pWrtShell->HideCrsr();
    else
        pWrtShell->ShowCrsr();
//!!    else if( DND_ACTION_NONE != nAction )
//!!        pWrtShell->ShowCrsr();
//!!    else
//!!    {
//!!        //Muss wohl sein weil gescrollt wurde und ?...?
//!!        pWrtShell->StartAction();
//!!        pWrtShell->EndAction();
//!!    }

    pWrtShell->GetView().GetEditWin().DragFinished();
    ((SwViewOption *)pWrtShell->GetViewOptions())->SetIdle( bOldIdle );
}


/*  */

int SwTransferable::PrivatePaste( SwWrtShell& rShell )
{
    // erst den SelectionType erfragen, dann Action-Klammerung !!!!
    // (sonst wird nicht in eine TabellenSelektion gepastet!!!)
    ASSERT( !rShell.ActionPend(), "Paste darf nie eine Actionklammerung haben" );
    const int nSelection = rShell.GetSelectionType();

    SwTrnsfrActionAndUndo aAction( &rShell, (USHORT)UNDO_INSERT );

    //Selektierten Inhalt loeschen, nicht bei Tabellen-Selektion und
    //Tabelle im Clipboard
    if( rShell.HasSelection() && !( nSelection & SwWrtShell::SEL_TBL_CELLS))
    {
        rShell.DelRight();
        // war ein Fly selektiert, so muss jetzt fuer eine gueltige
        // Cursor-Position gesorgt werden! (geparkter Cursor!)
        if( ( SwWrtShell::SEL_FRM | SwWrtShell::SEL_GRF |
            SwWrtShell::SEL_OLE | SwWrtShell::SEL_DRW |
            SwWrtShell::SEL_DRW_FORM ) & nSelection )
        {
            // den Cursor wieder positionieren
            Point aPt( rShell.GetCharRect().Pos() );
            rShell.SwCrsrShell::SetCrsr( aPt, TRUE );
        }
    }

    BOOL bInWrd = rShell.IsInWrd(),
         bSmart = eBufferType & TRNSFR_DOCUMENT_WORD &&
                  (bInWrd || rShell.IsEndWrd()),
         bSttWrd = rShell.IsSttWrd(),
         bEndWrd = rShell.IsEndWrd();

    if ( bSmart && !bSttWrd && (bInWrd || bEndWrd) )
        rShell.SwEditShell::Insert(' ');

    int nRet = rShell.Paste( pClpDocFac->GetDoc() );

    // Wenn Smart Paste dann Leerzeichen einfuegen
    if ( nRet && bSmart && (bInWrd || bSttWrd) )
        rShell.SwEditShell::Insert(' ');

    return nRet;
}

int SwTransferable::PrivateDrop( SwWrtShell& rSh, const Point& rDragPt,
                                BOOL bMove )
{
    int cWord    = 0;
    BOOL bInWrd  = FALSE;
    BOOL bEndWrd = FALSE;
    BOOL bSttWrd = FALSE;
    BOOL bSttPara= FALSE;
    BOOL bTblSel = FALSE;
    BOOL bFrmSel = FALSE;

    SwWrtShell& rSrcSh = *GetShell();

    rSh.UnSetVisCrsr();

    if( TRNSFR_INETFLD == eBufferType )
    {
        if( rSh.GetFmtFromObj( rDragPt ) )
        {
            INetBookmark aTmp;
            if( (TRNSFR_INETFLD & eBufferType) && pBkmk )
                aTmp = *pBkmk;

            // Zielgrafik selektieren
            if( rSh.SelectObj( rDragPt ) )
            {
                rSh.HideCrsr();
                rSh.EnterSelFrmMode( &rDragPt );
                bFrmDrag = TRUE;
            }

            const int nSelection = rSh.GetSelectionType();

            // Draw-Objekte erstmal noch nicht beruecksichtigen
            if( SwWrtShell::SEL_GRF & nSelection )
            {
                SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                rSh.GetFlyFrmAttr( aSet );
                SwFmtURL aURL( (SwFmtURL&)aSet.Get( RES_URL ) );
                aURL.SetURL( aTmp.GetURL(), FALSE );
                aSet.Put( aURL );
                rSh.SetFlyFrmAttr( aSet );
                return 1;
            }

            if( SwWrtShell::SEL_DRW & nSelection )
            {
                rSh.LeaveSelFrmMode();
                rSh.UnSelectFrm();
                rSh.ShowCrsr();
                bFrmDrag = FALSE;
            }
        }
    }

    if( &rSh != &rSrcSh && (SwWrtShell::SEL_GRF & rSh.GetSelectionType()) &&
        TRNSFR_GRAPHIC == eBufferType )
    {
        // ReRead auf die Grafik
        String sGrfNm, sFltNm;
        rSrcSh.GetGrfNms( &sGrfNm, &sFltNm );
        rSh.ReRead( sGrfNm, sFltNm, &rSrcSh.GetGraphic() );
        return 1;
    }

    //Nicht in Selektionen oder selektierten Rahmen
    if( rSh.ChgCurrPam( rDragPt ) ||
        ( rSh.IsSelFrmMode() && rSh.IsInsideSelectedObj( rDragPt )) )
        return 0;

    if( rSrcSh.IsTableMode() )
        bTblSel = TRUE;
    else if( rSrcSh.IsSelFrmMode() || rSrcSh.IsObjSelected() )
    {
        // keine positionsgeschuetzten Objecte verschieben!
        if( bMove && rSrcSh.IsSelObjProtected( FLYPROTECT_POS ) )
            return 0;

        bFrmSel = TRUE;
    }

    const int nSel = rSrcSh.GetSelectionType();

    USHORT nUndoId = bMove ? UIUNDO_DRAG_AND_MOVE : UIUNDO_DRAG_AND_COPY;
    if( rSrcSh.GetDoc() != rSh.GetDoc() )
        rSrcSh.StartUndo( nUndoId );
    rSh.StartUndo( nUndoId );

    rSh.StartAction();
    rSrcSh.StartAction();

    if( &rSrcSh != &rSh )
    {
        rSh.EnterStdMode();
        rSh.SwCrsrShell::SetCrsr( rDragPt, TRUE );
        cWord = rSrcSh.IntelligentCut( nSel, FALSE );
    }
    else if( !bTblSel && !bFrmSel )
    {
        if( !rSh.IsAddMode() )
            rSh.SwCrsrShell::CreateCrsr();
        rSh.SwCrsrShell::SetCrsr( rDragPt, TRUE );
        rSh.GoPrevCrsr();
        cWord = rSh.IntelligentCut( rSh.GetSelectionType(), FALSE );
        rSh.GoNextCrsr();
    }

    bInWrd  = rSh.IsInWrd();
    bEndWrd = rSh.IsEndWrd();
    bSttWrd = rSh.IsSttWrd();
    bSttPara= rSh.IsSttPara();

    Point aSttPt( SwEditWin::GetDDStartPosX(), SwEditWin::GetDDStartPosY() );

    //JP 05.03.96: INetFelder erstmal selektieren !
    if( TRNSFR_INETFLD == eBufferType )
    {
        if( &rSrcSh == &rSh )
        {
            rSh.GoPrevCrsr();
            rSh.SwCrsrShell::SetCrsr( aSttPt, TRUE );
            rSh.SelectTxtAttr( RES_TXTATR_INETFMT );
            if( rSh.ChgCurrPam( rDragPt ) )
            {
                // nicht in sich selbst kopieren/verschieben
                rSh.DestroyCrsr();
                rSh.EndUndo( nUndoId );
                rSh.EndAction();
                rSh.EndAction();
                return 0;
            }
            rSh.GoNextCrsr();
        }
        else
        {
            rSrcSh.SwCrsrShell::SetCrsr( aSttPt, TRUE );
            rSrcSh.SelectTxtAttr( RES_TXTATR_INETFMT );
        }

        // ist am Einfuege Punkt ein URL-Attribut? Dann das ersetzen,
        // also einfach eine Selektion aufspannen?
        rSh.DelINetAttrWithText();
        bDDINetAttr = TRUE;
    }

    if ( rSrcSh.IsSelFrmMode() )
    {
        //Hack: Spezialbehandlung austricksen
        aSttPt -= aSttPt - rSrcSh.GetObjRect().Pos();
    }

    BOOL bRet = rSrcSh.SwFEShell::Copy( &rSh, aSttPt, rDragPt, bMove );

    rSrcSh.Push();
    if ( bRet && bMove && !bFrmSel )
    {
        if ( bTblSel )
            rSrcSh.DeleteTblSel();
        else
        {
            //SmartCut, eines der Blank mitnehmen.
            rSh.SwCrsrShell::DestroyCrsr();
            if ( cWord == SwWrtShell::WORD_SPACE_BEFORE )
                rSh.ExtendSelection( FALSE );
            else if ( cWord == SwWrtShell::WORD_SPACE_AFTER )
                rSh.ExtendSelection();
            rSrcSh.DelRight();
        }
    }
    rSrcSh.KillPams();
    rSrcSh.Pop( FALSE );

    if( bRet && !bTblSel && !bFrmSel )
    {
        if( (bInWrd || bEndWrd) &&
            (cWord == SwWrtShell::WORD_SPACE_AFTER ||
                cWord == SwWrtShell::WORD_SPACE_BEFORE) )
        {
            if ( bSttWrd || bInWrd )
                rSh.SwEditShell::Insert(' ');
            if ( !bSttWrd || (bInWrd && !bSttPara) )
            {
                rSh.SwapPam();
                if ( !bSttWrd )
                    rSh.SwEditShell::Insert(' ');
                rSh.SwapPam();
            }
        }

        if( rSh.IsAddMode() )
            rSh.SwCrsrShell::CreateCrsr();
        else
        {
            // Selektionsmodus einschalten
            rSh.SttSelect();
            rSh.EndSelect();
        }
    }

    if( bRet && bMove && bFrmSel )
        rSrcSh.LeaveSelFrmMode();

    if( rSrcSh.GetDoc() != rSh.GetDoc() )
        rSrcSh.EndUndo( nUndoId );
    rSh.EndUndo( nUndoId );

        // Shell in den richtigen Status versetzen
    if( &rSrcSh != &rSh && ( rSh.IsFrmSelected() || rSh.IsObjSelected() ))
        rSh.EnterSelFrmMode();

    rSrcSh.EndAction();
    rSh.EndAction();
    return 1;
}


/*  */

#ifdef DDE_AVAILABLE

// -----------------------------------------------------------------------

SwTrnsfrDdeLink::SwTrnsfrDdeLink( SwTransferable& rTrans, SwWrtShell& rSh )
    : rTrnsfr( rTrans ), pDocShell( 0 ),
    bDelBookmrk( FALSE ), bInDisconnect( FALSE )
{
    // hier kommen wir nur bei Tabellen- oder Text-Selection an
    const int nSelection = rSh.GetSelectionType();
    if( SwWrtShell::SEL_TBL_CELLS & nSelection )
    {
        SwFrmFmt* pFmt = rSh.GetTableFmt();
        if( pFmt )
            sName = pFmt->GetName();
    }
    else
    {
        // wir erzeugen uns eine temp. Bookmark (ohne UNDO!)
        BOOL bUndo = rSh.DoesUndo();
        rSh.DoUndo( FALSE );
        BOOL bIsModified = rSh.IsModified();

        sName.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DDE_LINK" ));
        rSh.MakeUniqueBookmarkName( sName );

        //Ok, den eindeutigen Namen haben wir
        if( !rSh.SetBookmark( KeyCode(), sName, aEmptyStr ) )
            sName.Erase();
        else
        {
            USHORT nBookPos = rSh.FindBookmark( sName );
            if( USHRT_MAX != nBookPos )
            {
                SwBookmark& rBookMk = rSh.GetBookmark( nBookPos );
                rBookMk.SetType( DDE_BOOKMARK );
            }
            bDelBookmrk = TRUE;
            if( !bIsModified )
                rSh.ResetModified();
        }

        rSh.DoUndo( bUndo );
    }

    if( sName.Len() &&
        0 != ( pDocShell = rSh.GetDoc()->GetDocShell() ) )
    {
        // dann erzeugen wir uns mal unseren "Server" und connecten uns
        // zu diesem
        refObj = pDocShell->DdeCreateLinkSource( sName );
        if( refObj.Is() )
        {
            refObj->AddConnectAdvise( this );
            refObj->AddDataAdvise( this,
//                          SotExchange::GetFormatMimeType( FORMAT_RTF ),
                            aEmptyStr,
                            ADVISEMODE_NODATA | ADVISEMODE_ONLYONCE );
            nOldTimeOut = refObj->GetUpdateTimeout();
            refObj->SetUpdateTimeout( 0 );
        }
    }
}

// -----------------------------------------------------------------------

SwTrnsfrDdeLink::~SwTrnsfrDdeLink()
{
    if( refObj.Is() )
        Disconnect();
}

// -----------------------------------------------------------------------

void SwTrnsfrDdeLink::DataChanged( const String& ,
                                    const ::com::sun::star::uno::Any& )
{
    // tja das wars dann mit dem Link
    if( !bInDisconnect )
    {
        rTrnsfr.RemoveDDELinkFormat();
        Disconnect( FALSE );
    }
}

// -----------------------------------------------------------------------

BOOL SwTrnsfrDdeLink::WriteData( SvStream& rStrm )
{
    if( !refObj.Is() || !FindDocShell() )
        return FALSE;

    const ByteString aAppNm( GetpApp()->GetAppName(),  DDE_TXT_ENCODING );
    const ByteString aTopic( pDocShell->GetTitle( SFX_TITLE_FULLNAME ),
                            DDE_TXT_ENCODING );
    const ByteString aName( sName, DDE_TXT_ENCODING );

    sal_Char* pMem = new char[ aAppNm.Len() + aTopic.Len() + aName.Len() + 4 ];

    xub_StrLen nLen = aAppNm.Len();
    memcpy( pMem, aAppNm.GetBuffer(), nLen );
    pMem[ nLen++ ] = 0;
    memcpy( pMem + nLen, aTopic.GetBuffer(), aTopic.Len() );
    nLen += aTopic.Len();
    pMem[ nLen++ ] = 0;
    memcpy( pMem + nLen, aName.GetBuffer(), aName.Len() );
    nLen += aName.Len();
    pMem[ nLen++ ] = 0;
    pMem[ nLen++ ] = 0;

    rStrm.Write( pMem, nLen );
    delete pMem;

    if( bDelBookmrk )
    {
        // er wird das erstemal abgeholt, also ins Undo mitaufnehmen
        // aber wie??
    }

    SwDoc* pDoc = pDocShell->GetDoc();
    USHORT nBookPos = pDoc->FindBookmark( sName );
    if( USHRT_MAX != nBookPos )
    {
        SwBookmark* pBookMk = pDoc->GetBookmarks()[ nBookPos ];
        pBookMk->SetType( BOOKMARK );
        pDoc->SetModified();
    }

    bDelBookmrk = FALSE;
    return TRUE;
}

// -----------------------------------------------------------------------

void SwTrnsfrDdeLink::Disconnect( BOOL bRemoveDataAdvise )
{
    //JP 29.01.96 Bug 24432:
    //      kein DataChanged mehr entgegen nehmen, wenn man
    //      sich schon im Disconnet befindet!
    //      (DTOR vom Bookmark verschickt einen DataChanged!)
    BOOL bOldDisconnect = bInDisconnect;
    bInDisconnect = TRUE;

    // den nicht verwendeten Bookmark wieder zerstoeren (ohne Undo!)?
    if( bDelBookmrk && refObj.Is() && FindDocShell() )
    {
        SwDoc* pDoc = pDocShell->GetDoc();
        BOOL bUndo = pDoc->DoesUndo();
        pDoc->DoUndo( FALSE );

        BOOL bIsModified = pDoc->IsModified();

        pDoc->DelBookmark( sName );

        if( !bIsModified )
            pDoc->ResetModified();

        pDoc->DoUndo( bUndo );
        bDelBookmrk = FALSE;
    }

    if( refObj.Is() )
    {
        refObj->SetUpdateTimeout( nOldTimeOut );
        refObj->RemoveConnectAdvise( this );
        if( bRemoveDataAdvise )
            // in einem DataChanged darf das SelectionObject NIE geloescht
            // werden; wird schon von der Basisklasse erledigt
            // (ADVISEMODE_ONLYONCE!!!!)
            // Im normalen Disconnet aber schon!
            refObj->RemoveAllDataAdvise( this );
        refObj.Clear();
    }
    bInDisconnect = bOldDisconnect;
}

// -----------------------------------------------------------------------

BOOL SwTrnsfrDdeLink::FindDocShell()
{
    TypeId aType( TYPE( SwDocShell ) );
    SfxObjectShell* pTmpSh = SfxObjectShell::GetFirst( &aType );
    while( pTmpSh )
    {
        if( pTmpSh == pDocShell )       // die wollen wir haben
        {
            if( pDocShell->GetDoc() )
                return TRUE;
            break;      // das Doc ist nicht mehr vorhanden, also raus!
        }
        pTmpSh = SfxObjectShell::GetNext( *pTmpSh, &aType );
    }

    pDocShell = 0;
    return FALSE;
}

// -----------------------------------------------------------------------

void SwTrnsfrDdeLink::Closed()
{
    if( !bInDisconnect && refObj.Is() )
    {
        refObj->RemoveAllDataAdvise( this );
        refObj->RemoveConnectAdvise( this );
        refObj.Clear();
    }
}

#endif



