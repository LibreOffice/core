/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dapitype.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 13:21:09 $
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

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "dapitype.hxx"
#include "scresid.hxx"
#include "sc.hrc"
#include "dapitype.hrc"

using namespace com::sun::star;

//-------------------------------------------------------------------------

ScDataPilotSourceTypeDlg::ScDataPilotSourceTypeDlg( Window* pParent, BOOL bEnableExternal ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DAPITYPE ) ),
    //
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) ),
    aBtnSelection   ( this, ScResId( BTN_SELECTION ) ),
    aBtnDatabase    ( this, ScResId( BTN_DATABASE ) ),
    aBtnExternal    ( this, ScResId( BTN_EXTERNAL ) ),
    aFlFrame        ( this, ScResId( FL_FRAME ) )
{
    if (!bEnableExternal)
        aBtnExternal.Disable();

    aBtnSelection.Check();

    FreeResource();
}

ScDataPilotSourceTypeDlg::~ScDataPilotSourceTypeDlg()
{
}

BOOL ScDataPilotSourceTypeDlg::IsSelection() const
{
    return aBtnSelection.IsChecked();
}

BOOL ScDataPilotSourceTypeDlg::IsDatabase() const
{
    return aBtnDatabase.IsChecked();
}

BOOL ScDataPilotSourceTypeDlg::IsExternal() const
{
    return aBtnExternal.IsChecked();
}

//-------------------------------------------------------------------------

ScDataPilotServiceDlg::ScDataPilotServiceDlg( Window* pParent,
                                const uno::Sequence<rtl::OUString>& rServices ) :
    ModalDialog     ( pParent, ScResId( RID_SCDLG_DAPISERVICE ) ),
    //
    aBtnOk          ( this, ScResId( BTN_OK ) ),
    aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
    aBtnHelp        ( this, ScResId( BTN_HELP ) ),
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
    aFlFrame        ( this, ScResId( FL_FRAME ) )
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



