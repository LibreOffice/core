/*************************************************************************
 *
 *  $RCSfile: dbtools2.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:53:16 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDRIVERACCESS_HPP_
#include <com/sun/star/sdbc/XDriverAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADEFINITIONSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XDataDefinitionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif

//.........................................................................
namespace dbtools
{
//.........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::frame;
    using namespace connectivity;
    using namespace comphelper;

::rtl::OUString createStandardColumnPart(const Reference< XPropertySet >& xColProp,const Reference< XConnection>& _xConnection)
{

    Reference<XDatabaseMetaData> xMetaData = _xConnection->getMetaData();

    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();

    ::rtl::OUString sTypeName;
    sal_Int32       nDataType   = 0;
    sal_Int32       nPrecision  = 0;
    sal_Int32       nScale      = 0;

    ::rtl::OUString sQuoteString = xMetaData->getIdentifierQuoteString();
    ::rtl::OUString aSql = ::dbtools::quoteName(sQuoteString,::comphelper::getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))));

    aSql += ::rtl::OUString::createFromAscii(" ");

    nDataType = nPrecision = nScale = 0;
    sal_Bool bIsAutoIncrement = sal_False;
    xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_TYPENAME))           >>= sTypeName;
    xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_TYPE))               >>= nDataType;
    xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_PRECISION))          >>= nPrecision;
    xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCALE))              >>= nScale;
    xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT))    >>= bIsAutoIncrement;

    // check if the user enter a specific string to create autoincrement values
    ::rtl::OUString sAutoIncrementValue;
    Reference<XPropertySetInfo> xPropInfo = xColProp->getPropertySetInfo();
    if ( xPropInfo.is() && xPropInfo->hasPropertyByName(rPropMap.getNameByIndex(PROPERTY_ID_AUTOINCREMENTCREATION)) )
        xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_AUTOINCREMENTCREATION)) >>= sAutoIncrementValue;
    // look if we have to use precisions
    sal_Bool bUseLiteral = sal_False;
    ::rtl::OUString sPreFix,sPostFix;
    {
        Reference<XResultSet> xRes = xMetaData->getTypeInfo();
        if(xRes.is())
        {
            Reference<XRow> xRow(xRes,UNO_QUERY);
            while(xRes->next())
            {
                ::rtl::OUString sTypeName2Cmp = xRow->getString(1);
                sal_Int32 nType = xRow->getShort(2);
                sPreFix = xRow->getString (4);
                sPostFix = xRow->getString (5);
                ::rtl::OUString sCreateParams = xRow->getString(6);
                // first identical type will be used if typename is empty
                if ( !sTypeName.getLength() && nType == nDataType )
                    sTypeName = sTypeName2Cmp;

                if( sTypeName.equalsIgnoreAsciiCase(sTypeName2Cmp) && nType == nDataType && sCreateParams.getLength() && !xRow->wasNull())
                {
                    bUseLiteral = sal_True;
                    break;
                }
            }
        }
    }

    sal_Int32 nIndex = 0;
    if ( sAutoIncrementValue.getLength() && (nIndex = sTypeName.indexOf(sAutoIncrementValue)) != -1 )
    {
        sTypeName = sTypeName.replaceAt(nIndex,sTypeName.getLength() - nIndex,::rtl::OUString());
    }


    if ( nPrecision > 0 && bUseLiteral )
    {
        sal_Int32 nParenPos = sTypeName.indexOf('(');
        if ( nParenPos == -1 )
        {
            aSql += sTypeName;
            aSql += ::rtl::OUString::createFromAscii("(");
        }
        else
        {
            aSql += sTypeName.copy(0,++nParenPos);
        }
        aSql += ::rtl::OUString::valueOf(nPrecision);
        if ( nScale > 0 )
        {
            aSql += ::rtl::OUString::createFromAscii(",");
            aSql += ::rtl::OUString::valueOf(nScale);
        }
        if ( nParenPos == -1 )
            aSql += ::rtl::OUString::createFromAscii(")");
        else
        {
            nParenPos = sTypeName.indexOf(')',nParenPos);
            aSql += sTypeName.copy(nParenPos);
        }
    }
    else
        aSql += sTypeName; // simply add the type name

    ::rtl::OUString aDefault = ::comphelper::getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_DEFAULTVALUE)));
    if(aDefault.getLength())
        aSql += ::rtl::OUString::createFromAscii(" DEFAULT ") + sPreFix + aDefault + sPostFix;

    if(::comphelper::getINT32(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_ISNULLABLE))) == ColumnValue::NO_NULLS)
        aSql += ::rtl::OUString::createFromAscii(" NOT NULL");

    if ( bIsAutoIncrement && sAutoIncrementValue.getLength())
    {
        aSql += ::rtl::OUString::createFromAscii(" ");
        aSql += sAutoIncrementValue;
    }

    return aSql;
}
// -----------------------------------------------------------------------------

::rtl::OUString createStandardCreateStatement(const Reference< XPropertySet >& descriptor,const Reference< XConnection>& _xConnection)
{
    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE TABLE ");
    ::rtl::OUString sCatalog,sSchema,sTable,sComposedName;

    Reference<XDatabaseMetaData> xMetaData = _xConnection->getMetaData();
    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();

    descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME))  >>= sCatalog;
    descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME))   >>= sSchema;
    descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))         >>= sTable;

    ::dbtools::composeTableName(xMetaData,sCatalog,sSchema,sTable,sComposedName,sal_True,::dbtools::eInTableDefinitions);
    if ( !sComposedName.getLength() )
        ::dbtools::throwFunctionSequenceException(_xConnection);

    aSql += sComposedName + ::rtl::OUString::createFromAscii(" (");

    // columns
    Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
    Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
    // check if there are columns
    if(!xColumns.is() || !xColumns->getCount())
        ::dbtools::throwFunctionSequenceException(_xConnection);

    Reference< XPropertySet > xColProp;

    sal_Int32 nCount = xColumns->getCount();
    for(sal_Int32 i=0;i<nCount;++i)
    {
        if ( (xColumns->getByIndex(i) >>= xColProp) && xColProp.is() )
        {
            aSql += createStandardColumnPart(xColProp,_xConnection);
            aSql += ::rtl::OUString::createFromAscii(",");
        }
    }
    return aSql;
}
namespace
{
    ::rtl::OUString generateColumnNames(const Reference<XIndexAccess>& _xColumns,const Reference<XDatabaseMetaData>& _xMetaData)
    {
        ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
        static const ::rtl::OUString sComma(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(",")));

        const ::rtl::OUString sQuote(_xMetaData->getIdentifierQuoteString());
        ::rtl::OUString sSql = ::rtl::OUString::createFromAscii(" (");
        Reference< XPropertySet > xColProp;

        sal_Int32 nColCount  = _xColumns->getCount();
        for(sal_Int32 i=0;i<nColCount;++i)
        {
            if ( (_xColumns->getByIndex(i) >>= xColProp) && xColProp.is() )
                sSql += ::dbtools::quoteName(sQuote,::comphelper::getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))))
                        + sComma;
        }

        if ( nColCount )
            sSql = sSql.replaceAt(sSql.getLength()-1,1,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")")));
        return sSql;
    }
}
// -----------------------------------------------------------------------------
::rtl::OUString createStandardKeyStatement(const Reference< XPropertySet >& descriptor,const Reference< XConnection>& _xConnection)
{
    Reference<XDatabaseMetaData> xMetaData = _xConnection->getMetaData();
    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();

    ::rtl::OUString aSql;
    // keys
    Reference<XKeysSupplier> xKeySup(descriptor,UNO_QUERY);
    Reference<XIndexAccess> xKeys = xKeySup->getKeys();
    if ( xKeys.is() )
    {
        Reference< XPropertySet > xColProp;
        Reference<XIndexAccess> xColumns;
        Reference<XColumnsSupplier> xColumnSup;
        ::rtl::OUString sCatalog,sSchema,sTable,sComposedName;
        sal_Bool bPKey = sal_False;
        for(sal_Int32 i=0;i<xKeys->getCount();++i)
        {
            if ( (xKeys->getByIndex(i) >>= xColProp) && xColProp.is() )
            {

                sal_Int32 nKeyType      = ::comphelper::getINT32(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_TYPE)));

                if ( nKeyType == KeyType::PRIMARY )
                {
                    if(bPKey)
                        ::dbtools::throwFunctionSequenceException(_xConnection);

                    bPKey = sal_True;
                    xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                    xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                    if(!xColumns.is() || !xColumns->getCount())
                        ::dbtools::throwFunctionSequenceException(_xConnection);

                    const ::rtl::OUString sQuote     = xMetaData->getIdentifierQuoteString();
                    aSql += ::rtl::OUString::createFromAscii(" PRIMARY KEY ");
                    aSql += generateColumnNames(xColumns,xMetaData);
                }
                else if(nKeyType == KeyType::UNIQUE)
                {
                    xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                    xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                    if(!xColumns.is() || !xColumns->getCount())
                        ::dbtools::throwFunctionSequenceException(_xConnection);

                    const ::rtl::OUString sQuote     = xMetaData->getIdentifierQuoteString();
                    aSql += ::rtl::OUString::createFromAscii(" UNIQUE ");
                    aSql += generateColumnNames(xColumns,xMetaData);
                }
                else if(nKeyType == KeyType::FOREIGN)
                {
                    sal_Int32 nDeleteRule   = getINT32(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_DELETERULE)));

                    xColumnSup = Reference<XColumnsSupplier>(xColProp,UNO_QUERY);
                    xColumns = Reference<XIndexAccess>(xColumnSup->getColumns(),UNO_QUERY);
                    if(!xColumns.is() || !xColumns->getCount())
                        ::dbtools::throwFunctionSequenceException(_xConnection);

                    aSql += ::rtl::OUString::createFromAscii(" FOREIGN KEY ");
                    ::rtl::OUString sRefTable = getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_REFERENCEDTABLE)));
                    ::dbtools::qualifiedNameComponents(xMetaData,
                                                        sRefTable,
                                                        sCatalog,
                                                        sSchema,
                                                        sTable,
                                                        ::dbtools::eInDataManipulation);
                    ::dbtools::composeTableName(xMetaData,sCatalog, sSchema, sTable,sComposedName,sal_True,::dbtools::eInTableDefinitions);


                    if ( !sComposedName.getLength() )
                        ::dbtools::throwFunctionSequenceException(_xConnection);

                    aSql += generateColumnNames(xColumns,xMetaData);

                    switch(nDeleteRule)
                    {
                        case KeyRule::CASCADE:
                            aSql += ::rtl::OUString::createFromAscii(" ON DELETE CASCADE ");
                            break;
                        case KeyRule::RESTRICT:
                            aSql += ::rtl::OUString::createFromAscii(" ON DELETE RESTRICT ");
                            break;
                        case KeyRule::SET_NULL:
                            aSql += ::rtl::OUString::createFromAscii(" ON DELETE SET NULL ");
                            break;
                        case KeyRule::SET_DEFAULT:
                            aSql += ::rtl::OUString::createFromAscii(" ON DELETE SET DEFAULT ");
                            break;
                        default:
                            ;
                    }
                }
            }
        }
    }

    if ( aSql.getLength() )
    {
        if ( aSql.lastIndexOf(',') == (aSql.getLength()-1) )
            aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));
        else
            aSql += ::rtl::OUString::createFromAscii(")");
    }

    return aSql;

}
// -----------------------------------------------------------------------------
::rtl::OUString createSqlCreateTableStatement(  const Reference< XPropertySet >& descriptor,
                                                const Reference< XConnection>& _xConnection)
{
    ::rtl::OUString aSql = ::dbtools::createStandardCreateStatement(descriptor,_xConnection);
    ::rtl::OUString sKeyStmt = ::dbtools::createStandardKeyStatement(descriptor,_xConnection);
    if ( sKeyStmt.getLength() )
        aSql += sKeyStmt;
    else
    {
        if ( aSql.lastIndexOf(',') == (aSql.getLength()-1) )
            aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));
        else
            aSql += ::rtl::OUString::createFromAscii(")");
    }
    return aSql;
}
namespace
{
    Reference<XPropertySet> lcl_createSDBCXColumn(
                                          const Reference<XConnection>& _xConnection,
                                          const Any& _aCatalog,
                                          const ::rtl::OUString& _aSchema,
                                          const ::rtl::OUString& _aTable,
                                          const ::rtl::OUString& _rQueryName,
                                          const ::rtl::OUString& _rName,
                                          sal_Bool _bCase,
                                          sal_Bool _bQueryForInfo,
                                          sal_Bool _bIsAutoIncrement,
                                          sal_Bool _bIsCurrency,
                                          sal_Int32 _nDataType)
    {
        Reference<XPropertySet> xProp;
        Reference<XDatabaseMetaData> xMetaData = _xConnection->getMetaData();
        Reference< XResultSet > xResult = xMetaData->getColumns(_aCatalog, _aSchema, _aTable, _rQueryName);

        if ( xResult.is() )
        {
            UStringMixEqual aMixCompare(_bCase);
            Reference< XRow > xRow(xResult,UNO_QUERY);
            while( xResult->next() )
            {
                if ( aMixCompare(xRow->getString(4),_rName) )
                {
                    sal_Int32       nField5 = xRow->getInt(5);
                    ::rtl::OUString aField6 = xRow->getString(6);
                    sal_Int32       nField7 = xRow->getInt(7)
                                ,   nField9 = xRow->getInt(9)
                                ,   nField11= xRow->getInt(11);
                    ::rtl::OUString sField13 = xRow->getString(13);
                    ::comphelper::disposeComponent(xRow);

                    sal_Bool bAutoIncrement = _bIsAutoIncrement
                            ,bIsCurrency    = _bIsCurrency;
                    if ( _bQueryForInfo )
                    {
                        const ::rtl::OUString sQuote = xMetaData->getIdentifierQuoteString();
                        ::rtl::OUString sQuotedName  = ::dbtools::quoteName(sQuote,_rName);
                        ::rtl::OUString sComposedName;
                        sal_Bool bUseCatalogInSelect = isDataSourcePropertyEnabled(_xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseCatalogInSelect")),sal_True);
                        sal_Bool bUseSchemaInSelect = isDataSourcePropertyEnabled(_xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseSchemaInSelect")),sal_True);
                        ::dbtools::composeTableName(xMetaData,getString(_aCatalog),_aSchema,_aTable,sComposedName,sal_True,::dbtools::eInDataManipulation,bUseCatalogInSelect,bUseSchemaInSelect);

                        ColumnInformationMap aInfo(_bCase);
                        collectColumnInformation(_xConnection,sComposedName,sQuotedName,aInfo);
                        ColumnInformationMap::iterator aIter = aInfo.begin();
                        if ( aIter != aInfo.end() )
                        {
                            bAutoIncrement  = aIter->second.first.first;
                            bIsCurrency     = aIter->second.first.second;
                            if ( DataType::OTHER == nField5 )
                                nField5     = aIter->second.second;
                        }
                    }
                    else if ( DataType::OTHER == nField5 )
                        nField5 = _nDataType;

                    if ( nField11 != ColumnValue::NO_NULLS )
                    {
                        try
                        {
                            Reference< XResultSet > xResult = xMetaData->getPrimaryKeys(_aCatalog, _aSchema, _aTable);
                            Reference< XRow > xRow(xResult,UNO_QUERY);
                            if ( xRow.is() )
                            {
                                while( xResult->next() ) // there can be only one primary key
                                {
                                    ::rtl::OUString sKeyColumn = xRow->getString(4);
                                    if ( aMixCompare(_rName,sKeyColumn) )
                                    {
                                        nField11 = ColumnValue::NO_NULLS;
                                        break;
                                    }
                                }
                            }
                        }
                        catch(SQLException&)
                        {
                        }
                    }

                    connectivity::sdbcx::OColumn* pRet = new connectivity::sdbcx::OColumn(_rName,
                                                aField6,
                                                sField13,
                                                nField11,
                                                nField7,
                                                nField9,
                                                nField5,
                                                bAutoIncrement,
                                                sal_False,
                                                bIsCurrency,
                                                _bCase);

                    xProp = pRet;
                    break;
                }
            }
        }

        return xProp;
    }
    //------------------------------------------------------------------
    Reference< XModel> lcl_getXModel(const Reference< XInterface>& _xIface)
    {
        Reference< XInterface > xParent = _xIface;
        Reference< XModel > xModel(xParent,UNO_QUERY);;
        while( xParent.is() && !xModel.is() )
        {
            Reference<XChild> xChild(xParent,UNO_QUERY);
            xParent.set(xChild.is() ? xChild->getParent() : NULL,UNO_QUERY);
            xModel.set(xParent,UNO_QUERY);
        }
        return xModel;
    }
}
// -----------------------------------------------------------------------------
Reference<XPropertySet> createSDBCXColumn(const Reference<XPropertySet>& _xTable,
                                          const Reference<XConnection>& _xConnection,
                                          const ::rtl::OUString& _rName,
                                          sal_Bool _bCase,
                                          sal_Bool _bQueryForInfo,
                                          sal_Bool _bIsAutoIncrement,
                                          sal_Bool _bIsCurrency,
                                          sal_Int32 _nDataType)
{
    Reference<XPropertySet> xProp;
    OSL_ENSURE(_xTable.is(),"Table is NULL!");
    if ( !_xTable.is() )
        return xProp;

    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    Reference<XDatabaseMetaData> xMetaData = _xConnection->getMetaData();
    Any aCatalog;
    aCatalog = _xTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME));

    ::rtl::OUString aSchema, aTable;
    _xTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME))  >>= aSchema;
    _xTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))        >>= aTable;

    xProp = lcl_createSDBCXColumn(_xConnection,aCatalog, aSchema, aTable, _rName,_rName,_bCase,_bQueryForInfo,_bIsAutoIncrement,_bIsCurrency,_nDataType);
    if ( !xProp.is() )
    {
        xProp = lcl_createSDBCXColumn(_xConnection,aCatalog, aSchema, aTable, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("%")),_rName,_bCase,_bQueryForInfo,_bIsAutoIncrement,_bIsCurrency,_nDataType);
        if ( !xProp.is() )
            xProp = new connectivity::sdbcx::OColumn(_rName,
                                                ::rtl::OUString(),::rtl::OUString(),
                                                ColumnValue::NULLABLE_UNKNOWN,
                                                0,
                                                0,
                                                DataType::VARCHAR,
                                                _bIsAutoIncrement,
                                                sal_False,
                                                _bIsCurrency,
                                                _bCase);

    }

    return xProp;
}
// -----------------------------------------------------------------------------
::rtl::OUString composeTableName(const Reference<XDatabaseMetaData>& _xMetaData,
                                 const Reference<XPropertySet>& _xTable,
                                 sal_Bool _bQuote,
                                 EComposeRule _eComposeRule
                                 , sal_Bool _bUseCatalogInSelect
                                , sal_Bool _bUseSchemaInSelect)
{
    ::rtl::OUString aComposedName;
    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    Reference< XPropertySetInfo > xInfo = _xTable->getPropertySetInfo();
    if (    xInfo.is()
        &&  xInfo->hasPropertyByName(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME))
        &&  xInfo->hasPropertyByName(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME))
        &&  xInfo->hasPropertyByName(rPropMap.getNameByIndex(PROPERTY_ID_NAME)) )
    {

        ::rtl::OUString aCatalog;
        ::rtl::OUString aSchema;
        ::rtl::OUString aTable;
        _xTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)) >>= aCatalog;
        _xTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME))  >>= aSchema;
        _xTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))        >>= aTable;

        dbtools::composeTableName(_xMetaData,aCatalog,aSchema,aTable,aComposedName,_bQuote,_eComposeRule,_bUseCatalogInSelect,_bUseSchemaInSelect);
    }
    return aComposedName;
}
// -----------------------------------------------------------------------------
sal_Bool isDataSourcePropertyEnabled(const Reference<XInterface>& _xProp,const ::rtl::OUString& _sProperty,sal_Bool _bDefault)
{
    sal_Bool bEnabled = _bDefault;
    try
    {
        Reference< XPropertySet> xProp(findDataSource(_xProp),UNO_QUERY);
        if ( xProp.is() )
        {
            Sequence< PropertyValue > aInfo;
            xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Info"))) >>= aInfo;
            const PropertyValue* pValue =::std::find_if(aInfo.getConstArray(),
                                                aInfo.getConstArray() + aInfo.getLength(),
                                                ::std::bind2nd(TPropertyValueEqualFunctor(),_sProperty));
            if ( pValue && pValue != (aInfo.getConstArray() + aInfo.getLength()) )
                pValue->Value >>= bEnabled;
        }
    }
    catch(SQLException&)
    {
        OSL_ASSERT(!"isDataSourcePropertyEnabled");
    }
    return bEnabled;
}
// -----------------------------------------------------------------------------
Reference< XTablesSupplier> getDataDefinitionByURLAndConnection(
            const ::rtl::OUString& _rsUrl,
            const Reference< XConnection>& _xConnection,
            const Reference< XMultiServiceFactory>& _rxFactory)
{
    Reference< XDriverAccess> xManager(_rxFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.sdbc.DriverManager") ), UNO_QUERY);
    Reference< XDataDefinitionSupplier > xSupp(xManager->getDriverByURL(_rsUrl),UNO_QUERY);
    Reference< XTablesSupplier> xTablesSup;

    if ( xSupp.is() )
        xTablesSup = xSupp->getDataDefinitionByConnection(_xConnection);
    // if we don't get the catalog from the original driver we have to try them all.
    if ( !xTablesSup.is() )
    {
        Reference< XEnumerationAccess> xEnumAccess(xManager,UNO_QUERY);
        Reference< XEnumeration> xEnum = xEnumAccess->createEnumeration();
        while ( xEnum.is() && xEnum->hasMoreElements() && !xTablesSup.is() )
        {
            xEnum->nextElement() >>= xSupp;
            if ( xSupp.is() )
                xTablesSup = xSupp->getDataDefinitionByConnection(_xConnection);
        }
    }
    return xTablesSup;
}
// -----------------------------------------------------------------------------
sal_Int32 getTablePrivileges(const Reference< XDatabaseMetaData>& _xMetaData,
                             const ::rtl::OUString& _sCatalog,
                             const ::rtl::OUString& _sSchema,
                             const ::rtl::OUString& _sTable)
{
    OSL_ENSURE(_xMetaData.is(),"Invalid metadata!");
    sal_Int32 nPrivileges = 0;
    try
    {
        Any aVal;
        if(_sCatalog.getLength())
            aVal <<= _sCatalog;
        Reference< XResultSet > xPrivileges = _xMetaData->getTablePrivileges(aVal, _sSchema, _sTable);
        Reference< XRow > xCurrentRow(xPrivileges, UNO_QUERY);

        if ( xCurrentRow.is() )
        {
            ::rtl::OUString sUserWorkingFor = _xMetaData->getUserName();
            static const ::rtl::OUString sSELECT    = ::rtl::OUString::createFromAscii("SELECT");
            static const ::rtl::OUString sINSERT    = ::rtl::OUString::createFromAscii("INSERT");
            static const ::rtl::OUString sUPDATE    = ::rtl::OUString::createFromAscii("UPDATE");
            static const ::rtl::OUString sDELETE    = ::rtl::OUString::createFromAscii("DELETE");
            static const ::rtl::OUString sREAD      = ::rtl::OUString::createFromAscii("READ");
            static const ::rtl::OUString sCREATE    = ::rtl::OUString::createFromAscii("CREATE");
            static const ::rtl::OUString sALTER     = ::rtl::OUString::createFromAscii("ALTER");
            static const ::rtl::OUString sREFERENCE = ::rtl::OUString::createFromAscii("REFERENCE");
            static const ::rtl::OUString sDROP      = ::rtl::OUString::createFromAscii("DROP");
            // after creation the set is positioned before the first record, per definitionem
#ifdef DBG_UTIL
            Reference< XResultSetMetaDataSupplier > xSup(xPrivileges,UNO_QUERY);
            if ( xSup.is() )
            {
                Reference< XResultSetMetaData > xRsMetaData = xSup->getMetaData();
                if ( xRsMetaData.is() )
                {
                    sal_Int32 nCount = xRsMetaData->getColumnCount();
                    for (sal_Int32 i=1; i<=nCount; ++i)
                    {
                        ::rtl::OUString sColumnName = xRsMetaData->getColumnName(i);
                    }
                }
            }
#endif

            ::rtl::OUString sPrivilege, sGrantee;
            while ( xPrivileges->next() )
            {
#ifdef DBG_UTIL
                ::rtl::OUString sCat, sSchema, sName, sGrantor, sGrantable;
                sCat        = xCurrentRow->getString(1);
                sSchema     = xCurrentRow->getString(2);
                sName       = xCurrentRow->getString(3);
                sGrantor    = xCurrentRow->getString(4);
#endif
                sGrantee    = xCurrentRow->getString(5);
                sPrivilege  = xCurrentRow->getString(6);
#ifdef DBG_UTIL
                sGrantable  = xCurrentRow->getString(7);
#endif

                if (!sUserWorkingFor.equalsIgnoreAsciiCase(sGrantee))
                    continue;

                if (sPrivilege.equalsIgnoreAsciiCase(sSELECT))
                    nPrivileges |= Privilege::SELECT;
                else if (sPrivilege.equalsIgnoreAsciiCase(sINSERT))
                    nPrivileges |= Privilege::INSERT;
                else if (sPrivilege.equalsIgnoreAsciiCase(sUPDATE))
                    nPrivileges |= Privilege::UPDATE;
                else if (sPrivilege.equalsIgnoreAsciiCase(sDELETE))
                    nPrivileges |= Privilege::DELETE;
                else if (sPrivilege.equalsIgnoreAsciiCase(sREAD))
                    nPrivileges |= Privilege::READ;
                else if (sPrivilege.equalsIgnoreAsciiCase(sCREATE))
                    nPrivileges |= Privilege::CREATE;
                else if (sPrivilege.equalsIgnoreAsciiCase(sALTER))
                    nPrivileges |= Privilege::ALTER;
                else if (sPrivilege.equalsIgnoreAsciiCase(sREFERENCE))
                    nPrivileges |= Privilege::REFERENCE;
                else if (sPrivilege.equalsIgnoreAsciiCase(sDROP))
                    nPrivileges |= Privilege::DROP;
            }
        }
        disposeComponent(xPrivileges);
    }
    catch(const SQLException& e)
    {
        static ::rtl::OUString sNotSupportedState = ::rtl::OUString::createFromAscii("IM001");
        // some drivers don't support any privileges so we assume that we are allowed to do all we want :-)
        if(e.SQLState == sNotSupportedState)
            nPrivileges |=  Privilege::DROP         |
                            Privilege::REFERENCE    |
                            Privilege::ALTER        |
                            Privilege::CREATE       |
                            Privilege::READ         |
                            Privilege::DELETE       |
                            Privilege::UPDATE       |
                            Privilege::INSERT       |
                            Privilege::SELECT;
        else
            OSL_ENSURE(0,"Could not collect the privileges !");
    }
    return nPrivileges;
}
// -----------------------------------------------------------------------------
// we need some more information about the column
void collectColumnInformation(const Reference< XConnection>& _xConnection,
                              const ::rtl::OUString& _sComposedName,
                              const ::rtl::OUString& _rName,
                              ColumnInformationMap& _rInfo)
{
    static ::rtl::OUString STR_WHERE = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" WHERE "));

    ::rtl::OUString sSelect = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT "));
    sSelect += _rName;
    sSelect += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FROM "));
    sSelect += _sComposedName;
    sSelect += STR_WHERE;
    sSelect += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0 = 1"));

    try
    {
        Reference<XStatement> xStmt = _xConnection->createStatement();
        Reference<XResultSet> xResult = xStmt->executeQuery(sSelect);
        if ( xResult.is() )
        {
            Reference<XResultSetMetaData> xMD = Reference<XResultSetMetaDataSupplier>(xResult,UNO_QUERY)->getMetaData();
            if ( xMD.is() )
            {
                sal_Int32 nCount = xMD->getColumnCount();
                for (sal_Int32 i=1; i <= nCount ; ++i)
                {
                    _rInfo.insert(ColumnInformationMap::value_type(xMD->getColumnName(i),
                        ColumnInformation(TBoolPair(xMD->isAutoIncrement(i),xMD->isCurrency(i)),xMD->getColumnType(i))));
                }
                xMD = NULL;
            }
            xResult = NULL;
            ::comphelper::disposeComponent(xStmt);
        }
    }
    catch(SQLException&)
    {
    }
    catch(DisposedException&)
    {
        OSL_ENSURE(0,"Exception catched!");
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception catched!");
    }
}
// -----------------------------------------------------------------------------
Reference< XConnection > getActiveConnectionFromParent(const Reference< XInterface >& _xChild)
{
    Reference< XConnection >  xConnection;
    try
    {
        Reference< XModel > xModel = lcl_getXModel(_xChild);

        if ( xModel.is() )
        {
            Sequence< PropertyValue > aArgs = xModel->getArgs();
            const PropertyValue* pIter = aArgs.getConstArray();
            const PropertyValue* pEnd  = pIter + aArgs.getLength();
            for(;pIter != pEnd;++pIter)
            {
                if ( pIter->Name.equalsAscii("ComponentData") )
                {
                    Sequence<PropertyValue> aDocumentContext;
                    pIter->Value >>= aDocumentContext;
                    const PropertyValue* pContextIter = aDocumentContext.getConstArray();
                    const PropertyValue* pContextEnd  = pContextIter + aDocumentContext.getLength();
                    for(;pContextIter != pContextEnd;++pContextIter)
                    {
                        if ( pContextIter->Name.equalsAscii("ActiveConnection") && (pContextIter->Value >>= xConnection) && xConnection.is() )
                        {
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }
    catch(Exception&)
    {
        // not intereseted in
    }
    return xConnection;
}
//.........................................................................
}   // namespace dbtools
//.........................................................................

