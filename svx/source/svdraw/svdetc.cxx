/*************************************************************************
 *
 *  $RCSfile: svdetc.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: cl $ $Date: 2001-04-04 16:00:49 $
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

#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include "forbiddencharacterstable.hxx"
#endif

#include "svdetc.hxx"
#include "svditext.hxx"
#include "svdmodel.hxx"
#include "svdtrans.hxx"
#include "svdglob.hxx"
#include "svdstr.hrc"
#include "svdviter.hxx"
#include "svdview.hxx"
#include "svdoutl.hxx"

#ifndef _SV_EXCHANGE_HXX //autogen
#include <vcl/exchange.hxx>
#endif

#ifndef _SV_BMPACC_HXX //autogen
#include <vcl/bmpacc.hxx>
#endif

#ifndef _EEITEM_HXX //autogen
#include <eeitem.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _SV_CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif

#ifndef _SVX_FONTITEM_HXX //autogen
#include "fontitem.hxx"
#endif

#ifndef _SVX_COLRITEM_HXX //autogen
#include "colritem.hxx"
#endif

#ifndef _SVX_FHGTITEM_HXX //autogen
#include "fhgtitem.hxx"
#endif

#ifndef _SVX__XGRADIENT_HXX //autogen
#include "xgrad.hxx"
#endif

#ifndef SVX_XFILLIT0_HXX //autogen
#include "xfillit0.hxx"
#endif

#ifndef _SVX_XFLCLIT_HXX //autogen
#include "xflclit.hxx"
#endif

#ifndef _SVX_XFLHTIT_HXX //autogen
#include "xflhtit.hxx"
#endif

#ifndef _SVX_XBTMPIT_HXX //autogen
#include "xbtmpit.hxx"
#endif

#ifndef _SVX_XFLGRIT_HXX //autogen
#include "xflgrit.hxx"
#endif

#ifndef _SVDOOLE2_HXX //autogen
#include "svdoole2.hxx"
#endif

#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif



/******************************************************************************
* Globale Daten der DrawingEngine
******************************************************************************/
::com::sun::star::lang::Locale*     SdrGlobalData::pLocale = NULL;
CharClass*  SdrGlobalData::pCharClass = NULL;
LocaleDataWrapper* SdrGlobalData::pLocaleData = NULL;


SdrGlobalData::SdrGlobalData() :
    pOutliner(NULL),
    pDefaults(NULL),
    pResMgr(NULL),
    pStrCache(NULL),
    nExchangeFormat(0)
{
    String aLanguage, aCountry;
    ConvertLanguageToIsoNames( International::GetRealLanguage( LANGUAGE_SYSTEM ), aLanguage, aCountry );
    pLocale = new ::com::sun::star::lang::Locale( aLanguage, aCountry, String() );
    pCharClass = new CharClass( ::comphelper::getProcessServiceFactory(), *pLocale );
    pLocaleData = new LocaleDataWrapper( ::comphelper::getProcessServiceFactory(), *pLocale );
}

SdrGlobalData::~SdrGlobalData()
{
    delete pOutliner;
    delete pDefaults;
    delete pResMgr;
    delete [] pStrCache;
    DELETEZ( pLocaleData );
    DELETEZ( pCharClass );
    DELETEZ( pLocale );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

OLEObjCache::OLEObjCache()
:   Container( 0 )
{
    nSize = 20;
/* !!! IniManager
    SfxIniManager* pIniMgr = SfxIniManager::Get();

    if(pIniMgr)
    {
        sal_Char aTextOLECacheNum[] = "MaxOLEObjectsInDrawingEngineMemory";
        String aKeyName = UniString(aTextOLECacheNum, sizeof(aTextOLECacheNum-1));
        String aGroupName(pIniMgr->GetGroupName(SFX_GROUP_WORKINGSET_IMPL));

        if(!pIniMgr->ReadKey(aGroupName, aKeyName).Len())
        {
            pIniMgr->WriteKey(aGroupName, aKeyName, UniString::CreateFromInt32(nSize));
        }
        else
        {
            String aIniManagerString = pIniMgr->Get(SFX_GROUP_WORKINGSET_IMPL, aKeyName);
            nSize = aIniManagerString.ToInt32();

            if(nSize < 5)
                nSize = 5;
        }
    }
*/
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
    // Kein Unload notwendig, da zu diesem Zeitpunkt
    // die Objekte nicht mehr vorhanden sind
}

void OLEObjCache::SetSize(ULONG nNewSize)
{
    nSize = nNewSize;
}

void OLEObjCache::InsertObj(SdrOle2Obj* pObj)
{
    if (nSize <= Count())
    {
        // Eintraege reduzieren
        ULONG nIndex = Count() - 1;

        for (ULONG i = nIndex; i + 1 >= nSize; i--)
        {
            // Pruefen, ob Objekte entfernt werden koennen
            SdrOle2Obj* pCacheObj = (SdrOle2Obj*) GetObject(i);

            if ( pCacheObj != pObj &&  UnloadObj(pCacheObj) )
            {
                 Remove(i);
            }
        }
    }

    // Objekt ggf. entfernen und an erster Position einfuegen
    Remove(pObj);
    Insert(pObj, (ULONG) 0L);
}

void OLEObjCache::RemoveObj(SdrOle2Obj* pObj)
{
    UnloadObj( (SdrOle2Obj*) Remove(pObj) );
}

BOOL OLEObjCache::UnloadObj(SdrOle2Obj* pObj)
{
    BOOL bUnloaded = FALSE;

    if (pObj)
    {
        BOOL bVisible = FALSE;
          SdrViewIter aIter(pObj);
        SdrView* pView = aIter.FirstView();

        while (!bVisible && pView!=NULL)
        {
            bVisible = !pView->IsGrafDraft();

            if (!bVisible)
                pView = aIter.NextView();
        }

        if (!bVisible)
            bUnloaded = pObj->Unload();
    }

    return bUnloaded;
}

IMPL_LINK(OLEObjCache, UnloadCheckHdl, AutoTimer*, pTim)
{
    if (nSize <= Count())
    {
        // Eintraege reduzieren
        ULONG nIndex = Count() - 1;

        for (ULONG i = nIndex; i + 1 >= nSize; i--)
        {
            // Pruefen, ob Objekte entfernt werden koennen
            SdrOle2Obj* pCacheObj = (SdrOle2Obj*) GetObject(i);

            if ( UnloadObj(pCacheObj) )
                Remove(i);
        }
    }

    return 0;
}



void ContainerSorter::DoSort(ULONG a, ULONG b) const
{
    ULONG nAnz=rCont.Count();
    if (b>nAnz) b=nAnz;
    if (b>0) b--;
    if (a<b) ImpSubSort(a,b);
}

void ContainerSorter::Is1stLessThan2nd(const void* pElem1, const void* pElem2) const
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
    FASTBOOL   bClip;
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

ImpSdrHdcMerk::ImpSdrHdcMerk(const OutputDevice& rOut, USHORT nNewMode, FASTBOOL bAutoMerk):
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
            DBG_ERROR("SdrLinkList::InsertLink(): Versuch, einen nicht gesetzten Link einzufuegen");
        }
    } else {
        DBG_ERROR("SdrLinkList::InsertLink(): Link schon vorhanden");
    }
}

void SdrLinkList::RemoveLink(const Link& rLink)
{
    unsigned nFnd=FindEntry(rLink);
    if (nFnd!=0xFFFF) {
        Link* pLink=(Link*)aList.Remove(nFnd);
        delete pLink;
    } else {
        DBG_ERROR("SdrLinkList::RemoveLink(): Link nicht gefunden");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FASTBOOL GetDraftFillColor(const SfxItemSet& rSet, Color& rCol)
{
    XFillStyle eFill=((XFillStyleItem&)rSet.Get(XATTR_FILLSTYLE)).GetValue();
    switch (eFill) {
        case XFILL_SOLID: rCol=((XFillColorItem&)rSet.Get(XATTR_FILLCOLOR)).GetValue(); return TRUE;
        case XFILL_HATCH: {
            Color aCol1(((XFillHatchItem&)rSet.Get(XATTR_FILLHATCH)).GetValue().GetColor());
            Color aCol2(COL_WHITE);

            // What the hell is this old color mixing try from JOE? It's bloody WRONG
            // USHORT nRt=(USHORT)(((ULONG)aCol1.GetRed  ()+(ULONG)aCol2.GetRed  ())/2);
            // USHORT nGn=(USHORT)(((ULONG)aCol1.GetGreen()+(ULONG)aCol2.GetGreen())/2);
            // USHORT nBl=(USHORT)(((ULONG)aCol1.GetBlue ()+(ULONG)aCol2.GetBlue ())/2);

            UINT8 nRt = (UINT8)(((aCol1.GetRed() + 1)>>1) + (aCol2.GetRed()>>1));
            UINT8 nGn = (UINT8)(((aCol1.GetGreen() + 1)>>1) + (aCol2.GetGreen()>>1));
            UINT8 nBl = (UINT8)(((aCol1.GetBlue() + 1)>>1) + (aCol2.GetBlue()>>1));

            rCol=Color(nRt,nGn,nBl);
            return TRUE;
        }
        case XFILL_GRADIENT: {
            const XGradient& rGrad=((XFillGradientItem&)rSet.Get(XATTR_FILLGRADIENT)).GetValue();
            Color aCol1(rGrad.GetStartColor());
            Color aCol2(rGrad.GetEndColor());

            // Same here, look above.
            UINT8 nRt = (UINT8)(((aCol1.GetRed() + 1)>>1) + (aCol2.GetRed()>>1));
            UINT8 nGn = (UINT8)(((aCol1.GetGreen() + 1)>>1) + (aCol2.GetGreen()>>1));
            UINT8 nBl = (UINT8)(((aCol1.GetBlue() + 1)>>1) + (aCol2.GetBlue()>>1));

            rCol=Color(nRt,nGn,nBl);
            return TRUE;
        }
        case XFILL_BITMAP: {
            const Bitmap& rBmp=((XFillBitmapItem&)rSet.Get(XATTR_FILLBITMAP)).GetValue().GetBitmap();

            Size aSiz(rBmp.GetSizePixel());
            if (aSiz.Width()>0 && aSiz.Height()>0) {
                if (aSiz.Width ()>8) aSiz.Width ()=8;
                if (aSiz.Height()>8) aSiz.Height()=8;

                ULONG nRt=0,nGn=0,nBl=0;

                // in VCL koennen wir die Pixel von der Bitmap holen;
                // alles wird gut... (KA 21.09.97)
                Bitmap              aBmp( rBmp );
                BitmapReadAccess*   pAcc = aBmp.AcquireReadAccess();

                if( pAcc )
                {
                    const long nWidth = aSiz.Width();
                    const long nHeight = aSiz.Height();

                    if( pAcc->HasPalette() )
                    {
                        for( long nY = 0L; nY < nHeight; nY++ )
                        {
                            for( long nX = 0L; nX < nWidth; nX++ )
                            {
                                const BitmapColor& rCol = pAcc->GetPaletteColor( (BYTE) pAcc->GetPixel( nY, nX ) );
                                nRt+=rCol.GetRed(); nGn+=rCol.GetGreen(); nBl+=rCol.GetBlue();

                            }
                        }
                    }
                    else
                    {
                        for( long nY = 0L; nY < nHeight; nY++ )
                        {
                            for( long nX = 0L; nX < nWidth; nX++ )
                            {
                                const BitmapColor aCol( pAcc->GetPixel( nY, nX ) );
                                nRt+=aCol.GetRed(); nGn+=aCol.GetGreen(); nBl+=aCol.GetBlue();

                            }
                        }
                    }

                    aBmp.ReleaseAccess( pAcc );
                }

                unsigned nAnz=unsigned(aSiz.Width()*aSiz.Height());

                // same problem here (see above), also no cropping, but good enough for
                // now (was good enough since Jul 1998)
                nRt/=nAnz;
                nGn/=nAnz;
                nBl/=nAnz;

                rCol=Color(UINT8(nRt),UINT8(nGn),UINT8(nBl));
                return TRUE;
            }
        }
    } // switch
    return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrEngineDefaults::SdrEngineDefaults():
    aFontName(System::GetStandardFont(STDFONT_ROMAN).GetName()),
    eFontFamily(FAMILY_ROMAN),
    aFontColor(COL_BLACK),
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
    if (rGlobalData.pStrCache!=NULL) {
        delete [] rGlobalData.pStrCache;
        rGlobalData.pStrCache=NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrOutliner* SdrMakeOutliner( USHORT nOutlinerMode, SdrModel* pModel )
{
    SdrEngineDefaults& rDefaults = SdrEngineDefaults::GetDefaults();

/*
    MapUnit  eUn( (pMod==NULL) ? rDefaults.eMapUnit : pMod->GetScaleUnit());
    Fraction aFr( *((pMod==NULL) ? &rDefaults.aMapFraction : &pMod->GetScaleFraction()));

    if ( pMod->GetRefDevice() )
        pOutl->SetRefDevice( pMod->GetRefDevice() );
    else
    {
        MapMode aMapMode(eUn,Point(0,0),aFr,aFr);
        pOutl->SetRefMapMode( aMapMode );
    }

    SfxItemSet aSet(pOutl->GetEmptyItemSet());
    aSet.Put(SvxFontItem(rDefaults.eFontFamily, rDefaults.aFontName, String(), PITCH_DONTKNOW, gsl_getSystemTextEncoding() ) );
    aSet.Put(SvxColorItem(rDefaults.aFontColor));
    ULONG nHgt=rDefaults.nFontHeight;
    FASTBOOL bDifUn=(eUn!=rDefaults.eMapUnit); // different MapUnits
    FASTBOOL bDifFr=(aFr!=rDefaults.aMapFraction); // different MapFractions
    if (bDifUn || bDifFr) { // Wenn pMod!=NULL und pMod->Map!=rDef.Map
        long nTmpLong=long(nHgt); // caasting im Ctor bringt unter MSVC sehr merkwuerdige Fehlermeldungen
        BigInt aHgt1(nTmpLong); // umrechnen von DefMap in ModMap
        FrPair aUnitMul(GetMapFactor(rDefaults.eMapUnit,eUn));

        if (bDifUn) aHgt1*=aUnitMul.Y().GetNumerator();
        if (bDifFr) aHgt1*=aFr.GetNumerator();
        if (bDifFr) aHgt1*=rDefaults.aMapFraction.GetDenominator();
        if (bDifUn) aHgt1/=aUnitMul.Y().GetDenominator();
        if (bDifFr) aHgt1/=aFr.GetDenominator();
        if (bDifFr) aHgt1/=rDefaults.aMapFraction.GetNumerator();

        nHgt=ULONG(long(aHgt1));
    }
    aSet.Put(SvxFontHeightItem(nHgt));
    pOutl->SetDefaults(aSet);
*/

    SfxItemPool* pPool = &pModel->GetItemPool();
    SdrOutliner* pOutl = new SdrOutliner( pPool, nOutlinerMode );
    pOutl->SetEditTextObjectPool( pPool );
    pOutl->SetStyleSheetPool( (SfxStyleSheetPool*) pModel->GetStyleSheetPool() );
    pOutl->SetDefTab( pModel->GetDefaultTabulator() );
    pOutl->SetForbiddenCharsTable( pModel->GetForbiddenCharsTable() );

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
#ifndef SVX_LIGHT
        ByteString aName("svx");
#else
        ByteString aName("svl");
#endif
        INT32 nSolarUpd(SOLARUPD);
        aName += ByteString::CreateFromInt32( nSolarUpd );
        rGlobalData.pResMgr =
            ResMgr::CreateResMgr( aName.GetBuffer(), GetpApp()->GetAppInternational().GetLanguage() );
    }

    return rGlobalData.pResMgr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const XubString& ImpGetResStr(USHORT nResID)
{
    SdrGlobalData& rGlobalData=GetSdrGlobalData();
    if (rGlobalData.pStrCache==NULL) {
        USHORT nAnz=SDR_StringCacheEnd-SDR_StringCacheBegin+1;
        rGlobalData.pStrCache=new XubString[nAnz];
        XubString* pStr=rGlobalData.pStrCache;
        ResMgr* pResMgr=ImpGetResMgr();
        for (USHORT i=0; i<nAnz; i++) {
            USHORT nResNum=SDR_StringCacheBegin+i;
            {
                pStr[i]=XubString(ResId(nResNum,pResMgr));
            }
        }
    }
    if (nResID>=SDR_StringCacheBegin && nResID<=SDR_StringCacheEnd) {
        return rGlobalData.pStrCache[nResID-SDR_StringCacheBegin];
    } else {
#ifdef DBG_UTIL
        DBG_ERROR("ImpGetResStr(): ResourceID outside of cache range!");
#endif
        return String();
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



