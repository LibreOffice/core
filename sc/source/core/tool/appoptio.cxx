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

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <appoptio.hxx>
#include <global.hxx>
#include <userlist.hxx>
#include <formula/compiler.hxx>
#include <miscuno.hxx>
#include <vector>
#include <osl/diagnose.h>

using namespace utl;
using namespace com::sun::star::uno;

//      ScAppOptions - Application Options

ScAppOptions::ScAppOptions()
{
    SetDefaults();
}

ScAppOptions::ScAppOptions( const ScAppOptions& rCpy )
{
    *this = rCpy;
}

ScAppOptions::~ScAppOptions()
{
}

void ScAppOptions::SetDefaults()
{
    if ( ScOptionsUtil::IsMetricSystem() )
        eMetric     = FieldUnit::CM;             // default for countries with metric system
    else
        eMetric     = FieldUnit::INCH;           // default for others

    nZoom           = 100;
    eZoomType       = SvxZoomType::PERCENT;
    bSynchronizeZoom = true;
    nStatusFunc     = ( 1 << SUBTOTAL_FUNC_SUM );
    bAutoComplete   = true;
    bDetectiveAuto  = true;

    pLRUList.reset( new sal_uInt16[5] );               // sensible initialization
    pLRUList[0] = SC_OPCODE_SUM;
    pLRUList[1] = SC_OPCODE_AVERAGE;
    pLRUList[2] = SC_OPCODE_MIN;
    pLRUList[3] = SC_OPCODE_MAX;
    pLRUList[4] = SC_OPCODE_IF;
    nLRUFuncCount = 5;

    nTrackContentColor = COL_TRANSPARENT;
    nTrackInsertColor  = COL_TRANSPARENT;
    nTrackDeleteColor  = COL_TRANSPARENT;
    nTrackMoveColor    = COL_TRANSPARENT;
    eLinkMode          = LM_ON_DEMAND;

    nDefaultObjectSizeWidth = 8000;
    nDefaultObjectSizeHeight = 5000;

    mbShowSharedDocumentWarning = true;

    meKeyBindingType     = ScOptionsUtil::KEY_DEFAULT;
    mbLinksInsertedLikeMSExcel = false;
}

ScAppOptions& ScAppOptions::operator=( const ScAppOptions& rCpy )
{
    eMetric         = rCpy.eMetric;
    eZoomType       = rCpy.eZoomType;
    bSynchronizeZoom = rCpy.bSynchronizeZoom;
    nZoom           = rCpy.nZoom;
    SetLRUFuncList( rCpy.pLRUList.get(), rCpy.nLRUFuncCount );
    nStatusFunc     = rCpy.nStatusFunc;
    bAutoComplete   = rCpy.bAutoComplete;
    bDetectiveAuto  = rCpy.bDetectiveAuto;
    nTrackContentColor = rCpy.nTrackContentColor;
    nTrackInsertColor  = rCpy.nTrackInsertColor;
    nTrackDeleteColor  = rCpy.nTrackDeleteColor;
    nTrackMoveColor    = rCpy.nTrackMoveColor;
    eLinkMode       = rCpy.eLinkMode;
    nDefaultObjectSizeWidth = rCpy.nDefaultObjectSizeWidth;
    nDefaultObjectSizeHeight = rCpy.nDefaultObjectSizeHeight;
    mbShowSharedDocumentWarning = rCpy.mbShowSharedDocumentWarning;
    meKeyBindingType  = rCpy.meKeyBindingType;
    mbLinksInsertedLikeMSExcel = rCpy.mbLinksInsertedLikeMSExcel;
    return *this;
}

void ScAppOptions::SetLRUFuncList( const sal_uInt16* pList, const sal_uInt16 nCount )
{
    nLRUFuncCount = nCount;

    if ( nLRUFuncCount > 0 )
    {
        pLRUList.reset( new sal_uInt16[nLRUFuncCount] );

        for ( sal_uInt16 i=0; i<nLRUFuncCount; i++ )
            pLRUList[i] = pList[i];
    }
    else
        pLRUList.reset();
}

//  Config Item containing app options

static void lcl_GetLastFunctions( Any& rDest, const ScAppOptions& rOpt )
{
    tools::Long nCount = rOpt.GetLRUFuncListCount();
    sal_uInt16* pUShorts = rOpt.GetLRUFuncList();
    if ( nCount && pUShorts )
    {
        Sequence<sal_Int32> aSeq( nCount );
        sal_Int32* pArray = aSeq.getArray();
        for (tools::Long i=0; i<nCount; i++)
            pArray[i] = pUShorts[i];
        rDest <<= aSeq;
    }
    else
        rDest <<= Sequence<sal_Int32>(0);   // empty
}

static void lcl_GetSortList( Any& rDest )
{
    const ScUserList& rUserList = ScGlobal::GetUserList();
    size_t nCount = rUserList.size();
    Sequence<OUString> aSeq( nCount );
    OUString* pArray = aSeq.getArray();
    for (size_t i=0; i<nCount; ++i)
        pArray[i] = rUserList[sal::static_int_cast<sal_uInt16>(i)].GetString();
    rDest <<= aSeq;
}

constexpr OUStringLiteral CFGPATH_LAYOUT = u"Office.Calc/Layout";

#define SCLAYOUTOPT_MEASURE         0
#define SCLAYOUTOPT_STATUSBAR       1
#define SCLAYOUTOPT_ZOOMVAL         2
#define SCLAYOUTOPT_ZOOMTYPE        3
#define SCLAYOUTOPT_SYNCZOOM        4
#define SCLAYOUTOPT_STATUSBARMULTI  5

constexpr OUStringLiteral CFGPATH_INPUT = u"Office.Calc/Input";

#define SCINPUTOPT_LASTFUNCS        0
#define SCINPUTOPT_AUTOINPUT        1
#define SCINPUTOPT_DET_AUTO         2

constexpr OUStringLiteral CFGPATH_REVISION = u"Office.Calc/Revision/Color";

#define SCREVISOPT_CHANGE           0
#define SCREVISOPT_INSERTION        1
#define SCREVISOPT_DELETION         2
#define SCREVISOPT_MOVEDENTRY       3

constexpr OUStringLiteral CFGPATH_CONTENT = u"Office.Calc/Content/Update";

#define SCCONTENTOPT_LINK           0

constexpr OUStringLiteral CFGPATH_SORTLIST = u"Office.Calc/SortList";

#define SCSORTLISTOPT_LIST          0

constexpr OUStringLiteral CFGPATH_MISC = u"Office.Calc/Misc";

#define SCMISCOPT_DEFOBJWIDTH       0
#define SCMISCOPT_DEFOBJHEIGHT      1
#define SCMISCOPT_SHOWSHAREDDOCWARN 2

constexpr OUStringLiteral CFGPATH_COMPAT = u"Office.Calc/Compatibility";

#define SCCOMPATOPT_KEY_BINDING     0
#define SCCOMPATOPT_LINK_LIKE_MS    1

// Default value of Layout/Other/StatusbarMultiFunction
#define SCLAYOUTOPT_STATUSBARMULTI_DEFAULTVAL         514
// Default value of Layout/Other/StatusbarFunction
#define SCLAYOUTOPT_STATUSBAR_DEFAULTVAL              1
// Legacy default value of Layout/Other/StatusbarFunction
// prior to multiple statusbar functions feature addition
#define SCLAYOUTOPT_STATUSBAR_DEFAULTVAL_LEGACY       9

static sal_uInt32 lcl_ConvertStatusBarFuncSetToSingle( sal_uInt32 nFuncSet )
{
    if ( !nFuncSet )
        return 0;
    for ( sal_uInt32 nFunc = 1; nFunc < 32; ++nFunc )
        if ( nFuncSet & ( 1U << nFunc ) )
            return nFunc;
    return 0;
}

Sequence<OUString> ScAppCfg::GetLayoutPropertyNames()
{
    const bool bIsMetric = ScOptionsUtil::IsMetricSystem();

    return {(bIsMetric ? u"Other/MeasureUnit/Metric"_ustr
                       : u"Other/MeasureUnit/NonMetric"_ustr),  // SCLAYOUTOPT_MEASURE
             u"Other/StatusbarFunction"_ustr,                             // SCLAYOUTOPT_STATUSBAR
             u"Zoom/Value"_ustr,                                          // SCLAYOUTOPT_ZOOMVAL
             u"Zoom/Type"_ustr,                                           // SCLAYOUTOPT_ZOOMTYPE
             u"Zoom/Synchronize"_ustr,                                    // SCLAYOUTOPT_SYNCZOOM
             u"Other/StatusbarMultiFunction"_ustr};                       // SCLAYOUTOPT_STATUSBARMULTI
}

Sequence<OUString> ScAppCfg::GetInputPropertyNames()
{
    return {u"LastFunctions"_ustr,            // SCINPUTOPT_LASTFUNCS
            u"AutoInput"_ustr,                // SCINPUTOPT_AUTOINPUT
            u"DetectiveAuto"_ustr};           // SCINPUTOPT_DET_AUTO
}

Sequence<OUString> ScAppCfg::GetRevisionPropertyNames()
{
    return {u"Change"_ustr,                   // SCREVISOPT_CHANGE
            u"Insertion"_ustr,                // SCREVISOPT_INSERTION
            u"Deletion"_ustr,                 // SCREVISOPT_DELETION
            u"MovedEntry"_ustr};              // SCREVISOPT_MOVEDENTRY
}

Sequence<OUString> ScAppCfg::GetContentPropertyNames()
{
    return {u"Link"_ustr};                    // SCCONTENTOPT_LINK
}

Sequence<OUString> ScAppCfg::GetSortListPropertyNames()
{
    return {u"List"_ustr};                    // SCSORTLISTOPT_LIST
}

Sequence<OUString> ScAppCfg::GetMiscPropertyNames()
{
    return {u"DefaultObjectSize/Width"_ustr,      // SCMISCOPT_DEFOBJWIDTH
            u"DefaultObjectSize/Height"_ustr,     // SCMISCOPT_DEFOBJHEIGHT
            u"SharedDocument/ShowWarning"_ustr};  // SCMISCOPT_SHOWSHAREDDOCWARN
}

Sequence<OUString> ScAppCfg::GetCompatPropertyNames()
{
    return {u"KeyBindings/BaseGroup"_ustr,    // SCCOMPATOPT_KEY_BINDING
            u"Links"_ustr };                  // SCCOMPATOPT_LINK_LIKE_MS
}

ScAppCfg::ScAppCfg() :
    aLayoutItem( CFGPATH_LAYOUT ),
    aInputItem( CFGPATH_INPUT ),
    aRevisionItem( CFGPATH_REVISION ),
    aContentItem( CFGPATH_CONTENT ),
    aSortListItem( CFGPATH_SORTLIST ),
    aMiscItem( CFGPATH_MISC ),
    aCompatItem( CFGPATH_COMPAT )
{
    aLayoutItem.EnableNotification(GetLayoutPropertyNames());
    ReadLayoutCfg();
    aLayoutItem.SetCommitLink( LINK( this, ScAppCfg, LayoutCommitHdl ) );
    aLayoutItem.SetNotifyLink( LINK( this, ScAppCfg, LayoutNotifyHdl ) );

    aInputItem.EnableNotification(GetInputPropertyNames());
    ReadInputCfg();
    aInputItem.SetCommitLink( LINK( this, ScAppCfg, InputCommitHdl ) );
    aInputItem.SetNotifyLink( LINK( this, ScAppCfg, InputNotifyHdl ) );

    aRevisionItem.EnableNotification(GetRevisionPropertyNames());
    ReadRevisionCfg();
    aRevisionItem.SetCommitLink( LINK( this, ScAppCfg, RevisionCommitHdl ) );
    aRevisionItem.SetNotifyLink( LINK( this, ScAppCfg, RevisionNotifyHdl ) );

    aContentItem.EnableNotification(GetContentPropertyNames());
    ReadContentCfg();
    aContentItem.SetCommitLink( LINK( this, ScAppCfg, ContentCommitHdl ) );
    aContentItem.SetNotifyLink( LINK( this, ScAppCfg, ContentNotifyHdl ) );

    aSortListItem.EnableNotification(GetSortListPropertyNames());
    ReadSortListCfg();
    aSortListItem.SetCommitLink( LINK( this, ScAppCfg, SortListCommitHdl ) );
    aSortListItem.SetNotifyLink( LINK( this, ScAppCfg, SortListNotifyHdl ) );

    aMiscItem.EnableNotification(GetMiscPropertyNames());
    ReadMiscCfg();
    aMiscItem.SetCommitLink( LINK( this, ScAppCfg, MiscCommitHdl ) );
    aMiscItem.SetNotifyLink( LINK( this, ScAppCfg, MiscNotifyHdl ) );

    aCompatItem.EnableNotification(GetCompatPropertyNames());
    ReadCompatCfg();
    aCompatItem.SetCommitLink( LINK(this, ScAppCfg, CompatCommitHdl) );
    aCompatItem.SetNotifyLink( LINK(this, ScAppCfg, CompatNotifyHdl) );
}

void ScAppCfg::ReadLayoutCfg()
{
    const Sequence<OUString> aNames = GetLayoutPropertyNames();
    const Sequence<Any> aValues = aLayoutItem.GetProperties(aNames);
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if (aValues.getLength() != aNames.getLength())
        return;

    sal_uInt32 nStatusBarFuncSingle = 0;
    sal_uInt32 nStatusBarFuncMulti = 0;

    if (sal_Int32 nIntVal; aValues[SCLAYOUTOPT_MEASURE] >>= nIntVal)
        SetAppMetric(static_cast<FieldUnit>(nIntVal));
    if (sal_uInt32 nUIntVal; aValues[SCLAYOUTOPT_STATUSBAR] >>= nUIntVal)
        nStatusBarFuncSingle = nUIntVal;
    if (sal_uInt32 nUIntVal; aValues[SCLAYOUTOPT_STATUSBARMULTI] >>= nUIntVal)
        nStatusBarFuncMulti = nUIntVal;
    if (sal_Int32 nIntVal; aValues[SCLAYOUTOPT_ZOOMVAL] >>= nIntVal)
        SetZoom(static_cast<sal_uInt16>(nIntVal));
    if (sal_Int32 nIntVal; aValues[SCLAYOUTOPT_ZOOMTYPE] >>= nIntVal)
        SetZoomType(static_cast<SvxZoomType>(nIntVal));
    SetSynchronizeZoom(ScUnoHelpFunctions::GetBoolFromAny(aValues[SCLAYOUTOPT_SYNCZOOM]));

    if (nStatusBarFuncMulti != SCLAYOUTOPT_STATUSBARMULTI_DEFAULTVAL)
        SetStatusFunc(nStatusBarFuncMulti);
    else if (nStatusBarFuncSingle != SCLAYOUTOPT_STATUSBAR_DEFAULTVAL
             && nStatusBarFuncSingle != SCLAYOUTOPT_STATUSBAR_DEFAULTVAL_LEGACY)
    {
        if (nStatusBarFuncSingle)
            SetStatusFunc(1 << nStatusBarFuncSingle);
        else
            SetStatusFunc(0);
    }
    else
        SetStatusFunc(SCLAYOUTOPT_STATUSBARMULTI_DEFAULTVAL);
}

void ScAppCfg::ReadInputCfg()
{
    const Sequence<OUString> aNames = GetInputPropertyNames();
    const Sequence<Any> aValues = aInputItem.GetProperties(aNames);
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if (aValues.getLength() != aNames.getLength())
        return;

    if (Sequence<sal_Int32> aSeq; aValues[SCINPUTOPT_LASTFUNCS] >>= aSeq)
    {
        sal_Int32 nCount = aSeq.getLength();
        if (nCount < SAL_MAX_UINT16)
        {
            std::vector<sal_uInt16> pUShorts(nCount);
            for (sal_Int32 i = 0; i < nCount; i++)
                pUShorts[i] = aSeq[i];

            SetLRUFuncList(pUShorts.data(), nCount);
        }
    }
    SetAutoComplete(ScUnoHelpFunctions::GetBoolFromAny(aValues[SCINPUTOPT_AUTOINPUT]));
    SetDetectiveAuto(ScUnoHelpFunctions::GetBoolFromAny(aValues[SCINPUTOPT_DET_AUTO]));
}

void ScAppCfg::ReadRevisionCfg()
{
    const Sequence<OUString> aNames = GetRevisionPropertyNames();
    const Sequence<Any> aValues = aRevisionItem.GetProperties(aNames);
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if (aValues.getLength() != aNames.getLength())
        return;

    if (sal_Int32 nIntVal; aValues[SCREVISOPT_CHANGE] >>= nIntVal)
        SetTrackContentColor(Color(ColorTransparency, nIntVal));
    if (sal_Int32 nIntVal; aValues[SCREVISOPT_INSERTION] >>= nIntVal)
        SetTrackInsertColor(Color(ColorTransparency, nIntVal));
    if (sal_Int32 nIntVal; aValues[SCREVISOPT_DELETION] >>= nIntVal)
        SetTrackDeleteColor(Color(ColorTransparency, nIntVal));
    if (sal_Int32 nIntVal; aValues[SCREVISOPT_MOVEDENTRY] >>= nIntVal)
        SetTrackMoveColor(Color(ColorTransparency, nIntVal));
}

void ScAppCfg::ReadContentCfg()
{
    const Sequence<OUString> aNames = GetContentPropertyNames();
    const Sequence<Any> aValues = aContentItem.GetProperties(aNames);
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if (aValues.getLength() != aNames.getLength())
        return;

    if (sal_Int32 nIntVal; aValues[SCCONTENTOPT_LINK] >>= nIntVal)
        SetLinkMode(static_cast<ScLkUpdMode>(nIntVal));
}

void ScAppCfg::ReadSortListCfg()
{
    const Sequence<OUString> aNames = GetSortListPropertyNames();
    const Sequence<Any> aValues = aSortListItem.GetProperties(aNames);
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if (aValues.getLength() != aNames.getLength())
        return;

    if (Sequence<OUString> aSeq; aValues[SCSORTLISTOPT_LIST] >>= aSeq)
    {
        ScUserList aList(false); // Do not init defaults

        //  if setting is "default", keep default values
        //TODO: mark "default" in a safe way
        const bool bDefault = (aSeq.getLength() == 1 && aSeq[0] == "NULL");

        if (bDefault)
        {
            aList.AddDefaults();
        }
        else
        {
            for (const OUString& rStr : aSeq)
            {
                aList.emplace_back(rStr);
            }
        }

        ScGlobal::SetUserList(&aList);
    }
}

void ScAppCfg::ReadMiscCfg()
{
    const Sequence<OUString> aNames = GetMiscPropertyNames();
    const Sequence<Any> aValues = aMiscItem.GetProperties(aNames);
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if (aValues.getLength() != aNames.getLength())
        return;

    if (sal_Int32 nIntVal; aValues[SCMISCOPT_DEFOBJWIDTH] >>= nIntVal)
        SetDefaultObjectSizeWidth(nIntVal);
    if (sal_Int32 nIntVal; aValues[SCMISCOPT_DEFOBJHEIGHT] >>= nIntVal)
        SetDefaultObjectSizeHeight(nIntVal);
    SetShowSharedDocumentWarning(
        ScUnoHelpFunctions::GetBoolFromAny(aValues[SCMISCOPT_SHOWSHAREDDOCWARN]));
}

void ScAppCfg::ReadCompatCfg()
{
    const Sequence<OUString> aNames = GetCompatPropertyNames();
    const Sequence<Any> aValues = aCompatItem.GetProperties(aNames);
    if (aValues.getLength() != aNames.getLength())
        return;

    sal_Int32 nIntVal = 0; // 0 = 'Default'
    aValues[SCCOMPATOPT_KEY_BINDING] >>= nIntVal;
    SetKeyBindingType(static_cast<ScOptionsUtil::KeyBindingType>(nIntVal));

    if (aValues.getLength() > SCCOMPATOPT_LINK_LIKE_MS)
        SetLinksInsertedLikeMSExcel(
            ScUnoHelpFunctions::GetBoolFromAny(aValues[SCCOMPATOPT_LINK_LIKE_MS]));
}

IMPL_LINK_NOARG(ScAppCfg, LayoutCommitHdl, ScLinkConfigItem&, void)
{
    Sequence<OUString> aNames = GetLayoutPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCLAYOUTOPT_MEASURE:
                pValues[nProp] <<= static_cast<sal_Int32>(GetAppMetric());
                break;
            case SCLAYOUTOPT_STATUSBAR:
                pValues[nProp] <<= lcl_ConvertStatusBarFuncSetToSingle( GetStatusFunc() );
                break;
            case SCLAYOUTOPT_ZOOMVAL:
                pValues[nProp] <<= static_cast<sal_Int32>(GetZoom());
                break;
            case SCLAYOUTOPT_ZOOMTYPE:
                pValues[nProp] <<= static_cast<sal_Int32>(GetZoomType());
                break;
            case SCLAYOUTOPT_SYNCZOOM:
                pValues[nProp] <<= GetSynchronizeZoom();
                break;
            case SCLAYOUTOPT_STATUSBARMULTI:
                pValues[nProp] <<= GetStatusFunc();
                break;
        }
    }
    aLayoutItem.PutProperties(aNames, aValues);
}

IMPL_LINK_NOARG(ScAppCfg, LayoutNotifyHdl, ScLinkConfigItem&, void) { ReadLayoutCfg(); }

IMPL_LINK_NOARG(ScAppCfg, InputCommitHdl, ScLinkConfigItem&, void)
{
    Sequence<OUString> aNames = GetInputPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCINPUTOPT_LASTFUNCS:
                lcl_GetLastFunctions( pValues[nProp], *this );
                break;
            case SCINPUTOPT_AUTOINPUT:
                pValues[nProp] <<= GetAutoComplete();
                break;
            case SCINPUTOPT_DET_AUTO:
                pValues[nProp] <<= GetDetectiveAuto();
                break;
        }
    }
    aInputItem.PutProperties(aNames, aValues);
}

IMPL_LINK_NOARG(ScAppCfg, InputNotifyHdl, ScLinkConfigItem&, void) { ReadInputCfg(); }

IMPL_LINK_NOARG(ScAppCfg, RevisionCommitHdl, ScLinkConfigItem&, void)
{
    Sequence<OUString> aNames = GetRevisionPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCREVISOPT_CHANGE:
                pValues[nProp] <<= GetTrackContentColor();
                break;
            case SCREVISOPT_INSERTION:
                pValues[nProp] <<= GetTrackInsertColor();
                break;
            case SCREVISOPT_DELETION:
                pValues[nProp] <<= GetTrackDeleteColor();
                break;
            case SCREVISOPT_MOVEDENTRY:
                pValues[nProp] <<= GetTrackMoveColor();
                break;
        }
    }
    aRevisionItem.PutProperties(aNames, aValues);
}

IMPL_LINK_NOARG(ScAppCfg, RevisionNotifyHdl, ScLinkConfigItem&, void) { ReadRevisionCfg(); }

IMPL_LINK_NOARG(ScAppCfg, ContentCommitHdl, ScLinkConfigItem&, void)
{
    Sequence<OUString> aNames = GetContentPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCCONTENTOPT_LINK:
                pValues[nProp] <<= static_cast<sal_Int32>(GetLinkMode());
                break;
        }
    }
    aContentItem.PutProperties(aNames, aValues);
}

IMPL_LINK_NOARG(ScAppCfg, ContentNotifyHdl, ScLinkConfigItem&, void) { ReadContentCfg(); }

IMPL_LINK_NOARG(ScAppCfg, SortListCommitHdl, ScLinkConfigItem&, void)
{
    Sequence<OUString> aNames = GetSortListPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCSORTLISTOPT_LIST:
                lcl_GetSortList( pValues[nProp] );
                break;
        }
    }
    aSortListItem.PutProperties(aNames, aValues);
}

IMPL_LINK_NOARG(ScAppCfg, SortListNotifyHdl, ScLinkConfigItem&, void) { ReadSortListCfg(); }

IMPL_LINK_NOARG(ScAppCfg, MiscCommitHdl, ScLinkConfigItem&, void)
{
    Sequence<OUString> aNames = GetMiscPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCMISCOPT_DEFOBJWIDTH:
                pValues[nProp] <<= GetDefaultObjectSizeWidth();
                break;
            case SCMISCOPT_DEFOBJHEIGHT:
                pValues[nProp] <<= GetDefaultObjectSizeHeight();
                break;
            case SCMISCOPT_SHOWSHAREDDOCWARN:
                pValues[nProp] <<= GetShowSharedDocumentWarning();
                break;
        }
    }
    aMiscItem.PutProperties(aNames, aValues);
}

IMPL_LINK_NOARG(ScAppCfg, MiscNotifyHdl, ScLinkConfigItem&, void) { ReadMiscCfg(); }

IMPL_LINK_NOARG(ScAppCfg, CompatCommitHdl, ScLinkConfigItem&, void)
{
    Sequence<OUString> aNames = GetCompatPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for (int nProp = 0; nProp < aNames.getLength(); ++nProp)
    {
        switch(nProp)
        {
            case SCCOMPATOPT_KEY_BINDING:
                pValues[nProp] <<= static_cast<sal_Int32>(GetKeyBindingType());
            break;
            case SCCOMPATOPT_LINK_LIKE_MS:
                pValues[nProp] <<= GetLinksInsertedLikeMSExcel();
                break;
        }
    }
    aCompatItem.PutProperties(aNames, aValues);
}

IMPL_LINK_NOARG(ScAppCfg, CompatNotifyHdl, ScLinkConfigItem&, void) { ReadCompatCfg(); }

void ScAppCfg::SetOptions( const ScAppOptions& rNew )
{
    *static_cast<ScAppOptions*>(this) = rNew;

    aLayoutItem.SetModified();
    aInputItem.SetModified();
    aRevisionItem.SetModified();
    aContentItem.SetModified();
    aSortListItem.SetModified();
    aMiscItem.SetModified();
    aCompatItem.SetModified();

    aLayoutItem.Commit();
    aInputItem.Commit();
    aRevisionItem.Commit();
    aContentItem.Commit();
    aSortListItem.Commit();
    aMiscItem.Commit();
    aCompatItem.Commit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
