/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chrdlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 17:39:06 $
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
#ifndef _SWCHARDLG_HXX
#define _SWCHARDLG_HXX
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

class SwView;
class SvxMacroItem;

/*--------------------------------------------------------------------
   Beschreibung:    Der Tabdialog Traeger der TabPages
 --------------------------------------------------------------------*/

class SwCharDlg: public SfxTabDialog
{
    SwView&   rView;
    BOOL      bIsDrwTxtMode;

public:
    SwCharDlg(Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet,
              const String* pFmtStr = 0, BOOL bIsDrwTxtDlg = FALSE);

    ~SwCharDlg();

    virtual void PageCreated( USHORT nId, SfxTabPage &rPage );
};

/*-----------------14.08.96 11.03-------------------
    Beschreibung: Tabpage fuer URL-Attribut
--------------------------------------------------*/

class SwCharURLPage : public SfxTabPage
{
    FixedLine           aURLFL;

    FixedText           aURLFT;
    Edit                aURLED;
    FixedText           aTextFT;
    Edit                aTextED;
    FixedText           aNameFT;
    Edit                aNameED;
    FixedText           aTargetFrmFT;
    ComboBox            aTargetFrmLB;
    PushButton          aURLPB;
    PushButton          aEventPB;
    FixedLine           aStyleFL;
    FixedText           aVisitedFT;
    ListBox             aVisitedLB;
    FixedText           aNotVisitedFT;
    ListBox             aNotVisitedLB;

    SvxMacroItem*       pINetItem;
    BOOL                bModified;

    DECL_LINK( InsertFileHdl, PushButton * );
    DECL_LINK( EventHdl, PushButton * );

public:
                        SwCharURLPage( Window* pParent,
                                           const SfxItemSet& rSet );

                        ~SwCharURLPage();
    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif

