/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BCatalog.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:07:51 $
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

#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#include "adabas/BCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_BCONNECTION_HXX_
#include "adabas/BConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_GROUPS_HXX_
#include "adabas/BGroups.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_USERS_HXX_
#include "adabas/BUsers.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_TABLES_HXX_
#include "adabas/BTables.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_VIEWS_HXX_
#include "adabas/BViews.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif


// -------------------------------------------------------------------------
using namespace connectivity;
using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
// -----------------------------------------------------------------------------
OAdabasCatalog::OAdabasCatalog(SQLHANDLE _aConnectionHdl, OAdabasConnection* _pCon) : connectivity::sdbcx::OCatalog(_pCon)
                ,m_pConnection(_pCon)
                ,m_aConnectionHdl(_aConnectionHdl)
{
}
// -----------------------------------------------------------------------------
::rtl::OUString OAdabasCatalog::buildName(const Reference< XRow >& _xRow)
{
    ::rtl::OUString sName;
    sName = _xRow->getString(2);
    if ( sName.getLength() )
        sName += OAdabasCatalog::getDot();
    sName += _xRow->getString(3);


    return sName;
}
// -----------------------------------------------------------------------------
void OAdabasCatalog::fillVector(const ::rtl::OUString& _sQuery,TStringVector& _rVector)
{
    Reference< XStatement > xStmt = m_pConnection->createStatement(  );
    OSL_ENSURE(xStmt.is(),"OAdabasCatalog::fillVector: Could not create a statement!");
    Reference< XResultSet > xResult = xStmt->executeQuery(_sQuery);

    fillNames(xResult,_rVector);
    ::comphelper::disposeComponent(xStmt);

}
// -------------------------------------------------------------------------
void OAdabasCatalog::refreshTables()
{
    TStringVector aVector;
    {
        Sequence< ::rtl::OUString > aTypes(1);
        aTypes[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%"));
        Reference< XResultSet > xResult = m_xMetaData->getTables(Any(),
                                                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")),
                                                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")),
                                                                aTypes);
        fillNames(xResult,aVector);
    }

    if(m_pTables)
        m_pTables->reFill(aVector);
    else
        m_pTables = new OTables(m_xMetaData,*this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void OAdabasCatalog::refreshViews()
{
    TStringVector aVector;
    static const ::rtl::OUString s_sView(RTL_CONSTASCII_USTRINGPARAM("SELECT DISTINCT NULL,DOMAIN.VIEWDEFS.OWNER, DOMAIN.VIEWDEFS.VIEWNAME FROM DOMAIN.VIEWDEFS"));
    fillVector(s_sView,aVector);

    if(m_pViews)
        m_pViews->reFill(aVector);
    else
        m_pViews = new OViews(m_xMetaData,*this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void OAdabasCatalog::refreshGroups()
{
    TStringVector aVector;
    static const ::rtl::OUString s_sGroup(RTL_CONSTASCII_USTRINGPARAM("SELECT DISTINCT NULL,NULL,GROUPNAME FROM DOMAIN.USERS WHERE GROUPNAME IS NOT NULL AND GROUPNAME <> ' '"));
    fillVector(s_sGroup,aVector);
    if(m_pGroups)
        m_pGroups->reFill(aVector);
    else
        m_pGroups = new OGroups(*this,m_aMutex,aVector,m_pConnection,this);
}
// -------------------------------------------------------------------------
void OAdabasCatalog::refreshUsers()
{
    TStringVector aVector;
    static const ::rtl::OUString s_sUsers(RTL_CONSTASCII_USTRINGPARAM("SELECT DISTINCT NULL,NULL,USERNAME FROM DOMAIN.USERS WHERE USERNAME IS NOT NULL AND USERNAME <> ' ' AND USERNAME <> 'CONTROL'"));
    fillVector(s_sUsers,aVector);

    if(m_pUsers)
        m_pUsers->reFill(aVector);
    else
        m_pUsers = new OUsers(*this,m_aMutex,aVector,m_pConnection,this);
}
// -------------------------------------------------------------------------
const ::rtl::OUString& OAdabasCatalog::getDot()
{
    static const ::rtl::OUString sDot(RTL_CONSTASCII_USTRINGPARAM("."));
    return sDot;
}
// -----------------------------------------------------------------------------
void OAdabasCatalog::correctColumnProperties(sal_Int32 /*_nPrec*/, sal_Int32& _rnType,::rtl::OUString& _rsTypeName)
{
    switch(_rnType)
    {
    case DataType::DECIMAL:
        {
            static const ::rtl::OUString sDecimal(RTL_CONSTASCII_USTRINGPARAM("DECIMAL"));
            if(_rnType == DataType::DECIMAL && _rsTypeName == sDecimal)
                _rnType = DataType::NUMERIC;
        }
        break;
    case DataType::FLOAT:
        //  if(_nPrec >= 16)
        {
            static const ::rtl::OUString sDouble(RTL_CONSTASCII_USTRINGPARAM("DOUBLE PRECISION"));
            _rsTypeName = sDouble;
            _rnType = DataType::DOUBLE;
        }
//      else if(_nPrec > 15)
//      {
//          static const ::rtl::OUString sReal = ::rtl::OUString::createFromAscii("REAL");
//          _rsTypeName = sReal;
//          _rnType = DataType::REAL;
//      }
        break;
    }
}
// -----------------------------------------------------------------------------

