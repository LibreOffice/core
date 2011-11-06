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
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
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
        OModuleClient                           m_aModuleClient;
        ::std::auto_ptr<OTableListBoxControl>   m_pTableControl;
        OJoinTableView::OTableWindowMap*        m_pTableMap;

        FixedLine   aFL_CascUpd;
        RadioButton aRB_NoCascUpd,
                    aRB_CascUpd,
                    aRB_CascUpdNull,
                    aRB_CascUpdDefault;
        FixedLine   aFL_CascDel;
        RadioButton aRB_NoCascDel,
                    aRB_CascDel,
                    aRB_CascDelNull,
                    aRB_CascDelDefault;

        OKButton    aPB_OK;
        CancelButton aPB_CANCEL;
        HelpButton  aPB_HELP;


        TTableConnectionData::value_type                                        m_pConnData;
        TTableConnectionData::value_type                                        m_pOrigConnData;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;

        sal_Bool                                                                    m_bTriedOneUpdate;

    public:
        ORelationDialog(OJoinTableView* pParent,
                        const TTableConnectionData::value_type& pConnectionData,
                        sal_Bool bAllowTableSelect = sal_False );
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
        virtual TTableConnectionData::value_type getConnectionData() const;

        /** setValid set the valid inside, can be used for OK buttons
            @param  _bValid true when the using control allows an update
        */
        virtual void setValid(sal_Bool _bValid);

        /** notifyConnectionChange is callback which is called when the table selection has changed and a new connection exists
            @param  _pConnectionData    the connection which exists between the new tables
        */
        virtual void notifyConnectionChange();
    protected:
        void Init(const TTableConnectionData::value_type& _pConnectionData);

    private:
        DECL_LINK( OKClickHdl, Button* );
    };
}
#endif // DBAUI_RELATIONDIALOG_HXX


