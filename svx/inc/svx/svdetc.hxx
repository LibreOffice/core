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

#include <vcl/outdev.hxx>
#include <tools/shl.hxx>
#include <editeng/outliner.hxx>
#include "svx/svxdllapi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

// ExchangeFormat-Id der DrawingEngine holen. Daten koennen dann per
//   static sal_Bool CopyData(pData,nLen,nFormat);
// bereitgestellt werden, wobei pData/nLen einen SvMemoryStream beschreiben in
// dem ein SdrModel gestreamt wird an dem fuer die Zeitdauer des Streamens das
// Flag SdrModel::SetStreamingSdrModel(sal_True) gesetzt wird.
// sal_uIntPtr SdrGetExchangeFormat(); -- JP 18.01.99 - dafuer gibt es ein define

class SdrOutliner;
class SdrModel;
class SvtSysLocale;
class LocaleDataWrapper;

namespace com { namespace sun { namespace star { namespace lang {
    struct Locale;
}}}}

// Einen Outliner mit den engineglobalen
// Defaulteinstellungen auf dem Heap erzeugen.
// Ist pMod<>NULL, dann wird der MapMode des uebergebenen
// Models verwendet. Die resultierende Default-Fonthoehe bleibt
// jedoch dieselbe (die logische Fonthoehe wird umgerechnet).
SVX_DLLPUBLIC SdrOutliner* SdrMakeOutliner( sal_uInt16 nOutlinerMode, SdrModel* pMod );

// Globale Defaulteinstellungen fuer die DrawingEngine.
// Diese Einstellungen sollte man direkt beim Applikationsstart
// vornehmen, noch bevor andere Methoden der Engine gerufen werden.
class SVX_DLLPUBLIC SdrEngineDefaults
{
friend class SdrAttrObj;
    String     aFontName;
    FontFamily eFontFamily;
    Color      aFontColor;
    sal_uIntPtr      nFontHeight;
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
    static void       SetFontHeight(sal_uIntPtr nHeight)         { GetDefaults().nFontHeight=nHeight; }
    static sal_uIntPtr      GetFontHeight()                      { return GetDefaults().nFontHeight; }
    // Der MapMode wird fuer den globalen Outliner benoetigt.
    // Gleichzeitig bekommt auch jedes neu instanziierte SdrModel
    // diesen MapMode default zugewiesen.
    // Default MapUnit ist MAP_100TH_MM
    static void       SetMapUnit(MapUnit eMap)             { GetDefaults().eMapUnit=eMap; }
    static MapUnit    GetMapUnit()                         { return GetDefaults().eMapUnit; }
    // Default MapFraction ist 1/1.
    static void       SetMapFraction(const Fraction& rMap) { GetDefaults().aMapFraction=rMap; }
    static Fraction   GetMapFraction()                     { return GetDefaults().aMapFraction; }

    // Einen Outliner mit den engineglobalen
    // Defaulteinstellungen auf dem Heap erzeugen.
    // Ist pMod<>NULL, dann wird der MapMode des uebergebenen
    // Models verwendet. Die resultierende Default-Fonthoehe bleibt
    // jedoch dieselbe (die logische Fonthoehe wird umgerechnet).
    friend SVX_DLLPUBLIC SdrOutliner* SdrMakeOutliner( sal_uInt16 nOutlinerMode, SdrModel* pMod );
};

class SfxItemSet;
// Liefert eine Ersatzdarstellung fuer einen XFillStyle
// Bei XFILL_NONE gibt's sal_False und rCol bleibt unveraendert.
SVX_DLLPUBLIC bool GetDraftFillColor(const SfxItemSet& rSet, Color& rCol);

////////////////////////////////////////////////////////////////////////////////////////////////////

// Ein ItemSet auf Outliner- bzw. EditEngine-Items durchsuchen
// Liefert sal_True, wenn der Set solchen Items enthaelt.
sal_Bool SearchOutlinerItems(const SfxItemSet& rSet, sal_Bool bInklDefaults, sal_Bool* pbOnlyEE=NULL);

// zurueck erhaelt man einen neuen WhichTable den
// man dann irgendwann mit delete platthauen muss.
sal_uInt16* RemoveWhichRange(const sal_uInt16* pOldWhichTable, sal_uInt16 nRangeBeg, sal_uInt16 nRangeEnd);

////////////////////////////////////////////////////////////////////////////////////////////////////

class Link;

// Hilfsklasse zur kommunikation zwischen dem Dialog
// zum aufbrechen von Metafiles (sd/source/ui/dlg/brkdlg.cxx),
// SdrEditView::DoImportMarkedMtf() und
// ImpSdrGDIMetaFileImport::DoImport()
class SVX_DLLPUBLIC SvdProgressInfo
{
private:
    sal_uIntPtr nSumActionCount;    // Summe aller Actions
    sal_uIntPtr nSumCurAction;  // Summe aller bearbeiteten Actions

    sal_uIntPtr nActionCount;       // Anzahl der Actions im akt. Obj.
    sal_uIntPtr nCurAction;     // Anzahl bearbeiteter Act. im akt. Obj.

    sal_uIntPtr nInsertCount;       // Anzahl einzufuegender Act. im akt. Obj.
    sal_uIntPtr nCurInsert;     // Anzahl bereits eingefuegter Actions

    sal_uIntPtr nObjCount;      // Anzahl der selektierten Objekte
    sal_uIntPtr nCurObj;            // Aktuelles Objekt

    Link *pLink;

public:
    SvdProgressInfo( Link *_pLink );

    void Init( sal_uIntPtr _nSumActionCount, sal_uIntPtr _nObjCount );

    sal_Bool SetNextObject();

    void SetActionCount( sal_uIntPtr _nActionCount );
    void SetInsertCount( sal_uIntPtr _nInsertCount );

    sal_Bool ReportActions( sal_uIntPtr nAnzActions );
    sal_Bool ReportInserts( sal_uIntPtr nAnzInserts );

    sal_uIntPtr GetSumActionCount() const { return nSumActionCount; };
    sal_uIntPtr GetSumCurAction() const { return nSumCurAction; };
    sal_uIntPtr GetObjCount() const { return nObjCount; };
    sal_uIntPtr GetCurObj() const { return nCurObj; };

    sal_uIntPtr GetActionCount() const { return nActionCount; };
    sal_uIntPtr GetCurAction() const { return nCurAction; };

    sal_uIntPtr GetInsertCount() const { return nInsertCount; };
    sal_uIntPtr GetCurInsert() const { return nCurInsert; };

    void ReportError();
    sal_Bool ReportRescales( sal_uIntPtr nAnzRescales );
};



class SdrLinkList
{
    std::vector<Link*> aList;
protected:
    unsigned FindEntry(const Link& rLink) const;
public:
    SdrLinkList(): aList()                   {}
    ~SdrLinkList()                           { Clear(); }
    SVX_DLLPUBLIC void Clear();
    unsigned GetLinkCount() const            { return (unsigned)aList.size(); }
    Link& GetLink(unsigned nNum)             { return *aList[nNum]; }
    const Link& GetLink(unsigned nNum) const { return *aList[nNum]; }
    void InsertLink(const Link& rLink, unsigned nPos=0xFFFF);
    void RemoveLink(const Link& rLink);
    bool HasLink(const Link& rLink) const { return FindEntry(rLink)!=0xFFFF; }
};

// Fuer die Factory in SvdObj.CXX
SdrLinkList& ImpGetUserMakeObjHdl();
SdrLinkList& ImpGetUserMakeObjUserDataHdl();

class SdrOle2Obj;
class AutoTimer;

class OLEObjCache : public std::vector<SdrOle2Obj*>
{
    sal_uIntPtr         nSize;
    AutoTimer*          pTimer;

    void UnloadOnDemand();
    sal_Bool UnloadObj( SdrOle2Obj* pObj );
    DECL_LINK( UnloadCheckHdl, AutoTimer* );

public:
    OLEObjCache();
    SVX_DLLPUBLIC ~OLEObjCache();

    void InsertObj(SdrOle2Obj* pObj);
    void RemoveObj(SdrOle2Obj* pObj);
};


class SVX_DLLPUBLIC SdrGlobalData
{
    const SvtSysLocale*         pSysLocale;     // follows always locale settings
    const LocaleDataWrapper*    pLocaleData;    // follows always SysLocale
public:
    SdrLinkList         aUserMakeObjHdl;
    SdrLinkList         aUserMakeObjUserDataHdl;
    SdrOutliner*        pOutliner;
    SdrEngineDefaults*  pDefaults;
    ResMgr*             pResMgr;
    sal_uIntPtr                 nExchangeFormat;
    OLEObjCache         aOLEObjCache;


    const SvtSysLocale*         GetSysLocale();     // follows always locale settings
    const LocaleDataWrapper*    GetLocaleData();    // follows always SysLocale
public:
    SdrGlobalData();

    OLEObjCache&        GetOLEObjCache() { return aOLEObjCache; }
};

inline SdrGlobalData& GetSdrGlobalData()
{
    void** ppAppData=GetAppData(SHL_SVD);
    if (*ppAppData==NULL) {
        *ppAppData=new SdrGlobalData;
    }
    return *((SdrGlobalData*)*ppAppData);
}

namespace sdr
{

SVX_DLLPUBLIC String GetResourceString(sal_uInt16 nResID);

}

/////////////////////////////////////////////////////////////////////
// #i101872# isolated GetTextEditBackgroundColor for tooling
class SdrObjEditView;

SVX_DLLPUBLIC Color GetTextEditBackgroundColor(const SdrObjEditView& rView);

/////////////////////////////////////////////////////////////////////

#endif //_SVDETC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
