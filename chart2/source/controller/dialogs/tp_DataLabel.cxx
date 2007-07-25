/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_DataLabel.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-25 08:36:32 $
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
