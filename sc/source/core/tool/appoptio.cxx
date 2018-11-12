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

#include <vcl/svapp.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <appoptio.hxx>
#include <rechead.hxx>
#include <global.hxx>
#include <userlist.hxx>
#include <sc.hrc>
#include <formula/compiler.hxx>
#include <miscuno.hxx>
#include <memory>
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

static void lcl_SetLastFunctions( ScAppOptions& rOpt, const Any& rValue )
{
    Sequence<sal_Int32> aSeq;
    if ( rValue >>= aSeq )
    {
        sal_Int32 nCount = aSeq.getLength();
        if ( nCount < SAL_MAX_UINT16 )
        {
            const sal_Int32* pArray = aSeq.getConstArray();
            std::unique_ptr<sal_uInt16[]> pUShorts(new sal_uInt16[nCount]);
            for (sal_Int32 i=0; i<nCount; i++)
                pUShorts[i] = static_cast<sal_uInt16>(pArray[i]);

            rOpt.SetLRUFuncList( pUShorts.get(), sal::static_int_cast<sal_uInt16>(nCount) );
        }
    }
}

static void lcl_GetLastFunctions( Any& rDest, const ScAppOptions& rOpt )
{
    long nCount = rOpt.GetLRUFuncListCount();
    sal_uInt16* pUShorts = rOpt.GetLRUFuncList();
    if ( nCount && pUShorts )
    {
        Sequence<sal_Int32> aSeq( nCount );
        sal_Int32* pArray = aSeq.getArray();
        for (long i=0; i<nCount; i++)
            pArray[i] = pUShorts[i];
        rDest <<= aSeq;
    }
    else
        rDest <<= Sequence<sal_Int32>(0);   // empty
}

static void lcl_SetSortList( const Any& rValue )
{
    Sequence<OUString> aSeq;
    if ( rValue >>= aSeq )
    {
        long nCount = aSeq.getLength();
        const OUString* pArray = aSeq.getConstArray();
        ScUserList aList;

        //  if setting is "default", keep default values from ScUserList ctor
        //TODO: mark "default" in a safe way
        bool bDefault = ( nCount == 1 && pArray[0] == "NULL" );

        if (!bDefault)
        {
            aList.clear();

            for (long i=0; i<nCount; i++)
            {
                ScUserListData* pNew = new ScUserListData( pArray[i] );
                aList.push_back(pNew);
            }
        }

        ScGlobal::SetUserList( &aList );
    }
}

static void lcl_GetSortList( Any& rDest )
{
    const ScUserList* pUserList = ScGlobal::GetUserList();
    if (pUserList)
    {
        size_t nCount = pUserList->size();
        Sequence<OUString> aSeq( nCount );
        OUString* pArray = aSeq.getArray();
        for (size_t i=0; i<nCount; ++i)
            pArray[i] = (*pUserList)[sal::static_int_cast<sal_uInt16>(i)].GetString();
        rDest <<= aSeq;
    }
    else
        rDest <<= Sequence<OUString>(0);    // empty
}

#define CFGPATH_LAYOUT      "Office.Calc/Layout"

#define SCLAYOUTOPT_MEASURE         0
#define SCLAYOUTOPT_STATUSBAR       1
#define SCLAYOUTOPT_ZOOMVAL         2
#define SCLAYOUTOPT_ZOOMTYPE        3
#define SCLAYOUTOPT_SYNCZOOM        4
#define SCLAYOUTOPT_STATUSBARMULTI  5

#define CFGPATH_INPUT       "Office.Calc/Input"

#define SCINPUTOPT_LASTFUNCS        0
#define SCINPUTOPT_AUTOINPUT        1
#define SCINPUTOPT_DET_AUTO         2

#define CFGPATH_REVISION    "Office.Calc/Revision/Color"

#define SCREVISOPT_CHANGE           0
#define SCREVISOPT_INSERTION        1
#define SCREVISOPT_DELETION         2
#define SCREVISOPT_MOVEDENTRY       3

#define CFGPATH_CONTENT     "Office.Calc/Content/Update"

#define SCCONTENTOPT_LINK           0

#define CFGPATH_SORTLIST    "Office.Calc/SortList"

#define SCSORTLISTOPT_LIST          0

#define CFGPATH_MISC        "Office.Calc/Misc"

#define SCMISCOPT_DEFOBJWIDTH       0
#define SCMISCOPT_DEFOBJHEIGHT      1
#define SCMISCOPT_SHOWSHAREDDOCWARN 2

#define CFGPATH_COMPAT      "Office.Calc/Compatibility"

#define SCCOMPATOPT_KEY_BINDING     0

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
        if ( nFuncSet & ( 1 << nFunc ) )
            return nFunc;
    return 0;
}

Sequence<OUString> ScAppCfg::GetLayoutPropertyNames()
{
    const bool bIsMetric = ScOptionsUtil::IsMetricSystem();

    return {(bIsMetric ? OUString("Other/MeasureUnit/Metric")
                       : OUString("Other/MeasureUnit/NonMetric")),  // SCLAYOUTOPT_MEASURE
             "Other/StatusbarFunction",                             // SCLAYOUTOPT_STATUSBAR
             "Zoom/Value",                                          // SCLAYOUTOPT_ZOOMVAL
             "Zoom/Type",                                           // SCLAYOUTOPT_ZOOMTYPE
             "Zoom/Synchronize",                                    // SCLAYOUTOPT_SYNCZOOM
             "Other/StatusbarMultiFunction"};                       // SCLAYOUTOPT_STATUSBARMULTI
}

Sequence<OUString> ScAppCfg::GetInputPropertyNames()
{
    return {"LastFunctions",            // SCINPUTOPT_LASTFUNCS
            "AutoInput",                // SCINPUTOPT_AUTOINPUT
            "DetectiveAuto"};           // SCINPUTOPT_DET_AUTO
}

Sequence<OUString> ScAppCfg::GetRevisionPropertyNames()
{
    return {"Change",                   // SCREVISOPT_CHANGE
            "Insertion",                // SCREVISOPT_INSERTION
            "Deletion",                 // SCREVISOPT_DELETION
            "MovedEntry"};              // SCREVISOPT_MOVEDENTRY
}

Sequence<OUString> ScAppCfg::GetContentPropertyNames()
{
    return {"Link"};                    // SCCONTENTOPT_LINK
}

Sequence<OUString> ScAppCfg::GetSortListPropertyNames()
{
    return {"List"};                    // SCSORTLISTOPT_LIST
}

Sequence<OUString> ScAppCfg::GetMiscPropertyNames()
{
    return {"DefaultObjectSize/Width",      // SCMISCOPT_DEFOBJWIDTH
            "DefaultObjectSize/Height",     // SCMISCOPT_DEFOBJHEIGHT
            "SharedDocument/ShowWarning"};  // SCMISCOPT_SHOWSHAREDDOCWARN
}

Sequence<OUString> ScAppCfg::GetCompatPropertyNames()
{
    return {"KeyBindings/BaseGroup"};   // SCCOMPATOPT_KEY_BINDING
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
    sal_Int32 nIntVal = 0;

    Sequence<OUString> aNames;
    Sequence<Any> aValues;
    const Any* pValues = nullptr;

    aNames = GetLayoutPropertyNames();
    aValues = aLayoutItem.GetProperties(aNames);
    aLayoutItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        sal_uInt32 nStatusBarFuncSingle = 0;
        sal_uInt32 nStatusBarFuncMulti = 0;
        sal_uInt32 nUIntValTmp = 0;
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            OSL_ENSURE(pValues[nProp].hasValue(), "property value missing");
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCLAYOUTOPT_MEASURE:
                        if (pValues[nProp] >>= nIntVal) SetAppMetric( static_cast<FieldUnit>(nIntVal) );
                        break;
                    case SCLAYOUTOPT_STATUSBAR:
                        if ( pValues[SCLAYOUTOPT_STATUSBAR] >>= nUIntValTmp )
                            nStatusBarFuncSingle = nUIntValTmp;
                        break;
                    case SCLAYOUTOPT_STATUSBARMULTI:
                        if ( pValues[SCLAYOUTOPT_STATUSBARMULTI] >>= nUIntValTmp )
                            nStatusBarFuncMulti = nUIntValTmp;
                        break;
                    case SCLAYOUTOPT_ZOOMVAL:
                        if (pValues[nProp] >>= nIntVal) SetZoom( static_cast<sal_uInt16>(nIntVal) );
                        break;
                    case SCLAYOUTOPT_ZOOMTYPE:
                        if (pValues[nProp] >>= nIntVal) SetZoomType( static_cast<SvxZoomType>(nIntVal) );
                        break;
                    case SCLAYOUTOPT_SYNCZOOM:
                        SetSynchronizeZoom( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                }
            }
        }

        if ( nStatusBarFuncMulti != SCLAYOUTOPT_STATUSBARMULTI_DEFAULTVAL )
            SetStatusFunc( nStatusBarFuncMulti );
        else if ( nStatusBarFuncSingle != SCLAYOUTOPT_STATUSBAR_DEFAULTVAL &&
                  nStatusBarFuncSingle != SCLAYOUTOPT_STATUSBAR_DEFAULTVAL_LEGACY )
        {
            if ( nStatusBarFuncSingle )
                SetStatusFunc( 1 << nStatusBarFuncSingle );
            else
                SetStatusFunc( 0 );
        }
        else
            SetStatusFunc( SCLAYOUTOPT_STATUSBARMULTI_DEFAULTVAL );
    }
    aLayoutItem.SetCommitLink( LINK( this, ScAppCfg, LayoutCommitHdl ) );

    aNames = GetInputPropertyNames();
    aValues = aInputItem.GetProperties(aNames);
    aInputItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            OSL_ENSURE(pValues[nProp].hasValue(), "property value missing");
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCINPUTOPT_LASTFUNCS:
                        lcl_SetLastFunctions( *this, pValues[nProp] );
                        break;
                    case SCINPUTOPT_AUTOINPUT:
                        SetAutoComplete( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                    case SCINPUTOPT_DET_AUTO:
                        SetDetectiveAuto( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                }
            }
        }
    }
    aInputItem.SetCommitLink( LINK( this, ScAppCfg, InputCommitHdl ) );

    aNames = GetRevisionPropertyNames();
    aValues = aRevisionItem.GetProperties(aNames);
    aRevisionItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            OSL_ENSURE(pValues[nProp].hasValue(), "property value missing");
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCREVISOPT_CHANGE:
                        if (pValues[nProp] >>= nIntVal) SetTrackContentColor( Color(nIntVal) );
                        break;
                    case SCREVISOPT_INSERTION:
                        if (pValues[nProp] >>= nIntVal) SetTrackInsertColor( Color(nIntVal) );
                        break;
                    case SCREVISOPT_DELETION:
                        if (pValues[nProp] >>= nIntVal) SetTrackDeleteColor( Color(nIntVal) );
                        break;
                    case SCREVISOPT_MOVEDENTRY:
                        if (pValues[nProp] >>= nIntVal) SetTrackMoveColor( Color(nIntVal) );
                        break;
                }
            }
        }
    }
    aRevisionItem.SetCommitLink( LINK( this, ScAppCfg, RevisionCommitHdl ) );

    aNames = GetContentPropertyNames();
    aValues = aContentItem.GetProperties(aNames);
    aContentItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            OSL_ENSURE(pValues[nProp].hasValue(), "property value missing");
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCCONTENTOPT_LINK:
                        if (pValues[nProp] >>= nIntVal) SetLinkMode( static_cast<ScLkUpdMode>(nIntVal) );
                        break;
                }
            }
        }
    }
    aContentItem.SetCommitLink( LINK( this, ScAppCfg, ContentCommitHdl ) );

    aNames = GetSortListPropertyNames();
    aValues = aSortListItem.GetProperties(aNames);
    aSortListItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            OSL_ENSURE(pValues[nProp].hasValue(), "property value missing");
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCSORTLISTOPT_LIST:
                        lcl_SetSortList( pValues[nProp] );
                        break;
                }
            }
        }
    }
    aSortListItem.SetCommitLink( LINK( this, ScAppCfg, SortListCommitHdl ) );

    aNames = GetMiscPropertyNames();
    aValues = aMiscItem.GetProperties(aNames);
    aMiscItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            OSL_ENSURE(pValues[nProp].hasValue(), "property value missing");
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCMISCOPT_DEFOBJWIDTH:
                        if (pValues[nProp] >>= nIntVal) SetDefaultObjectSizeWidth( nIntVal );
                        break;
                    case SCMISCOPT_DEFOBJHEIGHT:
                        if (pValues[nProp] >>= nIntVal) SetDefaultObjectSizeHeight( nIntVal );
                        break;
                    case SCMISCOPT_SHOWSHAREDDOCWARN:
                        SetShowSharedDocumentWarning( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                }
            }
        }
    }
    aMiscItem.SetCommitLink( LINK( this, ScAppCfg, MiscCommitHdl ) );

    aNames = GetCompatPropertyNames();
    aValues = aCompatItem.GetProperties(aNames);
    aCompatItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    if (aValues.getLength() == aNames.getLength())
    {
        for (int nProp = 0; nProp < aNames.getLength(); ++nProp)
        {
            switch (nProp)
            {
                case SCCOMPATOPT_KEY_BINDING:
                {
                    nIntVal = 0; // 0 = 'Default'
                    pValues[nProp] >>= nIntVal;
                    SetKeyBindingType(static_cast<ScOptionsUtil::KeyBindingType>(nIntVal));
                }
                break;
            }
        }
    }
    aCompatItem.SetCommitLink( LINK(this, ScAppCfg, CompatCommitHdl) );
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
        }
    }
    aCompatItem.PutProperties(aNames, aValues);
}

void ScAppCfg::SetOptions( const ScAppOptions& rNew )
{
    *static_cast<ScAppOptions*>(this) = rNew;
    OptionsChanged();
}

void ScAppCfg::OptionsChanged()
{
    aLayoutItem.SetModified();
    aInputItem.SetModified();
    aRevisionItem.SetModified();
    aContentItem.SetModified();
    aSortListItem.SetModified();
    aMiscItem.SetModified();
    aCompatItem.SetModified();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
