/*************************************************************************
 *
 *  $RCSfile: ListBox.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-19 13:31:29 $
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

#ifndef _FORMS_LISTBOX_HXX_
#include "ListBox.hxx"
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
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif
#ifndef _COMPHELPER_DATETIME_HXX_
#include <comphelper/datetime.hxx>
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
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
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
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

//==================================================================
//= OListBoxModel
//==================================================================
sal_Int32 OListBoxModel::nSelectHandle = -1;
//------------------------------------------------------------------
InterfaceRef SAL_CALL OListBoxModel_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory) throw (staruno::RuntimeException)
{
    return *(new OListBoxModel(_rxFactory));
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Type> OListBoxModel::_getTypes()
{
    static staruno::Sequence<staruno::Type> aTypes;
    if (!aTypes.getLength())
    {
        // my two base classes
        staruno::Sequence<staruno::Type> aBaseClassTypes = OBoundControlModel::_getTypes();

        staruno::Sequence<staruno::Type> aOwnTypes(2);
        staruno::Type* pOwnTypes = aOwnTypes.getArray();
        pOwnTypes[0] = getCppuType((staruno::Reference<starsdb::XSQLErrorBroadcaster>*)NULL);
        pOwnTypes[1] = getCppuType((staruno::Reference<starutil::XRefreshable>*)NULL);

        aTypes = concatSequences(aBaseClassTypes, aOwnTypes);
    }
    return aTypes;
}


DBG_NAME(OListBoxModel);
//------------------------------------------------------------------
OListBoxModel::OListBoxModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
    :OBoundControlModel(_rxFactory, VCL_CONTROLMODEL_LISTBOX, FRM_CONTROL_LISTBOX)
                                    // use the old control name for compytibility reasons
    ,m_aRefreshListeners(m_aMutex)
    ,m_bBoundComponent(sal_False)
    ,m_nNULLPos(-1)
    ,m_aErrorListeners(m_aMutex)
{
    DBG_CTOR(OListBoxModel,NULL);

    m_nClassId = starform::FormComponentType::LISTBOX;
    m_eListSourceType = starform::ListSourceType_VALUELIST;
    m_aBoundColumn <<= (sal_Int16)1;
    m_sDataFieldConnectivityProperty = PROPERTY_SELECT_SEQ;
    if (OListBoxModel::nSelectHandle == -1)
        OListBoxModel::nSelectHandle = getOriginalHandle(PROPERTY_ID_SELECT_SEQ);
}

//------------------------------------------------------------------
OListBoxModel::~OListBoxModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR(OListBoxModel,NULL);
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OListBoxModel::getSupportedServiceNames() throw(staruno::RuntimeException)
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = ::rtl::OUString::createFromAscii("com.sun.star.form.component.DatabaseListBox");
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_LISTBOX;
    return aSupported;
}

//------------------------------------------------------------------------------
staruno::Any SAL_CALL OListBoxModel::queryAggregation(const staruno::Type& _rType) throw (staruno::RuntimeException)
{
    staruno::Any aReturn;

    aReturn = OBoundControlModel::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<starutil::XRefreshable*>(this)
            ,static_cast<starsdb::XSQLErrorBroadcaster*>(this)
        );

    return aReturn;
}

// OComponentHelper
//------------------------------------------------------------------------------
void OListBoxModel::disposing()
{
    starlang::EventObject aEvt(static_cast<staruno::XWeak*>(this));
    m_aRefreshListeners.disposeAndClear(aEvt);
    OBoundControlModel::disposing();
}

// XRefreshable
//------------------------------------------------------------------------------
void SAL_CALL OListBoxModel::addRefreshListener(const staruno::Reference<starutil::XRefreshListener>& _rxListener) throw(staruno::RuntimeException)
{
    m_aRefreshListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OListBoxModel::removeRefreshListener(const staruno::Reference<starutil::XRefreshListener>& _rxListener) throw(staruno::RuntimeException)
{
    m_aRefreshListeners.removeInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OListBoxModel::refresh() throw(staruno::RuntimeException)
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if (m_eListSourceType != starform::ListSourceType_VALUELIST)
        {
            if (m_xField.is())
                m_aValueSeq = StringSequence();

            if (m_xCursor.is())
                loadData();
        }
    }

    starlang::EventObject aEvt(static_cast<staruno::XWeak*>(this));
    NOTIFY_LISTENERS(m_aRefreshListeners, starutil::XRefreshListener, refreshed, aEvt);
}

//------------------------------------------------------------------------------
void OListBoxModel::getFastPropertyValue(staruno::Any& _rValue, sal_Int32 _nHandle) const
{
    switch (_nHandle)
    {
        case PROPERTY_ID_BOUNDCOLUMN:
            _rValue <<= m_aBoundColumn;
            break;

        case PROPERTY_ID_LISTSOURCETYPE:
            _rValue <<= m_eListSourceType;
            break;

        case PROPERTY_ID_LISTSOURCE:
            _rValue <<= m_aListSourceSeq;
            break;

        case PROPERTY_ID_VALUE_SEQ:
            _rValue <<= m_aValueSeq;
            break;

        case PROPERTY_ID_DEFAULT_SELECT_SEQ:
            _rValue <<= m_aDefaultSelectSeq;
            break;

        default:
            OBoundControlModel::getFastPropertyValue(_rValue, _nHandle);
    }
}

//------------------------------------------------------------------------------
void OListBoxModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const staruno::Any& _rValue) throw (com::sun::star::uno::Exception)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_BOUNDCOLUMN :
            DBG_ASSERT((_rValue.getValueType().getTypeClass() == staruno::TypeClass_SHORT) || (_rValue.getValueType().getTypeClass() == staruno::TypeClass_VOID),
                "OListBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            m_aBoundColumn = _rValue;
            break;

        case PROPERTY_ID_LISTSOURCETYPE :
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(reinterpret_cast<starform::ListSourceType*>(NULL))),
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_eListSourceType;
            break;

        case PROPERTY_ID_LISTSOURCE :
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(reinterpret_cast<StringSequence*>(NULL))),
                "OListBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aListSourceSeq;

            if (m_eListSourceType == starform::ListSourceType_VALUELIST)
                m_aValueSeq = m_aListSourceSeq;
            else if (m_xCursor.is() && !m_xField.is()) // Listbox bereits mit Datenbank verbunden
                // Aenderung der Datenquelle -> neu laden
                loadData();

            break;

        case PROPERTY_ID_VALUE_SEQ :
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(reinterpret_cast<StringSequence*>(NULL))),
                "OListBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aValueSeq;
            break;

        case PROPERTY_ID_DEFAULT_SELECT_SEQ :
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(reinterpret_cast<staruno::Sequence<sal_Int16>*>(NULL))),
                "OListBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aDefaultSelectSeq;

            DBG_ASSERT(m_xAggregateFastSet.is(), "OListBoxModel::setFastPropertyValue_NoBroadcast(DEFAULT_SELECT_SEQ) : invalid aggregate !");
            if (m_xAggregateFastSet.is())
            {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
                // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
                // our own mutex locked
                // FS - 72451 - 31.01.00
                MutexRelease aRelease(m_aMutex);
                m_xAggregateFastSet->setFastPropertyValue(OListBoxModel::nSelectHandle, _rValue);
            }
            break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    }

    if (PROPERTY_ID_STRINGITEMLIST == _nHandle)
        _reset();
}

//------------------------------------------------------------------------------
sal_Bool OListBoxModel::convertFastPropertyValue(
            staruno::Any& _rConvertedValue, staruno::Any& _rOldValue, sal_Int32 _nHandle, const staruno::Any& _rValue)
            throw (starlang::IllegalArgumentException)
{
    sal_Bool bModified(sal_False);
    switch (_nHandle)
    {
        case PROPERTY_ID_BOUNDCOLUMN :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aBoundColumn, ::getCppuType(reinterpret_cast<sal_Int16*>(NULL)));
            break;

        case PROPERTY_ID_LISTSOURCETYPE:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_eListSourceType);
            break;

        case PROPERTY_ID_LISTSOURCE:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aListSourceSeq);
            break;

        case PROPERTY_ID_VALUE_SEQ :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aValueSeq);
            break;

        case PROPERTY_ID_DEFAULT_SELECT_SEQ :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aDefaultSelectSeq);
            break;

        default:
            return OBoundControlModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
    }
    return bModified;
}

//------------------------------------------------------------------------------
staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL OListBoxModel::getPropertySetInfo() throw(staruno::RuntimeException)
{
    staruno::Reference<starbeans::XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
cppu::IPropertyArrayHelper& OListBoxModel::getInfoHelper()
{
    return *const_cast<OListBoxModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OListBoxModel::fillProperties(
        staruno::Sequence< starbeans::Property >& _rProps,
        staruno::Sequence< starbeans::Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(14)
        // die SelectSequence-Property soll transient sein ....
        ModifyPropertyAttributes(_rAggregateProps, PROPERTY_SELECT_SEQ, starbeans::PropertyAttribute::TRANSIENT, 0);

        DECL_PROP2(CLASSID,             sal_Int16,                      READONLY, TRANSIENT);
        DECL_PROP1(NAME,                ::rtl::OUString,                BOUND);
        DECL_PROP1(TAG,                 ::rtl::OUString,                BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,                      BOUND);
        DECL_PROP2(BOUNDCOLUMN,         sal_Int16,                      BOUND, MAYBEVOID);
        DECL_PROP1(LISTSOURCETYPE,      starform::ListSourceType,       BOUND);
        DECL_PROP1(LISTSOURCE,          StringSequence,                 BOUND);
        DECL_PROP3(VALUE_SEQ,           StringSequence,                 BOUND, READONLY, TRANSIENT);
        DECL_PROP1(DEFAULT_SELECT_SEQ,  staruno::Sequence<sal_Int16>,   BOUND);
        DECL_PROP1(CONTROLSOURCE,       ::rtl::OUString,                BOUND);
        DECL_PROP1(HELPTEXT,            ::rtl::OUString,                BOUND);
        DECL_IFACE_PROP2(BOUNDFIELD,    starbeans::XPropertySet,        READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,  starbeans::XPropertySet,        BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,   rtl::OUString,  READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OListBoxModel::getServiceName() throw(staruno::RuntimeException)
{
    return FRM_COMPONENT_LISTBOX;   // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL OListBoxModel::write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream)
    throw(stario::IOException, staruno::RuntimeException)
{
    OBoundControlModel::write(_rxOutStream);

    // Dummy-Seq, um Kompatible zu bleiben, wenn SelectSeq nicht mehr gespeichert wird
    staruno::Sequence<sal_Int16> aDummySeq;

    // Version
    // Version 0x0002: ListSource wird StringSeq
    _rxOutStream->writeShort(0x0004);

    // Maskierung fuer any
    sal_uInt16 nAnyMask = 0;
    if (m_aBoundColumn.getValueType().getTypeClass() != staruno::TypeClass_VOID)
        nAnyMask |= BOUNDCOLUMN;

    _rxOutStream << nAnyMask;

    _rxOutStream << m_aListSourceSeq;
    _rxOutStream << (sal_Int16)m_eListSourceType;
    _rxOutStream << aDummySeq;
    _rxOutStream << m_aDefaultSelectSeq;

    if ((nAnyMask & BOUNDCOLUMN) == BOUNDCOLUMN)
    {
        sal_Int16 nBoundColumn;
        m_aBoundColumn >>= nBoundColumn;
        _rxOutStream << nBoundColumn;
    }
    _rxOutStream << m_aHelpText;

    // from version 0x0004 : common properties
    writeCommonProperties(_rxOutStream);
}

//------------------------------------------------------------------------------
void SAL_CALL OListBoxModel::read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, staruno::RuntimeException)
{
    // Bei manchen Variblen muessen Abhaengigkeiten beruecksichtigt werden.
    // Deshalb muessen sie explizit ueber setPropertyValue() gesetzt werden.

    OBoundControlModel::read(_rxInStream);
    ::osl::MutexGuard aGuard(m_aMutex);

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();
    DBG_ASSERT(nVersion > 0, "OListBoxModel::read : version 0 ? this should never have been written !");

    if (nVersion > 0x0004)
    {
        DBG_ERROR("OListBoxModel::read : invalid (means unknown) version !");
        m_aListSourceSeq.realloc(0);
        m_aBoundColumn <<= (sal_Int16)0;
        m_aValueSeq.realloc(0);
        m_eListSourceType = starform::ListSourceType_VALUELIST;
        m_aDefaultSelectSeq.realloc(0);
        defaultCommonProperties();
        return;
    }

    // Maskierung fuer any
    sal_uInt16 nAnyMask;
    _rxInStream >> nAnyMask;

    // ListSourceSeq
    StringSequence aListSourceSeq;
    if (nVersion == 0x0001)
    {
        // ListSourceSeq aus String zusammenstellen;
        ::rtl::OUString sListSource;
        _rxInStream >> sListSource;

        aListSourceSeq.realloc( sListSource.getTokenCount() );
        for (sal_uInt16 i=0; i<sListSource.getTokenCount(); ++i)
            aListSourceSeq.getArray()[i] = sListSource.getToken(i);
    }
    else
        _rxInStream >> aListSourceSeq;

    sal_Int16 nListSourceType;
    _rxInStream >> nListSourceType;
    m_eListSourceType = (starform::ListSourceType)nListSourceType;
    staruno::Any aListSourceSeqAny;
    aListSourceSeqAny <<= aListSourceSeq;

    setFastPropertyValue(PROPERTY_ID_LISTSOURCE, aListSourceSeqAny );

    // Dummy-Seq, um Kompatible zu bleiben, wenn SelectSeq nicht mehr gespeichert wird
    staruno::Sequence<sal_Int16> aDummySeq;
    _rxInStream >> aDummySeq;

    // DefaultSelectSeq
    staruno::Sequence<sal_Int16> aDefaultSelectSeq;
    _rxInStream >> aDefaultSelectSeq;
    staruno::Any aDefaultSelectSeqAny;
    aDefaultSelectSeqAny <<= aDefaultSelectSeq;
    setFastPropertyValue(PROPERTY_ID_DEFAULT_SELECT_SEQ, aDefaultSelectSeqAny);

    // BoundColumn
    if ((nAnyMask & BOUNDCOLUMN) == BOUNDCOLUMN)
    {
        sal_Int16 nValue;
        _rxInStream >> nValue;
        m_aBoundColumn <<= nValue;
    }

    if (nVersion > 2)
        _rxInStream >> m_aHelpText;

    // Stringliste muß gelehrt werden, wenn nicht ueber WerteListe gefuellt wird
    // dieses kann der Fall sein wenn im alive modus gespeichert wird
    if (m_eListSourceType != starform::ListSourceType_VALUELIST && m_xAggregateSet.is())
    {
        StringSequence aSequence;
        setFastPropertyValue(PROPERTY_ID_STRINGITEMLIST, staruno::makeAny(aSequence));
    }

    if (nVersion > 3)
        readCommonProperties(_rxInStream);

    // Nach dem Lesen die Defaultwerte anzeigen
    if (m_aControlSource.getLength())
        // (not if we don't have a control source - the "State" property acts like it is persistent, then
        _reset();
}

//------------------------------------------------------------------------------
void OListBoxModel::loadData()
{
    DBG_ASSERT(m_eListSourceType != starform::ListSourceType_VALUELIST, "fuer Werteliste kein Laden aus der Datenbank")

    m_nNULLPos = -1;
    m_bBoundComponent = sal_False;

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
        DBG_ERROR("OListBoxModel::loadData : invalid connection !");
        return;
    }

    staruno::Reference<starsdbc::XResultSet> xListCursor;

    // Wenn der ListSourceType keine Werteliste ist,
    // muss die String-Seq zu einem String zusammengefasst werden
    ::rtl::OUString sListSource;
    const ::rtl::OUString* pustrListSouceStrings = m_aListSourceSeq.getConstArray();
    sal_Int32 i;
    for (i=0; i<m_aListSourceSeq.getLength(); ++i)
        sListSource = sListSource + pustrListSouceStrings[i];
    if (!sListSource.len())
        return;

    sal_Int16 nBoundColumn = 0;
    if (m_aBoundColumn.getValueType().getTypeClass() == staruno::TypeClass_SHORT)
        m_aBoundColumn >>= nBoundColumn;

    try
    {
        switch (m_eListSourceType)
        {
            case starform::ListSourceType_TABLEFIELDS:
                // don't work with a statement here, the fields will be collected below
                break;
            case starform::ListSourceType_TABLE:
            {
                staruno::Reference<starcontainer::XNameAccess> xFieldsByName = getTableFields(xConnection, sListSource);
                staruno::Reference<starcontainer::XIndexAccess> xFieldsByIndex(xFieldsByName, staruno::UNO_QUERY);

                // do we have a bound column if yes we have to select it
                // and the displayed column is the first column othwhise we act as a combobox
                ::rtl::OUString aFieldName;
                ::rtl::OUString aBoundFieldName;

                if ((nBoundColumn > 0) && xFieldsByIndex.is())
                {
                    if (xFieldsByIndex->getCount() <= nBoundColumn)
                        break;

                    staruno::Reference<starbeans::XPropertySet> xFieldAsSet;
                    xFieldsByIndex->getByIndex(nBoundColumn) >>= xFieldAsSet;
                    xFieldAsSet->getPropertyValue(PROPERTY_NAME) >>= aBoundFieldName;
                    nBoundColumn = 1;

                    xFieldsByIndex->getByIndex(0) >>= xFieldAsSet;
                    xFieldAsSet->getPropertyValue(PROPERTY_NAME) >>= aFieldName;
                }
                else if (xFieldsByName.is())
                {
                    if (xFieldsByName->hasByName(m_aControlSource))
                        aFieldName = m_aControlSource;
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
                        DBG_ASSERT(xSupplyFields.is(), "OListBoxModel::loadData : invalid query composer !");

                        staruno::Reference<starcontainer::XNameAccess> xFieldNames = xSupplyFields->getColumns();
                        if (xFieldNames->hasByName(m_aControlSource))
                        {
                            staruno::Reference<starbeans::XPropertySet> xComposerFieldAsSet;
                            xFieldNames->getByName(m_aControlSource) >>= xComposerFieldAsSet;
                            if (hasProperty(PROPERTY_FIELDSOURCE, xComposerFieldAsSet))
                                xComposerFieldAsSet->getPropertyValue(PROPERTY_FIELDSOURCE) >>= aFieldName;
                        }
                        disposeComponent(xComposer);
                    }
                }
                if (!aFieldName.len())
                    break;

                staruno::Reference<starsdbc::XDatabaseMetaData> xMeta = xConnection->getMetaData();
                ::rtl::OUString aQuote = xMeta->getIdentifierQuoteString();
                ::rtl::OUString aStatement = ::rtl::OUString::createFromAscii("SELECT ");
                if (!aBoundFieldName.getLength())   // act like a combobox
                    aStatement += ::rtl::OUString::createFromAscii("DISTINCT ");

                aStatement += quoteName(aQuote,aFieldName);
                if (aBoundFieldName.len())
                {
                    aStatement += ::rtl::OUString::createFromAscii(", ");
                    aStatement += quoteName(aQuote, aBoundFieldName);
                }
                aStatement += ::rtl::OUString::createFromAscii(" FROM ");
                aStatement += quoteTableName(xMeta, sListSource);

                staruno::Reference<starsdbc::XStatement> xStmt = xConnection->createStatement();
                xListCursor = xStmt->executeQuery(aStatement);
            }   break;
            case starform::ListSourceType_QUERY:
            {
                staruno::Reference<starsdb::XQueriesSupplier> xSupplyQueries(xConnection, staruno::UNO_QUERY);
                staruno::Reference<starbeans::XPropertySet> xQuery(*(InterfaceRef*)xSupplyQueries->getQueries()->getByName(sListSource).getValue(), staruno::UNO_QUERY);
                staruno::Reference<starsdbc::XStatement> xStmt = xConnection->createStatement();
                staruno::Reference<starbeans::XPropertySet>(xStmt, staruno::UNO_QUERY)->setPropertyValue(PROPERTY_ESCAPE_PROCESSING, xQuery->getPropertyValue(PROPERTY_ESCAPE_PROCESSING));

                ::rtl::OUString sCommand;
                xQuery->getPropertyValue(PROPERTY_COMMAND) >>= sCommand;
                xListCursor = xStmt->executeQuery(sCommand);
            }   break;
            default:
            {
                staruno::Reference<starsdbc::XStatement> xStmt = xConnection->createStatement();
                if (starform::ListSourceType_SQLPASSTHROUGH == m_eListSourceType)
                {
                    staruno::Reference<starbeans::XPropertySet> xStatementProps(xStmt, staruno::UNO_QUERY);
                    xStatementProps->setPropertyValue(PROPERTY_ESCAPE_PROCESSING, staruno::makeAny(sal_Bool(sal_False)));
                }
                xListCursor = xStmt->executeQuery(sListSource);
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

    // Anzeige- und Werteliste fuellen
    vector< ::rtl::OUString >   aValueList, aStringList;
    aValueList.reserve(16);
    aStringList.reserve(16);
    sal_Bool bUseNULL = m_xField.is() && !m_bRequired;
    try
    {
        switch (m_eListSourceType)
        {
            case starform::ListSourceType_SQL:
            case starform::ListSourceType_SQLPASSTHROUGH:
            case starform::ListSourceType_TABLE:
            case starform::ListSourceType_QUERY:
            {
                // Feld der 1. Column des ResultSets holen
                staruno::Reference<starsdbcx::XColumnsSupplier> xSupplyCols(xListCursor, staruno::UNO_QUERY);
                DBG_ASSERT(xSupplyCols.is(), "OListBoxModel::loadData : cursor supports the row set service but is no column supplier ??!");
                staruno::Reference<starcontainer::XIndexAccess> xColumns;
                if (xSupplyCols.is())
                {
                    xColumns = staruno::Reference<starcontainer::XIndexAccess>(xSupplyCols->getColumns(), staruno::UNO_QUERY);
                    DBG_ASSERT(xColumns.is(), "OListBoxModel::loadData : no columns supplied by the row set !");
                }
                staruno::Reference<starsdb::XColumn> xDataField;
                if (xColumns.is())
                    xDataField = staruno::Reference<starsdb::XColumn>(*(InterfaceRef*)xColumns->getByIndex(0).getValue(), staruno::UNO_QUERY);
                if (!xDataField.is())
                {
                    disposeComponent(xListCursor);
                    return;
                }

                starutil::Date aNullDate(DBTypeConversion::STANDARD_DB_DATE);
                sal_Int32 nFormatKey = 0;
                sal_Int32 nFieldType = starsdbc::DataType::OTHER;
                sal_Int16 nKeyType   = starutil::NumberFormat::UNDEFINED;
                try
                {
                    staruno::Reference<starbeans::XPropertySet> xFieldAsSet(xDataField, staruno::UNO_QUERY);
                    xFieldAsSet->getPropertyValue(PROPERTY_FIELDTYPE) >>= nFieldType;
                    xFieldAsSet->getPropertyValue(PROPERTY_FORMATKEY) >>= nFormatKey;
                }
                catch(...)
                {
                }

                staruno::Reference<starutil::XNumberFormatter> xFormatter;
                staruno::Reference<starutil::XNumberFormatsSupplier> xSupplier = getNumberFormats(xConnection, sal_False, m_xServiceFactory);
                if (xSupplier.is())
                {
                    xFormatter = staruno::Reference<starutil::XNumberFormatter>(
                                    m_xServiceFactory->createInstance(FRM_NUMBER_FORMATTER),
                                    staruno::UNO_QUERY
                                );
                    if (xFormatter.is())
                    {
                        xFormatter->attachNumberFormatsSupplier(xSupplier);
                        xFormatter->getNumberFormatsSupplier()->getNumberFormatSettings()->getPropertyValue(
                            ::rtl::OUString::createFromAscii("NullDate")) >>= aNullDate;
                        nKeyType = getNumberFormatType(xFormatter->getNumberFormatsSupplier()->getNumberFormats(), nFormatKey);
                    }
                }

                // Feld der BoundColumn des ResultSets holen
                staruno::Reference<starsdb::XColumn> xBoundField;
                if ((nBoundColumn > 0) && m_xColumn.is())
                    // don't look for a bound column if we're not connected to a field
                    xBoundField = staruno::Reference<starsdb::XColumn>(*(InterfaceRef*)xColumns->getByIndex(nBoundColumn).getValue(), staruno::UNO_QUERY);
                m_bBoundComponent = xBoundField.is();

                //  Ist die LB an ein Feld gebunden und sind Leereinträge zulaessig
                //  dann wird die Position fuer einen Leereintrag gemerkt

                ::rtl::OUString aStr;
                sal_Int16 i = 0;
                // per definitionem the list cursor is positioned _before_ the first row at the moment
                while (xListCursor->next() && (i++<SHRT_MAX)) // max anzahl eintraege
                {
                    aStr = DBTypeConversion::getValue(xDataField,
                                            xFormatter,
                                            aNullDate,
                                            nFormatKey,
                                            nKeyType);

                    aStringList.push_back(aStr);

                    if (m_bBoundComponent)
                    {
                        aStr = xBoundField->getString();
                        aValueList.push_back(aStr);
                    }

                    if (bUseNULL && (m_nNULLPos == -1) && !aStr.len())
                        m_nNULLPos = (sal_Int16)aStringList.size() - 1;
                }
            }
            break;

            case starform::ListSourceType_TABLEFIELDS:
            {
                staruno::Reference<starcontainer::XNameAccess> xFieldNames = getTableFields(xConnection, sListSource);
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


    // Value-staruno::Sequence erzeugen
    // NULL eintrag hinzufuegen
    if (bUseNULL && m_nNULLPos == -1)
    {
        if (m_bBoundComponent)
            aValueList.insert(aValueList.begin());

        aStringList.insert(aStringList.begin());
        m_nNULLPos = 0;
    }

    m_aValueSeq.realloc(aValueList.size());
    ::rtl::OUString* pustrValues = m_aValueSeq.getArray();
    for (i = 0; i < aValueList.size(); ++i)
        pustrValues[i] = aValueList[i];

    // String-staruno::Sequence fuer ListBox erzeugen
    StringSequence aStringSeq(aStringList.size());
    ::rtl::OUString* pustrStrings = aStringSeq.getArray();
    for (i = 0; i < aStringList.size(); ++i)
        pustrStrings[i] = aStringList[i];

    setFastPropertyValue(PROPERTY_ID_STRINGITEMLIST, staruno::makeAny(aStringSeq));


    // Statement + Cursor zerstoeren
    disposeComponent(xListCursor);
}

//------------------------------------------------------------------------------
void OListBoxModel::_loaded(const starlang::EventObject& rEvent)
{
    // an Felder gebundene Listboxen haben keine Multiselektion
    if (m_xField.is())
    {
        setFastPropertyValue(PROPERTY_ID_MULTISELECTION, staruno::makeAny(sal_Bool(sal_False)));
    }

    if (m_eListSourceType != starform::ListSourceType_VALUELIST)
    {
        if (m_xField.is())
            m_aValueSeq = StringSequence();

        if (m_xCursor.is())
            loadData();
    }
}

//------------------------------------------------------------------------------
void OListBoxModel::_unloaded()
{
    if (m_eListSourceType != starform::ListSourceType_VALUELIST)
    {
        m_aValueSeq = StringSequence();
        m_nNULLPos = -1;
        m_bBoundComponent = sal_False;

        StringSequence aSequence;
        setFastPropertyValue(PROPERTY_ID_STRINGITEMLIST, staruno::makeAny(aSequence));
    }
}

//------------------------------------------------------------------------------
StringSequence OListBoxModel::GetCurValueSeq() const
{
    StringSequence aCurValues;

    // Aus den selektierten Indizes Werte-staruno::Sequence aufbauen
    DBG_ASSERT(m_xAggregateFastSet.is(), "OListBoxModel::GetCurValueSeq : invalid aggregate !");
    if (!m_xAggregateFastSet.is())
        return aCurValues;
    staruno::Any aTmp = m_xAggregateFastSet->getFastPropertyValue(OListBoxModel::nSelectHandle);

    staruno::Sequence<sal_Int16> aSelectSeq = *(staruno::Sequence<sal_Int16>*)aTmp.getValue();
    const sal_Int16 *pSels = aSelectSeq.getConstArray();
    sal_uInt32 nSelCount = aSelectSeq.getLength();

    if (nSelCount)
    {
        const ::rtl::OUString *pVals    = NULL;
        sal_uInt16 nValCnt          = 0;
        if (m_aValueSeq.getLength())
        {
            pVals = m_aValueSeq.getConstArray();
            nValCnt = m_aValueSeq.getLength();
        }
        else
        {
            aTmp    = const_cast<OListBoxModel*>(this)->OPropertySetAggregationHelper::getFastPropertyValue(PROPERTY_ID_STRINGITEMLIST);
            pVals   = (*(StringSequence*)aTmp.getValue()).getConstArray();
            nValCnt = (*(StringSequence*)aTmp.getValue()).getLength();
        }

        if (nSelCount > 1)
        {
            // Einfach- oder Mehrfach-Selektion
            sal_Bool bMultiSel;
            const_cast<OListBoxModel*>(this)->OPropertySetAggregationHelper::getFastPropertyValue(PROPERTY_ID_MULTISELECTION) >>= bMultiSel;
            if (bMultiSel)
                nSelCount = 1;
        }

        // ist der Eintrag fuer NULL selektiert ?
        // dann leere Selektion liefern
        if (m_nNULLPos != -1 && nSelCount == 1 && pSels[0] == m_nNULLPos)
            nSelCount = 0;

        aCurValues.realloc(nSelCount);
        ::rtl::OUString *pCurVals = aCurValues.getArray();

        for (sal_uInt16 i = 0; i < nSelCount; i++)
        {
            if (pSels[i] < nValCnt)
                pCurVals[i] = pVals[pSels[i]];
        }
    }
    return aCurValues;
}

// XBoundComponent
//------------------------------------------------------------------------------
sal_Bool OListBoxModel::_commit()
{
    // derzeitige Selectionsliste
    staruno::Any aNewValue;
    StringSequence aCurValueSeq = GetCurValueSeq();
    if (aCurValueSeq.getLength())
        aNewValue <<= aCurValueSeq.getConstArray()[0];

    if (!compare(aNewValue, m_aSaveValue))
    {
        if (!aNewValue.hasValue())
            m_xColumnUpdate->updateNull();
        else
        {
            try
            {
                ::rtl::OUString sNewValue;
                aNewValue >>= sNewValue;
                m_xColumnUpdate->updateString(sNewValue);
            }
            catch(...)
            {
                return sal_False;
            }
        }
        m_aSaveValue = aNewValue;
    }
    return sal_True;
}

// XPropertiesChangeListener
//------------------------------------------------------------------------------
void OListBoxModel::_onValueChanged()
{
    DBG_ASSERT(m_xAggregateFastSet.is() && m_xAggregateSet.is(), "OListBoxModel::_onValueChanged : invalid aggregate !");
    if (!m_xAggregateFastSet.is() || !m_xAggregateSet.is())
        return;

    staruno::Sequence<sal_Int16> aSelSeq;

    // Bei NULL-Eintraegen Selektion aufheben!
    ::rtl::OUString sValue = m_xColumn->getString();
    if (m_xColumn->wasNull())
    {
        m_aSaveValue.clear();
        if (m_nNULLPos != -1)
        {
            aSelSeq.realloc(1);
            aSelSeq.getArray()[0] = m_nNULLPos;
        }
    }
    else
    {
        m_aSaveValue <<= sValue;

        // In der Werteliste nur einzelne Werte suchen, wenn das Control mit einem Datenbankfeld verbunden ist
        if (m_aValueSeq.getLength())    // WerteListe
            aSelSeq = findValue(m_aValueSeq, sValue, m_bBoundComponent);
        else
        {
            StringSequence aStringSeq = *(StringSequence*)m_xAggregateSet->getPropertyValue(PROPERTY_STRINGITEMLIST).getValue();
            aSelSeq = findValue(aStringSeq, sValue, m_bBoundComponent);
        }
    }
    staruno::Any aSelectAny;
    aSelectAny <<= aSelSeq;
    {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(OListBoxModel::nSelectHandle, aSelectAny );
    }
}

// XReset
//------------------------------------------------------------------------------
void OListBoxModel::_reset( void )
{
    DBG_ASSERT(m_xAggregateFastSet.is() && m_xAggregateSet.is(), "OListBoxModel::reset : invalid aggregate !");
    if (!m_xAggregateFastSet.is() || !m_xAggregateSet.is())
        return;

    staruno::Any aValue;
    if (m_aDefaultSelectSeq.getLength())
        aValue <<= m_aDefaultSelectSeq;
    else if (m_nNULLPos != -1)  // gebundene Listbox
    {
        staruno::Sequence<sal_Int16> aSeq(1);
        aSeq.getArray()[0] = m_nNULLPos;
        aValue <<= aSeq;
    }
    else
    {
        staruno::Sequence<sal_Int16> aSeq;
        aValue <<= aSeq;
    }
    {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(OListBoxModel::nSelectHandle, aValue);
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OListBoxModel::addSQLErrorListener(const staruno::Reference<starsdb::XSQLErrorListener>& _rxListener) throw(staruno::RuntimeException)
{
    m_aErrorListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OListBoxModel::removeSQLErrorListener(const staruno::Reference<starsdb::XSQLErrorListener>& _rxListener) throw(staruno::RuntimeException)
{
    m_aErrorListeners.removeInterface(_rxListener);
}

//------------------------------------------------------------------------------
void OListBoxModel::onError(starsdbc::SQLException& _rException, const ::rtl::OUString& _rContextDescription)
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

//==================================================================
// OListBoxControl
//==================================================================

//------------------------------------------------------------------
InterfaceRef SAL_CALL OListBoxControl_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory) throw (staruno::RuntimeException)
{
    return *(new OListBoxControl(_rxFactory));
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Type> OListBoxControl::_getTypes()
{
    static staruno::Sequence<staruno::Type> aTypes;
    if (!aTypes.getLength())
    {
        // my two base classes
        staruno::Sequence<staruno::Type> aBaseClassTypes = OBoundControl::_getTypes();

        staruno::Sequence<staruno::Type> aOwnTypes(2);
        staruno::Type* pOwnTypes = aOwnTypes.getArray();
        pOwnTypes[0] = ::getCppuType((staruno::Reference<starawt::XFocusListener>*)NULL);
        pOwnTypes[1] = ::getCppuType((staruno::Reference<starform::XChangeBroadcaster>*)NULL);

        aTypes = concatSequences(aBaseClassTypes, aOwnTypes);
    }
    return aTypes;
}

//------------------------------------------------------------------
staruno::Any SAL_CALL OListBoxControl::queryAggregation(const staruno::Type& _rType) throw (staruno::RuntimeException)
{
    staruno::Any aReturn;

    aReturn = ::cppu::queryInterface(_rType
        ,static_cast<starawt::XFocusListener*>(this)
        ,static_cast<starform::XChangeBroadcaster*>(this)
    );
    if (!aReturn.hasValue())
        aReturn = OBoundControl::queryAggregation(_rType);

    return aReturn;
}

DBG_NAME(OListBoxControl);
//------------------------------------------------------------------------------
OListBoxControl::OListBoxControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
                  :OBoundControl(_rxFactory, VCL_CONTROL_LISTBOX)
                  ,m_aChangeListeners(m_aMutex)
{
    DBG_CTOR(OListBoxControl,NULL);

    increment(m_refCount);
    {
        // als FocusListener anmelden
        staruno::Reference<starawt::XWindow> xComp;
        if (query_aggregation(m_xAggregate, xComp))
            xComp->addFocusListener(this);

        // als ItemListener anmelden
        staruno::Reference<starawt::XListBox> xListbox;
        if (query_aggregation(m_xAggregate, xListbox))
            xListbox->addItemListener(this);
    }
    // Refcount bei 2 fuer angemeldete Listener
    sal_Int32 n = decrement(m_refCount);

    m_aChangeTimer.SetTimeout(500);
    m_aChangeTimer.SetTimeoutHdl(LINK(this,OListBoxControl,OnTimeout));
}

//------------------------------------------------------------------------------
OListBoxControl::~OListBoxControl()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR(OListBoxControl,NULL);
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OListBoxControl::getSupportedServiceNames() throw(staruno::RuntimeException)
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_LISTBOX;
    return aSupported;
}


// XFocusListener
//------------------------------------------------------------------------------
void SAL_CALL OListBoxControl::focusGained(const starawt::FocusEvent& _rEvent) throw(staruno::RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard(m_aMutex);
    if (m_aChangeListeners.getLength()) // only if there are listeners
    {
        staruno::Reference<starbeans::XPropertySet> xSet(getModel(), staruno::UNO_QUERY);
        if (xSet.is())
        {
            // memorize the current selection for posting the change event
            m_aCurrentSelection = xSet->getPropertyValue(PROPERTY_SELECT_SEQ);
        }
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OListBoxControl::focusLost(const starawt::FocusEvent& _rEvent) throw(staruno::RuntimeException)
{
    m_aCurrentSelection.clear();
}

// XItemListener
//------------------------------------------------------------------------------
void SAL_CALL OListBoxControl::itemStateChanged(const starawt::ItemEvent& _rEvent) throw(staruno::RuntimeException)
{
   // call the changelistener delayed
   ::osl::ClearableMutexGuard aGuard(m_aMutex);
   if (m_aChangeTimer.IsActive())
   {
       staruno::Reference<starbeans::XPropertySet> xSet(getModel(), staruno::UNO_QUERY);
       m_aCurrentSelection = xSet->getPropertyValue(PROPERTY_SELECT_SEQ);

       m_aChangeTimer.Stop();
       m_aChangeTimer.Start();
   }
   else
   {
       if (m_aChangeListeners.getLength() && m_aCurrentSelection.hasValue())
       {
            staruno::Reference<starbeans::XPropertySet> xSet(getModel(), staruno::UNO_QUERY);
            if (xSet.is())
            {
                // Has the selection been changed?
                sal_Bool bModified(sal_False);
                staruno::Any aValue = xSet->getPropertyValue(PROPERTY_SELECT_SEQ);

                staruno::Sequence<sal_Int16>& rSelection = *(staruno::Sequence<sal_Int16> *)aValue.getValue();
                staruno::Sequence<sal_Int16>& rOldSelection = *(staruno::Sequence<sal_Int16> *)m_aCurrentSelection.getValue();
                sal_Int32 nLen = rSelection.getLength();
                if (nLen != rOldSelection.getLength())
                    bModified = sal_True;
                else
                {
                    const sal_Int16* pVal = rSelection.getConstArray();
                    const sal_Int16* pCompVal = rOldSelection.getConstArray();

                    while (nLen-- && !bModified)
                        bModified = pVal[nLen] != pCompVal[nLen];
                }

                if (bModified)
                {
                    m_aCurrentSelection = aValue;
                    m_aChangeTimer.Start();
                }
            }
        }
        else if (m_aCurrentSelection.hasValue())
            m_aCurrentSelection.clear();
   }
}

// XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OListBoxControl::disposing(const starlang::EventObject& _rSource) throw (staruno::RuntimeException)
{
    OBoundControl::disposing(_rSource);
}

// XChangeBroadcaster
//------------------------------------------------------------------------------
void SAL_CALL OListBoxControl::addChangeListener(const staruno::Reference<starform::XChangeListener>& _rxListener) throw(staruno::RuntimeException)
{
    m_aChangeListeners.addInterface(_rxListener);
}

//------------------------------------------------------------------------------
void SAL_CALL OListBoxControl::removeChangeListener(const staruno::Reference<starform::XChangeListener>& _rxListener) throw(staruno::RuntimeException)
{
    m_aChangeListeners.removeInterface(_rxListener);
}

// OComponentHelper
//------------------------------------------------------------------------------
void OListBoxControl::disposing()
{
    if (m_aChangeTimer.IsActive())
        m_aChangeTimer.Stop();

    starlang::EventObject aEvt(static_cast<staruno::XWeak*>(this));
    m_aChangeListeners.disposeAndClear(aEvt);

    OBoundControl::disposing();
}

//------------------------------------------------------------------------------
IMPL_LINK(OListBoxControl, OnTimeout, void*, EMPTYTAG)
{
    starlang::EventObject aEvt(static_cast<staruno::XWeak*>(this));
    NOTIFY_LISTENERS(m_aChangeListeners, starform::XChangeListener, changed, aEvt);
    return 1;
}

//.........................................................................
}
//.........................................................................

