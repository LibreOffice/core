/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_InsertDataLabel.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 16:44:32 $
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

#include "dlg_InsertDataLabel.hxx"
#include "dlg_InsertDataLabel.hrc"
#include "ResId.hxx"
#include "ObjectNameProvider.hxx"
#include "res_DataLabel.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

DataLabelsDialog::DataLabelsDialog(Window* pWindow, const SfxItemSet& rInAttrs, SvNumberFormatter* pFormatter) :
    ModalDialog(pWindow, SchResId(DLG_DATA_DESCR)),
    m_aBtnOK(this, SchResId(BTN_OK)),
    m_aBtnCancel(this, SchResId(BTN_CANCEL)),
    m_aBtnHelp(this, SchResId(BTN_HELP)),
    m_apDataLabelResources( new DataLabelResources(this,rInAttrs) ),
    m_rInAttrs(rInAttrs)
{
    FreeResource();
    SetText( ObjectNameProvider::getName(OBJECTTYPE_DATA_LABELS) );
    m_apDataLabelResources->SetNumberFormatter( pFormatter );
    Reset();
}

DataLabelsDialog::~DataLabelsDialog()
{
}

void DataLabelsDialog::Reset()
{
    m_apDataLabelResources->Reset(m_rInAttrs);
}

void DataLabelsDialog::FillItemSet(SfxItemSet& rOutAttrs)
{
    m_apDataLabelResources->FillItemSet(rOutAttrs);
}

//.............................................................................
} //namespace chart
//.............................................................................

