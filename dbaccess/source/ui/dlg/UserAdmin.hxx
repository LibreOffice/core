/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UserAdmin.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-10 10:23:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xORB;
    ::com::sun::star::uno::Sequence< ::rtl::OUString>                               m_aUserNames;

    String              m_UserName;

    // methods
    DECL_LINK( ListDblClickHdl, ListBox * );
    DECL_LINK( CloseHdl,        PushButton * );
    DECL_LINK( UserHdl,         PushButton * );

    void        FillUserNames();

    OUserAdmin( Window* pParent, const SfxItemSet& _rCoreAttrs);
public:
    void setServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _xORB) { m_xORB = _xORB; }

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
