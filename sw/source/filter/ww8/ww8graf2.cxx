/*************************************************************************
 *
 *  $RCSfile: ww8graf2.cxx,v $
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
#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif


#ifndef _TOOLS_TEMPFILE_HXX
#include <tools/tempfile.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX //autogen
#include <svx/impgrf.hxx>
#endif
#ifndef _SVDPAGE_HXX //autogen
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDOGRAF_HXX
#include <svx/svdograf.hxx>
#endif
#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif

#ifndef _MSDFFIMP_HXX
#include <svx/msdffimp.hxx>
#endif


#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif

#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif

#ifndef _SFX_FCONTNR_HXX
#include <sfx2/fcontnr.hxx>
#endif


#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif

#ifndef _GRFATR_HXX
#include <grfatr.hxx>           // class SwCropGrf
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FLTSHELL_HXX
#include <fltshell.hxx>
#endif
#ifndef _WW8STRUC_HXX
#include <ww8struc.hxx>
#endif
#ifndef _WW8SCAN_HXX
#include <ww8scan.hxx>
#endif
#ifndef _WW8PAR_HXX
#include <ww8par.hxx>           // class SwWWImplReader
#endif
#ifndef _WW8PAR2_HXX
#include <ww8par2.hxx>          // struct WWFlyPara
#endif
#ifndef _WW8GRAF_HXX
#include <ww8graf.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>            // class SwTxtNode
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // Progress
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif


#ifdef __WW8_NEEDS_COPY
extern void WW8PicShadowToReal(  WW8_PIC_SHADOW*  pPicS,  WW8_PIC*  pPic );
#endif // defined __WW8_NEEDS_COPY


//-----------------------------------------
//              los gehts
//-----------------------------------------


#ifdef USE_WMF_GRAF_FILE    // Definieren fuer Import WMF ueber File
                            // ( alte Version, kann kein MAC-PICT )
struct WmfFileHd
{                     // Vorsicht: Struktur ist nicht aligned
    SVBT16 nTyp;            // 0 = Mem, 1 = File
    SVBT16 nHdSiz;          // 0x009
    SVBT16 nVersion;        // 0x300
    SVBT32 nSize;           // filesize in bytes
    SVBT16 nNoObj;          // Number of Objects that exist at same time
    SVBT32 nMaxRecLen;      // largest record
    SVBT16 nNoParas;        // not used
};
#define WMF_FILE_HD_SIZ 18    // Groesse der Struktur WmfFileHd

ULONG SwWW8ImplReader::ReadWmfHeader( WmfFileHd* pHd, long nPos )
{
    pStrm->Seek( nPos );
    pStrm->Read( pHd, sizeof( *pHd ) );
    return SVBT32ToLong( pHd->nSize ) * 2;       // FileSize in Bytes
}

#endif // USE_WMF_GRAF_FILE




struct METAFILEHEADER       // ist aligned, deshalb gehts fuer alle Platformen
{
    UINT32  key;
    UINT16  hmf;
    UINT16  left;
    UINT16  top;
    UINT16  right;
    UINT16  bottom;
    UINT16  inch;
    UINT32  reserved;
    UINT16  checksum;
};


#define METAFILEHEADER_SIZE 22


static void WriteWmfPreHd( long nWidth, long nHeight, SvStream& rOStream )
{
    METAFILEHEADER aHeader;

    aHeader.key = 0x9AC6CDD7L;
    aHeader.hmf = 0;
    aHeader.left = 0;
    aHeader.top = 0;
    aHeader.right =  (UINT16)( nWidth * 100 / 144 );
    aHeader.bottom =  (UINT16)( nHeight * 100 / 144 );
    aHeader.inch= 1000;
    aHeader.reserved = 0;
    aHeader.checksum = 0;
    for( USHORT n=0; n < 10; n++ )
        aHeader.checksum ^= *(((UINT16*)&aHeader)+n);

#ifdef __BIGENDIAN
    aHeader.key = SWAPLONG( aHeader.key );
    aHeader.left = 0;
    aHeader.top = 0;
    aHeader.right = SWAPSHORT( aHeader.right );
    aHeader.bottom = SWAPSHORT( aHeader.bottom );
    aHeader.inch = SWAPSHORT( aHeader.inch );
    aHeader.checksum = SWAPSHORT( aHeader.checksum );
#endif // __BIGENDIAN

    rOStream.Write( (char*)&aHeader, METAFILEHEADER_SIZE );
}


#define WWBUFSIZ 4096           //  512 <= WWBUFSIZE <= UINT16_MAX !!!

static void _CopyFilePiece( SvStream* pDst, SvStream* pSrc, ULONG nRead,
                              void* pBuf, UINT16 nBufSize )
{
    UINT16 nRead1;
    do{
        nRead1 = ( nRead > nBufSize ) ? nBufSize : (UINT16)nRead;
        pSrc->Read( pBuf, nRead1 );
        pDst->Write( pBuf, nRead1 );
        nRead -= nRead1;
    }while( nRead );
}

BOOL SwWW8ImplReader::ReadGrafFile( String& rFileName, Graphic*& rpGraphic,
    const WW8_PIC& rPic,    SvStream* pSt, ULONG nFilePos, BOOL* pbInDoc )
{                                                  // Grafik in File schreiben
    ASSERT( WWBUFSIZ >= 512,
             "Au weia, Puffergroesse < 512, keine Mac-Picts moeglich" );

    *pbInDoc = TRUE;                               // default

    ULONG nPosFc = nFilePos + rPic.cbHeader;

    switch( rPic.MFP.mm ){
    case 94:                        // BMP-File ( nicht embeddet ) oder GIF
    case 99:
        {   // TIFF-File ( nicht embeddet )
            pSt->Seek( nPosFc );
            String aStr( WW8ReadPString( *pSt, eStructCharSet, 0 ) );// Name als P-String einlesen
            rFileName = aStr.GetChar( 0 );      // Wandeln in SV-String
            *pbInDoc = FALSE;       // Datei anschliessend nicht loeschen
            return rFileName.Len() != 0;        // Einlesen OK
        }
    }
#ifndef USE_WMF_GRAF_FILE
                            // Neu:WMF - Variante ohne File
    {
        GDIMetaFile aWMF;
        pSt->Seek( nPosFc );
        BOOL bOk = ReadWindowMetafile( *pSt, aWMF );
                    // *pSt >> aWMF; geht nicht ohne placable header
        if( !bOk || pSt->GetError() || !aWMF.GetActionCount() ){
//          ASSERT( bOk, "ReadWindowMetafile ging schief" );
//          ASSERT( !pSt->GetError(), "WMF einlesen ging schief ( Stream-Error )" );
//          ASSERT( aWMF.GetActionCount(), "WMF einlesen ging schief ( No Meta-Action )" );
            return FALSE;
        }
        if( pWwFib->envr != 1 ){            // !MAC als Creator

            aWMF.SetPrefMapMode( MapMode( MAP_100TH_MM ) );
            // MetaFile auf neue Groesse skalieren und
            // neue Groesse am MetaFile setzen
            Size        aOldSiz( aWMF.GetPrefSize() );
            Size        aNewSiz( rPic.MFP.xExt, rPic.MFP.yExt );
            Fraction    aFracX( aNewSiz.Width(), aOldSiz.Width() );
            Fraction    aFracY( aNewSiz.Height(), aOldSiz.Height() );

            aWMF.Scale( aFracX, aFracY );
            aWMF.SetPrefSize( aNewSiz );

            rpGraphic = new Graphic( aWMF );
            return TRUE;
        }
        // MAC - Word als Creator
        // im WMF steht nur "Benutzen sie Word 6.0c"
        // Mac-Pict steht dahinter
        // allerdings ohne die ersten 512 Bytes,
        // bei einem MAC-PICT egal sind ( werden nicht ausgewertet )

        long nCopy = rPic.lcb - ( pSt->Tell() - nPosFc );
        if( nCopy <= 0 || WWBUFSIZ < 512 )
            return FALSE;

        BYTE* pBuf = new BYTE[WWBUFSIZ];
        String sExt(WW8_ASCII2STR("pct"));
        TempFile aTempFile(aEmptyStr, &sExt);
        aTempFile.EnableKillingFile();
        {
            SvFileStream aOut(aTempFile.GetName(), STREAM_READ|STREAM_WRITE|STREAM_TRUNC);
            memset( pBuf, 0, 512 );
            aOut.Write( pBuf, 512 );        // Anfang Pict: 512 Byte Muell
            _CopyFilePiece( &aOut, pSt, nCopy, pBuf, WWBUFSIZ );    // Pict-Inhalt
            delete( pBuf );
        }

        GraphicFilter& rGF = *::GetGrfFilter(); // lese ueber Filter ein
        rpGraphic = new Graphic();
#if SUPD<591
        bOk = ( rGF.ImportGraphic( *rpGraphic, DirEntry(aTempFile.GetName()), GRFILTER_FORMAT_DONTKNOW )
                == 0 );
#else
        bOk = ( rGF.ImportGraphic( *rpGraphic, INetURLObject(aTempFile.GetName()), GRFILTER_FORMAT_DONTKNOW )
                == 0 );
#endif
        if( !bOk )
            DELETEZ( rpGraphic );

        return bOk;                 // Grafik drin
    }
#else   // ! USE_WMF_GRAF_FILE
                    // alt: ueber File mit selbstgebasteltem Header
    ULONG nMaxSize = rPic.lcb - rPic.cbHeader;
    WmfFileHd aHd;                              // lies Header
    ULONG nSiz = ReadWmfHeader( &aHd, nPosFc ); // FileSize in Bytes

    if( nSiz > nMaxSize )
        return FALSE;                           // irgendwas ist schiefgegangen

    DirEntry aFN = GetTmpFileName();
    aFN.SetExtension( String( "wmf" ) );
    aFN.ToAbs();
    rFileName = aFN.GetFull();

    SvFileStream aOut( rFileName, STREAM_WRITE );


    WriteWmfPreHd( rPic.dxaGoal, rPic.dyaGoal, aOut );  // Placeable WMF-Header

    ULONG nRead = nSiz;
    UINT16 nRead1;
    BYTE* pBuf = new BYTE[WWBUFSIZ];
    pSt->Seek( nPosFc );
    do{
        nRead1 = ( nRead > WWBUFSIZ ) ? WWBUFSIZ : (UINT16)nRead;
        pSt->Read( pBuf, nRead1 );
        aOut.Write( pBuf, nRead1 );
        nRead -= nRead1;
    }while( nRead );

    delete( pBuf );

    return TRUE;
#endif  // ! USE_WMF_GRAF_FILE
}

struct WW8PicDesc
{
    INT16 nCL, nCR, nCT, nCB;
    long nWidth, nHeight;

    WW8PicDesc( const WW8_PIC& rPic );
};

WW8PicDesc::WW8PicDesc( const WW8_PIC& rPic )
{
    long nOriWidth = (long)rPic.dxaGoal;        // Groesse in 1/100 mm ?
    long nOriHeight = (long)rPic.dyaGoal;

    nCL = rPic.dxaCropLeft;
    nCR = rPic.dxaCropRight;
    nCT = rPic.dyaCropTop;
    nCB = rPic.dyaCropBottom;

    long nAddWidth  = nOriWidth + nCL + nCR;        // Groesse nach Crop
    long nAddHeight = nOriHeight + nCT + nCB;
    if(!nAddWidth  )
        nAddWidth  = 1;
    if(!nAddHeight )
        nAddHeight = 1;

    nCL = (short)((long)nCL * nOriWidth / nAddWidth );// Crop absolut
    nCR = (short)((long)nCR * nOriWidth / nAddWidth );// -> skaliert
    nCT = (short)((long)nCT * nOriWidth / nAddWidth );// auf Original-ImageSize
    nCB = (short)((long)nCB * nOriWidth / nAddWidth );

    nWidth = nAddWidth * rPic.mx / 1000;        // entgueltige Sw-Groesse
    nHeight = nAddHeight * rPic.my / 1000;
}

/*
WW8PicDesc::WW8PicDesc( const WW8_FSPA& rFSPA )
{
    //long nOriWidth  = (long)rPic.dxaGoal;     // Groesse in 1/100 mm ?
    //long nOriHeight = (long)rPic.dyaGoal;

    nCL = rFSPA.nXaLeft;
    nCR = rFSPA.nXaRight;
    nCT = rFSPA.nYaTop;
    nCB = rFSPA.nYaBottom;

    //long nAddWidth  = nOriWidth  + nCL + nCR;     // Groesse nach Crop
    //long nAddHeight = nOriHeight + nCT + nCB;

    //nCL = (short)((long)nCL * nOriWidth / nAddWidth );// Crop absolut
    //nCR = (short)((long)nCR * nOriWidth / nAddWidth );// -> skaliert
    //nCT = (short)((long)nCT * nOriWidth / nAddWidth );// auf Original-ImageSize
    //nCB = (short)((long)nCB * nOriWidth / nAddWidth );

    nWidth  = nCR - nCL;
    nHeight = nCB - nCT;
}   */


// MakeGrafByFlyFmt setzt eine nicht-Zeichengebundene Grafik
SwFrmFmt* SwWW8ImplReader::MakeGrafByFlyFmt( SdrTextObj* pReplaceTextObj,
                                        const SwFrmFmt& rOldFmt, const WW8PicDesc& rPD,
                                        const Graphic* pGraph, const String& rFileName,
                                        const String& rGrName, const SfxItemSet& rGrfSet,
                                        const BOOL bSetToBackground )
{
    SwFrmFmt* pRet = 0;
    if( pReplaceTextObj )
    {
        // SdrGrafObj anstatt des SdrTextObj in dessen Gruppe einsetzen
        SdrObject* pGroupObject = pReplaceTextObj->GetUpGroup();
        if( pGroupObject )
        {
            SdrObjList* pObjectList = pGroupObject->GetSubList();

            // neues Sdr-Grafik-Objekt erzeugen und konfigurieren
            SdrGrafObj* pGrafObj = new SdrGrafObj;

            if( pGraph )
                pGrafObj->SetGraphic( *pGraph );

            if( !pDrawModel )   // 1. GrafikObjekt des Docs
                GrafikCtor();

            pGrafObj->SetModel( pDrawModel );

            pGrafObj->SetLogicRect( pReplaceTextObj->GetBoundRect() );

            pGrafObj->SetLayer( pReplaceTextObj->GetLayer() );


            /*
                schade: Wir koennen bisher keine SdrGrafObjekte mit Umrandung,
                        Schatten oder Hintergrundfuellung,
                        entsprechende Imports aus dem MS-Office muessen daher
                        auf die UEbernahme dieser Attribute verzichten.  :-(

            SfxItemSet aOldSet(pDrawModel->GetItemPool(),
                            // Box
                            XATTR_LINE_FIRST, XATTR_LINE_LAST,
                            // Shadow
                            SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST,
                            // Fill
                            XATTR_FILL_FIRST, XATTR_FILL_LAST,
                            0 );
            pReplaceTextObj->TakeAttributes( aOldSet, FALSE, TRUE );
            */


            if( rFileName.Len() )
            {
                ((SdrGrafObj*)pGrafObj)->SetFileName( rFileName );
                if( !pGraph )
                    pGrafObj->SetGraphicLink( rFileName, aEmptyStr );
            }
            // altes Objekt raus aus Gruppen-Liste und neues rein
            // (dies tauscht es ebenfalls in der Drawing-Page aus)
            pObjectList->ReplaceObject( pGrafObj, pReplaceTextObj->GetOrdNum() );


            /*
                schade: Wir koennen bisher keine SdrGrafObjekte mit Umrandung,
                        Schatten oder Hintergrundfuellung,
                        entsprechende Imports aus dem MS-Office muessen daher
                        auf die UEbernahme dieser Attribute verzichten.  :-(

            // Attribute aus dem alten SdrTextObjekt am SdrGrafObjekt setzen
            pGrafObj->SetAttributes( aOldSet, FALSE );
            */

        }
        else
        {
            // Riesenmist !!!
            ASSERT( !this, "keine Gruppe ? (wieso dann Ersetzen?)" );

            // wir haben derzeit nur Ersetzen vorgesehen, falls es ein Gruppen-Obj. ist !!!
        }
    }
    else
    {
        SfxItemSet aFlySet( rOldFmt.GetAttrSet() );

        if( bSetToBackground )
            aFlySet.Put( SvxOpaqueItem(RES_OPAQUE, FALSE) );

        // Groesse des alten Frames als Grafik-Groesse nehmen
        aFlySet.Put( SwFmtFrmSize( ATT_FIX_SIZE, rPD.nWidth, rPD.nHeight ) );

        // ggfs. horiz. Positionierungs-Relation korrigieren
        const SwFmtHoriOrient &rHori = rOldFmt.GetHoriOrient();
        if( FRAME == rHori.GetRelationOrient() )
            aFlySet.Put( SwFmtHoriOrient( rHori.GetPos(), HORI_NONE,
                                                          REL_PG_PRTAREA ) );
        // und rein damit ins Doc
        pRet = rDoc.Insert( *pPaM,  rFileName, aEmptyStr,
                            pGraph, &aFlySet,  &rGrfSet );

        // ggfs. fuer eindeutigen Grafik-Namen sorgen
        if( rGrName.Len() )
        {
            String aName;
            if(MakeUniqueGraphName(aName, rGrName))
                pRet->SetName( aName );
        }

        // bei Einfuegen in existierendes Doc: Frames erzeugen:
        if(     rDoc.GetRootFrm()
            &&  (FLY_AT_CNTNT == pRet->GetAnchor().GetAnchorId()) )
            pRet->MakeFrms();
    }
    return pRet;
}


// MakeGrafNotInCntnt setzt eine nicht-Zeichengebundene Grafik
// ( bGrafApo == TRUE )
SwFrmFmt* SwWW8ImplReader::MakeGrafNotInCntnt( const WW8PicDesc& rPD,
                   const Graphic* pGraph, const String& rFileName,
                   const String& rGrName, const SfxItemSet& rGrfSet )
{

    UINT32 nWidth = rPD.nWidth;
    UINT32 nHeight = rPD.nHeight;
//  nWidth += pSFlyPara->nLeMgn + pSFlyPara->nRiMgn;
//            + pSFlyPara->nLeLMgn + pSFlyPara->nRiLMgn;// bei bGrafApo ???
//  nHeight += pSFlyPara->nUpMgn + pSFlyPara->nLoMgn;
//            + pSFlyPara->nUpLMgn + pSFlyPara->nLoLMgn;// bei bGrafApo ???

                        // Vertikale Verschiebung durch Zeilenabstand
    INT32 nNetHeight = nHeight + rPD.nCT + rPD.nCB;
    if( pSFlyPara->nLineSpace && pSFlyPara->nLineSpace > nNetHeight )
        pSFlyPara->nYPos =
            (USHORT)( pSFlyPara->nYPos + pSFlyPara->nLineSpace - nNetHeight );

    WW8FlySet aFlySet( *this, pWFlyPara, pSFlyPara, TRUE );

    SwFmtAnchor aAnchor( pSFlyPara->eAnchor );
    aAnchor.SetAnchor( pPaM->GetPoint() );
    aFlySet.Put( aAnchor );

    aFlySet.Put( SwFmtFrmSize( ATT_FIX_SIZE, nWidth, nHeight ) );

    SwFlyFrmFmt* pFlyFmt = rDoc.Insert( *pPaM,
                    rFileName, aEmptyStr,   // Name der Grafik !!
                    pGraph,
                    &aFlySet,               // Attribute fuer den FlyFrm
                    &rGrfSet );             // Attribute fuer die Grafik
    if( rGrName.Len() )
    {
        String aName;
        if(MakeUniqueGraphName(aName, rGrName))
            pFlyFmt->SetName( aName );
    }

    // Damit die Frames bei Einfuegen in existierendes Doc erzeugt werden:
    if(     rDoc.GetRootFrm()
        &&  (FLY_AT_CNTNT == pFlyFmt->GetAnchor().GetAnchorId()) )
        pFlyFmt->MakeFrms();
    return pFlyFmt;
}


// MakeGrafInCntnt fuegt zeichengebundene Grafiken ein
SwFrmFmt* SwWW8ImplReader::MakeGrafInCntnt( const WW8_PIC& rPic, const WW8PicDesc& rPD,
                       const Graphic* pGraph, const String& rFileName,
                       const String& rGrName, const SfxItemSet& rGrfSet )
{
    WW8FlySet aFlySet( *this, pPaM, rPic, rPD.nWidth, rPD.nHeight );

    SwFrmFmt* pFlyFmt = 0;

    if( !rFileName.Len() && nObjLocFc )     // dann sollte ists ein OLE-Object
        pFlyFmt = ImportOle( pGraph, &aFlySet );


    if( !pFlyFmt )                          // dann eben als Graphic
        pFlyFmt = rDoc.Insert( *pPaM,
                    rFileName, aEmptyStr,   // Name der Grafik !!
                    pGraph,
                    &aFlySet,               // Attribute fuer den FlyFrm
                    &rGrfSet );             // Attribute fuer die Grafik

    if( rGrName.Len() )
    {
        String aName;
        if( MakeUniqueGraphName( aName, rGrName ))
            pFlyFmt->SetName( aName );
    }

    // Grafik im Rahmen ? ok, Rahmen auf Bildgroesse vergroessern
    //  ( nur wenn Auto-Breite )
    if( pSFlyPara )
        pSFlyPara->BoxUpWidth( rPD.nWidth );
    return pFlyFmt;
}

SwFrmFmt* SwWW8ImplReader::ImportGraf1( WW8_PIC& rPic, SvStream* pSt,
                                             ULONG nFilePos )
{
    SwFrmFmt* pRet = 0;
    if( pSt->IsEof() || rPic.fError || rPic.MFP.mm == 99 )
        return pRet;

    String aFileName;
    BOOL bInDoc;
    Graphic* pGraph = 0;
    BOOL bOk = ReadGrafFile( aFileName, pGraph, rPic, pSt, nFilePos, &bInDoc );

    if ( !bOk )
    {   delete pGraph;
        return pRet;                        // Grafik nicht korrekt eingelesen
    }

    WW8PicDesc aPD( rPic );


    SwAttrSet aGrfSet( rDoc.GetAttrPool(), RES_GRFATR_BEGIN, RES_GRFATR_END-1);
    if( aPD.nCL || aPD.nCR || aPD.nCT || aPD.nCB )
    {
        SwCropGrf aCrop( aPD.nCL, aPD.nCR, aPD.nCT, aPD.nCB) ;
        aGrfSet.Put( aCrop );
    }

    if( pWFlyPara && pWFlyPara->bGrafApo )
        pRet = MakeGrafNotInCntnt( aPD, pGraph, aFileName, aEmptyStr, aGrfSet );
    else
        pRet = MakeGrafInCntnt( rPic, aPD, pGraph, aFileName, aEmptyStr, aGrfSet );
    delete pGraph;
    return pRet;
}



BOOL SwWW8ImplReader::WW8QuickHackForMSDFF_DirectBLIPImport(SvStream& rSt,
                                                            WW8_PIC& rPic,
                                                            Graphic& rData,
                                                            String& rGraphName)
{
    BOOL bOk = FALSE;
    ULONG nFilePos = 0;
    ULONG nBLIPLen = 0;
    ULONG nLength;
    ULONG nSpContainerLen;
    ULONG nCode;
    rSt >> nSpContainerLen;

    pMSDffManager->RetrieveNameOfBLIP( rSt,
                                       rGraphName,
                                       nSpContainerLen );
    rSt.SeekRel( nSpContainerLen );
    rSt >> nCode;
    if(0xF0070000 == (nCode & 0xFFFF0000))
    {
        ULONG nLenFBSE;
        rSt >> nLength;

        const ULONG nSkipBLIPLen  = 20; // bis zu nBLIPLen zu ueberspringende Bytes
        const ULONG nSkipShapePos =  4; // dahinter bis zu nShapePos zu skippen
        const ULONG nSkipBLIP           =  4; // dahinter schliesst dann das BLIP an

        nLenFBSE = nLength;

        // ist FBSE gross genug fuer unsere Daten
        BOOL bOk2 = ( nSkipBLIPLen + 4 + nSkipShapePos + 4 <= nLenFBSE );
        if( bOk2 )
        {
            rSt.SeekRel( nSkipBLIPLen );
            rSt >> nBLIPLen;
            rSt.SeekRel( nSkipShapePos );
            rSt >> nFilePos;                        // FilePos des Shape!
            rSt.SeekRel( nSkipBLIP );
            bOk2 = (0 == rSt.GetError());
            nLength -= nSkipBLIPLen+ 4 + nSkipShapePos + 4;
            if( 0 > nLength )
                nLength = 0;
        }
        if( bOk2 )
        {
            // das BLIP Atom steht im Daten Stream unmittelbar hinter dem FBSE
            //
            bOk = pMSDffManager->GetBLIPDirect( rSt, rData );
        }
    }
    return bOk;
}

BOOL SwWW8ImplReader::ImportURL(String &sURL,String &sMark,WW8_CP nStart)
{
    BOOL bRet=FALSE;
    /*
     * Save the reader state and process the sprms for this anchor cp.
     * Doing so will set the nPicLocFc to the offset to find the hypertext
     * data in the data stream.
     */
    WW8_CP nEndCp = nStart+1; //Only interested in the single 0x01 character

    WW8ReaderSave aSave(this,nStart);

    WW8PLCFManResult aRes;
    nStart = pPlcxMan->Where();
    while(nStart <= nEndCp)
    {
        if ( pPlcxMan->Get(&aRes)
            && aRes.pMemPos && aRes.nSprmId )
        {
            //only interested in sprms which would set nPicLocFc
            if ( (68 == aRes.nSprmId) || (0x6A03 == aRes.nSprmId) )
            {
                Read_PicLoc( aRes.nSprmId, aRes.pMemPos+ 1+
                (8 > pWwFib->nVersion ? 0 : 1) + WW8SprmDataOfs(aRes.nSprmId),
                4);
                break;
            }
        }
        (*pPlcxMan)++;
        nStart = pPlcxMan->Where();
    }
    long nOffset = nPicLocFc;
    aSave.Restore(this);

    long nOldPos = pDataStream->Tell();
    WW8_PIC aPic;
    pDataStream->Seek( nOffset);
#ifdef __WW8_NEEDS_COPY
    WW8_PIC_SHADOW aPicS;
    pDataStream->Read( &aPicS, sizeof( aPicS ) );
    WW8PicShadowToReal( &aPicS, &aPic );
#else
    pDataStream->Read( &aPic, sizeof( aPic ) );
#endif
    if((aPic.lcb > 0x44) && !pDataStream->GetError() )
    {
#ifdef DEBUG
        pDataStream->SeekRel( 3 );
        const BYTE MAGIC_A[16] =
        {
            0xd0, 0xc9, 0xea, 0x79, 0xf9, 0xba, 0xce, 0x11,
            0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b
        };
        BYTE test[16];

        pDataStream->Read( test, 16 );
        ASSERT(!memcmp(MAGIC_A,test,16),
            "Found a different MAGIC_A, cause for concern");
#else
        pDataStream->SeekRel(19);
#endif
        ULONG nLen,nFlags;
        *pDataStream >> nLen;
        ASSERT(nLen == 2,"Wrong flag len, cause for concern");
        *pDataStream >> nFlags;
        /* all bits zero except for the bottom 4 bits which are...
         * 0: always set. bottom bit set to denote a link ?
         * 1: set to denote an absolute link
         * 2: never set ?
         * 3: set to denote an anchor
         */

        const BYTE MAGIC_B[16] =
        {
            0xe0, 0xc9, 0xea, 0x79, 0xf9, 0xba, 0xce, 0x11,
            0x8c, 0x82, 0x00, 0xaa, 0x00, 0x4b, 0xa9, 0x0b
        };
        const BYTE MAGIC_C[18] =
        {
            0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46,
            0x00, 0x00
        };

        BYTE testb[16];
        int nVersion=-1;

        pDataStream->Read( testb, 16 );
        if (memcmp(MAGIC_B,testb,16) == 0)
            nVersion=1;
        else if (memcmp(MAGIC_C,testb,16) == 0)
            nVersion=2;
        else
        {
            ASSERT(!this,"Found neither MAGIC_B or C, cause for concern");
            nVersion=-1;
        }

        switch(nVersion)
        {
            case 1:
                {
                    *pDataStream >> nLen;
                    sURL = WW8Read_xstz( *pDataStream, (USHORT)nLen/2, FALSE );
                    if (nFlags & 0x08)
                    {
                    long pos = pDataStream->Tell();
                    *pDataStream >> nLen;
                    sMark = WW8Read_xstz( *pDataStream, (USHORT)nLen, FALSE );
                    bRet = TRUE;
                    }
                }
                break;
            case 2:
                {
                    pDataStream->SeekRel(2); // skip over the last two bytes
                    *pDataStream >> nLen;
                    pDataStream->SeekRel(nLen); // skip over the 1st filename
                    pDataStream->SeekRel(24); // skip over MAGIC_D

                    *pDataStream >> nLen;   //full len
                    *pDataStream >> nLen;   //real str len
                    pDataStream->SeekRel(2); // skip over the value 00 03
                    sURL = WW8Read_xstz( *pDataStream, (USHORT)nLen/2, FALSE );
                    if (nFlags & 0x08)
                    {
                    *pDataStream >> nLen;
                    sMark = WW8Read_xstz( *pDataStream, (USHORT)nLen, FALSE );
                    bRet = TRUE;
                    }
                }
                break;
        }
    }
    return( bRet );
}


SwFrmFmt* SwWW8ImplReader::ImportGraf( SdrTextObj* pTextObj,
                                       SwFrmFmt* pOldFlyFmt,
                                       BOOL bSetToBackground )
{
    SwFrmFmt* pRet = 0;
    if(    ( ( pStrm == pDataStream ) && !nPicLocFc )
            || ( nIniFlags & WW8FL_NO_GRAF )                )
        return pRet;

    ::SetProgressState( nProgress, rDoc.GetDocShell() );         // Update

    /*
        kleiner Spass von Microsoft: manchmal existiert ein Stream Namens DATA
        Dieser enthaelt dann den PICF und die entsprechende Grafik !!!
        Wir mappen ansonsten die Variable pDataStream auf pStream.
    */

    long nOldPos = pDataStream->Tell();
    WW8_PIC aPic;
    pDataStream->Seek( nPicLocFc );
#ifdef __WW8_NEEDS_COPY
    WW8_PIC_SHADOW aPicS;
    pDataStream->Read( &aPicS, sizeof( aPicS ) );
    WW8PicShadowToReal( &aPicS, &aPic );
#else
    pDataStream->Read( &aPic, sizeof( aPic ) );
#endif // defined __WW8_NEEDS_COPY

        // Plausibilitaetstest ist noetig, da z.B. bei CheckBoxen im
        // Feld-Result ein WMF-aehnliches Struct vorkommt.
    if(     (aPic.lcb >= 58)
            && !pDataStream->GetError() )
    {
        if( pFlyFmtOfJustInsertedGraphic )
        {
            // Soeben haben wir einen Grafik-Link ins Doc inserted.
            // Wir muessen ihn jetzt noch Positioniern und Skalieren.
            //
            WW8PicDesc aPD( aPic );

            WW8FlySet aFlySet( *this, pPaM, aPic, aPD.nWidth, aPD.nHeight );

            pFlyFmtOfJustInsertedGraphic->SetAttr( aFlySet );


            // ACHTUNG: Zuschneiden ist hier noch nicht implementiert!

            // einfach die Crop-Infos bauen und in aFlySet setzen ???
            /*
            if( aPD.nCL || aPD.nCR || aPD.nCT || aPD.nCB )
            {
                SwCropGrf aCrop;
                aCrop.Left()   = aPD.nCL;
                aCrop.Right()  = aPD.nCR;
                aCrop.Top()    = aPD.nCT;
                aCrop.Bottom() = aPD.nCB;
                pFlyFmtOfJustInsertedGraphic->SetAttr( aCrop ); // ? ? ?
            }
            */


            pFlyFmtOfJustInsertedGraphic = 0;
        }
        else if((0x64 == aPic.MFP.mm) || (0x66 == aPic.MFP.mm))
        {
            pDataStream->SeekRel( 2 );
            // Tja, da haben wir wohl ein MSO Drawing File Format am Bein.
            //
            SdrObject* pObject = 0;
            WW8PicDesc aPD( aPic );
            String aGrName;
            if( 0x66 == aPic.MFP.mm )   // verlinkte Grafik im Escher-Objekt
            {                           // ================    =============
                BYTE nNameLen;
                *pDataStream >> nNameLen;
                pDataStream->SeekRel( nNameLen );

                Rectangle aRect( 0,0, aPD.nWidth,  aPD.nHeight);
                SvxMSDffImportData aData( aRect );
                pObject =
//                  pMSDffManager->ImportObjAtCurrentStreamPos(*pDataStream,
//                                                              aData );
                    pMSDffManager->ImportObj(*pDataStream, &aData, &aRect);
                if( pObject )
                {
                    // fuer den Rahmen
                    SfxItemSet aAttrSet( rDoc.GetAttrPool(),
                                         RES_FRMATR_BEGIN,
                                         RES_FRMATR_END-1 );
                    // fuer die Grafik
                    SfxItemSet aGrSet( rDoc.GetAttrPool(),
                                       RES_GRFATR_BEGIN,
                                       RES_GRFATR_END-1 );

                    if( aPD.nCL || aPD.nCR || aPD.nCT || aPD.nCB )
                    {
                        SwCropGrf aCrop( aPD.nCL, aPD.nCR, aPD.nCT, aPD.nCB );
                        aGrSet.Put( aCrop );
                    }

                    SwFrmFmt* pNewFlyFmt = 0;
                    BOOL bTextObjWasGrouped = FALSE;
                    // Groesse aus der WinWord PIC-Struktur als Grafik-Groesse nehmen
                    aAttrSet.Put( SwFmtFrmSize( ATT_FIX_SIZE, aPD.nWidth, aPD.nHeight ) );

                    // ggfs. altes AttrSet uebernehmen und
                    // horiz. Positionierungs-Relation korrigieren
                    if( pOldFlyFmt )
                    {
                        aAttrSet.Put( pOldFlyFmt->GetAttrSet() );
                        const SwFmtHoriOrient &rHori = pOldFlyFmt->GetHoriOrient();
                        if( FRAME == rHori.GetRelationOrient() )
                            aAttrSet.Put( SwFmtHoriOrient( rHori.GetPos(),
                                                          HORI_NONE,
                                                          REL_PG_PRTAREA ) );
                    }

                    XubString aObjectName( pObject->GetName() );
                    if( UINT16( OBJ_OLE2 ) == pObject->GetObjIdentifier() )
                    {
                        SvInPlaceObjectRef xIPRef(
                                    ((SdrOle2Obj*)pObject)->GetObjRef() );

                        // kein GrafSet uebergeben, da nur fuer Cropp sinnvoll,
                        // was die UI derzeit (27.1.99) noch nicht kann khz.
                        pNewFlyFmt = rDoc.Insert( *pPaM, &xIPRef, &aAttrSet );
                    }
                    else
                    {
                        SdrGrafObj* pGraphObject = 0;
                        pGraphObject = PTR_CAST(SdrGrafObj, pObject);
                        if( pGraphObject )
                        {
                            // Nun den Link bzw. die Grafik ins Doc stopfen
                            const Graphic& rGraph = pGraphObject->GetGraphic();
                            BOOL bDone = FALSE;

                            if( pGraphObject->IsLinkedGraphic() )
                            {
                                aGrName = pGraphObject->GetFileName();
                                DirEntry aGrfFileEntry( aGrName );
                                if(    aGrfFileEntry.Exists()
                                    || (GRAPHIC_NONE == rGraph.GetType()))
                                {
                                    if( pOldFlyFmt && pTextObj && pTextObj->GetUpGroup() )
                                    {
                                        bTextObjWasGrouped = TRUE;
                                        /*
                                            Hier *nichts* ins Doc inserten!
                                            ( lediglich in der DrawPage statt
                                              dem Sdr-Text-Objekt einen Grafik-
                                              Link einbauen )
                                        */
                                        pNewFlyFmt= MakeGrafByFlyFmt(
                                                        pTextObj, *pOldFlyFmt,
                                                        aPD, 0, aGrName,
                                                        aEmptyStr, aGrSet,
                                                        bSetToBackground );
                                    }
                                    else
                                        pNewFlyFmt= rDoc.Insert(*pPaM,
                                                                aGrName,
                                                                aEmptyStr,
                                                                0,          // Graphic*
                                                                &aAttrSet,  // SfxItemSet* Rahmen
                                                                &aGrSet );  // SfxItemSet* Grafik
                                    bDone = TRUE;
                                }
                            }

                            if( !bDone )
                                pNewFlyFmt= rDoc.Insert(*pPaM,
                                                        aEmptyStr,
                                                        aEmptyStr,
                                                        &rGraph,
                                                        &aAttrSet,      // SfxItemSet* Rahmen
                                                        &aGrSet );      // SfxItemSet* Grafik

                        }
                    }
                    if( pNewFlyFmt )  // also nur, wenn wir ein *Insert* gemacht haben
                    {
                        pRet = pNewFlyFmt;
                        // mehrfaches Auftreten gleicher Grafik-Namen vermeiden
                        if( aObjectName.Len() )
                        {
                            String aName;
                            if( MakeUniqueGraphName( aName, aObjectName ))
                                pNewFlyFmt->SetName( aName );
                        }
                        // Zeiger auf neues Objekt ermitteln und
                        // Z-Order-Liste entsprechend korrigieren (oder Eintrag loeschen)
                        SdrObject* pOurNewObject = pNewFlyFmt->FindSdrObject();
                        if( !pOurNewObject )
                        {
                            if( !pDrawModel )   // 1. GrafikObjekt des Docs
                                GrafikCtor();

                            SwFlyDrawContact* pContactObject
                                = new SwFlyDrawContact( (SwFlyFrmFmt*)pNewFlyFmt,
                                                        pDrawModel );
                            pOurNewObject = pContactObject->GetMaster();
                        }
                        if( pOurNewObject )
                        {
                            pMSDffManager->ExchangeInShapeOrder( pObject, 0, 0, pOurNewObject );
                            // Das Kontakt-Objekt MUSS in die Draw-Page gesetzt werden,
                            // damit in SwWW8ImplReader::LoadDoc1() die Z-Order
                            // festgelegt werden kann !!!
                            pDrawPg->InsertObject( pOurNewObject );

                            // altes SdrGrafObj aus der Page loeschen und zerstoeren
                            pDrawPg->RemoveObject( pObject->GetOrdNum() );
                            delete pObject;
                        }
                        else
                            pMSDffManager->RemoveFromShapeOrder( pObject );
                    }
                    else
                        pMSDffManager->RemoveFromShapeOrder( pObject );

                    // auch das ggfs. uebergebene alte Sdr-Text-Objekt aus der Page
                    // loeschen, falls nicht gruppiert,
                    if( pTextObj && !bTextObjWasGrouped )
                    {
                        pDrawPg->RemoveObject( pTextObj->GetOrdNum() );
                    }
                }
            }
            else                        // eingebettete Grafik im Escher-Objekt
            {                           // ===================    =============
                Graphic aGraph;
                ULONG nCode;
                *pDataStream >> nCode;
                if(0xF004000F == nCode)
                {
                    if( !WW8QuickHackForMSDFF_DirectBLIPImport(
                                        *pDataStream, aPic, aGraph, aGrName) )
                    {
                        ASSERT( !this, "Wo ist die Grafik ?" );
                    }
                    else
                    {
                        // Positionieren, Skalieren und Zuschneiden des Pictures
                        //
                        SfxItemSet aGrfSet(rDoc.GetAttrPool(), RES_GRFATR_BEGIN,
                                                               RES_GRFATR_END-1);
                        if( aPD.nCL || aPD.nCR || aPD.nCT || aPD.nCB )
                        {
                            SwCropGrf aCrop( aPD.nCL, aPD.nCR, aPD.nCT, aPD.nCB );
                            aGrfSet.Put( aCrop );
                        }
                        if( pOldFlyFmt )
                            pRet = MakeGrafByFlyFmt(pTextObj, *pOldFlyFmt, aPD,
                                                    &aGraph,
                                                    aEmptyStr,
                                                    aGrName,
                                                    aGrfSet, bSetToBackground);
                        else
                            if( pWFlyPara && pWFlyPara->bGrafApo )
                                pRet = MakeGrafNotInCntnt( aPD, &aGraph, aEmptyStr, aGrName, aGrfSet );
                            else
                                pRet = MakeGrafInCntnt( aPic, aPD, &aGraph, aEmptyStr, aGrName, aGrfSet );
                    }
                }
                else
                {
                    // leider verloren...
                    ASSERT( !this, "Wo ist das Shape fuer die Grafik ?" );
                }
            }
        }
        else if(   (aPic.lcb >= 58)
                 && aPic.MFP.xExt
                 && aPic.MFP.yExt )
        {
            pRet = ImportGraf1( aPic, pDataStream, nPicLocFc );
        }
    }
    pDataStream->Seek( nOldPos );
    return pRet;
}

#ifdef __WW8_NEEDS_COPY

void WW8PicShadowToReal( WW8_PIC_SHADOW * pPicS, WW8_PIC * pPic )
{
    pPic->lcb = SVBT32ToLong( pPicS->lcb );
    pPic->cbHeader = SVBT16ToShort( pPicS->cbHeader );
    pPic->MFP.mm = SVBT16ToShort( pPicS->MFP.mm );
    pPic->MFP.xExt = SVBT16ToShort( pPicS->MFP.xExt );
    pPic->MFP.yExt = SVBT16ToShort( pPicS->MFP.yExt );
    pPic->MFP.hMF = SVBT16ToShort( pPicS->MFP.hMF );
    for( USHORT i = 0; i < 14 ; i++ )
        pPic->rcWinMF[i] = SVBT8ToByte( pPicS->rcWinMF[i] );
    pPic->dxaGoal = SVBT16ToShort( pPicS->dxaGoal );
    pPic->dyaGoal = SVBT16ToShort( pPicS->dyaGoal );
    pPic->mx = SVBT16ToShort( pPicS->mx );
    pPic->my = SVBT16ToShort( pPicS->my );
    pPic->dxaCropLeft = SVBT16ToShort( pPicS->dxaCropLeft );
    pPic->dyaCropTop = SVBT16ToShort( pPicS->dyaCropTop );
    pPic->dxaCropRight = SVBT16ToShort( pPicS->dxaCropRight );
    pPic->dyaCropBottom = SVBT16ToShort( pPicS->dyaCropBottom );
    pPic->brcl = pPicS->aBits1[0] & 0x0f;
    pPic->fFrameEmpty = (pPicS->aBits1[0] & 0x10) >> 4;
    pPic->fBitmap = (pPicS->aBits1[0] & 0x20) >> 5;
    pPic->fDrawHatch = (pPicS->aBits1[0] & 0x40) >> 6;
    pPic->fError = (pPicS->aBits1[0] & 0x80) >> 7;
    pPic->bpp = pPicS->aBits2[0];
    for( USHORT j = 0; j < 4; j++ ){
        pPic->rgbrc[j].aBits1[0] = pPicS->rgbrc[j].aBits1[0];
        pPic->rgbrc[j].aBits1[1] = pPicS->rgbrc[j].aBits1[1];
    }
    pPic->dxaOrigin = SVBT16ToShort( pPicS->dxaOrigin );
    pPic->dyaOrigin = SVBT16ToShort( pPicS->dyaOrigin );
}

void WW8FSPAShadowToReal( WW8_FSPA_SHADOW * pFSPAS, WW8_FSPA * pFSPA )
{
    //long nSpId;       //Shape Identifier. Used in conjunction with the office art data (found via fcDggInfo in the FIB) to find the actual data for this shape.
    //long nXaLeft; //left of rectangle enclosing shape relative to the origin of the shape
    //long nYaTop;      //top of rectangle enclosing shape relative to the origin of the shape
    //long nXaRight;    //right of rectangle enclosing shape relative to the origin of the shape
    //long nYaBottom;//bottom of the rectangle enclosing shape relative to the origin of the shape
    //USHORT bHdr:1;
    //USHORT nbx:2;
    //USHORT nby:2;
    //USHORT nwr:4;
    //USHORT nwrk:4;
    //USHORT bRcaSimple:1;
    //USHORT bAnchorLock:1;
    //long nTxbx; //count of textboxes in shape (undo doc only)


    pFSPA->nSpId        = SVBT32ToLong( pFSPAS->nSpId );
    pFSPA->nXaLeft      = SVBT32ToLong( pFSPAS->nXaLeft );
    pFSPA->nYaTop       = SVBT32ToLong( pFSPAS->nYaTop );
    pFSPA->nXaRight     = SVBT32ToLong( pFSPAS->nXaRight );
    pFSPA->nYaBottom    = SVBT32ToLong( pFSPAS->nYaBottom );

    USHORT nBits            = (USHORT)SVBT16ToShort( pFSPAS->aBits1 );

    pFSPA->bHdr         = 0 !=  ( nBits & 0x0001 );
    pFSPA->nbx          =       ( nBits & 0x0006 ) >> 1;
    pFSPA->nby          =       ( nBits & 0x0018 ) >> 3;
    pFSPA->nwr          =       ( nBits & 0x01E0 ) >> 5;
    pFSPA->nwrk         =       ( nBits & 0x1E00 ) >> 9;
    pFSPA->bRcaSimple   = 0 !=  ( nBits & 0x2000 );
    pFSPA->bBelowText   = 0 !=  ( nBits & 0x4000 );
    pFSPA->bAnchorLock  = 0 !=  ( nBits & 0x8000 );
    pFSPA->nTxbx = SVBT32ToLong( pFSPAS->nTxbx );
}
#endif // defined __WW8_NEEDS_COPY


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/ww8graf2.cxx,v 1.1.1.1 2000-09-18 17:14:58 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.39  2000/09/18 16:04:59  willem.vandorp
      OpenOffice header added.

      Revision 1.38  2000/09/12 13:19:04  kz
      add. include tools/fsys.hxx

      Revision 1.37  2000/08/25 12:27:43  jp
      Graphic Crop-Attribut exported to SVX

      Revision 1.36  2000/08/17 10:24:12  khz
      #77690# prevent division by zero if Picture Size is zero

      Revision 1.35  2000/06/13 08:24:46  os
      using UCB

      Revision 1.34  2000/05/25 08:07:01  khz
      Piece Table optimization, Unicode changes, Bugfixes

      Revision 1.33  2000/05/16 12:12:57  jp
      ASS_FALSE define removed

      Revision 1.32  2000/05/16 11:14:50  khz
      Unicode code-conversion

      Revision 1.31  2000/04/04 16:05:17  hr
      #65293#: WW8PicShadowToReal()

      Revision 1.30  2000/02/21 13:08:19  jp
      #70473# changes for unicode

      Revision 1.29  2000/02/18 09:35:10  cmc
      #69372# Improved Hyperlink Importing for WW97

      Revision 1.28  2000/02/11 14:40:08  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.27  1999/11/19 15:02:46  khz
      Special Task! Use new method to import Shapes: SvxMSDffManager::ImportObj()

      Revision 1.26  1999/08/09 12:16:04  JP
      read W95 format from stream


      Rev 1.25   09 Aug 1999 14:16:04   JP
   read W95 format from stream

      Rev 1.24   06 Aug 1999 18:06:36   KHZ
   Task #65322# Changes in WW8FSPAShadowToReal() (#2)

      Rev 1.23   08 Jun 1999 19:05:50   KHZ
   Task #65322# Changes in WW8FSPAShadowToReal()

      Rev 1.22   02 Jun 1999 09:32:26   KHZ
   Task #66227# a) kein Unicode bei Ver67 ;-)  b) Grafik in grupp. Textbox

      Rev 1.21   26 Apr 1999 17:40:24   KHZ
   Task #61381# Attribute von Textboxen in denen NUR EINE Grafik steht

      Rev 1.20   18 Feb 1999 20:24:38   KHZ
   Task #61659# Umlauf im Hintergrund

      Rev 1.19   17 Feb 1999 15:23:42   KHZ
   Task #61382# jetzt auch Textbox-Grafiken im Hintergrund ( SvxOpaqItem(FALSE) )

      Rev 1.18   12 Feb 1999 16:51:06   KHZ
   Task #61381# Ersetzen von Sdr-Text-Objekten im Writer durch Rahmen (3)

      Rev 1.17   05 Feb 1999 10:32:46   KHZ
   Task #61381# in SdrTextObj verankerte Grafiken und OLEs (5)

      Rev 1.16   28 Jan 1999 17:10:48   KHZ
   Task #60715# in SdrTextObj verankerte Grafiken und OLEs (4)

      Rev 1.15   27 Jan 1999 12:22:40   KHZ
   Task #60715# in SdrTextObj verankerte Grafiken und OLEs (3)

      Rev 1.14   27 Jan 1999 10:58:02   KHZ
   Task #60715# in SdrTextObj verankerte Grafiken und OLEs (2)

      Rev 1.13   25 Jan 1999 10:25:46   KHZ
   Task #60715# in Textobjekt verankerte Grafik als Grafik importieren

      Rev 1.12   30 Nov 1998 17:46:54   JP
   Task #59822#: OLE-Objecte importieren

      Rev 1.11   24 Nov 1998 20:46:22   JP
   Task #59822#: OLE-Objecte einlesen

      Rev 1.10   03 Sep 1998 22:14:22   KHZ
   Task #55189# Textboxen

      Rev 1.9   05 Aug 1998 17:16:24   KHZ
   Task #53614# komprimierte WMF, Enh.Metafile und PICT Grafiken

      Rev 1.8   30 Jul 1998 20:52:38   KHZ
   Task #52607# GPF unter Unix behoben

      Rev 1.7   30 Jul 1998 14:31:34   KHZ
   Task #52607# Grafikimport ueber Quick-Hack auch fuer WMF

      Rev 1.6   30 Jul 1998 00:03:10   KHZ
   Task #53614# Grafiken, die NICHT ueber dem Text liegen (MSDFF-Quick-Hack)

      Rev 1.5   22 Jul 1998 16:16:04   HR
   ww8graf fuer UNX

      Rev 1.4   21 Jul 1998 14:52:10   KHZ
   Task #52607# (WW 97 Import)

      Rev 1.3   21 Jul 1998 12:33:46   KHZ
   als MSDrawingObject eingebettete Grafik (Teil 1)

      Rev 1.2   13 Jul 1998 15:29:16   KHZ
   Task #52607# embeded Grafiken (NICHT UEBER den Text gelegte)

      Rev 1.1   30 Jun 1998 21:33:18   KHZ
   Header/Footer/Footnotes weitgehend ok

      Rev 1.0   16 Jun 1998 11:06:48   KHZ
   Initial revision.

*************************************************************************/




