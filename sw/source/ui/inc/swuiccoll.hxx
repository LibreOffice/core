/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swuiccoll.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:11:06 $
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
#ifndef _SWUI_CCOLL_HXX
#define _SWUI_CCOLL_HXX

#include "ccoll.hxx"

class SwWrtShell;
class SwFmt;

class SwCondCollPage : public SfxTabPage
{
    FixedLine           aConditionFL;
    CheckBox            aConditionCB;

    FixedText           aContextFT;
    FixedText           aUsedFT;
    SvTabListBox        aTbLinks;

    FixedText           aStyleFT;
    ListBox             aStyleLB;
    ListBox             aFilterLB;

    PushButton          aRemovePB;
    PushButton          aAssignPB;

    String              sNoTmpl;
    ResStringArray      aStrArr;

    SwWrtShell          &rSh;
    const CommandStruct*pCmds;
    SwFmt*              pFmt;

    CollName*           pNms;

    BOOL                bNewTemplate;


            SwCondCollPage(Window *pParent, const SfxItemSet &rSet);
    virtual ~SwCondCollPage();

    virtual int     DeactivatePage(SfxItemSet *pSet);

    DECL_LINK( OnOffHdl, CheckBox* );
    DECL_LINK( AssignRemoveHdl, PushButton*);
    DECL_LINK( SelectHdl, ListBox* );

public:

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    static USHORT* GetRanges();

    virtual BOOL FillItemSet(      SfxItemSet &rSet);
    virtual void Reset      (const SfxItemSet &rSet);

    void SetCollection( SwFmt* pFormat, BOOL bNew );
};



#endif

