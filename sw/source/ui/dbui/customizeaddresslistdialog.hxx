/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: customizeaddresslistdialog.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 17:29:55 $
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
#ifndef _CUSTOMIZEADDRESSLISTDIALOG_HXX
#define _CUSTOMIZEADDRESSLISTDIALOG_HXX
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

struct SwCSVData;
/*-- 08.04.2004 14:04:39---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwCustomizeAddressListDialog : public SfxModalDialog
{
    FixedText               m_aFieldsFT;
    ListBox                 m_aFieldsLB;

    PushButton              m_aAddPB;
    PushButton              m_aDeletePB;
    PushButton              m_aRenamePB;

    ImageButton             m_aUpPB;
    ImageButton             m_aDownPB;

    FixedLine               m_aSeparatorFL;

    OKButton                m_aOK;
    CancelButton            m_aCancel;
    HelpButton              m_aHelp;

    SwCSVData*              m_pNewData;

    DECL_LINK(AddRenameHdl_Impl, PushButton*);
    DECL_LINK(DeleteHdl_Impl, PushButton*);
    DECL_LINK(UpDownHdl_Impl, PushButton*);
    DECL_LINK(ListBoxSelectHdl_Impl, ListBox*);

    void UpdateButtons();
public:
    SwCustomizeAddressListDialog(Window* pParent, const SwCSVData& rOldData);
    ~SwCustomizeAddressListDialog();

    SwCSVData*    GetNewData();
};
/*-- 13.04.2004 13:30:21---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwAddRenameEntryDialog : public SfxModalDialog
{
    FixedText               m_aFieldNameFT;
    Edit                    m_aFieldNameED;

    OKButton                m_aOK;
    CancelButton            m_aCancel;
    HelpButton              m_aHelp;

    const ::std::vector< ::rtl::OUString >& m_rCSVHeader;

    DECL_LINK(ModifyHdl_Impl, Edit*);
public:
    SwAddRenameEntryDialog(Window* pParent, bool bRename, const ::std::vector< ::rtl::OUString >& aCSVHeader);
    ~SwAddRenameEntryDialog();

    void                SetFieldName(const String& rName) {m_aFieldNameED.SetText(rName);}
    String              GetFieldName() const {return m_aFieldNameED.GetText();};

};

#endif

