/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xeroot.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:01:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif

#ifndef _SVSTOR_HXX
#include <so3/svstor.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif

#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif
#ifndef SC_XEHELPER_HXX
#include "xehelper.hxx"
#endif
#ifndef SC_XEFORMULA_HXX
#include "xeformula.hxx"
#endif
#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif
#ifndef SC_XENAME_HXX
#include "xename.hxx"
#endif
#ifndef SC_XESTYLE_HXX
#include "xestyle.hxx"
#endif
#ifndef SC_XECONTENT_HXX
#include "xecontent.hxx"
#endif
#ifndef SC_XEPIVOT_HXX
#include "xepivot.hxx"
#endif

// for filter manager
#include "excrecds.hxx"

// Global data ================================================================

XclExpRootData::XclExpRootData( XclBiff eBiff, SfxMedium& rMedium,
        SotStorageRef xRootStrg, ScDocument& rDoc, CharSet eCharSet ) :
    XclRootData( eBiff, rMedium, xRootStrg, rDoc, eCharSet, true )
{
    SvtSaveOptions aSaveOpt;
    mbRelUrl = mrMedium.IsRemote() ? aSaveOpt.IsSaveRelINet() : aSaveOpt.IsSaveRelFSys();
}

XclExpRootData::~XclExpRootData()
{
}

// ----------------------------------------------------------------------------

XclExpRoot::XclExpRoot( XclExpRootData& rExpRootData ) :
    XclRoot( rExpRootData ),
    mrExpData( rExpRootData )
{
}

XclExpTabInfo& XclExpRoot::GetTabInfo() const
{
    DBG_ASSERT( mrExpData.mxTabInfo.is(), "XclExpRoot::GetTabInfo - missing object (wrong BIFF?)" );
    return *mrExpData.mxTabInfo;
}

XclExpAddressConverter& XclExpRoot::GetAddressConverter() const
{
    DBG_ASSERT( mrExpData.mxAddrConv.is(), "XclExpRoot::GetAddressConverter - missing object (wrong BIFF?)" );
    return *mrExpData.mxAddrConv;
}

XclExpFormulaCompiler& XclExpRoot::GetFormulaCompiler() const
{
    DBG_ASSERT( mrExpData.mxFmlaComp.is(), "XclExpRoot::GetFormulaCompiler - missing object (wrong BIFF?)" );
    return *mrExpData.mxFmlaComp;
}

XclExpProgressBar& XclExpRoot::GetProgressBar() const
{
    DBG_ASSERT( mrExpData.mxProgress.is(), "XclExpRoot::GetProgressBar - missing object (wrong BIFF?)" );
    return *mrExpData.mxProgress;
}

XclExpSst& XclExpRoot::GetSst() const
{
    DBG_ASSERT( mrExpData.mxSst.is(), "XclExpRoot::GetSst - missing object (wrong BIFF?)" );
    return *mrExpData.mxSst;
}

XclExpPalette& XclExpRoot::GetPalette() const
{
    DBG_ASSERT( mrExpData.mxPalette.is(), "XclExpRoot::GetPalette - missing object (wrong BIFF?)" );
    return *mrExpData.mxPalette;
}

XclExpFontBuffer& XclExpRoot::GetFontBuffer() const
{
    DBG_ASSERT( mrExpData.mxFontBfr.is(), "XclExpRoot::GetFontBuffer - missing object (wrong BIFF?)" );
    return *mrExpData.mxFontBfr;
}

XclExpNumFmtBuffer& XclExpRoot::GetNumFmtBuffer() const
{
    DBG_ASSERT( mrExpData.mxNumFmtBfr.is(), "XclExpRoot::GetNumFmtBuffer - missing object (wrong BIFF?)" );
    return *mrExpData.mxNumFmtBfr;
}

XclExpXFBuffer& XclExpRoot::GetXFBuffer() const
{
    DBG_ASSERT( mrExpData.mxXFBfr.is(), "XclExpRoot::GetXFBuffer - missing object (wrong BIFF?)" );
    return *mrExpData.mxXFBfr;
}

XclExpLinkManager& XclExpRoot::GetGlobalLinkManager() const
{
    DBG_ASSERT( mrExpData.mxGlobLinkMgr.is(), "XclExpRoot::GetGlobalLinkManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxGlobLinkMgr;
}

XclExpLinkManager& XclExpRoot::GetLocalLinkManager() const
{
    DBG_ASSERT( GetLocalLinkMgrRef().is(), "XclExpRoot::GetLocalLinkManager - missing object (wrong BIFF?)" );
    return *GetLocalLinkMgrRef();
}

XclExpNameManager& XclExpRoot::GetNameManager() const
{
    DBG_ASSERT( mrExpData.mxNameMgr.is(), "XclExpRoot::GetNameManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxNameMgr;
}

XclExpFilterManager& XclExpRoot::GetFilterManager() const
{
    DBG_ASSERT( mrExpData.mxFilterMgr.is(), "XclExpRoot::GetFilterManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxFilterMgr;
}

XclExpPivotTableManager& XclExpRoot::GetPivotTableManager() const
{
    DBG_ASSERT( mrExpData.mxPTableMgr.is(), "XclExpRoot::GetPivotTableManager - missing object (wrong BIFF?)" );
    return *mrExpData.mxPTableMgr;
}

void XclExpRoot::InitializeConvert()
{
    mrExpData.mxTabInfo.reset( new XclExpTabInfo( GetRoot() ) );
    mrExpData.mxAddrConv.reset( new XclExpAddressConverter( GetRoot() ) );
    mrExpData.mxFmlaComp.reset( new XclExpFormulaCompiler( GetRoot() ) );
    mrExpData.mxProgress.reset( new XclExpProgressBar( GetRoot() ) );

    GetProgressBar().Initialize();
}

void XclExpRoot::InitializeGlobals()
{
    SetCurrScTab( SCTAB_GLOBAL );

    if( GetBiff() >= EXC_BIFF5 )
    {
        mrExpData.mxPalette.reset( new XclExpPalette( GetRoot() ) );
        mrExpData.mxFontBfr.reset( new XclExpFontBuffer( GetRoot() ) );
        mrExpData.mxNumFmtBfr.reset( new XclExpNumFmtBuffer( GetRoot() ) );
        mrExpData.mxXFBfr.reset( new XclExpXFBuffer( GetRoot() ) );
        mrExpData.mxGlobLinkMgr.reset( new XclExpLinkManager( GetRoot() ) );
        mrExpData.mxNameMgr.reset( new XclExpNameManager( GetRoot() ) );
    }

    if( GetBiff() == EXC_BIFF8 )
    {
        mrExpData.mxSst.reset( new XclExpSst );
        mrExpData.mxFilterMgr.reset( new XclExpFilterManager( GetRoot() ) );
        mrExpData.mxPTableMgr.reset( new XclExpPivotTableManager( GetRoot() ) );
        // BIFF8: only one link manager for all sheets
        mrExpData.mxLocLinkMgr = mrExpData.mxGlobLinkMgr;
    }

    GetXFBuffer().Initialize();
    GetNameManager().Initialize();
}

void XclExpRoot::InitializeTable( SCTAB nScTab )
{
    SetCurrScTab( nScTab );
    if( GetBiff() == EXC_BIFF5 )
    {
        // local link manager per sheet
        mrExpData.mxLocLinkMgr.reset( new XclExpLinkManager( GetRoot() ) );
    }
}

void XclExpRoot::InitializeSave()
{
    GetPalette().Finalize();
    GetXFBuffer().Finalize();
}

XclExpRecordRef XclExpRoot::CreateRecord( sal_uInt16 nRecId ) const
{
    XclExpRecordRef xRec;
    switch( nRecId )
    {
        case EXC_ID_PALETTE:        xRec = mrExpData.mxPalette;     break;
        case EXC_ID_FONTLIST:       xRec = mrExpData.mxFontBfr;     break;
        case EXC_ID_FORMATLIST:     xRec = mrExpData.mxNumFmtBfr;   break;
        case EXC_ID_XFLIST:         xRec = mrExpData.mxXFBfr;       break;
        case EXC_ID_SST:            xRec = mrExpData.mxSst;         break;
        case EXC_ID_EXTERNSHEET:    xRec = GetLocalLinkMgrRef();    break;
        case EXC_ID_NAME:           xRec = mrExpData.mxNameMgr;     break;
    }
    DBG_ASSERT( xRec.is(), "XclExpRoot::CreateRecord - unknown record ID or missing object" );
    return xRec;
}

XclExpRootData::XclExpLinkMgrRef XclExpRoot::GetLocalLinkMgrRef() const
{
    return IsInGlobals() ? mrExpData.mxGlobLinkMgr : mrExpData.mxLocLinkMgr;
}

// ============================================================================

