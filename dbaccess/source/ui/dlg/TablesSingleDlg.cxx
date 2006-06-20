/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TablesSingleDlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:04:07 $
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
            ,const ::com::sun::star::uno::Any& _aDataSourceName)
    :SfxSingleTabDialog(pParent,UID_DLG_TABLE_FILTER,_pItems)
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



