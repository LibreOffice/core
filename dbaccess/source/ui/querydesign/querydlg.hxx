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


#ifndef DBAUI_QUERYDLG_HXX
#define DBAUI_QUERYDLG_HXX
#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif

#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif

#ifndef DBAUI_RELCONTROLIFACE_HXX
#include "RelControliFace.hxx"
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif


namespace dbaui
{
    class OQueryTableConnectionData;
    class OTableListBoxControl;
    class OQueryTableView;
    class OJoinControl;
    class DlgQryJoin :  public ModalDialog
                        ,public IRelationControlInterface
    {
    protected:
        FixedText               aML_HelpText;
        OKButton                aPB_OK;
        CancelButton            aPB_CANCEL;
        HelpButton              aPB_HELP;

        OJoinControl*                       m_pJoinControl;
        OTableListBoxControl*               m_pTableControl;
        OJoinTableView::OTableWindowMap*    m_pTableMap;
        OQueryTableView*                    m_pTableView;

        EJoinType                           eJoinType;
        TTableConnectionData::value_type    m_pConnData; // enth"alt linke und rechte Tabelle
        TTableConnectionData::value_type    m_pOrigConnData;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xConnection;


        DECL_LINK( OKClickHdl, Button* );
        DECL_LINK( LBChangeHdl, ListBox* );
        DECL_LINK( NaturalToggleHdl, CheckBox* );

        /** setJoinType enables and set the new join type
            @param  _eNewJoinType   the new jointype
        */
        void setJoinType(EJoinType _eNewJoinType);
    public:
        DlgQryJoin( OQueryTableView * pParent,
                    const TTableConnectionData::value_type& pData,
                    OJoinTableView::OTableWindowMap*    _pTableMap,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _xConnection,
                    sal_Bool _bAllowTableSelect);
        virtual ~DlgQryJoin();
        EJoinType GetJoinType() const { return eJoinType; };

        /** getConnectionData returns the current connection data
            @return the current connectiondata
        */
        virtual TTableConnectionData::value_type getConnectionData() const;

        /** setValid set the valid inside, can be used for OK buttons
            @param  _bValid true when the using control allows an update
        */
        virtual void setValid(sal_Bool _bValid);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > getConnection() { return m_xConnection; }

        /** notifyConnectionChange is callback which is called when the table selection has changed and a new connection exists
            @param  _pConnectionData    the connection which exists between the new tables
        */
        virtual void notifyConnectionChange();
    };
}
#endif // DBAUI_QUERYDLG_HXX



