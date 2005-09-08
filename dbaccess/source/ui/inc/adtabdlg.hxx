/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: adtabdlg.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:43:49 $
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
#ifndef DBAUI_QYDLGTAB_HXX
#define DBAUI_QYDLGTAB_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _DBAUI_TABLETREE_HXX_
#include "tabletree.hxx"
#endif


namespace dbaui
{
    //========================================================================
    class OJoinTableView;
    class OAddTableDlg : public ModelessDialog
    {
        FixedText           aFTTable;
        OTableTreeListBox   aTableList;
        PushButton          aAddButton;
        CancelButton        aCloseButton;
        HelpButton          aHelpButton;
        FixedLine           aFixedLineTable;

        String              aDefaultString;

        OJoinTableView*     m_pTableView;
        sal_Bool            m_bInitialized;

        BOOL IsAddAllowed();
        void AddTable();

        DECL_LINK( AddClickHdl, Button* );
        DECL_LINK( CloseClickHdl, Button* );
        DECL_LINK( TableListDoubleClickHdl, ListBox* );
        DECL_LINK( TableListSelectHdl, ListBox* );
    public:
        OAddTableDlg(Window* pParent,OJoinTableView* _pTableView);
        virtual ~OAddTableDlg();

        virtual BOOL Close();

        void DetermineAddTable() { aAddButton.Enable( IsAddAllowed() ); }
        void Update();
    protected:
        void UpdateTableList(BOOL bViewsAllowed);
    };
}
#endif // DBAUI_QYDLGTAB_HXX


