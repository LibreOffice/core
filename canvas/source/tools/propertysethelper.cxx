/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propertysethelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:18:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
