/*************************************************************************
 *
 *  $RCSfile: ndgrf.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: mib $ $Date: 2001-04-06 10:56:07 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>             // FRound
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _UNDO_HXX //autogen
#include <svtools/undo.hxx>
#endif
#ifndef SVTOOLS_FSTATHELPER_HXX
#include <svtools/fstathelper.hxx>
#endif
#ifndef _IMAP_HXX //autogen
#include <svtools/imap.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SVXLINKMGR_HXX
#include <svx/linkmgr.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX //autogen
#include <svx/impgrf.hxx>
#endif


#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>
#endif
#ifndef _SWBASLNK_HXX
#include <swbaslnk.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif

// --------------------
// SwGrfNode
// --------------------
SwGrfNode::SwGrfNode( const SwNodeIndex & rWhere,
                  const String& rGrfName, const String& rFltName,
                  const Graphic* pGraphic,
                  SwGrfFmtColl *pGrfColl,
                  SwAttrSet* pAutoAttr )
    : SwNoTxtNode( rWhere, ND_GRFNODE, pGrfColl, pAutoAttr )
{
    aGrfObj.SetSwapStreamHdl( LINK( this, SwGrfNode, SwapGraphic ) );
    bInSwapIn = bChgTwipSize = bLoadLowResGrf = bFrameInPaint =
        bScaleImageMap = FALSE;
    bGrafikArrived = TRUE;
    ReRead( rGrfName, rFltName, pGraphic, 0, FALSE );
}

SwGrfNode::SwGrfNode( const SwNodeIndex & rWhere,
                          const GraphicObject& rGrfObj,
                          SwGrfFmtColl *pGrfColl, SwAttrSet* pAutoAttr )
    : SwNoTxtNode( rWhere, ND_GRFNODE, pGrfColl, pAutoAttr )
{
    aGrfObj = rGrfObj;
    aGrfObj.SetSwapStreamHdl( LINK( this, SwGrfNode, SwapGraphic ) );
    if( rGrfObj.HasUserData() && rGrfObj.IsSwappedOut() )
        aGrfObj.SetSwapState();
    bInSwapIn = bChgTwipSize = bLoadLowResGrf = bFrameInPaint =
        bScaleImageMap = FALSE;
    bGrafikArrived = TRUE;
}

// Konstruktor fuer den SW/G-Reader. Dieser ctor wird verwendet,
// wenn eine gelinkte Grafik gelesen wird. Sie liest diese NICHT ein.


SwGrfNode::SwGrfNode( const SwNodeIndex & rWhere,
                  const String& rGrfName, const String& rFltName,
                  SwGrfFmtColl *pGrfColl,
                  SwAttrSet* pAutoAttr )
    : SwNoTxtNode( rWhere, ND_GRFNODE, pGrfColl, pAutoAttr )
{
    aGrfObj.SetSwapStreamHdl( LINK( this, SwGrfNode, SwapGraphic ) );

    Graphic aGrf; aGrf.SetDefaultType();
    aGrfObj.SetGraphic( aGrf, rGrfName );

    bInSwapIn = bChgTwipSize = bLoadLowResGrf = bFrameInPaint =
        bScaleImageMap = FALSE;
    bGrafikArrived = TRUE;

    InsertLink( rGrfName, rFltName );
    if( IsLinkedFile() )
    {
        INetURLObject aUrl( rGrfName );
        if( INET_PROT_FILE == aUrl.GetProtocol() &&
            FStatHelper::IsDocument( aUrl.GetMainURL() ))
        {
            // File vorhanden, Verbindung herstellen ohne ein Update
            ((SwBaseLink*)&refLink)->Connect();
        }
    }
}


// erneutes Einlesen, falls Graphic nicht Ok ist. Die
// aktuelle wird durch die neue ersetzt.

BOOL SwGrfNode::ReRead( const String& rGrfName, const String& rFltName,
                        const Graphic* pGraphic, const GraphicObject* pGrfObj,
                        BOOL bNewGrf )
{
    BOOL bReadGrf = FALSE, bSetTwipSize = TRUE;

    ASSERT( pGraphic || pGrfObj || rGrfName.Len(),
            "GraphicNode without a name, Graphic or GraphicObject" );

    // ReadRead mit Namen
    if( refLink.Is() )
    {
        ASSERT( !bInSwapIn, "ReRead: stehe noch im SwapIn" );
        if( rGrfName.Len() )
        {
            // Besonderheit: steht im FltNamen DDE, handelt es sich um eine
            //                  DDE-gelinkte Grafik
            String sCmd( rGrfName );
            if( rFltName.Len() )
            {
                USHORT nNewType;
                if( rFltName.EqualsAscii( "DDE" ))
                    nNewType = OBJECT_CLIENT_DDE;
                else
                {
                    ::MakeLnkName( sCmd, 0, rGrfName, aEmptyStr, &rFltName );
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
            bReadGrf = TRUE;
        }
        else if( pGrfObj )
        {
            aGrfObj = *pGrfObj;
            if( pGrfObj->HasUserData() && pGrfObj->IsSwappedOut() )
                aGrfObj.SetSwapState();
            aGrfObj.SetLink( rGrfName );
            bReadGrf = TRUE;
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
                if( GetFrm() )
                {
                    SwMsgPoolItem aMsgHint( RES_GRF_REREAD_AND_INCACHE );
                    Modify( &aMsgHint, &aMsgHint );
                }
                else
                    ((SwBaseLink*)&refLink)->SwapIn();
            }
            bSetTwipSize = FALSE;
        }
    }
    else if( pGraphic && !rGrfName.Len() )
    {
        // MIB 27.02.2001: Old stream must be deleted before the new one is set.
        if( HasStreamName() )
            DelStreamName();

        aGrfObj.SetGraphic( *pGraphic );
        bReadGrf = TRUE;
    }
    else if( pGrfObj && !rGrfName.Len() )
    {
        // MIB 27.02.2001: Old stream must be deleted before the new one is set.
        if( HasStreamName() )
            DelStreamName();

        aGrfObj = *pGrfObj;
        if( pGrfObj->HasUserData() && pGrfObj->IsSwappedOut() )
            aGrfObj.SetSwapState();
        bReadGrf = TRUE;
    }
        // Import einer Grafik:
        // Ist die Grafik bereits geladen?
    else if( !bNewGrf && GRAPHIC_NONE != aGrfObj.GetType() )
        return TRUE;

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
                bReadGrf = TRUE;
                // Verbindung herstellen ohne ein Update; Grafik haben wir!
                ((SwBaseLink*)&refLink)->Connect();
            }
            else if( pGrfObj )
            {
                aGrfObj = *pGrfObj;
                aGrfObj.SetLink( rGrfName );
                bReadGrf = TRUE;
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
                ((SwBaseLink*)&refLink)->SwapIn();
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
        Modify( &aMsgHint, &aMsgHint );
    }

    return bReadGrf;
}


SwGrfNode::~SwGrfNode()
{
    SwDoc* pDoc = GetDoc();
    if( refLink.Is() )
    {
        ASSERT( !bInSwapIn, "DTOR: stehe noch im SwapIn" );
        pDoc->GetLinkManager().Remove( refLink );
        refLink->Disconnect();
    }
    else
    {
        if( !pDoc->IsInDtor() && HasStreamName() )
            DelStreamName();
    }
    //#39289# Die Frames muessen hier bereits geloescht weil der DTor der
    //Frms die Grafik noch fuer StopAnimation braucht.
    if( GetDepends() )
        DelFrms();
}


SwCntntNode *SwGrfNode::SplitNode( const SwPosition &rPos )
{
    return this;
}


SwGrfNode * SwNodes::MakeGrfNode( const SwNodeIndex & rWhere,
                                const String& rGrfName,
                                const String& rFltName,
                                const Graphic* pGraphic,
                                SwGrfFmtColl* pGrfColl,
                                SwAttrSet* pAutoAttr,
                                BOOL bDelayed )
{
    ASSERT( pGrfColl, "MakeGrfNode: Formatpointer ist 0." );
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
    ASSERT( pGrfColl, "MakeGrfNode: Formatpointer ist 0." );
    return new SwGrfNode( rWhere, rGrfObj, pGrfColl, pAutoAttr );
}


Size SwGrfNode::GetTwipSize() const
{
    return nGrfSize;
}



BOOL SwGrfNode::ImportGraphic( SvStream& rStrm )
{
    Graphic aGraphic;
    if( !GetGrfFilter()->ImportGraphic( aGraphic, String(), rStrm ) )
    {
        const String aUserData( aGrfObj.GetUserData() );

        aGrfObj.SetGraphic( aGraphic );
        aGrfObj.SetUserData( aUserData );
        return TRUE;
    }

    return FALSE;
}

// Returnwert:
// -1 : ReRead erfolgreich
//  0 : nicht geladen
//  1 : Einlesen erfolgreich

short SwGrfNode::SwapIn( BOOL bWaitForData )
{
    if( bInSwapIn )                 // nicht rekuriv!!
        return !aGrfObj.IsSwappedOut();

    short nRet = 0;
    bInSwapIn = TRUE;
    SwBaseLink* pLink = (SwBaseLink*)(::so3::SvBaseLink*) refLink;
    if( pLink )
    {
        if( GRAPHIC_NONE == aGrfObj.GetType() ||
            GRAPHIC_DEFAULT == aGrfObj.GetType() )
        {
            // noch nicht geladener Link
            if( pLink->SwapIn( bWaitForData ) )
                nRet = -1;
            else if( GRAPHIC_DEFAULT == aGrfObj.GetType() )
            {
                // keine default Bitmap mehr, also neu Painten!
                aGrfObj.SetGraphic( Graphic() );
                SwMsgPoolItem aMsgHint( RES_GRAPHIC_PIECE_ARRIVED );
                Modify( &aMsgHint, &aMsgHint );
            }
        }
        else if( aGrfObj.IsSwappedOut() )
            // nachzuladender Link
            nRet = pLink->SwapIn( bWaitForData ) ? 1 : 0;
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
            SvStorageRef refRoot = GetDoc()->GetDocStorage();
            ASSERT( refRoot.Is(), "Kein Storage am Doc" );
            if( refRoot.Is() )
            {
                String aStrmName, aPicStgName;
                BOOL bGraphic = GetStreamStorageNames( aStrmName, aPicStgName );
                SvStorageRef refPics = aPicStgName.Len()
                       ? refRoot->OpenStorage( aPicStgName,
                        STREAM_READ | STREAM_SHARE_DENYWRITE )
                    : &refRoot;
                if( refPics->GetError() == SVSTREAM_OK )
                {
                    SvStorageStreamRef refStrm =
                        refPics->OpenStream( aStrmName,
                        STREAM_READ | STREAM_SHARE_DENYWRITE );
                    if( refStrm->GetError() == SVSTREAM_OK )
                    {
                        refStrm->SetVersion( refRoot->GetVersion() );
                        if( bGraphic ? aGrfObj.SwapIn( refStrm )
                                     : ImportGraphic( *refStrm ) )
                            nRet = 1;
                    }
                }
            }
        }
        if( 1 == nRet )
        {
            SwMsgPoolItem aMsg( RES_GRAPHIC_SWAPIN );
            SwCntntNode::Modify( &aMsg, &aMsg );
        }
    }
    else
        nRet = 1;
    DBG_ASSERTWARNING( nRet, "Grafik kann nicht eingeswapt werden" );

    if( nRet )
    {
        if( !nGrfSize.Width() && !nGrfSize.Height() )
            SetTwipSize( ::GetGraphicSizeTwip( aGrfObj.GetGraphic(), 0 ) );
    }
    bInSwapIn = FALSE;
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

// Wird nach einem SaveAs aufgerufen und setzt die StreamNamen um


void SwGrfNode::SaveCompleted( BOOL bClear )
{
    if( aNewStrmName.Len() )
    {
        if( !bClear )       // der Name wird zum aktuellen
            SetStreamName( aNewStrmName );
        aNewStrmName.Erase();
    }
}


// Falls die Grafik noch nicht im Doc-Storage existiert,
// wird sie neu geschrieben; falls sie bereits drin ist,
// wird nicht geschrieben. Wenn der Storage nicht dem
// Doc-Storage entspricht, wird, falls aNewStrmName nicht
// besetzt ist, in diesem Storage unter dem angegebenen
// Streamnamen abgelegt (SaveAs). nach einem SaveAs wird
// vom SW3-I/O-System noch SaveCompleted() aufgerufen,
// da nun der Doc-Storage dem neuen Storage entspricht.
// MIB 02/28/2001: This method is called only to store graphics
// in the 3.1 to 5.0 formats. For the 6.0 format, graphics
// are exported using the SvXMLGraphicObjectHelper class.


BOOL SwGrfNode::StoreGraphics( SvStorage* pRoot )
{
    if( !refLink.Is() )
    {
        BOOL bGraphic = TRUE; // Does the graphic stream (if it exists)
                              // contain a streamed graphic (TRUE) or the
                              // raw image data only (FALSE)
        String aSrcStrmName, aSrcPicStgName;
        if( HasStreamName() )
            bGraphic = GetStreamStorageNames( aSrcStrmName, aSrcPicStgName );
        SvStorage* pDocStg = GetDoc()->GetDocStorage();
        if( !pRoot )
            pRoot = pDocStg;
        ASSERT( SOFFICE_FILEFORMAT_60 > pRoot->GetVersion(),
                "SwGrfNode::StoreGraphic called for 6.0+ file format" );

        String aDstPicStgName(
                RTL_CONSTASCII_STRINGPARAM( "EmbeddedPictures" ) );
        String aDstStrmName( aSrcStrmName );
        if( pRoot != pDocStg || !bGraphic )
        {
            // If the stream does not contain a streamed graphic object,
            // the graphic has to be stored again.
            ASSERT( pRoot != pDocStg || aSrcStrmName.Len(),
                    "raw image data stream but no stream name" );
            // Neuer Storage. Wenn die Grafik im DocStg drin ist,
            // kann sie bequem per CopyTo() kopiert werden.
            if( aSrcStrmName.Len() )
            {
                SvStorageRef refSrcPics = aSrcPicStgName.Len()
                    ? pDocStg->OpenStorage( aSrcPicStgName,
                        STREAM_READ | STREAM_SHARE_DENYWRITE )
                    : pDocStg;

                SvStorageStreamRef refStrm;

                BOOL bWriteNew = pDocStg->GetVersion() != pRoot->GetVersion() ||
                                 !bGraphic;
                if( !bWriteNew &&
                    SOFFICE_FILEFORMAT_40 <= pRoot->GetVersion() )
                {
                    refStrm = refSrcPics->OpenStream( aSrcStrmName,
                                    STREAM_READ | STREAM_SHARE_DENYWRITE );
                    if( SVSTREAM_OK == refStrm->GetError() )
                    {
                        // JP 21.06.98: pruefe ob der CompressMode uebereinstimmt
                        USHORT nCmprsMode =
                                Graphic::GetGraphicsCompressMode(*refStrm ) &
                                ~(COMPRESSMODE_ZBITMAP|COMPRESSMODE_NATIVE );
                        USHORT nNewCmprsMode = 0;
                        if( GRAPHIC_BITMAP == aGrfObj.GetType() &&
                            GetDoc()->GetInfo()->IsSaveGraphicsCompressed() )
                            nNewCmprsMode |= COMPRESSMODE_ZBITMAP;
                        if( SOFFICE_FILEFORMAT_40 < pRoot->GetVersion() &&
                            GetDoc()->GetInfo()->IsSaveOriginalGraphics() )
                            nNewCmprsMode |= COMPRESSMODE_NATIVE;

                        if( nCmprsMode != nNewCmprsMode )
                        {
                            // der Kompressedmode stimmt nicht, also muss
                            // ggfs. die Grafik reingeswappt und ueber den
                            // unteren Teil neu geschrieben werden.
                            bWriteNew = TRUE;
                            refStrm->Seek( STREAM_SEEK_TO_BEGIN );
                        }
                    }
                }

                if( bWriteNew )
                {
                    if( aGrfObj.IsSwappedOut() &&
                        SVSTREAM_OK == refSrcPics->GetError() )
                    {
                        if( !refStrm.Is() )
                            refStrm = refSrcPics->OpenStream( aSrcStrmName,
                                    STREAM_READ | STREAM_SHARE_DENYWRITE );
                        if( SVSTREAM_OK == refStrm->GetError() )
                        {
                            refStrm->SetVersion( pDocStg->GetVersion() );
                            if( !(bGraphic ? aGrfObj.SwapIn( refStrm )
                                            : ImportGraphic( *refStrm ) ) )
                                return FALSE;
                        }
                    }
                    // If the graphic is restored within the same storage,
                    // its storage has to be removed.
                    if( pRoot == pDocStg )
                    {
                        refStrm.Clear();
                        DelStreamName();
                    }
                    aDstStrmName.Erase();
                }
                else
                {
                    SvStorageRef refDstPics =
                        pRoot->OpenStorage( aDstPicStgName,
                            STREAM_READWRITE | STREAM_SHARE_DENYALL );
                    if( refDstPics->IsContained( aDstStrmName ) )
                        // nur neu erzeugen, wenn Name schon vorhanden ist!
                        aDstStrmName = Sw3Io::UniqueName( refDstPics, "Pic" );

                    if( refSrcPics->CopyTo( aSrcStrmName, refDstPics,
                                            aDstStrmName )
                        && refDstPics->Commit() )
                        aNewStrmName = aDstStrmName;
                    else
                        return FALSE;
                }
            }
        }

        if( !aDstStrmName.Len() )
        {
            ASSERT( pRoot, "Kein Storage gegeben" );
            if( pRoot )
            {
                SvStorageRef refPics =
                    pRoot->OpenStorage( aDstPicStgName,
                        STREAM_READWRITE | STREAM_SHARE_DENYALL );
                if( SVSTREAM_OK == refPics->GetError() )
                {
                    aDstStrmName = Sw3Io::UniqueName( refPics, "Pic" );
                    SvStorageStreamRef refStrm =
                        refPics->OpenStream( aDstStrmName,
                        STREAM_READWRITE | STREAM_SHARE_DENYALL );
                    if( SVSTREAM_OK == refStrm->GetError() )
                    {
                        // HACK bis die Grafik als Portable markiert
                        // werden kann!!!
                        // Die Grafik kann in einer TempFile sein!
                        FASTBOOL bIsSwapOut = aGrfObj.IsSwappedOut();
                        if( bIsSwapOut && !aGrfObj.SwapIn() )
                            return FALSE;

                        refStrm->SetVersion( pRoot->GetVersion() );

                        //JP 04.05.98: laut ChangesMail vom KA und Bug 49617
                        //JP 21.06.98: laut ChangesMail vom KA, natives Save
                        USHORT nComprMode = refStrm->GetCompressMode();
                        if( SOFFICE_FILEFORMAT_40 <= refStrm->GetVersion() &&
                            GRAPHIC_BITMAP == aGrfObj.GetType() &&
                            GetDoc()->GetInfo()->IsSaveGraphicsCompressed() )
                            nComprMode |= COMPRESSMODE_ZBITMAP;
                        else
                            nComprMode &= ~COMPRESSMODE_ZBITMAP;

                        //JP 21.06.98: laut ChangesMail vom KA, natives Save
                        if( SOFFICE_FILEFORMAT_40 < refStrm->GetVersion() &&
                            GetDoc()->GetInfo()->IsSaveOriginalGraphics() )
                            nComprMode |= COMPRESSMODE_NATIVE;
                        else
                            nComprMode &= ~COMPRESSMODE_NATIVE;
                        refStrm->SetCompressMode( nComprMode );

                        BOOL bRes = FALSE;
                        if( pRoot == pDocStg )
                        {
                            if( aGrfObj.SwapOut( refStrm ) &&
                                ( refStrm->Commit() | refPics->Commit()
                                  /*| pRoot->Commit()*/ ))
                            {
                                SetStreamName( aDstStrmName );
                                bRes = TRUE;
                            }
                        }
                        else if( ((Graphic&)aGrfObj.GetGraphic()).
                                                WriteEmbedded( *refStrm )
                                && ( refStrm->Commit() | refPics->Commit()
                                  /*| pRoot->Commit()*/ ))
                        {
                            if( bIsSwapOut )
                                aGrfObj.SwapOut();
                            aNewStrmName = aDstStrmName;
                            bRes = TRUE;
                        }
                        return bRes;
                    }
                }
            }
            // Da fehlte doch was?
            return FALSE;
        }
    }
    // Schon drin im Storage oder Linked
    return TRUE;
}


BOOL SwGrfNode::GetFileFilterNms( String* pFileNm, String* pFilterNm ) const
{
    BOOL bRet = FALSE;
    if( refLink.Is() && refLink->GetLinkManager() )
    {
        USHORT nType = refLink->GetObjType();
        if( OBJECT_CLIENT_GRF == nType )
            bRet = refLink->GetLinkManager()->GetDisplayNames(
                    refLink, 0, pFileNm, 0, pFilterNm );
        else if( OBJECT_CLIENT_DDE == nType && pFileNm && pFilterNm )
        {
            String sApp, sTopic, sItem;
            if( refLink->GetLinkManager()->GetDisplayNames(
                    refLink, &sApp, &sTopic, &sItem ) )
            {
                ( *pFileNm = sApp ) += cTokenSeperator;
                ( *pFileNm += sTopic ) += cTokenSeperator;
                *pFileNm += sItem;
                pFilterNm->AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DDE" ));
                bRet = TRUE;
            }
        }
    }
    return bRet;
}


String SwGrfNode::GetStreamName() const
{
    if( aNewStrmName.Len() )
        return aNewStrmName;
    return aGrfObj.GetUserData();
}

// Eine Grafik Undo-faehig machen. Falls sie sich bereits in
// einem Storage befindet, muss sie geladen werden.

BOOL SwGrfNode::SavePersistentData()
{
    if( refLink.Is() )
    {
        ASSERT( !bInSwapIn, "SavePersistentData: stehe noch im SwapIn" );
        GetDoc()->GetLinkManager().Remove( refLink );
        return TRUE;
    }

    // Erst mal reinswappen, falls sie im Storage ist
    if( HasStreamName() && !SwapIn() )
        return FALSE;

    if( HasStreamName() )
        DelStreamName();

    // Und in TempFile rausswappen
    return (BOOL) SwapOut();
}


BOOL SwGrfNode::RestorePersistentData()
{
    if( refLink.Is() )
    {
        refLink->SetVisible( GetDoc()->IsVisibleLinks() );
        GetDoc()->GetLinkManager().InsertDDELink( refLink );
        if( GetDoc()->GetRootFrm() )
            refLink->Update();
    }
    return TRUE;
}


void SwGrfNode::InsertLink( const String& rGrfName, const String& rFltName )
{
    refLink = new SwBaseLink( LINKUPDATE_ONCALL, FORMAT_GDIMETAFILE, this );
    SwDoc* pDoc = GetDoc();
    if( GetNodes().IsDocNodes() )
    {
        refLink->SetVisible( pDoc->IsVisibleLinks() );
        if( rFltName.EqualsAscii( "DDE" ))
        {
            USHORT nTmp = 0;
            String sApp, sTopic, sItem;
            sApp = rGrfName.GetToken( 0, cTokenSeperator, nTmp );
            sTopic = rGrfName.GetToken( 0, cTokenSeperator, nTmp );
            sItem = rGrfName.Copy( nTmp );
            pDoc->GetLinkManager().InsertDDELink( refLink,
                                            sApp, sTopic, sItem );
        }
        else
        {
            BOOL bSync = rFltName.EqualsAscii( "SYNCHRON" );
            refLink->SetSynchron( bSync );
            refLink->SetContentType( Graphic::RegisterClipboardFormatName() );

            pDoc->GetLinkManager().InsertFileLink( *refLink,
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
        // erst die Grafik reinswappen!
//      if( aGraphic.IsSwapOut() || !refLink->IsSynchron() )
        {
            bInSwapIn = TRUE;
            SwBaseLink* pLink = (SwBaseLink*)(::so3::SvBaseLink*) refLink;
            pLink->SwapIn( TRUE, TRUE );
            bInSwapIn = FALSE;
        }
        GetDoc()->GetLinkManager().Remove( refLink );
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
        SetScaleImageMap( FALSE );
    }
}

        // Prioritaet beim Laden der Grafik setzen. Geht nur, wenn der Link
        // ein FileObject gesetzt hat
void SwGrfNode::SetTransferPriority( USHORT nPrio )
{
    if( refLink.Is() && refLink->GetObj() )
        SvxLinkManager::SetTransferPriority( *refLink, nPrio );
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

    BOOL bScale = FALSE;
    Fraction aScaleX( 1, 1 );
    Fraction aScaleY( 1, 1 );

    const SwFmtFrmSize& rFrmSize = pFmt->GetFrmSize();
    const SvxBoxItem& rBox = pFmt->GetBox();

    if( !rFrmSize.GetWidthPercent() )
    {
        SwTwips nWidth = rFrmSize.GetWidth();

        nWidth -= rBox.CalcLineSpace(BOX_LINE_LEFT) +
                  rBox.CalcLineSpace(BOX_LINE_RIGHT);

        ASSERT( nWidth>0, "Gibt es 0 twip breite Grafiken!?" );

        if( nGrfSize.Width() != nWidth )
        {
            aScaleX = Fraction( nGrfSize.Width(), nWidth );
            bScale = TRUE;
        }
    }
    if( !rFrmSize.GetHeightPercent() )
    {
        SwTwips nHeight = rFrmSize.GetHeight();

        nHeight -= rBox.CalcLineSpace(BOX_LINE_TOP) +
                   rBox.CalcLineSpace(BOX_LINE_BOTTOM);

        ASSERT( nHeight>0, "Gibt es 0 twip hohe Grafiken!?" );

        if( nGrfSize.Height() != nHeight )
        {
            aScaleY = Fraction( nGrfSize.Height(), nHeight );
            bScale = TRUE;
        }
    }

    if( bScale )
    {
        aURL.GetMap()->Scale( aScaleX, aScaleY );
        pFmt->SetAttr( aURL );
    }
}


void SwGrfNode::DelStreamName()
{
    if( HasStreamName() )
    {
        // Dann die Grafik im Storage loeschen
        SvStorage* pDocStg = GetDoc()->GetDocStorage();
        if( pDocStg )
        {
            String aPicStgName, aStrmName;
            GetStreamStorageNames( aStrmName, aPicStgName );
            SvStorageRef refPics = aPicStgName.Len()
                ? pDocStg->OpenStorage( aPicStgName,
                    STREAM_READWRITE | STREAM_SHARE_DENYALL )
                : pDocStg;
            if( refPics->GetError() == SVSTREAM_OK )
            {
                refPics->Remove( aStrmName );
                refPics->Commit();
                refPics->ResetError();  // Falls wir ReadOnly waren
            }
        }
        aGrfObj.SetUserData();
    }
}

BOOL SwGrfNode::GetStreamStorageNames( String& rStrmName,
                                      String& rStorName ) const
{
    BOOL bGraphic = FALSE;
    rStorName.Erase();
    rStrmName.Erase();

    String aUserData( aGrfObj.GetUserData() );
    if( !aUserData.Len() )
        return FALSE;

    String aProt( RTL_CONSTASCII_STRINGPARAM( "vnd.sun.star.Package:" ) );
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
            rStorName = aUserData.Copy( aProt.Len(), nPos-aProt.Len() );
            rStrmName = aUserData.Copy( nPos+1 );
        }
    }
    else
    {
        // 3.1 - 5.2
        rStorName = String( RTL_CONSTASCII_STRINGPARAM( "EmbeddedPictures" ) );
        rStrmName = aUserData;
        bGraphic = TRUE;
    }
    ASSERT( STRING_NOTFOUND == rStrmName.Search( '/' ),
            "invalid graphic stream name" );

    return bGraphic;
}

SwCntntNode* SwGrfNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
{
    // kopiere die Formate in das andere Dokument:
    SwGrfFmtColl* pColl = pDoc->CopyGrfColl( *GetGrfColl() );

    SwGrfNode* pThis = (SwGrfNode*)this;

    Graphic aTmpGrf;
    SwBaseLink* pLink = (SwBaseLink*)(::so3::SvBaseLink*) refLink;
    if( !pLink && HasStreamName() )
    {
        SvStorageRef refRoot = pThis->GetDoc()->GetDocStorage();
        ASSERT( refRoot.Is(), "Kein Storage am Doc" );
        if( refRoot.Is() )
        {
            String aStrmName, aPicStgName;
            BOOL bGraphic = GetStreamStorageNames( aStrmName, aPicStgName );
            SvStorageRef refPics = aPicStgName.Len()
                   ? refRoot->OpenStorage( aPicStgName,
                        STREAM_READ | STREAM_SHARE_DENYWRITE )
                : &refRoot;
            if( refPics->GetError() == SVSTREAM_OK )
            {
                SvStorageStreamRef refStrm = refPics->OpenStream( aStrmName,
                        STREAM_READ | STREAM_SHARE_DENYWRITE );
                if( refStrm->GetError() == SVSTREAM_OK )
                {
                    refStrm->SetVersion( refRoot->GetVersion() );
                    if( bGraphic )
                        aTmpGrf.SwapIn( refStrm );
                    else
                        GetGrfFilter()->ImportGraphic( aTmpGrf, String(),
                                                       *refStrm );
                }
            }
        }
    }
    else
    {
        if( aGrfObj.IsSwappedOut() )
            pThis->SwapIn();
        aTmpGrf = aGrfObj.GetGraphic();
    }

    const so3::SvLinkManager& rMgr = GetDoc()->GetLinkManager();
    String sFile, sFilter;
    if( IsLinkedFile() )
        rMgr.GetDisplayNames( refLink, 0, &sFile, 0, &sFilter );
    else if( IsLinkedDDE() )
    {
        String sTmp1, sTmp2;
        rMgr.GetDisplayNames( refLink, &sTmp1, &sTmp2, &sFilter );
        ::MakeLnkName( sFile, &sTmp1, sTmp2, sFilter );
        sFilter.AssignAscii( RTL_CONSTASCII_STRINGPARAM( "DDE" ));
    }

    SwGrfNode* pGrfNd = pDoc->GetNodes().MakeGrfNode( rIdx, sFile, sFilter,
                                                    &aTmpGrf, pColl,
                                            (SwAttrSet*)GetpSwAttrSet() );
    pGrfNd->SetAlternateText( GetAlternateText() );
    pGrfNd->SetContour( HasContour() );
    return pGrfNd;
}

IMPL_LINK( SwGrfNode, SwapGraphic, GraphicObject*, pGrfObj )
{
    SvStream* pRet;
    if( refLink.Is() )
    {
        if( pGrfObj->IsInSwapIn() )
        {
            // then make it by your self
            if( !bInSwapIn )
            {
                BOOL bIsModifyLocked = IsModifyLocked();
                LockModify();
                SwapIn( FALSE );
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
            SvStorageRef refRoot = GetDoc()->GetDocStorage();
            ASSERT( refRoot.Is(), "Kein Storage am Doc" );
            if( refRoot.Is() )
            {
                String aStrmName, aPicStgName;
                BOOL bGraphic = GetStreamStorageNames( aStrmName, aPicStgName );
                SvStorageRef refPics = aPicStgName.Len()
                    ? refRoot->OpenStorage( aPicStgName,
                        STREAM_READ | STREAM_SHARE_DENYWRITE )
                    : &refRoot;
                if( refPics->GetError() == SVSTREAM_OK )
                {
                    SvStream* pTmp = refPics->OpenStream( aStrmName,
                        STREAM_READ | STREAM_SHARE_DENYWRITE );
                    BOOL bDelStrm = TRUE;
                    if( pTmp->GetError() == SVSTREAM_OK )
                    {
                        if( pGrfObj->IsInSwapOut() )
                            pRet = GRFMGR_AUTOSWAPSTREAM_LINK;
                        else
                        {
                            if( bGraphic )
                            {
                                pRet = pTmp;
                                bDelStrm = FALSE;
                                pRet->SetVersion( refRoot->GetVersion() );
                            }
                            else
                            {
                                ImportGraphic( *pTmp );
                                pRet = GRFMGR_AUTOSWAPSTREAM_LOADED;
                            }
                        }
                    }
                    if( bDelStrm )
                        delete pTmp;
                }
            }
        }
    }

    return (long)pRet;

/*
    SvStream* pStream = GetSwapStream();

    if( GRFMGR_AUTOSWAPSTREAM_NONE != pStream )
    {
        if( GRFMGR_AUTOSWAPSTREAM_LINK == pStream )
            mbAutoSwapped = maGraphic.SwapOut( NULL );
        else
        {
            if( GRFMGR_AUTOSWAPSTREAM_TEMP == pStream )
                mbAutoSwapped = maGraphic.SwapOut();
            else
                mbAutoSwapped = maGraphic.SwapOut( pStream );
        }
    }
*/
}


// alle QuickDraw-Bitmaps eines speziellen Docs loeschen
void DelAllGrfCacheEntries( SwDoc* pDoc )
{
    if( pDoc )
    {
        // alle Graphic-Links mit dem Namen aus dem Cache loeschen
        const SvxLinkManager& rLnkMgr = pDoc->GetLinkManager();
        const ::so3::SvBaseLinks& rLnks = rLnkMgr.GetLinks();
        SwGrfNode* pGrfNd;
        String sFileNm;
        for( USHORT n = rLnks.Count(); n; )
        {
            ::so3::SvBaseLink* pLnk = &(*rLnks[ --n ]);
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
    ULONG nMirror = BMP_MIRROR_NONE;
    if( rMirror.IsGrfToggle() && pFrm && !pFrm->FindPageFrm()->OnRightPage() )
    {
        switch( rMirror.GetValue() )
        {
        case RES_DONT_MIRROR_GRF:   nMirror = BMP_MIRROR_HORZ; break;
        case RES_MIRROR_GRF_VERT:   nMirror = BMP_MIRROR_NONE; break;
        case RES_MIRROR_GRF_HOR:    nMirror = BMP_MIRROR_HORZ|BMP_MIRROR_VERT;
                                    break;
        default:                    nMirror = BMP_MIRROR_VERT; break;
        }
    }
    else
        switch( rMirror.GetValue() )
        {
        case RES_MIRROR_GRF_BOTH:   nMirror = BMP_MIRROR_HORZ|BMP_MIRROR_VERT;
                                    break;
        case RES_MIRROR_GRF_VERT:   nMirror = BMP_MIRROR_HORZ; break;
        case RES_MIRROR_GRF_HOR:    nMirror = BMP_MIRROR_VERT; break;
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
    rGA.SetTransparency( (BYTE) FRound(
                                Min( nTrans, (USHORT) 100 )  * 2.55 ) );

    return rGA;
}

BOOL SwGrfNode::IsTransparent() const
{
    BOOL bRet = aGrfObj.IsTransparent();
    if( !bRet ) // ask the attribut
        bRet = 0 != GetSwAttrSet().GetTransparencyGrf().GetValue();
    return bRet;
}


