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

#ifndef _SVDETC_HXX
#define _SVDETC_HXX

#include <tools/string.hxx>

#include <tools/contnr.hxx>

#include <vcl/outdev.hxx>

#include <tools/shl.hxx>

namespace com { namespace sun { namespace star { namespace lang {
    struct Locale;
}}}}
class CharClass;
class LocaleDataWrapper;
class AutoTimer;
class Link;
namespace binfilter {

class SfxItemSet;
class SvtSysLocale;

////////////////////////////////////////////////////////////////////////////////////////////////////

// ExchangeFormat-Id der DrawingEngine holen. Daten koennen dann per
//   static BOOL CopyData(pData,nLen,nFormat);
// bereitgestellt werden, wobei pData/nLen einen SvMemoryStream beschreiben in
// dem ein SdrModel gestreamt wird an dem fuer die Zeitdauer des Streamens das
// Flag SdrModel::SetStreamingSdrModel(TRUE) gesetzt wird.
// ULONG SdrGetExchangeFormat(); -- JP 18.01.99 - dafuer gibt es ein define

class SdrOutliner;
class SdrModel;

//STRIP008 namespace com { namespace sun { namespace star { namespace lang {
//STRIP008 	struct Locale;
//STRIP008 }}}}

// Globale Defaulteinstellungen fuer die DrawingEngine.
// Diese Einstellungen sollte man direkt beim Applikationsstart
// vornehmen, noch bevor andere Methoden der Engine gerufen werden.
class SdrEngineDefaults
{
friend class SdrAttrObj;
    String     aFontName;
    FontFamily eFontFamily;
    Color      aFontColor;
    ULONG      nFontHeight;
    MapUnit    eMapUnit;
    Fraction   aMapFraction;

private:
    static SdrEngineDefaults& GetDefaults();

public:
    SdrEngineDefaults();
    // Default Fontname ist "Times New Roman"
    static void       SetFontName(const String& rFontName) { GetDefaults().aFontName=rFontName; }
    static String     GetFontName()                        { return GetDefaults().aFontName; }
    // Default FontFamily ist FAMILY_ROMAN
    static void       SetFontFamily(FontFamily eFam)       { GetDefaults().eFontFamily=eFam; }
    static FontFamily GetFontFamily()                      { return GetDefaults().eFontFamily; }
    // Default FontColor ist COL_BLACK
    static void       SetFontColor(const Color& rColor)    { GetDefaults().aFontColor=rColor; }
    static Color      GetFontColor()                       { return GetDefaults().aFontColor; }
    // Default FontHeight ist 847. Die Fonthoehe wird in logischen Einheiten
    // (MapUnit/MapFraction (siehe unten)) angegeben. Die Defaulteinstellung
    // 847/100mm entspricht also ca. 24 Point. Verwendet man stattdessen
    // beispielsweise Twips (SetMapUnit(MAP_TWIP)) (20 Twip = 1 Point) muss
    // man als Fonthoehe 480 angeben um 24 Point als default zu erhalten.
    static void       SetFontHeight(ULONG nHeight)         { GetDefaults().nFontHeight=nHeight; }
    static ULONG      GetFontHeight()                      { return GetDefaults().nFontHeight; }
    // Der MapMode wird fuer den globalen Outliner benoetigt.
    // Gleichzeitig bekommt auch jedes neu instanziierte SdrModel
    // diesen MapMode default zugewiesen.
    // Default MapUnit ist MAP_100TH_MM
    static void       SetMapUnit(MapUnit eMap)             { GetDefaults().eMapUnit=eMap; }
    static MapUnit    GetMapUnit()                         { return GetDefaults().eMapUnit; }
    // Default MapFraction ist 1/1.
    static void       SetMapFraction(const Fraction& rMap) { GetDefaults().aMapFraction=rMap; }
    static Fraction   GetMapFraction()                     { return GetDefaults().aMapFraction; }
    // Der Aufruf der folgenden Methode veranlasst die Engine
    // ihre sprachabhaengigen Resourcen neu zu initiallisieren.
    // Bereits bestehende Undotexte, etc. bleiben jedoch in der
    // sprache erhalten, in der sie erzeugt wurden.

    // Einen Outliner mit den engineglobalen
    // Defaulteinstellungen auf dem Heap erzeugen.
    // Ist pMod<>NULL, dann wird der MapMode des uebergebenen
    // Models verwendet. Die resultierende Default-Fonthoehe bleibt
    // jedoch dieselbe (die logische Fonthoehe wird umgerechnet).
    friend SdrOutliner* SdrMakeOutliner( USHORT nOutlinerMode, SdrModel* pMod );
};

// Einen Outliner mit den engineglobalen
// Defaulteinstellungen auf dem Heap erzeugen.
// Ist pMod<>NULL, dann wird der MapMode des uebergebenen
// Models verwendet. Die resultierende Default-Fonthoehe bleibt
// jedoch dieselbe (die logische Fonthoehe wird umgerechnet).
SdrOutliner* SdrMakeOutliner( USHORT nOutlinerMode, SdrModel* pMod );

// Liefert eine Ersatzdarstellung fuer einen XFillStyle
// Bei XFILL_NONE gibt's FALSE und rCol bleibt unveraendert.
FASTBOOL GetDraftFillColor(const SfxItemSet& rSet, Color& rCol);

// Ein Container fuer USHORTs (im Prinzip ein dynamisches Array)
class UShortCont {
    Container aArr;
public:
    UShortCont(USHORT nBlock, USHORT nInit, USHORT nResize): aArr(nBlock,nInit,nResize) {}
    void   Clear()                                           { aArr.Clear(); }
    void   Insert(USHORT nElem, ULONG nPos=CONTAINER_APPEND) { aArr.Insert((void*)ULONG(nElem),nPos); }
    void   Remove(ULONG nPos)                                { aArr.Remove(nPos); }
    void   Replace(USHORT nElem, ULONG nPos)                 { aArr.Replace((void*)ULONG(nElem),nPos); }
    USHORT GetObject(ULONG nPos)                       const { return USHORT(ULONG(aArr.GetObject(nPos))); }
    ULONG  GetPos(USHORT nElem)                        const { return aArr.GetPos((void*)(ULONG)nElem); }
    ULONG  GetCount()                                  const { return aArr.Count(); }
    void   Sort();
};

class ContainerSorter {
protected:
    Container& rCont;
private:
#if _SOLAR__PRIVATE
    void ImpSubSort(long nL, long nR) const;
#endif // __PRIVATE
public:
    ContainerSorter(Container& rNewCont): rCont(rNewCont) {}
    void DoSort(ULONG a=0, ULONG b=0xFFFFFFFF) const;
#ifdef This_Is_Just_For_A_Comment
    Compare() muss returnieren:
      -1 falls *pElem1<*pElem2
       0 falls *pElem1=*pElem2
      +1 falls *pElem1>*pElem2
#endif
    virtual int Compare(const void* pElem1, const void* pElem2) const = 0;
private: // damit keiner vergessen wird
//  virtual FASTBOOL Is1stLessThan2nd(const void* pElem1, const void* pElem2) const=NULL;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#if _SOLAR__PRIVATE
#define SDRHDC_SAVEPEN                1 /* Save Linecolor                     */
#define SDRHDC_SAVEBRUSH              2 /* Save Fillcolorn                    */
#define SDRHDC_SAVEFONT               4 /* Save Font                          */
#define SDRHDC_SAVEPENANDBRUSH        3 /* Save Line- and FillColor           */
#define SDRHDC_SAVEPENANDBRUSHANDFONT 7 /* Save Font, Line- and fillcolor     */
#define SDRHDC_SAVECLIPPING           8 /* Save Clipping                      */
#define SDRHDC_SAVEALL               15 /* Save Clipping, Font, fill- and linecolor */

class ImpClipMerk;
class ImpColorMerk;
class ImpSdrHdcMerk
{
    ImpColorMerk* pFarbMerk;
    ImpClipMerk*  pClipMerk;
    Color*		  pLineColorMerk;
    USHORT        nMode;
public:
    ImpSdrHdcMerk(const OutputDevice& rOut, USHORT nNewMode=SDRHDC_SAVEALL, FASTBOOL bAutoMerk=TRUE);
    ~ImpSdrHdcMerk();
    void Save(const OutputDevice& rOut);
    void Restore(OutputDevice& rOut, USHORT nMask=SDRHDC_SAVEALL) const;
};
#endif // __PRIVATE

////////////////////////////////////////////////////////////////////////////////////////////////////

// Ein ItemSet auf Outliner- bzw. EditEngine-Items durchsuchen
// Liefert TRUE, wenn der Set solchen Items enthaelt.

// zurueck erhaelt man einen neuen WhichTable den
// man dann irgendwann mit delete platthauen muss.

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////


// Hilfsklasse zur kommunikation zwischen dem Dialog
// zum aufbrechen von Metafiles (sd/source/ui/dlg/brkdlg.cxx),
// SdrEditView::DoImportMarkedMtf() und
// ImpSdrGDIMetaFileImport::DoImport()

class SdrLinkList
{
    Container aList;
protected:
    unsigned FindEntry(const Link& rLink) const;
public:
    SdrLinkList(): aList(1024,4,4)        {}
    ~SdrLinkList()                        { Clear(); }
    void Clear();
    unsigned GetLinkCount() const            { return (unsigned)aList.Count(); }
    Link& GetLink(unsigned nNum)             { return *((Link*)(aList.GetObject(nNum))); }
    const Link& GetLink(unsigned nNum) const { return *((Link*)(aList.GetObject(nNum))); }
    void InsertLink(const Link& rLink, unsigned nPos=0xFFFF);
    void RemoveLink(const Link& rLink);
    FASTBOOL HasLink(const Link& rLink) const { return FindEntry(rLink)!=0xFFFF; }
};

// Fuer die Factory in SvdObj.CXX
SdrLinkList& ImpGetUserMakeObjHdl();
SdrLinkList& ImpGetUserMakeObjUserDataHdl();

class SdrOle2Obj;

class OLEObjCache : public Container
{
    ULONG				nSize;
    AutoTimer*          pTimer;

    BOOL UnloadObj( SdrOle2Obj* pObj );
    DECL_LINK( UnloadCheckHdl, AutoTimer* );

public:
    OLEObjCache();
    ~OLEObjCache();

    void InsertObj(SdrOle2Obj* pObj);
    void RemoveObj(SdrOle2Obj* pObj);
};

class SdrGlobalData
{
public:
    SdrLinkList     	aUserMakeObjHdl;
    SdrLinkList     	aUserMakeObjUserDataHdl;
    SdrOutliner*       	pOutliner;
    SdrEngineDefaults* 	pDefaults;
    ResMgr*            	pResMgr;
    String*            	pStrCache;
    ULONG              	nExchangeFormat;
    OLEObjCache			aOLEObjCache;
    const SvtSysLocale*         pSysLocale;     // follows always locale settings
    const CharClass*            pCharClass;     // follows always SysLocale
    const LocaleDataWrapper*    pLocaleData;    // follows always SysLocale

public:
    SdrGlobalData();
    ~SdrGlobalData();

    OLEObjCache& 		GetOLEObjCache() { return aOLEObjCache; }
};

inline SdrGlobalData& GetSdrGlobalData()
{
    void** ppAppData=GetAppData(BF_SHL_SVD);
    if (*ppAppData==NULL) {
        *ppAppData=new SdrGlobalData;
    }
    return *((SdrGlobalData*)*ppAppData);
}

/////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDETC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
