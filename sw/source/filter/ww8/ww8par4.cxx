/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <doc.hxx>
#include "writerhelper.hxx"
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>

#include <algorithm>
#include <functional>
#include <osl/endian.h>
#include <sot/storage.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <hintids.hxx>
#include <svx/svdoole2.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include <filter/msfilter/sprmids.hxx>
#include <svx/unoapi.hxx>

#include <sot/exchange.hxx>
#include <swtypes.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <dcontact.hxx>
#include <frmfmt.hxx>
#include <pam.hxx>
#include <ndgrf.hxx>
#include <docsh.hxx>
#include <mdiexp.hxx>
#include <redline.hxx>
#include <fltshell.hxx>
#include <unodraw.hxx>
#include <shellio.hxx>
#include <ndole.hxx>

#include <vcl/graphicfilter.hxx>
#include <vcl/wmf.hxx>

#include "ww8scan.hxx"
#include "ww8par.hxx"
#include "ww8par2.hxx"

struct OLE_MFP
{
    sal_Int16 mm;       
    sal_Int16 xExt;     
    sal_Int16 yExt;     
    sal_Int16 hMF;      
};

using namespace ::com::sun::star;

static bool SwWw8ReadScaling(long& rX, long& rY, SvStorageRef& rSrc1)
{
    
    
    
    
    
    
    
    
    
    
    
    

    SvStorageStreamRef xSrc3 = rSrc1->OpenSotStream( OUString("\3PIC"),
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
    pS->ReadInt32( nOrgWidth )    
       .ReadInt32( nOrgHeight );
    pS->Seek( 0x2c );
    pS->ReadInt32( nScaleX )      
       .ReadInt32( nScaleY )
       .ReadInt32( nCropLeft )    
       .ReadInt32( nCropTop )
       .ReadInt32( nCropRight )
       .ReadInt32( nCropBottom );

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
    SvStorageStreamRef xSrc2 = rSrc1->OpenSotStream( OUString("\3META"),
        STREAM_STD_READ | STREAM_NOCREATE);
    SvStorageStream* pSt = xSrc2;
    pSt->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    sal_uLong nRead = pSt->Read( pMfp, sizeof(*pMfp ) );
                                
    if (nRead != sizeof(*pMfp))
        return false;

#if defined  OSL_BIGENDIAN
    pMfp->mm = OSL_SWAPWORD( pMfp->mm );
    pMfp->xExt = OSL_SWAPWORD( pMfp->xExt );
    pMfp->yExt = OSL_SWAPWORD( pMfp->yExt );
#endif 

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
    bool bOk = ReadWindowMetafile( *pSt, rWMF, NULL ) ? true : false;   
                    
    if (!bOk || pSt->GetError() || rWMF.GetActionSize() == 0)
    {
        OSL_ENSURE( !pSt, "+OLE: Konnte Metafile nicht lesen" );
        return false;
    }

    rWMF.SetPrefMapMode( MapMode( MAP_100TH_MM ) );


    
    
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
    
    SvStorageStreamRef xSrc4 = rSrc1->OpenSotStream(OUString("\3PICT"));
    SvStorageStream* pStp = xSrc4;
    pStp->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    sal_uInt8 aTestA[10];        
    sal_uLong nReadTst = pStp->Read( aTestA, sizeof( aTestA ) );
    if (nReadTst != sizeof(aTestA))
        return false;

    pStp->Seek( STREAM_SEEK_TO_BEGIN );

    
    
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

    
    
    sw::hack::DrawingOLEAdaptor aOLEObj(rObject, *pPersist);
    OUString sNewName;
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
    ::SetProgressState(nProgress, mpDocShell);     
    SwFrmFmt* pFmt = 0;

    GrafikCtor();

    Graphic aGraph;
    SdrObject* pRet = ImportOleBase(aGraph, pGrf, pFlySet, aVisArea );

    
    SfxItemSet* pTempSet = 0;
    if( !pFlySet )
    {
        pTempSet = new SfxItemSet( rDoc.GetAttrPool(), RES_FRMATR_BEGIN,
            RES_FRMATR_END-1);

        pFlySet = pTempSet;

        
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
            
            
            pSFlyPara->BoxUpWidth( aSizeTwip.Width() );
        }
    }

    if (pRet)       
    {
        if (pRet->ISA(SdrOle2Obj))
        {
            pFmt = InsertOle(*((SdrOle2Obj*)pRet), *pFlySet, *pGrfSet);
            SdrObject::Free( pRet );        
        }
        else
            pFmt = rDoc.InsertDrawObj(*pPaM, *pRet, *pFlySet );
    }
    else if (
                GRAPHIC_GDIMETAFILE == aGraph.GetType() ||
                GRAPHIC_BITMAP == aGraph.GetType()
            )
    {
        pFmt = rDoc.Insert(*pPaM, OUString(), OUString(), &aGraph, pFlySet,
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

    ::SetProgressState( nProgress, rDoc.GetDocShell() );     

    long nX=0, nY=0;                
    bool bOleOk = true;

    OUString aSrcStgName('_');
    
    aSrcStgName += OUString::number( nObjLocFc );

    SvStorageRef xSrc0 = pStg->OpenSotStorage(OUString(SL::aObjectPool));
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
            
            const Size aSizeTwip = OutputDevice::LogicToLogic(
                rGraph.GetPrefSize(), rGraph.GetPrefMapMode(), MAP_TWIP );
            nX = aSizeTwip.Width();
            nY = aSizeTwip.Height();
            
            bOleOk = false;
        }
    }       


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
        ::SetProgressState(nProgress, mpDocShell);     

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
                SvStorageStreamRef xObjInfoSrc = xSrc1->OpenSotStream(OUString("\3ObjInfo"),
                    STREAM_STD_READ | STREAM_NOCREATE );
                if ( xObjInfoSrc.Is() && !xObjInfoSrc->GetError() )
                {
                    sal_uInt8 nByte = 0;
                    xObjInfoSrc->ReadUChar( nByte );
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
    ::std::vector<OUString> aAuthorNames;
    WW8ReadSTTBF( !bVer67, rStrm, nTblPos, nTblSiz, bVer67 ? 2 : 0,
        eStructCharSet, aAuthorNames );

    sal_uInt16 nCount = static_cast< sal_uInt16 >(aAuthorNames.size());
    for( sal_uInt16 nAuthor = 0; nAuthor < nCount; ++nAuthor )
    {
        
        sal_uInt16 nSWId = rDocOut.InsertRedlineAuthor(aAuthorNames[nAuthor]);
        
        m_aAuthorInfos[nAuthor] = nSWId;
    }
}

/*
   Revision Marks ( == Redlining )
*/

void SwWW8ImplReader::Read_CRevisionMark(RedlineType_t eType,
    const sal_uInt8* pData, short nLen )
{
    
    
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
            pPlcxMan->HasCharSprm( bIns ? 0x6805 : NS_sprm::LN_CDttmRMarkDel, aResult);
            pSprmCDttmRMark = aResult.empty() ? 0 : aResult.back();
        }
    }

    if (nLen < 0)
        mpRedlineStack->close(*pPaM->GetPoint(), eType, pTableDesc );
    else
    {
        
        sal_uInt16 nWWAutNo = pSprmCIbstRMark ? SVBT16ToShort(pSprmCIbstRMark) : 0;
        sal_uInt32 nWWDate = pSprmCDttmRMark ? SVBT32ToUInt32(pSprmCDttmRMark): 0;
        DateTime aStamp(msfilter::util::DTTM2DateTime(nWWDate));
        sal_uInt16 nAuthorNo = m_aAuthorInfos[nWWAutNo];
        SwFltRedline  aNewAttr(eType, nAuthorNo, aStamp);
        NewAttr(aNewAttr);
    }
}


void SwWW8ImplReader::Read_CFRMark(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    Read_CRevisionMark( nsRedlineType_t::REDLINE_INSERT, pData, nLen );
}


void SwWW8ImplReader::Read_CFRMarkDel(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    Read_CRevisionMark( nsRedlineType_t::REDLINE_DELETE, pData, nLen );
}


void SwWW8ImplReader::Read_CPropRMark(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    
    
    
    
    Read_CRevisionMark( nsRedlineType_t::REDLINE_FORMAT, pData, nLen );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
