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

#include <config_features.h>

#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XEmbedObjectClipboardCreator.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/embed/MSOLEObjectSystemCreator.hpp>

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
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <svtools/imap.hxx>
#include <sot/storage.hxx>
#include <vcl/graphic.hxx>
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
#include <vcl/graphicfilter.hxx>

#include <svx/unomodel.hxx>
#include <fmturl.hxx>
#include <fmtinfmt.hxx>
#include <fmtfsize.hxx>
#include <swdtflvr.hxx>
#include <shellio.hxx>
#include <ddefld.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
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
#include <fldbas.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndole.hxx>
#include <swwait.hxx>
#include <viewopt.hxx>
#include <swerror.h>
#include <SwCapObjType.hxx>
#include <cmdid.h>
#include <dochdl.hrc>
#include <comcore.hrc>
#include <sot/stg.hxx>
#include <svx/svditer.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/svdpage.hxx>
#include <avmedia/mediawindow.hxx>
#include <swcrsr.hxx>
#include <SwRewriter.hxx>
#include <globals.hrc>
#include <app.hrc>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <swserv.hxx>
#include <calbck.hxx>
#include <fmtmeta.hxx>

#include <vcl/GraphicNativeTransform.hxx>
#include <vcl/GraphicNativeMetadata.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/classificationhelper.hxx>
#include <sfx2/sfxresid.hxx>

#include <memory>

extern bool g_bFrameDrag;
extern bool g_bDDINetAttr;
extern bool g_bExecuteDrag;

#define OLESIZE 11905 - 2 * lMinBorder, 6 * MM50

#define SWTRANSFER_OBJECTTYPE_DRAWMODEL         static_cast<SotClipboardFormatId>(0x00000001)
#define SWTRANSFER_OBJECTTYPE_HTML              static_cast<SotClipboardFormatId>(0x00000002)
#define SWTRANSFER_OBJECTTYPE_RTF               static_cast<SotClipboardFormatId>(0x00000004)
#define SWTRANSFER_OBJECTTYPE_STRING            static_cast<SotClipboardFormatId>(0x00000008)
#define SWTRANSFER_OBJECTTYPE_SWOLE             static_cast<SotClipboardFormatId>(0x00000010)
#define SWTRANSFER_OBJECTTYPE_DDE               static_cast<SotClipboardFormatId>(0x00000020)

using namespace ::svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::datatransfer;
using namespace nsTransferBufferType;

#define DDE_TXT_ENCODING    osl_getThreadTextEncoding()

class SwTrnsfrDdeLink : public ::sfx2::SvBaseLink
{
    OUString sName;
    ::sfx2::SvLinkSourceRef refObj;
    SwTransferable& rTrnsfr;
    SwDocShell* pDocShell;
    sal_uLong nOldTimeOut;
    bool bDelBookmrk : 1;
    bool bInDisconnect : 1;

    bool FindDocShell();

    using sfx2::SvBaseLink::Disconnect;

protected:
    virtual ~SwTrnsfrDdeLink();

public:
    SwTrnsfrDdeLink( SwTransferable& rTrans, SwWrtShell& rSh );

    virtual ::sfx2::SvBaseLink::UpdateResult DataChanged(
        const OUString& rMimeType, const css::uno::Any & rValue ) override;
    virtual void Closed() override;

    bool WriteData( SvStream& rStrm );

    void Disconnect( bool bRemoveDataAdvise );
};

// helper class for Action and Undo enclosing
class SwTrnsfrActionAndUndo
{
    SwWrtShell *pSh;
    SwUndoId eUndoId;
public:
    SwTrnsfrActionAndUndo( SwWrtShell *pS, SwUndoId nId,
                           const SwRewriter * pRewriter = nullptr,
                           bool bDelSel = false)
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
    : m_pWrtShell( &rSh ),
    m_pCreatorView( nullptr ),
    m_pClpDocFac( nullptr ),
    m_pClpGraphic( nullptr ),
    m_pClpBitmap( nullptr ),
    m_pOrigGraphic( nullptr ),
    m_pBookmark( nullptr ),
    m_pImageMap( nullptr ),
    m_pTargetURL( nullptr ),
    m_eBufferType( TRNSFR_NONE )
{
    rSh.GetView().AddTransferable(*this);
    SwDocShell* pDShell = rSh.GetDoc()->GetDocShell();
    if( pDShell )
    {
        pDShell->FillTransferableObjectDescriptor( m_aObjDesc );
        if( pDShell->GetMedium() )
        {
            const INetURLObject& rURLObj = pDShell->GetMedium()->GetURLObject();
            m_aObjDesc.maDisplayName = URIHelper::removePassword(
                                rURLObj.GetMainURL( INetURLObject::NO_DECODE ),
                                INetURLObject::WAS_ENCODED,
                                INetURLObject::DECODE_UNAMBIGUOUS );
        }

        PrepareOLE( m_aObjDesc );
    }
}

SwTransferable::~SwTransferable()
{
    Application::GetSolarMutex().acquire();

    // the DDELink still needs the WrtShell!
    if( m_xDdeLink.Is() )
    {
        static_cast<SwTrnsfrDdeLink*>(&m_xDdeLink)->Disconnect( true );
        m_xDdeLink.Clear();
    }

    m_pWrtShell = nullptr;

    // release reference to the document so that aDocShellRef will delete
    // it (if aDocShellRef is set). Otherwise, the OLE nodes keep references
    // to their sub-storage when the storage is already dead.
    delete m_pClpDocFac;

    // first close, then the Ref. can be cleared as well, so that
    // the DocShell really gets deleted!
    if( m_aDocShellRef.Is() )
    {
        SfxObjectShell * pObj = m_aDocShellRef;
        SwDocShell* pDocSh = static_cast<SwDocShell*>(pObj);
        pDocSh->DoClose();
    }
    m_aDocShellRef.Clear();

    SwModule* pMod = SW_MOD();
    if(pMod)
    {
        if ( pMod->m_pDragDrop == this )
            pMod->m_pDragDrop = nullptr;
        else if ( pMod->m_pXSelection == this )
            pMod->m_pXSelection = nullptr;
    }

    delete m_pClpGraphic;
    delete m_pClpBitmap;
    delete m_pImageMap;
    delete m_pTargetURL;
    delete m_pBookmark;

    m_eBufferType = TRNSFR_NONE;

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
    if( this == pMod->m_pDragDrop )
        pMod->m_pDragDrop = nullptr;
    else if( this == pMod->m_pXSelection )
        pMod->m_pXSelection = nullptr;
}

void SwTransferable::AddSupportedFormats()
{
    // only need if we are the current XSelection Object
    SwModule *pMod = SW_MOD();
    if( this == pMod->m_pXSelection || comphelper::LibreOfficeKit::isActive())
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
    rDoc.getIDocumentSettingAccess().set(DocumentSettingId::BROWSE_MODE, true );
}

uno::Reference < embed::XEmbeddedObject > SwTransferable::FindOLEObj( sal_Int64& nAspect ) const
{
    uno::Reference < embed::XEmbeddedObject > xObj;
    if( m_pClpDocFac )
    {
        SwIterator<SwContentNode,SwFormatColl> aIter( *m_pClpDocFac->GetDoc()->GetDfltGrfFormatColl() );
        for( SwContentNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
            if( ND_OLENODE == pNd->GetNodeType() )
            {
                xObj = static_cast<SwOLENode*>(pNd)->GetOLEObj().GetOleRef();
                nAspect = static_cast<SwOLENode*>(pNd)->GetAspect();
                break;
            }
    }
    return xObj;
}

const Graphic* SwTransferable::FindOLEReplacementGraphic() const
{
    if( m_pClpDocFac )
    {
        SwIterator<SwContentNode,SwFormatColl> aIter( *m_pClpDocFac->GetDoc()->GetDfltGrfFormatColl() );
        for( SwContentNode* pNd = aIter.First(); pNd; pNd = aIter.Next() )
            if( ND_OLENODE == pNd->GetNodeType() )
            {
                return static_cast<SwOLENode*>(pNd)->GetGraphic();
            }
    }

    return nullptr;
}

void SwTransferable::RemoveDDELinkFormat( const vcl::Window& rWin )
{
    RemoveFormat( SotClipboardFormatId::LINK );
    CopyToClipboard( const_cast<vcl::Window*>(&rWin) );
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

        rDest.ReplaceCompatibilityOptions(rSrc);
        rDest.ReplaceDefaults(rSrc);

        //It would probably make most sense here to only insert the styles used
        //by the selection, e.g. apply SwDoc::IsUsed on styles ?
        rDest.ReplaceStyles(rSrc, false);

        rSrcWrtShell.Copy(&rDest);

        rDest.GetMetaFieldManager().copyDocumentProperties(rSrc);
    }

    void lclCheckAndPerformRotation(Graphic& aGraphic)
    {
        GraphicNativeMetadata aMetadata;
        if ( aMetadata.read(aGraphic) )
        {
            sal_uInt16 aRotation = aMetadata.getRotation();
            if (aRotation != 0)
            {
                ScopedVclPtrInstance< MessageDialog > aQueryBox(
                        nullptr, "QueryRotateIntoStandardOrientationDialog",
                        "modules/swriter/ui/queryrotateintostandarddialog.ui");
                if (aQueryBox->Execute() == RET_YES)
                {
                    GraphicNativeTransform aTransform( aGraphic );
                    aTransform.rotate( aRotation );
                }
            }
        }
    }
}

bool SwTransferable::GetData( const DataFlavor& rFlavor, const OUString& rDestDoc )
{
    SotClipboardFormatId nFormat = SotExchange::GetFormat( rFlavor );

    // we can only fulfil the request if
    // 1) we have data for this format
    // 2) we have either a clipboard document (pClpDocFac), or
    //    we have a SwWrtShell (so we can generate a new clipboard document)
    if( !HasFormat( nFormat ) || ( m_pClpDocFac == nullptr && m_pWrtShell == nullptr ) )
        return false;

    if( !m_pClpDocFac )
    {
        SelectionType nSelectionType = m_pWrtShell->GetSelectionType();

        // when pending we will not get the correct type, but nsSelectionType::SEL_TXT
        // as fallback. This *happens* during D&D, so we need to check if we are in
        // the fallback and just try to get a graphic
        const bool bPending(m_pWrtShell->BasicActionPend());

        // SEL_GRF is from ContentType of editsh
        if(bPending || ((nsSelectionType::SEL_GRF | nsSelectionType::SEL_DRW_FORM) & nSelectionType))
        {
            m_pClpGraphic = new Graphic;
            if( !m_pWrtShell->GetDrawObjGraphic( SotClipboardFormatId::GDIMETAFILE, *m_pClpGraphic ))
                m_pOrigGraphic = m_pClpGraphic;
            m_pClpBitmap = new Graphic;
            if( !m_pWrtShell->GetDrawObjGraphic( SotClipboardFormatId::BITMAP, *m_pClpBitmap ))
                m_pOrigGraphic = m_pClpBitmap;

            // is it an URL-Button ?
            OUString sURL;
            OUString sDesc;
            if( m_pWrtShell->GetURLFromButton( sURL, sDesc ) )
            {
                m_pBookmark = new INetBookmark( sURL, sDesc );
                m_eBufferType = TRNSFR_INETFLD;
            }
        }

        m_pClpDocFac = new SwDocFac;
        SwDoc *const pTmpDoc = lcl_GetDoc(*m_pClpDocFac);

        pTmpDoc->getIDocumentFieldsAccess().LockExpFields();     // never update fields - leave text as it is
        lclOverWriteDoc(*m_pWrtShell, *pTmpDoc);

        // in CORE a new one was created (OLE-Objekte copied!)
        m_aDocShellRef = pTmpDoc->GetTmpDocShell();
        if( m_aDocShellRef.Is() )
            SwTransferable::InitOle( m_aDocShellRef, *pTmpDoc );
        pTmpDoc->SetTmpDocShell( nullptr );

        if( nSelectionType & nsSelectionType::SEL_TXT && !m_pWrtShell->HasMark() )
        {
            SwContentAtPos aContentAtPos( SwContentAtPos::SW_INETATTR );

            Point aPos( SwEditWin::GetDDStartPosX(), SwEditWin::GetDDStartPosY());

            bool bSelect = g_bExecuteDrag &&
                            m_pWrtShell->GetView().GetDocShell() &&
                            !m_pWrtShell->GetView().GetDocShell()->IsReadOnly();
            if( m_pWrtShell->GetContentAtPos( aPos, aContentAtPos, bSelect ) )
            {
                m_pBookmark = new INetBookmark(
                        static_cast<const SwFormatINetFormat*>(aContentAtPos.aFnd.pAttr)->GetValue(),
                        aContentAtPos.sStr );
                m_eBufferType = TRNSFR_INETFLD;
                if( bSelect )
                    m_pWrtShell->SelectTextAttr( RES_TXTATR_INETFMT );
            }
        }
        if( m_pWrtShell->IsFrameSelected() )
        {
             SfxItemSet aSet( m_pWrtShell->GetAttrPool(), RES_URL, RES_URL );
            m_pWrtShell->GetFlyFrameAttr( aSet );
            const SwFormatURL& rURL = static_cast<const SwFormatURL&>(aSet.Get( RES_URL ));
            if( rURL.GetMap() )
                m_pImageMap = new ImageMap( *rURL.GetMap() );
            else if( !rURL.GetURL().isEmpty() )
                m_pTargetURL = new INetImage( aEmptyOUStr, rURL.GetURL(),
                                            rURL.GetTargetFrameName(),
                                            aEmptyOUStr, Size() );
        }
    }

    bool bOK = false;
    if( TRNSFR_OLE == m_eBufferType )
    {
        //TODO/MBA: testing - is this the "single OLE object" case?!
        // get OLE-Object from ClipDoc and get the data from that.
        sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT; // will be set in the next statement
        uno::Reference < embed::XEmbeddedObject > xObj = FindOLEObj( nAspect );
        const Graphic* pOLEGraph = FindOLEReplacementGraphic();
        if( xObj.is() )
        {
            TransferableDataHelper aD( new SvEmbedTransferHelper( xObj, pOLEGraph, nAspect ) );
            uno::Any aAny = aD.GetAny(rFlavor, rDestDoc);
            if( aAny.hasValue() )
                bOK = SetAny( aAny, rFlavor );
        }

        // the following solution will be used in the case when the object can not generate the image
        // TODO/LATER: in future the transferhelper must probably be created based on object and the replacement stream
        // TODO: Block not required now, SvEmbedTransferHelper should be able to handle GDIMetaFile format
        if ( nFormat == SotClipboardFormatId::GDIMETAFILE )
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
        case SotClipboardFormatId::LINK:
            if( m_xDdeLink.Is() )
                bOK = SetObject( &m_xDdeLink, SWTRANSFER_OBJECTTYPE_DDE, rFlavor );
            break;

        case SotClipboardFormatId::OBJECTDESCRIPTOR:
        case SotClipboardFormatId::LINKSRCDESCRIPTOR:
            bOK = SetTransferableObjectDescriptor( m_aObjDesc, rFlavor );
            break;

        case SotClipboardFormatId::DRAWING:
            {
                SwDoc *const pDoc = lcl_GetDoc(*m_pClpDocFac);
                bOK = SetObject( pDoc->getIDocumentDrawModelAccess().GetDrawModel(),
                                SWTRANSFER_OBJECTTYPE_DRAWMODEL, rFlavor );
            }
            break;

        case SotClipboardFormatId::STRING:
        {
            SwDoc *const pDoc = lcl_GetDoc(*m_pClpDocFac);
            bOK = SetObject( pDoc, SWTRANSFER_OBJECTTYPE_STRING, rFlavor );
        }
        break;
        case SotClipboardFormatId::RTF:
        {
            SwDoc *const pDoc = lcl_GetDoc(*m_pClpDocFac);
            bOK = SetObject( pDoc, SWTRANSFER_OBJECTTYPE_RTF, rFlavor );
        }
            break;

        case SotClipboardFormatId::HTML:
        {
            SwDoc *const pDoc = lcl_GetDoc(*m_pClpDocFac);
            bOK = SetObject( pDoc, SWTRANSFER_OBJECTTYPE_HTML, rFlavor );
        }
            break;

        case SotClipboardFormatId::SVXB:
            if( m_eBufferType & TRNSFR_GRAPHIC && m_pOrigGraphic )
                bOK = SetGraphic( *m_pOrigGraphic, rFlavor );
            break;

        case SotClipboardFormatId::GDIMETAFILE:
            if( m_eBufferType & TRNSFR_GRAPHIC )
                bOK = SetGDIMetaFile( m_pClpGraphic->GetGDIMetaFile(), rFlavor );
            break;
        case SotClipboardFormatId::BITMAP:
        case SotClipboardFormatId::PNG:
            // Neither pClpBitmap nor pClpGraphic are necessarily set
            if( (m_eBufferType & TRNSFR_GRAPHIC) && (m_pClpBitmap != nullptr || m_pClpGraphic != nullptr))
                bOK = SetBitmapEx( (m_pClpBitmap ? m_pClpBitmap : m_pClpGraphic)->GetBitmapEx(), rFlavor );
            break;

        case SotClipboardFormatId::SVIM:
            if( m_pImageMap )
                bOK = SetImageMap( *m_pImageMap, rFlavor );
            break;

        case SotClipboardFormatId::INET_IMAGE:
            if( m_pTargetURL )
                bOK = SetINetImage( *m_pTargetURL, rFlavor );
            break;

        case SotClipboardFormatId::SOLK:
        case SotClipboardFormatId::NETSCAPE_BOOKMARK:
        case SotClipboardFormatId::FILEGRPDESCRIPTOR:
        case SotClipboardFormatId::FILECONTENT:
        case SotClipboardFormatId::UNIFORMRESOURCELOCATOR:
        case SotClipboardFormatId::SIMPLE_FILE:
            if( (TRNSFR_INETFLD & m_eBufferType) && m_pBookmark )
                bOK = SetINetBookmark( *m_pBookmark, rFlavor );
            break;

        case SotClipboardFormatId::EMBED_SOURCE:
            if( !m_aDocShellRef.Is() )
            {
                SwDoc *const pDoc = lcl_GetDoc(*m_pClpDocFac);
                SwDocShell* pNewDocSh = new SwDocShell( pDoc,
                                         SfxObjectCreateMode::EMBEDDED );
                m_aDocShellRef = pNewDocSh;
                m_aDocShellRef->DoInitNew();
                SwTransferable::InitOle( m_aDocShellRef, *pDoc );
            }
            bOK = SetObject( &m_aDocShellRef, SWTRANSFER_OBJECTTYPE_SWOLE,
                            rFlavor );
            break;
         default: break;
        }
    }
    return bOK;
}

bool SwTransferable::WriteObject( tools::SvRef<SotStorageStream>& xStream,
                                    void* pObject, SotClipboardFormatId nObjectType,
                                    const DataFlavor& /*rFlavor*/ )
{
    bool bRet = false;
    WriterRef xWrt;

    switch( nObjectType )
    {
    case SWTRANSFER_OBJECTTYPE_DRAWMODEL:
        {
            // don't change the sequence of commands
            SdrModel *pModel = static_cast<SdrModel*>(pObject);
            xStream->SetBufferSize( 16348 );

            // for the changed pool defaults from drawing layer pool set those
            // attributes as hard attributes to preserve them for saving
            const SfxItemPool& rItemPool = pModel->GetItemPool();
            const SvxFontHeightItem& rDefaultFontHeight = static_cast<const SvxFontHeightItem&>(rItemPool.GetDefaultItem(EE_CHAR_FONTHEIGHT));

            // SW should have no MasterPages
            OSL_ENSURE(0L == pModel->GetMasterPageCount(), "SW with MasterPages (!)");

            for(sal_uInt16 a(0); a < pModel->GetPageCount(); a++)
            {
                const SdrPage* pPage = pModel->GetPage(a);
                SdrObjListIter aIter(*pPage, IM_DEEPNOGROUPS);

                while(aIter.IsMore())
                {
                    SdrObject* pObj = aIter.Next();
                    const SvxFontHeightItem& rItem = static_cast<const SvxFontHeightItem&>(pObj->GetMergedItem(EE_CHAR_FONTHEIGHT));

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
            SfxObjectShell*   pEmbObj = static_cast<SfxObjectShell*>(pObject);
            try
            {
                ::utl::TempFile     aTempFile;
                aTempFile.EnableKillingFile();
                uno::Reference< embed::XStorage > xWorkStore =
                    ::comphelper::OStorageHelper::GetStorageFromURL( aTempFile.GetURL(), embed::ElementModes::READWRITE );

                // write document storage
                pEmbObj->SetupStorage( xWorkStore, SOFFICE_FILEFORMAT_CURRENT, false );
                // mba: no BaseURL for clipboard
                SfxMedium aMedium( xWorkStore, OUString() );
                bRet = pEmbObj->DoSaveObjectAs( aMedium, false );
                pEmbObj->DoSaveCompleted();

                uno::Reference< embed::XTransactedObject > xTransact( xWorkStore, uno::UNO_QUERY );
                if ( xTransact.is() )
                    xTransact->commit();

                std::unique_ptr<SvStream> pSrcStm(::utl::UcbStreamHelper::CreateStream( aTempFile.GetURL(), StreamMode::READ ));
                if( pSrcStm )
                {
                    xStream->SetBufferSize( 0xff00 );
                    xStream->WriteStream( *pSrcStm );
                    pSrcStm.reset();
                }

                bRet = true;

                xWorkStore->dispose();
                xWorkStore.clear();
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
            SwTrnsfrDdeLink* pDdeLnk = static_cast<SwTrnsfrDdeLink*>(pObject);
            if( pDdeLnk->WriteData( *xStream ) )
            {
                xStream->Commit();
                bRet = ERRCODE_NONE == xStream->GetError();
            }
        }
        break;

    case SWTRANSFER_OBJECTTYPE_HTML:
        GetHTMLWriter( aEmptyOUStr, OUString(), xWrt );
        break;

    case SWTRANSFER_OBJECTTYPE_RTF:
        GetRTFWriter( aEmptyOUStr, OUString(), xWrt );
        break;

    case SWTRANSFER_OBJECTTYPE_STRING:
        GetASCWriter( aEmptyOUStr, OUString(), xWrt );
        if( xWrt.Is() )
        {
            SwAsciiOptions aAOpt;
            aAOpt.SetCharSet( RTL_TEXTENCODING_UTF8 );
            xWrt->SetAsciiOptions( aAOpt );

            // no start char for clipboard
            xWrt->bUCS2_WithStartChar = false;
        }
        break;
    default: break;
    }

    if( xWrt.Is() )
    {
        SwDoc* pDoc = static_cast<SwDoc*>(pObject);
        xWrt->bWriteClipboardDoc = true;
        xWrt->bWriteOnlyFirstTable = 0 != (TRNSFR_TABELLE & m_eBufferType);
        xWrt->SetShowProgress(false);

#if defined(DEBUGPASTE)
        SvFileStream aPasteDebug(OUString(
            "PASTEBUFFER.debug"), StreamMode::WRITE|StreamMode::TRUNC);
        SwWriter aDbgWrt( aPasteDebug, *pDoc );
        aDbgWrt.Write( xWrt );
#endif

        SwWriter aWrt( *xStream, *pDoc );
        if( !IsError( aWrt.Write( xWrt )) )
        {
            xStream->WriteChar( '\0' );               // terminate with a zero
            xStream->Commit();
            bRet = true;
        }
    }

    return bRet;
}

int SwTransferable::Cut()
{
    int nRet = Copy( true );
    if( nRet )
        DeleteSelection();
    return nRet;
}

void SwTransferable::DeleteSelection()
{
    if(!m_pWrtShell)
        return;
    // ask for type of selection before action-bracketing
    const int nSelection = m_pWrtShell->GetSelectionType();
    m_pWrtShell->StartUndo( UNDO_START );
    if( ( nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL ) & nSelection )
        m_pWrtShell->IntelligentCut( nSelection );
    m_pWrtShell->DelRight();
    m_pWrtShell->EndUndo( UNDO_END );
}

int SwTransferable::PrepareForCopy( bool bIsCut )
{
    int nRet = 1;
    if(!m_pWrtShell)
        return 0;

    OUString sGrfNm;
    const int nSelection = m_pWrtShell->GetSelectionType();
    if( nSelection == nsSelectionType::SEL_GRF )
    {
        m_pClpGraphic = new Graphic;
        if( !m_pWrtShell->GetDrawObjGraphic( SotClipboardFormatId::GDIMETAFILE, *m_pClpGraphic ))
            m_pOrigGraphic = m_pClpGraphic;
        m_pClpBitmap = new Graphic;
        if( !m_pWrtShell->GetDrawObjGraphic( SotClipboardFormatId::BITMAP, *m_pClpBitmap ))
            m_pOrigGraphic = m_pClpBitmap;

        m_pClpDocFac = new SwDocFac;
        SwDoc *const pDoc = lcl_GetDoc(*m_pClpDocFac);
        m_pWrtShell->Copy( pDoc );

        if (m_pOrigGraphic && !m_pOrigGraphic->GetBitmap().IsEmpty())
          AddFormat( SotClipboardFormatId::SVXB );

        PrepareOLE( m_aObjDesc );
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );

        const Graphic* pGrf = m_pWrtShell->GetGraphic();
        if( pGrf && pGrf->IsSupportedGraphic() )
        {
            AddFormat( SotClipboardFormatId::GDIMETAFILE );
            AddFormat( SotClipboardFormatId::PNG );
            AddFormat( SotClipboardFormatId::BITMAP );
        }
        m_eBufferType = TRNSFR_GRAPHIC;
        m_pWrtShell->GetGrfNms( &sGrfNm, nullptr );
    }
    else if ( nSelection == nsSelectionType::SEL_OLE )
    {
        m_pClpDocFac = new SwDocFac;
        SwDoc *const pDoc = lcl_GetDoc(*m_pClpDocFac);
        m_aDocShellRef = new SwDocShell( pDoc, SfxObjectCreateMode::EMBEDDED);
        m_aDocShellRef->DoInitNew();
        m_pWrtShell->Copy( pDoc );

        AddFormat( SotClipboardFormatId::EMBED_SOURCE );

        // --> OD #i98753#
        // set size of embedded object at the object description structure
        m_aObjDesc.maSize = OutputDevice::LogicToLogic( m_pWrtShell->GetObjSize(), MAP_TWIP, MAP_100TH_MM );
        // <--
        PrepareOLE( m_aObjDesc );
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );

        AddFormat( SotClipboardFormatId::GDIMETAFILE );

        // Fetch the formats supported via embedtransferhelper as well
        sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
        uno::Reference < embed::XEmbeddedObject > xObj = FindOLEObj( nAspect );
        const Graphic* pOLEGraph = FindOLEReplacementGraphic();
        if( xObj.is() )
        {
            TransferableDataHelper aD( new SvEmbedTransferHelper( xObj, pOLEGraph, nAspect ) );
            if ( aD.GetTransferable().is() )
            {
                DataFlavorExVector              aVector( aD.GetDataFlavorExVector() );
                DataFlavorExVector::iterator    aIter( aVector.begin() ), aEnd( aVector.end() );

                while( aIter != aEnd )
                    AddFormat( *aIter++ );
            }
        }
        m_eBufferType = TRNSFR_OLE;
    }
    // Is there anything to provide anyway?
    else if ( m_pWrtShell->IsSelection() || m_pWrtShell->IsFrameSelected() ||
              m_pWrtShell->IsObjSelected() )
    {
        std::unique_ptr<SwWait> pWait;
        if( m_pWrtShell->ShouldWait() )
            pWait.reset(new SwWait( *m_pWrtShell->GetView().GetDocShell(), true ));

        m_pClpDocFac = new SwDocFac;

        // create additional cursor so that equal treatment of keyboard
        // and mouse selection is possible.
        // In AddMode with keyboard selection, the new cursor is not created
        // before the cursor is moved after end of selection.
        if( m_pWrtShell->IsAddMode() && m_pWrtShell->SwCursorShell::HasSelection() )
            m_pWrtShell->CreateCursor();

        SwDoc *const pTmpDoc = lcl_GetDoc(*m_pClpDocFac);

        pTmpDoc->getIDocumentFieldsAccess().LockExpFields();     // Never update fields - leave text as is
        lclOverWriteDoc(*m_pWrtShell, *pTmpDoc);

        {
            IDocumentMarkAccess* const pMarkAccess = pTmpDoc->getIDocumentMarkAccess();
            ::std::vector< ::sw::mark::IMark* > vDdeMarks;
            // find all DDE-Bookmarks
            for(IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->getAllMarksBegin();
                ppMark != pMarkAccess->getAllMarksEnd();
                ++ppMark)
            {
                if(IDocumentMarkAccess::MarkType::DDE_BOOKMARK == IDocumentMarkAccess::GetType(**ppMark))
                    vDdeMarks.push_back(ppMark->get());
            }
            // remove all DDE-Bookmarks, they are invalid inside the clipdoc!
            for(::std::vector< ::sw::mark::IMark* >::iterator ppMark = vDdeMarks.begin();
                ppMark != vDdeMarks.end();
                ++ppMark)
                pMarkAccess->deleteMark(*ppMark);
        }

        // a new one was created in CORE (OLE objects copied!)
        m_aDocShellRef = pTmpDoc->GetTmpDocShell();
        if( m_aDocShellRef.Is() )
            SwTransferable::InitOle( m_aDocShellRef, *pTmpDoc );
        pTmpDoc->SetTmpDocShell( nullptr );

        if( m_pWrtShell->IsObjSelected() )
            m_eBufferType = TRNSFR_DRAWING;
        else
        {
            m_eBufferType = TRNSFR_DOCUMENT;
            if (m_pWrtShell->IntelligentCut(nSelection, false) != SwWrtShell::NO_WORD)
                m_eBufferType = (TransferBufferType)(TRNSFR_DOCUMENT_WORD | m_eBufferType);
        }

        bool bDDELink = m_pWrtShell->IsSelection();
        if( nSelection & nsSelectionType::SEL_TBL_CELLS )
        {
            m_eBufferType = (TransferBufferType)(TRNSFR_TABELLE | m_eBufferType);
            bDDELink = m_pWrtShell->HasWholeTabSelection();
        }

        //When someone needs it, we 'OLE' him something
        AddFormat( SotClipboardFormatId::EMBED_SOURCE );

        //put RTF ahead of  the OLE's Metafile to have less loss
        if( !m_pWrtShell->IsObjSelected() )
        {
            AddFormat( SotClipboardFormatId::RTF );
            AddFormat( SotClipboardFormatId::HTML );
        }
        if( m_pWrtShell->IsSelection() )
            AddFormat( SotClipboardFormatId::STRING );

        if( nSelection & ( nsSelectionType::SEL_DRW | nsSelectionType::SEL_DRW_FORM ))
        {
            AddFormat( SotClipboardFormatId::DRAWING );
            if ( nSelection & nsSelectionType::SEL_DRW )
            {
                AddFormat( SotClipboardFormatId::GDIMETAFILE );
                AddFormat( SotClipboardFormatId::PNG );
                AddFormat( SotClipboardFormatId::BITMAP );
            }
            m_eBufferType = (TransferBufferType)( TRNSFR_GRAPHIC | m_eBufferType );

            m_pClpGraphic = new Graphic;
            if( !m_pWrtShell->GetDrawObjGraphic( SotClipboardFormatId::GDIMETAFILE, *m_pClpGraphic ))
                m_pOrigGraphic = m_pClpGraphic;
            m_pClpBitmap = new Graphic;
            if( !m_pWrtShell->GetDrawObjGraphic( SotClipboardFormatId::BITMAP, *m_pClpBitmap ))
                m_pOrigGraphic = m_pClpBitmap;

            // is it an URL-Button ?
            OUString sURL;
            OUString sDesc;
            if( m_pWrtShell->GetURLFromButton( sURL, sDesc ) )
            {
                AddFormat( SotClipboardFormatId::STRING );
                AddFormat( SotClipboardFormatId::SOLK );
                AddFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK );
                AddFormat( SotClipboardFormatId::FILECONTENT );
                AddFormat( SotClipboardFormatId::FILEGRPDESCRIPTOR );
                AddFormat( SotClipboardFormatId::UNIFORMRESOURCELOCATOR );
                m_eBufferType = (TransferBufferType)( TRNSFR_INETFLD | m_eBufferType );
                nRet = 1;
            }
        }

        // at Cut, DDE-Link doesn't make sense!!
        SwDocShell* pDShell;
        if( !bIsCut && bDDELink &&
            nullptr != ( pDShell = m_pWrtShell->GetDoc()->GetDocShell()) &&
            SfxObjectCreateMode::STANDARD == pDShell->GetCreateMode() )
        {
            AddFormat( SotClipboardFormatId::LINK );
            m_xDdeLink = new SwTrnsfrDdeLink( *this, *m_pWrtShell );
        }

        //ObjectDescriptor was already filly from the old DocShell.
        //Now adjust it. Thus in GetData the first query can still
        //be answered with delayed rendering.
        m_aObjDesc.mbCanLink = false;
        Size aSz( OLESIZE );
        m_aObjDesc.maSize = OutputDevice::LogicToLogic( aSz, MAP_TWIP, MAP_100TH_MM );

        PrepareOLE( m_aObjDesc );
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
    }
    else
        nRet = 0;

    if( m_pWrtShell->IsFrameSelected() )
    {
        SfxItemSet aSet( m_pWrtShell->GetAttrPool(), RES_URL, RES_URL );
        m_pWrtShell->GetFlyFrameAttr( aSet );
        const SwFormatURL& rURL = static_cast<const SwFormatURL&>(aSet.Get( RES_URL ));
        if( rURL.GetMap() )
        {
            m_pImageMap = new ImageMap( *rURL.GetMap() );
            AddFormat( SotClipboardFormatId::SVIM );
        }
        else if( !rURL.GetURL().isEmpty() )
        {
            m_pTargetURL = new INetImage( sGrfNm, rURL.GetURL(),
                                        rURL.GetTargetFrameName(),
                                        aEmptyOUStr, Size() );
            AddFormat( SotClipboardFormatId::INET_IMAGE );
        }
    }

    return nRet;
}

int SwTransferable::Copy( bool bIsCut )
{
    int nRet = PrepareForCopy( bIsCut );
    if ( nRet )
    {
        CopyToClipboard( &m_pWrtShell->GetView().GetEditWin() );
    }
    return nRet;
}

int SwTransferable::CalculateAndCopy()
{
    if(!m_pWrtShell)
        return 0;
    SwWait aWait( *m_pWrtShell->GetView().GetDocShell(), true );

    OUString aStr( m_pWrtShell->Calculate() );

    m_pClpDocFac = new SwDocFac;
    SwDoc *const pDoc = lcl_GetDoc(*m_pClpDocFac);
    m_pWrtShell->Copy(pDoc, & aStr);
    m_eBufferType = TRNSFR_DOCUMENT;
    AddFormat( SotClipboardFormatId::STRING );

    CopyToClipboard( &m_pWrtShell->GetView().GetEditWin() );

    return 1;
}

int SwTransferable::CopyGlossary( SwTextBlocks& rGlossary, const OUString& rStr )
{
    if(!m_pWrtShell)
        return 0;
    SwWait aWait( *m_pWrtShell->GetView().GetDocShell(), true );

    m_pClpDocFac = new SwDocFac;
    SwDoc *const pCDoc = lcl_GetDoc(*m_pClpDocFac);

    SwNodes& rNds = pCDoc->GetNodes();
    SwNodeIndex aNodeIdx( *rNds.GetEndOfContent().StartOfSectionNode() );
    SwContentNode* pCNd = rNds.GoNext( &aNodeIdx ); // go to 1st ContentNode
    SwPaM aPam( *pCNd );

    pCDoc->getIDocumentFieldsAccess().LockExpFields();   // never update fields - leave text as it is

    pCDoc->InsertGlossary( rGlossary, rStr, aPam );

    // a new one was created in CORE (OLE-Objects copied!)
    m_aDocShellRef = pCDoc->GetTmpDocShell();
    if( m_aDocShellRef.Is() )
        SwTransferable::InitOle( m_aDocShellRef, *pCDoc );
    pCDoc->SetTmpDocShell( nullptr );

    m_eBufferType = TRNSFR_DOCUMENT;

    //When someone needs it, we 'OLE' her something.
    AddFormat( SotClipboardFormatId::EMBED_SOURCE );
    AddFormat( SotClipboardFormatId::RTF );
    AddFormat( SotClipboardFormatId::HTML );
    AddFormat( SotClipboardFormatId::STRING );

    //ObjectDescriptor was already filled from the old DocShell.
    //Now adjust it. Thus in GetData the first query can still
    //be answered with delayed rendering.
    m_aObjDesc.mbCanLink = false;
    Size aSz( OLESIZE );
    m_aObjDesc.maSize = OutputDevice::LogicToLogic( aSz, MAP_TWIP, MAP_100TH_MM );

    PrepareOLE( m_aObjDesc );
    AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );

    CopyToClipboard( &m_pWrtShell->GetView().GetEditWin() );

    return 1;
}

static inline uno::Reference < XTransferable > * lcl_getTransferPointer ( uno::Reference < XTransferable > &xRef )
{
    return &xRef;
}

bool SwTransferable::IsPaste( const SwWrtShell& rSh,
                              const TransferableDataHelper& rData )
{
    // Check the common case first: We can always paste our own data!
    // If _only_ the internal format can be pasted, this check will
    // yield 'true', while the one below would give a (wrong) result 'false'.

    bool bIsPaste = ( GetSwTransferable( rData ) != nullptr );

    // if it's not our own data, we need to have a closer look:
    if( ! bIsPaste )
    {
        // determine the proper paste action, and return true if we find one
        uno::Reference<XTransferable> xTransferable( rData.GetXTransferable() );

        SotExchangeDest nDestination = SwTransferable::GetSotDestination( rSh );
        sal_uInt16 nSourceOptions =
                    (( SotExchangeDest::DOC_TEXTFRAME == nDestination ||
                       SotExchangeDest::SWDOC_FREE_AREA == nDestination ||
                       SotExchangeDest::DOC_TEXTFRAME_WEB == nDestination ||
                       SotExchangeDest::SWDOC_FREE_AREA_WEB == nDestination )
                                    ? EXCHG_IN_ACTION_COPY
                     : EXCHG_IN_ACTION_MOVE);

        SotClipboardFormatId nFormat;          // output param for GetExchangeAction
        sal_uInt16 nEventAction;    // output param for GetExchangeAction
        sal_uInt16 nAction = SotExchange::GetExchangeAction(
                                rData.GetDataFlavorExVector(),
                                nDestination,
                                nSourceOptions,             /* ?? */
                                EXCHG_IN_ACTION_DEFAULT,    /* ?? */
                                nFormat, nEventAction, SotClipboardFormatId::NONE,
                                lcl_getTransferPointer ( xTransferable ) );

        // if we find a suitable action, we can paste!
        bIsPaste = (EXCHG_INOUT_ACTION_NONE != nAction);
    }

    return bIsPaste;
}

bool SwTransferable::Paste(SwWrtShell& rSh, TransferableDataHelper& rData, sal_uInt16 nAnchorType)
{
    sal_uInt16 nEventAction, nAction=0;
    SotExchangeDest nDestination = SwTransferable::GetSotDestination( rSh );
    SotClipboardFormatId nFormat = SotClipboardFormatId::NONE;

    if( GetSwTransferable( rData ) )
    {
        nAction = EXCHG_OUT_ACTION_INSERT_PRIVATE;
    }
    else
    {
        sal_uInt16 nSourceOptions =
                    (( SotExchangeDest::DOC_TEXTFRAME == nDestination ||
                    SotExchangeDest::SWDOC_FREE_AREA == nDestination ||
                    SotExchangeDest::DOC_TEXTFRAME_WEB == nDestination ||
                    SotExchangeDest::SWDOC_FREE_AREA_WEB == nDestination )
                                    ? EXCHG_IN_ACTION_COPY
                                    : EXCHG_IN_ACTION_MOVE);
        uno::Reference<XTransferable> xTransferable( rData.GetXTransferable() );
        nAction = SotExchange::GetExchangeAction(
                                    rData.GetDataFlavorExVector(),
                                    nDestination,
                                    nSourceOptions,             /* ?? */
                                    EXCHG_IN_ACTION_DEFAULT,    /* ?? */
                                    nFormat, nEventAction, SotClipboardFormatId::NONE,
                                    lcl_getTransferPointer ( xTransferable ) );
    }

    // special case for tables from draw application
    if( EXCHG_OUT_ACTION_INSERT_DRAWOBJ == (nAction & EXCHG_ACTION_MASK) )
    {
        if( rData.HasFormat( SotClipboardFormatId::RTF ) )
        {
            nAction = EXCHG_OUT_ACTION_INSERT_STRING | (nAction & ~EXCHG_ACTION_MASK);
            nFormat = SotClipboardFormatId::RTF;
        }
    }

    return EXCHG_INOUT_ACTION_NONE != nAction &&
            SwTransferable::PasteData( rData, rSh, nAction, nFormat,
                                        nDestination, false, false, nullptr, 0, false, nAnchorType );
}

bool SwTransferable::PasteData( TransferableDataHelper& rData,
                            SwWrtShell& rSh, sal_uInt16 nAction, SotClipboardFormatId nFormat,
                            SotExchangeDest nDestination, bool bIsPasteFormat,
                            bool bIsDefault,
                            const Point* pPt, sal_Int8 nDropAction,
                            bool bPasteSelection, sal_uInt16 nAnchorType )
{
    SwWait aWait( *rSh.GetView().GetDocShell(), false );
    std::unique_ptr<SwTrnsfrActionAndUndo> pAction;
    SwModule* pMod = SW_MOD();

    bool bRet = false;
    bool bCallAutoCaption = false;

    if( pPt )
    {
        // external Drop
        if( bPasteSelection ? !pMod->m_pXSelection : !pMod->m_pDragDrop )
        {
            switch( nDestination )
            {
            case SotExchangeDest::DOC_LNKD_GRAPH_W_IMAP:
            case SotExchangeDest::DOC_LNKD_GRAPHOBJ:
            case SotExchangeDest::DOC_GRAPH_W_IMAP:
            case SotExchangeDest::DOC_GRAPHOBJ:
            case SotExchangeDest::DOC_OLEOBJ:
            case SotExchangeDest::DOC_DRAWOBJ:
            case SotExchangeDest::DOC_URLBUTTON:
            case SotExchangeDest::DOC_GROUPOBJ:
                // select frames/objects
                SwTransferable::SetSelInShell( rSh, true, pPt );
                break;

            default:
                SwTransferable::SetSelInShell( rSh, false, pPt );
                break;
            }
        }
    }
    else if( ( !GetSwTransferable( rData ) || bIsPasteFormat ) &&
            !rSh.IsTableMode() && rSh.HasSelection() )
    {
        // then delete the selections

        //don't delete selected content
        // - at table-selection
        // - at ReRead of a graphic/DDEData
        // - at D&D, for the right selection was taken care of
        //      in Drop-Handler
        bool bDelSel = false;
        switch( nDestination )
        {
        case SotExchangeDest::DOC_TEXTFRAME:
        case SotExchangeDest::SWDOC_FREE_AREA:
        case SotExchangeDest::DOC_TEXTFRAME_WEB:
        case SotExchangeDest::SWDOC_FREE_AREA_WEB:
            bDelSel = true;
            break;
        default:
            break;
        }

        if( bDelSel )
            // #i34830#
            pAction.reset(new SwTrnsfrActionAndUndo( &rSh, UNDO_PASTE_CLIPBOARD, nullptr,
                                                     true ));
    }

    SwTransferable *pTrans=nullptr, *pTunneledTrans=GetSwTransferable( rData );

    // check for private drop
    bool bPrivateDrop(pPt && (bPasteSelection ? nullptr != (pTrans = pMod->m_pXSelection) : nullptr != (pTrans = pMod->m_pDragDrop)));
    bool bNeedToSelectBeforePaste(false);

    if(bPrivateDrop && DND_ACTION_LINK == nDropAction)
    {
        // internal drop on object, suppress bPrivateDrop to change internal fill
        bPrivateDrop = false;
        bNeedToSelectBeforePaste = true;
    }

    if(bPrivateDrop && pPt && DND_ACTION_MOVE == nDropAction)
    {
        // check if dragged over a useful target. If yes, use as content exchange
        // drop as if from external
        const SwFrameFormat* pSwFrameFormat = rSh.GetFormatFromObj(*pPt);

        if(pSwFrameFormat && dynamic_cast< const SwDrawFrameFormat* >(pSwFrameFormat) !=  nullptr)
        {
            bPrivateDrop = false;
            bNeedToSelectBeforePaste = true;
        }
    }

    if(bPrivateDrop)
    {
        // then internal Drag & Drop or XSelection
        bRet = pTrans->PrivateDrop( rSh, *pPt, DND_ACTION_MOVE == nDropAction,
                                    bPasteSelection );
    }
    else if( !pPt && pTunneledTrans &&
            EXCHG_OUT_ACTION_INSERT_PRIVATE == nAction )
    {
        // then internal paste
        bRet = pTunneledTrans->PrivatePaste( rSh );
    }
    else if( EXCHG_INOUT_ACTION_NONE != nAction )
    {
        if( !pAction )
        {
            pAction.reset(new SwTrnsfrActionAndUndo( &rSh, UNDO_PASTE_CLIPBOARD));
        }

        // in Drag&Drop MessageBoxes must not be showed
        bool bMsg = nullptr == pPt;
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
            case SotClipboardFormatId::DRAWING:
                bRet = SwTransferable::_PasteSdrFormat( rData, rSh,
                                                SwPasteSdr::Insert, pPt,
                                                nActionFlags, bNeedToSelectBeforePaste);
                break;

            case SotClipboardFormatId::HTML:
            case SotClipboardFormatId::HTML_SIMPLE:
            case SotClipboardFormatId::HTML_NO_COMMENT:
            case SotClipboardFormatId::RTF:
            case SotClipboardFormatId::STRING:
                bRet = SwTransferable::_PasteFileContent( rData, rSh,
                                                            nFormat, bMsg );
                break;

            case SotClipboardFormatId::NETSCAPE_BOOKMARK:
                {
                    INetBookmark aBkmk;
                    if( rData.GetINetBookmark( nFormat, aBkmk ) )
                    {
                        SwFormatINetFormat aFormat( aBkmk.GetURL(), OUString() );
                        rSh.InsertURL( aFormat, aBkmk.GetDescription() );
                        bRet = true;
                    }
                }
                break;

            case SotClipboardFormatId::SD_OLE:
                bRet = SwTransferable::_PasteOLE( rData, rSh, nFormat,
                                                    nActionFlags, bMsg );
                break;

            case SotClipboardFormatId::SVIM:
                bRet = SwTransferable::_PasteImageMap( rData, rSh );
                break;

            case SotClipboardFormatId::SVXB:
            case SotClipboardFormatId::BITMAP:
            case SotClipboardFormatId::PNG:
            case SotClipboardFormatId::GDIMETAFILE:
                bRet = SwTransferable::_PasteGrf( rData, rSh, nFormat,
                                                SwPasteSdr::Insert,pPt,
                                                nActionFlags, nDropAction, bNeedToSelectBeforePaste);
                break;

            case SotClipboardFormatId::XFORMS:
            case SotClipboardFormatId::SBA_FIELDDATAEXCHANGE:
            case SotClipboardFormatId::SBA_DATAEXCHANGE:
            case SotClipboardFormatId::SBA_CTRLDATAEXCHANGE:
                bRet = SwTransferable::_PasteDBData( rData, rSh, nFormat,
                                            EXCHG_IN_ACTION_LINK == nClearedAction,
                                            pPt, bMsg );
                break;

            case SotClipboardFormatId::SIMPLE_FILE:
                bRet = SwTransferable::_PasteFileName( rData, rSh, nFormat,
                                ( EXCHG_IN_ACTION_MOVE == nClearedAction
                                    ? SwPasteSdr::Replace
                                    : EXCHG_IN_ACTION_LINK == nClearedAction
                                        ? SwPasteSdr::SetAttr
                                        : SwPasteSdr::Insert),
                                pPt, nActionFlags, bMsg, nullptr );
                break;

            case SotClipboardFormatId::FILE_LIST:
                // then insert as graphics only
                bRet = SwTransferable::_PasteFileList( rData, rSh,
                                    EXCHG_IN_ACTION_LINK == nClearedAction,
                                    pPt, bMsg );
                break;

            case SotClipboardFormatId::SONLK:
                if( pPt )
                {
                    NaviContentBookmark aBkmk;
                    if( aBkmk.Paste( rData ) )
                    {
                        if(bIsDefault)
                        {
                            switch(aBkmk.GetDefaultDragType())
                            {
                                case RegionMode::NONE: nClearedAction = EXCHG_IN_ACTION_COPY; break;
                                case RegionMode::EMBEDDED: nClearedAction = EXCHG_IN_ACTION_MOVE; break;
                                case RegionMode::LINK: nClearedAction = EXCHG_IN_ACTION_LINK; break;
                            }
                        }
                        rSh.NavigatorPaste( aBkmk, nClearedAction );
                        bRet = true;
                    }
                }
                break;

            case SotClipboardFormatId::INET_IMAGE:
            case SotClipboardFormatId::NETSCAPE_IMAGE:
                bRet = SwTransferable::_PasteTargetURL( rData, rSh,
                                                        SwPasteSdr::Insert,
                                                        pPt, true );
                break;

            default:
                OSL_ENSURE( pPt, "unknown format" );
            }
            break;

        case EXCHG_OUT_ACTION_INSERT_FILE:
            {
                bool graphicInserted;
                bRet = SwTransferable::_PasteFileName( rData, rSh, nFormat,
                                            SwPasteSdr::Insert, pPt,
                                            nActionFlags, bMsg,
                                            &graphicInserted );
                if( graphicInserted )
                    bCallAutoCaption = true;
            }
            break;

        case EXCHG_OUT_ACTION_INSERT_OLE:
            bRet = SwTransferable::_PasteOLE( rData, rSh, nFormat,
                                                nActionFlags,bMsg );
            break;

        case EXCHG_OUT_ACTION_INSERT_DDE:
            {
                bool bReRead = 0 != CNT_HasGrf( rSh.GetCntType() );
                bRet = SwTransferable::_PasteDDE( rData, rSh, bReRead, bMsg );
            }
            break;

        case EXCHG_OUT_ACTION_INSERT_HYPERLINK:
            {
                OUString sURL, sDesc;
                if( SotClipboardFormatId::SIMPLE_FILE == nFormat )
                {
                    if( rData.GetString( nFormat, sURL ) && !sURL.isEmpty() )
                    {
                        SwTransferable::_CheckForURLOrLNKFile( rData, sURL, &sDesc );
                        if( sDesc.isEmpty() )
                            sDesc = sURL;
                        bRet = true;
                    }
                }
                else
                {
                    INetBookmark aBkmk;
                    if( rData.GetINetBookmark( nFormat, aBkmk ) )
                    {
                        sURL = aBkmk.GetURL();
                        sDesc = aBkmk.GetDescription();
                        bRet = true;
                    }
                }

                if( bRet )
                {
                    SwFormatINetFormat aFormat( sURL, OUString() );
                    rSh.InsertURL( aFormat, sDesc );
                }
            }
            break;

        case EXCHG_OUT_ACTION_GET_ATTRIBUTES:
            switch( nFormat )
            {
            case SotClipboardFormatId::DRAWING:
                bRet = SwTransferable::_PasteSdrFormat( rData, rSh,
                                                SwPasteSdr::SetAttr, pPt,
                                                nActionFlags, bNeedToSelectBeforePaste);
                break;
            case SotClipboardFormatId::SVXB:
            case SotClipboardFormatId::GDIMETAFILE:
            case SotClipboardFormatId::BITMAP:
            case SotClipboardFormatId::PNG:
            case SotClipboardFormatId::NETSCAPE_BOOKMARK:
            case SotClipboardFormatId::SIMPLE_FILE:
            case SotClipboardFormatId::FILEGRPDESCRIPTOR:
            case SotClipboardFormatId::UNIFORMRESOURCELOCATOR:
                bRet = SwTransferable::_PasteGrf( rData, rSh, nFormat,
                                                SwPasteSdr::SetAttr, pPt,
                                                nActionFlags, nDropAction, bNeedToSelectBeforePaste);
                break;
            default:
                OSL_FAIL( "unknown format" );
            }

            break;

        case EXCHG_OUT_ACTION_INSERT_DRAWOBJ:
            bRet = SwTransferable::_PasteSdrFormat( rData, rSh,
                                                SwPasteSdr::Insert, pPt,
                                                nActionFlags, bNeedToSelectBeforePaste);
            break;
        case EXCHG_OUT_ACTION_INSERT_SVXB:
        case EXCHG_OUT_ACTION_INSERT_GDIMETAFILE:
        case EXCHG_OUT_ACTION_INSERT_BITMAP:
        case EXCHG_OUT_ACTION_INSERT_GRAPH:
            bRet = SwTransferable::_PasteGrf( rData, rSh, nFormat,
                                                SwPasteSdr::Insert, pPt,
                                                nActionFlags, nDropAction, bNeedToSelectBeforePaste, nAnchorType );
            break;

        case EXCHG_OUT_ACTION_REPLACE_DRAWOBJ:
            bRet = SwTransferable::_PasteSdrFormat( rData, rSh,
                                                SwPasteSdr::Replace, pPt,
                                                nActionFlags, bNeedToSelectBeforePaste);
            break;

        case EXCHG_OUT_ACTION_REPLACE_SVXB:
        case EXCHG_OUT_ACTION_REPLACE_GDIMETAFILE:
        case EXCHG_OUT_ACTION_REPLACE_BITMAP:
        case EXCHG_OUT_ACTION_REPLACE_GRAPH:
            bRet = SwTransferable::_PasteGrf( rData, rSh, nFormat,
                                                SwPasteSdr::Replace,pPt,
                                                nActionFlags, nDropAction, bNeedToSelectBeforePaste);
            break;

        case EXCHG_OUT_ACTION_INSERT_INTERACTIVE:
            bRet = SwTransferable::_PasteAsHyperlink( rData, rSh, nFormat );
            break;

        default:
            OSL_FAIL("unknown action" );
        }
    }

    if( !bPasteSelection && rSh.IsFrameSelected() )
    {
        rSh.EnterSelFrameMode();
        //force ::SelectShell
        rSh.GetView().StopShellTimer();
    }

    pAction.reset();
    if( bCallAutoCaption )
        rSh.GetView().AutoCaption( GRAPHIC_CAP );

    return bRet;
}

SotExchangeDest SwTransferable::GetSotDestination( const SwWrtShell& rSh,
                                            const Point* pPt )
{
    SotExchangeDest nRet = SotExchangeDest::NONE;

    ObjCntType eOType;
    if( pPt )
    {
        SdrObject *pObj = nullptr;
        eOType = rSh.GetObjCntType( *pPt, pObj );
    }
    else
        eOType = rSh.GetObjCntTypeOfSelection();

    switch( eOType )
    {
    case OBJCNT_GRF:
        {
            bool bIMap, bLink;
            if( pPt )
            {
                bIMap = nullptr != rSh.GetFormatFromObj( *pPt )->GetURL().GetMap();
                OUString aDummy;
                rSh.GetGrfAtPos( *pPt, aDummy, bLink );
            }
            else
            {
                bIMap = nullptr != rSh.GetFlyFrameFormat()->GetURL().GetMap();
                OUString aDummy;
                rSh.GetGrfNms( &aDummy, nullptr );
                bLink = !aDummy.isEmpty();
            }

            if( bLink && bIMap )
                nRet = SotExchangeDest::DOC_LNKD_GRAPH_W_IMAP;
            else if( bLink )
                nRet = SotExchangeDest::DOC_LNKD_GRAPHOBJ;
            else if( bIMap )
                nRet = SotExchangeDest::DOC_GRAPH_W_IMAP;
            else
                nRet = SotExchangeDest::DOC_GRAPHOBJ;
        }
        break;

    case OBJCNT_FLY:
        if( dynamic_cast< const SwWebDocShell *>( rSh.GetView().GetDocShell() ) != nullptr  )
            nRet = SotExchangeDest::DOC_TEXTFRAME_WEB;
        else
            nRet = SotExchangeDest::DOC_TEXTFRAME;
        break;
    case OBJCNT_OLE:        nRet = SotExchangeDest::DOC_OLEOBJ;       break;

    case OBJCNT_CONTROL:    /* no Action avail */
    case OBJCNT_SIMPLE:     nRet = SotExchangeDest::DOC_DRAWOBJ;      break;
    case OBJCNT_URLBUTTON:  nRet = SotExchangeDest::DOC_URLBUTTON;    break;
    case OBJCNT_GROUPOBJ:   nRet = SotExchangeDest::DOC_GROUPOBJ;     break;

    // what do we do at multiple selections???
    default:
        {
            if( dynamic_cast< const SwWebDocShell *>( rSh.GetView().GetDocShell() ) != nullptr  )
                nRet = SotExchangeDest::SWDOC_FREE_AREA_WEB;
            else
                nRet = SotExchangeDest::SWDOC_FREE_AREA;
        }
    }

    return nRet;
}

bool SwTransferable::_PasteFileContent( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, SotClipboardFormatId nFormat, bool bMsg )
{
    sal_uInt16 nResId = STR_CLPBRD_FORMAT_ERROR;
    bool bRet = false;

    MSE40HTMLClipFormatObj aMSE40ClpObj;

    tools::SvRef<SotStorageStream> xStrm;
    SvStream* pStream = nullptr;
    SwRead pRead = nullptr;
    OUString sData;
    switch( nFormat )
    {
    case SotClipboardFormatId::STRING:
        {
            pRead = ReadAscii;
            if( rData.GetString( nFormat, sData ) )
            {
                pStream = new SvMemoryStream( const_cast<sal_Unicode *>(sData.getStr()),
                            sData.getLength() * sizeof( sal_Unicode ),
                            StreamMode::READ );
#ifdef OSL_BIGENDIAN
                pStream->SetEndian( SvStreamEndian::BIG );
#else
                pStream->SetEndian( SvStreamEndian::LITTLE );
#endif

                SwAsciiOptions aAOpt;
                aAOpt.SetCharSet( RTL_TEXTENCODING_UCS2 );
                pRead->GetReaderOpt().SetASCIIOpts( aAOpt );
                break;
            }
        }
        // no break - because then test if we get a stream

    default:
        if( rData.GetSotStorageStream( nFormat, xStrm ) )
        {
            if( ( SotClipboardFormatId::HTML_SIMPLE == nFormat ) ||
                ( SotClipboardFormatId::HTML_NO_COMMENT == nFormat ) )
            {
                pStream = aMSE40ClpObj.IsValid( *xStrm );
                pRead = ReadHTML;
                pRead->SetReadUTF8( true );

                bool bNoComments =
                    ( nFormat == SotClipboardFormatId::HTML_NO_COMMENT );
                pRead->SetIgnoreHTMLComments( bNoComments );
            }
            else
            {
                pStream = &xStrm;
                if( SotClipboardFormatId::RTF == nFormat )
                    pRead = SwReaderWriter::GetReader( READER_WRITER_RTF );
                else if( !pRead )
                {
                    pRead = ReadHTML;
                    pRead->SetReadUTF8( true );
                }
            }
        }
        break;
    }

    if( pStream && pRead )
    {
        Link<SwCursorShell*,void> aOldLink( rSh.GetChgLnk() );
        rSh.SetChgLnk( Link<SwCursorShell*,void>() );

        const SwPosition& rInsPos = *rSh.GetCursor()->Start();
        SwReader aReader( *pStream, aEmptyOUStr, OUString(), *rSh.GetCursor() );
        rSh.SaveTableBoxContent( &rInsPos );
        if( IsError( aReader.Read( *pRead )) )
            nResId = STR_ERROR_CLPBRD_READ;
        else
        {
            nResId = 0;
            bRet = true;
        }

        rSh.SetChgLnk( aOldLink );
        if( bRet )
            rSh.CallChgLnk();
    }
    else
        nResId = STR_CLPBRD_FORMAT_ERROR;

    // Exist a SvMemoryStream? (data in the OUString and xStrm is empty)
    if( pStream && !xStrm.Is() )
        delete pStream;

    if( bMsg && nResId )
    {
        ScopedVclPtrInstance<MessageDialog>::Create( nullptr, SW_RES( nResId ), VCL_MESSAGE_INFO)->Execute();
    }
    return bRet;
}

bool SwTransferable::_PasteOLE( TransferableDataHelper& rData, SwWrtShell& rSh,
                                SotClipboardFormatId nFormat, sal_uInt8 nActionFlags, bool bMsg )
{
    bool bRet = false;
    TransferableObjectDescriptor aObjDesc;
    uno::Reference < io::XInputStream > xStrm;
    uno::Reference < embed::XStorage > xStore;
    Reader* pRead = nullptr;

    // Get the preferred format
    SotClipboardFormatId nId;
    if( rData.HasFormat( SotClipboardFormatId::EMBEDDED_OBJ ) )
        nId = SotClipboardFormatId::EMBEDDED_OBJ;
    else if( rData.HasFormat( SotClipboardFormatId::EMBED_SOURCE ) &&
             rData.HasFormat( SotClipboardFormatId::OBJECTDESCRIPTOR ))
        nId = SotClipboardFormatId::EMBED_SOURCE;
    else
        nId = SotClipboardFormatId::NONE;

    if (nId != SotClipboardFormatId::NONE)
    {
        SwDocShell* pDocSh = rSh.GetDoc()->GetDocShell();
        xStrm = rData.GetInputStream(nId, SfxObjectShell::CreateShellID(pDocSh));
    }

    if (xStrm.is())
    {
        // if there is an embedded object, first try if it's a writer object
        // this will be inserted into the document by using a Reader
        try
        {
            xStore = comphelper::OStorageHelper::GetStorageFromInputStream( xStrm );
            switch( SotStorage::GetFormatID( xStore ) )
            {
                case SotClipboardFormatId::STARWRITER_60:
                case SotClipboardFormatId::STARWRITERWEB_60:
                case SotClipboardFormatId::STARWRITERGLOB_60:
                case SotClipboardFormatId::STARWRITER_8:
                case SotClipboardFormatId::STARWRITERWEB_8:
                case SotClipboardFormatId::STARWRITERGLOB_8:
                    pRead = ReadXML;
                    break;
                default:
                    try
                    {
                        uno::Reference < lang::XComponent > xComp( xStore, uno::UNO_QUERY );
                        xComp->dispose();
                        xStore = nullptr;
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

        nFormat = nId;
    }

    if( pRead )
    {
        SwPaM &rPAM = *rSh.GetCursor();
        SwReader aReader( xStore, aEmptyOUStr, rPAM );
        if( !IsError( aReader.Read( *pRead )) )
            bRet = true;
        else if( bMsg )
            ScopedVclPtrInstance<MessageDialog>::Create( nullptr, SW_RES(STR_ERROR_CLPBRD_READ), VCL_MESSAGE_INFO )->Execute();
    }
    else
    {
        // temporary storage until the object is inserted
        uno::Reference< embed::XStorage > xTmpStor;
        uno::Reference < embed::XEmbeddedObject > xObj;
        OUString aName;
           comphelper::EmbeddedObjectContainer aCnt;

        if ( xStrm.is() )
        {
            if ( !rData.GetTransferableObjectDescriptor( SotClipboardFormatId::OBJECTDESCRIPTOR, aObjDesc ) )
            {
                OSL_ENSURE( !xStrm.is(), "An object without descriptor in clipboard!");
            }
        }
        else
        {
            if( rData.HasFormat( nFormat = SotClipboardFormatId::OBJECTDESCRIPTOR_OLE ) && rData.GetTransferableObjectDescriptor( nFormat, aObjDesc ) )
             {
                xStrm = rData.GetInputStream(SotClipboardFormatId::EMBED_SOURCE_OLE, OUString());
                if (!xStrm.is())
                    xStrm = rData.GetInputStream(SotClipboardFormatId::EMBEDDED_OBJ_OLE, OUString());

                if ( !xStrm.is() )
                {
                    // This is MSOLE object that should be created by direct using of system clipboard
                    try
                    {
                        xTmpStor = ::comphelper::OStorageHelper::GetTemporaryStorage();
                        uno::Reference < embed::XEmbedObjectClipboardCreator > xClipboardCreator =
                            embed::MSOLEObjectSystemCreator::create( ::comphelper::getProcessComponentContext() );

                        embed::InsertedObjectInfo aInfo = xClipboardCreator->createInstanceInitFromClipboard(
                                                            xTmpStor,
                                                            "DummyName",
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
            SotClipboardFormatId nGrFormat = SotClipboardFormatId::NONE;

            // limit the size of the preview metafile to 100000 actions
            GDIMetaFile aMetafile;
            if (rData.GetGDIMetaFile(SotClipboardFormatId::GDIMETAFILE, aMetafile, 100000))
            {
                nGrFormat = SotClipboardFormatId::GDIMETAFILE;
                aGraphic = aMetafile;
            }

            // insert replacement image ( if there is one ) into the object helper
            if ( nGrFormat != SotClipboardFormatId::NONE )
            {
                DataFlavor aDataFlavor;
                SotExchange::GetFormatDataFlavor( nGrFormat, aDataFlavor );
                xObjRef.SetGraphic( aGraphic, aDataFlavor.MimeType );
            }
            else if ( aObjDesc.mnViewAspect == embed::Aspects::MSOLE_ICON )
            {
                // it is important to have an icon, let an empty graphic be used
                // if no other graphic is provided
                // TODO/LATER: in future a default bitmap could be used
                OUString aMimeType;
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
            bRet = true;

            if( bRet && ( nActionFlags &
                ( EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL >> 8) ))
                SwTransferable::_PasteTargetURL( rData, rSh, SwPasteSdr::NONE, nullptr, false );

            // let the object be unloaded if possible
            SwOLEObj::UnloadObject( xObj, rSh.GetDoc(), embed::Aspects::MSOLE_CONTENT );
        }
    }
    return bRet;
}

bool SwTransferable::_PasteTargetURL( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, SwPasteSdr nAction,
                                    const Point* pPt, bool bInsertGRF )
{
    bool bRet = false;
    INetImage aINetImg;
    if( ( rData.HasFormat( SotClipboardFormatId::INET_IMAGE ) &&
          rData.GetINetImage( SotClipboardFormatId::INET_IMAGE, aINetImg )) ||
        ( rData.HasFormat( SotClipboardFormatId::NETSCAPE_IMAGE ) &&
          rData.GetINetImage( SotClipboardFormatId::NETSCAPE_IMAGE, aINetImg )) )
    {
        if( !aINetImg.GetImageURL().isEmpty() && bInsertGRF )
        {
            OUString sURL( aINetImg.GetImageURL() );
            SwTransferable::_CheckForURLOrLNKFile( rData, sURL );

            //!!! check at FileSystem - only then it make sense to test graphics !!!
            Graphic aGraphic;
            GraphicFilter &rFlt = GraphicFilter::GetGraphicFilter();
            bRet = GRFILTER_OK == GraphicFilter::LoadGraphic( sURL, aEmptyOUStr, aGraphic, &rFlt );

            if( bRet )
            {
                //Check and Perform rotation if needed
                lclCheckAndPerformRotation(aGraphic);

                switch( nAction )
                {
                case SwPasteSdr::Insert:
                    SwTransferable::SetSelInShell( rSh, false, pPt );
                    rSh.Insert( sURL, aEmptyOUStr, aGraphic );
                    break;

                case SwPasteSdr::Replace:
                    if( rSh.IsObjSelected() )
                    {
                        rSh.ReplaceSdrObj( sURL, aEmptyOUStr, &aGraphic );
                        Point aPt( pPt ? *pPt : rSh.GetCursorDocPos() );
                        SwTransferable::SetSelInShell( rSh, true, &aPt );
                    }
                    else
                        rSh.ReRead( sURL, aEmptyOUStr, &aGraphic );
                    break;

                case SwPasteSdr::SetAttr:
                    if( rSh.IsObjSelected() )
                        rSh.Paste( aGraphic, OUString() );
                    else if( OBJCNT_GRF == rSh.GetObjCntTypeOfSelection() )
                        rSh.ReRead( sURL, aEmptyOUStr, &aGraphic );
                    else
                    {
                        SwTransferable::SetSelInShell( rSh, false, pPt );
                        rSh.Insert( sURL, aEmptyOUStr, aGraphic );
                    }
                    break;
                default:
                    bRet = false;
                }
            }
        }
        else
            bRet = true;
    }

    if( bRet )
    {
        SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
        rSh.GetFlyFrameAttr( aSet );
        SwFormatURL aURL( static_cast<const SwFormatURL&>(aSet.Get( RES_URL )) );

        if( aURL.GetURL() != aINetImg.GetTargetURL() ||
            aURL.GetTargetFrameName() != aINetImg.GetTargetFrame() )
        {
            aURL.SetURL( aINetImg.GetTargetURL(), false );
            aURL.SetTargetFrameName( aINetImg.GetTargetFrame() );
            aSet.Put( aURL );
            rSh.SetFlyFrameAttr( aSet );
        }
    }
    return bRet;
}

void SwTransferable::SetSelInShell( SwWrtShell& rSh, bool bSelectFrame,
                                        const Point* pPt )
{
    if( bSelectFrame )
    {
        // select frames/objects
        if( pPt && !rSh.GetView().GetViewFrame()->GetDispatcher()->IsLocked() )
        {
            rSh.GetView().NoRotate();
            if( rSh.SelectObj( *pPt ))
            {
                rSh.HideCursor();
                rSh.EnterSelFrameMode( pPt );
                g_bFrameDrag = true;
            }
        }
    }
    else
    {
        if( rSh.IsFrameSelected() || rSh.IsObjSelected() )
        {
            rSh.UnSelectFrame();
            rSh.LeaveSelFrameMode();
            rSh.GetView().GetEditWin().StopInsFrame();
            g_bFrameDrag = false;
        }
        else if( rSh.GetView().GetDrawFuncPtr() )
            rSh.GetView().GetEditWin().StopInsFrame();

        rSh.EnterStdMode();
        if( pPt )
            rSh.SwCursorShell::SetCursor( *pPt, true );
    }
}

bool SwTransferable::_PasteDDE( TransferableDataHelper& rData,
                                SwWrtShell& rWrtShell, bool bReReadGrf,
                                bool bMsg )
{
    // data from Clipboardformat
    OUString aApp, aTopic, aItem;

    {
        tools::SvRef<SotStorageStream> xStrm;
        if( !rData.GetSotStorageStream( SotClipboardFormatId::LINK, xStrm ))
        {
            OSL_ENSURE( false, "DDE Data not found." );
            return false;
        }   // report useful error!!

        rtl_TextEncoding eEncoding = DDE_TXT_ENCODING;
        aApp = read_zeroTerminated_uInt8s_ToOUString(*xStrm, eEncoding);
        aTopic = read_zeroTerminated_uInt8s_ToOUString(*xStrm, eEncoding);
        aItem = read_zeroTerminated_uInt8s_ToOUString(*xStrm, eEncoding);
    }

    OUString aCmd;
    sfx2::MakeLnkName( aCmd, &aApp, aTopic, aItem );

    // do we want to read in a graphic now?
    SotClipboardFormatId nFormat;
    if( !rData.HasFormat( SotClipboardFormatId::RTF ) &&
        !rData.HasFormat( SotClipboardFormatId::HTML ) &&
        !rData.HasFormat( SotClipboardFormatId::STRING ) &&
        (rData.HasFormat( nFormat = SotClipboardFormatId::GDIMETAFILE ) ||
         rData.HasFormat( nFormat = SotClipboardFormatId::BITMAP )) )
    {
        Graphic aGrf;
        bool bRet = rData.GetGraphic( nFormat, aGrf );
        if( bRet )
        {
            OUString sLnkTyp("DDE");
            if ( bReReadGrf )
                rWrtShell.ReRead( aCmd, sLnkTyp, &aGrf );
            else
                rWrtShell.Insert( aCmd, sLnkTyp, aGrf );
        }
        return bRet;
    }

    SwFieldType* pTyp = nullptr;
    size_t i = 1;
    size_t j;
    OUString aName;
    bool bDoublePaste = false;
    const size_t nSize = rWrtShell.GetFieldTypeCount();
    const ::utl::TransliterationWrapper& rColl = ::GetAppCmpStrIgnore();

    do {
        aName = aApp;
        aName += OUString::number( i );
        for( j = INIT_FLDTYPES; j < nSize; j++ )
        {
            pTyp = rWrtShell.GetFieldType( j );
            if( RES_DDEFLD == pTyp->Which() )
            {
                OUString sTmp( static_cast<SwDDEFieldType*>(pTyp)->GetCmd() );
                if( rColl.isEqual( sTmp, aCmd ) &&
                    SfxLinkUpdateMode::ALWAYS == static_cast<SwDDEFieldType*>(pTyp)->GetType() )
                {
                    aName = pTyp->GetName();
                    bDoublePaste = true;
                    break;
                }
                else if( rColl.isEqual( aName, pTyp->GetName() ) )
                    break;
            }
        }
        if( j == nSize )
            break;
        ++i;
    }
    while( !bDoublePaste );

    if( !bDoublePaste )
    {
        SwDDEFieldType aType( aName, aCmd, SfxLinkUpdateMode::ALWAYS );
        pTyp = rWrtShell.InsertFieldType( aType );
    }

    SwDDEFieldType* pDDETyp = static_cast<SwDDEFieldType*>(pTyp);

    OUString aExpand;
    if( rData.GetString( SotClipboardFormatId::STRING, aExpand ))
    {
        do {            // middle checked loop

            // When data comes from a spreadsheet, we add a DDE-table
            if( ( rData.HasFormat( SotClipboardFormatId::SYLK ) ||
                  rData.HasFormat( SotClipboardFormatId::SYLK_BIGCAPS ) ) &&
                !aExpand.isEmpty() &&
                 ( 1 < comphelper::string::getTokenCount(aExpand, '\n') ||
                       comphelper::string::getTokenCount(aExpand, '\t') ) )
            {
                OUString sTmp( aExpand );
                sal_Int32 nRows = comphelper::string::getTokenCount(sTmp, '\n');
                if( nRows )
                    --nRows;
                sTmp = sTmp.getToken( 0, '\n' );
                sal_Int32 nCols = comphelper::string::getTokenCount(sTmp, '\t');

                // don't try to insert tables that are too large for writer
                if (nRows > USHRT_MAX || nCols > USHRT_MAX)
                {
                    if( bMsg )
                        ScopedVclPtrInstance<MessageDialog>::Create(nullptr, SW_RESSTR(STR_TABLE_TOO_LARGE), VCL_MESSAGE_INFO)->Execute();
                    pDDETyp = nullptr;
                    break;
                }

                // at least one column & row must be there
                if( !nRows || !nCols )
                {
                    if( bMsg )
                        ScopedVclPtrInstance<MessageDialog>::Create(nullptr, SW_RESSTR(STR_NO_TABLE), VCL_MESSAGE_INFO)->Execute();
                    pDDETyp = nullptr;
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

                pDDETyp = nullptr;                // remove FieldTypes again
            }
            else
            {
                // Einfuegen
                SwDDEField aSwDDEField( pDDETyp );
                rWrtShell.Insert( aSwDDEField );
            }

        } while( false );
    }
    else
        pDDETyp = nullptr;                        // remove FieldTypes again

    if( !pDDETyp && !bDoublePaste )
    {
        // remove FieldType again - error occurred!
        for( j = nSize; j >= INIT_FLDTYPES; --j )
            if( pTyp == rWrtShell.GetFieldType( j ) )
            {
                rWrtShell.RemoveFieldType( j );
                break;
            }
    }

    return true;
}

bool SwTransferable::_PasteSdrFormat(  TransferableDataHelper& rData,
                                    SwWrtShell& rSh, SwPasteSdr nAction,
                                    const Point* pPt, sal_uInt8 nActionFlags, bool bNeedToSelectBeforePaste)
{
    bool bRet = false;
    tools::SvRef<SotStorageStream> xStrm;
    if( rData.GetSotStorageStream( SotClipboardFormatId::DRAWING, xStrm ))
    {
        xStrm->SetVersion( SOFFICE_FILEFORMAT_50 );

        if(bNeedToSelectBeforePaste && pPt)
        {
            // if this is an internal drag, need to set the target right (select it), else
            // still the source will be selected
            SwTransferable::SetSelInShell( rSh, true, pPt );
        }

        rSh.Paste( *xStrm, nAction, pPt );
        bRet = true;

        if( bRet && ( nActionFlags &
            ( EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL >> 8) ))
            SwTransferable::_PasteTargetURL( rData, rSh, SwPasteSdr::NONE, nullptr, false );
    }
    return bRet;
}

bool SwTransferable::_PasteGrf( TransferableDataHelper& rData, SwWrtShell& rSh,
                                SotClipboardFormatId nFormat, SwPasteSdr nAction, const Point* pPt,
                                sal_uInt8 nActionFlags, sal_Int8 nDropAction, bool bNeedToSelectBeforePaste, sal_uInt16 nAnchorType )
{
    bool bRet = false;

    Graphic aGraphic;
    INetBookmark aBkmk;
    bool bCheckForGrf = false, bCheckForImageMap = false;

    switch( nFormat )
    {
    case SotClipboardFormatId::BITMAP:
    case SotClipboardFormatId::PNG:
    case SotClipboardFormatId::GDIMETAFILE:
        bRet = rData.GetGraphic( nFormat, aGraphic );
        break;

    case SotClipboardFormatId::SVXB:
    {
        tools::SvRef<SotStorageStream> xStm;

        if(rData.GetSotStorageStream(SotClipboardFormatId::SVXB, xStm))
        {
            ReadGraphic( *xStm, aGraphic );
            bRet = (GRAPHIC_NONE != aGraphic.GetType() && GRAPHIC_DEFAULT != aGraphic.GetType());
        }

        break;
    }

    case SotClipboardFormatId::NETSCAPE_BOOKMARK:
    case SotClipboardFormatId::FILEGRPDESCRIPTOR:
    case SotClipboardFormatId::UNIFORMRESOURCELOCATOR:
        if( ( bRet = rData.GetINetBookmark( nFormat, aBkmk ) ))
        {
            if( SwPasteSdr::SetAttr == nAction )
                nFormat = SotClipboardFormatId::NETSCAPE_BOOKMARK;
            else
                bCheckForGrf = true;
        }
        break;

    case SotClipboardFormatId::SIMPLE_FILE:
        {
            OUString sText;
            if( ( bRet = rData.GetString( nFormat, sText ) ) )
            {
                OUString sDesc;
                SwTransferable::_CheckForURLOrLNKFile( rData, sText, &sDesc );

                aBkmk = INetBookmark(
                        URIHelper::SmartRel2Abs(INetURLObject(), sText, Link<OUString *, bool>(), false ),
                        sDesc );
                bCheckForGrf = true;
                bCheckForImageMap = SwPasteSdr::Replace == nAction;
            }
        }
        break;

    default:
        bRet = rData.GetGraphic( nFormat, aGraphic );
        break;
    }

    if( bCheckForGrf )
    {
        //!!! check at FileSystem - only then it makes sense to test the graphics !!!
        GraphicFilter &rFlt = GraphicFilter::GetGraphicFilter();
        bRet = GRFILTER_OK == GraphicFilter::LoadGraphic( aBkmk.GetURL(), aEmptyOUStr,
                                            aGraphic, &rFlt );

        if( !bRet && SwPasteSdr::SetAttr == nAction &&
            SotClipboardFormatId::SIMPLE_FILE == nFormat &&
            // only at frame selection
            rSh.IsFrameSelected() )
        {
            // then set as hyperlink after the graphic
            nFormat = SotClipboardFormatId::NETSCAPE_BOOKMARK;
            bRet = true;
        }
    }

    if(pPt && bNeedToSelectBeforePaste)
    {
        // when using internal D&Ds, still the source object is selected and
        // this is necessary to get the correct source data which is also
        // dependent from selection. After receiving the drag data it is
        // now time to select the correct target object
        SwTransferable::SetSelInShell( rSh, true, pPt );
    }

    if( bRet )
    {
        //Check and Perform rotation if needed
        lclCheckAndPerformRotation(aGraphic);

        OUString sURL;
        if( dynamic_cast< const SwWebDocShell *>( rSh.GetView().GetDocShell() ) != nullptr
            // #i123922# if link action is noted, also take URL
            || DND_ACTION_LINK == nDropAction)
        {
            sURL = aBkmk.GetURL();
        }

        switch( nAction )
        {
            case SwPasteSdr::Insert:
            {
                SwTransferable::SetSelInShell( rSh, false, pPt );
                rSh.Insert( sURL, aEmptyOUStr, aGraphic, nullptr, false, nAnchorType );
                break;
            }

            case SwPasteSdr::Replace:
            {
                if( rSh.IsObjSelected() )
                {
                    // #i123922# for D&D on draw objects, do for now the same for
                    // SwPasteSdr::Replace (D&D) as for SwPasteSdr::SetAttr (D&D and
                    // CTRL+SHIFT). The code below replaces the draw object with
                    // a writer graphic; maybe this is an option later again if wanted
                    rSh.Paste( aGraphic, sURL );

                    // rSh.ReplaceSdrObj( sURL, aEmptyOUStr, &aGraphic );
                    // Point aPt( pPt ? *pPt : rSh.GetCursorDocPos() );
                    // SwTransferable::SetSelInShell( rSh, true, &aPt );
                }
                else
                {
                    // set graphic at writer graphic without link
                    rSh.ReRead( sURL, aEmptyOUStr, &aGraphic );
                }

                break;
            }

            case SwPasteSdr::SetAttr:
            {
                if( SotClipboardFormatId::NETSCAPE_BOOKMARK == nFormat )
                {
                    if( rSh.IsFrameSelected() )
                    {
                        SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                        rSh.GetFlyFrameAttr( aSet );
                        SwFormatURL aURL( static_cast<const SwFormatURL&>(aSet.Get( RES_URL )) );
                        aURL.SetURL( aBkmk.GetURL(), false );
                        aSet.Put( aURL );
                        rSh.SetFlyFrameAttr( aSet );
                    }
                }
                else if( rSh.IsObjSelected() )
                {
                    // set as attribute at DrawObject
                    rSh.Paste( aGraphic, sURL );
                }
                else if( OBJCNT_GRF == rSh.GetObjCntTypeOfSelection() )
                {
                    // set as linked graphic at writer graphic frame
                    rSh.ReRead( sURL, aEmptyOUStr, &aGraphic );
                }
                else
                {
                    SwTransferable::SetSelInShell( rSh, false, pPt );
                    rSh.Insert( aBkmk.GetURL(), aEmptyOUStr, aGraphic );
                }
                break;
            }
            default:
            {
                bRet = false;
                break;
            }
        }
    }

    if( bRet )
    {

        if( nActionFlags &
            (( EXCHG_OUT_ACTION_FLAG_INSERT_IMAGEMAP |
                EXCHG_OUT_ACTION_FLAG_REPLACE_IMAGEMAP ) >> 8) )
            SwTransferable::_PasteImageMap( rData, rSh );

        if( nActionFlags &
            ( EXCHG_OUT_ACTION_FLAG_INSERT_TARGETURL >> 8) )
            SwTransferable::_PasteTargetURL( rData, rSh, SwPasteSdr::NONE, nullptr, false );
    }
    else if( bCheckForImageMap )
    {
        // or should the file be an ImageMap-File?
        ImageMap aMap;
        SfxMedium aMed( INetURLObject(aBkmk.GetURL()).GetFull(),
                            STREAM_STD_READ );
        SvStream* pStream = aMed.GetInStream();
        if( pStream != nullptr  &&
            !pStream->GetError()  &&
            // mba: no BaseURL for clipboard functionality
            aMap.Read( *pStream, IMAP_FORMAT_DETECT, OUString() ) == IMAP_ERR_OK &&
            aMap.GetIMapObjectCount() )
        {
            SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
            rSh.GetFlyFrameAttr( aSet );
            SwFormatURL aURL( static_cast<const SwFormatURL&>(aSet.Get( RES_URL )) );
            aURL.SetMap( &aMap );
            aSet.Put( aURL );
            rSh.SetFlyFrameAttr( aSet );
            bRet = true;
        }
    }

    return bRet;
}

bool SwTransferable::_PasteImageMap( TransferableDataHelper& rData,
                                    SwWrtShell& rSh )
{
    bool bRet = false;
    if( rData.HasFormat( SotClipboardFormatId::SVIM ))
    {
        SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
        rSh.GetFlyFrameAttr( aSet );
        SwFormatURL aURL( static_cast<const SwFormatURL&>(aSet.Get( RES_URL )) );
        const ImageMap* pOld = aURL.GetMap();

        // set or replace, that is the question
        ImageMap aImageMap;
        if( rData.GetImageMap( SotClipboardFormatId::SVIM, aImageMap ) &&
            ( !pOld || aImageMap != *pOld ))
        {
            aURL.SetMap( &aImageMap );
            aSet.Put( aURL );
            rSh.SetFlyFrameAttr( aSet );
        }
        bRet = true;
    }
    return bRet;
}

bool SwTransferable::_PasteAsHyperlink( TransferableDataHelper& rData,
                                        SwWrtShell& rSh, SotClipboardFormatId nFormat )
{
    bool bRet = false;
    OUString sFile;
    if( rData.GetString( nFormat, sFile ) && !sFile.isEmpty() )
    {
        OUString sDesc;
        SwTransferable::_CheckForURLOrLNKFile( rData, sFile, &sDesc );

        // first, make the URL absolute
        INetURLObject aURL;
        aURL.SetSmartProtocol( INetProtocol::File );
        aURL.SetSmartURL( sFile );
        sFile = aURL.GetMainURL( INetURLObject::NO_DECODE );

        switch( rSh.GetObjCntTypeOfSelection() )
        {
        case OBJCNT_FLY:
        case OBJCNT_GRF:
        case OBJCNT_OLE:
            {
                SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                rSh.GetFlyFrameAttr( aSet );
                SwFormatURL aURL2( static_cast<const SwFormatURL&>(aSet.Get( RES_URL )) );
                aURL2.SetURL( sFile, false );
                if( aURL2.GetName().isEmpty() )
                    aURL2.SetName( sFile );
                aSet.Put( aURL2 );
                rSh.SetFlyFrameAttr( aSet );
            }
            break;

        default:
            {
                rSh.InsertURL( SwFormatINetFormat( sFile, OUString() ),
                                sDesc.isEmpty() ? sFile : sDesc);
            }
        }
        bRet = true;
    }
    return bRet;
}

bool SwTransferable::_PasteFileName( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, SotClipboardFormatId nFormat,
                                    SwPasteSdr nAction, const Point* pPt,
                    sal_uInt8 nActionFlags, bool /* bMsg */,
                    bool * graphicInserted)
{
    bool bRet = SwTransferable::_PasteGrf( rData, rSh, nFormat, nAction,
                                            pPt, nActionFlags, 0, false);
    if (graphicInserted != nullptr) {
        *graphicInserted = bRet;
    }
    if( !bRet )
    {
        OUString sFile, sDesc;
        if( rData.GetString( nFormat, sFile ) && !sFile.isEmpty() )
        {
#if HAVE_FEATURE_AVMEDIA
            INetURLObject aMediaURL;

            aMediaURL.SetSmartURL( sFile );
            const OUString aMediaURLStr( aMediaURL.GetMainURL( INetURLObject::NO_DECODE ) );

            if( ::avmedia::MediaWindow::isMediaURL( aMediaURLStr, ""/*TODO?*/ ) )
            {
                const SfxStringItem aMediaURLItem( SID_INSERT_AVMEDIA, aMediaURLStr );
                rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(
                                SID_INSERT_AVMEDIA, SfxCallMode::SYNCHRON,
                                &aMediaURLItem, 0L );
            }
#else
            if (false)
            {
            }
#endif
            else
            {
                bool bIsURLFile = SwTransferable::_CheckForURLOrLNKFile( rData, sFile, &sDesc );

                //Own FileFormat? --> insert, not for StarWriter/Web
                OUString sFileURL = URIHelper::SmartRel2Abs(INetURLObject(), sFile, Link<OUString *, bool>(), false );
                const SfxFilter* pFlt = SwPasteSdr::SetAttr == nAction
                        ? nullptr : SwIoSystem::GetFileFilter(sFileURL);
                if( pFlt && dynamic_cast< const SwWebDocShell *>( rSh.GetView().GetDocShell() ) == nullptr )
                {
                // and then pull up the insert-region-dialog by PostUser event
                    SwSectionData * pSect = new SwSectionData(
                                    FILE_LINK_SECTION,
                                    rSh.GetDoc()->GetUniqueSectionName() );
                    pSect->SetLinkFileName( sFileURL );
                    pSect->SetProtectFlag( true );

                    Application::PostUserEvent( LINK( &rSh, SwWrtShell,
                                                      InsertRegionDialog ), pSect );
                    bRet = true;
                    }
                else if( SwPasteSdr::SetAttr == nAction ||
                        ( bIsURLFile && SwPasteSdr::Insert == nAction ))
                {
                    //we can insert foreign files as links after all

                    // first, make the URL absolute
                    INetURLObject aURL;
                    aURL.SetSmartProtocol( INetProtocol::File );
                    aURL.SetSmartURL( sFile );
                    sFile = aURL.GetMainURL( INetURLObject::NO_DECODE );

                    switch( rSh.GetObjCntTypeOfSelection() )
                    {
                    case OBJCNT_FLY:
                    case OBJCNT_GRF:
                    case OBJCNT_OLE:
                        {
                            SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                            rSh.GetFlyFrameAttr( aSet );
                            SwFormatURL aURL2( static_cast<const SwFormatURL&>(aSet.Get( RES_URL )) );
                            aURL2.SetURL( sFile, false );
                            if( aURL2.GetName().isEmpty() )
                                aURL2.SetName( sFile );
                            aSet.Put( aURL2 );
                            rSh.SetFlyFrameAttr( aSet );
                        }
                        break;

                    default:
                        {
                            rSh.InsertURL( SwFormatINetFormat( sFile, OUString() ),
                                            sDesc.isEmpty() ? sFile : sDesc );
                        }
                    }
                    bRet = true;
                }
            }
        }
    }
    return bRet;
}

bool SwTransferable::_PasteDBData( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, SotClipboardFormatId nFormat, bool bLink,
                                    const Point* pDragPt, bool bMsg )
{
    bool bRet = false;
    OUString sText;
    if( rData.GetString( nFormat, sText ) && !sText.isEmpty() )
    {
        sal_uInt16 nWh = SotClipboardFormatId::SBA_CTRLDATAEXCHANGE == nFormat
                    ? 0
                    : SotClipboardFormatId::SBA_DATAEXCHANGE == nFormat
                                ? (bLink
                                    ? FN_QRY_MERGE_FIELD
                                    : FN_QRY_INSERT)
                                : (bLink
                                    ? 0
                                    : FN_QRY_INSERT_FIELD );
        DataFlavorExVector& rVector = rData.GetDataFlavorExVector();
        bool bHaveColumnDescriptor = OColumnTransferable::canExtractColumnDescriptor(rVector, ColumnTransferFormatFlags::COLUMN_DESCRIPTOR | ColumnTransferFormatFlags::CONTROL_EXCHANGE);
        if ( SotClipboardFormatId::XFORMS == nFormat )
        {
            rSh.MakeDrawView();
            FmFormView* pFmView = dynamic_cast<FmFormView*>( rSh.GetDrawView()  );
            if(pFmView) {
                const OXFormsDescriptor &rDesc = OXFormsTransferable::extractDescriptor(rData);
                SdrObject* pObj = pFmView->CreateXFormsControl(rDesc);
                if(nullptr != pObj)
                {
                    rSh.SwFEShell::InsertDrawObj( *pObj, *pDragPt );
                }
            }
        }
        else if( nWh )
        {
            std::unique_ptr<SfxUsrAnyItem> pConnectionItem;
            std::unique_ptr<SfxUsrAnyItem> pCursorItem;
            std::unique_ptr<SfxUsrAnyItem> pColumnItem;
            std::unique_ptr<SfxUsrAnyItem> pSourceItem;
            std::unique_ptr<SfxUsrAnyItem> pCommandItem;
            std::unique_ptr<SfxUsrAnyItem> pCommandTypeItem;
            std::unique_ptr<SfxUsrAnyItem> pColumnNameItem;
            std::unique_ptr<SfxUsrAnyItem> pSelectionItem;

            bool bDataAvailable = true;
            ODataAccessDescriptor aDesc;
            if(bHaveColumnDescriptor)
                aDesc = OColumnTransferable::extractColumnDescriptor(rData);
            else if(ODataAccessObjectTransferable::canExtractObjectDescriptor(rVector) )
                aDesc = ODataAccessObjectTransferable::extractObjectDescriptor(rData);
            else
                bDataAvailable = false;

            if ( bDataAvailable )
            {
                pConnectionItem.reset(new SfxUsrAnyItem(FN_DB_CONNECTION_ANY, aDesc[daConnection]));
                pColumnItem.reset(new SfxUsrAnyItem(FN_DB_COLUMN_ANY, aDesc[daColumnObject]));
                pSourceItem.reset(new SfxUsrAnyItem(FN_DB_DATA_SOURCE_ANY, makeAny(aDesc.getDataSource())));
                pCommandItem.reset(new SfxUsrAnyItem(FN_DB_DATA_COMMAND_ANY, aDesc[daCommand]));
                pCommandTypeItem.reset(new SfxUsrAnyItem(FN_DB_DATA_COMMAND_TYPE_ANY, aDesc[daCommandType]));
                pColumnNameItem.reset(new SfxUsrAnyItem(FN_DB_DATA_COLUMN_NAME_ANY, aDesc[daColumnName]));
                pSelectionItem.reset(new SfxUsrAnyItem(FN_DB_DATA_SELECTION_ANY, aDesc[daSelection]));
                pCursorItem.reset(new SfxUsrAnyItem(FN_DB_DATA_CURSOR_ANY, aDesc[daCursor]));
            }

            SwView& rView = rSh.GetView();
            //force ::SelectShell
            rView.StopShellTimer();

            SfxStringItem aDataDesc( nWh, sText );
            rView.GetViewFrame()->GetDispatcher()->Execute(
                                nWh, SfxCallMode::ASYNCHRON, &aDataDesc,
                                pConnectionItem.get(), pColumnItem.get(),
                                pSourceItem.get(), pCommandItem.get(), pCommandTypeItem.get(),
                                pColumnNameItem.get(), pSelectionItem.get(), pCursorItem.get(), 0L);
        }
        else
        {
            rSh.MakeDrawView();
            FmFormView* pFmView = dynamic_cast<FmFormView*>( rSh.GetDrawView()  );
            if (pFmView && bHaveColumnDescriptor)
            {
                SdrObject* pObj = pFmView->CreateFieldControl( OColumnTransferable::extractColumnDescriptor(rData) );
                if ( nullptr != pObj)
                    rSh.SwFEShell::InsertDrawObj( *pObj, *pDragPt );
            }
        }
        bRet = true;
    }
    else if( bMsg )
    {
        ScopedVclPtrInstance<MessageDialog>::Create( nullptr, SW_RES(STR_CLPBRD_FORMAT_ERROR), VCL_MESSAGE_INFO)->Execute();
    }
    return bRet;
}

bool SwTransferable::_PasteFileList( TransferableDataHelper& rData,
                                    SwWrtShell& rSh, bool bLink,
                                    const Point* pPt, bool bMsg )
{
    bool bRet = false;
    FileList aFileList;
    if( rData.GetFileList( SotClipboardFormatId::FILE_LIST, aFileList ) &&
        aFileList.Count() )
    {
        SwPasteSdr nAct = bLink ? SwPasteSdr::SetAttr : SwPasteSdr::Insert;
        OUString sFlyNm;
        // iterate over the filelist
        for( sal_uLong n = 0, nEnd = aFileList.Count(); n < nEnd; ++n )
        {
            TransferDataContainer* pHlp = new TransferDataContainer;
            pHlp->CopyString( SotClipboardFormatId::SIMPLE_FILE, aFileList.GetFile( n ));
            TransferableDataHelper aData( pHlp );

            if( SwTransferable::_PasteFileName( aData, rSh, SotClipboardFormatId::SIMPLE_FILE, nAct,
                                            pPt, 0, bMsg, nullptr ))
            {
                if( bLink )
                {
                    sFlyNm = rSh.GetFlyName();
                    SwTransferable::SetSelInShell( rSh, false, pPt );
                }
                bRet = true;
            }
        }
        if( !sFlyNm.isEmpty() )
            rSh.GotoFly( sFlyNm );
    }
    else if( bMsg )
    {
        ScopedVclPtrInstance<MessageDialog>::Create( nullptr, SW_RES(STR_CLPBRD_FORMAT_ERROR), VCL_MESSAGE_INFO)->Execute();
    }
    return bRet;
}

bool SwTransferable::_CheckForURLOrLNKFile( TransferableDataHelper& rData,
                                        OUString& rFileName, OUString* pTitle )
{
    bool bIsURLFile = false;
    INetBookmark aBkmk;
    if( rData.GetINetBookmark( SotClipboardFormatId::SOLK, aBkmk ) )
    {
        rFileName = aBkmk.GetURL();
        if( pTitle )
            *pTitle = aBkmk.GetDescription();
        bIsURLFile = true;
    }
    else
    {
        sal_Int32 nLen = rFileName.getLength();
        if( 4 < nLen && '.' == rFileName[ nLen - 4 ])
        {
            OUString sExt( rFileName.copy( nLen - 3 ));
            if( sExt.equalsIgnoreAsciiCase( "url" ))
            {
                OSL_ENSURE( false, "how do we read today .URL - Files?" );
            }
        }
    }
    return bIsURLFile;
}

bool SwTransferable::IsPasteSpecial( const SwWrtShell& rWrtShell,
                                     const TransferableDataHelper& rData )
{
    // we can paste-special if there's an entry in the paste-special-format list
    SvxClipboardFormatItem aClipboardFormatItem(0);
    FillClipFormatItem( rWrtShell, rData, aClipboardFormatItem);
    return aClipboardFormatItem.Count() > 0;
}

bool SwTransferable::PasteFormat( SwWrtShell& rSh,
                                    TransferableDataHelper& rData,
                                    SotClipboardFormatId nFormat )
{
    SwWait aWait( *rSh.GetView().GetDocShell(), false );
    bool bRet = false;

    SotClipboardFormatId nPrivateFormat = SotClipboardFormatId::PRIVATE;
    SwTransferable *pClipboard = GetSwTransferable( rData );
    if( pClipboard &&
        ((TRNSFR_DOCUMENT|TRNSFR_GRAPHIC|TRNSFR_OLE) & pClipboard->m_eBufferType ))
        nPrivateFormat = SotClipboardFormatId::EMBED_SOURCE;

    if( pClipboard && nPrivateFormat == nFormat )
        bRet = pClipboard->PrivatePaste( rSh );
    else if( rData.HasFormat( nFormat ) )
    {
        uno::Reference<XTransferable> xTransferable( rData.GetXTransferable() );
        sal_uInt16      nEventAction;
        SotExchangeDest nDestination = SwTransferable::GetSotDestination( rSh );
        sal_uInt16      nSourceOptions =
                    (( SotExchangeDest::DOC_TEXTFRAME == nDestination ||
                       SotExchangeDest::SWDOC_FREE_AREA == nDestination ||
                       SotExchangeDest::DOC_TEXTFRAME_WEB == nDestination ||
                       SotExchangeDest::SWDOC_FREE_AREA_WEB == nDestination )
                                        ? EXCHG_IN_ACTION_COPY
                                        : EXCHG_IN_ACTION_MOVE);
        sal_uInt16      nAction = SotExchange::GetExchangeAction(
                                    rData.GetDataFlavorExVector(),
                                    nDestination,
                                    nSourceOptions,             /* ?? */
                                    EXCHG_IN_ACTION_DEFAULT,    /* ?? */
                                    nFormat, nEventAction, nFormat,
                                    lcl_getTransferPointer ( xTransferable ) );

        if( EXCHG_INOUT_ACTION_NONE != nAction )
            bRet = SwTransferable::PasteData( rData, rSh, nAction, nFormat,
                                                nDestination, true, false );
    }
    return bRet;
}

bool SwTransferable::_TestAllowedFormat( const TransferableDataHelper& rData,
                                        SotClipboardFormatId nFormat, SotExchangeDest nDestination )
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
static SotClipboardFormatId aPasteSpecialIds[] =
{
    SotClipboardFormatId::HTML,
    SotClipboardFormatId::HTML_SIMPLE,
    SotClipboardFormatId::HTML_NO_COMMENT,
    SotClipboardFormatId::RTF,
    SotClipboardFormatId::STRING,
    SotClipboardFormatId::SONLK,
    SotClipboardFormatId::NETSCAPE_BOOKMARK,
    SotClipboardFormatId::DRAWING,
    SotClipboardFormatId::SVXB,
    SotClipboardFormatId::GDIMETAFILE,
    SotClipboardFormatId::BITMAP,
    SotClipboardFormatId::SVIM,
    SotClipboardFormatId::FILEGRPDESCRIPTOR,
    SotClipboardFormatId::NONE
};

bool SwTransferable::PasteUnformatted( SwWrtShell& rSh, TransferableDataHelper& rData )
{
    // Plain text == unformatted
    return SwTransferable::PasteFormat( rSh, rData, SotClipboardFormatId::STRING );
}

bool SwTransferable::PasteSpecial( SwWrtShell& rSh, TransferableDataHelper& rData, SotClipboardFormatId& rFormatUsed )
{
    bool bRet = false;
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    std::unique_ptr<SfxAbstractPasteDialog> pDlg(pFact->CreatePasteDialog( &rSh.GetView().GetEditWin() ));

    DataFlavorExVector aFormats( rData.GetDataFlavorExVector() );
    TransferableObjectDescriptor aDesc;

    SotExchangeDest nDest = SwTransferable::GetSotDestination( rSh );

    SwTransferable *pClipboard = GetSwTransferable( rData );
    if( pClipboard )
    {
        aDesc = pClipboard->m_aObjDesc;
        sal_uInt16 nResId;
        if( pClipboard->m_eBufferType & TRNSFR_DOCUMENT )
            nResId = STR_PRIVATETEXT;
        else if( pClipboard->m_eBufferType & TRNSFR_GRAPHIC )
            nResId = STR_PRIVATEGRAPHIC;
        else if( pClipboard->m_eBufferType == TRNSFR_OLE )
            nResId = STR_PRIVATEOLE;
        else
            nResId = 0;

        if( nResId )
        {
            if( STR_PRIVATEOLE == nResId || STR_PRIVATEGRAPHIC == nResId )
            {
                // add SotClipboardFormatId::EMBED_SOURCE to the formats. This
                // format display then the private format name.
                DataFlavorEx aFlavorEx;
                aFlavorEx.mnSotId = SotClipboardFormatId::EMBED_SOURCE;
                aFormats.insert( aFormats.begin(), aFlavorEx );
            }
            pDlg->SetObjName( pClipboard->m_aObjDesc.maClassName,
                                SW_RES( nResId ) );
            pDlg->Insert( SotClipboardFormatId::EMBED_SOURCE, aEmptyOUStr );
        }
    }
    else
    {
        if( rData.HasFormat( SotClipboardFormatId::OBJECTDESCRIPTOR ) )
        {
            (void)rData.GetTransferableObjectDescriptor(
                                SotClipboardFormatId::OBJECTDESCRIPTOR, aDesc );
        }

        if( SwTransferable::_TestAllowedFormat( rData, SotClipboardFormatId::EMBED_SOURCE, nDest ))
            pDlg->Insert( SotClipboardFormatId::EMBED_SOURCE, aEmptyOUStr );
        if( SwTransferable::_TestAllowedFormat( rData, SotClipboardFormatId::LINK_SOURCE, nDest ))
            pDlg->Insert( SotClipboardFormatId::LINK_SOURCE, aEmptyOUStr );
    }

    if( SwTransferable::_TestAllowedFormat( rData, SotClipboardFormatId::LINK, nDest ))
        pDlg->Insert( SotClipboardFormatId::LINK, SW_RES(STR_DDEFORMAT) );

    for( SotClipboardFormatId* pIds = aPasteSpecialIds; *pIds != SotClipboardFormatId::NONE; ++pIds )
        if( SwTransferable::_TestAllowedFormat( rData, *pIds, nDest ))
            pDlg->Insert( *pIds, aEmptyOUStr );

    SotClipboardFormatId nFormat = pDlg->GetFormat( rData.GetTransferable() );

    if( nFormat != SotClipboardFormatId::NONE )
        bRet = SwTransferable::PasteFormat( rSh, rData, nFormat );

    if ( bRet )
        rFormatUsed = nFormat;

    return bRet;
}

void SwTransferable::FillClipFormatItem( const SwWrtShell& rSh,
                                const TransferableDataHelper& rData,
                                SvxClipboardFormatItem & rToFill )
{
    SotExchangeDest nDest = SwTransferable::GetSotDestination( rSh );

    SwTransferable *pClipboard = GetSwTransferable( rData );
    if( pClipboard )
    {
        sal_uInt16 nResId;
        if( pClipboard->m_eBufferType & TRNSFR_DOCUMENT )
            nResId = STR_PRIVATETEXT;
        else if( pClipboard->m_eBufferType & TRNSFR_GRAPHIC )
            nResId = STR_PRIVATEGRAPHIC;
        else if( pClipboard->m_eBufferType == TRNSFR_OLE )
            nResId = STR_PRIVATEOLE;
        else
            nResId = 0;

        if( nResId )
            rToFill.AddClipbrdFormat( SotClipboardFormatId::EMBED_SOURCE,
                                        SW_RESSTR( nResId ) );
    }
    else
    {
        TransferableObjectDescriptor aDesc;
        if (rData.HasFormat(SotClipboardFormatId::OBJECTDESCRIPTOR))
        {
            (void)const_cast<TransferableDataHelper&>(rData).GetTransferableObjectDescriptor(
                                SotClipboardFormatId::OBJECTDESCRIPTOR, aDesc);
        }

        if( SwTransferable::_TestAllowedFormat( rData, SotClipboardFormatId::EMBED_SOURCE, nDest ))
            rToFill.AddClipbrdFormat( SotClipboardFormatId::EMBED_SOURCE,
                                            aDesc.maTypeName );
        if( SwTransferable::_TestAllowedFormat( rData, SotClipboardFormatId::LINK_SOURCE, nDest ))
            rToFill.AddClipbrdFormat( SotClipboardFormatId::LINK_SOURCE );

        SotClipboardFormatId nFormat;
        if ( rData.HasFormat(nFormat = SotClipboardFormatId::EMBED_SOURCE_OLE) || rData.HasFormat(nFormat = SotClipboardFormatId::EMBEDDED_OBJ_OLE) )
        {
            OUString sName,sSource;
            if ( SvPasteObjectHelper::GetEmbeddedName(rData,sName,sSource,nFormat) )
                rToFill.AddClipbrdFormat( nFormat, sName );
        }
    }

    if( SwTransferable::_TestAllowedFormat( rData, SotClipboardFormatId::LINK, nDest ))
        rToFill.AddClipbrdFormat( SotClipboardFormatId::LINK, SW_RESSTR(STR_DDEFORMAT) );

    for( SotClipboardFormatId* pIds = aPasteSpecialIds; *pIds != SotClipboardFormatId::NONE; ++pIds )
        if( SwTransferable::_TestAllowedFormat( rData, *pIds, nDest ))
            rToFill.AddClipbrdFormat( *pIds, aEmptyOUStr );
}

void SwTransferable::SetDataForDragAndDrop( const Point& rSttPos )
{
    if(!m_pWrtShell)
        return;
    OUString sGrfNm;
    const int nSelection = m_pWrtShell->GetSelectionType();
    if( nsSelectionType::SEL_GRF == nSelection)
    {
        AddFormat( SotClipboardFormatId::SVXB );
        const Graphic* pGrf = m_pWrtShell->GetGraphic();
        if ( pGrf && pGrf->IsSupportedGraphic() )
        {
            AddFormat( SotClipboardFormatId::GDIMETAFILE );
            AddFormat( SotClipboardFormatId::PNG );
            AddFormat( SotClipboardFormatId::BITMAP );
        }
        m_eBufferType = TRNSFR_GRAPHIC;
        m_pWrtShell->GetGrfNms( &sGrfNm, nullptr );
    }
    else if( nsSelectionType::SEL_OLE == nSelection )
    {
        AddFormat( SotClipboardFormatId::EMBED_SOURCE );
        PrepareOLE( m_aObjDesc );
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
        AddFormat( SotClipboardFormatId::GDIMETAFILE );
        m_eBufferType = TRNSFR_OLE;
    }
    //Is there anything to provide anyway?
    else if ( m_pWrtShell->IsSelection() || m_pWrtShell->IsFrameSelected() ||
              m_pWrtShell->IsObjSelected() )
    {
        if( m_pWrtShell->IsObjSelected() )
            m_eBufferType = TRNSFR_DRAWING;
        else
        {
            m_eBufferType = TRNSFR_DOCUMENT;
            if( SwWrtShell::NO_WORD !=
                m_pWrtShell->IntelligentCut( nSelection, false ))
                m_eBufferType = TransferBufferType( TRNSFR_DOCUMENT_WORD
                                                    | m_eBufferType);
        }

        if( nSelection & nsSelectionType::SEL_TBL_CELLS )
            m_eBufferType = (TransferBufferType)(TRNSFR_TABELLE | m_eBufferType);

        AddFormat( SotClipboardFormatId::EMBED_SOURCE );

        //put RTF ahead of the OLE's Metafile for less loss
        if( !m_pWrtShell->IsObjSelected() )
        {
            AddFormat( SotClipboardFormatId::RTF );
            AddFormat( SotClipboardFormatId::HTML );
        }
        if( m_pWrtShell->IsSelection() )
            AddFormat( SotClipboardFormatId::STRING );

        if( nSelection & ( nsSelectionType::SEL_DRW | nsSelectionType::SEL_DRW_FORM ))
        {
            AddFormat( SotClipboardFormatId::DRAWING );
            if ( nSelection & nsSelectionType::SEL_DRW )
            {
                AddFormat( SotClipboardFormatId::GDIMETAFILE );
                AddFormat( SotClipboardFormatId::PNG );
                AddFormat( SotClipboardFormatId::BITMAP );
            }
            m_eBufferType = (TransferBufferType)( TRNSFR_GRAPHIC | m_eBufferType );

            m_pClpGraphic = new Graphic;
            if( !m_pWrtShell->GetDrawObjGraphic( SotClipboardFormatId::GDIMETAFILE, *m_pClpGraphic ))
                m_pOrigGraphic = m_pClpGraphic;
            m_pClpBitmap = new Graphic;
            if( !m_pWrtShell->GetDrawObjGraphic( SotClipboardFormatId::BITMAP, *m_pClpBitmap ))
                m_pOrigGraphic = m_pClpBitmap;

            // is it an URL-Button ?
            OUString sURL;
            OUString sDesc;
            if( m_pWrtShell->GetURLFromButton( sURL, sDesc ) )
            {
                AddFormat( SotClipboardFormatId::STRING );
                AddFormat( SotClipboardFormatId::SOLK );
                AddFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK );
                AddFormat( SotClipboardFormatId::FILECONTENT );
                AddFormat( SotClipboardFormatId::FILEGRPDESCRIPTOR );
                AddFormat( SotClipboardFormatId::UNIFORMRESOURCELOCATOR );
                m_eBufferType = (TransferBufferType)( TRNSFR_INETFLD | m_eBufferType );
            }
        }

        //ObjectDescriptor was already filled from the old DocShell.
        //Now adjust it. Thus in GetData the first query can still
        //be answered with delayed rendering.
        m_aObjDesc.mbCanLink = false;
        m_aObjDesc.maDragStartPos = rSttPos;
        m_aObjDesc.maSize = OutputDevice::LogicToLogic( Size( OLESIZE ),
                                                MAP_TWIP, MAP_100TH_MM );
        PrepareOLE( m_aObjDesc );
        AddFormat( SotClipboardFormatId::OBJECTDESCRIPTOR );
    }
    else if( nSelection & nsSelectionType::SEL_TXT && !m_pWrtShell->HasMark() )
    {
        // is only one field - selected?
        SwContentAtPos aContentAtPos( SwContentAtPos::SW_INETATTR );
        Point aPos( SwEditWin::GetDDStartPosX(), SwEditWin::GetDDStartPosY());

        if( m_pWrtShell->GetContentAtPos( aPos, aContentAtPos ) )
        {
            AddFormat( SotClipboardFormatId::STRING );
            AddFormat( SotClipboardFormatId::SOLK );
            AddFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK );
            AddFormat( SotClipboardFormatId::FILECONTENT );
            AddFormat( SotClipboardFormatId::FILEGRPDESCRIPTOR );
            AddFormat( SotClipboardFormatId::UNIFORMRESOURCELOCATOR );
            m_eBufferType = TRNSFR_INETFLD;
        }
    }

    if( m_pWrtShell->IsFrameSelected() )
    {
        SfxItemSet aSet( m_pWrtShell->GetAttrPool(), RES_URL, RES_URL );
        m_pWrtShell->GetFlyFrameAttr( aSet );
        const SwFormatURL& rURL = static_cast<const SwFormatURL&>(aSet.Get( RES_URL ));
        if( rURL.GetMap() )
        {
            m_pImageMap = new ImageMap( *rURL.GetMap() );
            AddFormat( SotClipboardFormatId::SVIM );
        }
        else if( !rURL.GetURL().isEmpty() )
        {
            m_pTargetURL = new INetImage( sGrfNm, rURL.GetURL(),
                                        rURL.GetTargetFrameName(),
                                        aEmptyOUStr, Size() );
            AddFormat( SotClipboardFormatId::INET_IMAGE );
        }
    }
}

void SwTransferable::StartDrag( vcl::Window* pWin, const Point& rPos )
{
    if(!m_pWrtShell)
        return;
    m_bOldIdle = m_pWrtShell->GetViewOptions()->IsIdle();
    m_bCleanUp = true;

    m_pWrtShell->GetViewOptions()->SetIdle( false );

    if( m_pWrtShell->IsSelFrameMode() )
        m_pWrtShell->ShowCursor();

    SW_MOD()->m_pDragDrop = this;

    SetDataForDragAndDrop( rPos );

    sal_Int8 nDragOptions = DND_ACTION_COPYMOVE | DND_ACTION_LINK;
    SwDocShell* pDShell = m_pWrtShell->GetView().GetDocShell();
    if( ( pDShell && pDShell->IsReadOnly() ) || m_pWrtShell->HasReadonlySel() )
        nDragOptions &= ~DND_ACTION_MOVE;

    TransferableHelper::StartDrag( pWin, nDragOptions );
}

void SwTransferable::DragFinished( sal_Int8 nAction )
{
    //And the last finishing work so that all statuses are right
    if( DND_ACTION_MOVE == nAction  )
    {
        if( m_bCleanUp )
        {
            //It was dropped outside of Writer. We still have to
            //delete.

            m_pWrtShell->StartAllAction();
            m_pWrtShell->StartUndo( UNDO_UI_DRAG_AND_MOVE );
            if ( m_pWrtShell->IsTableMode() )
                m_pWrtShell->DeleteTableSel();
            else
            {
                if ( !(m_pWrtShell->IsSelFrameMode() || m_pWrtShell->IsObjSelected()) )
                    //SmartCut, take one of the blanks along
                    m_pWrtShell->IntelligentCut( m_pWrtShell->GetSelectionType() );
                m_pWrtShell->DelRight();
            }
            m_pWrtShell->EndUndo( UNDO_UI_DRAG_AND_MOVE );
            m_pWrtShell->EndAllAction();
        }
        else
        {
            const int nSelection = m_pWrtShell->GetSelectionType();
            if( ( nsSelectionType::SEL_FRM | nsSelectionType::SEL_GRF |
                 nsSelectionType::SEL_OLE | nsSelectionType::SEL_DRW ) & nSelection )
            {
                m_pWrtShell->EnterSelFrameMode();
            }
        }
    }
    m_pWrtShell->GetView().GetEditWin().DragFinished();

    if( m_pWrtShell->IsSelFrameMode() )
        m_pWrtShell->HideCursor();
    else
        m_pWrtShell->ShowCursor();

    m_pWrtShell->GetViewOptions()->SetIdle( m_bOldIdle );
}

namespace
{

bool lcl_checkClassification(SwDoc* pSourceDoc, SwDoc* pDestinationDoc)
{
    if (!pSourceDoc || !pDestinationDoc)
        return true;

    SwDocShell* pSourceShell = pSourceDoc->GetDocShell();
    SwDocShell* pDestinationShell = pDestinationDoc->GetDocShell();
    if (!pSourceShell || !pDestinationShell)
        return true;

    SfxClassificationCheckPasteResult eResult = SfxClassificationHelper::CheckPaste(pSourceShell->getDocProperties(), pDestinationShell->getDocProperties());
    return SfxClassificationHelper::ShowPasteInfo(eResult);
}

}

bool SwTransferable::PrivatePaste( SwWrtShell& rShell )
{
    // first, ask for the SelectionType, then action-bracketing !!!!
    // (otherwise it's not pasted into a TableSelection!!!)
    OSL_ENSURE( !rShell.ActionPend(), "Paste must never have an ActionPend" );
    if ( !m_pClpDocFac )
        return false; // the return value of the SwFEShell::Paste also is bool!

    const int nSelection = rShell.GetSelectionType();

    SwRewriter aRewriter;

    SwTrnsfrActionAndUndo aAction( &rShell, UNDO_PASTE_CLIPBOARD);

    bool bKillPaMs = false;

    //Delete selected content, not at table-selection and table in Clipboard, and don't delete hovering graphics.
    if( rShell.HasSelection() && !( nSelection & nsSelectionType::SEL_TBL_CELLS) && !( nSelection & nsSelectionType::SEL_DRW))
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
            rShell.SwCursorShell::SetCursor( aPt, true );
        }
        rShell.SetRetainSelection( false );
    }
    if ( nSelection & nsSelectionType::SEL_DRW) //unselect hovering graphics
    {
        rShell.ResetSelect(nullptr,false);
    }

    bool bInWrd = false, bEndWrd = false, bSttWrd = false,
         bSmart = 0 != (TRNSFR_DOCUMENT_WORD & m_eBufferType);
    if( bSmart )
    {
            // Why not for other Scripts? If TRNSFR_DOCUMENT_WORD is set, we have a word
            // in the buffer, word in this context means 'something with spaces at beginning
            // and end'. In this case we definitely want these spaces to be inserted here.
            bInWrd = rShell.IsInWord();
            bEndWrd = rShell.IsEndWrd();
            bSmart = bInWrd || bEndWrd;
            if( bSmart )
            {
                bSttWrd = rShell.IsStartWord();
                if( bSmart && !bSttWrd && (bInWrd || bEndWrd) )
                    rShell.SwEditShell::Insert(' ');
            }
    }

    bool bRet = true;
    // m_pWrtShell is nullptr when the source document is closed already.
    if (!m_pWrtShell || lcl_checkClassification(m_pWrtShell->GetDoc(), rShell.GetDoc()))
        bRet = rShell.Paste(m_pClpDocFac->GetDoc());

    if( bKillPaMs )
        rShell.KillPams();

    // If Smart Paste then insert blank
    if( bRet && bSmart && ((bInWrd && !bEndWrd )|| bSttWrd) )
        rShell.SwEditShell::Insert(' ');

    return bRet;
}

bool SwTransferable::PrivateDrop( SwWrtShell& rSh, const Point& rDragPt,
                                bool bMove, bool bIsXSelection )
{
    int cWord    = 0;
    bool bInWrd  = false;
    bool bEndWrd = false;
    bool bSttWrd = false;
    bool bSttPara = false;
    bool bTableSel = false;
    bool bFrameSel = false;

    SwWrtShell& rSrcSh = *GetShell();

    rSh.UnSetVisibleCursor();

    if( TRNSFR_INETFLD == m_eBufferType )
    {
        if( rSh.GetFormatFromObj( rDragPt ) )
        {
            INetBookmark aTmp;
            if( (TRNSFR_INETFLD & m_eBufferType) && m_pBookmark )
                aTmp = *m_pBookmark;

            // select target graphic
            if( rSh.SelectObj( rDragPt ) )
            {
                rSh.HideCursor();
                rSh.EnterSelFrameMode( &rDragPt );
                g_bFrameDrag = true;
            }

            const int nSelection = rSh.GetSelectionType();

            // not yet consider Draw objects
            if( nsSelectionType::SEL_GRF & nSelection )
            {
                SfxItemSet aSet( rSh.GetAttrPool(), RES_URL, RES_URL );
                rSh.GetFlyFrameAttr( aSet );
                SwFormatURL aURL( static_cast<const SwFormatURL&>(aSet.Get( RES_URL )) );
                aURL.SetURL( aTmp.GetURL(), false );
                aSet.Put( aURL );
                rSh.SetFlyFrameAttr( aSet );
                return true;
            }

            if( nsSelectionType::SEL_DRW & nSelection )
            {
                rSh.LeaveSelFrameMode();
                rSh.UnSelectFrame();
                rSh.ShowCursor();
                g_bFrameDrag = false;
            }
        }
    }

    if( &rSh != &rSrcSh && (nsSelectionType::SEL_GRF & rSh.GetSelectionType()) &&
        TRNSFR_GRAPHIC == m_eBufferType )
    {
        // ReRead the graphic
        OUString sGrfNm;
        OUString sFltNm;
        rSrcSh.GetGrfNms( &sGrfNm, &sFltNm );
        rSh.ReRead( sGrfNm, sFltNm, rSrcSh.GetGraphic() );
        return true;
    }

    //not in selections or selected frames
    if( rSh.ChgCurrPam( rDragPt ) ||
        ( rSh.IsSelFrameMode() && rSh.IsInsideSelectedObj( rDragPt )) )
        return false;

    if( rSrcSh.IsTableMode() )
        bTableSel = true;
    else if( rSrcSh.IsSelFrameMode() || rSrcSh.IsObjSelected() )
    {
        // don't move position-protected objects!
        if( bMove && rSrcSh.IsSelObjProtected( FlyProtectFlags::Pos ) != FlyProtectFlags::NONE )
            return false;

        bFrameSel = true;
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
        rSh.SwCursorShell::SetCursor( rDragPt, true );
        cWord = rSrcSh.IntelligentCut( nSel, false );
    }
    else if( !bTableSel && !bFrameSel )
    {
        if( !rSh.IsAddMode() )
        {
            // #i87233#
            if ( rSh.IsBlockMode() )
            {
                // preserve order of cursors for block mode
                rSh.GoPrevCursor();
            }

            rSh.SwCursorShell::CreateCursor();
        }
        rSh.SwCursorShell::SetCursor( rDragPt, true, false );
        rSh.GoPrevCursor();
        cWord = rSh.IntelligentCut( rSh.GetSelectionType(), false );
        rSh.GoNextCursor();
    }

    bInWrd  = rSh.IsInWord();
    bEndWrd = rSh.IsEndWrd();
    bSttWrd = !bEndWrd && rSh.IsStartWord();
    bSttPara= rSh.IsSttPara();

    Point aSttPt( SwEditWin::GetDDStartPosX(), SwEditWin::GetDDStartPosY() );

    // at first, select INetFelder!
    if( TRNSFR_INETFLD == m_eBufferType )
    {
        if( &rSrcSh == &rSh )
        {
            rSh.GoPrevCursor();
            rSh.SwCursorShell::SetCursor( aSttPt, true );
            rSh.SelectTextAttr( RES_TXTATR_INETFMT );
            if( rSh.ChgCurrPam( rDragPt ) )
            {
                // don't copy/move inside of yourself
                rSh.DestroyCursor();
                rSh.EndUndo();
                rSh.EndAction();
                rSh.EndAction();
                return false;
            }
            rSh.GoNextCursor();
        }
        else
        {
            rSrcSh.SwCursorShell::SetCursor( aSttPt, true );
            rSrcSh.SelectTextAttr( RES_TXTATR_INETFMT );
        }

        // is there an URL attribute at the insert point? Then replace that,
        // so simply put up a selection?
        rSh.DelINetAttrWithText();
        g_bDDINetAttr = true;
    }

    if ( rSrcSh.IsSelFrameMode() )
    {
        //Hack: fool the special treatment
        aSttPt -= aSttPt - rSrcSh.GetObjRect().Pos();
    }

    bool bRet = rSrcSh.SwFEShell::Copy( &rSh, aSttPt, rDragPt, bMove,
                                            !bIsXSelection );

    if( !bIsXSelection )
    {
        rSrcSh.Push();
        if ( bRet && bMove && !bFrameSel )
        {
            if ( bTableSel )
            {
                /* delete table contents not cells */
                rSrcSh.Delete();
            }
            else
            {
                //SmartCut, take one of the blanks along.
                rSh.SwCursorShell::DestroyCursor();
                if ( cWord == SwWrtShell::WORD_SPACE_BEFORE )
                    rSh.ExtendSelection( false );
                else if ( cWord == SwWrtShell::WORD_SPACE_AFTER )
                    rSh.ExtendSelection();
                rSrcSh.DelRight();
            }
        }
        rSrcSh.KillPams();
        rSrcSh.Pop( false );

        /* after dragging a table selection inside one shell
            set cursor to the drop position. */
        if( &rSh == &rSrcSh && ( bTableSel || rSh.IsBlockMode() ) )
        {
            rSrcSh.SwCursorShell::SetCursor(rDragPt);
            rSrcSh.GetSwCursor()->SetMark();
        }
    }

    if( bRet && !bTableSel && !bFrameSel )
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
                rSh.SwCursorShell::DestroyCursor();
                rSh.GoPrevCursor();
            }
            else
            {
                rSh.SwapPam();
                rSh.SwCursorShell::ClearMark();
            }
        }
        else
        {
            if( rSh.IsAddMode() )
                rSh.SwCursorShell::CreateCursor();
            else
            {
                // turn on selection mode
                rSh.SttSelect();
                rSh.EndSelect();
            }
        }
    }

    if( bRet && bMove && bFrameSel )
        rSrcSh.LeaveSelFrameMode();

    if( rSrcSh.GetDoc() != rSh.GetDoc() )
        rSrcSh.EndUndo();
    rSh.EndUndo();

        // put the shell in the right state
    if( &rSrcSh != &rSh && ( rSh.IsFrameSelected() || rSh.IsObjSelected() ))
        rSh.EnterSelFrameMode();

    rSrcSh.EndAction();
    rSh.EndAction();
    return true;
}

// Interfaces for Selection
void SwTransferable::CreateSelection( SwWrtShell& rSh,
                                      const SwFrameShell * _pCreatorView )
{
    SwModule *pMod = SW_MOD();
    SwTransferable* pNew = new SwTransferable( rSh );

    pNew->m_pCreatorView = _pCreatorView;

    uno::Reference< XTransferable > xRef( pNew );
    pMod->m_pXSelection = pNew;
    pNew->CopyToSelection( rSh.GetWin() );
}

void SwTransferable::ClearSelection( SwWrtShell& rSh,
                                     const SwFrameShell * _pCreatorView)
{
    SwModule *pMod = SW_MOD();
    if( pMod->m_pXSelection &&
        ((!pMod->m_pXSelection->m_pWrtShell) || (pMod->m_pXSelection->m_pWrtShell == &rSh)) &&
        (!_pCreatorView || (pMod->m_pXSelection->m_pCreatorView == _pCreatorView)) )
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

sal_Int64 SwTransferable::getSomething( const Sequence< sal_Int8 >& rId ) throw( RuntimeException, std::exception )
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
    SwTransferable* pSwTransferable = nullptr;

    uno::Reference<XUnoTunnel> xTunnel( rData.GetTransferable(), UNO_QUERY );
    if ( xTunnel.is() )
    {
        sal_Int64 nHandle = xTunnel->getSomething( getUnoTunnelId() );
        if ( nHandle )
            pSwTransferable = reinterpret_cast<SwTransferable*>( (sal_IntPtr) nHandle );
    }

    return pSwTransferable;

}

SwTrnsfrDdeLink::SwTrnsfrDdeLink( SwTransferable& rTrans, SwWrtShell& rSh )
    : rTrnsfr(rTrans)
    , pDocShell(nullptr)
    , nOldTimeOut(0)
    , bDelBookmrk(false)
    , bInDisconnect(false)
{
    // we only end up here with table- or text selection
    if( nsSelectionType::SEL_TBL_CELLS & rSh.GetSelectionType() )
    {
        SwFrameFormat* pFormat = rSh.GetTableFormat();
        if( pFormat )
            sName = pFormat->GetName();
    }
    else
    {
        // creating a temp. bookmark without undo
        bool bUndo = rSh.DoesUndo();
        rSh.DoUndo( false );
        bool bIsModified = rSh.IsModified();

        ::sw::mark::IMark* pMark = rSh.SetBookmark(
            vcl::KeyCode(),
            OUString(),
            OUString(),
            IDocumentMarkAccess::MarkType::DDE_BOOKMARK);
        if(pMark)
        {
            sName = pMark->GetName();
            bDelBookmrk = true;
            if( !bIsModified )
                rSh.ResetModified();
        }
        else
            sName.clear();
        rSh.DoUndo( bUndo );
    }

    if( !sName.isEmpty() &&
        nullptr != ( pDocShell = rSh.GetDoc()->GetDocShell() ) )
    {
        // then we create our "server" and connect to it
        refObj = pDocShell->DdeCreateLinkSource( sName );
        if( refObj.Is() )
        {
            refObj->AddConnectAdvise( this );
            refObj->AddDataAdvise( this,
                            aEmptyOUStr,
                            ADVISEMODE_NODATA | ADVISEMODE_ONLYONCE );
            nOldTimeOut = refObj->GetUpdateTimeout();
            refObj->SetUpdateTimeout( 0 );
        }
    }
}

SwTrnsfrDdeLink::~SwTrnsfrDdeLink()
{
    if( refObj.Is() )
        Disconnect( true );
}

::sfx2::SvBaseLink::UpdateResult SwTrnsfrDdeLink::DataChanged( const OUString& ,
                                    const uno::Any& )
{
    // well, that's it with the link
    if( !bInDisconnect )
    {
        if( FindDocShell() && pDocShell->GetView() )
            rTrnsfr.RemoveDDELinkFormat( pDocShell->GetView()->GetEditWin() );
        Disconnect( false );
    }
    return SUCCESS;
}

bool SwTrnsfrDdeLink::WriteData( SvStream& rStrm )
{
    if( !refObj.Is() || !FindDocShell() )
        return false;

    rtl_TextEncoding eEncoding = DDE_TXT_ENCODING;
    const OString aAppNm(OUStringToOString(
        Application::GetAppName(), eEncoding));
    const OString aTopic(OUStringToOString(
        pDocShell->GetTitle(SFX_TITLE_FULLNAME), eEncoding));
    const OString aName(OUStringToOString(sName, eEncoding));

    std::unique_ptr<sal_Char[]> pMem(new sal_Char[ aAppNm.getLength() + aTopic.getLength() + aName.getLength() + 4 ]);

    sal_Int32 nLen = aAppNm.getLength();
    memcpy( pMem.get(), aAppNm.getStr(), nLen );
    pMem[ nLen++ ] = 0;
    memcpy( pMem.get() + nLen, aTopic.getStr(), aTopic.getLength() );
    nLen = nLen + aTopic.getLength();
    pMem[ nLen++ ] = 0;
    memcpy( pMem.get() + nLen, aName.getStr(), aName.getLength() );
    nLen = nLen + aName.getLength();
    pMem[ nLen++ ] = 0;
    pMem[ nLen++ ] = 0;

    rStrm.Write( pMem.get(), nLen );
    pMem.reset();

    IDocumentMarkAccess* const pMarkAccess = pDocShell->GetDoc()->getIDocumentMarkAccess();
    IDocumentMarkAccess::const_iterator_t ppMark = pMarkAccess->findMark(sName);
    if(ppMark != pMarkAccess->getAllMarksEnd()
        && IDocumentMarkAccess::GetType(**ppMark) != IDocumentMarkAccess::MarkType::BOOKMARK)
    {
        // the mark is still a DdeBookmark
        // we replace it with a Bookmark, so it will get saved etc.
        ::sw::mark::IMark* const pMark = ppMark->get();
        ::sfx2::SvLinkSource* p = &refObj;
        SwServerObject& rServerObject = dynamic_cast<SwServerObject&>(*p);

        // collecting state of old mark
        SwPaM aPaM(pMark->GetMarkStart());
        *aPaM.GetPoint() = pMark->GetMarkStart();
        if(pMark->IsExpanded())
        {
            aPaM.SetMark();
            *aPaM.GetMark() = pMark->GetMarkEnd();
        }
        OUString sMarkName = pMark->GetName();

        // remove mark
        rServerObject.SetNoServer(); // this removes the connection between SwServerObject and mark
        // N.B. ppMark was not loaded from file and cannot have xml:id
        pMarkAccess->deleteMark(ppMark);

        // recreate as Bookmark
        ::sw::mark::IMark* const pNewMark = pMarkAccess->makeMark(
            aPaM,
            sMarkName,
            IDocumentMarkAccess::MarkType::BOOKMARK);
        rServerObject.SetDdeBookmark(*pNewMark);
    }

    bDelBookmrk = false;
    return true;
}

void SwTrnsfrDdeLink::Disconnect( bool bRemoveDataAdvise )
{
    //  don't accept DataChanged anymore, when already in Disconnect!
    //  (DTOR from Bookmark sends a DataChanged!)
    bool bOldDisconnect = bInDisconnect;
    bInDisconnect = true;

    // destroy the unused bookmark again (without Undo!)?
    if( bDelBookmrk && refObj.Is() && FindDocShell() )
    {
        SwDoc* pDoc = pDocShell->GetDoc();
        ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

        // #i58448#
        Link<bool,void> aSavedOle2Link( pDoc->GetOle2Link() );
        pDoc->SetOle2Link( Link<bool,void>() );

        bool bIsModified = pDoc->getIDocumentState().IsModified();

        IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
        pMarkAccess->deleteMark(pMarkAccess->findMark(sName));

        if( !bIsModified )
            pDoc->getIDocumentState().ResetModified();
        // #i58448#
        pDoc->SetOle2Link( aSavedOle2Link );

        bDelBookmrk = false;
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

bool SwTrnsfrDdeLink::FindDocShell()
{
    SfxObjectShell* pTmpSh = SfxObjectShell::GetFirst( checkSfxObjectShell<SwDocShell> );
    while( pTmpSh )
    {
        if( pTmpSh == pDocShell )       // that's what we want to have
        {
            if( pDocShell->GetDoc() )
                return true;
            break;      // the Doc is not there anymore, so leave!
        }
        pTmpSh = SfxObjectShell::GetNext( *pTmpSh, checkSfxObjectShell<SwDocShell> );
    }

    pDocShell = nullptr;
    return false;
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
