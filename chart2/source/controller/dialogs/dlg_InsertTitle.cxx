/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_InsertTitle.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:43:48 $
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
#include "precompiled_chart2.hxx"
#include "dlg_InsertTitle.hxx"
#include "dlg_InsertTitle.hrc"
#include "res_Titles.hxx"
#include "ResId.hxx"
#include "NoWarningThisInCTOR.hxx"
#include "ObjectNameProvider.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

SchTitleDlg::SchTitleDlg(Window* pWindow, const TitleDialogData& rInput )
    : ModalDialog(pWindow, SchResId(DLG_TITLE))
    , m_apTitleResources( new TitleResources(this,true) )
    , aBtnOK(this, SchResId(BTN_OK))
    , aBtnCancel(this, SchResId(BTN_CANCEL))
    , aBtnHelp(this, SchResId(BTN_HELP))
{
    FreeResource();

    this->SetText( ObjectNameProvider::getName(OBJECTTYPE_TITLE,true) );
    m_apTitleResources->writeToResources( rInput );
}

SchTitleDlg::~SchTitleDlg()
{
}

void SchTitleDlg::getResult( TitleDialogData& rOutput )
{
    m_apTitleResources->readFromResources( rOutput );
}

//.............................................................................
} //namespace chart
//.............................................................................

