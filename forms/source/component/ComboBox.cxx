/*************************************************************************
 *
 *  $RCSfile: ComboBox.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:04 $
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
#ifndef _UTL_NUMBERS_HXX_
#include <unotools/numbers.hxx>
#endif
#ifndef _UNOTOOLS_DATETIME_HXX_
#include <unotools/datetime.hxx>
#endif
#ifndef _UTL_UNO3_DB_TOOLS_HXX_
#include <unotools/dbtools.hxx>
#endif
#ifndef _UTL_DB_CONVERSION_HXX_
#include <unotools/dbconversion.hxx>
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

//.........................................................................
namespace frm
{

//========================================================================
// class OComboBoxModel
//========================================================================
sal_Int32 OComboBoxModel::nTextHandle = -1;

//------------------------------------------------------------------
InterfaceRef SAL_CALL OComboBoxModel_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory) throw (staruno::RuntimeException)
{
    return (*new OComboBoxModel(_rxFactory));
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Type> OComboBoxModel::_getTypes()
{
    static staruno::Sequence<staruno::Type> aTypes;
    if (!aTypes.getLength())
    {
        // my two base classes
        staruno::Sequence<staruno::Type> aBaseClassTypes = OBoundControlModel::_getTypes();

        staruno::Sequence<staruno::Type> aOwnTypes(1);
        staruno::Type* pOwnTypes = aOwnTypes.getArray();
        pOwnTypes[0] = getCppuType((staruno::Reference<starsdb::XSQLErrorBroadcaster>*)NULL);

        aTypes = concatSequences(aBaseClassTypes, aOwnTypes);
    }
    return aTypes;
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OComboBoxModel::getSupportedServiceNames() throw(staruno::RuntimeException)
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = ::rtl::OUString::createFromAscii("com.sun.star.form.component.DatabaseComboBox");
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_COMBOBOX;
    return aSupported;
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OComboBoxModel::queryAggregation(const staruno::Type& _rType) throw (staruno::RuntimeException)
{
    staruno::Any aReturn;

    aReturn = OBoundControlModel::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<starsdb::XSQLErrorBroadcaster*>(this)
        );

    return aReturn;
}

//------------------------------------------------------------------
OComboBoxModel::OComboBoxModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
                 :OBoundControlModel(_rxFactory, VCL_CONTROLMODEL_COMBOBOX, FRM_CONTROL_COMBOBOX)
                                    // use the old control name for compytibility reasons
                 ,m_eListSourceType(starform::ListSourceType_TABLE)
                 ,m_bEmptyIsNull(sal_True)
                  ,m_aNullDate(STANDARD_DB_DATE)
                 ,m_nKeyType(starutil::NumberFormat::UNDEFINED)
                 ,m_nFormatKey(0)
                 ,m_nFieldType(starsdbc::DataType::OTHER)
                 ,m_aErrorListeners(m_aMutex)
{
    m_nClassId = starform::FormComponentType::COMBOBOX;
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
void OComboBoxModel::getFastPropertyValue(staruno::Any& _rValue, sal_Int32 _nHandle) const
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
void OComboBoxModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const staruno::Any& _rValue)
            throw (staruno::Exception)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_LISTSOURCETYPE :
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(reinterpret_cast<starform::ListSourceType*>(NULL))),
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_eListSourceType;
            break;

        case PROPERTY_ID_LISTSOURCE :
            DBG_ASSERT(_rValue.getValueType().getTypeClass() == staruno::TypeClass_STRING,
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aListSource;
            // die ListSource hat sich geaendert -> neu laden
            if (starform::ListSourceType_VALUELIST != m_eListSourceType)
            {
                if (m_xCursor.is() && !m_xField.is()) // combobox bereits mit Datenbank verbunden ?
                    // neu laden
                    loadData();
            }
            break;

        case PROPERTY_ID_EMPTY_IS_NULL :
            DBG_ASSERT(_rValue.getValueType().getTypeClass() == staruno::TypeClass_BOOLEAN,
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_bEmptyIsNull;
            break;

        case PROPERTY_ID_DEFAULT_TEXT :
            DBG_ASSERT(_rValue.getValueType().getTypeClass() == staruno::TypeClass_STRING,
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
            staruno::Any& _rConvertedValue, staruno::Any& _rOldValue, sal_Int32 _nHandle, const staruno::Any& _rValue)
            throw (starlang::IllegalArgumentException)
{
    sal_Bool bModified(sal_False);
    switch (_nHandle)
    {
        case PROPERTY_ID_LISTSOURCETYPE :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_eListSourceType);
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
staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL OComboBoxModel::getPropertySetInfo() throw(staruno::RuntimeException)
{
    staruno::Reference<starbeans::XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
cppu::IPropertyArrayHelper& OComboBoxModel::getInfoHelper()
{
    return *const_cast<OComboBoxModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OComboBoxModel::fillProperties(
        staruno::Sequence< starbeans::Property >& _rProps,
        staruno::Sequence< starbeans::Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(13)
        // Text auf transient setzen
        ModifyPropertyAttributes(_rAggregateProps, PROPERTY_TEXT, starbeans::PropertyAttribute::TRANSIENT, 0);

        DECL_PROP2(CLASSID,             sal_Int16,                  READONLY, TRANSIENT);
        DECL_PROP1(NAME,                ::rtl::OUString,            BOUND);
        DECL_PROP1(TAG,                 ::rtl::OUString,            BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,                  BOUND);
        DECL_PROP1(LISTSOURCETYPE,      starform::ListSourceType,   BOUND);
        DECL_PROP1(LISTSOURCE,          ::rtl::OUString,            BOUND);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                              BOUND);
        DECL_PROP1(DEFAULT_TEXT,        ::rtl::OUString,            BOUND);
        DECL_PROP1(CONTROLSOURCE,       ::rtl::OUString,            BOUND);
        DECL_PROP1(HELPTEXT,            ::rtl::OUString,            BOUND);
        DECL_IFACE_PROP2(BOUNDFIELD,    starbeans::XPropertySet,    READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,  starbeans::XPropertySet,    BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,   rtl::OUString,  READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OComboBoxModel::getServiceName() throw(staruno::RuntimeException)
{
    return FRM_COMPONENT_COMBOBOX;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream)
    throw(stario::IOException, staruno::RuntimeException)
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
    if (m_aBoundColumn.getValueType().getTypeClass() == staruno::TypeClass_SHORT)
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
void SAL_CALL OComboBoxModel::read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, staruno::RuntimeException)
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
        m_eListSourceType = starform::ListSourceType_TABLE;
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
    m_eListSourceType = (starform::ListSourceType)nValue;

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
        m_xAggregateSet->setPropertyValue(PROPERTY_STRINGITEMLIST, staruno::makeAny(aSequence));
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
    DBG_ASSERT(m_eListSourceType != starform::ListSourceType_VALUELIST, "OComboBoxModel::loadData : do not call for a value list !");
    ////
    // Connection holen
    staruno::Reference<starsdbc::XRowSet> xForm(m_xCursor, staruno::UNO_QUERY);
    if (!xForm.is())
        return;
    staruno::Reference<starsdbc::XConnection> xConnection = getConnection(xForm);
    if (!xConnection.is())
        return;

    // we need a com::sun::star::sdb::Connection for some of the code below ...
    staruno::Reference<starlang::XServiceInfo> xServiceInfo(xConnection, staruno::UNO_QUERY);
    if (!xServiceInfo.is() || !xServiceInfo->supportsService(SRV_SDB_CONNECTION))
    {
        DBG_ERROR("OComboBoxModel::loadData : invalid connection !");
        return;
    }

    staruno::Reference<starsdbc::XResultSet> xListCursor;

    if (!m_aListSource.len() || m_eListSourceType == starform::ListSourceType_VALUELIST)
        return;

    try
    {
        switch (m_eListSourceType)
        {
            case starform::ListSourceType_TABLEFIELDS:
                // don't work with a statement here, the fields will be collected below
                break;
            case starform::ListSourceType_TABLE:
            {
                // does the bound field belong to the table ?
                // if we use an alias for the bound field, we won't find it
                // in that case we use the first field of the table

                staruno::Reference<starcontainer::XNameAccess> xFieldsByName = getTableFields(xConnection, m_aListSource);
                staruno::Reference<starcontainer::XIndexAccess> xFieldsByIndex(xFieldsByName, staruno::UNO_QUERY);

                ::rtl::OUString aFieldName;
                if (xFieldsByName.is() && xFieldsByName->hasByName(m_aControlSource))
                {
                    aFieldName = m_aControlSource;
                }
                else
                {
                    // otherwise look for the alias
                    staruno::Reference<starsdb::XSQLQueryComposerFactory> xFactory(xConnection, staruno::UNO_QUERY);
                    if (!xFactory.is())
                        break;

                    staruno::Reference<starsdb::XSQLQueryComposer> xComposer = xFactory->createQueryComposer();
                    try
                    {
                        staruno::Reference<starbeans::XPropertySet> xFormAsSet(xForm, staruno::UNO_QUERY);
                        ::rtl::OUString aStatement;
                        xFormAsSet->getPropertyValue(PROPERTY_ACTIVECOMMAND) >>= aStatement;
                        xComposer->setQuery(aStatement);
                    }
                    catch(...)
                    {
                        disposeComponent(xComposer);
                        break;
                    }

                    // search the field
                    staruno::Reference<starsdbcx::XColumnsSupplier> xSupplyFields(xComposer, staruno::UNO_QUERY);
                    DBG_ASSERT(xSupplyFields.is(), "OComboBoxModel::loadData : invalid query composer !");

                    staruno::Reference<starcontainer::XNameAccess> xFieldNames = xSupplyFields->getColumns();
                    if (xFieldNames->hasByName(m_aControlSource))
                    {
                        staruno::Reference<starbeans::XPropertySet> xComposerFieldAsSet(*(staruno::Reference<starbeans::XPropertySet>*)xFieldNames->getByName(m_aControlSource).getValue());
                        if (hasProperty(PROPERTY_FIELDSOURCE, xComposerFieldAsSet))
                            xComposerFieldAsSet->getPropertyValue(PROPERTY_FIELDSOURCE) >>= aFieldName;
                    }

                    disposeComponent(xComposer);
                }

                if (!aFieldName.len())
                    break;

                staruno::Reference<starsdbc::XDatabaseMetaData> xMeta = xConnection->getMetaData();
                ::rtl::OUString aQuote = xMeta->getIdentifierQuoteString();
                ::rtl::OUString aStatement = ::rtl::OUString::createFromAscii("SELECT DISTINCT ");

                aStatement += quoteName(aQuote, aFieldName);
                aStatement += ::rtl::OUString::createFromAscii(" FROM ");
                aStatement += quoteTableName(xMeta, m_aListSource);

                staruno::Reference<starsdbc::XStatement> xStmt = xConnection->createStatement();
                xListCursor = xStmt->executeQuery(aStatement);
            }   break;
            case starform::ListSourceType_QUERY:
            {
                staruno::Reference<starsdb::XQueriesSupplier> xSupplyQueries(xConnection, staruno::UNO_QUERY);
                staruno::Reference<starbeans::XPropertySet> xQuery(*(InterfaceRef*)xSupplyQueries->getQueries()->getByName(m_aListSource).getValue(), staruno::UNO_QUERY);
                staruno::Reference<starsdbc::XStatement> xStmt = xConnection->createStatement();
                staruno::Reference<starbeans::XPropertySet>(xStmt, staruno::UNO_QUERY)->setPropertyValue(PROPERTY_ESCAPE_PROCESSING, xQuery->getPropertyValue(PROPERTY_ESCAPE_PROCESSING));

                ::rtl::OUString sStatement;
                xQuery->getPropertyValue(PROPERTY_COMMAND) >>= sStatement;
                xListCursor = xStmt->executeQuery(sStatement);
            }   break;
            default:
            {
                staruno::Reference<starsdbc::XStatement> xStmt = xConnection->createStatement();
                if (starform::ListSourceType_SQLPASSTHROUGH == m_eListSourceType)
                {
                    staruno::Reference<starbeans::XPropertySet> xStatementProps(xStmt, staruno::UNO_QUERY);
                    xStatementProps->setPropertyValue(PROPERTY_ESCAPE_PROCESSING, staruno::makeAny(sal_Bool(sal_False)));
                }
                xListCursor = xStmt->executeQuery(m_aListSource);
            }
        }
    }
    catch(starsdbc::SQLException& eSQL)
    {
        onError(eSQL, FRM_RES_STRING(RID_BASELISTBOX_ERROR_FILLLIST));
        disposeComponent(xListCursor);
        return;
    }
    catch(staruno::Exception& eUnknown)
    {
        eUnknown;
        disposeComponent(xListCursor);
        return;
    }

    if (starform::ListSourceType_TABLEFIELDS != m_eListSourceType && !xListCursor.is())
        // something went wrong ...
        return;

    vector< ::rtl::OUString >   aStringList;
    aStringList.reserve(16);
    try
    {
        switch (m_eListSourceType)
        {
            case starform::ListSourceType_SQL:
            case starform::ListSourceType_SQLPASSTHROUGH:
            case starform::ListSourceType_TABLE:
            case starform::ListSourceType_QUERY:
            {
                // die XDatabaseVAriant der ersten Spalte
                staruno::Reference<starsdbcx::XColumnsSupplier> xSupplyCols(xListCursor, staruno::UNO_QUERY);
                DBG_ASSERT(xSupplyCols.is(), "OComboBoxModel::loadData : cursor supports the row set service but is no column supplier ??!");
                staruno::Reference<starcontainer::XIndexAccess> xColumns;
                if (xSupplyCols.is())
                {
                    xColumns = staruno::Reference<starcontainer::XIndexAccess>(xSupplyCols->getColumns(), staruno::UNO_QUERY);
                    DBG_ASSERT(xColumns.is(), "OComboBoxModel::loadData : no columns supplied by the row set !");
                }
                staruno::Reference<starsdb::XColumn> xDataField;
                if (xColumns.is())
                    xDataField = staruno::Reference<starsdb::XColumn>(*(InterfaceRef*)xColumns->getByIndex(0).getValue(), staruno::UNO_QUERY);
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
            case starform::ListSourceType_TABLEFIELDS:
            {
                staruno::Reference<starcontainer::XNameAccess> xFieldNames = getTableFields(xConnection, m_aListSource);
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
    catch(starsdbc::SQLException& eSQL)
    {
        onError(eSQL, FRM_RES_STRING(RID_BASELISTBOX_ERROR_FILLLIST));
        disposeComponent(xListCursor);
        return;
    }
    catch(staruno::Exception& eUnknown)
    {
        eUnknown;
        disposeComponent(xListCursor);
        return;
    }

    // String-staruno::Sequence fuer ListBox erzeugen
    StringSequence aStringSeq(aStringList.size());
    ::rtl::OUString* pStringAry = aStringSeq.getArray();
    for (sal_Int32 i = 0; i<aStringSeq.getLength(); ++i)
        pStringAry[i] = aStringList[i];

    // String-staruno::Sequence an ListBox setzen
    m_xAggregateSet->setPropertyValue(PROPERTY_STRINGITEMLIST, staruno::makeAny(aStringSeq));

    // Statement + Cursor zerstoeren
    disposeComponent(xListCursor);
}

//------------------------------------------------------------------------------
void OComboBoxModel::_loaded(const starlang::EventObject& rEvent)
{
    if (m_xField.is())
    {
        // jetzt den Key und typ ermitteln
        m_xField->getPropertyValue(PROPERTY_FIELDTYPE) >>= m_nFieldType;
        m_xField->getPropertyValue(PROPERTY_FORMATKEY) >>= m_nFormatKey;

        // XNumberFormatter besorgen
        staruno::Reference<starsdbc::XRowSet> xRowSet(rEvent.Source, staruno::UNO_QUERY);
        DBG_ASSERT(xRowSet.is(), "OComboBoxModel::_loaded : invalid event source !");
        staruno::Reference<starutil::XNumberFormatsSupplier> xSupplier = getNumberFormats(getConnection(xRowSet), sal_False, m_xServiceFactory);
        if (xSupplier.is())
        {
            m_xFormatter =  staruno::Reference<starutil::XNumberFormatter>(
                                m_xServiceFactory->createInstance(FRM_NUMBER_FORMATTER), staruno::UNO_QUERY
                                );
            if (m_xFormatter.is())
                m_xFormatter->attachNumberFormatsSupplier(xSupplier);

            m_nKeyType  = getNumberFormatType(xSupplier->getNumberFormats(), m_nFormatKey);
            typeConvert(*(starutil::Date*)xSupplier->getNumberFormatSettings()->getPropertyValue(::rtl::OUString::createFromAscii("NullDate")).getValue(),
                m_aNullDate);
        }
    }

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
        m_nFieldType = starsdbc::DataType::OTHER;
        m_nFormatKey = 0;
        m_nKeyType   = starutil::NumberFormat::UNDEFINED;
        m_aNullDate  = STANDARD_DB_DATE;
    }

    // Zuruecksetzen der Inhalte (s.o)
    if (m_aListSource.len() && m_xCursor.is())
    {
        StringSequence aSequence;
        m_xAggregateSet->setPropertyValue(PROPERTY_STRINGITEMLIST, staruno::makeAny(aSequence));
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::reloaded( const starlang::EventObject& aEvent ) throw(staruno::RuntimeException)
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
    if (aNewValue != m_aSaveValue)
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
            catch(...)
            {
                return sal_False;
            }
        }
        m_aSaveValue = aNewValue;
    }

    // add the new value to the list
    sal_Bool bAddToList = sal_True;
    // (only if this is not the "commit" triggered by a "reset")
    if  (m_bResetting)
        bAddToList = sal_False;

    staruno::Any aAnyList = m_xAggregateSet->getPropertyValue(PROPERTY_STRINGITEMLIST);
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

    m_xAggregateFastSet->setFastPropertyValue(OComboBoxModel::nTextHandle, staruno::makeAny(m_aSaveValue));
}

//------------------------------------------------------------------------------
void OComboBoxModel::_reset()
{
    {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(OComboBoxModel::nTextHandle, staruno::makeAny(m_aDefaultText));
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::addSQLErrorListener(const staruno::Reference<starsdb::XSQLErrorListener>& _rxListener) throw(staruno::RuntimeException)
{
    m_aErrorListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::removeSQLErrorListener(const staruno::Reference<starsdb::XSQLErrorListener>& _rxListener) throw(staruno::RuntimeException)
{
    m_aErrorListeners.removeInterface(_rxListener);
}

//------------------------------------------------------------------------------
void OComboBoxModel::onError(starsdbc::SQLException& _rException, const ::rtl::OUString& _rContextDescription)
{
    starsdb::SQLContext aError = prependContextInfo(_rException, static_cast<staruno::XWeak*>(this), _rContextDescription);

    if (m_aErrorListeners.getLength())
    {
        starsdb::SQLErrorEvent aEvent(static_cast<staruno::XWeak*>(this), staruno::makeAny(aError));

        ::cppu::OInterfaceIteratorHelper aIter(m_aErrorListeners);
        while (aIter.hasMoreElements())
            static_cast<starsdb::XSQLErrorListener*>(aIter.next())->errorOccured(aEvent);
    }
}

//========================================================================
//= OComboBoxControl
//========================================================================

//------------------------------------------------------------------
InterfaceRef SAL_CALL OComboBoxControl_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory) throw (staruno::RuntimeException)
{
    return *(new OComboBoxControl(_rxFactory));
}

//------------------------------------------------------------------------------
OComboBoxControl::OComboBoxControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_COMBOBOX)
{
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OComboBoxControl::getSupportedServiceNames() throw(staruno::RuntimeException)
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

