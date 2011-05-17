/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "TablesSingleDlg.hxx"
#include "DbAdminImpl.hxx"
#include "tablespage.hxx"
#include <vcl/msgbox.hxx>
#include "dsitems.hxx"

#include "propertysetitem.hxx"

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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
