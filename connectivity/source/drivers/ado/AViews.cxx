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

#include <ado/AViews.hxx>
#include <ado/AView.hxx>
#include <ado/ATables.hxx>
#include <ado/ACatalog.hxx>
#include <ado/AConnection.hxx>
#include <ado/Awrapado.hxx>
#include <TConnection.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <rtl/ref.hxx>
#include <strings.hrc>

using namespace ::comphelper;

using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;

sdbcx::ObjectType OViews::createObject(const OUString& _rName)
{
    rtl::Reference<OAdoView> pView = new OAdoView(isCaseSensitive(),m_aCollection.GetItem(_rName));
    pView->setNew(false);
    return pView;
}

void OViews::impl_refresh(  )
{
    m_aCollection.Refresh();
}

Reference< XPropertySet > OViews::createDescriptor()
{
    return new OAdoView(isCaseSensitive());
}


// XAppend
sdbcx::ObjectType OViews::appendObject( const OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    OAdoView* pView = getUnoTunnelImplementation<OAdoView>( descriptor );
    if ( pView == nullptr )
        m_pCatalog->getConnection()->throwGenericSQLException( STR_INVALID_VIEW_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );

    WpADOCommand aCommand;
    aCommand.Create();
    if ( !aCommand.IsValid() )
        m_pCatalog->getConnection()->throwGenericSQLException( STR_VIEW_NO_COMMAND_ERROR,static_cast<XTypeProvider*>(this) );

    OUString sName( _rForName );
    aCommand.put_Name(sName);
    aCommand.put_CommandText(getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_COMMAND))));
    ADOViews* pViews = m_aCollection;
    if(FAILED(pViews->Append(OLEString(sName).asBSTR(),aCommand)))
        ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),static_cast<XTypeProvider*>(this));

    OTables* pTables = static_cast<OTables*>(static_cast<OCatalog&>(m_rParent).getPrivateTables());
    if ( pTables )
        pTables->appendNew(sName);

    return createObject( _rForName );
}

// XDrop
void OViews::dropObject(sal_Int32 /*_nPos*/,const OUString& _sElementName)
{
    if(!m_aCollection.Delete(_sElementName))
        ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),static_cast<XTypeProvider*>(this));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
