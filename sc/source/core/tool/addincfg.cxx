/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: addincfg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-16 14:29:39 $
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

#include <tools/debug.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "global.hxx"
#include "addincol.hxx"
#include "addincfg.hxx"
#include "scmod.hxx"
#include "sc.hrc"

using namespace com::sun::star;

//==================================================================

#define CFGPATH_ADDINS  "Office.CalcAddIns/AddInInfo"

ScAddInCfg::ScAddInCfg() :
    ConfigItem( rtl::OUString::createFromAscii( CFGPATH_ADDINS ) )
{
    uno::Sequence<rtl::OUString> aNames(1);     // one entry: empty string
    EnableNotification( aNames );
}

void ScAddInCfg::Commit()
{
    DBG_ERROR("ScAddInCfg shouldn't be modified");
}

void ScAddInCfg::Notify( const uno::Sequence<rtl::OUString>& )
{
    // forget all add-in information, re-initialize when needed next time
    ScGlobal::GetAddInCollection()->Clear();

    // function list must also be rebuilt, but can't be modified while function
    // autopilot is open (function list for autopilot is then still old)
    if ( SC_MOD()->GetCurRefDlgId() != SID_OPENDLG_FUNCTION )
        ScGlobal::ResetFunctionList();
}


