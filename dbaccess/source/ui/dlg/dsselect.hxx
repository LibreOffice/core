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

