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
#include <com/sun/star/beans/PropertyAttribute.hpp>
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
    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

    //==============================================================================
    //= OColumnSettings
    //==============================================================================
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
    void OColumnSettings::registerProperties( IPropertyContainer& _rPropertyContainer )
    {
        const sal_Int32 nBoundAttr = PropertyAttribute::BOUND;
        const sal_Int32 nMayBeVoidAttr = PropertyAttribute::MAYBEVOID | nBoundAttr;

        const Type& rSalInt32Type = ::getCppuType( static_cast< sal_Int32* >( NULL ) );
        const Type& rStringType = ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) );

        _rPropertyContainer.registerMayBeVoidProperty( PROPERTY_ALIGN, PROPERTY_ID_ALIGN, nMayBeVoidAttr, &m_aAlignment, rSalInt32Type );
        _rPropertyContainer.registerMayBeVoidProperty( PROPERTY_NUMBERFORMAT, PROPERTY_ID_NUMBERFORMAT, nMayBeVoidAttr, &m_aFormatKey, rSalInt32Type );
        _rPropertyContainer.registerMayBeVoidProperty( PROPERTY_RELATIVEPOSITION, PROPERTY_ID_RELATIVEPOSITION, nMayBeVoidAttr, &m_aRelativePosition, rSalInt32Type );
        _rPropertyContainer.registerMayBeVoidProperty( PROPERTY_WIDTH, PROPERTY_ID_WIDTH, nMayBeVoidAttr, &m_aWidth, rSalInt32Type );
        _rPropertyContainer.registerMayBeVoidProperty( PROPERTY_HELPTEXT, PROPERTY_ID_HELPTEXT, nMayBeVoidAttr, &m_aHelpText, rStringType );
        _rPropertyContainer.registerMayBeVoidProperty( PROPERTY_CONTROLDEFAULT, PROPERTY_ID_CONTROLDEFAULT, nMayBeVoidAttr, &m_aControlDefault, rStringType );
        _rPropertyContainer.registerProperty( PROPERTY_CONTROLMODEL, PROPERTY_ID_CONTROLMODEL, nBoundAttr, &m_xControlModel, ::getCppuType( &m_xControlModel ) );
        _rPropertyContainer.registerProperty( PROPERTY_HIDDEN, PROPERTY_ID_HIDDEN, nBoundAttr, &m_bHidden, ::getCppuType( &m_bHidden ) );
    }

    //------------------------------------------------------------------------------
    bool OColumnSettings::isMine( const sal_Int32 _nPropertyHandle ) const
    {
        return  ( _nPropertyHandle == PROPERTY_ID_ALIGN )
            ||  ( _nPropertyHandle == PROPERTY_ID_NUMBERFORMAT )
            ||  ( _nPropertyHandle == PROPERTY_ID_RELATIVEPOSITION )
            ||  ( _nPropertyHandle == PROPERTY_ID_WIDTH )
            ||  ( _nPropertyHandle == PROPERTY_ID_HELPTEXT )
            ||  ( _nPropertyHandle == PROPERTY_ID_CONTROLDEFAULT )
            ||  ( _nPropertyHandle == PROPERTY_ID_CONTROLMODEL )
            ||  ( _nPropertyHandle == PROPERTY_ID_HIDDEN );
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
