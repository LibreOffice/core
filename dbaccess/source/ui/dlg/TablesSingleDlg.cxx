/*************************************************************************
 *
 *  $RCSfile: TablesSingleDlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:41:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/

#ifndef _DBAUI_TABLESSINGLEDLG_HXX_
#include "TablesSingleDlg.hxx"
#endif
#ifndef _DBAUI_DBADMINIMPL_HXX_
#include "DbAdminImpl.hxx"
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
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
            ,const ::com::sun::star::uno::Any& _aDataSourceName) : SfxSingleTabDialog(pParent,UID_DLG_TABLE_FILTER,_pItems)
            ,m_pOutSet(_pItems)
            ,m_bStopExecution(sal_False)
{
    DBG_CTOR(OTableSubscriptionDialog,NULL);
    m_pImpl = ::std::auto_ptr<ODbDataSourceAdministrationHelper>(new ODbDataSourceAdministrationHelper(_rxORB,pParent,this));
    m_pImpl->setCurrentDataSourceName(_aDataSourceName);
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



