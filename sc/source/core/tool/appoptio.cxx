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
#include "cfgids.hxx"
#include "appoptio.hxx"
#include "rechead.hxx"
#include "scresid.hxx"
#include "global.hxx"
#include "userlist.hxx"
#include "sc.hrc"
#include <formula/compiler.hrc>
#include "miscuno.hxx"

using namespace utl;
using namespace com::sun::star::uno;

//      ScAppOptions - Applikations-Optionen

ScAppOptions::ScAppOptions() : pLRUList( NULL )
{
    SetDefaults();
}

ScAppOptions::ScAppOptions( const ScAppOptions& rCpy ) : pLRUList( NULL )
{
    *this = rCpy;
}

ScAppOptions::~ScAppOptions()
{
    delete [] pLRUList;
}

void ScAppOptions::SetDefaults()
{
    if ( ScOptionsUtil::IsMetricSystem() )
        eMetric     = FUNIT_CM;             // default for countries with metric system
    else
        eMetric     = FUNIT_INCH;           // default for others

    nZoom           = 100;
    eZoomType       = SVX_ZOOM_PERCENT;
    bSynchronizeZoom = sal_True;
    nStatusFunc     = SUBTOTAL_FUNC_SUM;
    bAutoComplete   = sal_True;
    bDetectiveAuto  = sal_True;

    delete [] pLRUList;
    pLRUList = new sal_uInt16[5];               // sinnvoll vorbelegen
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

const ScAppOptions& ScAppOptions::operator=( const ScAppOptions& rCpy )
{
    eMetric         = rCpy.eMetric;
    eZoomType       = rCpy.eZoomType;
    bSynchronizeZoom = rCpy.bSynchronizeZoom;
    nZoom           = rCpy.nZoom;
    SetLRUFuncList( rCpy.pLRUList, rCpy.nLRUFuncCount );
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
    delete [] pLRUList;

    nLRUFuncCount = nCount;

    if ( nLRUFuncCount > 0 )
    {
        pLRUList = new sal_uInt16[nLRUFuncCount];

        for ( sal_uInt16 i=0; i<nLRUFuncCount; i++ )
            pLRUList[i] = pList[i];
    }
    else
        pLRUList = NULL;
}

//  Config Item containing app options

static void lcl_SetLastFunctions( ScAppOptions& rOpt, const Any& rValue )
{
    Sequence<sal_Int32> aSeq;
    if ( rValue >>= aSeq )
    {
        long nCount = aSeq.getLength();
        if ( nCount < USHRT_MAX )
        {
            const sal_Int32* pArray = aSeq.getConstArray();
            sal_uInt16* pUShorts = new sal_uInt16[nCount];
            for (long i=0; i<nCount; i++)
                pUShorts[i] = (sal_uInt16) pArray[i];

            rOpt.SetLRUFuncList( pUShorts, sal::static_int_cast<sal_uInt16>(nCount) );

            delete[] pUShorts;
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
        //! mark "default" in a safe way
        sal_Bool bDefault = ( nCount == 1 &&
                        pArray[0].equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "NULL" ) ) );

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
            pArray[i] = (*pUserList)[sal::static_int_cast<sal_uInt16>(i)]->GetString();
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
#define SCLAYOUTOPT_COUNT           5

#define CFGPATH_INPUT       "Office.Calc/Input"

#define SCINPUTOPT_LASTFUNCS        0
#define SCINPUTOPT_AUTOINPUT        1
#define SCINPUTOPT_DET_AUTO         2
#define SCINPUTOPT_COUNT            3

#define CFGPATH_REVISION    "Office.Calc/Revision/Color"

#define SCREVISOPT_CHANGE           0
#define SCREVISOPT_INSERTION        1
#define SCREVISOPT_DELETION         2
#define SCREVISOPT_MOVEDENTRY       3
#define SCREVISOPT_COUNT            4

#define CFGPATH_CONTENT     "Office.Calc/Content/Update"

#define SCCONTENTOPT_LINK           0
#define SCCONTENTOPT_COUNT          1

#define CFGPATH_SORTLIST    "Office.Calc/SortList"

#define SCSORTLISTOPT_LIST          0
#define SCSORTLISTOPT_COUNT         1

#define CFGPATH_MISC        "Office.Calc/Misc"

#define SCMISCOPT_DEFOBJWIDTH       0
#define SCMISCOPT_DEFOBJHEIGHT      1
#define SCMISCOPT_SHOWSHAREDDOCWARN 2
#define SCMISCOPT_COUNT             3

#define CFGPATH_COMPAT      "Office.Calc/Compatibility"

#define SCCOMPATOPT_KEY_BINDING     0
#define SCCOMPATOPT_COUNT           1

Sequence<OUString> ScAppCfg::GetLayoutPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Other/MeasureUnit/NonMetric",  // SCLAYOUTOPT_MEASURE
        "Other/StatusbarFunction",      // SCLAYOUTOPT_STATUSBAR
        "Zoom/Value",                   // SCLAYOUTOPT_ZOOMVAL
        "Zoom/Type",                    // SCLAYOUTOPT_ZOOMTYPE
        "Zoom/Synchronize"              // SCLAYOUTOPT_SYNCZOOM
    };
    Sequence<OUString> aNames(SCLAYOUTOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCLAYOUTOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    //  adjust for metric system
    if (ScOptionsUtil::IsMetricSystem())
        pNames[SCLAYOUTOPT_MEASURE] = OUString( "Other/MeasureUnit/Metric" );

    return aNames;
}

Sequence<OUString> ScAppCfg::GetInputPropertyNames()
{
    static const char* aPropNames[] =
    {
        "LastFunctions",            // SCINPUTOPT_LASTFUNCS
        "AutoInput",                // SCINPUTOPT_AUTOINPUT
        "DetectiveAuto"             // SCINPUTOPT_DET_AUTO
    };
    Sequence<OUString> aNames(SCINPUTOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCINPUTOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

Sequence<OUString> ScAppCfg::GetRevisionPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Change",                   // SCREVISOPT_CHANGE
        "Insertion",                // SCREVISOPT_INSERTION
        "Deletion",                 // SCREVISOPT_DELETION
        "MovedEntry"                // SCREVISOPT_MOVEDENTRY
    };
    Sequence<OUString> aNames(SCREVISOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCREVISOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

Sequence<OUString> ScAppCfg::GetContentPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Link"                      // SCCONTENTOPT_LINK
    };
    Sequence<OUString> aNames(SCCONTENTOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCCONTENTOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

Sequence<OUString> ScAppCfg::GetSortListPropertyNames()
{
    static const char* aPropNames[] =
    {
        "List"                      // SCSORTLISTOPT_LIST
    };
    Sequence<OUString> aNames(SCSORTLISTOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCSORTLISTOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

Sequence<OUString> ScAppCfg::GetMiscPropertyNames()
{
    static const char* aPropNames[] =
    {
        "DefaultObjectSize/Width",      // SCMISCOPT_DEFOBJWIDTH
        "DefaultObjectSize/Height",     // SCMISCOPT_DEFOBJHEIGHT
        "SharedDocument/ShowWarning"    // SCMISCOPT_SHOWSHAREDDOCWARN
    };
    Sequence<OUString> aNames(SCMISCOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCMISCOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

Sequence<OUString> ScAppCfg::GetCompatPropertyNames()
{
    static const char* aPropNames[] =
    {
        "KeyBindings/BaseGroup"         // SCCOMPATOPT_KEY_BINDING
    };
    Sequence<OUString> aNames(SCCOMPATOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for (int i = 0; i < SCCOMPATOPT_COUNT; ++i)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

    return aNames;
}

ScAppCfg::ScAppCfg() :
    aLayoutItem( OUString( CFGPATH_LAYOUT ) ),
    aInputItem( OUString( CFGPATH_INPUT ) ),
    aRevisionItem( OUString( CFGPATH_REVISION ) ),
    aContentItem( OUString( CFGPATH_CONTENT ) ),
    aSortListItem( OUString( CFGPATH_SORTLIST ) ),
    aMiscItem( OUString( CFGPATH_MISC ) ),
    aCompatItem( OUString(CFGPATH_COMPAT ) )
{
    sal_Int32 nIntVal = 0;

    Sequence<OUString> aNames;
    Sequence<Any> aValues;
    const Any* pValues = NULL;

    aNames = GetLayoutPropertyNames();
    aValues = aLayoutItem.GetProperties(aNames);
    aLayoutItem.EnableNotification(aNames);
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
                    case SCLAYOUTOPT_MEASURE:
                        if (pValues[nProp] >>= nIntVal) SetAppMetric( (FieldUnit) nIntVal );
                        break;
                    case SCLAYOUTOPT_STATUSBAR:
                        if (pValues[nProp] >>= nIntVal) SetStatusFunc( (sal_uInt16) nIntVal );
                        break;
                    case SCLAYOUTOPT_ZOOMVAL:
                        if (pValues[nProp] >>= nIntVal) SetZoom( (sal_uInt16) nIntVal );
                        break;
                    case SCLAYOUTOPT_ZOOMTYPE:
                        if (pValues[nProp] >>= nIntVal) SetZoomType( (SvxZoomType) nIntVal );
                        break;
                    case SCLAYOUTOPT_SYNCZOOM:
                        SetSynchronizeZoom( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
                }
            }
        }
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
                        if (pValues[nProp] >>= nIntVal) SetTrackContentColor( (sal_uInt32) nIntVal );
                        break;
                    case SCREVISOPT_INSERTION:
                        if (pValues[nProp] >>= nIntVal) SetTrackInsertColor( (sal_uInt32) nIntVal );
                        break;
                    case SCREVISOPT_DELETION:
                        if (pValues[nProp] >>= nIntVal) SetTrackDeleteColor( (sal_uInt32) nIntVal );
                        break;
                    case SCREVISOPT_MOVEDENTRY:
                        if (pValues[nProp] >>= nIntVal) SetTrackMoveColor( (sal_uInt32) nIntVal );
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
                        if (pValues[nProp] >>= nIntVal) SetLinkMode( (ScLkUpdMode) nIntVal );
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
 IMPL_LINK_NOARG(ScAppCfg, LayoutCommitHdl)
{
    Sequence<OUString> aNames = GetLayoutPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCLAYOUTOPT_MEASURE:
                pValues[nProp] <<= (sal_Int32) GetAppMetric();
                break;
            case SCLAYOUTOPT_STATUSBAR:
                pValues[nProp] <<= (sal_Int32) GetStatusFunc();
                break;
            case SCLAYOUTOPT_ZOOMVAL:
                pValues[nProp] <<= (sal_Int32) GetZoom();
                break;
            case SCLAYOUTOPT_ZOOMTYPE:
                pValues[nProp] <<= (sal_Int32) GetZoomType();
                break;
            case SCLAYOUTOPT_SYNCZOOM:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetSynchronizeZoom() );
                break;
        }
    }
    aLayoutItem.PutProperties(aNames, aValues);

    return 0;
}

IMPL_LINK_NOARG(ScAppCfg, InputCommitHdl)
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
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetAutoComplete() );
                break;
            case SCINPUTOPT_DET_AUTO:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetDetectiveAuto() );
                break;
        }
    }
    aInputItem.PutProperties(aNames, aValues);

    return 0;
}

IMPL_LINK_NOARG(ScAppCfg, RevisionCommitHdl)
{
    Sequence<OUString> aNames = GetRevisionPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCREVISOPT_CHANGE:
                pValues[nProp] <<= (sal_Int32) GetTrackContentColor();
                break;
            case SCREVISOPT_INSERTION:
                pValues[nProp] <<= (sal_Int32) GetTrackInsertColor();
                break;
            case SCREVISOPT_DELETION:
                pValues[nProp] <<= (sal_Int32) GetTrackDeleteColor();
                break;
            case SCREVISOPT_MOVEDENTRY:
                pValues[nProp] <<= (sal_Int32) GetTrackMoveColor();
                break;
        }
    }
    aRevisionItem.PutProperties(aNames, aValues);

    return 0;
}

IMPL_LINK_NOARG(ScAppCfg, ContentCommitHdl)
{
    Sequence<OUString> aNames = GetContentPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCCONTENTOPT_LINK:
                pValues[nProp] <<= (sal_Int32) GetLinkMode();
                break;
        }
    }
    aContentItem.PutProperties(aNames, aValues);

    return 0;
}

IMPL_LINK_NOARG(ScAppCfg, SortListCommitHdl)
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

    return 0;
}

IMPL_LINK_NOARG(ScAppCfg, MiscCommitHdl)
{
    Sequence<OUString> aNames = GetMiscPropertyNames();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        switch(nProp)
        {
            case SCMISCOPT_DEFOBJWIDTH:
                pValues[nProp] <<= (sal_Int32) GetDefaultObjectSizeWidth();
                break;
            case SCMISCOPT_DEFOBJHEIGHT:
                pValues[nProp] <<= (sal_Int32) GetDefaultObjectSizeHeight();
                break;
            case SCMISCOPT_SHOWSHAREDDOCWARN:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetShowSharedDocumentWarning() );
                break;
        }
    }
    aMiscItem.PutProperties(aNames, aValues);

    return 0;
}

IMPL_LINK_NOARG(ScAppCfg, CompatCommitHdl)
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
    return 0;
}

void ScAppCfg::SetOptions( const ScAppOptions& rNew )
{
    *(ScAppOptions*)this = rNew;
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
