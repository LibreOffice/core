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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <doc.hxx>
#include "writerhelper.hxx"
#include <com/sun/star/embed/XClassifiedObject.hpp>

#include <algorithm>
#include <functional>
#include <osl/endian.h>
#include <sot/storage.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <hintids.hxx>
#include <svx/svdoole2.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <svx/unoapi.hxx>
#include <filter/msfilter/msocximex.hxx>

#include <sot/exchange.hxx>
#include <swtypes.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <dcontact.hxx>
#include <frmfmt.hxx>
#include <pam.hxx>
#include <ndgrf.hxx>
#include <docsh.hxx>            // fuer Ole-Node
#include <mdiexp.hxx>           // Progress
#include <redline.hxx>
#include <fltshell.hxx>
#include <unodraw.hxx>
#include <shellio.hxx>
#include <ndole.hxx>

#include <svtools/filter.hxx>

#include "ww8scan.hxx"
#include "ww8par.hxx"
#include "ww8par2.hxx"          // WWFlyPara::BoxUpWidth()

struct OLE_MFP
{
    sal_Int16 mm;       // 0x6  int
    sal_Int16 xExt;     // 0x8  int in 1/100 mm
    sal_Int16 yExt;     // 0xa  int in 1/100 mm
    sal_Int16 hMF;      // 0xc  int
};

using namespace ::com::sun::star;

SV_IMPL_OP_PTRARR_SORT(WW8OleMaps, WW8OleMap_Ptr)

static bool SwWw8ReadScaling(long& rX, long& rY, SvStorageRef& rSrc1)
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

    SvStorageStreamRef xSrc3 = rSrc1->OpenSotStream( CREATE_CONST_ASC( "\3PIC" ),
        STREAM_STD_READ | STREAM_NOCREATE);
    SvStorageStream* pS = xSrc3;
    pS->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    pS->Seek( STREAM_SEEK_TO_END );

    OSL_ENSURE( pS->Tell() >=  76, "+OLE-PIC-Stream is shorter than 76 Byte" );

    sal_Int32 nOrgWidth,
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
    if (10 > nScaleX || 65536 < nScaleX || 10 > nScaleY || 65536 < nScaleY)
    {
        OSL_ENSURE( !pS, "+OLE-Scalinginformation in PIC-Stream wrong" );
        return false;
    }
    else
    {
        rX = (rX * nScaleX) / 1000;
        rY = (rY * nScaleY) / 1000;
    }
    return true;
}

static bool SwWw6ReadMetaStream(GDIMetaFile& rWMF, OLE_MFP* pMfp,
    SvStorageRef& rSrc1)
{
    SvStorageStreamRef xSrc2 = rSrc1->OpenSotStream( CREATE_CONST_ASC("\3META"),
        STREAM_STD_READ | STREAM_NOCREATE);
    SvStorageStream* pSt = xSrc2;
    pSt->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    sal_uLong nRead = pSt->Read( pMfp, sizeof(*pMfp ) );
                                // Mini-Placable-Header lesen
    if (nRead != sizeof(*pMfp))
        return false;

#if defined  OSL_BIGENDIAN
    pMfp->mm = SWAPSHORT( pMfp->mm );
    pMfp->xExt = SWAPSHORT( pMfp->xExt );
    pMfp->yExt = SWAPSHORT( pMfp->yExt );
#endif // OSL_BIGENDIAN

    if( pMfp->mm == 94 || pMfp->mm == 99 )
    {
        OSL_ENSURE( !pSt, "+OLE: Falscher Metafile-Typ" );
        return false;
    }
    if( pMfp->mm != 8 )
    {
        OSL_ENSURE( !pSt, "+OLE: Falscher Metafile-Typ ( nicht Anisotropic )" );
    }
    if( !pMfp->xExt || !pMfp->yExt )
    {
        OSL_ENSURE( !pSt, "+OLE: Groesse von 0 ???" );
        return false;
    }
    bool bOk = ReadWindowMetafile( *pSt, rWMF, NULL ) ? true : false;   // WMF lesen
                    // *pSt >> aWMF  geht nicht ohne placable Header
    if (!bOk || pSt->GetError() || rWMF.GetActionCount() == 0)
    {
        OSL_ENSURE( !pSt, "+OLE: Konnte Metafile nicht lesen" );
        return false;
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

    return true;
}

static bool SwWw6ReadMacPICTStream(Graphic& rGraph, SvStorageRef& rSrc1)
{
    // 03-META-Stream nicht da. Vielleicht ein 03-PICT ?
    SvStorageStreamRef xSrc4 = rSrc1->OpenSotStream( CREATE_CONST_ASC( "\3PICT" ));
    SvStorageStream* pStp = xSrc4;
    pStp->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    sal_uInt8 aTestA[10];        // Ist der 01Ole-Stream ueberhaupt vorhanden
    sal_uLong nReadTst = pStp->Read( aTestA, sizeof( aTestA ) );
    if (nReadTst != sizeof(aTestA))
        return false;

    pStp->Seek( STREAM_SEEK_TO_BEGIN );

    // Mac-Pict steht im 03PICT-StorageStream allerdings ohne die ersten 512
    // Bytes, die bei einem MAC-PICT egal sind ( werden nicht ausgewertet )
    return SwWW8ImplReader::GetPictGrafFromStream(rGraph, *pStp);
}

SwFlyFrmFmt* SwWW8ImplReader::InsertOle(SdrOle2Obj &rObject,
    const SfxItemSet &rFlySet, const SfxItemSet &rGrfSet)
{
    SfxObjectShell *pPersist = rDoc.GetPersist();
    OSL_ENSURE(pPersist, "No persist, cannot insert objects correctly");
    if (!pPersist)
        return 0;

    SwFlyFrmFmt *pRet = 0;

    SfxItemSet *pMathFlySet = 0;
    uno::Reference < embed::XClassifiedObject > xClass( rObject.GetObjRef(), uno::UNO_QUERY );
    if( xClass.is() )
    {
        SvGlobalName aClassName( xClass->getClassID() );
        if (SotExchange::IsMath(aClassName))
        {
            /*
            StarMath sets it own fixed size, so its counter productive to use the
            size word says it is. i.e. Don't attempt to override its size.
            */
            pMathFlySet = new SfxItemSet(rFlySet);
            pMathFlySet->ClearItem(RES_FRM_SIZE);
        }
    }

    /*
    Take complete responsibility of the object away from SdrOle2Obj and to
    me here locally. This utility class now owns the object.
    */

    // TODO/MBA: is the object inserted multiple times here? Testing!
    // And is it a problem that we now use the same naming scheme as in the other apps?
    sw::hack::DrawingOLEAdaptor aOLEObj(rObject, *pPersist);
    ::rtl::OUString sNewName;
    bool bSuccess = aOLEObj.TransferToDoc(sNewName);

    OSL_ENSURE(bSuccess, "Insert OLE failed");
    if (bSuccess)
    {
        const SfxItemSet *pFlySet = pMathFlySet ? pMathFlySet : &rFlySet;
        pRet = rDoc.InsertOLE(*pPaM, sNewName, rObject.GetAspect(), pFlySet, &rGrfSet, 0);
    }
    delete pMathFlySet;
    return pRet;
}

SwFrmFmt* SwWW8ImplReader::ImportOle(const Graphic* pGrf,
    const SfxItemSet* pFlySet, const SfxItemSet *pGrfSet, const Rectangle& aVisArea )
{
    ::SetProgressState(nProgress, mpDocShell);     // Update
    SwFrmFmt* pFmt = 0;

    GrafikCtor();

    Graphic aGraph;
    SdrObject* pRet = ImportOleBase(aGraph, pGrf, pFlySet, aVisArea );

    // create flyset
    SfxItemSet* pTempSet = 0;
    if( !pFlySet )
    {
        pTempSet = new SfxItemSet( rDoc.GetAttrPool(), RES_FRMATR_BEGIN,
            RES_FRMATR_END-1);

        pFlySet = pTempSet;

        // Abstand/Umrandung raus
        if (!mbNewDoc)
            Reader::ResetFrmFmtAttrs( *pTempSet );

        SwFmtAnchor aAnchor( FLY_AS_CHAR );
        aAnchor.SetAnchor( pPaM->GetPoint() );
        pTempSet->Put( aAnchor );

        const Size aSizeTwip = OutputDevice::LogicToLogic(
            aGraph.GetPrefSize(), aGraph.GetPrefMapMode(), MAP_TWIP );

        pTempSet->Put( SwFmtFrmSize( ATT_FIX_SIZE, aSizeTwip.Width(),
            aSizeTwip.Height() ) );
        pTempSet->Put( SwFmtVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ));

        if( pSFlyPara )
        {
            // OLE im Rahmen ?  ok, Rahmen auf Bildgroesse vergroessern (
            // nur wenn Auto-Breite )
            pSFlyPara->BoxUpWidth( aSizeTwip.Width() );
        }
    }

    if (pRet)       // Ole-Object wurde eingefuegt
    {
        if (pRet->ISA(SdrOle2Obj))
        {
            pFmt = InsertOle(*((SdrOle2Obj*)pRet), *pFlySet, *pGrfSet);
            SdrObject::Free( pRet );        // das brauchen wir nicht mehr
        }
        else
            pFmt = rDoc.Insert(*pPaM, *pRet, pFlySet, NULL);
    }
    else if (
                GRAPHIC_GDIMETAFILE == aGraph.GetType() ||
                GRAPHIC_BITMAP == aGraph.GetType()
            )
    {
        pFmt = rDoc.Insert(*pPaM, aEmptyStr, aEmptyStr, &aGraph, pFlySet,
            pGrfSet, NULL);
    }
    delete pTempSet;
    return pFmt;
}

bool SwWW8ImplReader::ImportOleWMF(SvStorageRef xSrc1,GDIMetaFile &rWMF,
    long &rX,long &rY)
{
    bool bOk = false;
    OLE_MFP aMfp;
    if( SwWw6ReadMetaStream( rWMF, &aMfp, xSrc1 ) )
    {
        /*
        take scaling factor as found in PIC and apply it to graphic.
        */
        SwWw8ReadScaling( rX, rY, xSrc1 );
        Size aFinalSize, aOrigSize;
        aFinalSize.Width() = rX;
        aFinalSize.Height() = rY;
        aFinalSize = OutputDevice::LogicToLogic(
            aFinalSize, MAP_TWIP, rWMF.GetPrefMapMode() );
        aOrigSize = rWMF.GetPrefSize();
        Fraction aScaleX(aFinalSize.Width(),aOrigSize.Width());
        Fraction aScaleY(aFinalSize.Height(),aOrigSize.Height());
        rWMF.Scale( aScaleX, aScaleY );
        bOk = true;
    }
    return bOk;
}

SdrObject* SwWW8ImplReader::ImportOleBase( Graphic& rGraph,
    const Graphic* pGrf, const SfxItemSet* pFlySet, const Rectangle& aVisArea )
{
    SdrObject* pRet = 0;
    OSL_ENSURE( pStg, "ohne storage geht hier fast gar nichts!" );

    ::SetProgressState( nProgress, rDoc.GetDocShell() );     // Update

    long nX=0, nY=0;                // nX, nY is graphic size
    bool bOleOk = true;

    String aSrcStgName = '_';
    // ergibt Name "_4711"
    aSrcStgName += String::CreateFromInt32( nObjLocFc );

    SvStorageRef xSrc0 = pStg->OpenSotStorage(CREATE_CONST_ASC(SL::aObjectPool));
    SvStorageRef xSrc1 = xSrc0->OpenSotStorage( aSrcStgName,
            STREAM_READWRITE| STREAM_SHARE_DENYALL );


    if (pGrf)
    {
        rGraph = *pGrf;
        const Size aSizeTwip = OutputDevice::LogicToLogic(
            rGraph.GetPrefSize(), rGraph.GetPrefMapMode(), MAP_TWIP );
        nX = aSizeTwip.Width();
        nY = aSizeTwip.Height();
    }
    else
    {
        GDIMetaFile aWMF;

        if (ImportOleWMF(xSrc1,aWMF,nX,nY))
            rGraph = Graphic( aWMF );
        else if( SwWw6ReadMacPICTStream( rGraph, xSrc1 ) )
        {
            // 03-META-Stream nicht da. Vielleicht ein 03-PICT ?
            const Size aSizeTwip = OutputDevice::LogicToLogic(
                rGraph.GetPrefSize(), rGraph.GetPrefMapMode(), MAP_TWIP );
            nX = aSizeTwip.Width();
            nY = aSizeTwip.Height();
            // PICT: kein WMF da -> Grafik statt OLE
            bOleOk = false;
        }
    }       // StorageStreams wieder zu


    Rectangle aRect(0, 0, nX, nY);

    if (pFlySet)
    {
        if (const SwFmtFrmSize* pSize =
            (const SwFmtFrmSize*)pFlySet->GetItem(RES_FRM_SIZE, false))
        {
            aRect.SetSize(pSize->GetSize());
        }
    }

    if (!(bIsHeader || bIsFooter))
    {
        //Can't put them in headers/footers :-(
        uno::Reference< drawing::XShape > xRef;
        OSL_ENSURE(pFormImpl, "Impossible");
        if (pFormImpl && pFormImpl->ReadOCXStream(xSrc1, &xRef, false))
        {
            pRet = GetSdrObjectFromXShape(xRef);
            OSL_ENSURE(pRet, "Impossible");
            if (pRet)
                pRet->SetLogicRect(aRect);
            return pRet;
        }
    }

    if (GRAPHIC_GDIMETAFILE == rGraph.GetType() ||
        GRAPHIC_BITMAP == rGraph.GetType())
    {
        ::SetProgressState(nProgress, mpDocShell);     // Update

        if (bOleOk)
        {
            sal_uLong nOldPos = pDataStream->Tell();
            pDataStream->Seek(STREAM_SEEK_TO_END);
            SvStream *pTmpData = 0;
            if (nObjLocFc < pDataStream->Tell())
            {
                pTmpData = pDataStream;
                pTmpData->Seek( nObjLocFc );
            }

            sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;

            {
                SvStorageStreamRef xObjInfoSrc = xSrc1->OpenSotStream( CREATE_CONST_ASC( "\3ObjInfo" ),
                    STREAM_STD_READ | STREAM_NOCREATE );
                if ( xObjInfoSrc.Is() && !xObjInfoSrc->GetError() )
                {
                    sal_uInt8 nByte = 0;
                    *xObjInfoSrc >> nByte;
                    if ( ( nByte >> 4 ) & embed::Aspects::MSOLE_ICON )
                        nAspect = embed::Aspects::MSOLE_ICON;
                }
            }

            ErrCode nError = ERRCODE_NONE;
            pRet = SvxMSDffManager::CreateSdrOLEFromStorage(
                aSrcStgName, xSrc0, mpDocShell->GetStorage(), rGraph, aRect, aVisArea, pTmpData, nError,
                SwMSDffManager::GetFilterFlags(), nAspect );
            pDataStream->Seek( nOldPos );
        }
    }
    return pRet;
}

void SwWW8ImplReader::ReadRevMarkAuthorStrTabl( SvStream& rStrm,
    sal_Int32 nTblPos, sal_Int32 nTblSiz, SwDoc& rDocOut )
{
    ::std::vector<String> aAuthorNames;
    WW8ReadSTTBF( !bVer67, rStrm, nTblPos, nTblSiz, bVer67 ? 2 : 0,
        eStructCharSet, aAuthorNames );

    sal_uInt16 nCount = static_cast< sal_uInt16 >(aAuthorNames.size());
    for( sal_uInt16 nAuthor = 0; nAuthor < nCount; ++nAuthor )
    {
        // Store author in doc
        sal_uInt16 nSWId = rDocOut.InsertRedlineAuthor(aAuthorNames[nAuthor]);
        // Store matchpair
        if( !pAuthorInfos )
            pAuthorInfos = new sw::util::AuthorInfos;
        sw::util::AuthorInfo* pAutorInfo = new sw::util::AuthorInfo( nAuthor, nSWId );
        if( 0 == pAuthorInfos->Insert( pAutorInfo ) )
            delete pAutorInfo;
    }
}

/*
   Revision Marks ( == Redlining )
*/
// insert or delete content (change char attributes resp.)
void SwWW8ImplReader::Read_CRevisionMark(RedlineType_t eType,
    const sal_uInt8* pData, short nLen )
{
    // there *must* be a SprmCIbstRMark[Del] and a SprmCDttmRMark[Del]
    // pointing to the very same char position as our SprmCFRMark[Del]
    if (!pPlcxMan)
        return;
    const sal_uInt8* pSprmCIbstRMark;
    const sal_uInt8* pSprmCDttmRMark;
    if( nsRedlineType_t::REDLINE_FORMAT == eType )
    {
        pSprmCIbstRMark = pData+1;
        pSprmCDttmRMark = pData+3;
    }
    else
    {
        /*
         It is possible to have a number of date stamps for the created time
         of the change, (possibly a word bug) so we must use the "get a full
         list" varient of HasCharSprm and take the last one as the true one.
        */
        std::vector<const sal_uInt8 *> aResult;
        bool bIns = (nsRedlineType_t::REDLINE_INSERT == eType);
        if( bVer67 )
        {
            pPlcxMan->HasCharSprm(69, aResult);
            pSprmCIbstRMark = aResult.empty() ? 0 : aResult.back();
            aResult.clear();
            pPlcxMan->HasCharSprm(70, aResult);
            pSprmCDttmRMark = aResult.empty() ? 0 : aResult.back();
        }
        else
        {
            pPlcxMan->HasCharSprm( bIns ? 0x4804 : 0x4863, aResult);
            pSprmCIbstRMark = aResult.empty() ? 0 : aResult.back();
            aResult.clear();
            pPlcxMan->HasCharSprm( bIns ? 0x6805 : 0x6864, aResult);
            pSprmCDttmRMark = aResult.empty() ? 0 : aResult.back();
        }
    }

    if (nLen < 0)
        mpRedlineStack->close(*pPaM->GetPoint(), eType, pTableDesc );
    else
    {
        // start of new revision mark, if not there default to first entry
        sal_uInt16 nWWAutNo = pSprmCIbstRMark ? SVBT16ToShort( pSprmCIbstRMark ) : 0;
        sw::util::AuthorInfo aEntry(nWWAutNo);
        sal_uInt16 nPos;
        if (pAuthorInfos && pAuthorInfos->Seek_Entry(&aEntry, &nPos))
        {
            if (const sw::util::AuthorInfo* pAuthor = pAuthorInfos->GetObject(nPos))
            {
                sal_uInt32 nWWDate = pSprmCDttmRMark ? SVBT32ToUInt32(pSprmCDttmRMark): 0;
                DateTime aStamp(sw::ms::DTTM2DateTime(nWWDate));
                sal_uInt16 nAutorNo = pAuthor->nOurId;
                SwFltRedline  aNewAttr(eType, nAutorNo, aStamp);

                NewAttr(aNewAttr);
            }
        }
    }
}

// insert new content
void SwWW8ImplReader::Read_CFRMark(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    Read_CRevisionMark( nsRedlineType_t::REDLINE_INSERT, pData, nLen );
}

// delete old content
void SwWW8ImplReader::Read_CFRMarkDel(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    Read_CRevisionMark( nsRedlineType_t::REDLINE_DELETE, pData, nLen );
}

// change properties of content ( == char formating)
void SwWW8ImplReader::Read_CPropRMark(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    // complex (len is always 7)
    // 1 byte  - chp.fPropRMark
    // 2 bytes - chp.ibstPropRMark
    // 4 bytes - chp.dttmPropRMark;
    Read_CRevisionMark( nsRedlineType_t::REDLINE_FORMAT, pData, nLen );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
