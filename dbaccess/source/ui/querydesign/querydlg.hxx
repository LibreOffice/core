/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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



