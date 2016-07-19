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


/**
 * Get ExchangeFormatID of the DrawingEngine.
 * The data can then be made available via
 *   static bool CopyData(pData,nLen,nFormat);
 *
 * However, pData/nLen describe an SvMemoryStream in which an SdrModel is
 * streamed. For its lifetime, the flag SdrModel::SetStreamingSdrModel(true)
 * is set.
 */

class SdrOutliner;
class SdrModel;
class SvtSysLocale;
class LocaleDataWrapper;

namespace com { namespace sun { namespace star { namespace lang {
    struct Locale;
}}}}

/**
 * Create an Outliner with the engine-global default settings on the heap.
 * If pMod != nullptr, the MapMode of the passed model is used.
 * The resulting default font height, however, stays the same (the logical
 * font height is converted).
 */
SVX_DLLPUBLIC SdrOutliner* SdrMakeOutliner(OutlinerMode nOutlinerMode, SdrModel& rMod);

/**
 * Global default settings for the DrawingEngine.
 *
 * One should set these default settings as the first
 * thing at program start, before any other method is called.
 */
class SVX_DLLPUBLIC SdrEngineDefaults
{
friend class SdrAttrObj;
    Color            aFontColor;
    size_t           nFontHeight;
    Fraction         aMapFraction;

private:
    static SdrEngineDefaults& GetDefaults();

public:
    SdrEngineDefaults();

    // Default FontColor is COL_BLACK
    static Color      GetFontColor()                       { return GetDefaults().aFontColor; }

    // Default FontHeight is 847. The font height uses logical units (MapUnit/MapFraction
    // see below for further details). The default setting 847/100mm corresponds to about
    // 24 Point. If e.g. one would use Twips (SetMapUnit(MAP_TWIP)) (20 Twip = 1 Point)
    // instead, one would need to set the font height to 480, in order to get a 24 Point height.
    static size_t      GetFontHeight()                      { return GetDefaults().nFontHeight; }

    // The MapMode is needed for the global Outliner.
    // Incidentally, every newly instantiated SdrModel is assigned this MapMode by default.
    // Default MapUnit is MAP_100TH_MM
    static MapUnit    GetMapUnit()                         { return MAP_100TH_MM; }

    // Default MapFraction is 1/1.
    static Fraction   GetMapFraction()                     { return GetDefaults().aMapFraction; }

    // Create an Outliner with the engine-global default values on the heap.
    // If pMod != nullptr, the MapMode of the passed model is used.
    // The resulting default font height, however, stays the same (the logical font height is converted).
    friend SVX_DLLPUBLIC SdrOutliner* SdrMakeOutliner(OutlinerMode nOutlinerMode, SdrModel& rMod);
};

class SfxItemSet;

/**
 * Returns a replacement for an XFillStyle
 *
 * @returns false for XFILL_NONE and rCol remains unchanged
 */
SVX_DLLPUBLIC bool GetDraftFillColor(const SfxItemSet& rSet, Color& rCol);


/**
 * Search an ItemSet for Outliner/EditEngine Items
 *
 * @returns true, if the set contains such items
 */
bool SearchOutlinerItems(const SfxItemSet& rSet, bool bInklDefaults, bool* pbOnlyEE=nullptr);

/**
 * @returns a new WhichTable, which we need to squash at some point with a delete
 */
sal_uInt16* RemoveWhichRange(const sal_uInt16* pOldWhichTable, sal_uInt16 nRangeBeg, sal_uInt16 nRangeEnd);

/**
 * Helper class for the communication between the dialog
 * In order to break open Metafiles (sd/source/ui/dlg/brkdlg.cxx),
 * SdrEditView::DoImportMarkedMtf() and ImpSdrGDIMetaFileImport::DoImport()
 */
class SVX_DLLPUBLIC SvdProgressInfo
{
private:
    size_t      m_nSumActionCount; // Sum of all Actions
    size_t      m_nSumCurAction;   // Sum of all handled Actions

    size_t      m_nActionCount;   // Count of Actions in the current object
    size_t      m_nCurAction;     // Count of handled Actions in the current object

    size_t      m_nInsertCount;   // Count of to-be-inserted Actions in the current object
    size_t      m_nCurInsert;     // Count of already inserted Actions

    size_t      m_nObjCount;      // Count of selected objects
    size_t      m_nCurObj;        // Current object

    Link<void*,bool>  maLink;

public:
    SvdProgressInfo( const Link<void*,bool>& _pLink );

    void Init( size_t _nSumActionCount, size_t _nObjCount );

    void SetNextObject();

    void SetActionCount( size_t _nActionCount );
    void SetInsertCount( size_t _nInsertCount );

    bool ReportActions( size_t nActionCount );
    void ReportInserts( size_t nInsertCount );

    size_t      GetSumCurAction() const { return m_nSumCurAction; };
    size_t      GetObjCount() const { return m_nObjCount; };
    size_t      GetCurObj() const { return m_nCurObj; };

    size_t      GetActionCount() const { return m_nActionCount; };
    size_t      GetCurAction() const { return m_nCurAction; };

    size_t      GetInsertCount() const { return m_nInsertCount; };
    size_t      GetCurInsert() const { return m_nCurInsert; };

    void ReportRescales( size_t nRescaleCount );
};


class SdrObjFactory;

class SdrLinkList
{
    std::vector<Link<SdrObjFactory*,void> > aList;
protected:
    unsigned FindEntry(const Link<SdrObjFactory*,void>& rLink) const;
public:
    SdrLinkList(): aList()                   {}
    ~SdrLinkList()                           { Clear(); }
    SVX_DLLPUBLIC void Clear();
    unsigned GetLinkCount() const            { return (unsigned)aList.size(); }
    Link<SdrObjFactory*,void>& GetLink(unsigned nNum)           { return aList[nNum]; }
    const Link<SdrObjFactory*,void>& GetLink(unsigned nNum) const { return aList[nNum]; }
    void InsertLink(const Link<SdrObjFactory*,void>& rLink);
    void RemoveLink(const Link<SdrObjFactory*,void>& rLink);
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
    SdrEngineDefaults*  pDefaults;
    ResMgr*             pResMgr;
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
