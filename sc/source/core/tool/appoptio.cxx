/*************************************************************************
 *
 *  $RCSfile: appoptio.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-22 07:56:13 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

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
#include "compiler.hrc"
#include "miscuno.hxx"

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

// STATIC DATA -----------------------------------------------------------

#define SC_VERSION ((USHORT)304)

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
    MeasurementSystem eSys = Application::GetAppInternational().GetMeasurementSystem();
    if ( eSys == MEASURE_METRIC )
        eMetric     = FUNIT_CM;             // fuer die zivilisierte Welt
    else
        eMetric     = FUNIT_INCH;           // fuer die komischen Amis

    nZoom           = 100;
    eZoomType       = SVX_ZOOM_PERCENT;
    nStatusFunc     = SUBTOTAL_FUNC_SUM;
    bAutoComplete   = TRUE;
    bDetectiveAuto  = TRUE;

    delete [] pLRUList;
    pLRUList = new USHORT[5];               // sinnvoll vorbelegen
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

}

//------------------------------------------------------------------------

const ScAppOptions& ScAppOptions::operator=( const ScAppOptions& rCpy )
{
    eMetric         = rCpy.eMetric;
    eZoomType       = rCpy.eZoomType;
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
    return *this;
}

//------------------------------------------------------------------------

SvStream& operator>>( SvStream& rStream, ScAppOptions& rOpt )
{
    rOpt.SetDefaults();

    ScReadHeader aHdr( rStream );

    BYTE n;
    rStream >> n; rOpt.eMetric = (FieldUnit)n;

    if ( aHdr.BytesLeft() )
    {
        rStream >> n; rOpt.nLRUFuncCount = (USHORT)n;

        delete [] rOpt.pLRUList;
        rOpt.pLRUList = new USHORT[rOpt.nLRUFuncCount];

        for ( USHORT i=0; i<rOpt.nLRUFuncCount; i++ )
            rStream >> rOpt.pLRUList[i];
    }

    // ab 31.10.95: globale benutzerdefinierte Listen einlesen :-/
    //              (kleiner Hack :-/)
    if ( aHdr.BytesLeft() )
    {
        ScUserList* pUserList = ScGlobal::GetUserList();
        pUserList->Load( rStream );
    }

    // ab 11.12.95 (304)
    // Zoom-Faktor
    if ( aHdr.BytesLeft() )
    {
        USHORT e;
        rStream >> e; rOpt.eZoomType = (SvxZoomType)e;
        rStream >> rOpt.nZoom;
    }

    // ab 23.5.96: Funktion fuer Statusbar-Controller, Flag fuer Auto-Eingabe
    if ( aHdr.BytesLeft() )
    {
        rStream >> rOpt.nStatusFunc;
        rStream >> rOpt.bAutoComplete;
    }

    // ab 15.3.98: Farben fuer Change-Tracking
    if ( aHdr.BytesLeft() )
    {
        rStream >> rOpt.nTrackContentColor;
        rStream >> rOpt.nTrackInsertColor;
        rStream >> rOpt.nTrackDeleteColor;
        rStream >> rOpt.nTrackMoveColor;
    }

    // ab 22.6.98: Automatisches Detektiv-Update
    if ( aHdr.BytesLeft() )
        rStream >> rOpt.bDetectiveAuto;

    if ( aHdr.BytesLeft() )
    {
        BYTE nLinkMode;
        rStream >> nLinkMode;
        rOpt.eLinkMode=(ScLkUpdMode) nLinkMode;
    }

    return rStream;
}

//------------------------------------------------------------------------

SvStream& operator<<( SvStream& rStream, const ScAppOptions& rOpt )
{
    ScWriteHeader aHdr( rStream, 25 );

    rStream << (BYTE)rOpt.eMetric;
    rStream << (BYTE)rOpt.nLRUFuncCount;

    if ( rOpt.nLRUFuncCount > 0 )
    {
        for ( USHORT i=0; i<rOpt.nLRUFuncCount; i++ )
            rStream << rOpt.pLRUList[i];
    }

    // ab 31.10.95: globale benutzerdefinierte Listen speichern
    //              (kleiner Hack :-/)
    ScUserList* pUserList = ScGlobal::GetUserList();
    pUserList->Store( rStream );

    // ab 11.12.95 (304)
    // Zoom-Faktor
    rStream << (USHORT)rOpt.eZoomType;
    rStream << rOpt.nZoom;

    // ab 23.5.96: Funktion fuer Statusbar-Controller, Flag fuer Auto-Eingabe
    rStream << rOpt.nStatusFunc;
    rStream << rOpt.bAutoComplete;

    // ab 15.3.98: Farben fuer Change-Tracking
    rStream << rOpt.nTrackContentColor;
    rStream << rOpt.nTrackInsertColor;
    rStream << rOpt.nTrackDeleteColor;
    rStream << rOpt.nTrackMoveColor;

    // ab 22.6.98: Automatisches Detektiv-Update
    rStream << rOpt.bDetectiveAuto;
    rStream << (BYTE) rOpt.eLinkMode;

    return rStream;
}

//------------------------------------------------------------------------

void ScAppOptions::SetLRUFuncList( const USHORT* pList, const USHORT nCount )
{
    delete [] pLRUList;

    nLRUFuncCount = nCount;

    if ( nLRUFuncCount > 0 )
    {
        pLRUList = new USHORT[nLRUFuncCount];

        for ( USHORT i=0; i<nLRUFuncCount; i++ )
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
            USHORT* pUShorts = new USHORT[nCount];
            for (long i=0; i<nCount; i++)
                pUShorts[i] = (USHORT) pArray[i];

            rOpt.SetLRUFuncList( pUShorts, nCount );

            delete[] pUShorts;
        }
    }
}

void lcl_GetLastFunctions( Any& rDest, const ScAppOptions& rOpt )
{
    long nCount = rOpt.GetLRUFuncListCount();
    USHORT* pUShorts = rOpt.GetLRUFuncList();
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
        BOOL bDefault = ( nCount == 1 &&
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
            pArray[i] = (*pUserList)[i]->GetString();
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
#define SCLAYOUTOPT_COUNT           4

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


Sequence<OUString> ScAppCfg::GetLayoutPropertyNames()
{
    static const char* aPropNames[] =
    {
        "Other/MeasureUnit",        // SCLAYOUTOPT_MEASURE
        "Other/StatusbarFunction",  // SCLAYOUTOPT_STATUSBAR
        "Zoom/Value",               // SCLAYOUTOPT_ZOOMVAL
        "Zoom/Type"                 // SCLAYOUTOPT_ZOOMTYPE
    };
    Sequence<OUString> aNames(SCLAYOUTOPT_COUNT);
    OUString* pNames = aNames.getArray();
    for(int i = 0; i < SCLAYOUTOPT_COUNT; i++)
        pNames[i] = OUString::createFromAscii(aPropNames[i]);

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


ScAppCfg::ScAppCfg() :
    aLayoutItem( OUString::createFromAscii( CFGPATH_LAYOUT ) ),
    aInputItem( OUString::createFromAscii( CFGPATH_INPUT ) ),
    aRevisionItem( OUString::createFromAscii( CFGPATH_REVISION ) ),
    aContentItem( OUString::createFromAscii( CFGPATH_CONTENT ) ),
    aSortListItem( OUString::createFromAscii( CFGPATH_SORTLIST ) )
{
    sal_Int32 nIntVal;

    Sequence<OUString> aNames;
    Sequence<Any> aValues;
    const Any* pValues = NULL;

    aNames = GetLayoutPropertyNames();
    aValues = aLayoutItem.GetProperties(aNames);
    aLayoutItem.EnableNotification(aNames);
    pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing")
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCLAYOUTOPT_MEASURE:
                        if (pValues[nProp] >>= nIntVal) SetAppMetric( (FieldUnit) nIntVal );
                        break;
                    case SCLAYOUTOPT_STATUSBAR:
                        if (pValues[nProp] >>= nIntVal) SetStatusFunc( (USHORT) nIntVal );
                        break;
                    case SCLAYOUTOPT_ZOOMVAL:
                        if (pValues[nProp] >>= nIntVal) SetZoom( (USHORT) nIntVal );
                        break;
                    case SCLAYOUTOPT_ZOOMTYPE:
                        if (pValues[nProp] >>= nIntVal) SetZoomType( (SvxZoomType) nIntVal );
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
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing")
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
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing")
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case SCREVISOPT_CHANGE:
                        if (pValues[nProp] >>= nIntVal) SetTrackContentColor( (ULONG) nIntVal );
                        break;
                    case SCREVISOPT_INSERTION:
                        if (pValues[nProp] >>= nIntVal) SetTrackInsertColor( (ULONG) nIntVal );
                        break;
                    case SCREVISOPT_DELETION:
                        if (pValues[nProp] >>= nIntVal) SetTrackDeleteColor( (ULONG) nIntVal );
                        break;
                    case SCREVISOPT_MOVEDENTRY:
                        if (pValues[nProp] >>= nIntVal) SetTrackMoveColor( (ULONG) nIntVal );
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
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing")
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
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed")
    if(aValues.getLength() == aNames.getLength())
    {
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            DBG_ASSERT(pValues[nProp].hasValue(), "property value missing")
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
}

IMPL_LINK( ScAppCfg, LayoutCommitHdl, void *, EMPTYARG )
{
    Sequence<OUString> aNames = GetLayoutPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
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
        }
    }
    aLayoutItem.PutProperties(aNames, aValues);

    return 0;
}

IMPL_LINK( ScAppCfg, InputCommitHdl, void *, EMPTYARG )
{
    Sequence<OUString> aNames = GetInputPropertyNames();
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
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
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
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
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
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
    OUString* pNames = aNames.getArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

    const Type& rType = ::getBooleanCppuType();
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
}


