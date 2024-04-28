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

#include <editeng/outliner.hxx>
#include <svx/svxdllapi.h>
#include <tools/link.hxx>
#include <svx/svdobj.hxx>
#include <svl/whichranges.hxx>
#include <unotools/syslocale.hxx>
#include <memory>
#include <o3tl/unit_conversion.hxx>


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

namespace com::sun::star::lang {
    struct Locale;
}

/**
 * Create an Outliner with the engine-global default settings on the heap.
 * If pMod != nullptr, the MapMode of the passed model is used.
 * The resulting default font height, however, stays the same (the logical
 * font height is converted).
 */
SVXCORE_DLLPUBLIC std::unique_ptr<SdrOutliner> SdrMakeOutliner(OutlinerMode nOutlinerMode, SdrModel& rMod);

/**
 * Global default settings for the DrawingEngine.
 *
 * One should set these default settings as the first
 * thing at program start, before any other method is called.
 */
namespace SdrEngineDefaults
{
    // Default FontColor is COL_AUTO
    inline Color GetFontColor() { return COL_AUTO; }

    // Default font height
    inline size_t GetFontHeight() { return o3tl::convert(24, o3tl::Length::pt, o3tl::Length::mm100); }

    // The MapMode is needed for the global Outliner.
    // Incidentally, every newly instantiated SdrModel is assigned this MapMode by default.
    // Default MapUnit is MapUnit::Map100thMM
    inline MapUnit GetMapUnit() { return MapUnit::Map100thMM; }
};

class SfxItemSet;

/**
 * Returns a replacement for an XFillStyle
 *
 * @returns false for XFILL_NONE and rCol remains unchanged
 */
SVXCORE_DLLPUBLIC std::optional<Color> GetDraftFillColor(const SfxItemSet& rSet);


/**
 * Search an ItemSet for Outliner/EditEngine Items
 *
 * @returns true, if the set contains such items
 */
bool SearchOutlinerItems(const SfxItemSet& rSet, bool bInklDefaults, bool* pbOnlyEE=nullptr);

/**
 * @returns a new WhichTable, which we need to squash at some point with a delete
 */
WhichRangesContainer RemoveWhichRange(const WhichRangesContainer& pOldWhichTable, sal_uInt16 nRangeBeg, sal_uInt16 nRangeEnd);

/**
 * Helper class for the communication between the dialog
 * In order to break open Metafiles (sd/source/ui/dlg/brkdlg.cxx),
 * SdrEditView::DoImportMarkedMtf() and ImpSdrGDIMetaFileImport::DoImport()
 */
class SVXCORE_DLLPUBLIC SvdProgressInfo
{
private:
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

    void Init( size_t _nObjCount );

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


std::vector<Link<SdrObjCreatorParams, rtl::Reference<SdrObject>>>& ImpGetUserMakeObjHdl();

class SdrOle2Obj;
class AutoTimer;

class OLEObjCache
{
    std::vector<SdrOle2Obj*> maObjs;

    size_t         nSize;
    std::unique_ptr<AutoTimer>  pTimer;

    static bool UnloadObj(SdrOle2Obj& rObj);
    DECL_LINK( UnloadCheckHdl, Timer*, void );

public:
    OLEObjCache();
    UNLESS_MERGELIBS_MORE(SVXCORE_DLLPUBLIC) ~OLEObjCache();

    void InsertObj(SdrOle2Obj* pObj);
    void RemoveObj(SdrOle2Obj* pObj);

    UNLESS_MERGELIBS_MORE(SVXCORE_DLLPUBLIC) size_t size() const;
    UNLESS_MERGELIBS_MORE(SVXCORE_DLLPUBLIC) SdrOle2Obj* operator[](size_t nPos);
    UNLESS_MERGELIBS_MORE(SVXCORE_DLLPUBLIC) const SdrOle2Obj* operator[](size_t nPos) const;
};


class SVXCORE_DLLPUBLIC SdrGlobalData
{
    SvtSysLocale  maSysLocale;     // follows always locale settings
public:
    std::vector<Link<SdrObjCreatorParams, rtl::Reference<SdrObject>>>
                        aUserMakeObjHdl;
    OLEObjCache         aOLEObjCache;

    SAL_DLLPRIVATE SdrGlobalData();

    const SvtSysLocale& GetSysLocale() { return maSysLocale;  } // follows always locale settings
    const LocaleDataWrapper& GetLocaleData();    // follows always SysLocale
    OLEObjCache&        GetOLEObjCache() { return aOLEObjCache; }
};

SVXCORE_DLLPUBLIC SdrGlobalData & GetSdrGlobalData();


// #i101872# isolated GetTextEditBackgroundColor for tooling
class SdrObjEditView;

SVXCORE_DLLPUBLIC Color GetTextEditBackgroundColor(const SdrObjEditView& rView);


#endif // INCLUDED_SVX_SVDETC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
