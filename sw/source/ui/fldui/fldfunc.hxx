/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fldfunc.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 07:38:20 $
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
#ifndef _SWFLDFUNC_HXX
#define _SWFLDFUNC_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#include "condedit.hxx"
#include "fldpage.hxx"
#ifndef _ACTCTRL_HXX
#include <actctrl.hxx>
#endif
/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

class SwFldFuncPage : public SwFldPage
{
    FixedText       aTypeFT;
    ListBox         aTypeLB;
    FixedText       aSelectionFT;
    ListBox         aSelectionLB;
    FixedText       aFormatFT;
    ListBox         aFormatLB;
    FixedText       aNameFT;
    ConditionEdit   aNameED;
    FixedText       aValueFT;
    Edit            aValueED;
    FixedText       aCond1FT;
    ConditionEdit   aCond1ED;
    FixedText       aCond2FT;
    ConditionEdit   aCond2ED;
    PushButton      aMacroBT;

    //controls of "Input list"
    FixedText       aListItemFT;
    ReturnActionEdit aListItemED;
    PushButton      aListAddPB;
    FixedText       aListItemsFT;
    ListBox         aListItemsLB;
    PushButton      aListRemovePB;
    PushButton      aListUpPB;
    PushButton      aListDownPB;
    FixedText       aListNameFT;
    Edit            aListNameED;

    String          sOldValueFT;
    String          sOldNameFT;

    ULONG           nOldFormat;
    bool            bDropDownLBChanged;

    DECL_LINK( TypeHdl, ListBox* pLB = 0 );
    DECL_LINK( SelectHdl, ListBox* pLB = 0 );
    DECL_LINK( InsertMacroHdl, ListBox* pLB = 0 );
    DECL_LINK( ModifyHdl, Edit *pEd = 0 );
    DECL_LINK( ListModifyHdl, Control*);
    DECL_LINK( ListEnableHdl, void*);

    // Macro ausw„hlen
    DECL_LINK( MacroHdl, Button * );

    void                UpdateSubType();
    String              TurnMacroString(const String &rMacro);

protected:
    virtual USHORT      GetGroup();

public:
                        SwFldFuncPage(Window* pParent, const SfxItemSet& rSet);

                        ~SwFldFuncPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
};


#endif

