/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DBACCESS_SOURCE_UI_DLG_DSSELECT_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_DLG_DSSELECT_HXX

#include "dsntypes.hxx"
#include "odbcconfig.hxx"
#include "commontypes.hxx"

#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <rtl/ustring.hxx>

#include <memory>

class SfxItemSet;
namespace dbaui
{

// ODatasourceSelector
class ODatasourceSelectDialog : public ModalDialog
{
protected:
    VclPtr<ListBox>        m_pDatasource;
    VclPtr<OKButton>       m_pOk;
    VclPtr<CancelButton>   m_pCancel;
#ifdef HAVE_ODBC_ADMINISTRATION
    VclPtr<PushButton>     m_pManageDatasources;
    ::std::unique_ptr< OOdbcManagement >
    m_pODBCManagement;
#endif

public:
    ODatasourceSelectDialog( vcl::Window* _pParent, const StringBag& _rDatasources );
    virtual ~ODatasourceSelectDialog();
    virtual void dispose() override;
    OUString GetSelected() const {
        return m_pDatasource->GetSelectEntry();
    }
    void     Select( const OUString& _rEntry ) {
        m_pDatasource->SelectEntry(_rEntry);
    }

    virtual bool    Close() override;

protected:
    DECL_LINK_TYPED( ListDblClickHdl, ListBox&, void );
#ifdef HAVE_ODBC_ADMINISTRATION
    DECL_LINK_TYPED(ManageClickHdl, Button*, void);
    DECL_LINK_TYPED( ManageProcessFinished, void*, void );
#endif
    void fillListBox(const StringBag& _rDatasources);
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_DLG_DSSELECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
