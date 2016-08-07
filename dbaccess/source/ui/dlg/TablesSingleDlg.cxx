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

#ifndef _DBAUI_TABLESSINGLEDLG_HXX_
#include "TablesSingleDlg.hxx"
#endif
#ifndef _DBAUI_DBADMINIMPL_HXX_
#include "DbAdminImpl.hxx"
#endif
#ifndef _DBAUI_TABLESPAGE_HXX_
#include "tablespage.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBAUI_DATASOURCEITEMS_HXX_
#include "dsitems.hxx"
#endif

#ifndef _DBAUI_PROPERTYSETITEM_HXX_
#include "propertysetitem.hxx"
#endif

#include "dbu_dlg.hrc"

//.........................................................................
namespace dbaui
{
//.........................................................................
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

DBG_NAME(OTableSubscriptionDialog)
    //========================================================================
    //= OTableSubscriptionDialog
    //========================================================================
OTableSubscriptionDialog::OTableSubscriptionDialog(Window* pParent
            ,SfxItemSet* _pItems
            ,const Reference< XMultiServiceFactory >& _rxORB
            ,const ::com::sun::star::uno::Any& _aDataSourceName)
    :SfxSingleTabDialog(pParent,DLG_TABLE_FILTER,_pItems)
    ,m_pImpl( new ODbDataSourceAdministrationHelper( _rxORB, pParent, this ) )
    ,m_bStopExecution(sal_False)
    ,m_pOutSet(_pItems)
{
    DBG_CTOR(OTableSubscriptionDialog,NULL);
    m_pImpl->setDataSourceOrName(_aDataSourceName);
    Reference< XPropertySet > xDatasource = m_pImpl->getCurrentDataSource();
    m_pOutSet = new SfxItemSet( *_pItems );

    m_pImpl->translateProperties(xDatasource, *m_pOutSet);
    SetInputSet(m_pOutSet);

    OTableSubscriptionPage* pTabPage = new OTableSubscriptionPage(this,*m_pOutSet,this);
    pTabPage->SetServiceFactory(_rxORB);
    SetTabPage(pTabPage);
}
// -----------------------------------------------------------------------------
OTableSubscriptionDialog::~OTableSubscriptionDialog()
{
    DBG_DTOR(OTableSubscriptionDialog,NULL);
    delete m_pOutSet;
}
// -----------------------------------------------------------------------------
short OTableSubscriptionDialog::Execute()
{
    short nRet = RET_CANCEL;
    if ( !m_bStopExecution )
    {
        nRet = SfxSingleTabDialog::Execute();
        if ( nRet == RET_OK )
        {
            m_pOutSet->Put(*GetOutputItemSet());
            m_pImpl->saveChanges(*m_pOutSet);
        }
    }
    return nRet;
}
// -----------------------------------------------------------------------------
sal_Bool OTableSubscriptionDialog::getCurrentSettings(Sequence< PropertyValue >& _rDriverParams)
{
    return m_pImpl->getCurrentSettings(_rDriverParams);
}
// -----------------------------------------------------------------------------
void OTableSubscriptionDialog::successfullyConnected()
{
    m_pImpl->successfullyConnected();
}
// -----------------------------------------------------------------------------
void OTableSubscriptionDialog::clearPassword()
{
    m_pImpl->clearPassword();
}
// -----------------------------------------------------------------------------
String OTableSubscriptionDialog::getConnectionURL() const
{
    return m_pImpl->getConnectionURL();
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OTableSubscriptionDialog::getCurrentDataSource()
{
    return m_pImpl->getCurrentDataSource();
}
// -----------------------------------------------------------------------------
const SfxItemSet* OTableSubscriptionDialog::getOutputSet() const
{
    return m_pOutSet;
}
// -----------------------------------------------------------------------------
SfxItemSet* OTableSubscriptionDialog::getWriteOutputSet()
{
    return m_pOutSet;
}
// -----------------------------------------------------------------------------
//.........................................................................
}   // namespace dbaui
//.........................................................................



