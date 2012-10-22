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

#include <hintids.hxx>
#include <tools/helpers.hxx>
#include <tools/urlobj.hxx>
#include <svl/undo.hxx>
#include <svl/fstathelper.hxx>
#include <svtools/imap.hxx>
#include <svtools/filter.hxx>
#include <sot/storage.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/boxitem.hxx>
#include <sot/formats.hxx>
#include <fmtfsize.hxx>
#include <fmturl.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <frmatr.hxx>
#include <grfatr.hxx>
#include <swtypes.hxx>
#include <ndgrf.hxx>
#include <fmtcol.hxx>
#include <hints.hxx>
#include <swbaslnk.hxx>
#include <pagefrm.hxx>
#include <editsh.hxx>
#include <pam.hxx>

#include <rtl/ustring.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <vcl/svapp.hxx>
#include <com/sun/star/io/XSeekable.hpp>
// #i73788#
#include <retrieveinputstreamconsumer.hxx>

using namespace com::sun::star;

// --------------------
// SwGrfNode
// --------------------
SwGrfNode::SwGrfNode(
        const SwNodeIndex & rWhere,
        const String& rGrfName, const String& rFltName,
        const Graphic* pGraphic,
        SwGrfFmtColl *pGrfColl,
        SwAttrSet* pAutoAttr ) :
    SwNoTxtNode( rWhere, ND_GRFNODE, pGrfColl, pAutoAttr ),
    // #i73788#
    mbLinkedInputStreamReady( false ),
    mbIsStreamReadOnly( sal_False )
{
    aGrfObj.SetSwapStreamHdl( LINK( this, SwGrfNode, SwapGraphic ) );
    bInSwapIn = bChgTwipSize = bChgTwipSizeFromPixel = bLoadLowResGrf =
        bFrameInPaint = bScaleImageMap = sal_False;

    bGrafikArrived = sal_True;
    ReRead(rGrfName,rFltName, pGraphic, 0, sal_False);
}

SwGrfNode::SwGrfNode( const SwNodeIndex & rWhere,
                          const GraphicObject& rGrfObj,
                      SwGrfFmtColl *pGrfColl, SwAttrSet* pAutoAttr ) :
    SwNoTxtNode( rWhere, ND_GRFNODE, pGrfColl, pAutoAttr ),
    // #i73788#
    mbLinkedInputStreamReady( false ),
    mbIsStreamReadOnly( sal_False )
{
    aGrfObj = rGrfObj;
    aGrfObj.SetSwapStreamHdl( LINK( this, SwGrfNode, SwapGraphic ) );
    if( rGrfObj.HasUserData() && rGrfObj.IsSwappedOut() )
        aGrfObj.SetSwapState();
    bInSwapIn = bChgTwipSize = bChgTwipSizeFromPixel= bLoadLowResGrf =
        bFrameInPaint = bScaleImageMap = sal_False;
    bGrafikArrived = sal_True;
}

// Konstruktor fuer den SW/G-Reader. Dieser ctor wird verwendet,
// wenn eine gelinkte Grafik gelesen wird. Sie liest diese NICHT ein.


SwGrfNode::SwGrfNode( const SwNodeIndex & rWhere,
                      const String& rGrfName, const String& rFltName,
                      SwGrfFmtColl *pGrfColl,
                      SwAttrSet* pAutoAttr ) :
    SwNoTxtNode( rWhere, ND_GRFNODE, pGrfColl, pAutoAttr ),
    // #i73788#
    mbLinkedInputStreamReady( false ),
    mbIsStreamReadOnly( sal_False )
{
    aGrfObj.SetSwapStreamHdl( LINK( this, SwGrfNode, SwapGraphic ) );

    Graphic aGrf; aGrf.SetDefaultType();
    aGrfObj.SetGraphic( aGrf, rGrfName );

    bInSwapIn = bChgTwipSize = bChgTwipSizeFromPixel = bLoadLowResGrf =
        bFrameInPaint = bScaleImageMap = sal_False;
    bGrafikArrived = sal_True;

    InsertLink( rGrfName, rFltName );
    if( IsLinkedFile() )
    {
        INetURLObject aUrl( rGrfName );
        if( INET_PROT_FILE == aUrl.GetProtocol() &&
            FStatHelper::IsDocument( aUrl.GetMainURL( INetURLObject::NO_DECODE ) ))
        {
            // File vorhanden, Verbindung herstellen ohne ein Update
            ((SwBaseLink*)&refLink)->Connect();
        }
    }
}

sal_Bool SwGrfNode::ReRead(
    const String& rGrfName, const String& rFltName,
    const Graphic* pGraphic, const GraphicObject* pGrfObj,
    sal_Bool bNewGrf )
{
    sal_Bool bReadGrf = sal_False, bSetTwipSize = sal_True;

    OSL_ENSURE( pGraphic || pGrfObj || rGrfName.Len(),
            "GraphicNode without a name, Graphic or GraphicObject" );

    // ReadRead mit Namen
    if( refLink.Is() )
    {
        OSL_ENSURE( !bInSwapIn, "ReRead: stehe noch im SwapIn" );

        if( rGrfName.Len() )
        {
            // Besonderheit: steht im FltNamen DDE, handelt es sich um eine
            //                  DDE-gelinkte Grafik
            String sCmd( rGrfName );
            if( rFltName.Len() )
            {
                sal_uInt16 nNewType;
                if( rFltName.EqualsAscii( "DDE" ))
                    nNewType = OBJECT_CLIENT_DDE;
                else
                {
                    sfx2::MakeLnkName( sCmd, 0, rGrfName, aEmptyStr, &rFltName );
                    nNewType = OBJECT_CLIENT_GRF;
                }

                if( nNewType != refLink->GetObjType() )
                {
                    refLink->Disconnect();
                    ((SwBaseLink*)&refLink)->SetObjType( nNewType );
                }
            }

            refLink->SetLinkSourceName( sCmd );
        }
        else        // kein Name mehr, Link aufheben
        {
            GetDoc()->GetLinkManager().Remove( refLink );
            refLink.Clear();
        }

        if( pGraphic )
        {
            aGrfObj.SetGraphic( *pGraphic, rGrfName );
            bReadGrf = sal_True;
        }
        else if( pGrfObj )
        {
            aGrfObj = *pGrfObj;
            if( pGrfObj->HasUserData() && pGrfObj->IsSwappedOut() )
                aGrfObj.SetSwapState();
            aGrfObj.SetLink( rGrfName );
            bReadGrf = sal_True;
        }
        else
        {
            // MIB 25.02.97: Daten der alten Grafik zuruecksetzen, damit
            // die korrekte Ersatz-Darstellung erscheint, wenn die
            // der neue Link nicht geladen werden konnte.
            Graphic aGrf; aGrf.SetDefaultType();
            aGrfObj.SetGraphic( aGrf, rGrfName );

            if( refLink.Is() )
            {
                if( getLayoutFrm( GetDoc()->GetCurrentLayout() ) )
                {
                    SwMsgPoolItem aMsgHint( RES_GRF_REREAD_AND_INCACHE );
                    ModifyNotification( &aMsgHint, &aMsgHint );
                }
                // #i59688#
                // do not load linked graphic, if it isn't a new linked graphic.
//                else {
                else if ( bNewGrf )
                {
                    //TODO refLink->setInputStream(getInputStream());
                    ((SwBaseLink*)&refLink)->SwapIn();
                }
            }
            bSetTwipSize = sal_False;
        }
    }
    else if( pGraphic && !rGrfName.Len() )
    {
        // MIB 27.02.2001: Old stream must be deleted before the new one is set.
        if( HasStreamName() )
            DelStreamName();

        aGrfObj.SetGraphic( *pGraphic );
        bReadGrf = sal_True;
    }
    else if( pGrfObj && !rGrfName.Len() )
    {
        // MIB 27.02.2001: Old stream must be deleted before the new one is set.
        if( HasStreamName() )
            DelStreamName();

        aGrfObj = *pGrfObj;
        if( pGrfObj->HasUserData() && pGrfObj->IsSwappedOut() )
            aGrfObj.SetSwapState();
        bReadGrf = sal_True;
    }
        // Import einer Grafik:
        // Ist die Grafik bereits geladen?
    else if( !bNewGrf && GRAPHIC_NONE != aGrfObj.GetType() )
        return sal_True;

    else
    {
        if( HasStreamName() )
            DelStreamName();

        // einen neuen Grafik-Link anlegen
        InsertLink( rGrfName, rFltName );

        if( GetNodes().IsDocNodes() )
        {
            if( pGraphic )
            {
                aGrfObj.SetGraphic( *pGraphic, rGrfName );
                bReadGrf = sal_True;
                // Verbindung herstellen ohne ein Update; Grafik haben wir!
                ((SwBaseLink*)&refLink)->Connect();
            }
            else if( pGrfObj )
            {
                aGrfObj = *pGrfObj;
                aGrfObj.SetLink( rGrfName );
                bReadGrf = sal_True;
                // Verbindung herstellen ohne ein Update; Grafik haben wir!
                ((SwBaseLink*)&refLink)->Connect();
            }
            else
            {
                // MIB 25.02.97: Daten der alten Grafik zuruecksetzen, damit
                // die korrekte Ersatz-Darstellung erscheint, wenn die
                // der neue Kink nicht geladen werden konnte.
                Graphic aGrf; aGrf.SetDefaultType();
                aGrfObj.SetGraphic( aGrf, rGrfName );
                // #i59688# - do not load linked graphic, if it isn't a new linked graphic.
                if ( bNewGrf )
                {
                    ((SwBaseLink*)&refLink)->SwapIn();
                }
            }
        }
    }

    // Bug 39281: Size nicht sofort loeschen - Events auf ImageMaps
    //            sollten nicht beim Austauschen nicht ins "leere greifen"
    if( bSetTwipSize )
        SetTwipSize( ::GetGraphicSizeTwip( aGrfObj.GetGraphic(), 0 ) );

    // erzeuge noch einen Update auf die Frames
    if( bReadGrf && bNewGrf )
    {
        SwMsgPoolItem aMsgHint( RES_UPDATE_ATTR );
        ModifyNotification( &aMsgHint, &aMsgHint );
    }

    return bReadGrf;
}


SwGrfNode::~SwGrfNode()
{
    // #i73788#
    mpThreadConsumer.reset();

    SwDoc* pDoc = GetDoc();
    if( refLink.Is() )
    {
        OSL_ENSURE( !bInSwapIn, "DTOR: stehe noch im SwapIn" );
        pDoc->GetLinkManager().Remove( refLink );
        refLink->Disconnect();
    }
    else
    {
        // #i40014# - A graphic node, which are in linked
        // section, whose link is another section is the document, doesn't
        // have to remove the stream from the storage.
        // Because it's hard to detect this case here and it would only fix
        // one problem with shared graphic files - there are also problems,
        // a certain graphic file is referenced by two independent graphic nodes,
        // brush item or drawing objects, the stream isn't no longer removed here.
        // To do this stuff correct, a reference counting on shared streams
        // inside one document have to be implemented.
//        if( !pDoc->IsInDtor() && HasStreamName() )
//          DelStreamName();
    }
    //#39289# Die Frames muessen hier bereits geloescht weil der DTor der
    //Frms die Grafik noch fuer StopAnimation braucht.
    if( GetDepends() )
        DelFrms();
}


SwCntntNode *SwGrfNode::SplitCntntNode( const SwPosition & )
{
    return this;
}


SwGrfNode * SwNodes::MakeGrfNode( const SwNodeIndex & rWhere,
                                const String& rGrfName,
                                const String& rFltName,
                                const Graphic* pGraphic,
                                SwGrfFmtColl* pGrfColl,
                                SwAttrSet* pAutoAttr,
                                sal_Bool bDelayed )
{
    OSL_ENSURE( pGrfColl, "MakeGrfNode: Formatpointer ist 0." );
    SwGrfNode *pNode;
    // Delayed erzeugen nur aus dem SW/G-Reader
    if( bDelayed )
        pNode = new SwGrfNode( rWhere, rGrfName,
                                rFltName, pGrfColl, pAutoAttr );
    else
        pNode = new SwGrfNode( rWhere, rGrfName,
                                rFltName, pGraphic, pGrfColl, pAutoAttr );
    return pNode;
}

SwGrfNode * SwNodes::MakeGrfNode( const SwNodeIndex & rWhere,
                                const GraphicObject& rGrfObj,
                                SwGrfFmtColl* pGrfColl,
                                SwAttrSet* pAutoAttr )
{
    OSL_ENSURE( pGrfColl, "MakeGrfNode: Formatpointer ist 0." );
    return new SwGrfNode( rWhere, rGrfObj, pGrfColl, pAutoAttr );
}


Size SwGrfNode::GetTwipSize() const
{
    return nGrfSize;
}



sal_Bool SwGrfNode::ImportGraphic( SvStream& rStrm )
{
    Graphic aGraphic;
    const String aGraphicURL( aGrfObj.GetUserData() );
    if( !GraphicFilter::GetGraphicFilter().ImportGraphic( aGraphic, aGraphicURL, rStrm ) )
    {
        aGrfObj.SetGraphic( aGraphic );
        aGrfObj.SetUserData( aGraphicURL );
        return sal_True;
    }

    return sal_False;
}

// Returnwert:
// -1 : ReRead erfolgreich
//  0 : nicht geladen
//  1 : Einlesen erfolgreich

short SwGrfNode::SwapIn( sal_Bool bWaitForData )
{
    if( bInSwapIn )                 // nicht rekuriv!!
        return !aGrfObj.IsSwappedOut();

    short nRet = 0;
    bInSwapIn = sal_True;
    SwBaseLink* pLink = (SwBaseLink*)(::sfx2::SvBaseLink*) refLink;

    if( pLink )
    {
        if( GRAPHIC_NONE == aGrfObj.GetType() ||
            GRAPHIC_DEFAULT == aGrfObj.GetType() )
        {
            // noch nicht geladener Link
            //TODO pLink->setInputStream(getInputStream());
            if( pLink->SwapIn( bWaitForData ) )
                nRet = -1;
            else if( GRAPHIC_DEFAULT == aGrfObj.GetType() )
            {
                // keine default Bitmap mehr, also neu Painten!
                aGrfObj.SetGraphic( Graphic() );
                SwMsgPoolItem aMsgHint( RES_GRAPHIC_PIECE_ARRIVED );
                ModifyNotification( &aMsgHint, &aMsgHint );
            }
        }
        else if( aGrfObj.IsSwappedOut() ) {
            // nachzuladender Link
            //TODO pLink->setInputStream(getInputStream());
            nRet = pLink->SwapIn( bWaitForData ) ? 1 : 0;
        }
        else
            nRet = 1;
    }
    else if( aGrfObj.IsSwappedOut() )
    {
        // Die Grafik ist im Storage oder im TempFile drin
        if( !HasStreamName() )
            nRet = (short)aGrfObj.SwapIn();
        else
        {

            // #i48434# - usage of new method <_GetStreamForEmbedGrf(..)>
            try
            {
                // #i53025# - needed correction of new method <_GetStreamForEmbedGrf(..)>
                String aStrmName, aPicStgName;
                _GetStreamStorageNames( aStrmName, aPicStgName );
                uno::Reference < embed::XStorage > refPics = _GetDocSubstorageOrRoot( aPicStgName );
                SvStream* pStrm = _GetStreamForEmbedGrf( refPics, aStrmName );
                if ( pStrm )
                {
                    if ( ImportGraphic( *pStrm ) )
                        nRet = 1;
                    delete pStrm;
                }
            }
            catch (const uno::Exception&)
            {
                // #i48434#
                OSL_FAIL( "<SwGrfNode::SwapIn(..)> - unhandled exception!" );
            }
        }

        if( 1 == nRet )
        {
            SwMsgPoolItem aMsg( RES_GRAPHIC_SWAPIN );
            ModifyNotification( &aMsg, &aMsg );
        }
    }
    else
        nRet = 1;
    OSL_ENSURE( nRet, "Grafik kann nicht eingeswapt werden" );

    if( nRet )
    {
        if( !nGrfSize.Width() && !nGrfSize.Height() )
            SetTwipSize( ::GetGraphicSizeTwip( aGrfObj.GetGraphic(), 0 ) );
    }
    bInSwapIn = sal_False;
    return nRet;
}


short SwGrfNode::SwapOut()
{
    if( aGrfObj.GetType() != GRAPHIC_DEFAULT &&
        aGrfObj.GetType() != GRAPHIC_NONE &&
        !aGrfObj.IsSwappedOut() && !bInSwapIn )
    {
        if( !refLink.Is() )
        {
            // Das Swapping brauchen wir nur fuer Embedded Pictures
            // Die Grafik wird in eine TempFile geschrieben, wenn
            // sie frisch eingefuegt war, d.h. wenn es noch keinen
            // Streamnamen im Storage gibt.
            if( !HasStreamName() )
                if( !aGrfObj.SwapOut() )
                    return 0;
        }
        // Geschriebene Grafiken oder Links werden jetzt weggeschmissen
        return (short) aGrfObj.SwapOut( NULL );
    }
    return 1;
}


sal_Bool SwGrfNode::GetFileFilterNms( String* pFileNm, String* pFilterNm ) const
{
    sal_Bool bRet = sal_False;
    if( refLink.Is() && refLink->GetLinkManager() )
    {
        sal_uInt16 nType = refLink->GetObjType();
        if( OBJECT_CLIENT_GRF == nType )
            bRet = refLink->GetLinkManager()->GetDisplayNames(
                    refLink, 0, pFileNm, 0, pFilterNm );
        else if( OBJECT_CLIENT_DDE == nType && pFileNm && pFilterNm )
        {
            String sApp, sTopic, sItem;
            if( refLink->GetLinkManager()->GetDisplayNames(
                    refLink, &sApp, &sTopic, &sItem ) )
            {
                ( *pFileNm = sApp ) += sfx2::cTokenSeperator;
                ( *pFileNm += sTopic ) += sfx2::cTokenSeperator;
                *pFileNm += sItem;
                pFilterNm->AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DDE" ));
                bRet = sal_True;
            }
        }
    }
    return bRet;
}


// Eine Grafik Undo-faehig machen. Falls sie sich bereits in
// einem Storage befindet, muss sie geladen werden.

sal_Bool SwGrfNode::SavePersistentData()
{
    if( refLink.Is() )
    {
        OSL_ENSURE( !bInSwapIn, "SavePersistentData: stehe noch im SwapIn" );
        GetDoc()->GetLinkManager().Remove( refLink );
        return sal_True;
    }

    // Erst mal reinswappen, falls sie im Storage ist
    if( HasStreamName() && !SwapIn() )
        return sal_False;

    // #i44367#
    // Do not delete graphic file in storage, because the graphic file could
    // be referenced by other graphic nodes.
    // Because it's hard to detect this case here and it would only fix
    // one problem with shared graphic files - there are also problems,
    // a certain graphic file is referenced by two independent graphic nodes,
    // brush item or drawing objects, the stream isn't no longer removed here.
    // To do this stuff correct, a reference counting on shared streams
    // inside one document have to be implemented.
    // Important note: see also fix for #i40014#
//    if( HasStreamName() )
//        DelStreamName();

    // Und in TempFile rausswappen
    return (sal_Bool) SwapOut();
}


sal_Bool SwGrfNode::RestorePersistentData()
{
    if( refLink.Is() )
    {
        IDocumentLinksAdministration* pIDLA = getIDocumentLinksAdministration();
        refLink->SetVisible( pIDLA->IsVisibleLinks() );
        pIDLA->GetLinkManager().InsertDDELink( refLink );
        if( getIDocumentLayoutAccess()->GetCurrentLayout() )    //swmod 080218
            refLink->Update();
    }
    return sal_True;
}


void SwGrfNode::InsertLink( const String& rGrfName, const String& rFltName )
{
    refLink = new SwBaseLink( sfx2::LINKUPDATE_ONCALL, FORMAT_GDIMETAFILE, this );

    IDocumentLinksAdministration* pIDLA = getIDocumentLinksAdministration();
    if( GetNodes().IsDocNodes() )
    {
        refLink->SetVisible( pIDLA->IsVisibleLinks() );
        if( rFltName.EqualsAscii( "DDE" ))
        {
            sal_uInt16 nTmp = 0;
            String sApp, sTopic, sItem;
            sApp = rGrfName.GetToken( 0, sfx2::cTokenSeperator, nTmp );
            sTopic = rGrfName.GetToken( 0, sfx2::cTokenSeperator, nTmp );
            sItem = rGrfName.Copy( nTmp );
            pIDLA->GetLinkManager().InsertDDELink( refLink,
                                            sApp, sTopic, sItem );
        }
        else
        {
            sal_Bool bSync = rFltName.EqualsAscii( "SYNCHRON" );
            refLink->SetSynchron( bSync );
            refLink->SetContentType( SOT_FORMATSTR_ID_SVXB );

            pIDLA->GetLinkManager().InsertFileLink( *refLink,
                                            OBJECT_CLIENT_GRF, rGrfName,
                                (!bSync && rFltName.Len() ? &rFltName : 0) );
        }
    }
    aGrfObj.SetLink( rGrfName );
}


void SwGrfNode::ReleaseLink()
{
    if( refLink.Is() )
    {
        {
            bInSwapIn = sal_True;
            SwBaseLink* pLink = (SwBaseLink*)(::sfx2::SvBaseLink*) refLink;
            //TODO pLink->setInputStream(getInputStream());
            pLink->SwapIn( sal_True, sal_True );
            bInSwapIn = sal_False;
        }
        getIDocumentLinksAdministration()->GetLinkManager().Remove( refLink );
        refLink.Clear();
        aGrfObj.SetLink();
    }
}


void SwGrfNode::SetTwipSize( const Size& rSz )
{
    nGrfSize = rSz;
    if( IsScaleImageMap() && nGrfSize.Width() && nGrfSize.Height() )
    {
        // Image-Map an Grafik-Groesse anpassen
        ScaleImageMap();

        // Image-Map nicht noch einmal skalieren
        SetScaleImageMap( sal_False );
    }
}

void SwGrfNode::ScaleImageMap()
{
    if( !nGrfSize.Width() || !nGrfSize.Height() )
        return;

    // dann die Image-Map skalieren
    SwFrmFmt* pFmt = GetFlyFmt();

    if( !pFmt )
        return;

    SwFmtURL aURL( pFmt->GetURL() );
    if ( !aURL.GetMap() )
        return;

    sal_Bool bScale = sal_False;
    Fraction aScaleX( 1, 1 );
    Fraction aScaleY( 1, 1 );

    const SwFmtFrmSize& rFrmSize = pFmt->GetFrmSize();
    const SvxBoxItem& rBox = pFmt->GetBox();

    if( !rFrmSize.GetWidthPercent() )
    {
        SwTwips nWidth = rFrmSize.GetWidth();

        nWidth -= rBox.CalcLineSpace(BOX_LINE_LEFT) +
                  rBox.CalcLineSpace(BOX_LINE_RIGHT);

        OSL_ENSURE( nWidth>0, "Gibt es 0 twip breite Grafiken!?" );

        if( nGrfSize.Width() != nWidth )
        {
            aScaleX = Fraction( nGrfSize.Width(), nWidth );
            bScale = sal_True;
        }
    }
    if( !rFrmSize.GetHeightPercent() )
    {
        SwTwips nHeight = rFrmSize.GetHeight();

        nHeight -= rBox.CalcLineSpace(BOX_LINE_TOP) +
                   rBox.CalcLineSpace(BOX_LINE_BOTTOM);

        OSL_ENSURE( nHeight>0, "Gibt es 0 twip hohe Grafiken!?" );

        if( nGrfSize.Height() != nHeight )
        {
            aScaleY = Fraction( nGrfSize.Height(), nHeight );
            bScale = sal_True;
        }
    }

    if( bScale )
    {
        aURL.GetMap()->Scale( aScaleX, aScaleY );
        pFmt->SetFmtAttr( aURL );
    }
}


void SwGrfNode::DelStreamName()
{
    if( HasStreamName() )
    {
        // Dann die Grafik im Storage loeschen
        uno::Reference < embed::XStorage > xDocStg = GetDoc()->GetDocStorage();
        if( xDocStg.is() )
        {
            try
            {
                String aPicStgName, aStrmName;
                _GetStreamStorageNames( aStrmName, aPicStgName );
                uno::Reference < embed::XStorage > refPics = xDocStg;
                if ( aPicStgName.Len() )
                    refPics = xDocStg->openStorageElement( aPicStgName, embed::ElementModes::READWRITE );
                refPics->removeElement( aStrmName );
                uno::Reference < embed::XTransactedObject > xTrans( refPics, uno::UNO_QUERY );
                if ( xTrans.is() )
                    xTrans->commit();
            }
            catch (const uno::Exception&)
            {
                // #i48434#
                OSL_FAIL( "<SwGrfNode::DelStreamName()> - unhandled exception!" );
            }
        }

        aGrfObj.SetUserData();
    }
}

/** helper method to get a substorage of the document storage for readonly access.

    OD, MAV 2005-08-17 #i53025#
    A substorage with the specified name will be opened readonly. If the provided
    name is empty the root storage will be returned.
*/
uno::Reference< embed::XStorage > SwGrfNode::_GetDocSubstorageOrRoot( const String& aStgName ) const
{
    uno::Reference < embed::XStorage > refStor =
        const_cast<SwGrfNode*>(this)->GetDoc()->GetDocStorage();
    OSL_ENSURE( refStor.is(), "Kein Storage am Doc" );

    if ( aStgName.Len() )
    {
        if( refStor.is() )
            return refStor->openStorageElement( aStgName, embed::ElementModes::READ );
    }

    return refStor;
}

/** helper method to determine stream for the embedded graphic.

    OD 2005-05-04 #i48434#
    Important note: caller of this method has to handle the thrown exceptions
    OD, MAV 2005-08-17 #i53025#
    Storage, which should contain the stream of the embedded graphic, is
    provided via parameter. Otherwise the returned stream will be closed
    after the the method returns, because its parent stream is closed and deleted.
    Proposed name of embedded graphic stream is also provided by parameter.

    @author OD
*/
SvStream* SwGrfNode::_GetStreamForEmbedGrf(
            const uno::Reference< embed::XStorage >& _refPics,
            String& _aStrmName ) const
{
    SvStream* pStrm( 0L );

    if( _refPics.is() && _aStrmName.Len() )
    {
        // If stream doesn't exist in the storage, try access the graphic file by
        // re-generating its name.
        // A save action can have changed the filename of the embedded graphic,
        // because a changed unique ID of the graphic is calculated.
        // --> recursive calls of <GetUniqueID()> have to be avoided.
        // Thus, use local static boolean to assure this.
        if ( !_refPics->hasByName( _aStrmName ) ||
               !_refPics->isStreamElement( _aStrmName ) )
        {
            xub_StrLen nExtPos = _aStrmName.Search( '.' );
            String aExtStr = _aStrmName.Copy( nExtPos );
            if ( GetGrfObj().GetType() != GRAPHIC_NONE )
            {
                _aStrmName = rtl::OStringToOUString(GetGrfObj().GetUniqueID(),
                    RTL_TEXTENCODING_ASCII_US);
                _aStrmName += aExtStr;
            }
        }

        // assure that graphic file exist in the storage.
        if ( _refPics->hasByName( _aStrmName ) &&
             _refPics->isStreamElement( _aStrmName ) )
        {
            uno::Reference < io::XStream > refStrm = _refPics->openStreamElement( _aStrmName, embed::ElementModes::READ );
            pStrm = utl::UcbStreamHelper::CreateStream( refStrm );
        }
        else
        {
            OSL_FAIL( "<SwGrfNode::_GetStreamForEmbedGrf(..)> - embedded graphic file not found!" );
        }
    }

    return pStrm;
}


// #i53025# - stream couldn't be in a 3.1 - 5.2 storage any more.
// Thus, removing corresponding code.
void SwGrfNode::_GetStreamStorageNames( String& rStrmName,
                                        String& rStorName ) const
{
    rStorName.Erase();
    rStrmName.Erase();

    String aUserData( aGrfObj.GetUserData() );
    if( !aUserData.Len() )
        return;

    if (aNewStrmName.Len()>0) {
        aUserData=aNewStrmName;
    }

    String aProt( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package:" ) );
    if( 0 == aUserData.CompareTo( aProt, aProt.Len() ) )
    {
        // 6.0 (XML) Package
        xub_StrLen nPos = aUserData.Search( '/' );
        if( STRING_NOTFOUND == nPos )
        {
            rStrmName = aUserData.Copy( aProt.Len() );
        }
        else
        {
            xub_StrLen nPathStart = aProt.Len();
            if( 0 == aUserData.CompareToAscii( "./", 2 ) )
                nPathStart += 2;
            rStorName = aUserData.Copy( nPathStart, nPos-nPathStart );
            rStrmName = aUserData.Copy( nPos+1 );
        }
    }
    else
    {
        OSL_FAIL( "<SwGrfNode::_GetStreamStorageNames(..)> - unknown graphic URL type. Code for handling 3.1 - 5.2 storages has been deleted by issue i53025." );
    }
    OSL_ENSURE( STRING_NOTFOUND == rStrmName.Search( '/' ),
            "invalid graphic stream name" );
}

SwCntntNode* SwGrfNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // kopiere die Formate in das andere Dokument:
    SwGrfFmtColl* pColl = pDoc->CopyGrfColl( *GetGrfColl() );

    Graphic aTmpGrf;
    SwBaseLink* pLink = (SwBaseLink*)(::sfx2::SvBaseLink*) refLink;
    if( !pLink && HasStreamName() )
    {
        // #i48434# - usage of new method <_GetStreamForEmbedGrf(..)>
        try
        {
            // #i53025# - needed correction of new method <_GetStreamForEmbedGrf(..)>
            String aStrmName, aPicStgName;
            _GetStreamStorageNames( aStrmName, aPicStgName );
            uno::Reference < embed::XStorage > refPics = _GetDocSubstorageOrRoot( aPicStgName );
            SvStream* pStrm = _GetStreamForEmbedGrf( refPics, aStrmName );
            if ( pStrm )
            {
                const String aGraphicURL( aGrfObj.GetUserData() );
                GraphicFilter::GetGraphicFilter().ImportGraphic( aTmpGrf, aGraphicURL, *pStrm );
                delete pStrm;
            }
        }
        catch (const uno::Exception& e)
        {
            // #i48434#
            SAL_WARN("sw.core", "<SwGrfNode::MakeCopy(..)> - unhandled exception!" << e.Message);
        }
    }
    else
    {
        if( aGrfObj.IsSwappedOut() )
            const_cast<SwGrfNode*>(this)->SwapIn();
        aTmpGrf = aGrfObj.GetGraphic();
    }

    const sfx2::LinkManager& rMgr = getIDocumentLinksAdministration()->GetLinkManager();
    String sFile, sFilter;
    if( IsLinkedFile() )
        rMgr.GetDisplayNames( refLink, 0, &sFile, 0, &sFilter );
    else if( IsLinkedDDE() )
    {
        String sTmp1, sTmp2;
        rMgr.GetDisplayNames( refLink, &sTmp1, &sTmp2, &sFilter );
        sfx2::MakeLnkName( sFile, &sTmp1, sTmp2, sFilter );
        sFilter.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DDE" ));
    }

    SwGrfNode* pGrfNd = pDoc->GetNodes().MakeGrfNode( rIdx, sFile, sFilter,
                                                    &aTmpGrf, pColl,
                                            (SwAttrSet*)GetpSwAttrSet() );
    pGrfNd->SetTitle( GetTitle() );
    pGrfNd->SetDescription( GetDescription() );
    pGrfNd->SetContour( HasContour(), HasAutomaticContour() );
    return pGrfNd;
}

IMPL_LINK( SwGrfNode, SwapGraphic, GraphicObject*, pGrfObj )
{
    SvStream* pRet;

    // Keep graphic while in swap in. That's at least important
    // when breaking links, because in this situation a reschedule call and
    // a DataChanged call lead to a paint of the graphic.
    if( pGrfObj->IsInSwapOut() && (IsSelected() || bInSwapIn) )
        pRet = GRFMGR_AUTOSWAPSTREAM_NONE;
    else if( refLink.Is() )
    {
        if( pGrfObj->IsInSwapIn() )
        {
            // then make it by your self
            if( !bInSwapIn )
            {
                sal_Bool bIsModifyLocked = IsModifyLocked();
                LockModify();
                SwapIn( sal_False );
                if( !bIsModifyLocked )
                    UnlockModify();
            }
            pRet = GRFMGR_AUTOSWAPSTREAM_NONE;
        }
        else
            pRet = GRFMGR_AUTOSWAPSTREAM_LINK;
    }
    else
    {
        pRet = GRFMGR_AUTOSWAPSTREAM_TEMP;

        if( HasStreamName() )
        {
            // #i48434# - usage of new method <_GetStreamForEmbedGrf(..)>
            try
            {
                // #i53025# - needed correction of new method <_GetStreamForEmbedGrf(..)>
                String aStrmName, aPicStgName;
                _GetStreamStorageNames( aStrmName, aPicStgName );
                uno::Reference < embed::XStorage > refPics = _GetDocSubstorageOrRoot( aPicStgName );
                SvStream* pStrm = _GetStreamForEmbedGrf( refPics, aStrmName );
                if ( pStrm )
                {
                    if( pGrfObj->IsInSwapOut() )
                    {
                        pRet = GRFMGR_AUTOSWAPSTREAM_LINK;
                    }
                    else
                    {
                        ImportGraphic( *pStrm );
                        pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
                    }
                    delete pStrm;
                }
            }
            catch (const uno::Exception&)
            {
                // #i48434#
                OSL_FAIL( "<SwapGraphic> - unhandled exception!" );
            }
        }
    }

    return (long)pRet;
}


// alle QuickDraw-Bitmaps eines speziellen Docs loeschen
void DelAllGrfCacheEntries( SwDoc* pDoc )
{
    if( pDoc )
    {
        // alle Graphic-Links mit dem Namen aus dem Cache loeschen
        const sfx2::LinkManager& rLnkMgr = pDoc->GetLinkManager();
        const ::sfx2::SvBaseLinks& rLnks = rLnkMgr.GetLinks();
        SwGrfNode* pGrfNd;
        String sFileNm;
        for( sal_uInt16 n = rLnks.size(); n; )
        {
            ::sfx2::SvBaseLink* pLnk = &(*rLnks[ --n ]);
            if( pLnk && OBJECT_CLIENT_GRF == pLnk->GetObjType() &&
                rLnkMgr.GetDisplayNames( pLnk, 0, &sFileNm ) &&
                pLnk->ISA( SwBaseLink ) && 0 != ( pGrfNd =
                ((SwBaseLink*)pLnk)->GetCntntNode()->GetGrfNode()) )
            {
                pGrfNd->GetGrfObj().ReleaseFromCache();
            }
        }
    }
}

// returns the with our graphic attributes filled Graphic-Attr-Structure
GraphicAttr& SwGrfNode::GetGraphicAttr( GraphicAttr& rGA,
                                        const SwFrm* pFrm ) const
{
    const SwAttrSet& rSet = GetSwAttrSet();

    rGA.SetDrawMode( (GraphicDrawMode)rSet.GetDrawModeGrf().GetValue() );

    const SwMirrorGrf & rMirror = rSet.GetMirrorGrf();
    sal_uLong nMirror = BMP_MIRROR_NONE;
    if( rMirror.IsGrfToggle() && pFrm && !pFrm->FindPageFrm()->OnRightPage() )
    {
        switch( rMirror.GetValue() )
        {
        case RES_MIRROR_GRAPH_DONT:     nMirror = BMP_MIRROR_HORZ; break;
        case RES_MIRROR_GRAPH_VERT:     nMirror = BMP_MIRROR_NONE; break;
        case RES_MIRROR_GRAPH_HOR:  nMirror = BMP_MIRROR_HORZ|BMP_MIRROR_VERT;
                                    break;
        default:                    nMirror = BMP_MIRROR_VERT; break;
        }
    }
    else
        switch( rMirror.GetValue() )
        {
        case RES_MIRROR_GRAPH_BOTH:     nMirror = BMP_MIRROR_HORZ|BMP_MIRROR_VERT;
                                    break;
        case RES_MIRROR_GRAPH_VERT: nMirror = BMP_MIRROR_HORZ; break;
        case RES_MIRROR_GRAPH_HOR:  nMirror = BMP_MIRROR_VERT; break;
        }

    rGA.SetMirrorFlags( nMirror );

    const SwCropGrf& rCrop = rSet.GetCropGrf();
    rGA.SetCrop( TWIP_TO_MM100( rCrop.GetLeft() ),
                 TWIP_TO_MM100( rCrop.GetTop() ),
                 TWIP_TO_MM100( rCrop.GetRight() ),
                 TWIP_TO_MM100( rCrop.GetBottom() ));

    const SwRotationGrf& rRotation = rSet.GetRotationGrf();
    rGA.SetRotation( rRotation.GetValue() );

    rGA.SetLuminance( rSet.GetLuminanceGrf().GetValue() );
    rGA.SetContrast( rSet.GetContrastGrf().GetValue() );
    rGA.SetChannelR( rSet.GetChannelRGrf().GetValue() );
    rGA.SetChannelG( rSet.GetChannelGGrf().GetValue() );
    rGA.SetChannelB( rSet.GetChannelBGrf().GetValue() );
    rGA.SetGamma( rSet.GetGammaGrf().GetValue() );
    rGA.SetInvert( rSet.GetInvertGrf().GetValue() );

    const sal_uInt16 nTrans = rSet.GetTransparencyGrf().GetValue();
    rGA.SetTransparency( (sal_uInt8) FRound(
                                Min( nTrans, (sal_uInt16) 100 )  * 2.55 ) );

    return rGA;
}

sal_Bool SwGrfNode::IsTransparent() const
{
    sal_Bool bRet = aGrfObj.IsTransparent();
    if( !bRet ) // ask the attribut
        bRet = 0 != GetSwAttrSet().GetTransparencyGrf().GetValue();

    return bRet;
}


sal_Bool SwGrfNode::IsSelected() const
{
    sal_Bool bRet = sal_False;
    const SwEditShell* pESh = GetDoc()->GetEditShell();
    if( pESh )
    {
        const SwNode* pN = this;
        const ViewShell* pV = pESh;
        do {
            if( pV->ISA( SwEditShell ) && pN == &((SwCrsrShell*)pV)
                                ->GetCrsr()->GetPoint()->nNode.GetNode() )
            {
                bRet = sal_True;
                break;
            }
        }
        while( pESh != ( pV = (ViewShell*)pV->GetNext() ));
    }
    return bRet;
}

// #i73788#
boost::weak_ptr< SwAsyncRetrieveInputStreamThreadConsumer > SwGrfNode::GetThreadConsumer()
{
    return mpThreadConsumer;
}

void SwGrfNode::TriggerAsyncRetrieveInputStream()
{
    if ( !IsLinkedFile() )
    {
        OSL_FAIL( "<SwGrfNode::TriggerAsyncLoad()> - Method is misused. Method call is only valid for graphic nodes, which refer a linked graphic file" );
        return;
    }

    if ( mpThreadConsumer.get() == 0 )
    {
        mpThreadConsumer.reset( new SwAsyncRetrieveInputStreamThreadConsumer( *this ) );

        String sGrfNm;
        refLink->GetLinkManager()->GetDisplayNames( refLink, 0, &sGrfNm, 0, 0 );

        mpThreadConsumer->CreateThread( sGrfNm );
    }
}

bool SwGrfNode::IsLinkedInputStreamReady() const
{
    return mbLinkedInputStreamReady;
}

void SwGrfNode::ApplyInputStream(
    com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream,
    const sal_Bool bIsStreamReadOnly )
{
    if ( IsLinkedFile() )
    {
        if ( xInputStream.is() )
        {
            mxInputStream = xInputStream;
            mbIsStreamReadOnly = bIsStreamReadOnly;
            mbLinkedInputStreamReady = true;
            SwMsgPoolItem aMsgHint( RES_LINKED_GRAPHIC_STREAM_ARRIVED );
            ModifyNotification( &aMsgHint, &aMsgHint );
        }
    }
}

void SwGrfNode::UpdateLinkWithInputStream()
{
    // --> OD #i85105#
    // do not work on link, if a <SwapIn> has been triggered.
    if ( !bInSwapIn && IsLinkedFile() )
    // <--
    {
        GetLink()->setStreamToLoadFrom( mxInputStream, mbIsStreamReadOnly );
        GetLink()->Update();
        SwMsgPoolItem aMsgHint( RES_GRAPHIC_ARRIVED );
        ModifyNotification( &aMsgHint, &aMsgHint );

        // #i88291#
        mxInputStream.clear();
        GetLink()->clearStreamToLoadFrom();
        mbLinkedInputStreamReady = false;
        mpThreadConsumer.reset();
    }
}

// #i90395#
bool SwGrfNode::IsAsyncRetrieveInputStreamPossible() const
{
    bool bRet = false;

    if ( IsLinkedFile() )
    {
        String sGrfNm;
        refLink->GetLinkManager()->GetDisplayNames( refLink, 0, &sGrfNm, 0, 0 );
        String sProtocol( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.pkg:" ) );
        if ( sGrfNm.CompareTo( sProtocol, sProtocol.Len() ) != 0 )
        {
            bRet = true;
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
