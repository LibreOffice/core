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



#ifndef _DBAUI_UNOADMIN_
#define _DBAUI_UNOADMIN_

#ifndef _SVT_GENERICUNODIALOG_HXX_
#include <svtools/genericunodialog.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif

class SfxItemSet;
class SfxItemPool;
class SfxPoolItem;

//.........................................................................
namespace dbaui
{
//.........................................................................

class IDatabaseSettingsDialog;

//=========================================================================
//= ODatabaseAdministrationDialog
//=========================================================================
typedef ::svt::OGenericUnoDialog ODatabaseAdministrationDialogBase;
class ODatabaseAdministrationDialog
        :public ODatabaseAdministrationDialogBase
{
    OModuleClient    m_aModuleClient;
protected:
    SfxItemSet*             m_pDatasourceItems;     // item set for the dialog
    SfxItemPool*            m_pItemPool;            // item pool for the item set for the dialog
    SfxPoolItem**           m_pItemPoolDefaults;    // pool defaults
    ::dbaccess::ODsnTypeCollection*
                            m_pCollection;          // datasource type collection

    ::com::sun::star::uno::Any          m_aInitialSelection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > m_xActiveConnection;

protected:
    ODatabaseAdministrationDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
    virtual ~ODatabaseAdministrationDialog();
protected:
// OGenericUnoDialog overridables
    virtual void destroyDialog();
    virtual void implInitialize(const com::sun::star::uno::Any& _rValue);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_UNOADMIN_

