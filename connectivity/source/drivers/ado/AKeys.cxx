/*************************************************************************
 *
 *  $RCSfile: AKeys.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: oj $ $Date: 2002-07-05 09:56:08 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_ADO_KEYS_HXX_
#include "ado/AKeys.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_INDEX_HXX_
#include "ado/AKey.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _CONNECTIVITY_ADO_ACONNECTION_HXX_
#include "ado/AConnection.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#include "ado/Awrapado.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::ado;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;

Reference< XNamed > OKeys::createObject(const ::rtl::OUString& _rName)
{
    return new OAdoKey(isCaseSensitive(),m_pConnection,m_aCollection.GetItem(_rName));
}
// -------------------------------------------------------------------------
void OKeys::impl_refresh() throw(RuntimeException)
{
    m_aCollection.Refresh();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OKeys::createEmptyObject()
{
    return new OAdoKey(isCaseSensitive(),m_pConnection);
}
// -------------------------------------------------------------------------
// XAppend
void OKeys::appendObject( const Reference< XPropertySet >& descriptor )
{
    OAdoKey* pKey = NULL;
    if(getImplementation(pKey,descriptor) && pKey != NULL)
    {
        // To pass as column parameter to Key's Apppend method
        OLEVariant vOptional;
        vOptional.setNoArg();

        KeyTypeEnum eKey = OAdoKey::Map2KeyRule(getINT32(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))));
        WpADOKey aKey = pKey->getImpl();
        ::rtl::OUString sName = aKey.get_Name();
        if(!sName.getLength())
            aKey.put_Name(::rtl::OUString::createFromAscii("PrimaryKey") );

        ADOKeys* pKeys = m_aCollection;
        if ( FAILED(pKeys->Append(OLEVariant((ADOKey*)aKey),
                                adKeyPrimary, // must be every time adKeyPrimary
                                vOptional)) )
        {
            ADOS::ThrowException(*m_pConnection->getConnection(),*this);
            // just make sure that an SQLExceptionis thrown here
            throw SQLException(::rtl::OUString::createFromAscii("Could not append key!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
        }
    }
    else
        throw SQLException(::rtl::OUString::createFromAscii("Could not append key!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
}
// -------------------------------------------------------------------------
// XDrop
void OKeys::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    if(!m_aCollection.Delete(OLEVariant(_sElementName)))
        ADOS::ThrowException(*m_pConnection->getConnection(),*this);
}
// -----------------------------------------------------------------------------
Reference< XNamed > OKeys::cloneObject(const Reference< XPropertySet >& _xDescriptor)
{
    OAdoKey* pKey = NULL;
    if(getImplementation(pKey,_xDescriptor) && pKey != NULL)
        return new OAdoKey(isCaseSensitive(),m_pConnection,pKey->getImpl());
    return Reference< XNamed >();
}
// -----------------------------------------------------------------------------


