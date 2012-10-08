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


#include <canvas/propertysethelper.hxx>

using namespace ::com::sun::star;

namespace canvas
{
    namespace
    {
        void throwUnknown( const ::rtl::OUString& aPropertyName )
        {
            throw beans::UnknownPropertyException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PropertySetHelper: property " )) +
                aPropertyName +
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " not found." )),
                uno::Reference< uno::XInterface >()
                );
        }

        void throwVeto( const ::rtl::OUString& aPropertyName )
        {
            throw beans::PropertyVetoException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PropertySetHelper: property " )) +
                aPropertyName +
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " access was vetoed." )),
                uno::Reference< uno::XInterface >() );
        }

        struct EntryComparator
        {
            bool operator()( const PropertySetHelper::MapType::MapEntry& rLHS,
                             const PropertySetHelper::MapType::MapEntry& rRHS )
            {
                return strcmp( rLHS.maKey,
                               rRHS.maKey ) < 0;
            }
        };
    }

    PropertySetHelper::PropertySetHelper() :
        mpMap(),
        maMapEntries()
    {
    }

    void PropertySetHelper::initProperties( const InputMap& rMap )
    {
        mpMap.reset();
        maMapEntries = rMap;

        std::sort( maMapEntries.begin(),
                   maMapEntries.end(),
                   EntryComparator() );

        if( !maMapEntries.empty() )
            mpMap.reset( new MapType(&maMapEntries[0],
                                     maMapEntries.size(),
                                     true) );
    }

    void PropertySetHelper::addProperties( const InputMap& rMap )
    {
        InputMap aMerged( getPropertyMap() );
        aMerged.insert( aMerged.end(),
                        rMap.begin(),
                        rMap.end() );

        initProperties( aMerged );
    }

    bool PropertySetHelper::isPropertyName( const ::rtl::OUString& aPropertyName ) const
    {
        if( !mpMap.get() )
            return false;

        Callbacks aDummy;
        return mpMap->lookup( aPropertyName,
                              aDummy );
    }

    uno::Reference< beans::XPropertySetInfo > PropertySetHelper::getPropertySetInfo() const
    {
        // we're a stealth property set
        return uno::Reference< beans::XPropertySetInfo >();
    }

    void PropertySetHelper::setPropertyValue( const ::rtl::OUString& aPropertyName,
                                              const uno::Any&        aValue )
    {
        Callbacks aCallbacks;
        if( !mpMap.get() ||
            !mpMap->lookup( aPropertyName,
                            aCallbacks ) )
        {
            throwUnknown( aPropertyName );
        }

        if( aCallbacks.setter.empty() )
            throwVeto( aPropertyName );

        aCallbacks.setter(aValue);
    }

    uno::Any PropertySetHelper::getPropertyValue( const ::rtl::OUString& aPropertyName ) const
    {
        Callbacks aCallbacks;
        if( !mpMap.get() ||
            !mpMap->lookup( aPropertyName,
                            aCallbacks ) )
        {
            throwUnknown( aPropertyName );
        }

        if( !aCallbacks.getter.empty() )
            return aCallbacks.getter();

        // TODO(Q1): subtlety, empty getter method silently returns
        // the empty any
        return uno::Any();
    }

    void PropertySetHelper::addPropertyChangeListener( const ::rtl::OUString&                                  aPropertyName,
                                                       const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
    {
        // check validity of property, but otherwise ignore the
        // request
        if( !isPropertyName( aPropertyName ) )
            throwUnknown( aPropertyName );
    }

    void PropertySetHelper::removePropertyChangeListener( const ::rtl::OUString&                                  /*aPropertyName*/,
                                                          const uno::Reference< beans::XPropertyChangeListener >& /*xListener*/ )
    {
        // ignore request, no listener added in the first place
    }

    void PropertySetHelper::addVetoableChangeListener( const ::rtl::OUString&                                  aPropertyName,
                                                       const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/ )
    {
        // check validity of property, but otherwise ignore the
        // request
        if( !isPropertyName( aPropertyName ) )
            throwUnknown( aPropertyName );
    }

    void PropertySetHelper::removeVetoableChangeListener( const ::rtl::OUString&                                  /*aPropertyName*/,
                                                          const uno::Reference< beans::XVetoableChangeListener >& /*xListener*/ )
    {
        // ignore request, no listener added in the first place
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
