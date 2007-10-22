/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: flddb.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 15:16:07 $
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
#ifndef _SWFLDDB_HXX
#define _SWFLDDB_HXX

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

#include "condedit.hxx"
#include "dbtree.hxx"
#include "numfmtlb.hxx"

#include "fldpage.hxx"

/*--------------------------------------------------------------------
   Beschreibung:
 --------------------------------------------------------------------*/

class SwFldDBPage : public SwFldPage
{
    FixedText           aTypeFT;
    ListBox             aTypeLB;
    FixedText           aSelectionFT;
    SwDBTreeList        aDatabaseTLB;

    FixedText           aAddDBFT;
    PushButton          aAddDBPB;

    FixedText           aConditionFT;
    ConditionEdit       aConditionED;
    FixedText           aValueFT;
    Edit                aValueED;
    RadioButton         aDBFormatRB;
    RadioButton         aNewFormatRB;
    NumFormatListBox    aNumFormatLB;
    ListBox             aFormatLB;
    FixedLine           aFormatFL;
    FixedLine           aFormatVertFL;

    String              sOldDBName;
    String              sOldTableName;
    String              sOldColumnName;
    ULONG               nOldFormat;
    USHORT              nOldSubType;
    Link                aOldNumSelectHdl;

    DECL_LINK( TypeHdl, ListBox* );
    DECL_LINK( NumSelectHdl, NumFormatListBox* pLB = 0);
    DECL_LINK( TreeSelectHdl, SvTreeListBox* pBox );
    DECL_LINK( ModifyHdl, Edit *pED = 0 );
    DECL_LINK( AddDBHdl, PushButton* );

    void                CheckInsert();

    using SwFldPage::SetWrtShell;

protected:
    virtual USHORT      GetGroup();

public:
                        SwFldDBPage(Window* pParent, const SfxItemSet& rSet);

                        ~SwFldDBPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
    void                ActivateMailMergeAddress();

    void                SetWrtShell(SwWrtShell& rSh);
};


#endif

