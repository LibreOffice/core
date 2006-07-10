/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AViews.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:24:39 $
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
#ifndef _CONNECTIVITY_ADO_VIEWS_HXX_
#include "ado/AViews.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_VIEW_HXX_
#include "ado/AView.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_TABLES_HXX_
#include "ado/ATables.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_CATALOG_HXX_
#include "ado/ACatalog.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_BCONNECTION_HXX_
#include "ado/AConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#include "ado/Awrapado.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

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
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii( "Could not create view: invalid object descriptor." ),
            static_cast<XTypeProvider*>(this)
        );

    WpADOCommand aCommand;
    aCommand.Create();
    if ( !aCommand.IsValid() )
        ::dbtools::throwGenericSQLException(
            ::rtl::OUString::createFromAscii( "Could not create view: no command object." ),
            static_cast<XTypeProvider*>(this)
        );

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



