/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: createaddresslistdialog.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:30:21 $
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
#ifndef _CREATEADDRESSLISTDIALOG_HXX
#define _CREATEADDRESSLISTDIALOG_HXX

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
#include <vector>

class SwAddressControl_Impl;
class SwMailMergeConfigItem;
/*-- 19.04.2004 12:09:46---------------------------------------------------
    container of the created database
  -----------------------------------------------------------------------*/
struct SwCSVData
{
    ::std::vector< ::rtl::OUString >                    aDBColumnHeaders;
    ::std::vector< ::std::vector< ::rtl::OUString> >     aDBData;
};
/*-- 08.04.2004 14:04:39---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwFindEntryDialog;
class SwCreateAddressListDialog : public SfxModalDialog
{
    FixedInfo               m_aAddressInformation;
    SwAddressControl_Impl*  m_pAddressControl;

    PushButton              m_aNewPB;
    PushButton              m_aDeletePB;
    PushButton              m_aFindPB;
    PushButton              m_aCustomizePB;

    FixedInfo               m_aViewEntriesFI;
    PushButton              m_aStartPB;
    PushButton              m_aPrevPB;
    NumericField            m_aSetNoNF;
    PushButton              m_aNextPB;
    PushButton              m_aEndPB;

    FixedLine               m_aSeparatorFL;

    OKButton                m_aOK;
    CancelButton            m_aCancel;
    HelpButton              m_aHelp;

    String                  m_sAddressListFilterName;
    String                  m_sURL;

    SwCSVData*              m_pCSVData;
    SwFindEntryDialog*      m_pFindDlg;

    DECL_LINK(NewHdl_Impl, PushButton*);
    DECL_LINK(DeleteHdl_Impl, PushButton*);
    DECL_LINK(FindHdl_Impl, PushButton*);
    DECL_LINK(CustomizeHdl_Impl, PushButton*);
    DECL_LINK(OkHdl_Impl, PushButton*);
    DECL_LINK(DBCursorHdl_Impl, PushButton*);
    DECL_LINK(DBNumCursorHdl_Impl, NumericField*);

    void UpdateButtons();

public:
    SwCreateAddressListDialog(
            Window* pParent, const String& rURL, SwMailMergeConfigItem& rConfig);
    ~SwCreateAddressListDialog();

    const String&           GetURL() const {    return m_sURL;    }
    void                    Find( const String& rSearch, sal_Int32 nColumn);
};
/*-- 13.04.2004 13:30:21---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwFindEntryDialog : public ModelessDialog
{
    FixedText               m_aFindFT;
    Edit                    m_aFindED;
    CheckBox                m_aFindOnlyCB;
    ListBox                 m_aFindOnlyLB;

    PushButton              m_aFindPB;
    CancelButton            m_aCancel;
    HelpButton              m_aHelp;

    SwCreateAddressListDialog*  m_pParent;

    DECL_LINK(FindHdl_Impl, PushButton*);
    DECL_LINK(FindEnableHdl_Impl, Edit*);
    DECL_LINK(CloseHdl_Impl, PushButton*);

public:
    SwFindEntryDialog(SwCreateAddressListDialog* pParent);
    ~SwFindEntryDialog();

    ListBox&                GetFieldsListBox(){return m_aFindOnlyLB;}
    String                  GetFindString() const {return m_aFindED.GetText();}
};

#endif

