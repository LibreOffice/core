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

#include "sal/config.h"

#include "editeng/forbiddencharacterstable.hxx"
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include "officecfg/Office/Common.hxx"
#include <svx/svdetc.hxx>
#include "svx/svditext.hxx"
#include <svx/svdmodel.hxx>
#include <svx/svdtrans.hxx>
#include "svx/svdglob.hxx"
#include "svx/svdstr.hrc"
#include "svx/svdviter.hxx"
#include <svx/svdview.hxx>
#include <svx/svdoutl.hxx>
#include <vcl/bmpacc.hxx>
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>
#include <svl/whiter.hxx>
#include "editeng/fontitem.hxx"
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/xgrad.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/svdoole2.hxx>
#include <svl/itempool.hxx>
#include <unotools/localedatawrapper.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <i18npool/lang.h>
#include <unotools/syslocale.hxx>
#include <svx/xflbckit.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/fontworkbar.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdotable.hxx>
#include <svx/sdrhittesthelper.hxx>

using namespace ::com::sun::star;

/******************************************************************************
* Global data of the DrawingEngine
******************************************************************************/

SdrGlobalData::SdrGlobalData() :
    pSysLocale(NULL),
    pLocaleData(NULL),
    pOutliner(NULL),
    pDefaults(NULL),
    pResMgr(NULL),
    nExchangeFormat(0)
{

    svx::ExtrusionBar::RegisterInterface();
    svx::FontworkBar::RegisterInterface();
}

const SvtSysLocale*         SdrGlobalData::GetSysLocale()
{
    if ( !pSysLocale )
        pSysLocale = new SvtSysLocale;
    return pSysLocale;
}
const LocaleDataWrapper*    SdrGlobalData::GetLocaleData()
{
    if ( !pLocaleData )
        pLocaleData = GetSysLocale()->GetLocaleDataPtr();
    return pLocaleData;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

OLEObjCache::OLEObjCache()
:   Container( 0 )
{
    nSize = officecfg::Office::Common::Cache::DrawingEngine::OLE_Objects::get();
    pTimer = new AutoTimer();
    Link aLink = LINK(this, OLEObjCache, UnloadCheckHdl);

    pTimer->SetTimeoutHdl(aLink);
    pTimer->SetTimeout(20000);
    pTimer->Start();

    aLink.Call(pTimer);
}

OLEObjCache::~OLEObjCache()
{
    pTimer->Stop();
    delete pTimer;
}

void OLEObjCache::UnloadOnDemand()
{
    if ( nSize < Count() )
    {
        // more objects than configured cache size try to remove objects
        // of course not the freshly inserted one at nIndex=0
        sal_uIntPtr nCount2 = Count();
        sal_uIntPtr nIndex = nCount2-1;
        while( nIndex && nCount2 > nSize )
        {
            SdrOle2Obj* pUnloadObj = (SdrOle2Obj*) GetObject(nIndex--);
            if ( pUnloadObj )
            {
                try
                {
                    // it is important to get object without reinitialization to avoid reentrance
                    uno::Reference< embed::XEmbeddedObject > xUnloadObj = pUnloadObj->GetObjRef_NoInit();

                    sal_Bool bUnload = SdrOle2Obj::CanUnloadRunningObj( xUnloadObj, pUnloadObj->GetAspect() );

                    // check whether the object can be unloaded before looking for the parent objects
                    if ( xUnloadObj.is() && bUnload )
                    {
                        uno::Reference< frame::XModel > xUnloadModel( xUnloadObj->getComponent(), uno::UNO_QUERY );
                        if ( xUnloadModel.is() )
                        {
                            for ( sal_uIntPtr nCheckInd = 0; nCheckInd < Count(); nCheckInd++ )
                            {
                                SdrOle2Obj* pCacheObj = (SdrOle2Obj*) GetObject(nCheckInd);
                                if ( pCacheObj && pCacheObj != pUnloadObj )
                                {
                                    uno::Reference< frame::XModel > xParentModel = pCacheObj->GetParentXModel();
                                    if ( xUnloadModel == xParentModel )
                                        bUnload = sal_False; // the object has running embedded objects
                                }
                            }
                        }
                    }

                    if ( bUnload && UnloadObj(pUnloadObj) )
                        // object was successfully unloaded
                        nCount2--;
                }
                catch( uno::Exception& )
                {}
            }
        }
    }
}

void OLEObjCache::InsertObj(SdrOle2Obj* pObj)
{
    if ( Count() )
    {
        SdrOle2Obj* pExistingObj = (SdrOle2Obj*)GetObject( 0 );
        if ( pObj == pExistingObj )
            // the object is already on the top, nothing has to be changed
            return;
    }

    // get the old position of the object to know whether it is already in container
    sal_uIntPtr nOldPos = GetPos( pObj );

    // insert object into first position
    Remove( nOldPos );
    Insert(pObj, (sal_uIntPtr) 0L);

    if ( nOldPos == CONTAINER_ENTRY_NOTFOUND )
    {
        // a new object was inserted, recalculate the cache
        UnloadOnDemand();
    }
}

void OLEObjCache::RemoveObj(SdrOle2Obj* pObj)
{
    Remove(pObj);
}

sal_Bool OLEObjCache::UnloadObj(SdrOle2Obj* pObj)
{
    sal_Bool bUnloaded = sal_False;
    if (pObj)
    {
        //#i80528# The old mechanism is completely useless, only taking into account if
        // in all views the GrafDraft feature is used. This will nearly never have been the
        // case since no one ever used this option.
        //
        // A much better (and working) criteria would be the VOC contact count.
        // The question is what will happen when i make it work now suddenly? I
        // will try it for 2.4.
        const sdr::contact::ViewContact& rViewContact = pObj->GetViewContact();
        const bool bVisible(rViewContact.HasViewObjectContacts(true));

        if(!bVisible)
        {
            bUnloaded = pObj->Unload();
        }
    }

    return bUnloaded;
}

IMPL_LINK(OLEObjCache, UnloadCheckHdl, AutoTimer*, /*pTim*/)
{
    UnloadOnDemand();
    return 0;
}

void ContainerSorter::DoSort(sal_uIntPtr a, sal_uIntPtr b) const
{
    sal_uIntPtr nAnz=rCont.Count();
    if (b>nAnz) b=nAnz;
    if (b>0) b--;
    if (a<b) ImpSubSort(a,b);
}

void ContainerSorter::ImpSubSort(long nL, long nR) const
{
    long i,j;
    const void* pX;
    void* pI;
    void* pJ;
    i=nL;
    j=nR;
    pX=rCont.GetObject((nL+nR)/2);
    do {
        pI=rCont.Seek(i);
        while (pI!=pX && Compare(pI,pX)<0) { i++; pI=rCont.Next(); }
        pJ=rCont.Seek(j);
        while (pJ!=pX && Compare(pX,pJ)<0) { j--; pJ=rCont.Prev(); }
        if (i<=j) {
            rCont.Replace(pJ,i);
            rCont.Replace(pI,j);
            i++;
            j--;
        }
    } while (i<=j);
    if (nL<j) ImpSubSort(nL,j);
    if (i<nR) ImpSubSort(i,nR);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpUShortContainerSorter: public ContainerSorter {
public:
    ImpUShortContainerSorter(Container& rNewCont): ContainerSorter(rNewCont) {}
    virtual ~ImpUShortContainerSorter() {}
    virtual int Compare(const void* pElem1, const void* pElem2) const;
};

int ImpUShortContainerSorter::Compare(const void* pElem1, const void* pElem2) const
{
    sal_uInt16 n1=sal_uInt16(sal_uIntPtr(pElem1));
    sal_uInt16 n2=sal_uInt16(sal_uIntPtr(pElem2));
    return n1<n2 ? -1 : n1>n2 ? 1 : 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrLinkList::Clear()
{
    unsigned nAnz=GetLinkCount();
    for (unsigned i=0; i<nAnz; i++) {
        delete (Link*)aList.GetObject(i);
    }
    aList.Clear();
}

unsigned SdrLinkList::FindEntry(const Link& rLink) const
{
    unsigned nAnz=GetLinkCount();
    for (unsigned i=0; i<nAnz; i++) {
        if (GetLink(i)==rLink) return i;
    }
    return 0xFFFF;
}

void SdrLinkList::InsertLink(const Link& rLink, unsigned nPos)
{
    unsigned nFnd=FindEntry(rLink);
    if (nFnd==0xFFFF) {
        if (rLink.IsSet()) {
            aList.Insert(new Link(rLink),nPos);
        } else {
            OSL_FAIL("SdrLinkList::InsertLink(): Tried to insert a link that was not set already.");
        }
    } else {
        OSL_FAIL("SdrLinkList::InsertLink(): Link already in place.");
    }
}

void SdrLinkList::RemoveLink(const Link& rLink)
{
    unsigned nFnd=FindEntry(rLink);
    if (nFnd!=0xFFFF) {
        Link* pLink=(Link*)aList.Remove(nFnd);
        delete pLink;
    } else {
        OSL_FAIL("SdrLinkList::RemoveLink(): Link not found.");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool GetDraftFillColor(const SfxItemSet& rSet, Color& rCol)
{
    XFillStyle eFill=((XFillStyleItem&)rSet.Get(XATTR_FILLSTYLE)).GetValue();
    bool bRetval = false;

    switch(eFill)
    {
        case XFILL_SOLID:
        {
            rCol = ((XFillColorItem&)rSet.Get(XATTR_FILLCOLOR)).GetColorValue();
            bRetval = true;

            break;
        }
        case XFILL_HATCH:
        {
            Color aCol1(((XFillHatchItem&)rSet.Get(XATTR_FILLHATCH)).GetHatchValue().GetColor());
            Color aCol2(COL_WHITE);

            // when hatched background is activated, use object fill color as hatch color
            sal_Bool bFillHatchBackground = ((const XFillBackgroundItem&)(rSet.Get(XATTR_FILLBACKGROUND))).GetValue();
            if(bFillHatchBackground)
            {
                aCol2 = ((const XFillColorItem&)(rSet.Get(XATTR_FILLCOLOR))).GetColorValue();
            }

            const basegfx::BColor aAverageColor(basegfx::average(aCol1.getBColor(), aCol2.getBColor()));
            rCol = Color(aAverageColor);
            bRetval = true;

            break;
        }
        case XFILL_GRADIENT: {
            const XGradient& rGrad=((XFillGradientItem&)rSet.Get(XATTR_FILLGRADIENT)).GetGradientValue();
            Color aCol1(rGrad.GetStartColor());
            Color aCol2(rGrad.GetEndColor());
            const basegfx::BColor aAverageColor(basegfx::average(aCol1.getBColor(), aCol2.getBColor()));
            rCol = Color(aAverageColor);
            bRetval = true;

            break;
        }
        case XFILL_BITMAP:
        {
            const Bitmap& rBitmap = ((XFillBitmapItem&)rSet.Get(XATTR_FILLBITMAP)).GetBitmapValue().GetBitmap();
            const Size aSize(rBitmap.GetSizePixel());
            const sal_uInt32 nWidth = aSize.Width();
            const sal_uInt32 nHeight = aSize.Height();
            Bitmap aBitmap(rBitmap);
            BitmapReadAccess* pAccess = aBitmap.AcquireReadAccess();

            if(pAccess && nWidth > 0 && nHeight > 0)
            {
                sal_uInt32 nRt(0L);
                sal_uInt32 nGn(0L);
                sal_uInt32 nBl(0L);
                const sal_uInt32 nMaxSteps(8L);
                const sal_uInt32 nXStep((nWidth > nMaxSteps) ? nWidth / nMaxSteps : 1L);
                const sal_uInt32 nYStep((nHeight > nMaxSteps) ? nHeight / nMaxSteps : 1L);
                sal_uInt32 nAnz(0L);

                for(sal_uInt32 nY(0L); nY < nHeight; nY += nYStep)
                {
                    for(sal_uInt32 nX(0L); nX < nWidth; nX += nXStep)
                    {
                        const BitmapColor& rCol2 = (pAccess->HasPalette())
                            ? pAccess->GetPaletteColor((sal_uInt8)pAccess->GetPixel(nY, nX))
                            : pAccess->GetPixel(nY, nX);

                        nRt += rCol2.GetRed();
                        nGn += rCol2.GetGreen();
                        nBl += rCol2.GetBlue();
                        nAnz++;
                    }
                }

                nRt /= nAnz;
                nGn /= nAnz;
                nBl /= nAnz;

                rCol = Color(sal_uInt8(nRt), sal_uInt8(nGn), sal_uInt8(nBl));

                bRetval = true;
            }

            if(pAccess)
            {
                aBitmap.ReleaseAccess(pAccess);
            }

            break;
        }
        default: break;
    }

    return bRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrEngineDefaults::SdrEngineDefaults():
    aFontName( OutputDevice::GetDefaultFont( DEFAULTFONT_SERIF, LANGUAGE_SYSTEM, DEFAULTFONT_FLAGS_ONLYONE ).GetName() ),
    eFontFamily(FAMILY_ROMAN),
    aFontColor(COL_AUTO),
    nFontHeight(847),             // 847/100mm = ca. 24 Point
    eMapUnit(MAP_100TH_MM),
    aMapFraction(1,1)
{
}

SdrEngineDefaults& SdrEngineDefaults::GetDefaults()
{
    SdrGlobalData& rGlobalData=GetSdrGlobalData();
    if (rGlobalData.pDefaults==NULL) {
        rGlobalData.pDefaults=new SdrEngineDefaults;
    }
    return *rGlobalData.pDefaults;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrOutliner* SdrMakeOutliner( sal_uInt16 nOutlinerMode, SdrModel* pModel )
{
    SfxItemPool* pPool = &pModel->GetItemPool();
    SdrOutliner* pOutl = new SdrOutliner( pPool, nOutlinerMode );
    pOutl->SetEditTextObjectPool( pPool );
    pOutl->SetStyleSheetPool( (SfxStyleSheetPool*) pModel->GetStyleSheetPool() );
    pOutl->SetDefTab( pModel->GetDefaultTabulator() );
    pOutl->SetForbiddenCharsTable( pModel->GetForbiddenCharsTable() );
    pOutl->SetAsianCompressionMode( pModel->GetCharCompressType() );
    pOutl->SetKernAsianPunctuation( pModel->IsKernAsianPunctuation() );
    pOutl->SetAddExtLeading( pModel->IsAddExtLeading() );

    return pOutl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////


SdrLinkList& ImpGetUserMakeObjHdl()
{
    SdrGlobalData& rGlobalData=GetSdrGlobalData();
    return rGlobalData.aUserMakeObjHdl;
}

SdrLinkList& ImpGetUserMakeObjUserDataHdl()
{
    SdrGlobalData& rGlobalData=GetSdrGlobalData();
    return rGlobalData.aUserMakeObjUserDataHdl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ResMgr* ImpGetResMgr()
{
    SdrGlobalData& rGlobalData = GetSdrGlobalData();

    if(!rGlobalData.pResMgr)
    {
        rGlobalData.pResMgr =
            ResMgr::CreateResMgr( "svx", Application::GetSettings().GetUILocale() );
    }

    return rGlobalData.pResMgr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

String ImpGetResStr(sal_uInt16 nResID)
{
    return String(ResId(nResID, *ImpGetResMgr()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace sdr
{
String GetResourceString(sal_uInt16 nResID)
{
    return ImpGetResStr( nResID );
}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SearchOutlinerItems(const SfxItemSet& rSet, sal_Bool bInklDefaults, sal_Bool* pbOnlyEE)
{
    sal_Bool bHas=sal_False;
    sal_Bool bOnly=sal_True;
    sal_Bool bLookOnly=pbOnlyEE!=NULL;
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich=aIter.FirstWhich();
    while (((bLookOnly && bOnly) || !bHas) && nWhich!=0) {
        // For bInklDefaults, the entire Which range is decisive,
        // in other cases only the set items are.
        // Disabled and DontCare are regarded as holes in the Which range.
        SfxItemState eState=rSet.GetItemState(nWhich);
        if ((eState==SFX_ITEM_DEFAULT && bInklDefaults) || eState==SFX_ITEM_SET) {
            if (nWhich<EE_ITEMS_START || nWhich>EE_ITEMS_END) bOnly=sal_False;
            else bHas=sal_True;
        }
        nWhich=aIter.NextWhich();
    }
    if (!bHas) bOnly=sal_False;
    if (pbOnlyEE!=NULL) *pbOnlyEE=bOnly;
    return bHas;
}

sal_uInt16* RemoveWhichRange(const sal_uInt16* pOldWhichTable, sal_uInt16 nRangeBeg, sal_uInt16 nRangeEnd)
{
    // Six possible cases (per range):
    //         [Beg..End]          Range, to delete
    // [b..e]    [b..e]    [b..e]  Cases 1,3,2: doesn't matter, delete, doesn't matter  + Ranges
    // [b........e]  [b........e]  Cases 4,5  : shrink range                            | in
    // [b......................e]  Case  6    : splitting                               + pOldWhichTable
    sal_uInt16 nAnz=0;
    while (pOldWhichTable[nAnz]!=0) nAnz++;
    nAnz++; // nAnz should now be an odd number (0 for end of array)
    DBG_ASSERT((nAnz&1)==1,"RemoveWhichRange: WhichTable doesn't have an odd number of entries.");
    sal_uInt16 nAlloc=nAnz;
    // check necessary size of new array
    sal_uInt16 nNum=nAnz-1;
    while (nNum!=0) {
        nNum-=2;
        sal_uInt16 nBeg=pOldWhichTable[nNum];
        sal_uInt16 nEnd=pOldWhichTable[nNum+1];
        if (nEnd<nRangeBeg)  /*nCase=1*/ ;
        else if (nBeg>nRangeEnd) /* nCase=2 */ ;
        else if (nBeg>=nRangeBeg && nEnd<=nRangeEnd) /* nCase=3 */ nAlloc-=2;
        else if (nEnd<=nRangeEnd) /* nCase=4 */;
        else if (nBeg>=nRangeBeg) /* nCase=5*/ ;
        else /* nCase=6 */ nAlloc+=2;
    }

    sal_uInt16* pNewWhichTable=new sal_uInt16[nAlloc];
    memcpy(pNewWhichTable,pOldWhichTable,nAlloc*sizeof(sal_uInt16));
    pNewWhichTable[nAlloc-1]=0; // in case 3, there's no 0 at the end.
    // now remove the unwanted ranges
    nNum=nAlloc-1;
    while (nNum!=0) {
        nNum-=2;
        sal_uInt16 nBeg=pNewWhichTable[nNum];
        sal_uInt16 nEnd=pNewWhichTable[nNum+1];
        unsigned nCase=0;
        if (nEnd<nRangeBeg) nCase=1;
        else if (nBeg>nRangeEnd) nCase=2;
        else if (nBeg>=nRangeBeg && nEnd<=nRangeEnd) nCase=3;
        else if (nEnd<=nRangeEnd) nCase=4;
        else if (nBeg>=nRangeBeg) nCase=5;
        else nCase=6;
        switch (nCase) {
            case 3: {
                unsigned nTailBytes=(nAnz-(nNum+2))*sizeof(sal_uInt16);
                memcpy(&pNewWhichTable[nNum],&pNewWhichTable[nNum+2],nTailBytes);
                nAnz-=2; // remember: array is now smaller
            } break;
            case 4: pNewWhichTable[nNum+1]=nRangeBeg-1; break;
            case 5: pNewWhichTable[nNum]=nRangeEnd+1;     break;
            case 6: {
                unsigned nTailBytes=(nAnz-(nNum+2))*sizeof(sal_uInt16);
                memcpy(&pNewWhichTable[nNum+4],&pNewWhichTable[nNum+2],nTailBytes);
                nAnz+=2; // remember:array is now larger
                pNewWhichTable[nNum+2]=nRangeEnd+1;
                pNewWhichTable[nNum+3]=pNewWhichTable[nNum+1];
                pNewWhichTable[nNum+1]=nRangeBeg-1;
            } break;
        } // switch
    }
    return pNewWhichTable;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SvdProgressInfo::SvdProgressInfo( Link *_pLink )
{
    DBG_ASSERT(_pLink!=NULL,"SvdProgressInfo(): No Link stated!");

    pLink = _pLink;
    nSumActionCount = 0;
    nSumCurAction   = 0;

    nObjCount = 0;
    nCurObj   = 0;

    nActionCount = 0;
    nCurAction   = 0;

    nInsertCount = 0;
    nCurInsert   = 0;
}

void SvdProgressInfo::Init( sal_uIntPtr _nSumActionCount, sal_uIntPtr _nObjCount )
{
    nSumActionCount = _nSumActionCount;
    nObjCount = _nObjCount;
}

sal_Bool SvdProgressInfo::ReportActions( sal_uIntPtr nAnzActions )
{
    nSumCurAction += nAnzActions;
    nCurAction += nAnzActions;
    if(nCurAction > nActionCount)
        nCurAction = nActionCount;

    return pLink->Call(NULL) == 1L;
}

sal_Bool SvdProgressInfo::ReportInserts( sal_uIntPtr nAnzInserts )
{
    nSumCurAction += nAnzInserts;
    nCurInsert += nAnzInserts;

    return pLink->Call(NULL) == 1L;
}

sal_Bool SvdProgressInfo::ReportRescales( sal_uIntPtr nAnzRescales )
{
    nSumCurAction += nAnzRescales;
    return pLink->Call(NULL) == 1L;
}

void SvdProgressInfo::SetActionCount( sal_uIntPtr _nActionCount )
{
    nActionCount = _nActionCount;
}

void SvdProgressInfo::SetInsertCount( sal_uIntPtr _nInsertCount )
{
    nInsertCount = _nInsertCount;
}

sal_Bool SvdProgressInfo::SetNextObject()
{
    nActionCount = 0;
    nCurAction   = 0;

    nInsertCount = 0;
    nCurInsert   = 0;

    nCurObj++;
    return ReportActions(0);
}

void SvdProgressInfo::ReportError()
{
    pLink->Call((void *)1L);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #i101872# isolate GetTextEditBackgroundColor to tooling; it will anyways only be used as long
// as text edit is not running on overlay

namespace
{
    bool impGetSdrObjListFillColor(
        const SdrObjList& rList,
        const Point& rPnt,
        const SdrPageView& rTextEditPV,
        const SetOfByte& rVisLayers,
        Color& rCol)
    {
        if(!rList.GetModel())
            return false;

        bool bRet(false);
        bool bMaster(rList.GetPage() ? rList.GetPage()->IsMasterPage() : false);

        for(sal_uIntPtr no(rList.GetObjCount()); !bRet && no > 0; )
        {
            no--;
            SdrObject* pObj = rList.GetObj(no);
            SdrObjList* pOL = pObj->GetSubList();

            if(pOL)
            {
                // group object
                bRet = impGetSdrObjListFillColor(*pOL, rPnt, rTextEditPV, rVisLayers, rCol);
            }
            else
            {
                SdrTextObj* pText = dynamic_cast< SdrTextObj * >(pObj);

                // Exclude zero master page object (i.e. background shape) from color query
                if(pText
                    && pObj->IsClosedObj()
                    && (!bMaster || (!pObj->IsNotVisibleAsMaster() && 0 != no))
                    && pObj->GetCurrentBoundRect().IsInside(rPnt)
                    && !pText->IsHideContour()
                    && SdrObjectPrimitiveHit(*pObj, rPnt, 0, rTextEditPV, &rVisLayers, false))
                {
                    bRet = GetDraftFillColor(pObj->GetMergedItemSet(), rCol);
                }
            }
        }

        return bRet;
    }

    bool impGetSdrPageFillColor(
        const SdrPage& rPage,
        const Point& rPnt,
        const SdrPageView& rTextEditPV,
        const SetOfByte& rVisLayers,
        Color& rCol,
        bool bSkipBackgroundShape)
    {
        if(!rPage.GetModel())
            return false;

        bool bRet(impGetSdrObjListFillColor(rPage, rPnt, rTextEditPV, rVisLayers, rCol));

        if(!bRet && !rPage.IsMasterPage())
        {
            if(rPage.TRG_HasMasterPage())
            {
                SetOfByte aSet(rVisLayers);
                aSet &= rPage.TRG_GetMasterPageVisibleLayers();
                SdrPage& rMasterPage = rPage.TRG_GetMasterPage();

                // Don't fall back to background shape on
                // master pages. This is later handled by
                // GetBackgroundColor, and is necessary to cater for
                // the silly ordering: 1. shapes, 2. master page
                // shapes, 3. page background, 4. master page
                // background.
                bRet = impGetSdrPageFillColor(rMasterPage, rPnt, rTextEditPV, aSet, rCol, true);
            }
        }

        // Only now determine background color from background shapes
        if(!bRet && !bSkipBackgroundShape)
        {
            rCol = rPage.GetPageBackgroundColor();
            return true;
        }

        return bRet;
    }

    Color impCalcBackgroundColor(
        const Rectangle& rArea,
        const SdrPageView& rTextEditPV,
        const SdrPage& rPage)
    {
        svtools::ColorConfig aColorConfig;
        Color aBackground(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);
        const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        if(!rStyleSettings.GetHighContrastMode())
        {
            // search in page
            const sal_uInt16 SPOTCOUNT(5);
            Point aSpotPos[SPOTCOUNT];
            Color aSpotColor[SPOTCOUNT];
            sal_uIntPtr nHeight( rArea.GetSize().Height() );
            sal_uIntPtr nWidth( rArea.GetSize().Width() );
            sal_uIntPtr nWidth14  = nWidth / 4;
            sal_uIntPtr nHeight14 = nHeight / 4;
            sal_uIntPtr nWidth34  = ( 3 * nWidth ) / 4;
            sal_uIntPtr nHeight34 = ( 3 * nHeight ) / 4;

            sal_uInt16 i;
            for ( i = 0; i < SPOTCOUNT; i++ )
            {
                // five spots are used
                switch ( i )
                {
                    case 0 :
                    {
                        // Center-Spot
                        aSpotPos[i] = rArea.Center();
                    }
                    break;

                    case 1 :
                    {
                        // TopLeft-Spot
                        aSpotPos[i] = rArea.TopLeft();
                        aSpotPos[i].X() += nWidth14;
                        aSpotPos[i].Y() += nHeight14;
                    }
                    break;

                    case 2 :
                    {
                        // TopRight-Spot
                        aSpotPos[i] = rArea.TopLeft();
                        aSpotPos[i].X() += nWidth34;
                        aSpotPos[i].Y() += nHeight14;
                    }
                    break;

                    case 3 :
                    {
                        // BottomLeft-Spot
                        aSpotPos[i] = rArea.TopLeft();
                        aSpotPos[i].X() += nWidth14;
                        aSpotPos[i].Y() += nHeight34;
                    }
                    break;

                    case 4 :
                    {
                        // BottomRight-Spot
                        aSpotPos[i] = rArea.TopLeft();
                        aSpotPos[i].X() += nWidth34;
                        aSpotPos[i].Y() += nHeight34;
                    }
                    break;

                }

                aSpotColor[i] = Color( COL_WHITE );
                impGetSdrPageFillColor(rPage, aSpotPos[i], rTextEditPV, rTextEditPV.GetVisibleLayers(), aSpotColor[i], false);
            }

            sal_uInt16 aMatch[SPOTCOUNT];

            for ( i = 0; i < SPOTCOUNT; i++ )
            {
                // were same spot colors found?
                aMatch[i] = 0;

                for ( sal_uInt16 j = 0; j < SPOTCOUNT; j++ )
                {
                    if( j != i )
                    {
                        if( aSpotColor[i] == aSpotColor[j] )
                        {
                            aMatch[i]++;
                        }
                    }
                }
            }

            // highest weight to center spot
            aBackground = aSpotColor[0];

            for ( sal_uInt16 nMatchCount = SPOTCOUNT - 1; nMatchCount > 1; nMatchCount-- )
            {
                // which spot color was found most?
                for ( i = 0; i < SPOTCOUNT; i++ )
                {
                    if( aMatch[i] == nMatchCount )
                    {
                        aBackground = aSpotColor[i];
                        nMatchCount = 1;   // break outer for-loop
                        break;
                    }
                }
            }
        }

        return aBackground;
    }
} // end of anonymous namespace

Color GetTextEditBackgroundColor(const SdrObjEditView& rView)
{
    svtools::ColorConfig aColorConfig;
    Color aBackground(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if(!rStyleSettings.GetHighContrastMode())
    {
        bool bFound(false);
        SdrTextObj* pText = dynamic_cast< SdrTextObj * >(rView.GetTextEditObject());

        if(pText && pText->IsClosedObj())
        {
            ::sdr::table::SdrTableObj* pTable = dynamic_cast< ::sdr::table::SdrTableObj * >( pText );

            if( pTable )
                bFound = GetDraftFillColor(pTable->GetActiveCellItemSet(), aBackground );

            if( !bFound )
                bFound=GetDraftFillColor(pText->GetMergedItemSet(), aBackground);
        }

        if(!bFound && pText)
        {
            SdrPageView* pTextEditPV = rView.GetTextEditPageView();

            if(pTextEditPV)
            {
                Point aPvOfs(pText->GetTextEditOffset());
                const SdrPage* pPg = pTextEditPV->GetPage();

                if(pPg)
                {
                    Rectangle aSnapRect( pText->GetSnapRect() );
                    aSnapRect.Move(aPvOfs.X(), aPvOfs.Y());

                    return impCalcBackgroundColor(aSnapRect, *pTextEditPV, *pPg);
                }
            }
        }
    }

    return aBackground;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
