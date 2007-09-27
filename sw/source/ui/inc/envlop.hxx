/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: envlop.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:59:19 $
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
#ifndef _ENVLOP_HXX
#define _ENVLOP_HXX

#ifndef _SV_MEDIT_HXX
#include <svtools/svmedit.hxx>
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#include "envimg.hxx"

#define GetFldVal(rField)         (rField).Denormalize((rField).GetValue(FUNIT_TWIP))
#define SetFldVal(rField, lValue) (rField).SetValue((rField).Normalize(lValue), FUNIT_TWIP)

class SwEnvPage;
class SwEnvFmtPage;
class SwWrtShell;
class Printer;

// class SwEnvPreview ---------------------------------------------------------

class SwEnvPreview : public Window
{
    void Paint(const Rectangle&);

public:

     SwEnvPreview(SfxTabPage* pParent, const ResId& rResID);
    ~SwEnvPreview();

protected:
    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};

// class SwEnvDlg -----------------------------------------------------------

class SwEnvDlg : public SfxTabDialog
{
friend class SwEnvPage;
friend class SwEnvFmtPage;
friend class SwEnvPrtPage;
friend class SwEnvPreview;

    String          sInsert;
    String          sChange;
    SwEnvItem       aEnvItem;
    SwWrtShell      *pSh;
    Printer         *pPrinter;
    SfxItemSet      *pAddresseeSet;
    SfxItemSet      *pSenderSet;

    virtual void    PageCreated( USHORT nId, SfxTabPage &rPage );
    virtual short   Ok();

public:
     SwEnvDlg(Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, BOOL bInsert);
    ~SwEnvDlg();
};

// class SwEnvPage ----------------------------------------------------------

class SwEnvPage : public SfxTabPage
{
    FixedText     aAddrText;
    MultiLineEdit aAddrEdit;
    FixedText     aDatabaseFT;
    ListBox       aDatabaseLB;
    FixedText     aTableFT;
    ListBox       aTableLB;
    ImageButton   aInsertBT;
    FixedText     aDBFieldFT;
    ListBox       aDBFieldLB;
    CheckBox      aSenderBox;
    MultiLineEdit aSenderEdit;
    SwEnvPreview  aPreview;

    SwWrtShell*   pSh;
    String        sActDBName;

     SwEnvPage(Window* pParent, const SfxItemSet& rSet);
    ~SwEnvPage();

    DECL_LINK( DatabaseHdl, ListBox * );
    DECL_LINK( FieldHdl, Button * );
    DECL_LINK( SenderHdl, Button * );

    void InitDatabaseBox();

    using Window::GetParent;
    SwEnvDlg* GetParent() {return (SwEnvDlg*) SfxTabPage::GetParent()->GetParent();}

public:

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
            void FillItem(SwEnvItem& rItem);
    virtual BOOL FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);
};

#endif


