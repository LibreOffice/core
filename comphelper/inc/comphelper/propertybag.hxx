/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertybag.hxx,v $
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

#ifndef COMPHELPER_PROPERTYBAG_HXX
#define COMPHELPER_PROPERTYBAG_HXX

#include "comphelper/comphelperdllapi.h"
#include <comphelper/propertycontainerhelper.hxx>

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <memory>

//........................................................................
namespace comphelper
{
//........................................................................

    struct PropertyBag_Impl;
    //====================================================================
    //= PropertyBag
    //====================================================================
    /** provides a bag of properties associated with their values

        This class can, for instance, be used for components which need to implement
        the com.sun.star.beans.PropertyBag service.
    */
    class COMPHELPER_DLLPUBLIC PropertyBag : protected OPropertyContainerHelper
    {
    private:
        ::std::auto_ptr< PropertyBag_Impl > m_pImpl;

    public:
        PropertyBag();
        virtual ~PropertyBag();

        /** adds a property to the bag

            The type of the property is determined from its initial value (<code>_rInitialValue</code>).

            @param _rName
                the name of the new property. Must not be empty.
            @param _nHandle
                the handle of the new property
            @param _nAttributes
                the attributes of the property
            @param _rInitialValue
                the initial value of the property. Must not be <NULL/>, to allow
                determining the property type.

            @throws ::com::sun::star::beans::IllegalTypeException
                if the initial value is <NULL/>
            @throws ::com::sun::star::beans::PropertyExistException
                if the name or the handle are already used
            @throws ::com::sun::star::beans::IllegalArgumentException
                if the name is empty
        */
        void    addProperty(
                    const ::rtl::OUString& _rName,
                    sal_Int32 _nHandle,
                    sal_Int32 _nAttributes,
                    const ::com::sun::star::uno::Any& _rInitialValue
                );

        /** removes a property from the bag
            @param _rName
                the name of the to-be-removed property.
            @throws UnknownPropertyException
                if the bag does not contain a property with the given name
            @throws NotRemoveableException
                if the property with the given name is not removeable, as indicated
                by the property attributes used in a previous <code>addProperty</code>
                call.
        */
        void    removeProperty(
                    const ::rtl::OUString& _rName
                );

        /** describes all properties in the bag
            @param _out_rProps
                takes, upon return, the descriptions of all properties in the bag
        */
        inline void describeProperties(
                    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& _out_rProps
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
                    ::com::sun::star::uno::Any& _out_rValue
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
                        const ::com::sun::star::uno::Any& _rNewValue,
                        ::com::sun::star::uno::Any& _out_rConvertedValue,
                        ::com::sun::star::uno::Any& _out_rCurrentValue
                    ) const;

        /** sets a new value for a property given by handle
            @throws UnknownPropertyException
                if the given handle does not denote a property in the bag
        */
        void    setFastPropertyValue(
                        sal_Int32 _nHandle,
                        const ::com::sun::star::uno::Any& _rValue
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
                        ::com::sun::star::uno::Any& _out_rValue
                    ) const;

        /** determines whether a property with a given name is part of the bag
        */
        inline  bool    hasPropertyByName( const ::rtl::OUString& _rName ) const
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

//........................................................................
} // namespace comphelper
//........................................................................

#endif // COMPHELPER_PROPERTYBAG_HXX

