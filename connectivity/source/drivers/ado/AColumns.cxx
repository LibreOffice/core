/*************************************************************************
 *
 *  $RCSfile: AColumns.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-24 15:18:23 $
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

#ifndef _CONNECTIVITY_ADO_COLUMNS_HXX_
#include "ado/AColumns.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_COLUMN_HXX_
#include "ado/AColumn.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_ACONNECTION_HXX_
#include "ado/AConnection.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADO_HXX_
#include "ado/Awrapado.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace connectivity::ado;
using namespace connectivity;
using namespace comphelper;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;

Reference< XNamed > OColumns::createObject(const ::rtl::OUString& _rName)
{
    return new OAdoColumn(isCaseSensitive(),m_pConnection,m_aCollection.GetItem(_rName));
}

// -------------------------------------------------------------------------
void OColumns::impl_refresh() throw(RuntimeException)
{
    m_aCollection.Refresh();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OColumns::createEmptyObject()
{
    return new OAdoColumn(isCaseSensitive(),m_pConnection);
}
// -------------------------------------------------------------------------
// XAppend
void OColumns::appendObject( const Reference< XPropertySet >& descriptor )
{
    OAdoColumn* pColumn = NULL;
    if(getImplementation(pColumn,descriptor) && pColumn != NULL)
    {
        WpADOColumn aColumn = pColumn->getColumnImpl();
        DataTypeEnum eType = aColumn.get_Type();

        sal_Int32 nPrecision    = aColumn.get_Precision();
        sal_Int32 nScale        = aColumn.get_NumericScale();
        sal_Int32 nType         = ADOS::MapADOType2Jdbc(eType);

        ::rtl::OUString sTypeName;
        pColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME)) >>= sTypeName;

        sal_Bool bForceTo = sal_True;
        const OTypeInfoMap* pTypeInfoMap = m_pConnection->getTypeInfo();
        const OExtendedTypeInfo* pTypeInfo = OConnection::getTypeInfoFromType(  *m_pConnection->getTypeInfo()
                                                                                ,nType
                                                                                ,sTypeName
                                                                                ,nPrecision
                                                                                ,nScale
                                                                                ,eType
                                                                                ,bForceTo);
        if ( pTypeInfo && static_cast<DataTypeEnum>(pTypeInfo->eType) != eType ) // change column type if necessary
            aColumn.put_Type(static_cast<DataTypeEnum>(pTypeInfo->eType));

        if ( SUCCEEDED(((ADOColumns*)m_aCollection)->Append(OLEVariant(aColumn.get_Name()),aColumn.get_Type(),aColumn.get_DefinedSize())) )
        {
            WpADOColumn aAddedColumn = m_aCollection.GetItem(OLEVariant(aColumn.get_Name()));
            if ( aAddedColumn.IsValid() )
            {
                sal_Bool bAutoIncrement = sal_False;
                pColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT)) >>= bAutoIncrement;
                if ( bAutoIncrement )
                    OTools::putValue( aAddedColumn.get_Properties(), ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Autoincrement")), bAutoIncrement );

                aAddedColumn.put_Precision(aColumn.get_Precision());
                aAddedColumn.put_NumericScale(aColumn.get_NumericScale());
                aAddedColumn.put_Attributes(aColumn.get_Attributes());
                aAddedColumn.put_SortOrder(aColumn.get_SortOrder());
                aAddedColumn.put_RelatedColumn(aColumn.get_RelatedColumn());
            }
        }
        ADOS::ThrowException(*m_pConnection->getConnection(),*this);
    }
    else
        throw SQLException(::rtl::OUString::createFromAscii("Could not append column!"),*this,OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_HY0000),1000,Any());
}
// -------------------------------------------------------------------------
// XDrop
void OColumns::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    if(!m_aCollection.Delete(_sElementName))
        ADOS::ThrowException(*m_pConnection->getConnection(),*this);
}
// -----------------------------------------------------------------------------
Reference< XNamed > OColumns::cloneObject(const Reference< XPropertySet >& _xDescriptor)
{
    OAdoColumn* pColumn = NULL;
    if(getImplementation(pColumn,_xDescriptor) && pColumn != NULL)
        return new OAdoColumn(isCaseSensitive(),m_pConnection,pColumn->getColumnImpl());
    return Reference< XNamed >();
}
// -----------------------------------------------------------------------------



