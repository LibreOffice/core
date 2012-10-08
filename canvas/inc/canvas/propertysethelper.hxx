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

#ifndef INCLUDED_CANVAS_PROPERTYSETHELPER_HXX
#define INCLUDED_CANVAS_PROPERTYSETHELPER_HXX

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <canvas/canvastools.hxx>

#include <boost/function.hpp>
#include <vector>
#include <memory>

#include <canvas/canvastoolsdllapi.h>

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
    class CANVASTOOLS_DLLPUBLIC PropertySetHelper
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
