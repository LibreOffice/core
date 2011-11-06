/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
