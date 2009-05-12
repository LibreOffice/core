/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dlg_InsertDataLabel.cxx,v $
 * $Revision: 1.10.72.1 $
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
    m_apDataLabelResources( new DataLabelResources(this,rInAttrs,false) ),
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

