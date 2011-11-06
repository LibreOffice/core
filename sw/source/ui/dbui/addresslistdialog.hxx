/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _ADDRESSLISTDIALOG_HXX
#define _ADDRESSLISTDIALOG_HXX

#include <sfx2/basedlgs.hxx>
#include <svtools/stdctrl.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <svtools/svtabbx.hxx>
#include <svtools/headbar.hxx>
#include <swdbdata.hxx>
#include "sharedconnection.hxx"


namespace com{namespace sun{namespace star{
    namespace container{
        class XNameAccess;
    }
    namespace sdbc{
        class XDataSource;
        class XConnection;
    }
    namespace sdbcx{
        class XColumnsSupplier;
    }
}}}
class SwMailMergeAddressBlockPage;
/*-- 08.04.2004 14:04:29---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwAddressListDialog : public SfxModalDialog
{
    FixedInfo       m_aDescriptionFI;

    FixedInfo       m_aListFT;
    HeaderBar       m_aListHB;
    SvTabListBox    m_aListLB;

    PushButton      m_aLoadListPB;
    PushButton      m_aCreateListPB;
    PushButton      m_aFilterPB;
    PushButton      m_aEditPB;
    PushButton      m_aTablePB;

    FixedLine       m_aSeparatorFL;

    OKButton        m_aOK;
    CancelButton    m_aCancel;
    HelpButton      m_aHelp;

    String          m_sName;
    String          m_sTable;
    String          m_sConnecting;

    String          m_sCreatedURL;
    SvLBoxEntry*    m_pCreatedDataSource;

    bool            m_bInSelectHdl;

    SwMailMergeAddressBlockPage* m_pAddressPage;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>     m_xDBContext;
//    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>          m_xSource;
//    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>          m_xConnection;
//    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>    m_xColumnsSupplier;
    SwDBData                                                                        m_aDBData;

    void DetectTablesAndQueries(SvLBoxEntry* pSelect, bool bWidthDialog);

    DECL_LINK(FilterHdl_Impl, PushButton*);
    DECL_LINK(LoadHdl_Impl,   PushButton*);
    DECL_LINK(CreateHdl_Impl, PushButton*);
    DECL_LINK(ListBoxSelectHdl_Impl, SvTabListBox*);
    DECL_LINK(EditHdl_Impl, PushButton*);
    DECL_LINK(TableSelectHdl_Impl, PushButton*);
    DECL_LINK(OKHdl_Impl, PushButton*);

    DECL_STATIC_LINK(SwAddressListDialog, StaticListBoxSelectHdl_Impl, SvLBoxEntry*);

public:
    SwAddressListDialog(SwMailMergeAddressBlockPage* pParent);
    ~SwAddressListDialog();

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>
                        GetSource();

    SharedConnection    GetConnection();

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>
                        GetColumnsSupplier();

    const SwDBData&     GetDBData() const       {return m_aDBData;}
    ::rtl::OUString     GetFilter();
};
#endif

