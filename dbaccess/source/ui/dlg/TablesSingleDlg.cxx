/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "TablesSingleDlg.hxx"
#include "DbAdminImpl.hxx"
#include "tablespage.hxx"
#include <vcl/msgbox.hxx>
#include "dsitems.hxx"
#include <comphelper/processfactory.hxx>

#include "propertysetitem.hxx"

#include "dbu_dlg.hrc"

namespace dbaui
{
using namespace com::sun::star::uno;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

DBG_NAME(OTableSubscriptionDialog)
    // OTableSubscriptionDialog
OTableSubscriptionDialog::OTableSubscriptionDialog(Window* pParent
            ,SfxItemSet* _pItems
            ,const Reference< XComponentContext >& _rxORB
            ,const ::com::sun::star::uno::Any& _aDataSourceName)
    :SfxNoLayoutSingleTabDialog(pParent,DLG_TABLE_FILTER,_pItems)
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

OTableSubscriptionDialog::~OTableSubscriptionDialog()
{
    DBG_DTOR(OTableSubscriptionDialog,NULL);
    delete m_pOutSet;
}

short OTableSubscriptionDialog::Execute()
{
    short nRet = RET_CANCEL;
    if ( !m_bStopExecution )
    {
        nRet = SfxNoLayoutSingleTabDialog::Execute();
        if ( nRet == RET_OK )
        {
            m_pOutSet->Put(*GetOutputItemSet());
            m_pImpl->saveChanges(*m_pOutSet);
        }
    }
    return nRet;
}

sal_Bool OTableSubscriptionDialog::getCurrentSettings(Sequence< PropertyValue >& _rDriverParams)
{
    return m_pImpl->getCurrentSettings(_rDriverParams);
}

void OTableSubscriptionDialog::successfullyConnected()
{
    m_pImpl->successfullyConnected();
}

void OTableSubscriptionDialog::clearPassword()
{
    m_pImpl->clearPassword();
}

String OTableSubscriptionDialog::getConnectionURL() const
{
    return m_pImpl->getConnectionURL();
}

Reference< XPropertySet > OTableSubscriptionDialog::getCurrentDataSource()
{
    return m_pImpl->getCurrentDataSource();
}

const SfxItemSet* OTableSubscriptionDialog::getOutputSet() const
{
    return m_pOutSet;
}

SfxItemSet* OTableSubscriptionDialog::getWriteOutputSet()
{
    return m_pOutSet;
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
