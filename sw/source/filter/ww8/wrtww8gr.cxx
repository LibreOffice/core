/*************************************************************************
 *
 *  $RCSfile: wrtww8gr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:58 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen wg. SvStorage
#include <so3/svstor.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _MSOLEEXP_HXX
#include <svx/msoleexp.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>       // class SwFlyFrmFmt
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>       // class SwCropGrf
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _WW8STRUC_HXX
#include <ww8struc.hxx>
#endif
#ifndef _WRTWW8_HXX
#include <wrtww8.hxx>
#endif


#define WW8_ASCII2STR(s) String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM(s))

// Damit KA debuggen kann, ohne sich den ganzen Writer zu holen, ist
// temporaer dieses Debug gesetzt. Ist ausserdem noch das passende IniFlag
// gesetzt, dann werden in d:\ Hilfsdateien erzeugt.
// !! sollte demnaechst wieder entfernt werden !!
// #define DEBUG_KA


// ToDo:
// 5. Die MapModes, die Win nicht kann, umrechnen

// OutGrf() wird fuer jeden GrafNode im Doc gerufen. Es wird ein PicLocFc-Sprm
// eingefuegt, der statt Adresse ein Magic ULONG enthaelt. Ausserdem wird
// in der Graf-Klasse der GrfNode-Ptr gemerkt ( fuers spaetere Ausgeben der
// Grafiken und Patchen der PicLocFc-Attribute )

Writer& OutWW8_SwGrfNode( Writer& rWrt, SwCntntNode& rNode )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    rWW8Wrt.OutGrf( rNode.GetGrfNode() );

    rWW8Wrt.pFib->fHasPic = 1;

    return rWrt;
}

Writer& OutWW8_SwOleNode( Writer& rWrt, SwCntntNode& rNode )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    if( !(rWW8Wrt.GetIniFlags() & WWFL_NO_OLE ) )
    {
        if( rWW8Wrt.bWrtWW8 )
        {
            static BYTE aSpecOLE[] = {
                0x03, 0x6a, 0, 0, 0, 0,         // sprmCPicLocation
                0x0a, 0x08, 1,                  // sprmCFOLE2
                0x56, 0x08, 1                   // sprmCFObj
            };

            const SwOLENode& rOLENd = (SwOLENode&)rNode;
            UINT32 nPictureId = (long)&rOLENd;
            BYTE* pDataAdr = aSpecOLE + 2;
            Set_UInt32( pDataAdr, nPictureId );

            SvStorageRef xObjStg = rWW8Wrt.GetStorage().OpenStorage(
                            String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "ObjectPool" )),
                                    STREAM_READWRITE| STREAM_SHARE_DENYALL );
            if( xObjStg.Is()  )
            {
                String sStorageName( '_' );
                sStorageName += String::CreateFromInt32( nPictureId );
                SvStorageRef xOleStg = xObjStg->OpenStorage( sStorageName,
                                    STREAM_READWRITE| STREAM_SHARE_DENYALL );
                if( xOleStg.Is() )
                {
                    SvInPlaceObjectRef xObj( ((SwOLENode&)rOLENd).
                                                GetOLEObj().GetOleRef() );
                    if( xObj.Is() )
                    {
                        rWW8Wrt.GetOLEExp().ExportOLEObject( *xObj, *xOleStg );

                        // write as embedded field - the other things will
                        // be done in the escher export
                        String sServer( String::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( " EINBETTEN " )));
                        ( sServer += xOleStg->GetUserName() ) += ' ';

                        rWW8Wrt.OutField( 0, 58, sServer,
                                                WRITEFIELD_START |
                                                WRITEFIELD_CMD_START |
                                                WRITEFIELD_CMD_END );

                        rWW8Wrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(),
                                        sizeof( aSpecOLE ), aSpecOLE );

                        rWW8Wrt.WriteChar( 0x1 );

                        rWW8Wrt.OutField( 0, 58, aEmptyStr,
                                                WRITEFIELD_END |
                                                WRITEFIELD_CLOSE );
                        rWW8Wrt.WriteCR();              // CR danach
                    }
                }
            }
        }
        else
            rWW8Wrt.OutGrf( rNode.GetOLENode() );
    }
    return rWrt;
}


void SwWW8Writer::OutGrf( const SwNoTxtNode* pNd )
{
    if( nIniFlags & WWFL_NO_GRAF )
        return;     // Iniflags: kein Grafik-Export

    if( !pFlyFmt )              // Grafik mit eigenem Frame ( eigentlich immer )
    {
        ASSERT( !this, "+Grafik ohne umgebenden Fly" );
        return ;
    }

    // GrfNode fuer spaeteres rausschreiben der Grafik merken
    pGrf->Insert( pNd, pFlyFmt );

    pChpPlc->AppendFkpEntry( pStrm->Tell(), pO->Count(), pO->GetData() );
    pO->Remove( 0, pO->Count() );                   // leeren

    WriteChar( (char)1 );   // Grafik-Sonderzeichen in Haupttext einfuegen

    BYTE aArr[ 14 ];
    BYTE* pArr = aArr;

    // sprmCFSpec
    if( bWrtWW8 )
        Set_UInt16( pArr, 0x855 );
    else
        Set_UInt8( pArr, 117 );
    Set_UInt8( pArr, 1 );

    // sprmCPicLocation
    if( bWrtWW8 )
        Set_UInt16( pArr, 0x6a03 );
    else
    {
        Set_UInt8( pArr, 68 );
        Set_UInt8( pArr, 4 );
    }
    Set_UInt32( pArr, GRF_MAGIC_321 );

    // Magic variieren, damit verschiedene Grafik-Attribute nicht
    // gemerged werden
    static BYTE nAttrMagicIdx = 0;
    --pArr;
    Set_UInt8( pArr, nAttrMagicIdx++ );
    pChpPlc->AppendFkpEntry( pStrm->Tell(), pArr - aArr, aArr );

    RndStdIds eAn = pFlyFmt->GetAttrSet().GetAnchor( FALSE ).GetAnchorId();
    if( eAn == FLY_AT_CNTNT || eAn == FLY_PAGE )
    {
        WriteChar( (char)0x0d ); // umgebenden Rahmen mit CR abschliessen

        static BYTE __READONLY_DATA nSty[2] = { 0, 0 };
        pO->Insert( nSty, 2, pO->Count() );     // Style #0
        BOOL bOldGrf = bOutGrf;
        bOutGrf = TRUE;

        Out_SwFmt( *pFlyFmt, FALSE, FALSE, TRUE );          // Fly-Attrs

        bOutGrf = bOldGrf;
        pPapPlc->AppendFkpEntry( pStrm->Tell(), pO->Count(), pO->GetData() );
        pO->Remove( 0, pO->Count() );                   // leeren
    }
}

void SwWW8WrGrf::Insert( const SwNoTxtNode* pNd, const SwFlyFrmFmt* pFly )
{
    aNds.Insert( (VoidPtr)pNd, aNds.Count() );
    aFlys.Insert( (VoidPtr)pFly, aFlys.Count() );
    if( rWrt.nFlyWidth > 0 && rWrt.nFlyHeight > 0 )
    {
        aWid.Insert( (UINT16)rWrt.nFlyWidth, aWid.Count() ); // hier steht die ware Groesse
        aHei.Insert( (UINT16)rWrt.nFlyHeight, aHei.Count() ); // naemlich im uebergeordneten Frame
    }
    else
    {
        Size aGrTwipSz( pNd->GetTwipSize() );
        //JP 05.12.98: falls die Grafik noch nie angezeigt wurde und es sich
        //              um eine gelinkte handelt, so ist keine Size gesetzt. In
        //              diesem Fall sollte man sie mal reinswappen.
        if( ( !aGrTwipSz.Width() || !aGrTwipSz.Height() ) &&
            pNd->IsGrfNode() &&
            GRAPHIC_DEFAULT == ((SwGrfNode*)pNd)->GetGrf().GetType() )
        {
            ((SwGrfNode*)pNd)->SwapIn();
            aGrTwipSz = pNd->GetTwipSize();
        }

        aWid.Insert( (UINT16)aGrTwipSz.Width(), aWid.Count() );
        aHei.Insert( (UINT16)aGrTwipSz.Height(), aHei.Count() );
    }
}

void SwWW8WrGrf::Write1GrfHdr( SvStream& rStrm, const SwNoTxtNode* pNd,
                                const SwFlyFrmFmt* pFly,
                            UINT16 mm, UINT16 nWidth, UINT16 nHeight )
{
    INT16 nXSizeAdd = 0, nYSizeAdd = 0;
    INT16 nCropL = 0, nCropR = 0, nCropT = 0, nCropB = 0;

            // Crop-AttributInhalt in Header schreiben ( falls vorhanden )
    const SwAttrSet* pAttrSet = pNd->GetpSwAttrSet();
    const SfxPoolItem* pItem;
    if( pAttrSet && ( SFX_ITEM_ON
        == pAttrSet->GetItemState( RES_GRFATR_CROPGRF, FALSE, &pItem ) ) )
    {
        const SwCropGrf& rCr = *(SwCropGrf*)pItem;
        nCropL = rCr.GetLeft();
        nCropR = rCr.GetRight();
        nCropT = rCr.GetTop();
        nCropB = rCr.GetBottom();
        nXSizeAdd -= (INT16)( rCr.GetLeft() - rCr.GetRight() );
        nYSizeAdd -= (INT16)( rCr.GetTop() - rCr.GetBottom() );
    }

    Size aGrTwipSz( pNd->GetTwipSize() );
    //JP 05.12.98: falls die Grafik noch nie angezeigt wurde und es sich
    //              um eine gelinkte handelt, so ist keine Size gesetzt. In
    //              diesem Fall sollte man sie mal reinswappen.
    if( ( !aGrTwipSz.Width() || !aGrTwipSz.Height() ) &&
        pNd->IsGrfNode() &&
        GRAPHIC_DEFAULT == ((SwGrfNode*)pNd)->GetGrf().GetType() )
    {
        ((SwGrfNode*)pNd)->SwapIn();
        aGrTwipSz = pNd->GetTwipSize();
    }


    BOOL bWrtWW8 = rWrt.bWrtWW8;
    UINT16 nHdrLen = bWrtWW8 ? 0x3A : 0x44;

    BYTE aArr[ sizeof( WW8_PIC_SHADOW ) ];
    memset( aArr, 0, nHdrLen );

    BYTE* pArr = aArr + 4;                          //skip lcb
    Set_UInt16( pArr, nHdrLen );                    // set cbHeader

    Set_UInt16( pArr, mm );                         // set mm
    Set_UInt16( pArr, aGrTwipSz.Width() * 254L / 144 );     // set xExt
    Set_UInt16( pArr, aGrTwipSz.Height() * 254L / 144 );    // set yExt
    pArr += 16;                                     // skip hMF & rcWinMF
    Set_UInt16( pArr, (UINT16)aGrTwipSz.Width() );  // set dxaGoal
    Set_UInt16( pArr, (UINT16)aGrTwipSz.Height() ); // set dyaGoal

    if( aGrTwipSz.Width() + nXSizeAdd )             // set mx
        Set_UInt16( pArr, ( nWidth * 1000L /
                            ( aGrTwipSz.Width() + nXSizeAdd ) ) );
    else
        pArr += 2;
    if( aGrTwipSz.Height() + nYSizeAdd )            // set my
        Set_UInt16( pArr, ( nHeight * 1000L /
                            ( aGrTwipSz.Height() + nYSizeAdd ) ) );
    else
        pArr += 2;

    Set_UInt16( pArr, nCropL );                     // set dxaCropLeft
    Set_UInt16( pArr, nCropT );                     // set dyaCropTop
    Set_UInt16( pArr, nCropR );                     // set dxaCropRight
    Set_UInt16( pArr, nCropB );                     // set dyaCropBottom
    pArr += 2;                                      // skip Flags

    if( pFly )
    {
        const SwAttrSet& rAttrSet = pFly->GetAttrSet();
        if( SFX_ITEM_ON == rAttrSet.GetItemState( RES_BOX, FALSE, &pItem ) )
        {
            const SvxBoxItem* pBox = (const SvxBoxItem*)pItem;
            if( pBox )
            {
                BOOL bShadow = FALSE;               // Shadow ?
                const SfxPoolItem* pItem;
                if( SFX_ITEM_ON
                    == rAttrSet.GetItemState( RES_SHADOW, TRUE, &pItem ) )
                {
                    const SvxShadowItem* pSI = (const SvxShadowItem*)pItem;
                    bShadow = ( pSI->GetLocation() != SVX_SHADOW_NONE )
                            && ( pSI->GetWidth() != 0 );
                }

                BYTE aLnArr[4] = { BOX_LINE_TOP, BOX_LINE_LEFT,
                                    BOX_LINE_BOTTOM, BOX_LINE_RIGHT };
                for( BYTE i = 0; i < 4; ++i )
                {
                    const SvxBorderLine* pLn = pBox->GetLine( aLnArr[ i ] );
                    UINT32 nBrd = pLn
                                    ? rWrt.TranslateBorderLine( *pLn,
                                        pBox->GetDistance( aLnArr[ i ] ),
                                        bShadow )
                                    : 0;

                    if( bWrtWW8 )
                        Set_UInt32( pArr, nBrd );
                    else
                        Set_UInt16( pArr, (USHORT)nBrd );
                }
            }
        }
    }
    rStrm.Write( aArr, nHdrLen );
}

void SwWW8WrGrf::Write1Grf1( SvStream& rStrm, const SwGrfNode* pGrfNd,
                    const SwFlyFrmFmt* pFly, UINT16 nWidth, UINT16 nHeight )
{
    if( pGrfNd->IsLinkedFile() )        // Linked File
    {
        String aFileN, aFiltN;
        UINT16 mm;
        pGrfNd->GetFileFilterNms( &aFileN, &aFiltN );

        aFileN = INetURLObject::AbsToRel( aFileN, INetURLObject::WAS_ENCODED,
                                        INetURLObject::DECODE_WITH_CHARSET );
        INetURLObject aUrl( aFileN );
        if( aUrl.GetProtocol() == INET_PROT_FILE )
            aFileN = aUrl.PathToFileName();

//JP 05.12.98: nach einigen tests hat sich gezeigt, das WW mit 99 nicht
//              klarkommt. Sie selbst schreiben aber bei Verknuepfunfen,
//              egal um welchen Type es sich handelt, immer den Wert 94.
//              Bug 59859
//      if ( COMPARE_EQUAL == aFiltN.ICompare( "TIF", 3 ) )
//          mm = 99;                    // 99 = TIFF
//      else
            mm = 94;                    // 94 = BMP, GIF

        Write1GrfHdr( rStrm, pGrfNd, pFly, mm, nWidth, nHeight );   // Header
        rStrm << (BYTE)aFileN.Len();    // Pascal-String schreiben
        SwWW8Writer::WriteString8( rStrm, aFileN, FALSE, RTL_TEXTENCODING_MS_1252 );
    }
    else                                // Embedded File oder DDE oder so was
    {
        Graphic& rGrf = (Graphic&)(pGrfNd->GetGrf());
        BOOL bSwapped = rGrf.IsSwapOut();
        ((SwGrfNode*)pGrfNd)->SwapIn(); // immer ueber den Node einswappen !!!

        GDIMetaFile aMeta;

        switch ( rGrf.GetType() )
        {
        case GRAPHIC_BITMAP:            // Bitmap -> in Metafile abspielen
            {
                VirtualDevice aVirt;
                aMeta.Record( &aVirt );
                aVirt.DrawBitmap( Point( 0,0 ), rGrf.GetBitmap() );
                aMeta.Stop();
                aMeta.WindStart();
                aMeta.SetPrefMapMode( rGrf.GetPrefMapMode());
                aMeta.SetPrefSize( rGrf.GetPrefSize());
            }
            break;
        case GRAPHIC_GDIMETAFILE :      // GDI ( =SV ) Metafile
#ifndef VCL
        case GRAPHIC_WINMETAFILE :      // WinMetafile
#endif
                aMeta = rGrf.GetGDIMetaFile();
                break;
        default : return;
        }

//      ASSERT( aMeta.GetPrefMapMode().GetMapUnit() == MAP_100TH_MM,
//              "MapMode der Grafik ist nicht 1/100mm!" );

#ifdef DEBUG
        BOOL bSchreibsRaus  = FALSE;
        BOOL bSchreibsRein  = FALSE;
        long nSchreibsRausA = rStrm.Tell();
        long nSchreibsReinA = nSchreibsRausA;
#endif

        Write1GrfHdr( rStrm, pGrfNd, pFly, 8, nWidth, nHeight );    // Header
        WriteWindowMetafileBits( rStrm, aMeta );        // eigentliche Grafik

#ifdef DEBUG
        if(  bSchreibsRaus )
        {
            long nSchreibsRausZ = rStrm.Tell();
            SvFileStream aS( WW8_ASCII2STR( "e:\\ww-exp.wmf" ), STREAM_WRITE );
            rStrm.Seek( nSchreibsRausA );
            UINT16 nRead1;
            BYTE* pBuf = new BYTE[ 8192 ];
            ULONG nRead = nSchreibsRausZ - nSchreibsRausA;
            do{
                nRead1 = ( nRead > 8192 ) ? 8192 : (UINT16)nRead;
                rStrm.Read( pBuf, nRead1 );
                aS.Write(   pBuf, nRead1 );
                nRead -= nRead1;
            }while( nRead );
            aS.Close();
            rStrm.Seek( nSchreibsRausZ );
        }
        if(  bSchreibsRein )
        {
            SvFileStream aS( WW8_ASCII2STR( "e:\\ww-imp.wmf" ), STREAM_READ );
            aS.Seek( STREAM_SEEK_TO_END );
            ULONG nRead = aS.Tell();
            aS.Seek( 0 );
            rStrm.Seek( nSchreibsReinA );
            UINT16 nRead1;
            BYTE* pBuf = new BYTE[ 8192 ];
            do{
                nRead1 = ( nRead > 8192 ) ? 8192 : (UINT16)nRead;
                aS.Read(     pBuf, nRead1 );
                rStrm.Write( pBuf, nRead1 );
                nRead -= nRead1;
            }while( nRead );
            aS.Close();
            // kein Seek, da es hier im Stream weitergehen soll...
        }
#endif

        if( bSwapped ) rGrf.SwapOut();
    }
}

void SwWW8WrGrf::Write1Grf( SvStream& rStrm, const SwNoTxtNode* pNd,
                    const SwFlyFrmFmt* pFly, UINT16 nWidth, UINT16 nHeight )
{
    if( !pNd || ( !pNd->IsGrfNode() && !pNd->IsOLENode() ) )
        return;

    UINT32 nPos = rStrm.Tell();         // Grafik-Anfang merken

    if( pNd->IsGrfNode() )
    {
        const SwGrfNode* pGrfNd = pNd->GetGrfNode();
        Write1Grf1( rStrm, pGrfNd, pFly, nWidth, nHeight );
    }
    else if( pNd->IsOLENode() )
    {
        SwOLENode*                  pOleNd = ((SwNoTxtNode*)pNd)->GetOLENode(); // const wegcasten
        ASSERT( pOleNd, " Wer hat den OleNode versteckt ?" );
        SwOLEObj&                   rSObj= pOleNd->GetOLEObj();
        const SvInPlaceObjectRef    rObj(  rSObj.GetOleRef() );
        SvData                      aData( FORMAT_GDIMETAFILE );

        // das MetaFile wird sich jetzt ueber die SvData-Schnittstelle
        // besorgt, d.h. SvData-Object anlegen und via GetData
        // das MetaFile abholen. MakeMetaFile entfaellt,
        // der Rest ist gleich geblieben (KA 30.09.96)
        if ( rObj->GetData( &aData ) )
        {
            GDIMetaFile* pMtf = NULL;

            if ( aData.GetData( &pMtf, TRANSFER_MOVE ) )
            {
                DBG_ASSERT( pMtf, "Wo ist denn nun das MetaFile?" );

                Size aS ( pMtf->GetPrefSize() );
#ifdef DEBUG
                MapMode aMap ( pMtf->GetPrefMapMode() );
                ASSERT( pMtf->GetActionCount(), "OLE schreiben OK ? ( No Meta-Action )" );
                ASSERT( pMtf->GetPrefMapMode().GetMapUnit() == MAP_100TH_MM,
                        "MapMode des Ole ist nicht 1/100mm!" );
#endif
                pMtf->WindStart();
                pMtf->Play( Application::GetDefaultDevice(),
                            Point( 0, 0 ), Size( 2880, 2880 ) );
                Write1GrfHdr( rStrm, pNd, pFly, 8, nWidth, nHeight );   // Header
                WriteWindowMetafileBits( rStrm, *pMtf );                // eigentliche Grafik

#ifdef DEBUG_KA

                if( rWrt.GetIniFlags() & WWFL_KA_DEBUG )
                {
                    SvFileStream aS( WW8_ASCII2STR( "d:\\xxx.svm" ), STREAM_WRITE | STREAM_TRUNC );

                    aS << *pMtf;
                    aS.Close();

                    aS.Open( WW8_ASCII2STR( "d:\\xxx.wmf" ), STREAM_WRITE | STREAM_TRUNC  );
                    WriteWindowMetafile( aS, *pMtf );
                }

#endif // DEBUG_KA

                // brauchen wir nicht mehr
                delete pMtf;
            }
        }
    }

    UINT32 nPos2 = rStrm.Tell();                    // Ende merken
    rStrm.Seek( nPos );
    SVBT32 nLen;
    LongToSVBT32( nPos2 - nPos, nLen );             // Grafik-Laenge ausrechnen
    rStrm.Write( nLen, 4 );                         // im Header einpatchen
    rStrm.Seek( nPos2 );                            // Pos wiederherstellen
}

// SwWW8WrGrf::Write() wird nach dem Text gerufen. Es schreibt die alle
// Grafiken raus und merkt sich die File-Positionen der Grafiken, damit
// beim Schreiben der Attribute die Positionen in die PicLocFc-Sprms
// eingepatcht werden koennen.
// Das Suchen in den Attributen nach dem Magic ULONG und das Patchen
// passiert beim Schreiben der Attribute. Die SwWW8WrGrf-Klasse liefert
// hierfuer nur mit GetFPos() sequentiell die Positionen.
void SwWW8WrGrf::Write()
{
    nIdx = 0;       // fuers anschliessende Abfragen
    SvStream& rStrm = *rWrt.pDataStrm;
    for( USHORT i = 0; i < aNds.Count(); i++ )
    {
        UINT32 nPos = rStrm.Tell();                 // auf 4 Bytes alignen
        if( nPos & 0x3 )
            SwWW8Writer::FillCount( rStrm, 4 - ( nPos & 0x3 ) );

        aPos.Insert( rStrm.Tell(), aPos.Count() );  // Pos merken
        const SwNoTxtNode* pNd = (const SwNoTxtNode*)aNds[i];
        Write1Grf( rStrm, pNd, (const SwFlyFrmFmt*)aFlys[i],
                                aWid[i], aHei[i] );
    }
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/wrtww8gr.cxx,v 1.1.1.1 2000-09-18 17:14:58 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.16  2000/09/18 16:04:58  willem.vandorp
      OpenOffice header added.

      Revision 1.15  2000/08/25 12:27:35  jp
      Graphic Crop-Attribut exported to SVX

      Revision 1.14  2000/08/18 13:02:29  jp
      don't export escaped URLs

      Revision 1.13  2000/07/05 16:07:38  jp
      new MS Filteroptions - change StarOffice Objects to OLE-Objects

      Revision 1.12  2000/06/26 09:58:38  jp
      must change: GetAppWindow->GetDefaultDevice

      Revision 1.11  2000/05/25 09:19:59  khz
      Changes for Unicode (for debug mode)

      Revision 1.10  2000/05/12 16:14:14  jp
      Changes for Unicode

      Revision 1.9  2000/02/11 14:39:59  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.8  1999/12/02 10:41:05  jp
      Bug #70093#: OutWW8_SwOleNode - write field with storage-username, W97 need it

      Revision 1.7  1999/10/29 17:40:22  jp
      export of 'complex'(vert. merged) tables

      Revision 1.6  1999/08/24 18:15:52  JP
      Escher Export


      Rev 1.5   24 Aug 1999 20:15:52   JP
   Escher Export

      Rev 1.4   06 Jul 1999 14:54:06   JP
   W97-Export corrections

      Rev 1.3   30 Jun 1999 19:56:46   JP
   W97-Export: Fields/PostIts/References

      Rev 1.2   04 Jun 1999 17:47:56   JP
   WW97-Export: Code aufgeteilt

      Rev 1.1   27 May 1999 23:42:28   JP
   Schnittstellen angepasst

      Rev 1.0   19 May 1999 11:20:20   JP
   WinWord97-ExportFilter

*************************************************************************/


