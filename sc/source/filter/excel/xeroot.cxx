/*************************************************************************
 *
 *  $RCSfile: xeroot.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-09 15:03:36 $
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
#ifndef SC_XESTYLE_HXX
#include "xestyle.hxx"
#endif
#ifndef SC_XECONTENT_HXX
#include "xecontent.hxx"
#endif
#ifndef SC_XEPIVOT_HXX
#include "xepivot.hxx"
#endif

// Global data ================================================================

XclExpRootData::XclExpRootData( XclBiff eBiff, SfxMedium& rMedium,
        SotStorageRef xRootStrg, SvStream& rBookStrm, ScDocument& rDoc, CharSet eCharSet ) :
    XclRootData( eBiff, rMedium, xRootStrg, rBookStrm, rDoc, eCharSet, true )
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
    mrExpData.mxProgress.reset( new XclExpProgressBar( GetRoot() ) );
    mrExpData.mxPalette.reset( new XclExpPalette( GetRoot() ) );
    mrExpData.mxFontBfr.reset( new XclExpFontBuffer( GetRoot() ) );
    mrExpData.mxNumFmtBfr.reset( new XclExpNumFmtBuffer( GetRoot() ) );
    mrExpData.mxXFBfr.reset( new XclExpXFBuffer( GetRoot() ) );
    mrExpData.mxTabInfo.reset( new XclExpTabInfo( GetRoot() ) );
    mrExpData.mxLinkMgr.reset( new XclExpLinkManager( GetRoot() ) );
    mrExpData.mxFmlaComp.reset( new XclExpFormulaCompiler( GetRoot() ) );

    // initialization of objects needing complete root data
    mrExpData.mxProgress->Initialize();
    mrExpData.mxXFBfr->InitDefaults();
}

XclExpProgressBar& XclExpRoot::GetProgressBar() const
{
    return *mrExpData.mxProgress;
}

XclExpSst& XclExpRoot::GetSst() const
{
    if( !mrExpData.mxSst.get() )
        mrExpData.mxSst.reset( new XclExpSst );
    return *mrExpData.mxSst;
}

XclExpPalette& XclExpRoot::GetPalette() const
{
    return *mrExpData.mxPalette;
}

XclExpFontBuffer& XclExpRoot::GetFontBuffer() const
{
    return *mrExpData.mxFontBfr;
}

XclExpNumFmtBuffer& XclExpRoot::GetNumFmtBuffer() const
{
    return *mrExpData.mxNumFmtBfr;
}

XclExpXFBuffer& XclExpRoot::GetXFBuffer() const
{
    return *mrExpData.mxXFBfr;
}

XclExpTabInfo& XclExpRoot::GetTabInfo() const
{
    return *mrExpData.mxTabInfo;
}

XclExpLinkManager& XclExpRoot::GetLinkManager() const
{
    return *mrExpData.mxLinkMgr;
}

XclExpFormulaCompiler& XclExpRoot::GetFormulaCompiler() const
{
    return *mrExpData.mxFmlaComp;
}

XclExpPivotTableManager& XclExpRoot::GetPivotTableManager() const
{
    if( !mrExpData.mxPTableMgr.get() )
        mrExpData.mxPTableMgr.reset( new XclExpPivotTableManager( GetRoot() ) );
    return *mrExpData.mxPTableMgr;
}

bool XclExpRoot::CheckCellAddress( const ScAddress& rPos ) const
{
    return XclRoot::CheckCellAddress( rPos, GetXclMaxPos() );
}

bool XclExpRoot::CheckCellRange( ScRange& rRange ) const
{
    return XclRoot::CheckCellRange( rRange, GetXclMaxPos() );
}

void XclExpRoot::CheckCellRangeList( ScRangeList& rRanges ) const
{
    XclRoot::CheckCellRangeList( rRanges, GetXclMaxPos() );
}

// ============================================================================

