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



#ifndef DBAUI_USERADMINDLG_HXX
#define DBAUI_USERADMINDLG_HXX

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#ifndef DBAUI_ITEMSETHELPER_HXX
#include "IItemSetHelper.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#include <memory>

FORWARD_DECLARE_INTERFACE(beans,XPropertySet)
FORWARD_DECLARE_INTERFACE(sdbc,XConnection)
FORWARD_DECLARE_INTERFACE(lang,XMultiServiceFactory)

//.........................................................................
namespace dbaui
{
//.........................................................................
    class ODbDataSourceAdministrationHelper;
    //=========================================================================
    //= OUserAdminDlg
    //=========================================================================

    /** implements the user adin dialog
    */
    class OUserAdminDlg : public SfxTabDialog, public IItemSetHelper, public IDatabaseSettingsDialog,public dbaui::OModuleClient
    {
        OModuleClient m_aModuleClient;
        ::std::auto_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;
        SfxItemSet* m_pItemSet;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>          m_xConnection;
        sal_Bool    m_bOwnConnection;
    protected:
        virtual void PageCreated(sal_uInt16 _nId, SfxTabPage& _rPage);
    public:
        OUserAdminDlg( Window* _pParent
                            ,SfxItemSet* _pItems
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
                            ,const ::com::sun::star::uno::Any& _aDataSourceName
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection);

        virtual ~OUserAdminDlg();

        virtual const SfxItemSet* getOutputSet() const;
        virtual SfxItemSet* getWriteOutputSet();

        virtual short   Execute();

        // forwards to ODbDataSourceAdministrationHelper
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() const;
        virtual ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >,sal_Bool> createConnection();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > getDriver();
        virtual ::rtl::OUString getDatasourceType(const SfxItemSet& _rSet) const;
        virtual void clearPassword();
        virtual sal_Bool saveDatasource();
        virtual void setTitle(const ::rtl::OUString& _sTitle);
        virtual void enableConfirmSettings( bool _bEnable );
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // DBAUI_USERADMINDLG_HXX
