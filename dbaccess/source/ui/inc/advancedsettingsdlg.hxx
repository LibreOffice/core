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



#ifndef DBACCESS_ADVANCEDSETTINGSDLG_HXX
#define DBACCESS_ADVANCEDSETTINGSDLG_HXX

#include "IItemSetHelper.hxx"
#include "moduledbu.hxx"

 /** === begin UNO includes === **/
/** === end UNO includes === **/

#include <sfx2/tabdlg.hxx>

#include <memory>

//........................................................................
namespace dbaui
{
//........................................................................

    //=========================================================================
    //= AdvancedSettingsDialog
    //=========================================================================
    class ODbDataSourceAdministrationHelper;
    /** implements the advanced page dlg of the data source properties.
    */
    class AdvancedSettingsDialog    :public SfxTabDialog
                                    ,public IItemSetHelper
                                    ,public IDatabaseSettingsDialog
    {
        OModuleClient                                       m_aModuleClient;
        ::std::auto_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;
        SfxItemSet*                                         m_pItemSet;

    protected:
        virtual void PageCreated(sal_uInt16 _nId, SfxTabPage& _rPage);

    public:
        AdvancedSettingsDialog( Window* _pParent
                            ,SfxItemSet* _pItems
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
                            ,const ::com::sun::star::uno::Any& _aDataSourceName);

        virtual ~AdvancedSettingsDialog();

        /// determines whether or not the given data source type has any advanced setting
        static  bool    doesHaveAnyAdvancedSettings( const ::rtl::OUString& _sURL );

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

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_ADVANCEDSETTINGSDLG_HXX
