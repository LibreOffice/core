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

#ifndef _DBAUI_DSSELECT_HXX_
#define _DBAUI_DSSELECT_HXX_

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
//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= ODatasourceSelector
//=========================================================================
class ODatasourceSelectDialog : public ModalDialog
{
protected:
    FixedText       m_aDescription;
    ListBox         m_aDatasource;
    OKButton        m_aOk;
    CancelButton    m_aCancel;
    HelpButton      m_aHelp;
#ifdef HAVE_ODBC_ADMINISTRATION
    PushButton      m_aManageDatasources;
#endif
    PushButton      m_aCreateAdabasDB;
    SfxItemSet*     m_pOutputSet;
#ifdef HAVE_ODBC_ADMINISTRATION
    ::std::auto_ptr< OOdbcManagement >
                    m_pODBCManagement;
#endif

public:
    ODatasourceSelectDialog( Window* _pParent, const StringBag& _rDatasources, bool _bAdabas,SfxItemSet* _pOutputSet = NULL );
    ~ODatasourceSelectDialog();

    inline String   GetSelected() const { return m_aDatasource.GetSelectEntry();}
    void            Select( const String& _rEntry ) { m_aDatasource.SelectEntry(_rEntry); }

    virtual sal_Bool    Close();

protected:
    DECL_LINK( ListDblClickHdl, ListBox * );
#ifdef HAVE_ODBC_ADMINISTRATION
    DECL_LINK( ManageClickHdl, PushButton * );
    DECL_LINK( ManageProcessFinished, void* );
#endif
    DECL_LINK( CreateDBClickHdl, PushButton * );
    void fillListBox(const StringBag& _rDatasources);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DSSELECT_HXX_

