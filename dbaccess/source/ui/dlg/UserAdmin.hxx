/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UserAdmin.hxx,v $
 * $Revision: 1.7 $
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
