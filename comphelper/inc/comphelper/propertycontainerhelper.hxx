/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertycontainerhelper.hxx,v $
 * $Revision: 1.6 $
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

#ifndef COMPHELPER_PROPERTYCONTAINERHELPER_HXX
#define COMPHELPER_PROPERTYCONTAINERHELPER_HXX

#include <cppuhelper/propshlp.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/beans/Property.hpp>
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#include "comphelper/comphelperdllapi.h"

//.........................................................................
namespace comphelper
{
//.........................................................................

// infos about one single property
struct COMPHELPER_DLLPRIVATE PropertyDescription
{
    // the possibilities where a property holding object may be located
    enum LocationType
    {
        ltDerivedClassRealType,     // within the derived class, it's a "real" (non-Any) type
        ltDerivedClassAnyType,      // within the derived class, it's a <type scope="com.sun.star.uno">Any</type>
        ltHoldMyself                // within m_aHoldProperties
    };
    // the location of an object holding a property value :
    union LocationAccess
    {
        void*       pDerivedClassMember;        // a pointer to a member of an object of a derived class
        sal_Int32   nOwnClassVectorIndex;       // an index within m_aHoldProperties
    };

    ::com::sun::star::beans::Property
                        aProperty;
    LocationType        eLocated;       // where is the object containing the value located ?
    LocationAccess      aLocation;      // access to the property value

    PropertyDescription()
        :aProperty( ::rtl::OUString(), -1, ::com::sun::star::uno::Type(), 0 )
        ,eLocated( ltHoldMyself )
    {
        aLocation.nOwnClassVectorIndex = -1;
    }
};

//==========================================================================
//= OPropertyContainerHelper
//==========================================================================
/** helper class for managing property values, and implementing most of the X*Property* interfaces

    The property values are usually held in derived classes, but can also be given to the
    responsibility of this class here.

    For more information, see http://wiki.services.openoffice.org/wiki/Development/Cpp/Helper/PropertyContainerHelper.
*/
class COMPHELPER_DLLPUBLIC OPropertyContainerHelper
{
    typedef ::std::vector< ::com::sun::star::uno::Any > PropertyContainer;
    typedef PropertyContainer::iterator                 PropertyContainerIterator;
    typedef PropertyContainer::const_iterator           ConstPropertyContainerIterator;
    PropertyContainer   m_aHoldProperties;
        // the properties which are hold by this class' instance, not the derived one's

private:
    typedef ::std::vector< PropertyDescription >    Properties;
    typedef Properties::iterator                    PropertiesIterator;
    typedef Properties::const_iterator              ConstPropertiesIterator;
    Properties      m_aProperties;

    sal_Bool        m_bUnused;

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
    void    registerProperty(const ::rtl::OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes,
        void* _pPointerToMember, const ::com::sun::star::uno::Type& _rMemberType);


    /** register a property. The property is represented through a ::com::sun::star::uno::Any member of the
        derived class which calls this methdod.
        @param      _rName              the name of the property
        @param      _nHandle            the handle of the property
        @param      _nAttributes        the attributes of the property
        @param      _pPointerToMember   the pointer to the member representing the property
                                        within the derived class, which has to be a ::com::sun::star::uno::Any.
        @param      _rExpectedType      the expected type of the property. NOT the type of the object to which
                                        _pPointerToMember points (this is always an Any).
    */
    void    registerMayBeVoidProperty(const ::rtl::OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes,
        ::com::sun::star::uno::Any* _pPointerToMember, const ::com::sun::star::uno::Type& _rExpectedType);

    /** register a property. The repository will create an own object holding this property, so there is no
        need to declare an extra member in your derived class
        @param      _rName              the name of the property
        @param      _nHandle            the handle of the property
        @param      _nAttributes        the attributes of the property
        @param      _rType              the type of the property
        @param      _pInitialValue      the initial value of the property. May be null if _nAttributes includes
                                        the ::com::sun::star::beans::PropertyAttribute::MAYBEVOID flag.
                                        Else it must be a pointer to an object of the type described by _rType.
    */
    void    registerPropertyNoMember(const ::rtl::OUString& _rName, sal_Int32 _nHandle, sal_Int32 _nAttributes,
        const ::com::sun::star::uno::Type& _rType, const void* _pInitialValue);

    /** revokes a previously registered property
        @throw  com::sun::star::beans::UnknownPropertyException
            if no property with the given handle is registered
    */
    void    revokeProperty( sal_Int32 _nHandle );


    /// checkes whether a property with the given handle has been registered
    sal_Bool    isRegisteredProperty( sal_Int32 _nHandle ) const;

    /// checkes whether a property with the given name has been registered
    sal_Bool    isRegisteredProperty( const ::rtl::OUString& _rName ) const;


    // helper for implementing OPropertySetHelper overridables
    sal_Bool    convertFastPropertyValue(
                    ::com::sun::star::uno::Any & rConvertedValue,
                    ::com::sun::star::uno::Any & rOldValue,
                    sal_Int32 nHandle,
                    const ::com::sun::star::uno::Any& rValue
                )
                SAL_THROW((::com::sun::star::lang::IllegalArgumentException));

    void        setFastPropertyValue(
                        sal_Int32 nHandle,
                        const ::com::sun::star::uno::Any& rValue
                    )
                    SAL_THROW((::com::sun::star::uno::Exception));

    void        getFastPropertyValue(
                        ::com::sun::star::uno::Any& rValue,
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
    void    describeProperties(::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps) const;

    /** modify the attributes of an already registered property.

        You may want to use this if you're a derived from OPropertyContainer indirectly and want to override
        some settings your base class did.
    */
    void    modifyAttributes(sal_Int32 _nHandle, sal_Int32 _nAddAttrib, sal_Int32 _nRemoveAttrib);

    /** retrieves the description for a registered property
        @throw  com::sun::star::beans::UnknownPropertyException
            if no property with the given name is registered
    */
    const ::com::sun::star::beans::Property&
            getProperty( const ::rtl::OUString& _rName ) const;

private:
    /// insertion of _rProp into m_aProperties, keeping the sort order
    COMPHELPER_DLLPRIVATE void  implPushBackProperty(const PropertyDescription& _rProp);

    /// search the PropertyDescription for the given handle (within m_aProperties)
    COMPHELPER_DLLPRIVATE PropertiesIterator    searchHandle(sal_Int32 _nHandle);

private:
    COMPHELPER_DLLPRIVATE OPropertyContainerHelper( const OPropertyContainerHelper& );            // never implemented
    COMPHELPER_DLLPRIVATE OPropertyContainerHelper& operator=( const OPropertyContainerHelper& ); // never implemented
};

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // COMPHELPER_PROPERTYCONTAINERHELPER_HXX
