/*************************************************************************
 *
 *  $RCSfile: ComboBox.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-27 17:03:58 $
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

#ifndef _FORMS_COMBOBOX_HXX_
#include "ComboBox.hxx"
#endif
#ifndef _FRM_PROPERTY_HXX_
#include "property.hxx"
#endif
#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _FRM_RESOURCE_HXX_
#include "frm_resource.hxx"
#endif
#ifndef _FRM_RESOURCE_HRC_
#include "frm_resource.hrc"
#endif
#ifndef _FORMS_BASELISTBOX_HXX_
#include "BaseListBox.hxx"
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif
#ifndef _COMPHELPER_DATETIME_HXX_
#include <comphelper/datetime.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif

#ifndef _COM_SUN_STAR_SDB_SQLERROREVENT_HPP_
#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLISTBOX_HPP_
#include <com/sun/star/awt/XListBox.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif

using namespace dbtools;

//.........................................................................
namespace frm
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

//========================================================================
// class OComboBoxModel
//========================================================================
sal_Int32 OComboBoxModel::nTextHandle = -1;

//------------------------------------------------------------------
InterfaceRef SAL_CALL OComboBoxModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return (*new OComboBoxModel(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OComboBoxModel::_getTypes()
{
        static Sequence<Type> aTypes;
    if (!aTypes.getLength())
    {
        // my two base classes
                Sequence<Type> aBaseClassTypes = OBoundControlModel::_getTypes();

                Sequence<Type> aOwnTypes(1);
                Type* pOwnTypes = aOwnTypes.getArray();
                pOwnTypes[0] = getCppuType((Reference<XSQLErrorBroadcaster>*)NULL);

        aTypes = concatSequences(aBaseClassTypes, aOwnTypes);
    }
    return aTypes;
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OComboBoxModel::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = ::rtl::OUString::createFromAscii("com.sun.star.form.component.DatabaseComboBox");
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_COMBOBOX;
    return aSupported;
}

//------------------------------------------------------------------------------
Any SAL_CALL OComboBoxModel::queryAggregation(const Type& _rType) throw (RuntimeException)
{
        Any aReturn;

    aReturn = OBoundControlModel::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<XSQLErrorBroadcaster*>(this)
        );

    return aReturn;
}

//------------------------------------------------------------------
OComboBoxModel::OComboBoxModel(const Reference<XMultiServiceFactory>& _rxFactory)
                 :OBoundControlModel(_rxFactory, VCL_CONTROLMODEL_COMBOBOX, FRM_CONTROL_COMBOBOX)
                                    // use the old control name for compytibility reasons
                 ,m_eListSourceType(ListSourceType_TABLE)
                 ,m_bEmptyIsNull(sal_True)
                 ,m_aNullDate(DBTypeConversion::getStandardDate())
                 ,m_nKeyType(NumberFormat::UNDEFINED)
                 ,m_nFormatKey(0)
                 ,m_nFieldType(DataType::OTHER)
                 ,m_aErrorListeners(m_aMutex)
{
    m_nClassId = FormComponentType::COMBOBOX;
    m_sDataFieldConnectivityProperty = PROPERTY_TEXT;
    if (OComboBoxModel::nTextHandle == -1)
        OComboBoxModel::nTextHandle = getOriginalHandle(PROPERTY_ID_TEXT);
}

//------------------------------------------------------------------
OComboBoxModel::~OComboBoxModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }
}

//------------------------------------------------------------------------------
void OComboBoxModel::disposing()
{
    OBoundControlModel::disposing();
    m_xFormatter = NULL;
}

//------------------------------------------------------------------------------
void OComboBoxModel::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    switch (_nHandle)
    {
        case PROPERTY_ID_LISTSOURCETYPE : _rValue <<= m_eListSourceType; break;
        case PROPERTY_ID_LISTSOURCE     : _rValue <<= m_aListSource; break;
        case PROPERTY_ID_EMPTY_IS_NULL  : _rValue <<= m_bEmptyIsNull; break;
        case PROPERTY_ID_DEFAULT_TEXT   : _rValue <<= m_aDefaultText; break;
        default:
            OBoundControlModel::getFastPropertyValue(_rValue, _nHandle);
    }
}

//------------------------------------------------------------------------------
void OComboBoxModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue)
                        throw (Exception)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_LISTSOURCETYPE :
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(reinterpret_cast<ListSourceType*>(NULL))),
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_eListSourceType;
            break;

        case PROPERTY_ID_LISTSOURCE :
                        DBG_ASSERT(_rValue.getValueType().getTypeClass() == TypeClass_STRING,
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aListSource;
            // die ListSource hat sich geaendert -> neu laden
            if (ListSourceType_VALUELIST != m_eListSourceType)
            {
                if (m_xCursor.is() && !m_xField.is()) // combobox bereits mit Datenbank verbunden ?
                    // neu laden
                    loadData();
            }
            break;

        case PROPERTY_ID_EMPTY_IS_NULL :
                        DBG_ASSERT(_rValue.getValueType().getTypeClass() == TypeClass_BOOLEAN,
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_bEmptyIsNull;
            break;

        case PROPERTY_ID_DEFAULT_TEXT :
                        DBG_ASSERT(_rValue.getValueType().getTypeClass() == TypeClass_STRING,
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aDefaultText;
            _reset();
            break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    }
}

//------------------------------------------------------------------------------
sal_Bool OComboBoxModel::convertFastPropertyValue(
                        Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue)
                        throw (IllegalArgumentException)
{
    sal_Bool bModified(sal_False);
    switch (_nHandle)
    {
        case PROPERTY_ID_LISTSOURCETYPE :
            bModified = tryPropertyValueEnum(_rConvertedValue, _rOldValue, _rValue, m_eListSourceType);
            break;

        case PROPERTY_ID_LISTSOURCE :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aListSource);
            break;

        case PROPERTY_ID_EMPTY_IS_NULL :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_bEmptyIsNull);
            break;

        case PROPERTY_ID_DEFAULT_TEXT :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aDefaultText);
            break;

        default:
            bModified = OBoundControlModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL OComboBoxModel::getPropertySetInfo() throw(RuntimeException)
{
        Reference<XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
cppu::IPropertyArrayHelper& OComboBoxModel::getInfoHelper()
{
    return *const_cast<OComboBoxModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OComboBoxModel::fillProperties(
                Sequence< Property >& _rProps,
                Sequence< Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(13)
        // Text auf transient setzen
                ModifyPropertyAttributes(_rAggregateProps, PROPERTY_TEXT, PropertyAttribute::TRANSIENT, 0);

        DECL_PROP2(CLASSID,             sal_Int16,                  READONLY, TRANSIENT);
        DECL_PROP1(NAME,                ::rtl::OUString,            BOUND);
        DECL_PROP1(TAG,                 ::rtl::OUString,            BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,                  BOUND);
        DECL_PROP1(LISTSOURCETYPE,      ListSourceType, BOUND);
        DECL_PROP1(LISTSOURCE,          ::rtl::OUString,            BOUND);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                              BOUND);
        DECL_PROP1(DEFAULT_TEXT,        ::rtl::OUString,            BOUND);
        DECL_PROP1(CONTROLSOURCE,       ::rtl::OUString,            BOUND);
        DECL_PROP1(HELPTEXT,            ::rtl::OUString,            BOUND);
                DECL_IFACE_PROP2(BOUNDFIELD,    XPropertySet,        READONLY, TRANSIENT);
                DECL_IFACE_PROP2(CONTROLLABEL,  XPropertySet,        BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,   rtl::OUString,  READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OComboBoxModel::getServiceName() throw(RuntimeException)
{
    return FRM_COMPONENT_COMBOBOX;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::write(const Reference<stario::XObjectOutputStream>& _rxOutStream)
        throw(stario::IOException, RuntimeException)
{
    OBoundControlModel::write(_rxOutStream);

    // Version
    // Version 0x0002:  EmptyIsNull
    // Version 0x0003:  ListSource->Seq
    // Version 0x0004:  DefaultText
    // Version 0x0005:  HelpText
    _rxOutStream->writeShort(0x0006);

    // Maskierung fuer any
    sal_uInt16 nAnyMask = 0;
    if (m_aBoundColumn.getValueType().getTypeClass() == TypeClass_SHORT)
        nAnyMask |= BOUNDCOLUMN;
    _rxOutStream << nAnyMask;

    StringSequence aListSourceSeq(&m_aListSource, 1);
    _rxOutStream << aListSourceSeq;
    _rxOutStream << (sal_Int16)m_eListSourceType;

    if ((nAnyMask & BOUNDCOLUMN) == BOUNDCOLUMN)
    {
        sal_Int16 nBoundColumn;
        m_aBoundColumn >>= nBoundColumn;
        _rxOutStream << nBoundColumn;
    }

    _rxOutStream << (sal_Bool)m_bEmptyIsNull;
    _rxOutStream << m_aDefaultText;
    _rxOutStream << m_aHelpText;

    // from version 0x0006 : common properties
    writeCommonProperties(_rxOutStream);
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::read(const Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, RuntimeException)
{
    OBoundControlModel::read(_rxInStream);
    ::osl::MutexGuard aGuard(m_aMutex);

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();
    DBG_ASSERT(nVersion > 0, "OComboBoxModel::read : version 0 ? this should never have been written !");

    if (nVersion > 0x0006)
    {
        DBG_ERROR("OComboBoxModel::read : invalid (means unknown) version !");
        m_aListSource = ::rtl::OUString();
        m_aBoundColumn <<= (sal_Int16)0;
        m_aDefaultText = ::rtl::OUString();
        m_eListSourceType = ListSourceType_TABLE;
        m_bEmptyIsNull = sal_True;
        defaultCommonProperties();
        return;
    }

    // Maskierung fuer any
    sal_uInt16 nAnyMask;
    _rxInStream >> nAnyMask;

    // ListSource
    if (nVersion < 0x0003)
    {
        ::rtl::OUString sListSource;
        _rxInStream >> m_aListSource;
    }
    else // nVersion == 4
    {
        m_aListSource = ::rtl::OUString();
        StringSequence aListSource;
        _rxInStream >> aListSource;
        const ::rtl::OUString* pToken = aListSource.getConstArray();
        sal_Int32 nLen = aListSource.getLength();
        for (sal_Int32 i = 0; i < nLen; ++i, ++pToken)
            m_aListSource += *pToken;
    }

    sal_Int16 nValue;
    _rxInStream >> nValue;
    m_eListSourceType = (ListSourceType)nValue;

    if ((nAnyMask & BOUNDCOLUMN) == BOUNDCOLUMN)
    {
        sal_Int16 nValue;
        _rxInStream >> nValue;
        m_aBoundColumn <<= nValue;
    }

    if (nVersion > 0x0001)
    {
        sal_Bool bNull;
        _rxInStream >> bNull;
        m_bEmptyIsNull = bNull;
    }

    if (nVersion > 0x0003)  // nVersion == 4
        _rxInStream >> m_aDefaultText;

    // Stringliste muß geleert werden, wenn eine Listenquelle gesetzt ist
    // dieses kann der Fall sein wenn im alive modus gespeichert wird
    if (m_aListSource.len() && m_xAggregateSet.is())
    {
        StringSequence aSequence;
        m_xAggregateSet->setPropertyValue(PROPERTY_STRINGITEMLIST, makeAny(aSequence));
    }

    if (nVersion > 0x0004)
        _rxInStream >> m_aHelpText;

    if (nVersion > 0x0005)
        readCommonProperties(_rxInStream);

    // Nach dem Lesen die Defaultwerte anzeigen
    if (m_aControlSource.getLength())
        // (not if we don't have a control source - the "State" property acts like it is persistent, then
        _reset();
}

//------------------------------------------------------------------------------
void OComboBoxModel::loadData()
{
    DBG_ASSERT(m_eListSourceType != ListSourceType_VALUELIST, "OComboBoxModel::loadData : do not call for a value list !");
    ////
    // Connection holen
    Reference<XRowSet> xForm(m_xCursor, UNO_QUERY);
    if (!xForm.is())
        return;
    Reference<XConnection> xConnection = getConnection(xForm);
    if (!xConnection.is())
        return;

    // we need a com::sun::star::sdb::Connection for some of the code below ...
    Reference<XServiceInfo> xServiceInfo(xConnection, UNO_QUERY);
    if (!xServiceInfo.is() || !xServiceInfo->supportsService(SRV_SDB_CONNECTION))
    {
        DBG_ERROR("OComboBoxModel::loadData : invalid connection !");
        return;
    }

    Reference<XStatement> xStmt;
    Reference<XResultSet> xListCursor;

    if (!m_aListSource.len() || m_eListSourceType == ListSourceType_VALUELIST)
        return;

    try
    {
        switch (m_eListSourceType)
        {
            case ListSourceType_TABLEFIELDS:
                // don't work with a statement here, the fields will be collected below
                break;
            case ListSourceType_TABLE:
            {
                // does the bound field belong to the table ?
                // if we use an alias for the bound field, we won't find it
                // in that case we use the first field of the table

                Reference<XNameAccess> xFieldsByName = getTableFields(xConnection, m_aListSource);
                Reference<XIndexAccess> xFieldsByIndex(xFieldsByName, UNO_QUERY);

                ::rtl::OUString aFieldName;
                if (xFieldsByName.is() && xFieldsByName->hasByName(m_aControlSource))
                {
                    aFieldName = m_aControlSource;
                }
                else
                {
                    // otherwise look for the alias
                    Reference<XSQLQueryComposerFactory> xFactory(xConnection, UNO_QUERY);
                    if (!xFactory.is())
                        break;

                    Reference<XSQLQueryComposer> xComposer = xFactory->createQueryComposer();
                    try
                    {
                        Reference<XPropertySet> xFormAsSet(xForm, UNO_QUERY);
                        ::rtl::OUString aStatement;
                        xFormAsSet->getPropertyValue(PROPERTY_ACTIVECOMMAND) >>= aStatement;
                        xComposer->setQuery(aStatement);
                    }
                    catch(Exception&)
                    {
                        disposeComponent(xComposer);
                        break;
                    }

                    // search the field
                                        Reference<XColumnsSupplier> xSupplyFields(xComposer, UNO_QUERY);
                    DBG_ASSERT(xSupplyFields.is(), "OComboBoxModel::loadData : invalid query composer !");

                                        Reference<XNameAccess> xFieldNames = xSupplyFields->getColumns();
                    if (xFieldNames->hasByName(m_aControlSource))
                    {
                                                Reference<XPropertySet> xComposerFieldAsSet(*(Reference<XPropertySet>*)xFieldNames->getByName(m_aControlSource).getValue());
                        if (hasProperty(PROPERTY_FIELDSOURCE, xComposerFieldAsSet))
                            xComposerFieldAsSet->getPropertyValue(PROPERTY_FIELDSOURCE) >>= aFieldName;
                    }

                    disposeComponent(xComposer);
                }

                if (!aFieldName.len())
                    break;

                Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
                ::rtl::OUString aQuote = xMeta->getIdentifierQuoteString();
                ::rtl::OUString aStatement = ::rtl::OUString::createFromAscii("SELECT DISTINCT ");

                aStatement += quoteName(aQuote, aFieldName);
                aStatement += ::rtl::OUString::createFromAscii(" FROM ");
                aStatement += quoteTableName(xMeta, m_aListSource);

                xStmt = xConnection->createStatement();
                xListCursor = xStmt->executeQuery(aStatement);
            }   break;
            case ListSourceType_QUERY:
            {
                Reference<XQueriesSupplier> xSupplyQueries(xConnection, UNO_QUERY);
                Reference<XPropertySet> xQuery(*(InterfaceRef*)xSupplyQueries->getQueries()->getByName(m_aListSource).getValue(), UNO_QUERY);
                xStmt = xConnection->createStatement();
                Reference<XPropertySet>(xStmt, UNO_QUERY)->setPropertyValue(PROPERTY_ESCAPE_PROCESSING, xQuery->getPropertyValue(PROPERTY_ESCAPE_PROCESSING));

                ::rtl::OUString sStatement;
                xQuery->getPropertyValue(PROPERTY_COMMAND) >>= sStatement;
                xListCursor = xStmt->executeQuery(sStatement);
            }   break;
            default:
            {
                xStmt = xConnection->createStatement();
                if (ListSourceType_SQLPASSTHROUGH == m_eListSourceType)
                {
                    Reference<XPropertySet> xStatementProps(xStmt, UNO_QUERY);
                    xStatementProps->setPropertyValue(PROPERTY_ESCAPE_PROCESSING, makeAny(sal_Bool(sal_False)));
                }
                xListCursor = xStmt->executeQuery(m_aListSource);
            }
        }
    }
    catch(SQLException& eSQL)
    {
        onError(eSQL, FRM_RES_STRING(RID_BASELISTBOX_ERROR_FILLLIST));
        disposeComponent(xListCursor);
        disposeComponent(xStmt);
        return;
    }
    catch(Exception& eUnknown)
    {
        eUnknown;
        disposeComponent(xListCursor);
        disposeComponent(xStmt);
        return;
    }

    if (ListSourceType_TABLEFIELDS != m_eListSourceType && !xListCursor.is())
        // something went wrong ...
        return;

    vector< ::rtl::OUString >   aStringList;
    aStringList.reserve(16);
    try
    {
        switch (m_eListSourceType)
        {
            case ListSourceType_SQL:
            case ListSourceType_SQLPASSTHROUGH:
            case ListSourceType_TABLE:
            case ListSourceType_QUERY:
            {
                // die XDatabaseVAriant der ersten Spalte
                Reference<XColumnsSupplier> xSupplyCols(xListCursor, UNO_QUERY);
                DBG_ASSERT(xSupplyCols.is(), "OComboBoxModel::loadData : cursor supports the row set service but is no column supplier ??!");
                Reference<XIndexAccess> xColumns;
                if (xSupplyCols.is())
                {
                    xColumns = Reference<XIndexAccess>(xSupplyCols->getColumns(), UNO_QUERY);
                    DBG_ASSERT(xColumns.is(), "OComboBoxModel::loadData : no columns supplied by the row set !");
                }
                Reference<XColumn> xDataField;
                if (xColumns.is())
                    xDataField = Reference<XColumn>(*(InterfaceRef*)xColumns->getByIndex(0).getValue(), UNO_QUERY);
                if (!xDataField.is())
                {
                    disposeComponent(xListCursor);
                    return;
                }

                // Listen fuellen
                sal_Int16 i = 0;
                // per definitionem the list cursor is positioned _before_ the first row at the moment
                while (xListCursor->next() && (i++<SHRT_MAX)) // max anzahl eintraege
                {

                    aStringList.push_back(DBTypeConversion::getValue(xDataField,
                                            m_xFormatter,
                                            m_aNullDate,
                                            m_nFormatKey,
                                            m_nKeyType));
                }
            }
            break;
            case ListSourceType_TABLEFIELDS:
            {
                Reference<XNameAccess> xFieldNames = getTableFields(xConnection, m_aListSource);
                if (xFieldNames.is())
                {
                    StringSequence seqNames = xFieldNames->getElementNames();
                    sal_Int32 nFieldsCount = seqNames.getLength();
                    const ::rtl::OUString* pustrNames = seqNames.getConstArray();

                    for (sal_Int32 k=0; k<nFieldsCount; ++k)
                        aStringList.push_back(pustrNames[k]);
                }
            }
            break;
        }
    }
    catch(SQLException& eSQL)
    {
        onError(eSQL, FRM_RES_STRING(RID_BASELISTBOX_ERROR_FILLLIST));
        disposeComponent(xListCursor);
        disposeComponent(xStmt);
        return;
    }
    catch(Exception& eUnknown)
    {
        eUnknown;
        disposeComponent(xListCursor);
        disposeComponent(xStmt);
        return;
    }

        // String-Sequence fuer ListBox erzeugen
    StringSequence aStringSeq(aStringList.size());
    ::rtl::OUString* pStringAry = aStringSeq.getArray();
    for (sal_Int32 i = 0; i<aStringSeq.getLength(); ++i)
        pStringAry[i] = aStringList[i];

    // String-Sequence an ListBox setzen
    m_xAggregateSet->setPropertyValue(PROPERTY_STRINGITEMLIST, makeAny(aStringSeq));

    // destroy cursor & statement
    disposeComponent(xListCursor);
    disposeComponent(xStmt);
}

//------------------------------------------------------------------------------
void OComboBoxModel::_loaded(const EventObject& rEvent)
{
    if (m_xField.is())
    {
        // jetzt den Key und typ ermitteln
        m_xField->getPropertyValue(PROPERTY_FIELDTYPE) >>= m_nFieldType;
        m_xField->getPropertyValue(PROPERTY_FORMATKEY) >>= m_nFormatKey;

        // XNumberFormatter besorgen
                Reference<XRowSet> xRowSet(rEvent.Source, UNO_QUERY);
        DBG_ASSERT(xRowSet.is(), "OComboBoxModel::_loaded : invalid event source !");
                Reference<XNumberFormatsSupplier> xSupplier = getNumberFormats(getConnection(xRowSet), sal_False, m_xServiceFactory);
        if (xSupplier.is())
        {
            m_xFormatter =  Reference<XNumberFormatter>(
                                                    m_xServiceFactory->createInstance(FRM_NUMBER_FORMATTER), UNO_QUERY
                                );
            if (m_xFormatter.is())
                m_xFormatter->attachNumberFormatsSupplier(xSupplier);

            m_nKeyType  = getNumberFormatType(xSupplier->getNumberFormats(), m_nFormatKey);
            xSupplier->getNumberFormatSettings()->getPropertyValue(::rtl::OUString::createFromAscii("NullDate")) >>= m_aNullDate;
        }
    }

    m_xAggregateSet->getPropertyValue(PROPERTY_STRINGITEMLIST) >>= m_aDesignModeStringItems;

    // Daten nur laden, wenn eine Listenquelle angegeben wurde
    if (m_aListSource.len() && m_xCursor.is())
        loadData();
}

//------------------------------------------------------------------------------
void OComboBoxModel::_unloaded()
{
    if (m_xField.is())
    {
        m_xFormatter = 0;
        m_nFieldType = DataType::OTHER;
        m_nFormatKey = 0;
        m_nKeyType   = NumberFormat::UNDEFINED;
        m_aNullDate  = DBTypeConversion::getStandardDate();
    }

    // reset the string item list
    m_xAggregateSet->setPropertyValue(PROPERTY_STRINGITEMLIST, makeAny(m_aDesignModeStringItems));
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::reloaded( const EventObject& aEvent ) throw(RuntimeException)
{
    OBoundControlModel::reloaded(aEvent);

    // reload data if we have a list source
    if (m_aListSource.len() && m_xCursor.is())
        loadData();
}

// XBoundComponent
//------------------------------------------------------------------------------
sal_Bool OComboBoxModel::_commit()
{
    ::rtl::OUString aNewValue;
    m_xAggregateFastSet->getFastPropertyValue(OComboBoxModel::nTextHandle) >>= aNewValue;
    sal_Bool bModified = (aNewValue != m_aSaveValue);
    if (bModified)
    {
        if (!aNewValue.getLength() && !m_bRequired && m_bEmptyIsNull)
            m_xColumnUpdate->updateNull();
        else
        {
            try
            {
                DBTypeConversion::setValue(m_xColumnUpdate, m_xFormatter, m_aNullDate, aNewValue,
                                           m_nFormatKey, m_nFieldType, m_nKeyType);
            }
            catch(Exception&)
            {
                return sal_False;
            }
        }
        m_aSaveValue = aNewValue;
    }

    // add the new value to the list
    sal_Bool bAddToList = bModified;
    // (only if this is not the "commit" triggered by a "reset")
    if  (m_bResetting)
        bAddToList = sal_False;

    Any aAnyList = m_xAggregateSet->getPropertyValue(PROPERTY_STRINGITEMLIST);
    if (bAddToList && aAnyList.getValueType().equals(::getCppuType(reinterpret_cast<StringSequence*>(NULL))))
    {
        StringSequence aStringItemList = *(StringSequence*)aAnyList.getValue();
        const ::rtl::OUString* pStringItems = aStringItemList.getConstArray();
        sal_Int32 i;
        for (i=0; i<aStringItemList.getLength(); ++i, ++pStringItems)
        {
            if (pStringItems->equals(aNewValue))
                break;
        }

        // not found -> add
        if (i >= aStringItemList.getLength())
        {
            sal_Int32 nOldLen = aStringItemList.getLength();
            aStringItemList.realloc(nOldLen + 1);
            ::rtl::OUString* pStringItems = aStringItemList.getArray() + nOldLen;
            *pStringItems = aNewValue;

            aAnyList <<= aStringItemList;
            m_xAggregateSet->setPropertyValue(PROPERTY_STRINGITEMLIST, aAnyList);
        }
    }

    return sal_True;
}

// XPropertiesChangeListener
//------------------------------------------------------------------------------
void OComboBoxModel::_onValueChanged()
{
    // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
    // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
    // our own mutex locked
    // and in this special case do it before calling DBTypeConversion::getValue, as this uses the number formatter
    // which's implementation locks the SM, too :(
    // FS - 72451 - 31.01.00
    MutexRelease aRelease(m_aMutex);
    DBG_ASSERT(m_xColumn.is(), "OComboBoxModel::_onValueChanged : have no column !");
    m_aSaveValue = DBTypeConversion::getValue(m_xColumn,
                                              m_xFormatter,
                                              m_aNullDate,
                                              m_nFormatKey,
                                              m_nKeyType);

        m_xAggregateFastSet->setFastPropertyValue(OComboBoxModel::nTextHandle, makeAny(m_aSaveValue));
}

//------------------------------------------------------------------------------
void OComboBoxModel::_reset()
{
    {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
                m_xAggregateFastSet->setFastPropertyValue(OComboBoxModel::nTextHandle, makeAny(m_aDefaultText));
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::addSQLErrorListener(const Reference<XSQLErrorListener>& _rxListener) throw(RuntimeException)
{
    m_aErrorListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::removeSQLErrorListener(const Reference<XSQLErrorListener>& _rxListener) throw(RuntimeException)
{
    m_aErrorListeners.removeInterface(_rxListener);
}

//------------------------------------------------------------------------------
void OComboBoxModel::onError(SQLException& _rException, const ::rtl::OUString& _rContextDescription)
{
        SQLContext aError = prependContextInfo(_rException, static_cast<XWeak*>(this), _rContextDescription);

    if (m_aErrorListeners.getLength())
    {
                SQLErrorEvent aEvent(static_cast<XWeak*>(this), makeAny(aError));

        ::cppu::OInterfaceIteratorHelper aIter(m_aErrorListeners);
        while (aIter.hasMoreElements())
            static_cast<XSQLErrorListener*>(aIter.next())->errorOccured(aEvent);
    }
}

//========================================================================
//= OComboBoxControl
//========================================================================

//------------------------------------------------------------------
InterfaceRef SAL_CALL OComboBoxControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new OComboBoxControl(_rxFactory));
}

//------------------------------------------------------------------------------
OComboBoxControl::OComboBoxControl(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_COMBOBOX)
{
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OComboBoxControl::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_COMBOBOX;
    return aSupported;
}

//.........................................................................
}
//.........................................................................

