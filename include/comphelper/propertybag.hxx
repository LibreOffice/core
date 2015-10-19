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

#ifndef INCLUDED_COMPHELPER_PROPERTYBAG_HXX
#define INCLUDED_COMPHELPER_PROPERTYBAG_HXX

#include <comphelper/comphelperdllapi.h>
#include <comphelper/propertycontainerhelper.hxx>

#include <memory>


namespace comphelper
{


    struct PropertyBag_Impl;

    //= PropertyBag

    /** provides a bag of properties associated with their values

        This class can, for instance, be used for components which need to implement
        the com.sun.star.beans.PropertyBag service.
    */
    class COMPHELPER_DLLPUBLIC PropertyBag : protected OPropertyContainerHelper
    {
    private:
        ::std::unique_ptr< PropertyBag_Impl > m_pImpl;

    public:
        PropertyBag();
        virtual ~PropertyBag();

        /** allow adding property with empty string as name
            (by default, such names are rejected with IllegalActionException).
            @param i_isAllowed
                iff true, empty property name will be allowed
         */
        void setAllowEmptyPropertyName(bool i_isAllowed = true);

        /** adds a property to the bag

            The type of the property is determined from its initial value (<code>_rInitialValue</code>).

            @param _rName
                the name of the new property. Must not be empty unless
                explicitly allowed with setAllowEmptyPropertyName.
            @param _nHandle
                the handle of the new property
            @param _nAttributes
                the attributes of the property
            @param _rInitialValue
                the initial value of the property. Must not be <NULL/>, to allow
                determining the property type.

            @throws css::beans::IllegalTypeException
                if the initial value is <NULL/>
            @throws css::beans::PropertyExistException
                if the name or the handle are already used
            @throws css::beans::IllegalArgumentException
                if the name is empty
        */
        void    addProperty(
                    const OUString& _rName,
                    sal_Int32 _nHandle,
                    sal_Int32 _nAttributes,
                    const css::uno::Any& _rInitialValue
                );

        /** adds a property to the bag

            The initial value of the property is <NULL/>.

            @param _rName
                the name of the new property. Must not be empty unless
                explicitly allowed with setAllowEmptyPropertyName.
            @param _rType
                the type of the new property
            @param _nHandle
                the handle of the new property
            @param _nAttributes
                the attributes of the property

            @throws css::beans::IllegalTypeException
                if the initial value is <NULL/>
            @throws css::beans::PropertyExistException
                if the name or the handle are already used
            @throws css::beans::IllegalArgumentException
                if the name is empty
        */
        void    addVoidProperty(
                    const OUString& _rName,
                    const css::uno::Type& _rType,
                    sal_Int32 _nHandle,
                    sal_Int32 _nAttributes
                );

        /** removes a property from the bag
            @param _rName
                the name of the to-be-removed property.
            @throws UnknownPropertyException
                if the bag does not contain a property with the given name
            @throws NotRemoveableException
                if the property with the given name is not removable, as indicated
                by the property attributes used in a previous <code>addProperty</code>
                call.
        */
        void    removeProperty(
                    const OUString& _rName
                );

        /** describes all properties in the bag
            @param _out_rProps
                takes, upon return, the descriptions of all properties in the bag
        */
        inline void describeProperties(
                    css::uno::Sequence< css::beans::Property >& _out_rProps
                ) const
        {
            OPropertyContainerHelper::describeProperties( _out_rProps );
        }

        /** retrieves the value of a property given by handle
            @param _nHandle
                the handle of the property whose value is to be retrieved
            @param _out_rValue
                output parameter taking the property value
            @throws UnknownPropertyException
                if the given handle does not denote a property in the bag
        */
        void    getFastPropertyValue(
                    sal_Int32 _nHandle,
                    css::uno::Any& _out_rValue
                ) const;

        /** converts a to-be-set value of a property (given by handle) so that it can
            be used in subsequent calls to setFastPropertyValue
            @param _nHandle
                the handle of the property
            @param _rNewValue
                the new value, which should be converted
            @param _out_rConvertedValue
                output parameter taking the converted value
            @param _out_rCurrentValue
                output parameter taking the current value of the
                property
            @throws UnknownPropertyException
                if the given handle does not denote a property in the bag
            @throws IllegalArgumentException
                if the given value cannot be lossless converted into a value
                for the given property.
        */
        bool    convertFastPropertyValue(
                        sal_Int32 _nHandle,
                        const css::uno::Any& _rNewValue,
                        css::uno::Any& _out_rConvertedValue,
                        css::uno::Any& _out_rCurrentValue
                    ) const;

        /** sets a new value for a property given by handle
            @throws UnknownPropertyException
                if the given handle does not denote a property in the bag
        */
        void    setFastPropertyValue(
                        sal_Int32 _nHandle,
                        const css::uno::Any& _rValue
                    );

        /** returns the default value for a property given by handle

            The default value of a property is its initial value, as passed
            to ->addProperty.

            @param _nHandle
                handle of the property whose default value is to be obtained
            @param _out_rValue
                the default value
            @throws UnknownPropertyException
                if the given handle does not denote a property in the bag
        */
        void    getPropertyDefaultByHandle(
                        sal_Int32 _nHandle,
                        css::uno::Any& _out_rValue
                    ) const;

        /** determines whether a property with a given name is part of the bag
        */
        inline  bool    hasPropertyByName( const OUString& _rName ) const
        {
            return isRegisteredProperty( _rName );
        }

        /** determines whether a property with a given handle is part of the bag
        */
        inline  bool    hasPropertyByHandle( sal_Int32 _nHandle ) const
        {
            return isRegisteredProperty( _nHandle );
        }
    protected:
        using OPropertyContainerHelper::convertFastPropertyValue;
        using OPropertyContainerHelper::getFastPropertyValue;
    };


} // namespace comphelper


#endif // INCLUDED_COMPHELPER_PROPERTYBAG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
