/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    ConfigItem( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( CFGPATH_ADDINS )) )
{
    uno::Sequence<rtl::OUString> aNames(1);     // one entry: empty string
    EnableNotification( aNames );
}

void ScAddInCfg::Commit()
{
    OSL_FAIL("ScAddInCfg shouldn't be modified");
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
