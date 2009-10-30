/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "columnsettings.hxx"
#include "dbastrings.hrc"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <cppuhelper/typeprovider.hxx>
#include <comphelper/property.hxx>
#include <tools/debug.hxx>

//........................................................................
namespace dbaccess
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::lang::IllegalArgumentException;
    using ::com::sun::star::beans::XPropertySet;
    /** === end UNO using === **/

    //==============================================================================
    //= OColumnSettings
    //==============================================================================
    DBG_NAME( OColumnSettings )
    //------------------------------------------------------------------------------
    OColumnSettings::OColumnSettings()
        :m_bHidden(sal_False)
    {
        OSL_TRACE( "ColumnSettings: +%p", this );
        DBG_CTOR( OColumnSettings, NULL );
    }

    //------------------------------------------------------------------------------
    OColumnSettings::~OColumnSettings()
    {
        DBG_DTOR( OColumnSettings, NULL );
        OSL_TRACE( "ColumnSettings: -%p", this );
    }

    // XUnoTunnel
    //------------------------------------------------------------------
    sal_Int64 OColumnSettings::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
    {
           if  (  ( rId.getLength() == 16 )
            && ( 0 == rtl_compareMemory( getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
            )
                   return reinterpret_cast< sal_Int64 >( this );

           return 0;
    }

    //--------------------------------------------------------------------------
    Sequence< sal_Int8 > OColumnSettings::getUnoTunnelImplementationId()
    {
           static ::cppu::OImplementationId * pId = 0;
           if (! pId)
           {
                   ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
                   if (! pId)
                   {
                           static ::cppu::OImplementationId aId;
                           pId = &aId;
                   }
           }
           return pId->getImplementationId();
    }

    //------------------------------------------------------------------------------
    void OColumnSettings::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
    {
        switch (nHandle)
        {
            case PROPERTY_ID_ALIGN:
                rValue = m_aAlignment;
                break;
            case PROPERTY_ID_NUMBERFORMAT:
                rValue = m_aFormatKey;
                break;
            case PROPERTY_ID_RELATIVEPOSITION:
                rValue = m_aRelativePosition;
                break;
            case PROPERTY_ID_WIDTH:
                rValue = m_aWidth;
                break;
            case PROPERTY_ID_HIDDEN:
                rValue.setValue(&m_bHidden, getBooleanCppuType());
                break;
            case PROPERTY_ID_CONTROLMODEL:
                rValue <<= m_xControlModel;
                break;
            case PROPERTY_ID_HELPTEXT:
                rValue = m_aHelpText;
                break;
            case PROPERTY_ID_CONTROLDEFAULT:
                rValue = m_aControlDefault;
                break;
        }
    }

    //------------------------------------------------------------------------------
    sal_Bool OColumnSettings::convertFastPropertyValue( Any& rConvertedValue, Any & rOldValue, sal_Int32 nHandle,
                                                        const Any& rValue ) throw (IllegalArgumentException)
    {
        sal_Bool bModified = sal_False;
        switch (nHandle)
        {
            case PROPERTY_ID_ALIGN:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aAlignment,
                    ::getCppuType(static_cast< sal_Int32* >(NULL)));
                break;
            case PROPERTY_ID_WIDTH:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aWidth,
                    ::getCppuType(static_cast< sal_Int32* >(NULL)));
                break;
            case PROPERTY_ID_HIDDEN:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_bHidden);
                break;
            case PROPERTY_ID_RELATIVEPOSITION:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aRelativePosition,
                    ::getCppuType(static_cast< sal_Int32* >(NULL)));
                break;
            case PROPERTY_ID_NUMBERFORMAT:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aFormatKey,
                    ::getCppuType(static_cast< sal_Int32* >(NULL)));
                break;
            case PROPERTY_ID_CONTROLMODEL:
            {
                Reference< XPropertySet > xTest;
                if (!::cppu::extractInterface(xTest, rValue))
                    throw IllegalArgumentException();
                if (xTest.get() != m_xControlModel.get())
                {
                    bModified = sal_True;
                    rOldValue <<= m_xControlModel;
                    rConvertedValue <<= rValue;
                }
            }
            break;
            case PROPERTY_ID_HELPTEXT:
                bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aHelpText,
                    ::getCppuType(static_cast< ::rtl::OUString* >(NULL)));
                break;
            case PROPERTY_ID_CONTROLDEFAULT:
                bModified = rValue != m_aControlDefault;
                if ( bModified )
                {
                    rConvertedValue = rValue;
                    rOldValue = m_aControlDefault;
                }
                break;
        }
        return bModified;
    }

    //------------------------------------------------------------------------------
    void OColumnSettings::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (Exception)
    {
        switch (nHandle)
        {
            case PROPERTY_ID_ALIGN:
                OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                    "OColumnSettings::setFastPropertyValue_NoBroadcast(ALIGN) : invalid value !");
                m_aAlignment = rValue;
                break;
            case PROPERTY_ID_WIDTH:
                OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                    "OColumnSettings::setFastPropertyValue_NoBroadcast(WIDTH) : invalid value !");
                m_aWidth = rValue;
                break;
            case PROPERTY_ID_NUMBERFORMAT:
                OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                    "OColumnSettings::setFastPropertyValue_NoBroadcast(NUMBERFORMAT) : invalid value !");
                m_aFormatKey = rValue;
                break;
            case PROPERTY_ID_RELATIVEPOSITION:
                OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< sal_Int32* >(NULL))),
                    "OColumnSettings::setFastPropertyValue_NoBroadcast(ID_RELATIVEPOSITION) : invalid value !");
                m_aRelativePosition = rValue;
                break;
            case PROPERTY_ID_HIDDEN:
                OSL_ENSURE(rValue.getValueType().equals(::getBooleanCppuType()),
                    "OColumnSettings::setFastPropertyValue_NoBroadcast(HIDDEN) : invalid value !");
                OSL_VERIFY( rValue >>= m_bHidden );
                break;
            case PROPERTY_ID_HELPTEXT:
                OSL_ENSURE(!rValue.hasValue() || rValue.getValueType().equals(::getCppuType(static_cast< ::rtl::OUString* >(NULL))),
                    "OColumnSettings::setFastPropertyValue_NoBroadcast(ID_RELATIVEPOSITION) : invalid value !");
                m_aHelpText = rValue;
                break;
            case PROPERTY_ID_CONTROLDEFAULT:
                m_aControlDefault = rValue;
                break;
        }
    }

    //------------------------------------------------------------------------------
    sal_Bool OColumnSettings::isDefaulted() const
    {
        return  !m_aAlignment.hasValue()
            &&  !m_aWidth.hasValue()
            &&  !m_aFormatKey.hasValue()
            &&  !m_aRelativePosition.hasValue()
            &&  !m_aHelpText.hasValue()
            &&  !m_aControlDefault.hasValue()
            &&  !m_bHidden;
    }

//........................................................................
} // namespace dbaccess
//........................................................................
