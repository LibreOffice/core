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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------

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
using namespace rtl;
using namespace com::sun::star::uno;

// STATIC DATA -----------------------------------------------------------

#define SC_VERSION ((sal_uInt16)304)

//========================================================================
//      ScAppOptions - Applikations-Optionen
//========================================================================

ScAppOptions::ScAppOptions() : pLRUList( NULL )
{
    SetDefaults();
}

//------------------------------------------------------------------------

ScAppOptions::ScAppOptions( const ScAppOptions& rCpy ) : pLRUList( NULL )
{
    *this = rCpy;
}

//------------------------------------------------------------------------

ScAppOptions::~ScAppOptions()
{
    delete [] pLRUList;
}

//------------------------------------------------------------------------

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
}

//------------------------------------------------------------------------

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
    return *this;
}

//------------------------------------------------------------------------

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

//==================================================================
//  Config Item containing app options
//==================================================================

void lcl_SetLastFunctions( ScAppOptions& rOpt, const Any& rValue )
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

void lcl_GetLastFunctions( Any& rDest, const ScAppOptions& rOpt )
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

void lcl_SetSortList( const Any& rValue )
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
            aList.FreeAll();

            for (long i=0; i<nCount; i++)
            {
                ScUserListData* pNew = new ScUserListData( pArray[i] );
                if ( !aList.Insert(pNew) )
                    delete pNew;
            }
        }

        ScGlobal::SetUserList( &aList );
    }
}

void lcl_GetSortList( Any& rDest )
{
    const ScUserList* pUserList = ScGlobal::GetUserList();
    if (pUserList)
    {
        long nCount = pUserList->GetCount();
        Sequence<OUString> aSeq( nCount );
        OUString* pArray = aSeq.getArray();
        for (long i=0; i<nCount; i++)
            pArray[i] = (*pUserList)[sal::static_int_cast<sal_uInt16>(i)]->GetString();
        rDest <<= aSeq;
    }
    else
        rDest <<= Sequence<OUString>(0);    // empty
}

//------------------------------------------------------------------

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
        pNames[SCLAYOUTOPT_MEASURE] = OUString::createFromAscii( "Other/MeasureUnit/Metric" );

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


ScAppCfg::ScAppCfg() :
    aLayoutItem( OUString::createFromAscii( CFGPATH_LAYOUT ) ),
    aInputItem( OUString::createFromAscii( CFGPATH_INPUT ) ),
    aRevisionItem( OUString::createFromAscii( CFGPATH_REVISION ) ),
    aContentItem( OUString::createFromAscii( CFGPATH_CONTENT ) ),
    aSortListItem( OUString::createFromAscii( CFGPATH_SORTLIST ) ),
    aMiscItem( OUString::createFromAscii( CFGPATH_MISC ) )
{
    sal_Int32 nIntVal = 0;

    Sequence<OUString> aNames;
    Sequence<Any> aValues;
    const Any* pValues = NULL;

    aNames = GetLayoutPropertyNames();
    aValues = aLayoutItem.GetProperties(aNames);
    aLayoutItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
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
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
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
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
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
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
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
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
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
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
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
}

IMPL_LINK( ScAppCfg, LayoutCommitHdl, void *, EMPTYARG )
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

IMPL_LINK( ScAppCfg, InputCommitHdl, void *, EMPTYARG )
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

IMPL_LINK( ScAppCfg, RevisionCommitHdl, void *, EMPTYARG )
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

IMPL_LINK( ScAppCfg, ContentCommitHdl, void *, EMPTYARG )
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

IMPL_LINK( ScAppCfg, SortListCommitHdl, void *, EMPTYARG )
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

IMPL_LINK( ScAppCfg, MiscCommitHdl, void *, EMPTYARG )
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
}


