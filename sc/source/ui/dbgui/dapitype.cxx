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

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "dapitype.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "dapitype.hrc"

using namespace com::sun::star;

//-------------------------------------------------------------------------

ScDataPilotSourceTypeDlg::ScDataPilotSourceTypeDlg( Window* pParent, sal_Bool bEnableExternal ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DAPITYPE ) ),
    //
    aFlFrame        ( this, ScResId( FL_FRAME ) ),
    aBtnSelection   ( this, ScResId( BTN_SELECTION ) ),
    aBtnDatabase    ( this, ScResId( BTN_DATABASE ) ),
    aBtnExternal    ( this, ScResId( BTN_EXTERNAL ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    if (!bEnableExternal)
        aBtnExternal.Disable();

    aBtnSelection.Check();

    FreeResource();
}

ScDataPilotSourceTypeDlg::~ScDataPilotSourceTypeDlg()
{
}

sal_Bool ScDataPilotSourceTypeDlg::IsDatabase() const
{
    return aBtnDatabase.IsChecked();
}

sal_Bool ScDataPilotSourceTypeDlg::IsExternal() const
{
    return aBtnExternal.IsChecked();
}

//-------------------------------------------------------------------------

ScDataPilotServiceDlg::ScDataPilotServiceDlg( Window* pParent,
                                const uno::Sequence<rtl::OUString>& rServices ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DAPISERVICE ) ),
    //
    aFlFrame        ( this, ScResId( FL_FRAME ) ),
    aFtService      ( this, ScResId( FT_SERVICE ) ),
    aLbService      ( this, ScResId( LB_SERVICE ) ),
    aFtSource       ( this, ScResId( FT_SOURCE ) ),
    aEdSource       ( this, ScResId( ED_SOURCE ) ),
    aFtName         ( this, ScResId( FT_NAME ) ),
    aEdName         ( this, ScResId( ED_NAME ) ),
    aFtUser         ( this, ScResId( FT_USER ) ),
    aEdUser         ( this, ScResId( ED_USER ) ),
    aFtPasswd       ( this, ScResId( FT_PASSWD ) ),
    aEdPasswd       ( this, ScResId( ED_PASSWD ) ),
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) )
{
    long nCount = rServices.getLength();
    const rtl::OUString* pArray = rServices.getConstArray();
    for (long i=0; i<nCount; i++)
    {
        String aName = pArray[i];
        aLbService.InsertEntry( aName );
    }
    aLbService.SelectEntryPos( 0 );

    FreeResource();
}

ScDataPilotServiceDlg::~ScDataPilotServiceDlg()
{
}

String ScDataPilotServiceDlg::GetServiceName() const
{
    return aLbService.GetSelectEntry();
}

String ScDataPilotServiceDlg::GetParSource() const
{
    return aEdSource.GetText();
}

String ScDataPilotServiceDlg::GetParName() const
{
    return aEdName.GetText();
}

String ScDataPilotServiceDlg::GetParUser() const
{
    return aEdUser.GetText();
}

String ScDataPilotServiceDlg::GetParPass() const
{
    return aEdPasswd.GetText();
}



