/*************************************************************************
 *
 *  $RCSfile: EditBase.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:05 $
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

#ifndef _FORMS_EDITBASE_HXX_
#include "EditBase.hxx"
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

//.........................................................................
namespace frm
{

const sal_uInt16 DEFAULT_LONG    =  0x0001;
const sal_uInt16 DEFAULT_DOUBLE  =  0x0002;
const sal_uInt16 FILTERPROPOSAL  =  0x0004;

//------------------------------------------------------------------
OEditBaseModel::OEditBaseModel(
            const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory,
            const ::rtl::OUString& rUnoControlModelName,
            const ::rtl::OUString& rDefault )
     :OBoundControlModel( _rxFactory, rUnoControlModelName, rDefault )
     ,m_bFilterProposal(sal_False)
     ,m_bEmptyIsNull(sal_True)
     ,m_nLastReadVersion(0)
{
}

// stario::XPersist
//------------------------------------------------------------------------------
void OEditBaseModel::write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream)
{
    OBoundControlModel::write(_rxOutStream);

    // Version
    sal_uInt16 nVersionId = 0x0005;
    DBG_ASSERT((getPersistenceFlags() & ~(sal_Int16)PF_SPECIAL_FLAGS) == 0,
        "OEditBaseModel::write : invalid special version flags !");
        // please don't use other flags, older versions can't interpret them !

    nVersionId |= getPersistenceFlags();
    _rxOutStream->writeShort(nVersionId);

    // Name
    _rxOutStream->writeShort(0);    // obsolete
    _rxOutStream << m_aDefaultText;

    // Maskierung fuer any
    sal_uInt16 nAnyMask = 0;
    if (m_aDefault.getValueType().getTypeClass() == staruno::TypeClass_LONG)
        nAnyMask |= DEFAULT_LONG;
    else if (m_aDefault.getValueType().getTypeClass() == staruno::TypeClass_DOUBLE)
        nAnyMask |= DEFAULT_DOUBLE;

    if (m_bFilterProposal)  // da boolean, kein Wert speichern
        nAnyMask |= FILTERPROPOSAL;

    _rxOutStream->writeBoolean(m_bEmptyIsNull);
    _rxOutStream->writeShort(nAnyMask);

    if ((nAnyMask & DEFAULT_LONG) == DEFAULT_LONG)
        _rxOutStream->writeLong(getINT32(m_aDefault));
    else if ((nAnyMask & DEFAULT_DOUBLE) == DEFAULT_DOUBLE)
        _rxOutStream->writeDouble(getDouble(m_aDefault));

    // since version 5 we write the help text
    _rxOutStream << m_aHelpText;
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
sal_Int16 OEditBaseModel::getPersistenceFlags() const
{
    return PF_HANDLE_COMMON_PROPS;
}

//------------------------------------------------------------------------------
void OEditBaseModel::read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream)
{
    OBoundControlModel::read(_rxInStream);
    ::osl::MutexGuard aGuard(m_aMutex);

    // Version eigene Versionsnummer
    sal_uInt16 nVersion = _rxInStream->readShort();
    m_nLastReadVersion = nVersion;

    sal_Bool bHandleCommonProps = (nVersion & PF_HANDLE_COMMON_PROPS) != 0;
    nVersion = nVersion & ~PF_SPECIAL_FLAGS;

    sal_uInt16 nOld     = _rxInStream->readShort();
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
        _rxInStream >> m_aHelpText;

    if (bHandleCommonProps)
        readCommonEditProperties(_rxInStream);

    // Nach dem Lesen die Defaultwerte anzeigen
    if (m_aControlSource.getLength())
        // (not if we don't have a control source - the "State" property acts like it is persistent, then)
        _reset();
};

//------------------------------------------------------------------------------
void OEditBaseModel::defaultCommonEditProperties()
{
    OBoundControlModel::defaultCommonProperties();
    // no own common properties at the moment
}

//------------------------------------------------------------------------------
void OEditBaseModel::readCommonEditProperties(const staruno::Reference<stario::XObjectInputStream>& _rxInStream)
{
    sal_Int32 nLen = _rxInStream->readLong();

    staruno::Reference<stario::XMarkableStream>  xMark(_rxInStream, staruno::UNO_QUERY);
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
void OEditBaseModel::writeCommonEditProperties(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream)
{
    staruno::Reference<stario::XMarkableStream>  xMark(_rxOutStream, staruno::UNO_QUERY);
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
void OEditBaseModel::getFastPropertyValue( staruno::Any& rValue, sal_Int32 nHandle ) const
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
sal_Bool OEditBaseModel::convertFastPropertyValue( staruno::Any& rConvertedValue, staruno::Any& rOldValue,
                                            sal_Int32 nHandle, const staruno::Any& rValue ) throw( starlang::IllegalArgumentException )
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
void OEditBaseModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const staruno::Any& rValue )
{
    switch (nHandle)
    {
        case PROPERTY_ID_EMPTY_IS_NULL:
            DBG_ASSERT(rValue.getValueType().getTypeClass() == staruno::TypeClass_BOOLEAN, "invalid type" );
            m_bEmptyIsNull = getBOOL(rValue);
            break;
        case PROPERTY_ID_FILTERPROPOSAL:
            DBG_ASSERT(rValue.getValueType().getTypeClass() == staruno::TypeClass_BOOLEAN, "invalid type" );
            m_bFilterProposal = getBOOL(rValue);
            break;
        // Aenderung der defaultwerte fuehrt zu reset
        case PROPERTY_ID_DEFAULT_TEXT:
            DBG_ASSERT(rValue.getValueType().getTypeClass() == staruno::TypeClass_STRING, "invalid type" );
            rValue >>= m_aDefaultText;
            _reset();
            break;
        case PROPERTY_ID_DEFAULT_VALUE:
        case PROPERTY_ID_DEFAULT_DATE:
        case PROPERTY_ID_DEFAULT_TIME:
            m_aDefault = rValue;
            _reset();
            break;
        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue );
    }
}

//XPropertyState
//------------------------------------------------------------------------------
starbeans::PropertyState OEditBaseModel::getPropertyStateByHandle(sal_Int32 nHandle)
{
    starbeans::PropertyState eState;
    switch (nHandle)
    {
        case PROPERTY_ID_DEFAULT_TEXT:
            if (!m_aDefaultText.len())
                eState = starbeans::PropertyState_DEFAULT_VALUE;
            else
                eState = starbeans::PropertyState_DIRECT_VALUE;
            break;
        case PROPERTY_ID_FILTERPROPOSAL:
            if (!m_bFilterProposal)
                eState = starbeans::PropertyState_DEFAULT_VALUE;
            else
                eState = starbeans::PropertyState_DIRECT_VALUE;
            break;
        case PROPERTY_ID_DEFAULT_VALUE:
        case PROPERTY_ID_DEFAULT_DATE:
        case PROPERTY_ID_DEFAULT_TIME:
            if (!m_aDefault.hasValue())
                eState = starbeans::PropertyState_DEFAULT_VALUE;
            else
                eState = starbeans::PropertyState_DIRECT_VALUE;
            break;
        default:
            eState = OBoundControlModel::getPropertyStateByHandle(nHandle);
    }
    return eState;
}

//------------------------------------------------------------------------------
void OEditBaseModel::setPropertyToDefaultByHandle(sal_Int32 nHandle)
{
    switch (nHandle)
    {
        case PROPERTY_ID_DEFAULT_TEXT:
            setFastPropertyValue(nHandle, staruno::makeAny(::rtl::OUString()));
            break;
        case PROPERTY_ID_FILTERPROPOSAL:
            setFastPropertyValue(nHandle, staruno::makeAny((sal_Bool)sal_False));
            break;
        case PROPERTY_ID_DEFAULT_VALUE:
        case PROPERTY_ID_DEFAULT_DATE:
        case PROPERTY_ID_DEFAULT_TIME:
            setFastPropertyValue(nHandle, staruno::Any());
            break;
        default:
            OBoundControlModel::setPropertyToDefaultByHandle(nHandle);
    }
}

//------------------------------------------------------------------------------
staruno::Any OEditBaseModel::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_DEFAULT_TEXT:
            return staruno::makeAny(::rtl::OUString());
            break;
        case PROPERTY_ID_FILTERPROPOSAL:
            return staruno::Any(staruno::makeAny((sal_Bool)sal_False));
            break;
        case PROPERTY_ID_DEFAULT_VALUE:
        case PROPERTY_ID_DEFAULT_DATE:
        case PROPERTY_ID_DEFAULT_TIME:
            return staruno::Any();
        default:
            return OBoundControlModel::getPropertyDefaultByHandle(nHandle);
    }
}

//.........................................................................
}
//.........................................................................

