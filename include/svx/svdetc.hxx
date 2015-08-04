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

#ifndef INCLUDED_SVX_SVDETC_HXX
#define INCLUDED_SVX_SVDETC_HXX

#include <rtl/ustring.hxx>
#include <editeng/outliner.hxx>
#include <svx/svxdllapi.h>
#include <tools/link.hxx>
#include <tools/fract.hxx>
#include <vcl/outdev.hxx>



// ExchangeFormat-Id der DrawingEngine holen. Daten koennen dann per
//   static bool CopyData(pData,nLen,nFormat);
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
SVX_DLLPUBLIC SdrOutliner* SdrMakeOutliner(sal_uInt16 nOutlinerMode, SdrModel& rMod);

// Globale Defaulteinstellungen fuer die DrawingEngine.
// Diese Einstellungen sollte man direkt beim Applikationsstart
// vornehmen, noch bevor andere Methoden der Engine gerufen werden.
class SVX_DLLPUBLIC SdrEngineDefaults
{
friend class SdrAttrObj;
    OUString   aFontName;
    FontFamily eFontFamily;
    Color      aFontColor;
    sal_uIntPtr      nFontHeight;
    MapUnit    eMapUnit;
    Fraction   aMapFraction;

private:
    static SdrEngineDefaults& GetDefaults();

public:
    SdrEngineDefaults();

    // Default FontColor ist COL_BLACK
    static Color      GetFontColor()                       { return GetDefaults().aFontColor; }

    // Default FontHeight ist 847. Die Fonthoehe wird in logischen Einheiten
    // (MapUnit/MapFraction (siehe unten)) angegeben. Die Defaulteinstellung
    // 847/100mm entspricht also ca. 24 Point. Verwendet man stattdessen
    // beispielsweise Twips (SetMapUnit(MAP_TWIP)) (20 Twip = 1 Point) muss
    // man als Fonthoehe 480 angeben um 24 Point als default zu erhalten.
    static sal_uIntPtr      GetFontHeight()                      { return GetDefaults().nFontHeight; }

    // Der MapMode wird fuer den globalen Outliner benoetigt.
    // Gleichzeitig bekommt auch jedes neu instanziierte SdrModel
    // diesen MapMode default zugewiesen.
    // Default MapUnit ist MAP_100TH_MM
    static MapUnit    GetMapUnit()                         { return GetDefaults().eMapUnit; }

    // Default MapFraction ist 1/1.
    static Fraction   GetMapFraction()                     { return GetDefaults().aMapFraction; }

    // Einen Outliner mit den engineglobalen
    // Defaulteinstellungen auf dem Heap erzeugen.
    // Ist pMod<>NULL, dann wird der MapMode des uebergebenen
    // Models verwendet. Die resultierende Default-Fonthoehe bleibt
    // jedoch dieselbe (die logische Fonthoehe wird umgerechnet).
    friend SVX_DLLPUBLIC SdrOutliner* SdrMakeOutliner(sal_uInt16 nOutlinerMode, SdrModel& rMod);
};

class SfxItemSet;
// Liefert eine Ersatzdarstellung fuer einen XFillStyle
// Bei XFILL_NONE gibt's sal_False und rCol bleibt unveraendert.
SVX_DLLPUBLIC bool GetDraftFillColor(const SfxItemSet& rSet, Color& rCol);



// Ein ItemSet auf Outliner- bzw. EditEngine-Items durchsuchen
// Liefert sal_True, wenn der Set solchen Items enthaelt.
bool SearchOutlinerItems(const SfxItemSet& rSet, bool bInklDefaults, bool* pbOnlyEE=NULL);

// zurueck erhaelt man einen neuen WhichTable den
// man dann irgendwann mit delete platthauen muss.
sal_uInt16* RemoveWhichRange(const sal_uInt16* pOldWhichTable, sal_uInt16 nRangeBeg, sal_uInt16 nRangeEnd);

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

    Link<> *pLink;

public:
    SvdProgressInfo( Link<> *_pLink );

    void Init( sal_uIntPtr _nSumActionCount, sal_uIntPtr _nObjCount );

    bool SetNextObject();

    void SetActionCount( sal_uIntPtr _nActionCount );
    void SetInsertCount( sal_uIntPtr _nInsertCount );

    bool ReportActions( sal_uIntPtr nActionCount );
    bool ReportInserts( sal_uIntPtr nInsertCount );

    sal_uIntPtr GetSumCurAction() const { return nSumCurAction; };
    sal_uIntPtr GetObjCount() const { return nObjCount; };
    sal_uIntPtr GetCurObj() const { return nCurObj; };

    sal_uIntPtr GetActionCount() const { return nActionCount; };
    sal_uIntPtr GetCurAction() const { return nCurAction; };

    sal_uIntPtr GetInsertCount() const { return nInsertCount; };
    sal_uIntPtr GetCurInsert() const { return nCurInsert; };

    bool ReportRescales( sal_uIntPtr nRescaleCount );
};



class SdrLinkList
{
    std::vector<Link<>*> aList;
protected:
    unsigned FindEntry(const Link<>& rLink) const;
public:
    SdrLinkList(): aList()                   {}
    ~SdrLinkList()                           { Clear(); }
    SVX_DLLPUBLIC void Clear();
    unsigned GetLinkCount() const            { return (unsigned)aList.size(); }
    Link<>& GetLink(unsigned nNum)           { return *aList[nNum]; }
    const Link<>& GetLink(unsigned nNum) const { return *aList[nNum]; }
    void InsertLink(const Link<>& rLink, unsigned nPos=0xFFFF);
    void RemoveLink(const Link<>& rLink);
};

SdrLinkList& ImpGetUserMakeObjHdl();
SdrLinkList& ImpGetUserMakeObjUserDataHdl();

class SdrOle2Obj;
class AutoTimer;

class OLEObjCache
{
    std::vector<SdrOle2Obj*> maObjs;

    size_t         nSize;
    AutoTimer*          pTimer;

    void UnloadOnDemand();
    static bool UnloadObj( SdrOle2Obj* pObj );
    DECL_LINK_TYPED( UnloadCheckHdl, Timer*, void );

public:
    OLEObjCache();
    SVX_DLLPUBLIC ~OLEObjCache();

    void InsertObj(SdrOle2Obj* pObj);
    void RemoveObj(SdrOle2Obj* pObj);

    SVX_DLLPUBLIC size_t size() const;
    SVX_DLLPUBLIC SdrOle2Obj* operator[](size_t nPos);
    SVX_DLLPUBLIC const SdrOle2Obj* operator[](size_t nPos) const;
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

SVX_DLLPUBLIC SdrGlobalData & GetSdrGlobalData();

namespace sdr
{
    SVX_DLLPUBLIC OUString GetResourceString(sal_uInt16 nResID);
}


// #i101872# isolated GetTextEditBackgroundColor for tooling
class SdrObjEditView;

SVX_DLLPUBLIC Color GetTextEditBackgroundColor(const SdrObjEditView& rView);



#endif // INCLUDED_SVX_SVDETC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
