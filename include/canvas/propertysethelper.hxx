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

#include <canvas/canvastools.hxx>

#include <functional>
#include <vector>
#include <memory>

#include <canvas/canvastoolsdllapi.h>

namespace com::sun::star::beans { class XPropertyChangeListener; }
namespace com::sun::star::beans { class XPropertySetInfo; }
namespace com::sun::star::beans { class XVetoableChangeListener; }

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

        The core responsibility of this class is the name/value
        mapping for property sets.
     */
    class CANVASTOOLS_DLLPUBLIC PropertySetHelper
    {
    public:
        typedef std::function<css::uno::Any ()> GetterType;
        typedef std::function<void (const css::uno::Any&)> SetterType;
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
                push_back(aEntry);
            }
            MakeMap(const char*       name,
                    const GetterType& getter)
            {
                MapType::MapEntry aEntry={name, {getter, SetterType()}};
                push_back(aEntry);
            }
            MakeMap& operator()(const char*        name,
                                const GetterType&  getter,
                                const SetterType&  setter)
            {
                MapType::MapEntry aEntry={name, {getter, setter}};
                push_back(aEntry);
                return *this;
            }
            MakeMap& operator()(const char*       name,
                                const GetterType& getter)
            {
                MapType::MapEntry aEntry={name, {getter, SetterType()}};
                push_back(aEntry);
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
        bool isPropertyName( const OUString& aPropertyName ) const;

        // XPropertySet implementation
        css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() const;
        void setPropertyValue( const OUString&      aPropertyName,
                               const css::uno::Any& aValue );
        css::uno::Any getPropertyValue( const OUString& PropertyName ) const;
        void addPropertyChangeListener( const OUString& aPropertyName,
                                        const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener );
        void addVetoableChangeListener( const OUString& aPropertyName,
                                        const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener );

    private:
        std::unique_ptr<MapType> mpMap;
        InputMap                maMapEntries;
    };
}

#endif /* INCLUDED_CANVAS_PROPERTYSETHELPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
