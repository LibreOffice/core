/*************************************************************************
 *
 *  $RCSfile: Columns.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-22 16:37:15 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#ifndef _FRM_COLUMNS_HXX
#include "Columns.hxx"
#endif
#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_PROPERTY_HXX_
#include "property.hxx"
#endif
#ifndef _FRM_IDS_HXX_
#include "ids.hxx"
#endif

#ifndef _COM_SUN_STAR_IO_XPERSISTOBJECT_HPP_
#include <com/sun/star/io/XPersistObject.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XMARKABLESTREAM_HPP_
#include <com/sun/star/io/XMarkableStream.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCOMPONENT_HPP_
#include <com/sun/star/form/XFormComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif

#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif
#ifndef _FRM_RESOURCE_HRC_
#include "frm_resource.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

namespace internal {
sal_Int32 findPos(const ::rtl::OUString& aStr, const StringSequence& rList);
}

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

const sal_uInt16 WIDTH              = 0x0001;
const sal_uInt16 ALIGN              = 0x0002;
const sal_uInt16 OLD_HIDDEN         = 0x0004;
const sal_uInt16 COMPATIBLE_HIDDEN  = 0x0008;

IMPLEMENT_CONSTASCII_USTRING(FRM_COL_TEXTFIELD, "TextField");
IMPLEMENT_CONSTASCII_USTRING(FRM_COL_COMBOBOX, "ComboBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_COL_CHECKBOX, "CheckBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_COL_TIMEFIELD, "TimeField");
IMPLEMENT_CONSTASCII_USTRING(FRM_COL_DATEFIELD, "DateField");
IMPLEMENT_CONSTASCII_USTRING(FRM_COL_NUMERICFIELD, "NumericField");
IMPLEMENT_CONSTASCII_USTRING(FRM_COL_CURRENCYFIELD, "CurrencyField");
IMPLEMENT_CONSTASCII_USTRING(FRM_COL_PATTERNFIELD, "PatternField");
IMPLEMENT_CONSTASCII_USTRING(FRM_COL_LISTBOX, "ListBox");
IMPLEMENT_CONSTASCII_USTRING(FRM_COL_FORMATTEDFIELD, "FormattedField");

//------------------------------------------------------------------------------
const StringSequence& getColumnTypes()
{
    static StringSequence aColumnTypes(10);
    if (!aColumnTypes.getConstArray()[0].getLength())
    {
        ::rtl::OUString* pNames = aColumnTypes.getArray();
        pNames[TYPE_CHECKBOX]       = FRM_COL_CHECKBOX;
        pNames[TYPE_COMBOBOX]       = FRM_COL_COMBOBOX;
        pNames[TYPE_CURRENCYFIELD]  = FRM_COL_CURRENCYFIELD;
        pNames[TYPE_DATEFIELD]      = FRM_COL_DATEFIELD;
        pNames[TYPE_FORMATTEDFIELD] = FRM_COL_FORMATTEDFIELD;
        pNames[TYPE_LISTBOX]        = FRM_COL_LISTBOX;
        pNames[TYPE_NUMERICFIELD]   = FRM_COL_NUMERICFIELD;
        pNames[TYPE_PATTERNFIELD]   = FRM_COL_PATTERNFIELD;
        pNames[TYPE_TEXTFIELD]      = FRM_COL_TEXTFIELD;
        pNames[TYPE_TIMEFIELD]      = FRM_COL_TIMEFIELD;
    }
    return aColumnTypes;
}

//------------------------------------------------------------------------------
sal_Int32 getColumnTypeByModelName(const ::rtl::OUString& aModelName)
{
    const ::rtl::OUString aModelPrefix = ::rtl::OUString::createFromAscii("com.sun.star.form.component.");
    const ::rtl::OUString aCompatibleModelPrefix = ::rtl::OUString::createFromAscii("stardiv.one.form.component.");

    sal_Int32 nTypeId = -1;
    if (aModelName == FRM_COMPONENT_EDIT)
        nTypeId = TYPE_TEXTFIELD;
    else
    {
        sal_Int32 nPrefixPos = aModelName.search(aModelPrefix);
        sal_Int32 nCampatiblePrefixPos = aModelName.search(aCompatibleModelPrefix);
        DBG_ASSERT( (nPrefixPos != -1) ||   (nCampatiblePrefixPos != -1),
                "::getColumnTypeByModelName() : wrong servivce !");

        ::rtl::OUString aColumnType = (nPrefixPos != -1)
            ? aModelName.copy(aModelPrefix.len())
            : aModelName.copy(aCompatibleModelPrefix.len());

        const StringSequence& rColumnTypes = getColumnTypes();
        nTypeId = ::internal::findPos(aColumnType, rColumnTypes);
    }
    return nTypeId;
}

/*************************************************************************/

//------------------------------------------------------------------
InterfaceRef SAL_CALL OGridColumn_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OGridColumn(_rxFactory));
}

//------------------------------------------------------------------
const Sequence<sal_Int8>& OGridColumn::getUnoTunnelImplementationId()
{
    static Sequence< sal_Int8 > * pSeq = 0;
    if( !pSeq )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

//------------------------------------------------------------------
sal_Int64 SAL_CALL OGridColumn::getSomething( const Sequence<sal_Int8>& _rIdentifier) throw(RuntimeException)
{
    sal_Int64 nReturn(0);

    if  (   (_rIdentifier.getLength() == 16)
        &&  (0 == rtl_compareMemory( getUnoTunnelImplementationId().getConstArray(), _rIdentifier.getConstArray(), 16 ))
        )
    {
        nReturn = reinterpret_cast<sal_Int64>(this);
    }
    else
    {
        Reference<XUnoTunnel> xAggregateTunnel(m_xAggregate, UNO_QUERY);
        if (xAggregateTunnel.is())
            nReturn = xAggregateTunnel->getSomething(_rIdentifier);
    }
    return nReturn;
}

//------------------------------------------------------------------
Sequence<sal_Int8> SAL_CALL OGridColumn::getImplementationId() throw(RuntimeException)
{
    return OImplementationIds::getImplementationId(getTypes());
}

//------------------------------------------------------------------
Sequence<Type> SAL_CALL OGridColumn::getTypes() throw(RuntimeException)
{
    Reference<XTypeProvider> xProv;

    if (query_aggregation(m_xAggregate, xProv))
        return concatSequences(OGridColumn_BASE::getTypes(), xProv->getTypes());
    return OGridColumn_BASE::getTypes();
}

//------------------------------------------------------------------
Any SAL_CALL OGridColumn::queryAggregation( const Type& _rType ) throw (RuntimeException)
{
    Any aReturn;
    // though our aggregate may be an XFormComponent or an XServiceInfo, we aren't anymore
    if  (   _rType.equals(::getCppuType(static_cast< Reference< XFormComponent >* >(NULL)))
        ||  _rType.equals(::getCppuType(static_cast< Reference< XServiceInfo >* >(NULL)))
        )
        return aReturn;

    aReturn = OGridColumn_BASE::queryAggregation(_rType);
    if (!aReturn.hasValue())
    {
        aReturn = OPropertySetAggregationHelper::queryInterface(_rType);
        if (!aReturn.hasValue() && m_xAggregate.is())
            aReturn = m_xAggregate->queryAggregation(_rType);
    }

    return aReturn;
}

DBG_NAME(OGridColumn);
//------------------------------------------------------------------------------
OGridColumn::OGridColumn(const Reference<XMultiServiceFactory>& _rxFactory, const ::rtl::OUString& _sModelName)
          :OGridColumn_BASE(m_aMutex)
          ,OPropertySetAggregationHelper(OGridColumn_BASE::rBHelper)
          ,m_aModelName(_sModelName)
{
    DBG_CTOR(OGridColumn,NULL);
    // Anlegen des UnoControlModels
    if (m_aModelName.getLength())    // gibt es ein zu aggregierendes Model
    {
        increment(m_refCount);

        // Muss im eigenen Block,
        // da xAgg vor dem delegator setzen wieder freigesetzt sein muﬂ !
        {
            m_xAggregate = Reference<XAggregation> (_rxFactory->createInstance(m_aModelName), UNO_QUERY);
            setAggregation(m_xAggregate);
        }

        if (m_xAggregate.is())
        {
            m_xAggregate->setDelegator(static_cast< ::cppu::OWeakObject* >(this));
        }

        // Refcount wieder bei NULL
        decrement(m_refCount);
    }

    m_aHidden <<= (sal_Bool)sal_False;
}

//------------------------------------------------------------------------------
OGridColumn::~OGridColumn()
{
    if (!OGridColumn_BASE::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    // freigeben der Agg
    if (m_xAggregate.is())
    {
        InterfaceRef  xIface;
        m_xAggregate->setDelegator(xIface);
    }
    DBG_DTOR(OGridColumn,NULL);
}

// XChild
//------------------------------------------------------------------------------
void SAL_CALL OGridColumn::setParent(const InterfaceRef& Parent) throw(NoSupportException, RuntimeException)
{
    m_xParent = Parent;
}

// XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OGridColumn::disposing(const EventObject& _rSource) throw(RuntimeException)
{
    OPropertySetAggregationHelper::disposing(_rSource);

    Reference<XEventListener>  xEvtLstner;
    if (query_aggregation(m_xAggregate, xEvtLstner))
        xEvtLstner->disposing(_rSource);
}

// OGridColumn_BASE
//-----------------------------------------------------------------------------
void OGridColumn::disposing()
{
    OGridColumn_BASE::disposing();
    OPropertySetAggregationHelper::disposing();

    Reference<XComponent>  xComp;
    if (query_aggregation(m_xAggregate, xComp))
        xComp->dispose();

    setParent(InterfaceRef ());
}

//------------------------------------------------------------------------------
void OGridColumn::clearAggregateProperties(Sequence<Property>& seqProps, sal_Bool bAllowDropDown)
{
    RemoveProperty(seqProps, PROPERTY_ALIGN);
    RemoveProperty(seqProps, PROPERTY_AUTOCOMPLETE);
    RemoveProperty(seqProps, PROPERTY_BACKGROUNDCOLOR);
    RemoveProperty(seqProps, PROPERTY_BORDER);
    if (!bAllowDropDown)
        RemoveProperty(seqProps, PROPERTY_DROPDOWN);
    RemoveProperty(seqProps, PROPERTY_ECHO_CHAR);
    RemoveProperty(seqProps, PROPERTY_FILLCOLOR);
    RemoveProperty(seqProps, PROPERTY_FONT);
    RemoveProperty(seqProps, PROPERTY_FONT_NAME);
    RemoveProperty(seqProps, PROPERTY_FONT_STYLENAME);
    RemoveProperty(seqProps, PROPERTY_FONT_FAMILY);
    RemoveProperty(seqProps, PROPERTY_FONT_CHARSET);
    RemoveProperty(seqProps, PROPERTY_FONT_HEIGHT);
    RemoveProperty(seqProps, PROPERTY_FONT_WEIGHT);
    RemoveProperty(seqProps, PROPERTY_FONT_SLANT);
    RemoveProperty(seqProps, PROPERTY_FONT_UNDERLINE);
    RemoveProperty(seqProps, PROPERTY_FONT_STRIKEOUT);
    RemoveProperty(seqProps, PROPERTY_HARDLINEBREAKS);
    RemoveProperty(seqProps, PROPERTY_HSCROLL);
    RemoveProperty(seqProps, PROPERTY_LABEL);
    RemoveProperty(seqProps, PROPERTY_LINECOLOR);
    RemoveProperty(seqProps, PROPERTY_MULTI);
    RemoveProperty(seqProps, PROPERTY_MULTILINE);
    RemoveProperty(seqProps, PROPERTY_MULTISELECTION);
    RemoveProperty(seqProps, PROPERTY_PRINTABLE);
    RemoveProperty(seqProps, PROPERTY_TABINDEX);
    RemoveProperty(seqProps, PROPERTY_TABSTOP);
    RemoveProperty(seqProps, PROPERTY_TEXTCOLOR);
    RemoveProperty(seqProps, PROPERTY_TRISTATE);
    RemoveProperty(seqProps, PROPERTY_VSCROLL);
    RemoveProperty(seqProps, PROPERTY_CONTROLLABEL);
}

//------------------------------------------------------------------------------
void OGridColumn::setOwnProperties(Sequence<Property>& aDescriptor)
{
    aDescriptor.realloc(5);
    Property* pProps = aDescriptor.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP1(LABEL,               ::rtl::OUString,    BOUND);
    DECL_PROP3(WIDTH,               sal_Int32,          BOUND, MAYBEVOID, MAYBEDEFAULT);
    DECL_PROP3(ALIGN,               sal_Int16,          BOUND, MAYBEVOID, MAYBEDEFAULT);
    DECL_BOOL_PROP2(HIDDEN,                             BOUND, MAYBEDEFAULT);
    DECL_PROP1(COLUMNSERVICENAME,   ::rtl::OUString,    READONLY);
}

// Reference<XPropertySet>
//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL OGridColumn::getPropertySetInfo() throw(RuntimeException)
{
    DBG_ERROR("OGridColumn::getPropertySetInfo() : Dummy Called");
    return Reference<XPropertySetInfo> ();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OGridColumn::getInfoHelper()
{
    DBG_ERROR("OGridColumn::getInfoHelper() : Dummy Called");

    Sequence<Property> aDescriptor, aAggProperties;
    static OPropertyArrayAggregationHelper aDescAry(aDescriptor, aAggProperties);
    return aDescAry;
}

//------------------------------------------------------------------------------
void OGridColumn::getFastPropertyValue(Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_COLUMNSERVICENAME:
            rValue <<= m_aModelName;
            break;
        case PROPERTY_ID_LABEL:
            rValue <<= m_aLabel;
            break;
        case PROPERTY_ID_WIDTH:
            rValue = m_aWidth;
            break;
        case PROPERTY_ID_ALIGN:
            rValue = m_aAlign;
            break;
        case PROPERTY_ID_HIDDEN:
            rValue = m_aHidden;
            break;
        default:
            OPropertySetAggregationHelper::getFastPropertyValue(rValue, nHandle);
    }
}

//------------------------------------------------------------------------------
sal_Bool OGridColumn::convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue,
                                            sal_Int32 nHandle, const Any& rValue )throw( IllegalArgumentException )
{
    sal_Bool bModified(sal_False);
    switch (nHandle)
    {
        case PROPERTY_ID_LABEL:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aLabel);
            break;
        case PROPERTY_ID_WIDTH:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aWidth, ::getCppuType((const sal_Int32*)NULL));
            break;
        case PROPERTY_ID_ALIGN:
            try
            {
                bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aAlign, ::getCppuType((const sal_Int16*)NULL));
            }
            catch(starlang::IllegalArgumentException&)
            {
                OSL_ENSURE(0,"OGridColumn::convertFastPropertyValue: TextAlign must be casted to sal_Int16!");
                throw;
            }
            break;
        case PROPERTY_ID_HIDDEN:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, getBOOL(m_aHidden));
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OGridColumn::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue )
{
    switch (nHandle)
    {
        case PROPERTY_ID_LABEL:
            DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_STRING, "invalid type" );
            rValue >>= m_aLabel;
            break;
        case PROPERTY_ID_WIDTH:
            m_aWidth = rValue;
            break;
        case PROPERTY_ID_ALIGN:
            m_aAlign = rValue;
            break;
        case PROPERTY_ID_HIDDEN:
            m_aHidden = rValue;
            break;
    }
}


// XPropertyState
//------------------------------------------------------------------------------
PropertyState OGridColumn::getPropertyStateByHandle(sal_Int32 nHandle)
{
    PropertyState eState;
    switch (nHandle)
    {
        case PROPERTY_ID_WIDTH:
            if (!m_aWidth.hasValue())
                eState = PropertyState_DEFAULT_VALUE;
            else
                eState = PropertyState_DIRECT_VALUE;
            break;
        case PROPERTY_ID_ALIGN:
            if (!m_aAlign.hasValue())
                eState = PropertyState_DEFAULT_VALUE;
            else
                eState = PropertyState_DIRECT_VALUE;
            break;
        default:
            eState = OPropertySetAggregationHelper::getPropertyStateByHandle(nHandle);
    }
    return eState;
}

//------------------------------------------------------------------------------
void OGridColumn::setPropertyToDefaultByHandle(sal_Int32 nHandle)
{
    switch (nHandle)
    {
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_ALIGN:
            setFastPropertyValue(nHandle, Any());
            break;
        case PROPERTY_ID_HIDDEN:
            setFastPropertyValue(nHandle, makeAny((sal_Bool)sal_True));
            break;
        default:
            OPropertySetAggregationHelper::setPropertyToDefaultByHandle(nHandle);
    }
}

//------------------------------------------------------------------------------
Any OGridColumn::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_ALIGN:
            return Any();
        case PROPERTY_ID_HIDDEN:
            return makeAny((sal_Bool)sal_False);
        default:
            return OPropertySetAggregationHelper::getPropertyDefaultByHandle(nHandle);
    }
}

//XPersistObject
//------------------------------------------------------------------------------
void SAL_CALL OGridColumn::write(const Reference<XObjectOutputStream>& _rxOutStream)
{
    // 1. Schreiben des UnoControls
    Reference<XMarkableStream>  xMark(_rxOutStream, UNO_QUERY);
    sal_Int32 nMark = xMark->createMark();

    sal_Int32 nLen = 0;
    _rxOutStream->writeLong(nLen);

    Reference<XPersistObject>  xPersist;
    if (query_aggregation(m_xAggregate, xPersist))
        xPersist->write(_rxOutStream);

    // feststellen der Laenge
    nLen = xMark->offsetToMark(nMark) - 4;
    xMark->jumpToMark(nMark);
    _rxOutStream->writeLong(nLen);
    xMark->jumpToFurthest();
    xMark->deleteMark(nMark);

    // 2. Schreiben einer VersionsNummer
    _rxOutStream->writeShort(0x0002);

    sal_uInt16 nAnyMask = 0;
    if (m_aWidth.getValueType().getTypeClass() == TypeClass_LONG)
        nAnyMask |= WIDTH;

    if (m_aAlign.getValueTypeClass() == TypeClass_SHORT)
        nAnyMask |= ALIGN;

    nAnyMask |= COMPATIBLE_HIDDEN;

    _rxOutStream->writeShort(nAnyMask);

    if (nAnyMask & WIDTH)
        _rxOutStream->writeLong(getINT32(m_aWidth));
    if (nAnyMask & ALIGN)
        _rxOutStream->writeShort(getINT16(m_aAlign));

    // Name
    _rxOutStream << m_aLabel;

    // the new place for the hidden flag : after m_aLabel, so older office version read the correct label, too
    if (nAnyMask & COMPATIBLE_HIDDEN)
        _rxOutStream->writeBoolean(getBOOL(m_aHidden));
}

//------------------------------------------------------------------------------
void SAL_CALL OGridColumn::read(const Reference<XObjectInputStream>& _rxInStream)
{
    // 1. Lesen des UnoControls
    sal_Int32 nLen = _rxInStream->readLong();
    if (nLen)
    {
        Reference<XMarkableStream>  xMark(_rxInStream, UNO_QUERY);
        sal_Int32 nMark = xMark->createMark();
        Reference<XPersistObject>  xPersist;
        if (query_aggregation(m_xAggregate, xPersist))
            xPersist->read(_rxInStream);

        xMark->jumpToMark(nMark);
        _rxInStream->skipBytes(nLen);
        xMark->deleteMark(nMark);
    }

    // 2. Lesen des Versionsnummer
    sal_uInt16 nVersion = _rxInStream->readShort();
    sal_uInt16 nAnyMask = _rxInStream->readShort();

    if (nAnyMask & WIDTH)
    {
        sal_Int32 nValue = _rxInStream->readLong();
        m_aWidth <<= (sal_Int32)nValue;
    }

    if (nAnyMask & ALIGN)
    {
        sal_Int16 nValue = _rxInStream->readShort();
        m_aAlign <<= nValue;
    }
    if (nAnyMask & OLD_HIDDEN)
    {
        sal_Bool bValue = _rxInStream->readBoolean();
        m_aHidden <<= (sal_Bool)bValue;
    }

    // Name
    _rxInStream >> m_aLabel;

    if (nAnyMask & COMPATIBLE_HIDDEN)
    {
        sal_Bool bValue = _rxInStream->readBoolean();
        m_aHidden <<= (sal_Bool)bValue;
    }
}

//------------------------------------------------------------------------------
IMPL_COLUMN(TextFieldColumn,        FRM_SUN_COMPONENT_TEXTFIELD,        sal_False);
IMPL_COLUMN(PatternFieldColumn,     FRM_SUN_COMPONENT_PATTERNFIELD,     sal_False);
IMPL_COLUMN(DateFieldColumn,        FRM_SUN_COMPONENT_DATEFIELD,        sal_True);
IMPL_COLUMN(TimeFieldColumn,        FRM_SUN_COMPONENT_TIMEFIELD,        sal_False);
IMPL_COLUMN(NumericFieldColumn,     FRM_SUN_COMPONENT_NUMERICFIELD,     sal_False);
IMPL_COLUMN(CurrencyFieldColumn,    FRM_SUN_COMPONENT_CURRENCYFIELD,    sal_False);
IMPL_COLUMN(CheckBoxColumn,         FRM_SUN_COMPONENT_CHECKBOX,         sal_False);
IMPL_COLUMN(ComboBoxColumn,         FRM_SUN_COMPONENT_COMBOBOX,         sal_False);
IMPL_COLUMN(ListBoxColumn,          FRM_SUN_COMPONENT_LISTBOX,          sal_False);
IMPL_COLUMN(FormattedFieldColumn,   FRM_SUN_COMPONENT_FORMATTEDFIELD,   sal_False);

//.........................................................................
}   // namespace frm
//.........................................................................

