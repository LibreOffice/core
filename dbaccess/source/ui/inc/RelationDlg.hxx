/*************************************************************************
 *
 *  $RCSfile: RelationDlg.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: oj $ $Date: 2002-02-06 07:23:39 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_RELATIONDIALOG_HXX
#define DBAUI_RELATIONDIALOG_HXX

#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
//#ifndef _EDIT_HXX //autogen
//#include <vcl/edit.hxx>
//#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef DBAUI_RELCONTROLIFACE_HXX
#include "RelControliFace.hxx"
#endif


namespace dbaui
{
    class OJoinTableView;
    class OTableListBoxControl;
    class ORelationTableConnectionData;
    //========================================================================
    class ORelationDialog : public ModalDialog
                            ,public IRelationControlInterface
    {

        OTableListBoxControl*               m_pTableControl;
        OJoinTableView::OTableWindowMap*    m_pTableMap;

        FixedLine    aFL_CascUpd;
        RadioButton aRB_NoCascUpd,
                    aRB_CascUpd,
                    aRB_CascUpdNull,
                    aRB_CascUpdDefault;
        FixedLine    aFL_CascDel;
        RadioButton aRB_NoCascDel,
                    aRB_CascDel,
                    aRB_CascDelNull,
                    aRB_CascDelDefault;

        OKButton    aPB_OK;
        CancelButton aPB_CANCEL;
        HelpButton  aPB_HELP;


        ORelationTableConnectionData*                                           m_pConnData;
        ORelationTableConnectionData*                                           m_pOrigConnData;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;

        BOOL                                                                    m_bTriedOneUpdate;

    public:
        ORelationDialog(OJoinTableView* pParent,
                        ORelationTableConnectionData* pConnectionData,
                        BOOL bAllowTableSelect = FALSE );
        virtual ~ORelationDialog();

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > getConnection(){ return m_xConnection; }

        virtual short Execute();

        /** getTableMap gives acces to the table window map
            @retrun the table window from the join view
        */
        OJoinTableView::OTableWindowMap* getTableMap() const { return m_pTableMap; }

        /** getConnectionData returns the current connection data
            @return the current connectiondata
        */
        virtual OTableConnectionData* getConnectionData() const;

        /** setValid set the valid inside, can be used for OK buttons
            @param  _bValid true when the using control allows an update
        */
        virtual void setValid(sal_Bool _bValid);

        /** notifyConnectionChange is callback which is called when the table selection has changed and a new connection exists
            @param  _pConnectionData    the connection which exists between the new tables
        */
        virtual void notifyConnectionChange(OTableConnectionData* _pConnectionData);
    protected:
        void Init(ORelationTableConnectionData* _pConnData);

    private:
        DECL_LINK( OKClickHdl, Button* );
    };
}
#endif // DBAUI_RELATIONDIALOG_HXX


