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

#include "EditBase.hxx"
#include "property.hxx"
#include "property.hrc"
#include "services.hxx"
#include <tools/debug.hxx>
#include <comphelper/basicio.hxx>
#include <cppuhelper/queryinterface.hxx>
#include "frm_resource.hxx"
#include "frm_resource.hrc"

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

const sal_uInt16 DEFAULT_LONG    =  0x0001;
const sal_uInt16 DEFAULT_DOUBLE  =  0x0002;
const sal_uInt16 FILTERPROPOSAL  =  0x0004;

DBG_NAME( OEditBaseModel )
//------------------------------------------------------------------
OEditBaseModel::OEditBaseModel( const Reference< XMultiServiceFactory >& _rxFactory, const ::rtl::OUString& rUnoControlModelName,
        const ::rtl::OUString& rDefault, const sal_Bool _bSupportExternalBinding, const sal_Bool _bSupportsValidation )
    :OBoundControlModel( _rxFactory, rUnoControlModelName, rDefault, sal_True, _bSupportExternalBinding, _bSupportsValidation )
    ,m_nLastReadVersion(0)
    ,m_bEmptyIsNull(sal_True)
    ,m_bFilterProposal(sal_False)
{
    DBG_CTOR( OEditBaseModel, NULL );
}

//------------------------------------------------------------------
OEditBaseModel::OEditBaseModel( const OEditBaseModel* _pOriginal, const Reference< XMultiServiceFactory >& _rxFactory )
     :OBoundControlModel( _pOriginal, _rxFactory )
     ,m_nLastReadVersion(0)
{
    DBG_CTOR( OEditBaseModel, NULL );

    m_bFilterProposal = _pOriginal->m_bFilterProposal;
    m_bEmptyIsNull = _pOriginal->m_bEmptyIsNull;
    m_aDefault = _pOriginal->m_aDefault;
    m_aDefaultText = _pOriginal->m_aDefaultText;
}

//------------------------------------------------------------------
OEditBaseModel::~OEditBaseModel( )
{
    DBG_DTOR( OEditBaseModel, NULL );
}

// XPersist
//------------------------------------------------------------------------------
void OEditBaseModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OBoundControlModel::write(_rxOutStream);

    // Version
    sal_uInt16 nVersionId = 0x0005;
    DBG_ASSERT((getPersistenceFlags() & ~PF_SPECIAL_FLAGS) == 0,
        "OEditBaseModel::write : invalid special version flags !");
        // please don't use other flags, older versions can't interpret them !

    nVersionId |= getPersistenceFlags();
    _rxOutStream->writeShort(nVersionId);

    // Name
    _rxOutStream->writeShort(0);    // obsolete
    _rxOutStream << m_aDefaultText;

    // Masking for any
    sal_uInt16 nAnyMask = 0;
    if (m_aDefault.getValueType().getTypeClass() == TypeClass_LONG)
        nAnyMask |= DEFAULT_LONG;
    else if (m_aDefault.getValueType().getTypeClass() == TypeClass_DOUBLE)
        nAnyMask |= DEFAULT_DOUBLE;

    if (m_bFilterProposal)  // Don't save a value, because it's boolean
        nAnyMask |= FILTERPROPOSAL;

    _rxOutStream->writeBoolean(m_bEmptyIsNull);
    _rxOutStream->writeShort(nAnyMask);

    if ((nAnyMask & DEFAULT_LONG) == DEFAULT_LONG)
        _rxOutStream->writeLong(getINT32(m_aDefault));
    else if ((nAnyMask & DEFAULT_DOUBLE) == DEFAULT_DOUBLE)
        _rxOutStream->writeDouble(getDouble(m_aDefault));

    // since version 5 we write the help text
    writeHelpTextCompatibly(_rxOutStream);
    // (that's potentially bad : at the time I added the above line we had two derived classes : OEditModel and
    // OFormattedModel. The first one does not have an own version handling, so it can't write the help text itself,
    // the second one does it's own writing (reading) after calling our method, so normally we shouldn't write any
    // additional members as this is not compatible to older office versions.
    // We decided to place the writing of the help text here as it seems the less worse alternative. There is no delivered
    // office version including formatted controls (and thus the OFormattedModel), and the OFormattedModel::read seems
    // robust against this change (as it will read a wrong and unknown file version and thus set it's members to defaults).

    if ((nVersionId & PF_HANDLE_COMMON_PROPS) != 0)
        writeCommonEditProperties(_rxOutStream);

    // !!! properties common to all OEditBaseModel derived classes should be written in writeCommonEditProperties !!!
}

//------------------------------------------------------------------------------
sal_uInt16 OEditBaseModel::getPersistenceFlags() const
{
    return PF_HANDLE_COMMON_PROPS;
}

//------------------------------------------------------------------------------
void OEditBaseModel::read(const Reference<XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OBoundControlModel::read(_rxInStream);
    ::osl::MutexGuard aGuard(m_aMutex);

    // Version's own version number
    sal_uInt16 nVersion = _rxInStream->readShort();
    m_nLastReadVersion = nVersion;

    sal_Bool bHandleCommonProps = (nVersion & PF_HANDLE_COMMON_PROPS) != 0;
    nVersion = nVersion & ~PF_SPECIAL_FLAGS;

    // obsolete
    _rxInStream->readShort();

    _rxInStream >> m_aDefaultText;

    if (nVersion >= 0x0003)
    {
        m_bEmptyIsNull = _rxInStream->readBoolean();

        sal_uInt16 nAnyMask = _rxInStream->readShort();
        if ((nAnyMask & DEFAULT_LONG) == DEFAULT_LONG)
        {
            sal_Int32 nValue = _rxInStream->readLong();
            m_aDefault <<= (sal_Int32)nValue;
        }
        else if ((nAnyMask & DEFAULT_DOUBLE) == DEFAULT_DOUBLE)
        {
            double fValue = _rxInStream->readDouble();
            m_aDefault <<= (double)fValue;
        }

        if ((nAnyMask & FILTERPROPOSAL) == FILTERPROPOSAL)
            m_bFilterProposal = sal_True;
    }

    if (nVersion > 4)
        readHelpTextCompatibly(_rxInStream);

    if (bHandleCommonProps)
        readCommonEditProperties(_rxInStream);

    // After reading, display default values
    if ( !getControlSource().isEmpty() )
        // (not if we don't have a control source - the "State" property acts like it is persistent, then)
        resetNoBroadcast();
};

//------------------------------------------------------------------------------
void OEditBaseModel::defaultCommonEditProperties()
{
    OBoundControlModel::defaultCommonProperties();
    // no own common properties at the moment
}

//------------------------------------------------------------------------------
void OEditBaseModel::readCommonEditProperties(const Reference<XObjectInputStream>& _rxInStream)
{
    sal_Int32 nLen = _rxInStream->readLong();

    Reference<XMarkableStream>  xMark(_rxInStream, UNO_QUERY);
    DBG_ASSERT(xMark.is(), "OBoundControlModel::readCommonProperties : can only work with markable streams !");
    sal_Int32 nMark = xMark->createMark();

    // read properties common to all OBoundControlModels
    OBoundControlModel::readCommonProperties(_rxInStream);

    // read properties common to all OEditBaseModels

    // skip the remaining bytes
    xMark->jumpToMark(nMark);
    _rxInStream->skipBytes(nLen);
    xMark->deleteMark(nMark);
}

//------------------------------------------------------------------------------
void OEditBaseModel::writeCommonEditProperties(const Reference<XObjectOutputStream>& _rxOutStream)
{
    Reference<XMarkableStream>  xMark(_rxOutStream, UNO_QUERY);
    DBG_ASSERT(xMark.is(), "OEditBaseModel::writeCommonProperties : can only work with markable streams !");
    sal_Int32 nMark = xMark->createMark();

    // a placeholder where we will write the overall length (later in this method)
    sal_Int32 nLen = 0;
    _rxOutStream->writeLong(nLen);

    // write properties common to all OBoundControlModels
    OBoundControlModel::writeCommonProperties(_rxOutStream);

    // write properties common to all OEditBaseModels

    // close the block - write the correct length at the beginning
    nLen = xMark->offsetToMark(nMark) - sizeof(nLen);
    xMark->jumpToMark(nMark);
    _rxOutStream->writeLong(nLen);
    xMark->jumpToFurthest();
    xMark->deleteMark(nMark);
}

//------------------------------------------------------------------------------
void OEditBaseModel::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_EMPTY_IS_NULL:
            rValue <<= (sal_Bool)m_bEmptyIsNull;
            break;
        case PROPERTY_ID_FILTERPROPOSAL:
            rValue <<= (sal_Bool)m_bFilterProposal;
            break;
        case PROPERTY_ID_DEFAULT_TEXT:
            rValue <<= m_aDefaultText;
            break;
        case PROPERTY_ID_DEFAULT_VALUE:
        case PROPERTY_ID_DEFAULT_DATE:
        case PROPERTY_ID_DEFAULT_TIME:
            rValue = m_aDefault;
            break;
        default:
            OBoundControlModel::getFastPropertyValue(rValue, nHandle);
    }
}

//------------------------------------------------------------------------------
sal_Bool OEditBaseModel::convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue,
                                            sal_Int32 nHandle, const Any& rValue ) throw( IllegalArgumentException )
{
    sal_Bool bModified(sal_False);
    switch (nHandle)
    {
        case PROPERTY_ID_EMPTY_IS_NULL:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bEmptyIsNull);
            break;
        case PROPERTY_ID_FILTERPROPOSAL:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bFilterProposal);
            break;
        case PROPERTY_ID_DEFAULT_TEXT:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aDefaultText);
            break;
        case PROPERTY_ID_DEFAULT_VALUE:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aDefault, ::getCppuType((const double*)0));
            break;
        case PROPERTY_ID_DEFAULT_DATE:
        case PROPERTY_ID_DEFAULT_TIME:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aDefault, ::getCppuType((const sal_Int32*)0));
            break;
        default:
            bModified = OBoundControlModel::convertFastPropertyValue(
                                            rConvertedValue,
                                            rOldValue,
                                            nHandle,
                                            rValue);
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OEditBaseModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw ( ::com::sun::star::uno::Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_EMPTY_IS_NULL:
            DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_BOOLEAN, "invalid type" );
            m_bEmptyIsNull = getBOOL(rValue);
            break;
        case PROPERTY_ID_FILTERPROPOSAL:
            DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_BOOLEAN, "invalid type" );
            m_bFilterProposal = getBOOL(rValue);
            break;
        // Changing the default values causes a reset
        case PROPERTY_ID_DEFAULT_TEXT:
            DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_STRING, "invalid type" );
            rValue >>= m_aDefaultText;
            resetNoBroadcast();
            break;
        case PROPERTY_ID_DEFAULT_VALUE:
        case PROPERTY_ID_DEFAULT_DATE:
        case PROPERTY_ID_DEFAULT_TIME:
            m_aDefault = rValue;
            resetNoBroadcast();
            break;
        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue );
    }
}

// XPropertyState
//------------------------------------------------------------------------------
Any OEditBaseModel::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_DEFAULT_TEXT:
            return makeAny(::rtl::OUString());
        case PROPERTY_ID_FILTERPROPOSAL:
            return Any(makeAny((sal_Bool)sal_False));
        case PROPERTY_ID_DEFAULT_VALUE:
        case PROPERTY_ID_DEFAULT_DATE:
        case PROPERTY_ID_DEFAULT_TIME:
            return Any();
        default:
            return OBoundControlModel::getPropertyDefaultByHandle(nHandle);
    }
}

//.........................................................................
}
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
