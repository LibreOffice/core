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



#ifndef _DBAUI_TABLESSINGLEDLG_HXX_
#define _DBAUI_TABLESSINGLEDLG_HXX_

#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#ifndef DBAUI_ITEMSETHELPER_HXX
#include "IItemSetHelper.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
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
    //========================================================================
    //= OTableSubscriptionDialog
    //========================================================================
    class OTableSubscriptionDialog : public SfxSingleTabDialog, public IItemSetHelper
    {
        ::std::auto_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;
        sal_Bool m_bStopExecution; // set when the dialog should not be executed

        DECL_LINK( OKClickHdl, OKButton* );
        SfxItemSet*             m_pOutSet;
    public:

        OTableSubscriptionDialog(Window* pParent
            ,SfxItemSet* _pItems
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
            ,const ::com::sun::star::uno::Any& _aDataSourceName
        );
        virtual ~OTableSubscriptionDialog();

        // forwards from ODbDataSourceAdministrationHelper
        void        successfullyConnected();
        sal_Bool    getCurrentSettings(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rDriverParams);
        void        clearPassword();
        String      getConnectionURL() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   getCurrentDataSource();
        inline void endExecution() { m_bStopExecution = sal_True; }

        virtual const SfxItemSet* getOutputSet() const;
        virtual SfxItemSet* getWriteOutputSet();

        virtual short   Execute();
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_TABLESSINGLEDLG_HXX_

