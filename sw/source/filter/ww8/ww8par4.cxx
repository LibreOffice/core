/*************************************************************************
 *
 *  $RCSfile: ww8par4.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-01 12:12:16 $
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

#include <tools/solar.h>

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _TOOLS_TEMPFILE_HXX
#include <tools/tempfile.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _FILTER_HXX //autogen
#include <svtools/filter.hxx>
#endif
#ifndef _SVDOOLE2_HXX //autogen
#include <svx/svdoole2.hxx>
#endif
#ifndef _MSDFFIMP_HXX
#include <svx/msdffimp.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX
#include <svx/impgrf.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen wg. SwFmtCntnt
#include <fmtcntnt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDGRF_HXX //autogen wg. SwGrfNode
#include <ndgrf.hxx>
#endif
/*
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif*/
#ifndef _DOCSH_HXX
#include <docsh.hxx>            // fuer Ole-Node
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // Progress
#endif
#ifndef _WW8SCAN_HXX
#include <ww8scan.hxx>          // __WW_NEEDS_COPY
#endif
#ifndef _WW8PAR_HXX
#include <ww8par.hxx>
#endif
#ifndef _WW8PAR2_HXX
#include <ww8par2.hxx>          // WWFlyPara::BoxUpWidth()
#endif
/*
#ifndef _REDLENUM_HXX
#include <redlenum.hxx>
#endif*/
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _FLTSHELL_HXX
#include <fltshell.hxx>
#endif
#ifndef _MSOCXIMEX_HXX
#include <svx/msocximex.hxx>
#endif
#ifndef _UNODRAW_HXX
#include <unodraw.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif


struct OLE_MFP
{
    INT16 mm;       // 0x6  int
    INT16 xExt;     // 0x8  int in 1/100 mm
    INT16 yExt;     // 0xa  int in 1/100 mm
    INT16 hMF;      // 0xc  int
};


SV_IMPL_OP_PTRARR_SORT(WW8AuthorInfos, WW8AuthorInfo_Ptr)


static BOOL SwWw8ReadScaling( INT16& rX, INT16& rY, SvStorageRef& rSrc1 )
{
    // Skalierungsfaktoren holen:
    //      Informationen in PIC-Stream ( durch ausprobieren )
    //      0x0  (l)cb
    //      0x08 .. 0x0a Flags ??
    //      0x08 Inh: 1 / 0
    //      0x09 Inh: 0,8,0x18
    //      0x0a Inh: immer 8, MAP_ANISOTROPIC ???
    //      0x0b Inh: immer 0
    //      0x0c, 0x10 Originalgroesse x,y in 1/100 mm
    //      0x14, 0x16 Originalgroesse x,y in tw
    //      0x2c, 0x30 Skalierung x,y in Promille
    //      0x34, 0x38, 0x3c, 0x40 Crop Left, Top, Right, Bot in tw

    SvStorageStreamRef xSrc3 = rSrc1->OpenStream( WW8_ASCII2STR( "\3PIC" ));
    SvStorageStream* pS = xSrc3;
    pS->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    pS->Seek( STREAM_SEEK_TO_END );

    ASSERT( pS->Tell() >=  76, "+OLE-PIC-Stream is shorter than 76 Byte" );

    INT32 nOrgWidth,
          nOrgHeight,
          nScaleX,
          nScaleY,
          nCropLeft,
          nCropTop,
          nCropRight,
          nCropBottom;
    pS->Seek( 0x14 );
    *pS >> nOrgWidth    // Original Size in 1/100 mm
        >> nOrgHeight;
    pS->Seek( 0x2c );
    *pS >> nScaleX      // Scaling in Promille
        >> nScaleY
        >> nCropLeft    // Cropping in 1/100 mm
        >> nCropTop
        >> nCropRight
        >> nCropBottom;

    rX = nOrgWidth  - nCropLeft - nCropRight;
    rY = nOrgHeight - nCropTop  - nCropBottom;
    if(       10 > nScaleX
        || 65536 < nScaleX
        ||    10 > nScaleY
        || 65536 < nScaleY)
    {
        ASSERT( !pS, "+OLE-Scalinginformation in PIC-Stream wrong" );
        return FALSE;
    }
    else
    {
        rX = (INT16)( (long)rX * nScaleX / 1000 );
        rY = (INT16)( (long)rY * nScaleY / 1000 );
    }
    return TRUE;
}

static BOOL SwWw6ReadMetaStream( GDIMetaFile& rWMF, OLE_MFP* pMfp,
                         SvStorageRef& rSrc1 )
{
    SvStorageStreamRef xSrc2 = rSrc1->OpenStream( WW8_ASCII2STR( "\3META" ));
    SvStorageStream* pSt = xSrc2;
    pSt->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    ULONG nRead = pSt->Read( pMfp, sizeof(*pMfp ) );
                                // Mini-Placable-Header lesen
    if( nRead != sizeof( *pMfp ) )
        return FALSE;

#if defined  __BIGENDIAN
    pMfp->mm = SWAPSHORT( pMfp->mm );
    pMfp->xExt = SWAPSHORT( pMfp->xExt );
    pMfp->yExt = SWAPSHORT( pMfp->yExt );
#endif // __BIGENDIAN

    if( pMfp->mm == 94 || pMfp->mm == 99 )
    {
        ASSERT( !pSt, "+OLE: Falscher Metafile-Typ" );
        return FALSE;
    }
    if( pMfp->mm != 8 )
    {
        ASSERT( !pSt, "+OLE: Falscher Metafile-Typ ( nicht Anisotropic )" );
    }
    if( !pMfp->xExt || !pMfp->yExt )
    {
        ASSERT( !pSt, "+OLE: Groesse von 0 ???" );
        return FALSE;
    }
    BOOL bOk = ReadWindowMetafile( *pSt, rWMF );    // WMF lesen
                    // *pSt >> aWMF  geht nicht ohne placable Header
    if( !bOk || pSt->GetError() || rWMF.GetActionCount() == 0 ){
        ASSERT( !pSt, "+OLE: Konnte Metafile nicht lesen" );
        return FALSE;
    }

    rWMF.SetPrefMapMode( MapMode( MAP_100TH_MM ) );


    // MetaFile auf neue Groesse skalieren und
    // neue Groesse am MetaFile setzen
    Size        aOldSiz( rWMF.GetPrefSize() );
    Size        aNewSiz( pMfp->xExt, pMfp->yExt );
    Fraction    aFracX( aNewSiz.Width(), aOldSiz.Width() );
    Fraction    aFracY( aNewSiz.Height(), aOldSiz.Height() );

    rWMF.Scale( aFracX, aFracY );
    rWMF.SetPrefSize( aNewSiz );

    return TRUE;
}

static BOOL SwWw6ReadMacPICTStream( Graphic& rGraph, SvStorageRef& rSrc1 )
{
        // 03-META-Stream nicht da. Vielleicht ein 03-PICT ?
    SvStorageStreamRef xSrc4 = rSrc1->OpenStream( WW8_ASCII2STR( "\3PICT" ));
    SvStorageStream* pStp = xSrc4;
    pStp->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    BYTE aTestA[10];        // Ist der 01Ole-Stream ueberhaupt vorhanden
    ULONG nReadTst = pStp->Read( aTestA, sizeof( aTestA ) );
    if(nReadTst != sizeof(aTestA))
        return FALSE;

    pStp->Seek( STREAM_SEEK_TO_BEGIN );

            // Mac-Pict steht im 03PICT-StorageStream
            // allerdings ohne die ersten 512 Bytes, die
            // bei einem MAC-PICT egal sind ( werden nicht ausgewertet )

    return SwWW8ImplReader::GetPictGrafFromStream( rGraph, *pStp );
}


using namespace ::com::sun::star;


SwFrmFmt* SwWW8ImplReader::ImportOle( const Graphic* pGrf,
                                     const SfxItemSet* pFlySet )
{
    SwFrmFmt* pFmt = 0;
    if( !(nIniFlags & WW8FL_NO_OLE ))
    {
        ASSERT( pStg, "ohne storage geht hier fast gar nichts!" );

        ::SetProgressState( nProgress, rDoc.GetDocShell() );     // Update

        INT16 nX, nY;               // nX, nY ist Ziel-Groesse
        BOOL bOleOk = TRUE;
        Graphic aGraph;

        String aSrcStgName = '_';
        aSrcStgName += String::CreateFromInt32( nObjLocFc );        // ergibt Name "_4711"

        SvStorageRef xSrc0 = pStg->OpenStorage( WW8_ASCII2STR( "ObjectPool" ) );

        if( pGrf )
        {
            aGraph = *pGrf;
            const Size aSizeTwip = OutputDevice::LogicToLogic(
                                        aGraph.GetPrefSize(),
                                        aGraph.GetPrefMapMode(),
                                        MAP_TWIP );
            nX = (INT16) aSizeTwip.Width();
            nY = (INT16) aSizeTwip.Height();
        }
        else
        {
            SvStorageRef xSrc1 = xSrc0->OpenStorage( aSrcStgName,
                                STREAM_READWRITE| STREAM_SHARE_DENYALL );
            OLE_MFP aMfp;
            GDIMetaFile aWMF;
            if( SwWw6ReadMetaStream( aWMF, &aMfp, xSrc1 ) )
            {
                /* ignore size, take Size in SwWw8ReadScaling instead!
                INT16 nXO = (INT16)( (long)aMfp.xExt * 144 / 254 );
                INT16 nYO = (INT16)( (long)aMfp.yExt * 144 / 254 );
                */
                SwWw8ReadScaling( nX, nY, xSrc1 );
                aGraph = Graphic( aWMF );
            }
            // 03-META-Stream nicht da. Vielleicht ein 03-PICT ?
            else if( SwWw6ReadMacPICTStream( aGraph, xSrc1 ) )
            {
                const Size aSizeTwip = OutputDevice::LogicToLogic(
                                            aGraph.GetPrefSize(),
                                            aGraph.GetPrefMapMode(),
                                            MAP_TWIP );
                nX = (INT16) aSizeTwip.Width();
                nY = (INT16) aSizeTwip.Height();
                bOleOk = FALSE;         // PICT: kein WMF da -> Grafik statt OLE
            }
#if 0
//JP 23.06.99 - if the OLE-Storage does not contained the streams META
//              or PICT, then import the grafic from the escher.
//              But the Question is, is the OLE-Storage then a valid
//              OLE-Object and is the user be able to activate it. So it's
//              better to import it as grafic.

            else
            {
                ULONG nOldObjLocFc = nOldObjLocFc;
                nObjLocFc = 0;

                SwFrmFmt* pGrfFmt = ImportGraf();
                if( pGrfFmt )
                {
                    const SwNodeIndex* pCntIdx = pGrfFmt->GetCntnt( FALSE ).
                                            GetCntntIdx();
                    if( pCntIdx )
                    {
                        SwNodeIndex aIdx( *pCntIdx, 1 );
                        if( aIdx.GetNode().IsGrfNode() )
                        {
                            // get the grafic of the node and delete it
                            // then complete, because its become a OLE-Object
                            aGraph = aIdx.GetNode().GetGrfNode()->GetGrf();
                            Size aSz( aIdx.GetNode().GetGrfNode()->GetTwipSize() );
                            nX = (INT16) aSz.Width();
                            nY = (INT16) aSz.Height();

                            rDoc.DelLayoutFmt( pGrfFmt );
                            pGrfFmt = 0;
                        }
                    }
                    pFmt = pGrfFmt;
                }
                nObjLocFc = nOldObjLocFc;
            }
#endif
        }       // StorageStreams wieder zu

//#ifdef DEBUG
        SvStorageRef xSrc1 = xSrc0->OpenStorage( aSrcStgName,
            STREAM_READWRITE| STREAM_SHARE_DENYALL );

        if(!pFormImpl)
            pFormImpl = new SwMSConvertControls(rDoc.GetDocShell(),pPaM);
        uno::Reference< drawing::XShape > xRef;
        if (pFormImpl->ReadOCXStream(xSrc1,&xRef,bFloatingCtrl))
        {
            bOleOk = FALSE;
            uno::Reference< beans::XPropertySet >  xPropSet( xRef, uno::UNO_QUERY );
            uno::Reference< lang::XUnoTunnel> xTunnel( xPropSet, uno::UNO_QUERY);
            SwXShape *pSwShape = 0;
            if(xTunnel.is())
            {
                pSwShape = (SwXShape*)xTunnel->getSomething(SwXShape::getUnoTunnelId());
            }
            return pSwShape ? (SwFrmFmt*)(pSwShape->GetRegisteredIn()) : 0;
        }
//#endif


        if( GRAPHIC_GDIMETAFILE == aGraph.GetType() ||
            GRAPHIC_BITMAP == aGraph.GetType() )
        {
            ::SetProgressState( nProgress, rDoc.GetDocShell() );     // Update

            Point aTmpPoint;
            Rectangle aRect( aTmpPoint, Size( nX, nY ) );
            SwAttrSet aFlySet( rDoc.GetAttrPool(), RES_FRMATR_BEGIN,
                                                    RES_FRMATR_END-1);
            if( !pFlySet )
            {
                pFlySet = &aFlySet;

                if( !bNew )
                    Reader::ResetFrmFmtAttrs( aFlySet );    // Abstand/Umrandung raus

                SwFmtAnchor aAnchor( FLY_IN_CNTNT );
                aAnchor.SetAnchor( pPaM->GetPoint() );
                aFlySet.Put( aAnchor );

                aFlySet.Put( SwFmtFrmSize( ATT_FIX_SIZE, nX, nY ) );
                aFlySet.Put( SwFmtVertOrient( 0, VERT_TOP, FRAME ));

                if( pSFlyPara )         // OLE im Rahmen ?
                {                       // ok, Rahmen auf Bildgroesse vergroessern
                                        //  ( nur wenn Auto-Breite )
                    pSFlyPara->BoxUpWidth( nX );
                }
            }
            else
            {
                const SwFmtFrmSize* pSize = (SwFmtFrmSize*)pFlySet->GetItem(
                                            RES_FRM_SIZE, FALSE );
                if( pSize )
                    aRect.SetSize( pSize->GetSize() );
            }


            if( bOleOk && !( nIniFlags & WW8FL_OLE_TO_GRAF ))
            {
                ULONG nOldPos = pDataStream->Tell();

                pDataStream->Seek(STREAM_SEEK_TO_END);
                SvStream *pTmpData;
                if (nObjLocFc < pDataStream->Tell())
                {
                    pTmpData = pDataStream;
                    pTmpData->Seek( nObjLocFc );
                }
                else
                    pTmpData = 0;
                SvStorageRef xDst0( rDoc.GetDocShell()->GetStorage() );
                SdrOle2Obj* pRet = SvxMSDffManager::CreateSdrOLEFromStorage(
                                        aSrcStgName, xSrc0, xDst0,
                                        aGraph, aRect, pTmpData,
                                        SwMSDffManager::GetFilterFlags());
                if( pRet )      // Ole-Object wurde eingefuegt
                {
                    pFmt = rDoc.Insert( *pPaM, &pRet->GetObjRef(), pFlySet );
                    delete pRet;        // das brauchen wir nicht mehr
                }
                else
                    bOleOk = FALSE;
                pDataStream->Seek( nOldPos );
            }

            if( !bOleOk || (nIniFlags & WW8FL_OLE_TO_GRAF) )
                pFmt = rDoc.Insert( *pPaM,
                            aEmptyStr, aEmptyStr,   // Name der Grafik !!
                            &aGraph,
                            pFlySet,                // Attribute fuer den FlyFrm
                            0 );                    // Attribute fuer die Grafik
        }
    }
    return pFmt;
}


void SwWW8ImplReader::ReadRevMarkAuthorStrTabl( SvStream& rStrm,
                                                INT32 nTblPos,
                                                INT32 nTblSiz,
                                                SwDoc& rDoc )
{
    SvStrings aAuthorNames( 0, 16 );
    WW8ReadSTTBF( !bVer67, rStrm, nTblPos, nTblSiz, bVer67 ? 2 : 0,
                    eStructCharSet, aAuthorNames );

    for( USHORT nAuthor = 0; nAuthor < aAuthorNames.Count(); ++nAuthor )
    {
        // Store author in doc
        USHORT nSWId = rDoc.InsertRedlineAuthor( *aAuthorNames[ nAuthor ] );
        // Store matchpair
        if( !pAuthorInfos )
            pAuthorInfos = new WW8AuthorInfos;
        WW8AuthorInfo* pAutorInfo = new WW8AuthorInfo( nAuthor, nSWId );
        if( 0 == pAuthorInfos->Insert( pAutorInfo ) )
            delete pAutorInfo;
    }
    aAuthorNames.DeleteAndDestroy( 0, aAuthorNames.Count() );
}

/*
   Revision Marks ( == Redlining )
*/
// insert or delete content (change char attributes resp.)
void SwWW8ImplReader::Read_CRevisionMark(SwRedlineType eType, USHORT nId, BYTE* pData, short nLen )
{
    // there *must* be a SprmCIbstRMark[Del] and a SprmCDttmRMark[Del]
    // pointing to the very same char position as our SprmCFRMark[Del]
    if( !pPlcxMan ) return;
    const BYTE* pSprmCIbstRMark;
    const BYTE* pSprmCDttmRMark;
//  const BYTE* pSprmCIdslRMark;
    if( REDLINE_FORMAT == eType )
    {
        pSprmCIbstRMark = pData+1;
        pSprmCDttmRMark = pData+3;
    }
    else
    {
        BOOL bIns = (REDLINE_INSERT == eType);
        if( bVer67 )
        {
            pSprmCIbstRMark = pPlcxMan->HasCharSprm( 69 );
            pSprmCDttmRMark = pPlcxMan->HasCharSprm( 70 );
        }
        else
        {
            pSprmCIbstRMark = pPlcxMan->HasCharSprm( bIns ? 0x4804 : 0x4863 );
            pSprmCDttmRMark = pPlcxMan->HasCharSprm( bIns ? 0x6805 : 0x6864 );
//          pSprmCIdslRMark = pPlcxMan->HasCharSprm( bIns ? 0x4807 : 0x4867 );
        }
    }
    if( !pSprmCIbstRMark || !pSprmCDttmRMark ) return;

    if( nLen < 0 )
    {
        // end of current revision mark
        pCtrlStck->SetAttr( *pPaM->GetPoint(), RES_FLTR_REDLINE );
    }
    else
    {
        // start of new revision mark
        USHORT nWWAutNo = SVBT16ToShort( pSprmCIbstRMark );
        UINT32 nWWDate  = SVBT32ToLong(  pSprmCDttmRMark );
        WW8AuthorInfo aEntry( nWWAutNo );
        USHORT nPos;
        if( pAuthorInfos->Seek_Entry( &aEntry, &nPos ) )
        {
            const WW8AuthorInfo* pAuthor = pAuthorInfos->GetObject( nPos );
            if( pAuthor )
            {
                USHORT        nAutorNo = pAuthor->nOurId;
                DateTime      aStamp(WW8ScannerBase::WW8DTTM2DateTime(nWWDate));

                SwFltRedline  aNewAttr(eType, nAutorNo, aStamp);

                const SwFltRedline* pOldAttr =
                    (const SwFltRedline*)pCtrlStck->GetOpenStackAttr(
                                                        *pPaM->GetPoint(),
                                                        RES_FLTR_REDLINE );
                // 1st look if there is already another redline Attribute
                // set on this text-region
                // If so, we take it's data and store it as 'previous'
                //                              or the other way around
                if( pOldAttr )
                {           // Insert on top of Delete ?  This is not allowed !
                    BOOL bDateWrongWayAround
                            =    (REDLINE_INSERT == eType)
                              && (REDLINE_DELETE == pOldAttr->eType);
                    if(    bDateWrongWayAround
                        || (      (aStamp < pOldAttr->aStamp)
                             && ! (    (REDLINE_INSERT == pOldAttr->eType)
                                    && (REDLINE_DELETE == eType)
                                  )
                           )
                      )
                    {
                        if(     bDateWrongWayAround
                            && !(nAutorNo == pOldAttr->nAutorNo) )
                        {
                            aNewAttr.eTypePrev    = eType;
                            aNewAttr.nAutorNoPrev = nAutorNo;
                            aNewAttr.aStampPrev   = aStamp;
                            aNewAttr.eType        = pOldAttr->eType;
                            aNewAttr.nAutorNo     = pOldAttr->nAutorNo;
                            aNewAttr.aStamp       = pOldAttr->aStamp;
                        }
                        // else do nothing: so only the INSERT will be stored!
                    }
                    else
                    {
                        aNewAttr.eTypePrev    = pOldAttr->eType;
                        aNewAttr.nAutorNoPrev = pOldAttr->nAutorNo;
                        aNewAttr.aStampPrev   = pOldAttr->aStamp;
                    }
                }
                NewAttr( aNewAttr );
            }
        }
    }
}
// insert new content
void SwWW8ImplReader::Read_CFRMark(   USHORT nId, BYTE* pData, short nLen )
{
    Read_CRevisionMark( REDLINE_INSERT, nId, pData, nLen );
}
// delete old content
void SwWW8ImplReader::Read_CFRMarkDel(   USHORT nId, BYTE* pData, short nLen )
{
    Read_CRevisionMark( REDLINE_DELETE, nId, pData, nLen );
}
// change properties of content ( == char formating)
void SwWW8ImplReader::Read_CPropRMark( USHORT nId, BYTE* pData, short nLen )
{
    // complex (len is always 7)
    // 1 byte  - chp.fPropRMark
    // 2 bytes - chp.ibstPropRMark
    // 4 bytes - chp.dttmPropRMark;
    Read_CRevisionMark( REDLINE_FORMAT, nId, pData, nLen );
}


/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/ww8par4.cxx,v 1.3 2000-11-01 12:12:16 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/10/10 16:54:06  cmc
      MSOffice 97/2000 Controls {Im|Ex}port

      Revision 1.1.1.1  2000/09/18 17:14:58  hr
      initial import

      Revision 1.31  2000/09/18 16:05:01  willem.vandorp
      OpenOffice header added.

      Revision 1.30  2000/08/22 17:09:02  cmc
      #77743# OLE Import, bad seek & bad FilterOptions

      Revision 1.29  2000/06/29 20:59:11  jp
      new MS Filteroptions - change OLE-Objects to StarOffice Objects

      Revision 1.28  2000/06/13 08:24:46  os
      using UCB

      Revision 1.27  2000/05/18 11:01:01  jp
      Changes for Unicode

      Revision 1.26  2000/05/16 11:06:24  khz
      Unicode code-conversion

      Revision 1.25  2000/05/05 14:06:55  cmc
      #75358# WW8 97Controls Import Fix

      Revision 1.24  2000/04/03 15:29:02  cmc
      #74329# Floating Form Controls

      Revision 1.23  2000/03/27 11:45:31  cmc
      #74329# Added OCX Reader to ImportOle

      Revision 1.22  2000/03/22 14:35:32  khz
      Task #74390# release pAutorInfo in case Insert() was not sucessful

      Revision 1.21  2000/02/28 12:04:32  khz
      Task #73544#: Use size and cropping from \3PIC stream rather than \3META

      Revision 1.20  2000/02/21 13:08:27  jp
      #70473# changes for unicode

      Revision 1.19  2000/02/11 14:40:23  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.18  1999/10/21 15:36:41  khz
      Import Redlining (4)

      Revision 1.17  1999/10/13 21:04:34  khz
      Import Redlining (3)

      Revision 1.16  1999/10/13 13:27:45  khz
      Import Redlining (2)

      Revision 1.15  1999/10/08 09:26:19  khz
      Import Redlining

      Revision 1.14  1999/09/08 13:26:09  khz
      Better performance by reducing use of SWAP..() and SVBT..To..() methods

      Revision 1.13  1999/08/30 19:53:06  JP
      Bug #68219#: no static members - be reentrant


      Rev 1.12   30 Aug 1999 21:53:06   JP
   Bug #68219#: no static members - be reentrant

      Rev 1.11   09 Aug 1999 14:16:10   JP
   read W95 format from stream

      Rev 1.10   30 Jul 1999 13:59:28   JP
   Bug #60140#: CreateSdrOLEFromStorage - read OLE1-Objects

      Rev 1.9   23 Jun 1999 12:46:58   JP
   delete unused variables

      Rev 1.8   26 Apr 1999 19:22:54   JP
   Bug #65298#: Grf/Ole bei Zeichenbindung immer Oben zu Grundline anordnen

      Rev 1.7   12 Jan 1999 12:29:54   MH
   chg: Syntax GCC

      Rev 1.6   30 Nov 1998 18:18:20   JP
   Task #59822#: OLE-Objecte importieren

      Rev 1.5   30 Nov 1998 17:46:54   JP
   Task #59822#: OLE-Objecte importieren

      Rev 1.4   27 Nov 1998 09:45:22   JP
   Task #59822#: OLE-Objecte importieren - Namen korrekt ermitteln

      Rev 1.3   25 Nov 1998 11:29:14   JP
   Task #59822#: OLE-Objecte einlesen - SetVisArea soll kein modified setzen

      Rev 1.2   24 Nov 1998 20:47:50   JP
   Task #59822#: OLE-Objecte einlesen

      Rev 1.1   18 Sep 1998 13:10:10   JP
   Bug #56310#: ImportOLE - die Zeichenbindung nicht doppelt machen

      Rev 1.0   16 Jun 1998 11:08:16   KHZ
   Initial revision.

*************************************************************************/




