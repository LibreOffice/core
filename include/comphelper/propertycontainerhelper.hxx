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

#ifndef INCLUDED_COMPHELPER_PROPERTYCONTAINERHELPER_HXX
#define INCLUDED_COMPHELPER_PROPERTYCONTAINERHELPER_HXX

#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/beans/Property.hpp>
#include <vector>
#include <comphelper/comphelperdllapi.h>


namespace comphelper
{


// infos about one single property
struct COMPHELPER_DLLPUBLIC PropertyDescription
{
    // the possibilities where a property holding object may be located
    enum class LocationType
    {
        DerivedClassRealType,     // within the derived class, it's a "real" (non-Any) type
        DerivedClassAnyType,      // within the derived class, it's a com.sun.star.uno::Any
        HoldMyself                // within m_aHoldProperties
    };
    // the location of an object holding a property value :
    union LocationAccess
    {
        void*       pDerivedClassMember;        // a pointer to a member of an object of a derived class
        sal_Int32   nOwnClassVectorIndex;       // an index within m_aHoldProperties
    };

    css::beans::Property aProperty;
    LocationType         eLocated;       // where is the object containing the value located ?
    LocationAccess       aLocation;      // access to the property value

    PropertyDescription()
        :aProperty( OUString(), -1, css::uno::Type(), 0 )
        ,eLocated( LocationType::HoldMyself )
    {
        aLocation.nOwnClassVectorIndex = -1;
    }
};


//= OPropertyContainerHelper

/** helper class for managing property values, and implementing most of the X*Property* interfaces

    The property values are usually held in derived classes, but can also be given to the
    responsibility of this class here.

    For more information, see http://wiki.openoffice.org/wiki/Development/Cpp/Helper/PropertyContainerHelper.
*/
class COMPHELPER_DLLPUBLIC OPropertyContainerHelper
{
    typedef ::std::vector< css::uno::Any >              PropertyContainer;
    PropertyContainer   m_aHoldProperties;
        // the properties which are hold by this class' instance, not the derived one's

private:
    typedef ::std::vector< PropertyDescription >    Properties;
    typedef Properties::iterator                    PropertiesIterator;
    typedef Properties::const_iterator              ConstPropertiesIterator;
    Properties      m_aProperties;

protected:
    OPropertyContainerHelper();
    ~OPropertyContainerHelper();

    /** register a property. The property is represented through a member of the derived class which calls
        this methdod.
        @param      _rName              the name of the property
        @param      _nHandle            the handle of the property
        @param      _nAttributes        the attributes of the property
        @param      _pPointerToMember   the pointer to the member representing the property
                                        within the derived class.
        @param      _rMemberType        the cppu type of the property represented by the object
                                        to which _pPointerToMember points.
    */
    void    registerProperty(const OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes,
        void* _pPointerToMember, const css::uno::Type& _rMemberType);


    /** register a property. The property is represented through a css::uno::Any member of the
        derived class which calls this methdod.
        @param      _rName              the name of the property
        @param      _nHandle            the handle of the property
        @param      _nAttributes        the attributes of the property
        @param      _pPointerToMember   the pointer to the member representing the property
                                        within the derived class, which has to be a css::uno::Any.
        @param      _rExpectedType      the expected type of the property. NOT the type of the object to which
                                        _pPointerToMember points (this is always an Any).
    */
    void    registerMayBeVoidProperty(const OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes,
        css::uno::Any* _pPointerToMember, const css::uno::Type& _rExpectedType);

    /** register a property. The repository will create an own object holding this property, so there is no
        need to declare an extra member in your derived class
        @param      _rName              the name of the property
        @param      _nHandle            the handle of the property
        @param      _nAttributes        the attributes of the property
        @param      _rType              the type of the property
        @param      _pInitialValue      the initial value of the property. May be void if _nAttributes includes
                                        the css::beans::PropertyAttribute::MAYBEVOID flag.
                                        Else it must contain a value compatible with the type described by _rType.
    */
    void    registerPropertyNoMember(const OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes,
        const css::uno::Type& _rType, css::uno::Any const & _pInitialValue);

    /** revokes a previously registered property
        @throw  css::beans::UnknownPropertyException
            if no property with the given handle is registered
    */
    void    revokeProperty( sal_Int32 _nHandle );


    /// checks whether a property with the given handle has been registered
    bool    isRegisteredProperty( sal_Int32 _nHandle ) const;

    /// checks whether a property with the given name has been registered
    bool    isRegisteredProperty( const OUString& _rName ) const;


    // helper for implementing OPropertySetHelper overridables
    bool    convertFastPropertyValue(
                    css::uno::Any & rConvertedValue,
                    css::uno::Any & rOldValue,
                    sal_Int32 nHandle,
                    const css::uno::Any& rValue
                );

    void        setFastPropertyValue(
                        sal_Int32 nHandle,
                        const css::uno::Any& rValue
                    );

    void        getFastPropertyValue(
                        css::uno::Any& rValue,
                        sal_Int32 nHandle
                    ) const;

// helper
    /** appends the descriptions of all properties which were registered 'til that moment to the given sequence,
        keeping the array sorted (by name)
        @precond
            the given sequence is already sorted by name
        @param  _rProps
            initial property sequence which is to be extended
    */
    void    describeProperties(css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps) const;

    /** retrieves the description for a registered property
        @throw  css::beans::UnknownPropertyException
            if no property with the given name is registered
    */
    const css::beans::Property&
            getProperty( const OUString& _rName ) const;

private:
    /// insertion of _rProp into m_aProperties, keeping the sort order
    COMPHELPER_DLLPRIVATE void  implPushBackProperty(const PropertyDescription& _rProp);

    /// search the PropertyDescription for the given handle (within m_aProperties)
    COMPHELPER_DLLPRIVATE PropertiesIterator    searchHandle(sal_Int32 _nHandle);

private:
    OPropertyContainerHelper( const OPropertyContainerHelper& ) = delete;
    OPropertyContainerHelper& operator=( const OPropertyContainerHelper& ) = delete;
};


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_PROPERTYCONTAINERHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
