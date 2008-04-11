/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertysethelper.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include <canvas/propertysethelper.hxx>

using namespace ::com::sun::star;

namespace canvas
{
    namespace
    {
        void throwUnknown( const ::rtl::OUString& aPropertyName )
        {
            throw beans::UnknownPropertyException(
                ::rtl::OUString::createFromAscii("PropertySetHelper: property ") +
                aPropertyName +
                ::rtl::OUString::createFromAscii(" not found."),
                uno::Reference< uno::XInterface >()
                );
        }

        void throwVeto( const ::rtl::OUString& aPropertyName )
        {
            throw beans::PropertyVetoException(
                ::rtl::OUString::createFromAscii("PropertySetHelper: property ") +
                aPropertyName +
                ::rtl::OUString::createFromAscii(" access was vetoed."),
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

    PropertySetHelper::PropertySetHelper( const InputMap& rMap ) :
        mpMap(),
        maMapEntries()
    {
        initProperties(rMap);
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
