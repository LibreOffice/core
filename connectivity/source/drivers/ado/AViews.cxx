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
#include "precompiled_connectivity.hxx"
#include "ado/AViews.hxx"
#include "ado/AView.hxx"
#include "ado/ATables.hxx"
#include "ado/ACatalog.hxx"
#include "ado/AConnection.hxx"
#include "ado/Awrapado.hxx"
#include "TConnection.hxx"
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include "resource/ado_res.hrc"

using namespace ::comphelper;

using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;

sdbcx::ObjectType OViews::createObject(const ::rtl::OUString& _rName)
{
    OAdoView* pView = new OAdoView(isCaseSensitive(),m_aCollection.GetItem(_rName));
    pView->setNew(sal_False);
    return pView;
}
// -------------------------------------------------------------------------
void OViews::impl_refresh(  ) throw(RuntimeException)
{
    m_aCollection.Refresh();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OViews::createDescriptor()
{
    return new OAdoView(isCaseSensitive());
}

// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OViews::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    OAdoView* pView = NULL;
    if ( !getImplementation( pView, descriptor ) || pView == NULL )
        m_pCatalog->getConnection()->throwGenericSQLException( STR_INVALID_VIEW_DESCRIPTOR_ERROR,static_cast<XTypeProvider*>(this) );

    WpADOCommand aCommand;
    aCommand.Create();
    if ( !aCommand.IsValid() )
        m_pCatalog->getConnection()->throwGenericSQLException( STR_VIEW_NO_COMMAND_ERROR,static_cast<XTypeProvider*>(this) );

    ::rtl::OUString sName( _rForName );
    aCommand.put_Name(sName);
    aCommand.put_CommandText(getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_COMMAND))));
    ADOViews* pViews = (ADOViews*)m_aCollection;
    if(FAILED(pViews->Append(OLEString(sName),aCommand)))
        ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),static_cast<XTypeProvider*>(this));

    OTables* pTables = static_cast<OTables*>(static_cast<OCatalog&>(m_rParent).getPrivateTables());
    if ( pTables )
        pTables->appendNew(sName);

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OViews::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    if(!m_aCollection.Delete(_sElementName))
        ADOS::ThrowException(*m_pCatalog->getConnection()->getConnection(),static_cast<XTypeProvider*>(this));
}
// -------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
