/*************************************************************************
 *
 *  $RCSfile: column.cxx,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:02:56 $
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

#ifndef _DBA_COREAPI_COLUMN_HXX_
#include "column.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COMPHELPER_SEQSTREAM_HXX
#include <comphelper/seqstream.hxx>
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include <connectivity/sdbcx/VColumn.hxx>
#endif
#include <connectivity/TTableHelper.hxx>
#ifndef _DBACORE_DEFINITIONCOLUMN_HXX_
#include "definitioncolumn.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif

#include <algorithm>

using namespace dbaccess;
using namespace connectivity;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;
using namespace ::utl;

DBG_NAME(OColumn);

#define COLUMN_STREAM_SIGNATURE ::rtl::OUString::createFromAscii("Columns")

//============================================================
//= OColumn
//============================================================
//--------------------------------------------------------------------------
OColumn::OColumn()
        :OColumnBase(m_aMutex)
        , OPropertySetHelper(OColumnBase::rBHelper)
{
    DBG_CTOR(OColumn, NULL);
}

//--------------------------------------------------------------------------
OColumn::~OColumn()
{
    DBG_DTOR(OColumn, NULL);
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OColumn::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XPropertySet > *)0 ),
                           ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                           OColumnBase::getTypes());
    return aTypes.getTypes();
}

// com::sun::star::uno::XInterface
//--------------------------------------------------------------------------
Any OColumn::queryInterface( const Type & rType ) throw (RuntimeException)
{
    Any aIface = OColumnBase::queryInterface( rType );
    if (!aIface.hasValue())
        aIface = ::cppu::queryInterface(
                    rType,
                    static_cast< XPropertySet * >( this ),
                    static_cast< XMultiPropertySet * >( this ));

    return aIface;
}

//--------------------------------------------------------------------------
void OColumn::acquire() throw()
{
    OColumnBase::acquire();
}

//--------------------------------------------------------------------------
void OColumn::release() throw()
{
    OColumnBase::release();
}

// ::com::sun::star::lang::XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OColumn::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OColumn");
}

//------------------------------------------------------------------------------
sal_Bool OColumn::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OColumn::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDBCX_COLUMN;
    return aSNS;
}

// OComponentHelper
//------------------------------------------------------------------------------
void OColumn::disposing()
{
    OPropertySetHelper::disposing();
}

// com::sun::star::beans::XPropertySet
//------------------------------------------------------------------------------
Reference< XPropertySetInfo > OColumn::getPropertySetInfo() throw (RuntimeException)
{
    return createPropertySetInfo( getInfoHelper() ) ;
}

//------------------------------------------------------------------------------
void OColumn::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            rValue <<= m_sName;
            break;
    }
}

//------------------------------------------------------------------------------
sal_Bool OColumn::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            bModified = ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sName);
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OColumn::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_NAME:
            OSL_ENSURE(rValue.getValueType().equals(::getCppuType(static_cast< ::rtl::OUString* >(NULL))),
                "OColumn::setFastPropertyValue_NoBroadcast(NAME) : invalid value !");
            rValue >>= m_sName;
            break;
    }
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OColumn::getUnoTunnelImplementationId()
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

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OColumn::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return 0;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OColumn::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_sName;
}
// -----------------------------------------------------------------------------
void SAL_CALL OColumn::setName( const ::rtl::OUString& _rName ) throw(::com::sun::star::uno::RuntimeException)
{
    m_sName = _rName;
}
// -----------------------------------------------------------------------------
OColumnSettings*    OColumn::getSettings()
{
    return NULL;
}
// -----------------------------------------------------------------------------
void OColumn::fireValueChange(const ::connectivity::ORowSetValue& _rOldValue)
{
}
// -----------------------------------------------------------------------------
//============================================================
//= OColumnSettings
//============================================================
DBG_NAME( OColumnSettings )
//------------------------------------------------------------------------------
OColumnSettings::OColumnSettings()
    :m_bHidden(sal_False)
{
    DBG_CTOR( OColumnSettings, NULL );
}

//------------------------------------------------------------------------------
OColumnSettings::~OColumnSettings()
{
    DBG_DTOR( OColumnSettings, NULL );
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
sal_Bool OColumnSettings::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (IllegalArgumentException)
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
            if ( bModified = !::comphelper::compare(rValue,m_aControlDefault) )
            {
                rConvertedValue = rValue;
                rOldValue = m_aControlDefault;
            }
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OColumnSettings::setFastPropertyValue_NoBroadcast(
                                            sal_Int32 nHandle,
                                            const Any& rValue
                                                 )
                                                 throw (Exception)
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
            m_aWidth = rValue;
            break;
        case PROPERTY_ID_HIDDEN:
            OSL_ENSURE(rValue.getValueType().equals(::getBooleanCppuType()),
                "OColumnSettings::setFastPropertyValue_NoBroadcast(HIDDEN) : invalid value !");
            m_bHidden = ::comphelper::getBOOL(rValue);
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

//------------------------------------------------------------------------------
sal_Bool OColumnSettings::writeUITo(const OConfigurationNode& _rConfigNode, const Reference< XNumberFormatsSupplier >& _rxFormats)
{
    OSL_ENSURE( _rxFormats.is(), "OColumnSettings::writeUITo: invalid (insufficient) context!" );

    // the plain  stuff
    _rConfigNode.setNodeValue(CONFIGKEY_COLUMN_ALIGNMENT, m_aAlignment);
    _rConfigNode.setNodeValue(CONFIGKEY_COLUMN_WIDTH, m_aWidth);
    _rConfigNode.setNodeValue(CONFIGKEY_COLUMN_RELPOSITION, m_aRelativePosition);
    _rConfigNode.setNodeValue(CONFIGKEY_COLUMN_HIDDEN, ::cppu::bool2any(m_bHidden));

    // for the format key:
    Any aPersistentFormatKey = m_aFormatKey;
    Any aPersistentFormatString;
    Any aPersistentFomatLocale;
    // first, check if our format is a standard one
    try
    {
        if ( m_aFormatKey.hasValue() && _rxFormats.is() )
        {
            // we have a non-NULL format

            // extract it
            sal_Int32 nFormatKey = 0;
#if OSL_DEBUG_LEVEL > 0
            sal_Bool bSuccess =
#endif
            m_aFormatKey >>= nFormatKey;
            OSL_ENSURE( bSuccess, "OColumnSettings::writeUITo: could not extract the format key!" );

            // get the UNO descriptor for the format
            Reference< XNumberFormats > xFormats = _rxFormats->getNumberFormats();
            Reference< XPropertySet > xKeyDescriptor;
            if ( xFormats.is() )
                xKeyDescriptor = xFormats->getByKey( nFormatKey );
            OSL_ENSURE( xKeyDescriptor.is(), "OColumnSettings::writeUITo: invalid format key!" );

            // is this a user-defined format?
            sal_Bool bUserDefinedFormat = sal_False;
            if ( xKeyDescriptor.is() )
                bUserDefinedFormat = ::cppu::any2bool( xKeyDescriptor->getPropertyValue( ::rtl::OUString::createFromAscii( "UserDefined" ) ) );

            if ( bUserDefinedFormat )
            {   // yest
                // -> obtain format string and locale
                ::rtl::OUString sFormatString;
                Locale aFormatLocale;
                xKeyDescriptor->getPropertyValue( ::rtl::OUString::createFromAscii( "FormatString" ) ) >>= sFormatString;
                xKeyDescriptor->getPropertyValue( ::rtl::OUString::createFromAscii( "Locale" ) ) >>= aFormatLocale;

                OSL_ENSURE( sFormatString.getLength(), "OColumnSettings::writeUITo: invalid format string!" );
                OSL_ENSURE( aFormatLocale.Language.getLength(), "OColumnSettings::writeUITo: invalid locale!" );

                // concat the language/Country of the locale
                ::rtl::OUString sLocaleString = aFormatLocale.Language;
                if ( aFormatLocale.Country.getLength() )
                {
                    sal_Unicode cSeparator = '-';
                    sLocaleString += ::rtl::OUString( &cSeparator, 1 );
                    sLocaleString += aFormatLocale.Country;
                }

                aPersistentFormatString <<= sFormatString;
                aPersistentFomatLocale <<= sLocaleString;
                // now that we know that we save the format as descriptor (string/language), reset the key
                aPersistentFormatKey.clear();
            }
        }
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "OColumnSettings::writeUITo: caught an exception while examining the format!" );
    }

    _rConfigNode.setNodeValue( CONFIGKEY_COLUMN_NUMBERFORMAT, aPersistentFormatKey );
    _rConfigNode.setNodeValue( CONFIGKEY_FORMATSTRING, aPersistentFormatString );
    _rConfigNode.setNodeValue( CONFIGKEY_FORMATLOCALE, aPersistentFomatLocale );

    _rConfigNode.setNodeValue( CONFIGKEY_COLUMN_HELPTEXT,       m_aHelpText );
    _rConfigNode.setNodeValue( CONFIGKEY_COLUMN_CONTROLDEFAULT, m_aControlDefault );

    return sal_True;
}

//------------------------------------------------------------------------------
void OColumnSettings::readUIFrom(const OConfigurationNode& _rConfigNode, const Reference< XNumberFormatsSupplier >& _rxFormats)
{
    OSL_ENSURE( _rxFormats.is(), "OColumnSettings::readUIFrom: invalid (insufficient) context!" );

    // some defaults
    m_bHidden = sal_False;
    m_aRelativePosition.clear();
    m_aFormatKey.clear();
    m_aWidth.clear();
    m_aAlignment.clear();
    m_aHelpText.clear();
    m_aControlDefault.clear();

    m_aAlignment        = _rConfigNode.getNodeValue(CONFIGKEY_COLUMN_ALIGNMENT);
    m_aWidth            = _rConfigNode.getNodeValue(CONFIGKEY_COLUMN_WIDTH);
    m_aRelativePosition = _rConfigNode.getNodeValue(CONFIGKEY_COLUMN_RELPOSITION);
    m_bHidden           = ::cppu::any2bool(_rConfigNode.getNodeValue(CONFIGKEY_COLUMN_HIDDEN));
    m_aHelpText         = _rConfigNode.getNodeValue(CONFIGKEY_COLUMN_HELPTEXT);
    m_aControlDefault   = _rConfigNode.getNodeValue(CONFIGKEY_COLUMN_CONTROLDEFAULT);

    // the format key is somewhat more complicated
    m_aFormatKey        = _rConfigNode.getNodeValue( CONFIGKEY_COLUMN_NUMBERFORMAT );
    if ( !m_aFormatKey.hasValue() && _rxFormats.is() )
    {
        Any aPersistentFormatString = _rConfigNode.getNodeValue( CONFIGKEY_FORMATSTRING );
        Any aPersistentFormatLocale = _rConfigNode.getNodeValue( CONFIGKEY_FORMATLOCALE );

        if ( aPersistentFormatString.hasValue() && aPersistentFormatLocale.hasValue() )
        {   // okay, the format key is void because the format was user defined, and a format descriptor
            // (string/locale) has been stored.

            OSL_ENSURE( aPersistentFormatString.getValueTypeClass() == TypeClass_STRING
                    &&  aPersistentFormatLocale.getValueTypeClass() == TypeClass_STRING,
                    "OColumnSettings::readUIFrom: invalid format descriptor!" );
            // the string
            ::rtl::OUString sFormatString; aPersistentFormatString >>= sFormatString;
            // the locale
            ::rtl::OUString sLocale; aPersistentFormatLocale >>= sLocale;
            // split the parts of the locale
            Locale aLocale;
            sal_Int32 nSeparatorPos = sLocale.indexOf( '-' );
            if ( 0 <= nSeparatorPos )
            {
                aLocale.Language = sLocale.copy( 0, nSeparatorPos );
                aLocale.Country = sLocale.copy( nSeparatorPos + 1 );
            }
            else
                aLocale.Language = sLocale;

            try
            {
                // check if the number formatter already knows this format
                Reference< XNumberFormats > xFormats( _rxFormats->getNumberFormats() );
                OSL_ENSURE( xFormats.is(), "OColumnSettings::readUIFrom: invalid number formats supplier!" );
                sal_Int32 nFormatKey = 0;
                if ( xFormats.is() )
                {
                    nFormatKey = xFormats->queryKey( sFormatString, aLocale, sal_False );
                    if ( -1 == nFormatKey )
                        nFormatKey = xFormats->addNew( sFormatString, aLocale );
                    OSL_ENSURE( -1 != nFormatKey, "OColumnSettings::readUIFrom: could not add the format!" );
                    // normalize in case something went wrong
                    if ( -1 == nFormatKey )
                        nFormatKey = 0;
                }
                m_aFormatKey <<= nFormatKey;
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "OColumnSettings::readUIFrom: caught an exception while creating the user defined format!" );
            }
        }
    }
}

//============================================================
//= OColumns
//============================================================
DBG_NAME(OColumns);

//--------------------------------------------------------------------------
OColumns::OColumns(::cppu::OWeakObject& _rParent,
                   ::osl::Mutex& _rMutex,
                   sal_Bool _bCaseSensitive,const ::std::vector< ::rtl::OUString> &_rVector,
                   IColumnFactory* _pColFactory,
                   ::connectivity::sdbcx::IRefreshableColumns* _pRefresh,
                   sal_Bool _bAddColumn,sal_Bool _bDropColumn)
                   : OColumns_BASE(_rParent,_bCaseSensitive,_rMutex,_rVector)
    ,m_bInitialized(sal_False)
    ,m_bAddColumn(_bAddColumn)
    ,m_bDropColumn(_bDropColumn)
    ,m_xDrvColumns(NULL)
    ,m_pColFactoryImpl(_pColFactory)
    ,m_pRefreshColumns(_pRefresh)
{
    DBG_CTOR(OColumns, NULL);
}
// -------------------------------------------------------------------------
OColumns::OColumns(::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxDrvColumns,
        sal_Bool _bCaseSensitive,const ::std::vector< ::rtl::OUString> &_rVector,
        IColumnFactory* _pColFactory,
        ::connectivity::sdbcx::IRefreshableColumns* _pRefresh,
        sal_Bool _bAddColumn,sal_Bool _bDropColumn)
       : OColumns_BASE(_rParent,_bCaseSensitive,_rMutex,_rVector)
    ,m_bInitialized(sal_False)
    ,m_bAddColumn(_bAddColumn)
    ,m_bDropColumn(_bDropColumn)
    ,m_xDrvColumns(_rxDrvColumns)
    ,m_pColFactoryImpl(_pColFactory)
    ,m_pRefreshColumns(_pRefresh)
{
    DBG_CTOR(OColumns, NULL);
}
//--------------------------------------------------------------------------
OColumns::~OColumns()
{
    clearColumnSettings();

    DBG_DTOR(OColumns, NULL);
}

// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OColumns::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.OColumns");
}

//------------------------------------------------------------------------------
sal_Bool OColumns::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}

//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OColumns::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDBCX_CONTAINER;
    return aSNS;
}

//------------------------------------------------------------------
struct DeleteColumnSettings : ::std::unary_function< MapName2Settings::value_type, void >
{
    void operator()( const MapName2Settings::value_type& _rMapElement ) const
    {
        delete _rMapElement.second;
    }
};

//------------------------------------------------------------------
void OColumns::append( const ::rtl::OUString& _rName, OColumn* _pColumn )
{
    MutexGuard aGuard(m_rMutex);

    OSL_ENSURE( _pColumn, "OColumns::append: invalid column!" );
    OSL_ENSURE( 0 == m_aNameMap.count( _rName ),"OColumns::append: Column already exists");

    _pColumn->m_sName = _rName;

    // do we have settings for this column?
    MapName2Settings::iterator aExistentSettings = m_aSettings.find( _rName );
    if ( m_aSettings.end() != aExistentSettings )
    {   // yes, we do
        // merge the settings into the columns
        OColumnSettings* pColSettings = _pColumn->getSettings();
        if ( pColSettings )
            *pColSettings = *aExistentSettings->second;

        // and remove the settings
        DeleteColumnSettings()( *aExistentSettings );
        m_aSettings.erase( aExistentSettings );
    }

    // now really insert the column
    insertElement( _rName, _pColumn );
}

//------------------------------------------------------------------
void OColumns::clearColumns()
{
    MutexGuard aGuard(m_rMutex);
    disposing();
}

// -----------------------------------------------------------------------------
void SAL_CALL OColumns::disposing(void)
{
    MutexGuard aGuard(m_rMutex);
    m_xDrvColumns = NULL;
    OColumns_BASE::disposing();
}

//------------------------------------------------------------------------------
void OColumns::clearColumnSettings()
{
    ::std::for_each( m_aSettings.begin(), m_aSettings.end(), DeleteColumnSettings() );
    m_aSettings.clear();
    MapName2Settings(m_aSettings).swap(m_aSettings);
}

//------------------------------------------------------------------------------
void OColumns::loadSettings( const OConfigurationNode& _rLocation, const Reference< XNumberFormatsSupplier >& _rxNumberFormats )
{
    MutexGuard aGuard(m_rMutex);

    OConfigurationNode aLocation(_rLocation);
    aLocation.setEscape(aLocation.isSetNode());

    OSL_ENSURE(m_pColFactoryImpl, "OColumns::loadSettings: need a factory to create columns!");

    // empty our remembered settings
    clearColumnSettings();

    Sequence< ::rtl::OUString > aColumNames = aLocation.getNodeNames();
    const ::rtl::OUString* pColumNames = aColumNames.getConstArray();
    for (sal_Int32 i=0; i<aColumNames.getLength(); ++i, ++pColumNames)
    {
        OColumnSettings* pSettings = NULL;

        // do we already have a column with that name ?
        // create the column if neccessary
        if ( !hasByName( *pColumNames ) )
        {
            // create a new object to hold the settings
            // later on, they will be merged into the real column if it get's inserted
            pSettings = new OColumnSettings;
            OSL_ENSURE( m_aSettings.end() == m_aSettings.find( *pColumNames ),
                "OColumns::loadSettings: already have settings for this name!" );
            m_aSettings.insert( MapName2Settings::value_type( *pColumNames, pSettings ) );
        }
        else
        {
            Reference< XUnoTunnel > xTunnel;
            getByName( *pColumNames ) >>= xTunnel;
            if ( xTunnel.is() )
            {
                OColumn* pExistent = reinterpret_cast< OColumn* >( xTunnel->getSomething( OColumn::getUnoTunnelImplementationId() ) );
                if ( pExistent )
                    pSettings = pExistent->getSettings();
            }
        }

        OSL_ENSURE( pSettings, "OColumns::loadSettings: no object which is able to hold the settings!" );
        if ( pSettings )
        {
            OConfigurationNode aCurrent = aLocation.openNode( *pColumNames );
            pSettings->readUIFrom( aCurrent, _rxNumberFormats );
        }
    }
}

//------------------------------------------------------------------------------
void OColumns::storeSettings( const OConfigurationNode& _rLocation, const Reference< XNumberFormatsSupplier >& _rxNumberFormats )
{
    MutexGuard aGuard(m_rMutex);
    if (!_rLocation.isValid())
    {
        OSL_ENSURE(sal_False, "OColumns::storeSettings: have no location !");
        return;
    }
    if (_rLocation.isReadonly())
    {
        OSL_ENSURE(sal_False, "OColumns::storeSettings: the location is read-only !");
        return;
    }

    DECLARE_STL_USTRINGACCESS_MAP(OConfigurationNode, MapName2Node);
    MapName2Node aObjectKeys;

    // collect the sub keys of existent column descriptions
    Sequence< ::rtl::OUString > aColumNames = _rLocation.getNodeNames();
    const ::rtl::OUString* pColumNames = aColumNames.getConstArray();
    for (sal_Int32 i=0; i<aColumNames.getLength(); ++i, ++pColumNames)
        aObjectKeys[*pColumNames] = _rLocation.openNode(*pColumNames);

    // now write all descriptions of my columns
    OColumn* pCurrent = NULL;
    ::rtl::OUString sCurrent;
    for (   ::std::vector< ObjectIter >::const_iterator aIter = m_aElements.begin();
            aIter != m_aElements.end();
            ++aIter
        )
    {
        // set the name
        Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel((*aIter)->second.get(),UNO_QUERY);
        if(xTunnel.is())
        {
            pCurrent = (OColumn*)xTunnel->getSomething(OColumn::getUnoTunnelImplementationId());

            OSL_ENSURE(pCurrent,"OColumns::storeSettings: No column from unotunnelhelper!");

            OColumnSettings* pCurrentSettings = pCurrent->getSettings();
            if (!pCurrentSettings)
            {
                OSL_ENSURE(sal_False, "OColumns::storeSettings: can't write column without settings!");
                continue;
            }
            sCurrent = pCurrent->m_sName;

            OConfigurationNode aColumnNode;
            // do we we have an existent key for that column ?
            ConstMapName2NodeIterator aExistentObjectKey = aObjectKeys.find(sCurrent);
            if (aExistentObjectKey != aObjectKeys.end())
            {
                aColumnNode = aExistentObjectKey->second;
                // these sub key is used (and must not be deleted afterwards)
                // -> remove from the key maps
                aObjectKeys.erase(sCurrent);
            }
            else
            {   // no -> create one

                if (pCurrentSettings->isDefaulted())
                    // no need to write the configuration data: it's all in it's default state
                    continue;

                aColumnNode = _rLocation.createNode(sCurrent);
                if (!aColumnNode.isValid())
                {
                    OSL_ENSURE(sal_False, "OColumns::storeSettings: could not create the structures for writing a column !");
                    continue;
                }
            }

            // let the column write itself
            pCurrentSettings->writeUITo( aColumnNode, _rxNumberFormats );
        }
    }

    // delete all description keys where we have no columns for
    for (   ConstMapName2NodeIterator   aRemove = aObjectKeys.begin();
            aRemove != aObjectKeys.end();
            ++aRemove
        )
    {
        // the configuration does not support different types of operations in one transaction, so we must commit
        // before and after we create the new node, to ensure, that every transaction we ever do contains only
        // one type of operation (insert, remove, update)
        _rLocation.removeNode(aRemove->first);
    }
}

// -------------------------------------------------------------------------
void OColumns::impl_refresh() throw(::com::sun::star::uno::RuntimeException)
{
    if (m_pRefreshColumns)
        m_pRefreshColumns->refreshColumns();
}

// -------------------------------------------------------------------------
Reference< XNamed > OColumns::createObject(const ::rtl::OUString& _rName)
{
    OSL_ENSURE(m_pColFactoryImpl, "OColumns::createObject: no column factory!");

    if (m_pColFactoryImpl)
        return m_pColFactoryImpl->createColumn(_rName);
    else
        return Reference< XNamed >();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OColumns::createEmptyObject()
{
    if (m_pColFactoryImpl)
        return m_pColFactoryImpl->createEmptyObject();
    else
        return Reference< XPropertySet >();
}
// -------------------------------------------------------------------------
Any SAL_CALL OColumns::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet;
    if(m_xDrvColumns.is())
    {
        aRet = m_xDrvColumns->queryInterface(rType);
        if(aRet.hasValue())
            aRet = OColumns_BASE::queryInterface( rType);
    }
    else if(!m_pTable || (m_pTable && !m_pTable->isNew()))
    {
        if(!m_bAddColumn    && rType == getCppuType( (Reference<XAppend>*)0))
            return Any();
        if(!m_bDropColumn   && rType == getCppuType( (Reference<XDrop>*)0))
            return Any();
    }

    return OColumns_BASE::queryInterface( rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OColumns::getTypes(  ) throw(RuntimeException)
{
    sal_Bool bAppendFound = sal_False,bDropFound = sal_False;

    sal_Int32 nSize = 0;
    Type aAppendType = getCppuType( (Reference<XAppend>*)0);
    Type aDropType   = getCppuType( (Reference<XDrop>*)0);
    if(m_xDrvColumns.is())
    {
        Reference<XTypeProvider> xTypes(m_xDrvColumns,UNO_QUERY);
        Sequence< Type > aTypes(xTypes->getTypes());

        Sequence< Type > aSecTypes(OColumns_BASE::getTypes());


        const Type* pBegin = aTypes.getConstArray();
        const Type* pEnd = pBegin + aTypes.getLength();
        for (;pBegin != pEnd ; ++pBegin)
        {
            if(aAppendType == *pBegin)
                bAppendFound = sal_True;
            else if(aDropType == *pBegin)
                bDropFound = sal_True;
        }
        nSize = (bDropFound ? (bAppendFound ? 0 : 1) : (bAppendFound ? 1 : 2));
    }
    else
    {
        nSize = ((m_pTable && m_pTable->isNew()) ? 0 :
                    ((m_bDropColumn ?
                        (m_bAddColumn ? 0 : 1) : (m_bAddColumn ? 1 : 2))));
        bDropFound      = (m_pTable && m_pTable->isNew()) || m_bDropColumn;
        bAppendFound    = (m_pTable && m_pTable->isNew()) || m_bAddColumn;
    }
    Sequence< Type > aTypes(OColumns_BASE::getTypes());
    Sequence< Type > aRet(aTypes.getLength() - nSize);

    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    for(sal_Int32 i=0;pBegin != pEnd ;++pBegin)
    {
        if(*pBegin != aAppendType && *pBegin != aDropType)
            aRet.getArray()[i++] = *pBegin;
        else if(bDropFound && *pBegin == aDropType)
            aRet.getArray()[i++] = *pBegin;
        else if(bAppendFound && *pBegin == aAppendType)
            aRet.getArray()[i++] = *pBegin;
    }
    return aRet;
}
// -------------------------------------------------------------------------
// XAppend
void OColumns::appendObject( const Reference< XPropertySet >& descriptor )
{
    Reference<XAppend> xAppend(m_xDrvColumns,UNO_QUERY);
    if(xAppend.is())
    {
        xAppend->appendByDescriptor(descriptor);
    }
    else if(m_pTable && !m_pTable->isNew() && m_bAddColumn)
    {
        OColumns_BASE::appendObject(descriptor);
    }
    else if(m_pTable && !m_pTable->isNew() && !m_bAddColumn)
        throw SQLException();
}
// -------------------------------------------------------------------------
// XDrop
void OColumns::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    Reference<XDrop> xDrop(m_xDrvColumns,UNO_QUERY);
    if(xDrop.is())
    {
        xDrop->dropByName(_sElementName);
    }
    else if(m_pTable && !m_pTable->isNew() && m_bDropColumn)
    {
        OColumns_BASE::dropObject(_nPos,_sElementName);
    }
    else if(m_pTable && !m_pTable->isNew() && !m_bDropColumn)
        throw SQLException();
}
// -------------------------------------------------------------------------
Reference< XNamed > OColumns::cloneObject(const Reference< XPropertySet >& _xDescriptor)
{
    Reference<XPropertySet> xProp = createEmptyObject();
    Reference< XNamed > xName(xProp,UNO_QUERY);
    OSL_ENSURE(xName.is(),"Must be a XName interface here !");
    if(xProp.is())
        ::comphelper::copyProperties(_xDescriptor,xProp);
    return xName;
}
// -----------------------------------------------------------------------------


