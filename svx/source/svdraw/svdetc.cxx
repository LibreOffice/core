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
#include "precompiled_svx.hxx"
#include "editeng/forbiddencharacterstable.hxx"
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <svx/svdetc.hxx>
#include "svditext.hxx"
#include <svx/svdmodel.hxx>
#include <svx/svdtrans.hxx>
#include "svdglob.hxx"
#include "svdstr.hrc"
#include "svdviter.hxx"
#include <svx/svdview.hxx>
#include <svx/svdoutl.hxx>
#include <vcl/bmpacc.hxx>
#include <editeng/eeitem.hxx>
#include <svl/itemset.hxx>
#include <tools/config.hxx>
#include <unotools/cacheoptions.hxx>
#include <svl/whiter.hxx>
#include <tools/bigint.hxx>
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
#include <comphelper/processfactory.hxx>
#include <i18npool/lang.h>
#include <unotools/charclass.hxx>
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
* Globale Daten der DrawingEngine
******************************************************************************/

SdrGlobalData::SdrGlobalData() :
    pSysLocale(NULL),
    pCharClass(NULL),
    pLocaleData(NULL),
    pOutliner(NULL),
    pDefaults(NULL),
    pResMgr(NULL),
    nExchangeFormat(0)
{
    //pSysLocale = new SvtSysLocale;
    //pCharClass = pSysLocale->GetCharClassPtr();
    //pLocaleData = pSysLocale->GetLocaleDataPtr();

    svx::ExtrusionBar::RegisterInterface();
    svx::FontworkBar::RegisterInterface();
}

SdrGlobalData::~SdrGlobalData()
{
    delete pOutliner;
    delete pDefaults;
    delete pResMgr;
    //! do NOT delete pCharClass and pLocaleData
    delete pSysLocale;
}
const SvtSysLocale*         SdrGlobalData::GetSysLocale()
{
    if ( !pSysLocale )
        pSysLocale = new SvtSysLocale;
    return pSysLocale;
}
const CharClass*            SdrGlobalData::GetCharClass()
{
    if ( !pCharClass )
        pCharClass = GetSysLocale()->GetCharClassPtr();
    return pCharClass;
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
    SvtCacheOptions aCacheOptions;

    nSize = aCacheOptions.GetDrawingEngineOLE_Objects();
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
        ULONG nCount2 = Count();
        ULONG nIndex = nCount2-1;
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
                            for ( ULONG nCheckInd = 0; nCheckInd < Count(); nCheckInd++ )
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

void OLEObjCache::SetSize(ULONG nNewSize)
{
    nSize = nNewSize;
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
    ULONG nOldPos = GetPos( pObj );

    // insert object into first position
    Remove( nOldPos );
    Insert(pObj, (ULONG) 0L);

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

BOOL OLEObjCache::UnloadObj(SdrOle2Obj* pObj)
{
    BOOL bUnloaded = FALSE;
    if (pObj)
    {
        //#i80528# The old mechanism is completely useless, only taking into account if
        // in all views the GrafDraft feature is used. This will nearly never have been the
        // case since no one ever used this option.
        //
        // A much better (and working) criteria would be the VOC contact count.
        // The quesion is what will happen whe i make it work now suddenly? I
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

void ContainerSorter::DoSort(ULONG a, ULONG b) const
{
    ULONG nAnz=rCont.Count();
    if (b>nAnz) b=nAnz;
    if (b>0) b--;
    if (a<b) ImpSubSort(a,b);
}

void ContainerSorter::Is1stLessThan2nd(const void* /*pElem1*/, const void* /*pElem2*/) const
{
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
    virtual int Compare(const void* pElem1, const void* pElem2) const;
};

int ImpUShortContainerSorter::Compare(const void* pElem1, const void* pElem2) const
{
    USHORT n1=USHORT(ULONG(pElem1));
    USHORT n2=USHORT(ULONG(pElem2));
    return n1<n2 ? -1 : n1>n2 ? 1 : 0;
}

void UShortCont::Sort()
{
    ImpUShortContainerSorter aSorter(aArr);
    aSorter.DoSort();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class ImpClipMerk {
    Region aClip;
    bool   bClip;
public:
    ImpClipMerk(const OutputDevice& rOut): aClip(rOut.GetClipRegion()),bClip(rOut.IsClipRegion()) {}
    void Restore(OutputDevice& rOut)
    {
        // Kein Clipping in die Metafileaufzeichnung
        GDIMetaFile* pMtf=rOut.GetConnectMetaFile();
        if (pMtf!=NULL && (!pMtf->IsRecord() || pMtf->IsPause())) pMtf=NULL;
        if (pMtf!=NULL) pMtf->Pause(TRUE);
        if (bClip) rOut.SetClipRegion(aClip);
        else rOut.SetClipRegion();
        if (pMtf!=NULL) pMtf->Pause(FALSE);
    }
};

class ImpColorMerk {
    Color aLineColor;
    Color aFillColor;
    Color aBckgrdColor;
    Font  aFont;
public:
    ImpColorMerk(const OutputDevice& rOut):
        aLineColor( rOut.GetLineColor() ),
        aFillColor( rOut.GetFillColor() ),
        aBckgrdColor( rOut.GetBackground().GetColor() ),
        aFont (rOut.GetFont()) {}

    ImpColorMerk(const OutputDevice& rOut, USHORT nMode)
    {
        if ( (nMode & SDRHDC_SAVEPEN) == SDRHDC_SAVEPEN )
            aLineColor = rOut.GetLineColor();

        if ( (nMode & SDRHDC_SAVEBRUSH) == SDRHDC_SAVEBRUSH)
        {
            aFillColor = rOut.GetFillColor();
            aBckgrdColor = rOut.GetBackground().GetColor();
        }

        if ( (nMode & SDRHDC_SAVEFONT) == SDRHDC_SAVEFONT)
            aFont=rOut.GetFont();
    }

    void Restore(OutputDevice& rOut, USHORT nMode=SDRHDC_SAVEPENANDBRUSHANDFONT)
    {
        if ( (nMode & SDRHDC_SAVEPEN) == SDRHDC_SAVEPEN)
            rOut.SetLineColor( aLineColor );

        if ( (nMode & SDRHDC_SAVEBRUSH) == SDRHDC_SAVEBRUSH)
        {
            rOut.SetFillColor( aFillColor );
            rOut.SetBackground( Wallpaper( aBckgrdColor ) );
        }
        if ((nMode & SDRHDC_SAVEFONT) ==SDRHDC_SAVEFONT)
        {
            if (!rOut.GetFont().IsSameInstance(aFont))
            {
                rOut.SetFont(aFont);
            }
        }
    }

    const Color& GetLineColor() const { return aLineColor; }
};

ImpSdrHdcMerk::ImpSdrHdcMerk(const OutputDevice& rOut, USHORT nNewMode, bool bAutoMerk):
    pFarbMerk(NULL),
    pClipMerk(NULL),
    pLineColorMerk(NULL),
    nMode(nNewMode)
{
    if (bAutoMerk) Save(rOut);
}

ImpSdrHdcMerk::~ImpSdrHdcMerk()
{
    if (pFarbMerk!=NULL) delete pFarbMerk;
    if (pClipMerk!=NULL) delete pClipMerk;
    if (pLineColorMerk !=NULL) delete pLineColorMerk;
}

void ImpSdrHdcMerk::Save(const OutputDevice& rOut)
{
    if (pFarbMerk!=NULL)
    {
        delete pFarbMerk;
        pFarbMerk=NULL;
    }
    if (pClipMerk!=NULL)
    {
        delete pClipMerk;
        pClipMerk=NULL;
    }
    if (pLineColorMerk !=NULL)
    {
        delete pLineColorMerk ;
        pLineColorMerk =NULL;
    }
    if ((nMode & SDRHDC_SAVECLIPPING) ==SDRHDC_SAVECLIPPING)
        pClipMerk=new ImpClipMerk(rOut);

    USHORT nCol=nMode & SDRHDC_SAVEPENANDBRUSHANDFONT;

    if (nCol==SDRHDC_SAVEPEN)
        pLineColorMerk=new Color( rOut.GetLineColor() );
    else if (nCol==SDRHDC_SAVEPENANDBRUSHANDFONT)
        pFarbMerk=new ImpColorMerk(rOut);
    else if (nCol!=0)
        pFarbMerk=new ImpColorMerk(rOut,nCol);
}

void ImpSdrHdcMerk::Restore(OutputDevice& rOut, USHORT nMask) const
{
    nMask&=nMode; // nur restaurieren, was auch gesichert wurde

    if ((nMask & SDRHDC_SAVECLIPPING) ==SDRHDC_SAVECLIPPING && pClipMerk!=NULL)
        pClipMerk->Restore(rOut);

    USHORT nCol=nMask & SDRHDC_SAVEPENANDBRUSHANDFONT;

    if (nCol==SDRHDC_SAVEPEN)
    {
        if (pLineColorMerk!=NULL)
            rOut.SetLineColor(*pLineColorMerk);
        else if (pFarbMerk!=NULL)
            rOut.SetLineColor( pFarbMerk->GetLineColor() );
    } else if (nCol!=0 && pFarbMerk!=NULL)
        pFarbMerk->Restore(rOut,nCol);
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
            OSL_FAIL("SdrLinkList::InsertLink(): Versuch, einen nicht gesetzten Link einzufuegen");
        }
    } else {
        OSL_FAIL("SdrLinkList::InsertLink(): Link schon vorhanden");
    }
}

void SdrLinkList::RemoveLink(const Link& rLink)
{
    unsigned nFnd=FindEntry(rLink);
    if (nFnd!=0xFFFF) {
        Link* pLink=(Link*)aList.Remove(nFnd);
        delete pLink;
    } else {
        OSL_FAIL("SdrLinkList::RemoveLink(): Link nicht gefunden");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #98988# Re-implement GetDraftFillColor(...)

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

            // #97870# when hatch background is activated, use object fill color as hatch color
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
                            ? pAccess->GetPaletteColor((BYTE)pAccess->GetPixel(nY, nX))
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

                rCol = Color(UINT8(nRt), UINT8(nGn), UINT8(nBl));

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

void SdrEngineDefaults::LanguageHasChanged()
{
    SdrGlobalData& rGlobalData=GetSdrGlobalData();
    if (rGlobalData.pResMgr!=NULL) {
        delete rGlobalData.pResMgr;
        rGlobalData.pResMgr=NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrOutliner* SdrMakeOutliner( USHORT nOutlinerMode, SdrModel* pModel )
{
    //SdrEngineDefaults& rDefaults = SdrEngineDefaults::GetDefaults();

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

BOOL SearchOutlinerItems(const SfxItemSet& rSet, BOOL bInklDefaults, BOOL* pbOnlyEE)
{
    BOOL bHas=FALSE;
    BOOL bOnly=TRUE;
    BOOL bLookOnly=pbOnlyEE!=NULL;
    SfxWhichIter aIter(rSet);
    USHORT nWhich=aIter.FirstWhich();
    while (((bLookOnly && bOnly) || !bHas) && nWhich!=0) {
        // bei bInklDefaults ist der gesamte Which-Range
        // ausschlaggebend, ansonsten nur die gesetzten Items
        // Disabled und DontCare wird als Loch im Which-Range betrachtet
        SfxItemState eState=rSet.GetItemState(nWhich);
        if ((eState==SFX_ITEM_DEFAULT && bInklDefaults) || eState==SFX_ITEM_SET) {
            if (nWhich<EE_ITEMS_START || nWhich>EE_ITEMS_END) bOnly=FALSE;
            else bHas=TRUE;
        }
        nWhich=aIter.NextWhich();
    }
    if (!bHas) bOnly=FALSE;
    if (pbOnlyEE!=NULL) *pbOnlyEE=bOnly;
    return bHas;
}

USHORT* RemoveWhichRange(const USHORT* pOldWhichTable, USHORT nRangeBeg, USHORT nRangeEnd)
{
    // insgesamt sind 6 Faelle moeglich (je Range):
    //         [Beg..End]          zu entfernender Range
    // [b..e]    [b..e]    [b..e]  Fall 1,3,2: egal, ganz weg, egal  + Ranges
    // [b........e]  [b........e]  Fall 4,5  : Bereich verkleinern   | in
    // [b......................e]  Fall 6    : Splitting             + pOldWhichTable
    USHORT nAnz=0;
    while (pOldWhichTable[nAnz]!=0) nAnz++;
    nAnz++; // nAnz muesste nun in jedem Fall eine ungerade Zahl sein (0 am Ende des Arrays)
    DBG_ASSERT((nAnz&1)==1,"Joe: RemoveWhichRange: WhichTable hat keine ungerade Anzahl von Eintraegen");
    USHORT nAlloc=nAnz;
    // benoetigte Groesse des neuen Arrays ermitteln
    USHORT nNum=nAnz-1;
    while (nNum!=0) {
        nNum-=2;
        USHORT nBeg=pOldWhichTable[nNum];
        USHORT nEnd=pOldWhichTable[nNum+1];
        if (nEnd<nRangeBeg)  /*nCase=1*/ ;
        else if (nBeg>nRangeEnd) /* nCase=2 */ ;
        else if (nBeg>=nRangeBeg && nEnd<=nRangeEnd) /* nCase=3 */ nAlloc-=2;
        else if (nEnd<=nRangeEnd) /* nCase=4 */;
        else if (nBeg>=nRangeBeg) /* nCase=5*/ ;
        else /* nCase=6 */ nAlloc+=2;
    }

    USHORT* pNewWhichTable=new USHORT[nAlloc];
    memcpy(pNewWhichTable,pOldWhichTable,nAlloc*sizeof(USHORT));
    pNewWhichTable[nAlloc-1]=0; // im Falle 3 fehlt die 0 am Ende
    // nun die unerwuenschten Ranges entfernen
    nNum=nAlloc-1;
    while (nNum!=0) {
        nNum-=2;
        USHORT nBeg=pNewWhichTable[nNum];
        USHORT nEnd=pNewWhichTable[nNum+1];
        unsigned nCase=0;
        if (nEnd<nRangeBeg) nCase=1;
        else if (nBeg>nRangeEnd) nCase=2;
        else if (nBeg>=nRangeBeg && nEnd<=nRangeEnd) nCase=3;
        else if (nEnd<=nRangeEnd) nCase=4;
        else if (nBeg>=nRangeBeg) nCase=5;
        else nCase=6;
        switch (nCase) {
            case 3: {
                unsigned nTailBytes=(nAnz-(nNum+2))*sizeof(USHORT);
                memcpy(&pNewWhichTable[nNum],&pNewWhichTable[nNum+2],nTailBytes);
                nAnz-=2; // Merken: Array hat sich verkleinert
            } break;
            case 4: pNewWhichTable[nNum+1]=nRangeBeg-1; break;
            case 5: pNewWhichTable[nNum]=nRangeEnd+1;     break;
            case 6: {
                unsigned nTailBytes=(nAnz-(nNum+2))*sizeof(USHORT);
                memcpy(&pNewWhichTable[nNum+4],&pNewWhichTable[nNum+2],nTailBytes);
                nAnz+=2; // Merken: Array hat sich vergroessert
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
    DBG_ASSERT(_pLink!=NULL,"SvdProgressInfo(): Kein Link angegeben!!");

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

void SvdProgressInfo::Init( ULONG _nSumActionCount, ULONG _nObjCount )
{
    nSumActionCount = _nSumActionCount;
    nObjCount = _nObjCount;
}

BOOL SvdProgressInfo::ReportActions( ULONG nAnzActions )
{
    nSumCurAction += nAnzActions;
    nCurAction += nAnzActions;
    if(nCurAction > nActionCount)
        nCurAction = nActionCount;

    return pLink->Call(NULL) == 1L;
}

BOOL SvdProgressInfo::ReportInserts( ULONG nAnzInserts )
{
    nSumCurAction += nAnzInserts;
    nCurInsert += nAnzInserts;

    return pLink->Call(NULL) == 1L;
}

BOOL SvdProgressInfo::ReportRescales( ULONG nAnzRescales )
{
    nSumCurAction += nAnzRescales;
    return pLink->Call(NULL) == 1L;
}

void SvdProgressInfo::SetActionCount( ULONG _nActionCount )
{
    nActionCount = _nActionCount;
}

void SvdProgressInfo::SetInsertCount( ULONG _nInsertCount )
{
    nInsertCount = _nInsertCount;
}

BOOL SvdProgressInfo::SetNextObject()
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
// #i101872# isolate GetTextEditBackgroundColor to tooling; it woll anyways only be used as long
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

        for(ULONG no(rList.GetObjCount()); !bRet && no > 0; )
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

                // #108867# Exclude zero master page object (i.e. background shape) from color query
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

                // #108867# Don't fall back to background shape on
                // master pages. This is later handled by
                // GetBackgroundColor, and is necessary to cater for
                // the silly ordering: 1. shapes, 2. master page
                // shapes, 3. page background, 4. master page
                // background.
                bRet = impGetSdrPageFillColor(rMasterPage, rPnt, rTextEditPV, aSet, rCol, true);
            }
        }

        // #108867# Only now determine background color from background shapes
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
            const USHORT SPOTCOUNT(5);
            Point aSpotPos[SPOTCOUNT];
            Color aSpotColor[SPOTCOUNT];
            ULONG nHeight( rArea.GetSize().Height() );
            ULONG nWidth( rArea.GetSize().Width() );
            ULONG nWidth14  = nWidth / 4;
            ULONG nHeight14 = nHeight / 4;
            ULONG nWidth34  = ( 3 * nWidth ) / 4;
            ULONG nHeight34 = ( 3 * nHeight ) / 4;

            USHORT i;
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

            USHORT aMatch[SPOTCOUNT];

            for ( i = 0; i < SPOTCOUNT; i++ )
            {
                // were same spot colors found?
                aMatch[i] = 0;

                for ( USHORT j = 0; j < SPOTCOUNT; j++ )
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

            for ( USHORT nMatchCount = SPOTCOUNT - 1; nMatchCount > 1; nMatchCount-- )
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
