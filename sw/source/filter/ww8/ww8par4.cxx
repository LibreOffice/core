/*************************************************************************
 *
 *  $RCSfile: ww8par4.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 11:38:09 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef SW_WRITERHELPER
#include "writerhelper.hxx"
#endif

#ifndef __SGI_STL_ALGORITHM
#include <algorithm>
#endif
#ifndef __SGI_STL_FUNCTIONAL
#include <functional>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVDOOLE2_HXX
#include <svx/svdoole2.hxx>
#endif
#ifndef _MSDFFIMP_HXX
#include <svx/msdffimp.hxx>
#endif
#ifndef _SVX_IMPGRF_HXX
#include <svx/impgrf.hxx>
#endif
#ifndef _SVX_UNOAPI_HXX_
#include <svx/unoapi.hxx>
#endif
#ifndef _MSOCXIMEX_HXX
#include <svx/msocximex.hxx>
#endif

#include <sot/exchange.hxx>

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNTNT_HXX
#include <fmtcntnt.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>            // fuer Ole-Node
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>           // Progress
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _FLTSHELL_HXX
#include <fltshell.hxx>
#endif
#ifndef _UNODRAW_HXX
#include <unodraw.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif


#ifndef _WW8SCAN_HXX
#include "ww8scan.hxx"
#endif
#ifndef _WW8PAR_HXX
#include "ww8par.hxx"
#endif
#ifndef _WW8PAR2_HXX
#include "ww8par2.hxx"          // WWFlyPara::BoxUpWidth()
#endif

struct OLE_MFP
{
    INT16 mm;       // 0x6  int
    INT16 xExt;     // 0x8  int in 1/100 mm
    INT16 yExt;     // 0xa  int in 1/100 mm
    INT16 hMF;      // 0xc  int
};

using namespace ::com::sun::star;

SV_IMPL_OP_PTRARR_SORT(WW8AuthorInfos, WW8AuthorInfo_Ptr)
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

    SvStorageStreamRef xSrc3 = rSrc1->OpenStream( CREATE_CONST_ASC( "\3PIC" ),
        STREAM_STD_READ | STREAM_NOCREATE);
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
    if (10 > nScaleX || 65536 < nScaleX || 10 > nScaleY || 65536 < nScaleY)
    {
        ASSERT( !pS, "+OLE-Scalinginformation in PIC-Stream wrong" );
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
    SvStorageStreamRef xSrc2 = rSrc1->OpenStream( CREATE_CONST_ASC("\3META"),
        STREAM_STD_READ | STREAM_NOCREATE);
    SvStorageStream* pSt = xSrc2;
    pSt->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    ULONG nRead = pSt->Read( pMfp, sizeof(*pMfp ) );
                                // Mini-Placable-Header lesen
    if (nRead != sizeof(*pMfp))
        return false;

#if defined  __BIGENDIAN
    pMfp->mm = SWAPSHORT( pMfp->mm );
    pMfp->xExt = SWAPSHORT( pMfp->xExt );
    pMfp->yExt = SWAPSHORT( pMfp->yExt );
#endif // __BIGENDIAN

    if( pMfp->mm == 94 || pMfp->mm == 99 )
    {
        ASSERT( !pSt, "+OLE: Falscher Metafile-Typ" );
        return false;
    }
    if( pMfp->mm != 8 )
    {
        ASSERT( !pSt, "+OLE: Falscher Metafile-Typ ( nicht Anisotropic )" );
    }
    if( !pMfp->xExt || !pMfp->yExt )
    {
        ASSERT( !pSt, "+OLE: Groesse von 0 ???" );
        return false;
    }
    bool bOk = ReadWindowMetafile(*pSt, rWMF) ? true : false;   // WMF lesen
                    // *pSt >> aWMF  geht nicht ohne placable Header
    if (!bOk || pSt->GetError() || rWMF.GetActionCount() == 0)
    {
        ASSERT( !pSt, "+OLE: Konnte Metafile nicht lesen" );
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
    SvStorageStreamRef xSrc4 = rSrc1->OpenStream( CREATE_CONST_ASC( "\3PICT" ));
    SvStorageStream* pStp = xSrc4;
    pStp->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    BYTE aTestA[10];        // Ist der 01Ole-Stream ueberhaupt vorhanden
    ULONG nReadTst = pStp->Read( aTestA, sizeof( aTestA ) );
    if (nReadTst != sizeof(aTestA))
        return false;

    pStp->Seek( STREAM_SEEK_TO_BEGIN );

    // Mac-Pict steht im 03PICT-StorageStream allerdings ohne die ersten 512
    // Bytes, die bei einem MAC-PICT egal sind ( werden nicht ausgewertet )
    return SwWW8ImplReader::GetPictGrafFromStream( rGraph, *pStp );
}

SwFlyFrmFmt* SwWW8ImplReader::InsertOle(SdrOle2Obj &rObject,
    const SfxItemSet &rFlySet)
{
    SvPersist *pPersist = rDoc.GetPersist();
    ASSERT(pPersist, "No persist, cannot insert objects correctly");
    if (!pPersist)
        return 0;

    SwFlyFrmFmt *pRet = 0;

    SfxItemSet *pMathFlySet = 0;
    if (SotExchange::IsMath(*rObject.GetObjRef()->GetSvFactory()))
    {
        /*
        StarMath sets it own fixed size, so its counter productive to use the
        size word says it is. i.e. Don't attempt to override its size.
        */
        pMathFlySet = new SfxItemSet(rFlySet);
        pMathFlySet->ClearItem(RES_FRM_SIZE);
    }

    String sNewName = Sw3Io::UniqueName(rDoc.GetDocShell()->GetStorage(),"Obj");

    /*
    Take complete responsibility of the object away from SdrOle2Obj and to
    me here locally. This utility class now owns the object.
    */
    sw::hack::DrawingOLEAdaptor aOLEObj(rObject, *pPersist);

    bool bSuccess = aOLEObj.TransferToDoc(sNewName);

    ASSERT(bSuccess, "Insert OLE failed");
    if (bSuccess)
    {
        const SfxItemSet *pFlySet = pMathFlySet ? pMathFlySet : &rFlySet;
        pRet = rDoc.InsertOLE(*pPaM, sNewName, pFlySet);
    }
    delete pMathFlySet;
    return pRet;
}

SwFrmFmt* SwWW8ImplReader::ImportOle(const Graphic* pGrf,
    const SfxItemSet* pFlySet)
{
    SwFrmFmt* pFmt = 0;
    if( !(nIniFlags & WW8FL_NO_OLE ))
    {
        Graphic aGraph;
        SdrObject* pRet = ImportOleBase(aGraph, pGrf, pFlySet);

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

            SwFmtAnchor aAnchor( FLY_IN_CNTNT );
            aAnchor.SetAnchor( pPaM->GetPoint() );
            pTempSet->Put( aAnchor );

            const Size aSizeTwip = OutputDevice::LogicToLogic(
                aGraph.GetPrefSize(), aGraph.GetPrefMapMode(), MAP_TWIP );

            pTempSet->Put( SwFmtFrmSize( ATT_FIX_SIZE, aSizeTwip.Width(),
                aSizeTwip.Height() ) );
            pTempSet->Put( SwFmtVertOrient( 0, VERT_TOP, FRAME ));

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
                pFmt = InsertOle(*((SdrOle2Obj*)pRet),*pFlySet);
                delete pRet;        // das brauchen wir nicht mehr
            }
            else
                pFmt = rDoc.Insert(*pPaM, *pRet, pFlySet);
        }
        else if (
                    GRAPHIC_GDIMETAFILE == aGraph.GetType() ||
                    GRAPHIC_BITMAP == aGraph.GetType()
                )
        {
            pFmt = rDoc.Insert(*pPaM, aEmptyStr, aEmptyStr, &aGraph, pFlySet,0);
        }
        delete pTempSet;
    }
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
    const Graphic* pGrf, const SfxItemSet* pFlySet )
{
    SdrObject* pRet = 0;
    if( !(nIniFlags & WW8FL_NO_OLE ))
    {
        ASSERT( pStg, "ohne storage geht hier fast gar nichts!" );

        ::SetProgressState( nProgress, rDoc.GetDocShell() );     // Update

        long nX=0, nY=0;                // nX, nY is graphic size
        bool bOleOk = true;

        String aSrcStgName = '_';
        // ergibt Name "_4711"
        aSrcStgName += String::CreateFromInt32( nObjLocFc );

        SvStorageRef xSrc0 = pStg->OpenStorage(CREATE_CONST_ASC(
            SL::aObjectPool));

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
            SvStorageRef xSrc1 = xSrc0->OpenStorage( aSrcStgName,
                STREAM_READWRITE| STREAM_SHARE_DENYALL );

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

        SvStorageRef xSrc1 = xSrc0->OpenStorage( aSrcStgName,
            STREAM_READWRITE| STREAM_SHARE_DENYALL );

        if (!(bIsHeader || bIsFooter))
        {
            //Can't put them in headers/footers :-(
            if(!pFormImpl)
                pFormImpl = new SwMSConvertControls(rDoc.GetDocShell(),pPaM);
            uno::Reference< drawing::XShape > xRef;
            if (pFormImpl->ReadOCXStream(xSrc1, &xRef, false))
            {
                pRet = GetSdrObjectFromXShape(xRef);
                pRet->SetLogicRect(aRect);
                return pRet;
            }
        }

        if (GRAPHIC_GDIMETAFILE == rGraph.GetType() ||
            GRAPHIC_BITMAP == rGraph.GetType())
        {
            ::SetProgressState( nProgress, rDoc.GetDocShell() );     // Update

            if (bOleOk)
            {
                ULONG nOldPos = pDataStream->Tell();
                pDataStream->Seek(STREAM_SEEK_TO_END);
                SvStream *pTmpData = 0;
                if (nObjLocFc < pDataStream->Tell())
                {
                    pTmpData = pDataStream;
                    pTmpData->Seek( nObjLocFc );
                }

                SvStorageRef xDst0( rDoc.GetDocShell()->GetStorage() );

                pRet = SvxMSDffManager::CreateSdrOLEFromStorage(
                    aSrcStgName, xSrc0, xDst0, rGraph, aRect, pTmpData,
                    SwMSDffManager::GetFilterFlags());
                pDataStream->Seek( nOldPos );
            }
        }
    }
    return pRet;
}

void wwRedlineStack::open(const SwPosition& rPos, const SfxPoolItem& rAttr)
{
    ASSERT(rAttr.Which() == RES_FLTR_REDLINE, "not a redline");
    maStack.push_back(new SwFltStackEntry(rPos,rAttr.Clone()));
}

class SameOpenRedlineType :
    public std::unary_function<const SwFltStackEntry*, bool>
{
private:
    SwRedlineType meType;
public:
    SameOpenRedlineType(SwRedlineType eType) : meType(eType) {}
    bool operator()(const SwFltStackEntry *pEntry) const
    {
        const SwFltRedline *pTest = static_cast<const SwFltRedline *>
            (pEntry->pAttr);
        return (pEntry->bLocked && (pTest->eType == meType));
    }
};

void wwRedlineStack::close(const SwPosition& rPos, SwRedlineType eType)
{
    //Search from end for same type
    myriter aResult = std::find_if(maStack.rbegin(), maStack.rend(),
        SameOpenRedlineType(eType));
    ASSERT(aResult != maStack.rend(), "close without open!");
    if (aResult != maStack.rend())
        (*aResult)->SetEndPos(rPos);
}

class CloseIfOpen       //Subclass from something ?
{
private:
    const SwPosition &mrPos;
public:
    explicit CloseIfOpen(const SwPosition &rPos) : mrPos(rPos) {}
        void operator()(SwFltStackEntry *pEntry) const
    {
        if (pEntry->bLocked)
            pEntry->SetEndPos(mrPos);
    }
private:
   //No assignment
   CloseIfOpen& operator=(const CloseIfOpen&);
};

void wwRedlineStack::closeall(const SwPosition& rPos)
{
    std::for_each(maStack.begin(), maStack.end(), CloseIfOpen(rPos));
}

class SetInDocAndDelete
{
private:
    SwDoc &mrDoc;
public:
    explicit SetInDocAndDelete(SwDoc &rDoc) : mrDoc(rDoc) {}
    void operator()(SwFltStackEntry *pEntry);
private:
   //No assignment
   SetInDocAndDelete& operator=(const SetInDocAndDelete&);
};

void SetInDocAndDelete::operator()(SwFltStackEntry *pEntry)
{
    SwPaM aRegion(pEntry->nMkNode);
    if (
         pEntry->MakeRegion(&mrDoc, aRegion, true) &&
         (*aRegion.GetPoint() != *aRegion.GetMark())
       )
    {
        mrDoc.SetRedlineMode(REDLINE_ON | REDLINE_SHOW_INSERT |
            REDLINE_SHOW_DELETE);
        const SwFltRedline *pFltRedline = static_cast<const SwFltRedline*>
            (pEntry->pAttr);

        if (USHRT_MAX != pFltRedline->nAutorNoPrev)
        {
            SwRedlineData aData(pFltRedline->eTypePrev,
                pFltRedline->nAutorNoPrev, pFltRedline->aStampPrev, aEmptyStr,
                0);

            mrDoc.AppendRedline(new SwRedline(aData, aRegion));
        }

        SwRedlineData aData(pFltRedline->eType, pFltRedline->nAutorNo,
                pFltRedline->aStamp, aEmptyStr, 0);

        mrDoc.AppendRedline(new SwRedline(aData, aRegion));
        mrDoc.SetRedlineMode(REDLINE_NONE | REDLINE_SHOW_INSERT |
            REDLINE_SHOW_DELETE );
    }
    delete pEntry;
}

class CompareRedlines:
    public std::binary_function<const SwFltStackEntry*, const SwFltStackEntry*,
       bool>
{
public:
    bool operator()(const SwFltStackEntry *pOneE, const SwFltStackEntry *pTwoE)
        const;
};

bool CompareRedlines::operator()(const SwFltStackEntry *pOneE,
    const SwFltStackEntry *pTwoE) const
{
    const SwFltRedline *pOne= static_cast<const SwFltRedline*>
        (pOneE->pAttr);
    const SwFltRedline *pTwo= static_cast<const SwFltRedline*>
        (pTwoE->pAttr);

    //Return the earlier time, if two have the same time, prioritize
    //inserts over deletes
    if (pOne->aStamp == pTwo->aStamp)
        return (pOne->eType == REDLINE_INSERT && pTwo->eType != REDLINE_INSERT);
    else
        return (pOne->aStamp < pTwo->aStamp) ? true : false;
}

wwRedlineStack::~wwRedlineStack()
{
    std::sort(maStack.begin(), maStack.end(), CompareRedlines());
    std::for_each(maStack.begin(), maStack.end(), SetInDocAndDelete(mrDoc));
}

void SwWW8ImplReader::ReadRevMarkAuthorStrTabl( SvStream& rStrm,
    INT32 nTblPos, INT32 nTblSiz, SwDoc& rDocOut )
{
    ::std::vector<String> aAuthorNames;
    WW8ReadSTTBF( !bVer67, rStrm, nTblPos, nTblSiz, bVer67 ? 2 : 0,
        eStructCharSet, aAuthorNames );

    USHORT nCount = aAuthorNames.size();
    for( USHORT nAuthor = 0; nAuthor < nCount; ++nAuthor )
    {
        // Store author in doc
        USHORT nSWId = rDocOut.InsertRedlineAuthor(aAuthorNames[nAuthor]);
        // Store matchpair
        if( !pAuthorInfos )
            pAuthorInfos = new WW8AuthorInfos;
        WW8AuthorInfo* pAutorInfo = new WW8AuthorInfo( nAuthor, nSWId );
        if( 0 == pAuthorInfos->Insert( pAutorInfo ) )
            delete pAutorInfo;
    }
}

/*
   Revision Marks ( == Redlining )
*/
// insert or delete content (change char attributes resp.)
void SwWW8ImplReader::Read_CRevisionMark(SwRedlineType eType,
    const BYTE* pData, short nLen )
{
    // there *must* be a SprmCIbstRMark[Del] and a SprmCDttmRMark[Del]
    // pointing to the very same char position as our SprmCFRMark[Del]
    if (!pPlcxMan || bIgnoreText)
        return;
    const BYTE* pSprmCIbstRMark;
    const BYTE* pSprmCDttmRMark;
    if( REDLINE_FORMAT == eType )
    {
        pSprmCIbstRMark = pData+1;
        pSprmCDttmRMark = pData+3;
    }
    else
    {
        /*
         #101578#
         It is possible to have a number of date stamps for the created time
         of the change, (possibly a word bug) so we must use the "get a full
         list" varient of HasCharSprm and take the last one as the true one.
        */
        std::vector<const BYTE *> aResult;
        bool bIns = (REDLINE_INSERT == eType);
        if( bVer67 )
        {
            pPlcxMan->HasCharSprm(69, aResult);
            pSprmCIbstRMark = aResult.empty() ? 0 : aResult.back();
            pPlcxMan->HasCharSprm(70, aResult);
            pSprmCDttmRMark = aResult.empty() ? 0 : aResult.back();
        }
        else
        {
            pPlcxMan->HasCharSprm( bIns ? 0x4804 : 0x4863, aResult);
            pSprmCIbstRMark = aResult.empty() ? 0 : aResult.back();
            pPlcxMan->HasCharSprm( bIns ? 0x6805 : 0x6864, aResult);
            pSprmCDttmRMark = aResult.empty() ? 0 : aResult.back();
        }
    }

#if 0
    ASSERT(nLen < 0 || (pSprmCIbstRMark || pSprmCDttmRMark),
        "The wheels have fallen off revision mark import");
#endif

    if (nLen < 0)
        mpRedlineStack->close(*pPaM->GetPoint(), eType);
    else
    {
        // start of new revision mark, if not there default to first entry
        USHORT nWWAutNo = pSprmCIbstRMark ? SVBT16ToShort( pSprmCIbstRMark ) : 0;
        WW8AuthorInfo aEntry(nWWAutNo);
        USHORT nPos;
        if (pAuthorInfos && pAuthorInfos->Seek_Entry(&aEntry, &nPos))
        {
            if (const WW8AuthorInfo* pAuthor = pAuthorInfos->GetObject(nPos))
            {
                UINT32 nWWDate = pSprmCDttmRMark ? SVBT32ToLong(pSprmCDttmRMark): 0;
#if 0
                ASSERT(nWWDate, "Date is 0, this will cause trouble!");
#endif

                DateTime aStamp(WW8ScannerBase::WW8DTTM2DateTime(nWWDate));
                USHORT nAutorNo = pAuthor->nOurId;
                SwFltRedline  aNewAttr(eType, nAutorNo, aStamp);

                NewAttr(aNewAttr);
            }
        }
    }
}

// insert new content
void SwWW8ImplReader::Read_CFRMark(USHORT , const BYTE* pData, short nLen)
{
    Read_CRevisionMark( REDLINE_INSERT, pData, nLen );
}

// delete old content
void SwWW8ImplReader::Read_CFRMarkDel(USHORT , const BYTE* pData, short nLen)
{
    Read_CRevisionMark( REDLINE_DELETE, pData, nLen );
}

// change properties of content ( == char formating)
void SwWW8ImplReader::Read_CPropRMark(USHORT , const BYTE* pData, short nLen)
{
    // complex (len is always 7)
    // 1 byte  - chp.fPropRMark
    // 2 bytes - chp.ibstPropRMark
    // 4 bytes - chp.dttmPropRMark;
    Read_CRevisionMark( REDLINE_FORMAT, pData, nLen );
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
