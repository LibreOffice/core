/*************************************************************************
 *
 *  $RCSfile: appoptio.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:17 $
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

#include "cfgids.hxx"
#include "appoptio.hxx"
#include "rechead.hxx"
#include "scresid.hxx"
#include "global.hxx"
#include "userlist.hxx"
#include "sc.hrc"
#include "compiler.hrc"

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
// CfgItem fuer App-Optionen
//==================================================================

ScAppCfg::ScAppCfg() : SfxConfigItem( SCCFG_APP )
{
}

//------------------------------------------------------------------------

int __EXPORT ScAppCfg::Load( SvStream& rStream )
{
    USHORT nVersion;

    rStream >> nVersion;
    rStream >> (ScAppOptions&)*this;

    SetDefault( FALSE );

    return ( nVersion == SC_VERSION )
                ? SfxConfigItem::ERR_OK
                : SfxConfigItem::WARNING_VERSION;
}

//------------------------------------------------------------------------

BOOL __EXPORT ScAppCfg::Store( SvStream& rStream)
{
    rStream << SC_VERSION;
    rStream << *this;

    SetDefault( FALSE );

    return SfxConfigItem::ERR_OK;
}

//------------------------------------------------------------------------

void __EXPORT ScAppCfg::UseDefault()
{
    SetDefaults();
    SetDefault( TRUE );
}


//------------------------------------------------------------------------

String __EXPORT ScAppCfg::GetName() const
{
    return String( ScResId( SCSTR_CFG_APP ) );
}



