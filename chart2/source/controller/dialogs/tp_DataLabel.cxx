/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tp_DataLabel.cxx,v $
 * $Revision: 1.9.72.1 $
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
#include "tp_DataLabel.hxx"

#include "TabPages.hrc"
#include "ResId.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

DataLabelsTabPage::DataLabelsTabPage(Window* pWindow, const SfxItemSet& rInAttrs) :
        SfxTabPage(pWindow, SchResId(TP_DATA_DESCR), rInAttrs),
        m_aDataLabelResources(this, rInAttrs)
{
    FreeResource();
}

DataLabelsTabPage::~DataLabelsTabPage()
{
}

SfxTabPage* DataLabelsTabPage::Create(Window* pWindow,
                                        const SfxItemSet& rOutAttrs)
{
    return new DataLabelsTabPage(pWindow, rOutAttrs);
}

BOOL DataLabelsTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    return m_aDataLabelResources.FillItemSet(rOutAttrs);
}

void DataLabelsTabPage::Reset(const SfxItemSet& rInAttrs)
{
    m_aDataLabelResources.Reset(rInAttrs);
}

void DataLabelsTabPage::SetNumberFormatter( SvNumberFormatter* pFormatter )
{
    m_aDataLabelResources.SetNumberFormatter( pFormatter );
}

//.............................................................................
} //namespace chart
//.............................................................................
