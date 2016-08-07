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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbui.hxx"

#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _DBAUI_UNOADMIN_
#include "unoadmin.hxx"
#endif
#ifndef _DBAUI_DBADMIN_HXX_
#include "dbadmin.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

// --- needed because of the solar mutex
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
// ---

#define THISREF()   static_cast< XServiceInfo* >(this)

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

//=========================================================================
DBG_NAME(ODatabaseAdministrationDialog)
//-------------------------------------------------------------------------
ODatabaseAdministrationDialog::ODatabaseAdministrationDialog(const Reference< XMultiServiceFactory >& _rxORB)
    :ODatabaseAdministrationDialogBase(_rxORB)
    ,m_pDatasourceItems(NULL)
    ,m_pItemPool(NULL)
    ,m_pItemPoolDefaults(NULL)
    ,m_pCollection(NULL)
{
    DBG_CTOR(ODatabaseAdministrationDialog,NULL);

    m_pCollection = new ::dbaccess::ODsnTypeCollection(_rxORB);
    ODbAdminDialog::createItemSet(m_pDatasourceItems, m_pItemPool, m_pItemPoolDefaults, m_pCollection);
}

//-------------------------------------------------------------------------
ODatabaseAdministrationDialog::~ODatabaseAdministrationDialog()
{
    // we do this here cause the base class' call to destroyDialog won't reach us anymore : we're within an dtor,
    // so this virtual-method-call the base class does does not work, we're already dead then ...
    if (m_pDialog)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if (m_pDialog)
            destroyDialog();
    }

    delete m_pCollection;
    m_pCollection = NULL;

    DBG_DTOR(ODatabaseAdministrationDialog,NULL);
}
//-------------------------------------------------------------------------
void ODatabaseAdministrationDialog::destroyDialog()
{
    ODatabaseAdministrationDialogBase::destroyDialog();
    ODbAdminDialog::destroyItemSet(m_pDatasourceItems, m_pItemPool, m_pItemPoolDefaults);
}
//------------------------------------------------------------------------------
void ODatabaseAdministrationDialog::implInitialize(const Any& _rValue)
{
    PropertyValue aProperty;
    if (_rValue >>= aProperty)
    {
        if (0 == aProperty.Name.compareToAscii("InitialSelection"))
        {
            m_aInitialSelection = aProperty.Value;
        }
        else if (0 == aProperty.Name.compareToAscii("ActiveConnection"))
        {
            m_xActiveConnection.set(aProperty.Value,UNO_QUERY);
        }
        else
            ODatabaseAdministrationDialogBase::implInitialize(_rValue);
    }
    else
        ODatabaseAdministrationDialogBase::implInitialize(_rValue);
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

