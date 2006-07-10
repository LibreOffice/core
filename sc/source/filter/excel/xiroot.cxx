/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xiroot.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:42:26 $
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

#ifndef SC_XIROOT_HXX
#include "xiroot.hxx"
#endif

#ifndef SC_ADDINCOL_HXX
#include "addincol.hxx"
#endif

#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif
#ifndef SC_XIFORMULA_HXX
#include "xiformula.hxx"
#endif
#ifndef SC_XILINK_HXX
#include "xilink.hxx"
#endif
#ifndef SC_XINAME_HXX
#include "xiname.hxx"
#endif
#ifndef SC_XISTYLE_HXX
#include "xistyle.hxx"
#endif
#ifndef SC_XICONTENT_HXX
#include "xicontent.hxx"
#endif
#ifndef SC_XIESCHER_HXX
#include "xiescher.hxx"
#endif
#ifndef SC_XIPIVOT_HXX
#include "xipivot.hxx"
#endif
#ifndef SC_XIPAGE_HXX
#include "xipage.hxx"
#endif
#ifndef SC_XIVIEW_HXX
#include "xiview.hxx"
#endif

#include "root.hxx"
#include "excimp8.hxx"

// Global data ================================================================

XclImpRootData::XclImpRootData( XclBiff eBiff, SfxMedium& rMedium,
        SotStorageRef xRootStrg, ScDocument& rDoc, CharSet eCharSet ) :
    XclRootData( eBiff, rMedium, xRootStrg, rDoc, eCharSet, false )
{
}

XclImpRootData::~XclImpRootData()
{
}

// ----------------------------------------------------------------------------

XclImpRoot::XclImpRoot( XclImpRootData& rImpRootData ) :
    XclRoot( rImpRootData ),
    mrImpData( rImpRootData )
{
    mrImpData.mxAddrConv.reset( new XclImpAddressConverter( GetRoot() ) );
    mrImpData.mxFmlaComp.reset( new XclImpFormulaCompiler( GetRoot() ) );
    mrImpData.mxPalette.reset( new XclImpPalette( GetRoot() ) );
    mrImpData.mxFontBfr.reset( new XclImpFontBuffer( GetRoot() ) );
    mrImpData.mxNumFmtBfr.reset( new XclImpNumFmtBuffer( GetRoot() ) );
    mrImpData.mpXFBfr.reset( new XclImpXFBuffer( GetRoot() ) );
    mrImpData.mxXFRangeBfr.reset( new XclImpXFRangeBuffer( GetRoot() ) );
    mrImpData.mxTabInfo.reset( new XclImpTabInfo );
    mrImpData.mxNameMgr.reset( new XclImpNameManager( GetRoot() ) );
    mrImpData.mxObjMgr.reset( new XclImpObjectManager( GetRoot() ) );

    if( GetBiff() == EXC_BIFF8 )
    {
        mrImpData.mxLinkMgr.reset( new XclImpLinkManager( GetRoot() ) );
        mrImpData.mxSst.reset( new XclImpSst( GetRoot() ) );
        mrImpData.mxCondFmtMgr.reset( new XclImpCondFormatManager( GetRoot() ) );
        // TODO still in old RootData (deleted by RootData)
        GetOldRoot().pAutoFilterBuffer = new XclImpAutoFilterBuffer;
        mrImpData.mxWebQueryBfr.reset( new XclImpWebQueryBuffer( GetRoot() ) );
        mrImpData.mxPTableMgr.reset( new XclImpPivotTableManager( GetRoot() ) );
    }

    mrImpData.mxPageSett.reset( new XclImpPageSettings( GetRoot() ) );
    mrImpData.mxDocViewSett.reset( new XclImpDocViewSettings( GetRoot() ) );
    mrImpData.mxTabViewSett.reset( new XclImpTabViewSettings( GetRoot() ) );
}

void XclImpRoot::InitializeTable( SCTAB nScTab )
{
    GetXFRangeBuffer().Initialize();
    GetPageSettings().Initialize();
    GetTabViewSettings().Initialize();
}

void XclImpRoot::FinalizeTable()
{
    GetXFRangeBuffer().Finalize();
    GetPageSettings().Finalize();
    GetTabViewSettings().Finalize();
}

XclImpAddressConverter& XclImpRoot::GetAddressConverter() const
{
    return *mrImpData.mxAddrConv;
}

XclImpFormulaCompiler& XclImpRoot::GetFormulaCompiler() const
{
    return *mrImpData.mxFmlaComp;
}

ExcelToSc& XclImpRoot::GetOldFmlaConverter() const
{
    // TODO still in old RootData
    return *GetOldRoot().pFmlaConverter;
}

XclImpSst& XclImpRoot::GetSst() const
{
    DBG_ASSERT( mrImpData.mxSst.is(), "XclImpRoot::GetSst - invalid call, wrong BIFF" );
    return *mrImpData.mxSst;
}

XclImpPalette& XclImpRoot::GetPalette() const
{
    return *mrImpData.mxPalette;
}

XclImpFontBuffer& XclImpRoot::GetFontBuffer() const
{
    return *mrImpData.mxFontBfr;
}

XclImpNumFmtBuffer& XclImpRoot::GetNumFmtBuffer() const
{
    return *mrImpData.mxNumFmtBfr;
}

XclImpXFBuffer& XclImpRoot::GetXFBuffer() const
{
    return *mrImpData.mpXFBfr;
}

XclImpXFRangeBuffer& XclImpRoot::GetXFRangeBuffer() const
{
    return *mrImpData.mxXFRangeBfr;
}

_ScRangeListTabs& XclImpRoot::GetPrintAreaBuffer() const
{
    // TODO still in old RootData
    return *GetOldRoot().pPrintRanges;
}

_ScRangeListTabs& XclImpRoot::GetTitleAreaBuffer() const
{
    // TODO still in old RootData
    return *GetOldRoot().pPrintTitles;
}

XclImpTabInfo& XclImpRoot::GetTabInfo() const
{
    return *mrImpData.mxTabInfo;
}

XclImpNameManager& XclImpRoot::GetNameManager() const
{
    return *mrImpData.mxNameMgr;
}

XclImpLinkManager& XclImpRoot::GetLinkManager() const
{
    DBG_ASSERT( mrImpData.mxLinkMgr.is(), "XclImpRoot::GetLinkManager - invalid call, wrong BIFF" );
    return *mrImpData.mxLinkMgr;
}

XclImpObjectManager& XclImpRoot::GetObjectManager() const
{
    return *mrImpData.mxObjMgr;
}

XclImpCondFormatManager& XclImpRoot::GetCondFormatManager() const
{
    DBG_ASSERT( mrImpData.mxCondFmtMgr.is(), "XclImpRoot::GetCondFormatManager - invalid call, wrong BIFF" );
    return *mrImpData.mxCondFmtMgr;
}

XclImpAutoFilterBuffer& XclImpRoot::GetFilterManager() const
{
    // TODO still in old RootData
    DBG_ASSERT( GetOldRoot().pAutoFilterBuffer, "XclImpRoot::GetFilterManager - invalid call, wrong BIFF" );
    return *GetOldRoot().pAutoFilterBuffer;
}

XclImpWebQueryBuffer& XclImpRoot::GetWebQueryBuffer() const
{
    DBG_ASSERT( mrImpData.mxWebQueryBfr.is(), "XclImpRoot::GetWebQueryBuffer - invalid call, wrong BIFF" );
    return *mrImpData.mxWebQueryBfr;
}

XclImpPivotTableManager& XclImpRoot::GetPivotTableManager() const
{
    DBG_ASSERT( mrImpData.mxPTableMgr.is(), "XclImpRoot::GetPivotTableManager - invalid call, wrong BIFF" );
    return *mrImpData.mxPTableMgr;
}

XclImpPageSettings& XclImpRoot::GetPageSettings() const
{
    return *mrImpData.mxPageSett;
}

XclImpDocViewSettings& XclImpRoot::GetDocViewSettings() const
{
    return *mrImpData.mxDocViewSett;
}

XclImpTabViewSettings& XclImpRoot::GetTabViewSettings() const
{
    return *mrImpData.mxTabViewSett;
}

String XclImpRoot::GetScAddInName( const String& rXclName ) const
{
    String aScName;
    if( ScGlobal::GetAddInCollection()->GetCalcName( rXclName, aScName ) )
        return aScName;
    return rXclName;
}

// ============================================================================

