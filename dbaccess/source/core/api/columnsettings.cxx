/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "columnsettings.hxx"
#include "stringconstants.hxx"
#include "strings.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <cppuhelper/typeprovider.hxx>
#include <comphelper/property.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

namespace dbaccess
{

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::XPropertySetInfo;

    namespace PropertyAttribute = ::com::sun::star::beans::PropertyAttribute;

    // OColumnSettings

    OColumnSettings::OColumnSettings()
        :m_bHidden(false)
    {
    }

    OColumnSettings::~OColumnSettings()
    {
    }

    void OColumnSettings::registerProperties( IPropertyContainer& _rPropertyContainer )
    {
        const sal_Int32 nBoundAttr = PropertyAttribute::BOUND;
        const sal_Int32 nMayBeVoidAttr = PropertyAttribute::MAYBEVOID | nBoundAttr;

        const Type& rSalInt32Type = ::cppu::UnoType<sal_Int32>::get();
        const Type& rStringType = ::cppu::UnoType<OUString>::get();

        _rPropertyContainer.registerMayBeVoidProperty( PROPERTY_ALIGN, PROPERTY_ID_ALIGN, nMayBeVoidAttr, &m_aAlignment, rSalInt32Type );
        _rPropertyContainer.registerMayBeVoidProperty( PROPERTY_NUMBERFORMAT, PROPERTY_ID_NUMBERFORMAT, nMayBeVoidAttr, &m_aFormatKey, rSalInt32Type );
        _rPropertyContainer.registerMayBeVoidProperty( PROPERTY_RELATIVEPOSITION, PROPERTY_ID_RELATIVEPOSITION, nMayBeVoidAttr, &m_aRelativePosition, rSalInt32Type );
        _rPropertyContainer.registerMayBeVoidProperty( PROPERTY_WIDTH, PROPERTY_ID_WIDTH, nMayBeVoidAttr, &m_aWidth, rSalInt32Type );
        _rPropertyContainer.registerMayBeVoidProperty( PROPERTY_HELPTEXT, PROPERTY_ID_HELPTEXT, nMayBeVoidAttr, &m_aHelpText, rStringType );
        _rPropertyContainer.registerMayBeVoidProperty( PROPERTY_CONTROLDEFAULT, PROPERTY_ID_CONTROLDEFAULT, nMayBeVoidAttr, &m_aControlDefault, rStringType );
        _rPropertyContainer.registerProperty( PROPERTY_CONTROLMODEL, PROPERTY_ID_CONTROLMODEL, nBoundAttr, &m_xControlModel, cppu::UnoType<decltype(m_xControlModel)>::get() );
        _rPropertyContainer.registerProperty( PROPERTY_HIDDEN, PROPERTY_ID_HIDDEN, nBoundAttr, &m_bHidden, cppu::UnoType<decltype(m_bHidden)>::get() );
    }

    bool OColumnSettings::isColumnSettingProperty( const sal_Int32 _nPropertyHandle )
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

    bool OColumnSettings::isDefaulted( const sal_Int32 _nPropertyHandle, const Any& _rPropertyValue )
    {
        switch ( _nPropertyHandle )
        {
        case PROPERTY_ID_ALIGN:
        case PROPERTY_ID_NUMBERFORMAT:
        case PROPERTY_ID_RELATIVEPOSITION:
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_HELPTEXT:
        case PROPERTY_ID_CONTROLDEFAULT:
            return !_rPropertyValue.hasValue();

        case PROPERTY_ID_CONTROLMODEL:
            return !Reference< XPropertySet >( _rPropertyValue, UNO_QUERY ).is();

        case PROPERTY_ID_HIDDEN:
            {
                bool bHidden = false;
                OSL_VERIFY( _rPropertyValue >>= bHidden );
                return !bHidden;
            }
        }
        OSL_FAIL( "OColumnSettings::isDefaulted: illegal property handle!" );
        return false;
    }

    bool OColumnSettings::hasDefaultSettings( const Reference< XPropertySet >& _rxColumn )
    {
        ENSURE_OR_THROW( _rxColumn.is(), "illegal column" );
        try
        {
            Reference< XPropertySetInfo > xPSI( _rxColumn->getPropertySetInfo(), UNO_SET_THROW );

            struct PropertyDescriptor
            {
                OUString sName;
                sal_Int32       nHandle;
            };
            const PropertyDescriptor aProps[] =
            {
                { OUString(PROPERTY_ALIGN),            PROPERTY_ID_ALIGN },
                { OUString(PROPERTY_NUMBERFORMAT),     PROPERTY_ID_NUMBERFORMAT },
                { OUString(PROPERTY_RELATIVEPOSITION), PROPERTY_ID_RELATIVEPOSITION },
                { OUString(PROPERTY_WIDTH),            PROPERTY_ID_WIDTH },
                { OUString(PROPERTY_HELPTEXT),         PROPERTY_ID_HELPTEXT },
                { OUString(PROPERTY_CONTROLDEFAULT),   PROPERTY_ID_CONTROLDEFAULT },
                { OUString(PROPERTY_CONTROLMODEL),     PROPERTY_ID_CONTROLMODEL },
                { OUString(PROPERTY_HIDDEN),           PROPERTY_ID_HIDDEN }
            };

            for (const auto & aProp : aProps)
            {
                if ( xPSI->hasPropertyByName( aProp.sName ) )
                    if ( !isDefaulted( aProp.nHandle, _rxColumn->getPropertyValue( aProp.sName ) ) )
                        return false;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return true;
    }

} // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
