/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef DBAUI_RELATIONDIALOG_HXX
#define DBAUI_RELATIONDIALOG_HXX

#include <vcl/dialog.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/msgbox.hxx>
#include "JoinTableView.hxx"
#include "RelControliFace.hxx"
#include "moduledbu.hxx"


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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
