/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_InsertLegend.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:27:22 $
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
#ifndef _CHART2_DLG_INSERT_LEGEND_GRID_HXX
#define _CHART2_DLG_INSERT_LEGEND_GRID_HXX

// header for class ModalDialog
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
// header for class FixedLine
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for class CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for class SfxItemSet
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................


/*************************************************************************
|*
|* Legenden-Dialog
|*
\************************************************************************/
class SchLegendDlg : public ModalDialog
{
private:
    CheckBox        aCbxShow;
    RadioButton     aRbtLeft;
    RadioButton     aRbtTop;
    RadioButton     aRbtRight;
    RadioButton     aRbtBottom;
    FixedLine       aFlLegend;
    OKButton        aBtnOK;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    const SfxItemSet&   m_rInAttrs;

    void Reset();
    DECL_LINK (CbxClick, CheckBox *);

public:
    SchLegendDlg(Window* pParent, const SfxItemSet& rInAttrs);
    virtual ~SchLegendDlg();

    void GetAttr(SfxItemSet& rOutAttrs);
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

