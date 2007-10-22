/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_InsertDataLabel.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 16:50:23 $
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
#ifndef CHART2_DLG_INSERT_DATALABELS_GRID_HXX
#define CHART2_DLG_INSERT_DATALABELS_GRID_HXX

// header for class ModalDialog
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
// header for class CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class SfxItemSet
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
//for auto_ptr
#include <memory>

class SvNumberFormatter;

//.............................................................................
namespace chart
{
//.............................................................................

class DataLabelResources;
class DataLabelsDialog : public ModalDialog
{
private:
    OKButton            m_aBtnOK;
    CancelButton        m_aBtnCancel;
    HelpButton          m_aBtnHelp;
    ::std::auto_ptr< DataLabelResources >    m_apDataLabelResources;

    const SfxItemSet&   m_rInAttrs;

    void Reset();

public:
    DataLabelsDialog(Window* pParent, const SfxItemSet& rInAttrs, SvNumberFormatter* pFormatter);
    virtual ~DataLabelsDialog();

    void FillItemSet(SfxItemSet& rOutAttrs);
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
