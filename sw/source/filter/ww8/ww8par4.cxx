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

#include <doc.hxx>
#include <IDocumentContentOperations.hxx>
#include "writerhelper.hxx"
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/Aspects.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <osl/endian.h>
#include <sot/storage.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <hintids.hxx>
#include <svx/svdoole2.hxx>
#include <filter/msfilter/msdffimp.hxx>
#include "sprmids.hxx"
#include <svx/unoapi.hxx>
#include <sal/log.hxx>

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
#include <vcl/gdimtf.hxx>

#include "ww8scan.hxx"
#include "ww8par.hxx"
#include "ww8par2.hxx"

struct OLE_MFP
{
    sal_Int16 mm;       // 0x6  int
    sal_Int16 xExt;     // 0x8  int in 1/100 mm
    sal_Int16 yExt;     // 0xa  int in 1/100 mm
    sal_Int16 hMF;      // 0xc  int
};

using namespace ::com::sun::star;

static bool SwWw8ReadScaling(long& rX, long& rY, tools::SvRef<SotStorage> const & rSrc1)
{
    // Getting the scaling factor:
    //      Information in the PIC-stream (by trying out)
    //      0x0  (l)cb
    //      0x08 .. 0x0a Flags ??
    //      0x08 contains: 1 / 0
    //      0x09 contains: 0,8,0x18
    //      0x0a contains: always 8, MAP_ANISOTROPIC ???
    //      0x0b contains: always 0
    //      0x0c, 0x10 original size x,y in 1/100 mm
    //      0x14, 0x16 original size x,y in tw
    //      0x2c, 0x30 scaling x,y in per thousand
    //      0x34, 0x38, 0x3c, 0x40 Crop Left, Top, Right, Bot in tw

    tools::SvRef<SotStorageStream> xSrc3 = rSrc1->OpenSotStream( "\3PIC",
        StreamMode::STD_READ );
    SotStorageStream* pS = xSrc3.get();
    pS->SetEndian( SvStreamEndian::LITTLE );
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
    pS->ReadInt32( nOrgWidth )    // Original Size in 1/100 mm
       .ReadInt32( nOrgHeight );
    pS->Seek( 0x2c );
    pS->ReadInt32( nScaleX )      // Scaling in Promille
       .ReadInt32( nScaleY )
       .ReadInt32( nCropLeft )    // Cropping in 1/100 mm
       .ReadInt32( nCropTop )
       .ReadInt32( nCropRight )
       .ReadInt32( nCropBottom );

    rX = nOrgWidth  - nCropLeft - nCropRight;
    rY = nOrgHeight - nCropTop  - nCropBottom;
    if (10 > nScaleX || 65536 < nScaleX || 10 > nScaleY || 65536 < nScaleY)
    {
        OSL_ENSURE( !pS, "+OLE-scaling information in PIC-stream wrong" );
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
    tools::SvRef<SotStorage> const & rSrc1)
{
    tools::SvRef<SotStorageStream> xSrc2 = rSrc1->OpenSotStream( "\3META",
        StreamMode::STD_READ );
    SotStorageStream* pSt = xSrc2.get();
    pSt->SetEndian( SvStreamEndian::LITTLE );
    size_t const nRead = pSt->ReadBytes(pMfp, sizeof(*pMfp));
                                // read mini-placable-header
    if (nRead != sizeof(*pMfp))
        return false;

#if defined  OSL_BIGENDIAN
    pMfp->mm = OSL_SWAPWORD( pMfp->mm );
    pMfp->xExt = OSL_SWAPWORD( pMfp->xExt );
    pMfp->yExt = OSL_SWAPWORD( pMfp->yExt );
#endif // OSL_BIGENDIAN

    if( pMfp->mm == 94 || pMfp->mm == 99 )
    {
        SAL_WARN("sw.ww8", "+OLE: wrong metafile type");
        return false;
    }
    if( pMfp->mm != 8 )
    {
        SAL_WARN("sw.ww8", "OLE: wrong mMetafile type (not anisotropic)");
    }
    if( !pMfp->xExt || !pMfp->yExt )
    {
        SAL_WARN("sw.ww8", "+OLE: size of 0?");
        return false;
    }
    bool bOk = ReadWindowMetafile( *pSt, rWMF );   // read WMF
                    // *pSt >> aWMF  doesn't work without the placable header
    if (!bOk || pSt->GetError() || rWMF.GetActionSize() == 0)
    {
        SAL_WARN("sw.ww8", "+OLE: could not read the metafile");
        return false;
    }

    rWMF.SetPrefMapMode( MapMode( MapUnit::Map100thMM ) );

    // Scale MetaFile to new size and save new size to MetaFile
    Size        aOldSiz( rWMF.GetPrefSize() );
    Size        aNewSiz( pMfp->xExt, pMfp->yExt );
    Fraction    aFracX( aNewSiz.Width(), aOldSiz.Width() );
    Fraction    aFracY( aNewSiz.Height(), aOldSiz.Height() );

    rWMF.Scale( aFracX, aFracY );
    rWMF.SetPrefSize( aNewSiz );

    return true;
}

static bool SwWw6ReadMacPICTStream(Graphic& rGraph, tools::SvRef<SotStorage> const & rSrc1)
{
    // 03-META-stream does not exist. Maybe a 03-PICT?
    tools::SvRef<SotStorageStream> xSrc4 = rSrc1->OpenSotStream("\3PICT");
    SotStorageStream* pStp = xSrc4.get();
    pStp->SetEndian( SvStreamEndian::LITTLE );
    sal_uInt8 aTestA[10];        // Does the 01Ole-stream even exist?
    size_t const nReadTst = pStp->ReadBytes(aTestA, sizeof(aTestA));
    if (nReadTst != sizeof(aTestA))
        return false;

    pStp->Seek( STREAM_SEEK_TO_BEGIN );

    // Mac-Pict is in the 03PICT-StorageStream but without the first 512 Bytes
    // which are not relevant in a MAC-PICT (they are not evaluated)
    return SwWW8ImplReader::GetPictGrafFromStream(rGraph, *pStp);
}

SwFlyFrameFormat* SwWW8ImplReader::InsertOle(SdrOle2Obj &rObject,
    const SfxItemSet &rFlySet, const SfxItemSet *rGrfSet)
{
    SfxObjectShell *pPersist = m_rDoc.GetPersist();
    OSL_ENSURE(pPersist, "No persist, cannot insert objects correctly");
    if (!pPersist)
        return nullptr;

    SwFlyFrameFormat *pRet = nullptr;

    std::unique_ptr<SfxItemSet> pMathFlySet;
    uno::Reference < embed::XClassifiedObject > xClass( rObject.GetObjRef(), uno::UNO_QUERY );
    if( xClass.is() )
    {
        SvGlobalName aClassName( xClass->getClassID() );
        if (SotExchange::IsMath(aClassName))
        {
            // StarMath sets it own fixed size, so its counter productive to use
            // the size Word says it is. i.e. Don't attempt to override its size.
            pMathFlySet.reset(new SfxItemSet(rFlySet));
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
    OUString sNewName;
    bool bSuccess = aOLEObj.TransferToDoc(sNewName);

    OSL_ENSURE(bSuccess, "Insert OLE failed");
    if (bSuccess)
    {
        const SfxItemSet *pFlySet = pMathFlySet ? pMathFlySet.get() : &rFlySet;
        pRet = m_rDoc.getIDocumentContentOperations().InsertOLE(*m_pPaM, sNewName, rObject.GetAspect(), pFlySet, rGrfSet);
    }
    return pRet;
}

SwFrameFormat* SwWW8ImplReader::ImportOle(const Graphic* pGrf,
    const SfxItemSet* pFlySet, const SfxItemSet *pGrfSet, const tools::Rectangle& aVisArea )
{
    ::SetProgressState(m_nProgress, m_pDocShell);     // Update
    SwFrameFormat* pFormat = nullptr;

    GrafikCtor();

    Graphic aGraph;
    SdrObject* pRet = ImportOleBase(aGraph, pGrf, pFlySet, aVisArea );

    // create flyset
    std::unique_ptr<SfxItemSet> pTempSet;
    if( !pFlySet )
    {
        pTempSet.reset( new SfxItemSet( m_rDoc.GetAttrPool(), svl::Items<RES_FRMATR_BEGIN,
            RES_FRMATR_END-1>{}) );

        pFlySet = pTempSet.get();

        // Remove distance/borders
        Reader::ResetFrameFormatAttrs( *pTempSet );

        SwFormatAnchor aAnchor( RndStdIds::FLY_AS_CHAR );
        aAnchor.SetAnchor( m_pPaM->GetPoint() );
        pTempSet->Put( aAnchor );

        const Size aSizeTwip = OutputDevice::LogicToLogic(
            aGraph.GetPrefSize(), aGraph.GetPrefMapMode(), MapMode(MapUnit::MapTwip));

        pTempSet->Put( SwFormatFrameSize( ATT_FIX_SIZE, aSizeTwip.Width(),
            aSizeTwip.Height() ) );
        pTempSet->Put( SwFormatVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ));

        if (m_xSFlyPara)
        {
            // Resize the frame to the picture size if there is an OLE object
            // in the frame (only if auto-width)
            m_xSFlyPara->BoxUpWidth(aSizeTwip.Width());
        }
    }

    if (pRet)       // OLE object was inserted
    {
        if (SdrOle2Obj *pOleObj = dynamic_cast<SdrOle2Obj*>(pRet))
        {
            pFormat = InsertOle(*pOleObj, *pFlySet, pGrfSet);
            SdrObject::Free(pRet);     // we don't need this anymore
        }
        else
            pFormat = m_rDoc.getIDocumentContentOperations().InsertDrawObj(*m_pPaM, *pRet, *pFlySet );
    }
    else if (
                GraphicType::GdiMetafile == aGraph.GetType() ||
                GraphicType::Bitmap == aGraph.GetType()
            )
    {
        pFormat = m_rDoc.getIDocumentContentOperations().InsertGraphic(
            *m_pPaM, OUString(), OUString(), &aGraph, pFlySet,
            pGrfSet, nullptr);
    }
    return pFormat;
}

bool SwWW8ImplReader::ImportOleWMF(const tools::SvRef<SotStorage>& xSrc1, GDIMetaFile& rWMF,
                                   long& rX, long& rY)
{
    bool bOk = false;
    OLE_MFP aMfp;
    if( SwWw6ReadMetaStream( rWMF, &aMfp, xSrc1 ) )
    {
        // take scaling factor as found in PIC and apply it to graphic.
        SwWw8ReadScaling( rX, rY, xSrc1 );
        Size aFinalSize, aOrigSize;
        aFinalSize.setWidth( rX );
        aFinalSize.setHeight( rY );
        aFinalSize = OutputDevice::LogicToLogic(
            aFinalSize, MapMode(MapUnit::MapTwip), rWMF.GetPrefMapMode() );
        aOrigSize = rWMF.GetPrefSize();
        Fraction aScaleX(aFinalSize.Width(),aOrigSize.Width());
        Fraction aScaleY(aFinalSize.Height(),aOrigSize.Height());
        rWMF.Scale( aScaleX, aScaleY );
        bOk = true;
    }
    return bOk;
}

SdrObject* SwWW8ImplReader::ImportOleBase( Graphic& rGraph,
    const Graphic* pGrf, const SfxItemSet* pFlySet, const tools::Rectangle& aVisArea )
{
    if (!m_pStg)
    {
        SAL_WARN("sw.ww8", "no storage for ole objects");
        return nullptr;
    }

    ::SetProgressState( m_nProgress, m_rDoc.GetDocShell() );     // Update

    long nX=0, nY=0;                // nX, nY is graphic size
    bool bOleOk = true;

    OUString aSrcStgName('_');
    // results in the name "_4711"
    aSrcStgName += OUString::number( m_nObjLocFc );

    tools::SvRef<SotStorage> xSrc0 = m_pStg->OpenSotStorage(SL::aObjectPool);
    tools::SvRef<SotStorage> xSrc1 = xSrc0->OpenSotStorage( aSrcStgName );

    if (pGrf)
    {
        rGraph = *pGrf;
        const Size aSizeTwip = OutputDevice::LogicToLogic(
            rGraph.GetPrefSize(), rGraph.GetPrefMapMode(), MapMode(MapUnit::MapTwip));
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
            // 03-META stream is not available. Maybe it's a 03-PICT?
            const Size aSizeTwip = OutputDevice::LogicToLogic(
                rGraph.GetPrefSize(), rGraph.GetPrefMapMode(), MapMode(MapUnit::MapTwip));
            nX = aSizeTwip.Width();
            nY = aSizeTwip.Height();
            // PICT: no WMF available -> Graphic instead of OLE
            bOleOk = false;
        }
    }       // StorageStreams closed again

    tools::Rectangle aRect(0, 0, nX, nY);

    if (pFlySet)
    {
        if (const SwFormatFrameSize* pSize = pFlySet->GetItem<SwFormatFrameSize>(RES_FRM_SIZE, false))
        {
            aRect.SetSize(pSize->GetSize());
        }
    }

    SdrObject* pRet = nullptr;

    if (!(m_bIsHeader || m_bIsFooter))
    {
        //Can't put them in headers/footers :-(
        uno::Reference< drawing::XShape > xRef;
        OSL_ENSURE(m_xFormImpl, "Impossible");
        if (m_xFormImpl && m_xFormImpl->ReadOCXStream(xSrc1, &xRef))
        {
            pRet = GetSdrObjectFromXShape(xRef);
            OSL_ENSURE(pRet, "Impossible");
            if (pRet)
                pRet->SetLogicRect(aRect);
            return pRet;
        }
    }

    if (GraphicType::GdiMetafile == rGraph.GetType() ||
        GraphicType::Bitmap == rGraph.GetType())
    {
        ::SetProgressState(m_nProgress, m_pDocShell);     // Update

        if (bOleOk)
        {
            sal_uLong nOldPos = m_pDataStream->Tell();
            m_pDataStream->Seek(STREAM_SEEK_TO_END);
            SvStream *pTmpData = nullptr;
            if (m_nObjLocFc < m_pDataStream->Tell())
            {
                pTmpData = m_pDataStream;
                pTmpData->Seek( m_nObjLocFc );
            }

            sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;

            {
                tools::SvRef<SotStorageStream> xObjInfoSrc = xSrc1->OpenSotStream("\3ObjInfo",
                    StreamMode::STD_READ );
                if ( xObjInfoSrc.is() && !xObjInfoSrc->GetError() )
                {
                    sal_uInt8 nByte = 0;
                    xObjInfoSrc->ReadUChar( nByte );
                    if ( ( nByte >> 4 ) & embed::Aspects::MSOLE_ICON )
                        nAspect = embed::Aspects::MSOLE_ICON;
                }
            }

            ErrCode nError = ERRCODE_NONE;
            GrafikCtor();

            pRet = SvxMSDffManager::CreateSdrOLEFromStorage(
                *m_pDrawModel,
                aSrcStgName,
                xSrc0,
                m_pDocShell->GetStorage(),
                rGraph,
                aRect,
                aVisArea,
                pTmpData,
                nError,
                SwMSDffManager::GetFilterFlags(),
                nAspect,
                GetBaseURL());
            m_pDataStream->Seek( nOldPos );
        }
    }
    return pRet;
}

void SwWW8ImplReader::ReadRevMarkAuthorStrTabl( SvStream& rStrm,
    sal_Int32 nTablePos, sal_Int32 nTableSiz, SwDoc& rDocOut )
{
    std::vector<OUString> aAuthorNames;
    WW8ReadSTTBF( !m_bVer67, rStrm, nTablePos, nTableSiz, m_bVer67 ? 2 : 0,
        m_eStructCharSet, aAuthorNames );

    sal_uInt16 nCount = static_cast< sal_uInt16 >(aAuthorNames.size());
    for( sal_uInt16 nAuthor = 0; nAuthor < nCount; ++nAuthor )
    {
        // Store author in doc
        std::size_t nSWId = rDocOut.getIDocumentRedlineAccess().InsertRedlineAuthor(aAuthorNames[nAuthor]);
        // Store matchpair
        m_aAuthorInfos[nAuthor] = nSWId;
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
    if (!m_xPlcxMan)
        return;
    const sal_uInt8* pSprmCIbstRMark;
    const sal_uInt8* pSprmCDttmRMark;
    if( nsRedlineType_t::REDLINE_FORMAT == eType )
    {
        pSprmCIbstRMark = nLen >= 3 ? pData+1 : nullptr;
        pSprmCDttmRMark = nLen >= 7 ? pData+3 : nullptr;
    }
    else
    {
        /* It is possible to have a number of date stamps for the created time
         * of the change, (possibly a word bug) so we must use the "get a full
         * list" variant of HasCharSprm and take the last one as the true one.
         */
        std::vector<SprmResult> aResult;
        bool bIns = (nsRedlineType_t::REDLINE_INSERT == eType);
        if( m_bVer67 )
        {
            m_xPlcxMan->HasCharSprm(69, aResult);
            pSprmCIbstRMark = (aResult.empty() || aResult.back().nRemainingData < 2) ? nullptr : aResult.back().pSprm;
            aResult.clear();
            m_xPlcxMan->HasCharSprm(70, aResult);
            pSprmCDttmRMark = (aResult.empty() || aResult.back().nRemainingData < 4) ? nullptr : aResult.back().pSprm;
        }
        else
        {
            m_xPlcxMan->HasCharSprm( bIns ? 0x4804 : 0x4863, aResult);
            pSprmCIbstRMark = (aResult.empty() || aResult.back().nRemainingData < 2) ? nullptr : aResult.back().pSprm;
            aResult.clear();
            m_xPlcxMan->HasCharSprm( bIns ? 0x6805 : NS_sprm::sprmCDttmRMarkDel, aResult);
            pSprmCDttmRMark = (aResult.empty() || aResult.back().nRemainingData < 4) ? nullptr : aResult.back().pSprm;
        }
    }

    if (nLen < 0)
        m_xRedlineStack->close(*m_pPaM->GetPoint(), eType, m_xTableDesc.get());
    else
    {
        // start of new revision mark, if not there default to first entry
        sal_uInt16 nWWAutNo = pSprmCIbstRMark ? SVBT16ToUInt16(pSprmCIbstRMark) : 0;
        sal_uInt32 nWWDate = pSprmCDttmRMark ? SVBT32ToUInt32(pSprmCDttmRMark): 0;
        DateTime aStamp(msfilter::util::DTTM2DateTime(nWWDate));
        std::size_t nAuthorNo = m_aAuthorInfos[nWWAutNo];
        SwFltRedline  aNewAttr(eType, nAuthorNo, aStamp);
        NewAttr(aNewAttr);
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

// change properties of content ( == char formatting)
void SwWW8ImplReader::Read_CPropRMark(sal_uInt16 , const sal_uInt8* pData, short nLen)
{
    // complex (len is always 7)
    // 1 byte  - chp.fPropRMark
    // 2 bytes - chp.ibstPropRMark
    // 4 bytes - chp.dttmPropRMark;
    Read_CRevisionMark( nsRedlineType_t::REDLINE_FORMAT, pData, nLen );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
