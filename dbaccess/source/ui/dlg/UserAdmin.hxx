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


#ifndef DBAUI_USERADMIN_HXX
#define DBAUI_USERADMIN_HXX

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef DBAUI_TABLEGRANTCONTROL_HXX
#include "TableGrantCtrl.hxx"
#endif
#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

FORWARD_DECLARE_INTERFACE(beans,XPropertySet)
FORWARD_DECLARE_INTERFACE(sdbc,XConnection)
FORWARD_DECLARE_INTERFACE(lang,XMultiServiceFactory)

namespace dbaui
{

class OUserAdmin : public OGenericAdministrationPage
{
protected:
    FixedLine           m_FL_USER;
    FixedText           m_FT_USER;
    ListBox             m_LB_USER;
    PushButton          m_PB_NEWUSER;
    PushButton          m_PB_CHANGEPWD;
    PushButton          m_PB_DELETEUSER;
    FixedLine           m_FL_TABLE_GRANTS;
    OTableGrantControl  m_TableCtrl; // show the grant rights of one user

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>          m_xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xUsers;
    ::com::sun::star::uno::Sequence< ::rtl::OUString>                               m_aUserNames;

    String              m_UserName;

    // methods
    DECL_LINK( ListDblClickHdl, ListBox * );
    DECL_LINK( CloseHdl,        PushButton * );
    DECL_LINK( UserHdl,         PushButton * );

    void        FillUserNames();

    OUserAdmin( Window* pParent, const SfxItemSet& _rCoreAttrs);
public:
    static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );

    ~OUserAdmin();
    String GetUser();

    // must be overloaded by subclasses, but it isn't pure virtual
    virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

    // <method>OGenericAdministrationPage::fillControls</method>
    virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);

    // <method>OGenericAdministrationPage::fillWindows</method>
    virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
};
}
#endif // DBAUI_USERADMIN_HXX
