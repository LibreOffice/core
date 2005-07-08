/*************************************************************************
 *
 *  $RCSfile: addresslistdialog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-08 10:29:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _ADDRESSLISTDIALOG_HXX
#define _ADDRESSLISTDIALOG_HXX

#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SVTABBX_HXX
#include <svtools/svtabbx.hxx>
#endif
#ifndef _HEADBAR_HXX
#include <svtools/headbar.hxx>
#endif
#ifndef _SWDBDATA_HXX
#include <swdbdata.hxx>
#endif
#ifndef SW_SHARED_UNO_COMPONENT_HXX
#include "sharedunocomponent.hxx"
#endif


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

