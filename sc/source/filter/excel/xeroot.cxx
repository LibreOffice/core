/*************************************************************************
 *
 *  $RCSfile: xeroot.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:34:44 $
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

// ============================================================================

#ifndef SC_XEROOT_HXX
#include "xeroot.hxx"
#endif

#ifndef SC_ADDINCOL_HXX
#include "addincol.hxx"
#endif

#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
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


// Global data ================================================================

XclExpRootData::XclExpRootData( XclBiff eBiff, ScDocument& rDocument, const String& rDocUrl, CharSet eCharSet, bool bRelUrl ) :
    XclRootData( eBiff, rDocument, rDocUrl, eCharSet ),
    mbRelUrl( bRelUrl )
{
}

XclExpRootData::~XclExpRootData()
{
}


// ----------------------------------------------------------------------------

XclExpRoot::XclExpRoot( XclExpRootData& rExpRootData ) :
    XclRoot( rExpRootData ),
    mrExpData( rExpRootData )
{
    mrExpData.mpTracer.reset( new XclTracer( GetDocUrl(), CREATE_OUSTRING( "Office.Tracing/Export/Excel" ) ) );
    mrExpData.mpPalette.reset( new XclExpPalette( GetRoot() ) );
    mrExpData.mpFontBuffer.reset( new XclExpFontBuffer( GetRoot() ) );
    mrExpData.mpNumFmtBuffer.reset( new XclExpNumFmtBuffer( GetRoot() ) );
    mrExpData.mpXFBuffer.reset( new XclExpXFBuffer( GetRoot() ) );
    mrExpData.mpTabIdBuffer.reset( new XclExpTabIdBuffer( GetDoc() ) );
    mrExpData.mpLinkManager.reset( new XclExpLinkManager( GetRoot() ) );

    mrExpData.mpXFBuffer->InitDefaults();
}

XclExpSst& XclExpRoot::GetSst() const
{
    if( !mrExpData.mpSst.get() )
        mrExpData.mpSst.reset( new XclExpSst );
    return *mrExpData.mpSst;
}

XclExpPalette& XclExpRoot::GetPalette() const
{
    return *mrExpData.mpPalette;
}

XclExpFontBuffer& XclExpRoot::GetFontBuffer() const
{
    return *mrExpData.mpFontBuffer;
}

XclExpNumFmtBuffer& XclExpRoot::GetNumFmtBuffer() const
{
    return *mrExpData.mpNumFmtBuffer;
}

XclExpXFBuffer& XclExpRoot::GetXFBuffer() const
{
    return *mrExpData.mpXFBuffer;
}

XclExpTabIdBuffer& XclExpRoot::GetTabIdBuffer() const
{
    return *mrExpData.mpTabIdBuffer;
}

XclExpLinkManager& XclExpRoot::GetLinkManager() const
{
    return *mrExpData.mpLinkManager;
}

String XclExpRoot::GetXclAddInName( const String& rScName ) const
{
    String aXclName;
    if( ScGlobal::GetAddInCollection()->GetExcelName( rScName, GetUILanguage(), aXclName ) )
        return aXclName;
    return rScName;
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

