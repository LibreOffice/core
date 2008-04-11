/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertysethelper.hxx,v $
 * $Revision: 1.4 $
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

#ifndef INCLUDED_CANVAS_PROPERTYSETHELPER_HXX
#define INCLUDED_CANVAS_PROPERTYSETHELPER_HXX

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <canvas/canvastools.hxx>

#include <boost/function.hpp>
#include <vector>
#include <memory>


namespace canvas
{
    /** Really simplistic XPropertySet helper for properties.

        This class provides easy access to properties, referenced via
        ASCII strings. The name/property modification callbacks pairs
        are passed into this class via a vector. Each time a property
        is set or queried, the corresponding getter or setter callback
        is called.

        Use this class as a delegate for the corresponding
        XPropertySet methods, and take care of UNO XInterface and lock
        handling by yourself.

        The core responsibility of this this class is the name/value
        mapping for property sets.
     */
    class PropertySetHelper
    {
    public:
        typedef boost::function0< ::com::sun::star::uno::Any >            GetterType;
        typedef boost::function1<void, const ::com::sun::star::uno::Any&> SetterType;
        struct Callbacks
        {
            GetterType getter;
            SetterType setter;
        };
        typedef tools::ValueMap< Callbacks >     MapType;
        typedef std::vector< MapType::MapEntry > InputMap;

        class MakeMap : public InputMap
        {
        public:
            MakeMap(const char*        name,
                    const GetterType&  getter,
                    const SetterType&  setter)
            {
                MapType::MapEntry aEntry={name, {getter, setter}};
                this->push_back(aEntry);
            }
            MakeMap(const char*       name,
                    const GetterType& getter)
            {
                MapType::MapEntry aEntry={name, {getter, SetterType()}};
                this->push_back(aEntry);
            }
            MakeMap& operator()(const char*        name,
                                const GetterType&  getter,
                                const SetterType&  setter)
            {
                MapType::MapEntry aEntry={name, {getter, setter}};
                this->push_back(aEntry);
                return *this;
            }
            MakeMap& operator()(const char*       name,
                                const GetterType& getter)
            {
                MapType::MapEntry aEntry={name, {getter, SetterType()}};
                this->push_back(aEntry);
                return *this;
            }
        };

        /** Create helper with zero properties
         */
        PropertySetHelper();

        /** Create helper with given name/value map
         */
        explicit PropertySetHelper( const InputMap& rMap );

        /** Init helper with new name/value map

            @param rMap
            Vector of name/function pointers. Each name is offered as
            a property, and reading/writing to this property is passed
            on to the given function pointer.
         */
        void initProperties( const InputMap& rMap );

        /** Add given properties to helper

            @param rMap
            Vector of name/function pointers. Each name is offered as
            a property, and reading/writing to this property is passed
            on to the given function pointer. These name/function
            pairs are added to the already existing ones.
         */
        void addProperties( const InputMap& rMap );

        /** Checks whether the given string corresponds to a valid
            property name.

            @return true, if the given name maps to a known property.
         */
        bool isPropertyName( const ::rtl::OUString& aPropertyName ) const;

        /** Request the currently active map
         */
        const InputMap& getPropertyMap() const { return maMapEntries; }

        // XPropertySet implementation
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > getPropertySetInfo() const;
        void setPropertyValue( const ::rtl::OUString&            aPropertyName,
                               const ::com::sun::star::uno::Any& aValue );
        ::com::sun::star::uno::Any getPropertyValue( const ::rtl::OUString& PropertyName ) const;
        void addPropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener );
        void removePropertyChangeListener( const ::rtl::OUString& aPropertyName,
                                           const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener );
        void addVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& xListener );
        void removeVetoableChangeListener( const ::rtl::OUString& aPropertyName,
                                           const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& xListener );

    private:
        std::auto_ptr<MapType>  mpMap;
        InputMap                maMapEntries;
    };
}

#endif /* INCLUDED_CANVAS_PROPERTYSETHELPER_HXX */
