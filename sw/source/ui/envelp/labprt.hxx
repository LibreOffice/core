/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: labprt.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 15:15:36 $
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
#ifndef _LABPRT_HXX
#define _LABPRT_HXX

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif


class SwLabDlg;
class SwLabItem;

// class SwLabPrtPage -------------------------------------------------------

class SwLabPrtPage : public SfxTabPage
{
    Printer*      pPrinter;             //Fuer die Schachteinstellug - leider.

    RadioButton   aPageButton;
    RadioButton   aSingleButton;
    FixedText     aColText;
    NumericField  aColField;
    FixedText     aRowText;
    NumericField  aRowField;
    CheckBox      aSynchronCB;
    FixedLine     aFLDontKnow;

    FixedInfo     aPrinterInfo;
    PushButton    aPrtSetup;
    FixedLine     aFLPrinter;

     SwLabPrtPage(Window* pParent, const SfxItemSet& rSet);
    ~SwLabPrtPage();

    DECL_LINK( CountHdl, Button * );

    using Window::GetParent;
    SwLabDlg* GetParent() {return (SwLabDlg*) SfxTabPage::GetParent()->GetParent();}

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
            void FillItem(SwLabItem& rItem);
    virtual BOOL FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);
    inline Printer* GetPrt() { return (pPrinter); }
};

#endif


