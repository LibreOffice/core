/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef	_MULTI_PROPERTY_SET_HANDLER_HXX
#define	_MULTI_PROPERTY_SET_HANDLER_HXX

#include	<rtl/ustring.hxx>

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
namespace binfilter {


/**	@descr	MultiPropertySetHandler handles the two slightly different
        interfaces XPropertySet and XMultiPorpertySet for accessing
        properties of an object.

        It uses the classes PropertyWrapperBase and the template
        PropertyWrapper for a type safe access to single properties.

        The function class OUStringComparison is used by a STL map to
        sort the properties by names.
*/

/**	@descr	Base class for the templated property wrappers.
        Having a common base class allows to set a variable to the
        property's value without explicit knowledge of its type.
*/
class	PropertyWrapperBase
{
public:
    /**	@descr	Create a class instance and store the given name.
        @param	rName	The name of the property.
    */
    PropertyWrapperBase	(const ::rtl::OUString & rName)
        :	msName (rName)
    {}

    /**	@descr	Abstract interface of a method for setting a variables
            value to that of the property.
    */
    virtual	void	SetValue	(const ::com::sun::star::uno::Any & rValue) = 0;

    const ::rtl::OUString msName;
};




/**	@descr	For every property type there will be one instantiation of this
        template class with its own and type specific version of SetValue.
*/
template<class T> class PropertyWrapper : public PropertyWrapperBase
{
public:
    /**	@descr	Create a wrapper for a property of type T.
    */
    PropertyWrapper (const ::rtl::OUString & rName, T & rValue)
        :	PropertyWrapperBase (rName),
            mrValue (rValue)
    {}

    /**	descr	Set the given value inside an Any to the variable referenced
        by the data member.
    */
    virtual	void	SetValue	(const ::com::sun::star::uno::Any & rValue)
    {
        rValue >>= mrValue;
    }

private:
    ///	Reference to a variable.  Its value can be modified by a call to SetValue.
    T	&	mrValue;
};




/**	@descr	Function object for comparing two OUStrings.
*/
class	OUStringComparison
{
public:
    ///	Compare two strings.  Returns true if the first is before the second.
    inline	bool	operator()	(const ::rtl::OUString & a, const ::rtl::OUString & b) const
    {
        return (a.compareTo (b) < 0);
    }
};




/**	@descr	This class lets you get the values from an object that either
        supports the interface XPropertySet or XMultiPropertySet.  If it
        supports both interfaces then XMultiPropertySet is preferred.

        Using it works in three steps.
        1.	Create an instance and pass a reference to the object from which to
            get the property values.
        2.	Make all properties whose values you want to get known to the object
            by using the Add method.  This creates instances of a template class
            that stores the properties name and a reference to the variable in
            which to store its value.
        3.	Finally call GetProperties to store the properties values into the
            variables specified in step 2.  This uses either the XPropertySet or
            (preferred) the XMultiPropertySet interface.
*/
class	MultiPropertySetHandler
{
public:
    /**	@descr	Create a handler of the property set of the given
            object.
        @param	xObject	A reference to any of the object's interfaces.
            not neccessarily XPropertySet or XMultiPropertySet.  It
            is casted later to one of the two of them.
    */
    MultiPropertySetHandler	(::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface> xObject);
    ~MultiPropertySetHandler	(void);
    /**	@descr	Add a property to handle.  The type given implicitely by the
            reference to a variable is used to create an instance of
            the PropertyWrapper template class.
        @param	sName	Name of the property.
        @param	rValue	Reference to a variable whose value is set by the
            call to GetProperties to the property's value.
    */
    template<class T> void	Add	(const ::rtl::OUString & sName, T& rValue)
    {
        aPropertyList[sName] = new PropertyWrapper<T> (sName, rValue);
    }

    /**	@descr	Try to get the values for all properties added with the Add
            method.  If possible it uses the XMultiPropertySet.  If that fails
            (i.e. for an UnknownPropertyExcption) or if the interface is not
            supported it uses the XPropertySet interface.
        @return	If none of the two interfaces is supported or using them both
            fails then FALSE is returned.  Else True is returned.
    */
    inline	BOOL	GetProperties	(void);
    
private:
    /**	@descr	Try to use the XMultiPropertySet interface to get the property
            values.
        @param	rNameList	A precomputed and sorted sequence of OUStrings
            containing the properties names.
        @return	True if values could be derived.
    */
    inline	BOOL	MultiGet	(const ::com::sun::star::uno::Sequence<
        ::rtl::OUString> & rNameList);
    
    /**	@descr	Try to use the XPropertySet interface to get the property
            values.
        @param	rNameList	A precomputed and sorted sequence of OUStrings
            containing the properties names.
        @return	True if values could be derived.
    */
    inline	BOOL	SingleGet	(const ::com::sun::star::uno::Sequence<	
        ::rtl::OUString> & rNameList);
    
    /**	@descr	STL map that maps from property names to polymorphic instances of
            PropertyWrapper.  It uses OUStringComparison for sorting
            the property names.
    */
    ::std::map< ::rtl::OUString, PropertyWrapperBase*, OUStringComparison> aPropertyList;
    
    ///	The object from which to get the property values.
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>	mxObject;
};



//=====  Inline implementation of the methods declared above  ==========================

MultiPropertySetHandler::MultiPropertySetHandler (::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XInterface> xObject)
        :	mxObject (xObject)
{
}




MultiPropertySetHandler::~MultiPropertySetHandler (void)
{
    ::std::map< ::rtl::OUString, PropertyWrapperBase*, OUStringComparison>::iterator I;
    for (I=aPropertyList.begin(); I!=aPropertyList.end(); I++)
        delete I->second;
}


/*	
template<class T> void	MultiPropertySetHandler::Add (const ::rtl::OUString & sName, T& pValue)
{
    aPropertyList[sName] = new PropertyWrapper<T> (sName, pValue);
}
*/



BOOL	MultiPropertySetHandler::GetProperties	(void)
{
    ::std::map< ::rtl::OUString, PropertyWrapperBase*, OUStringComparison>::iterator I;
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aNameList (aPropertyList.size());
    int i;
    for (I=aPropertyList.begin(),i=0; I!=aPropertyList.end(); I++)
        aNameList[i++] = I->second->msName;
    if ( ! MultiGet(aNameList))
        if ( ! SingleGet(aNameList))
            return FALSE;
    return TRUE;
}




BOOL	MultiPropertySetHandler::MultiGet	(const ::com::sun::star::uno::Sequence<
    ::rtl::OUString> & rNameList)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet> xMultiSet (
        mxObject, ::com::sun::star::uno::UNO_QUERY);
    if (xMultiSet.is())
        try
        {
            ::std::map< ::rtl::OUString, PropertyWrapperBase*, OUStringComparison>::iterator I;
            int	i;
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any> aValueList = 
                xMultiSet->getPropertyValues (rNameList);
            for (I=aPropertyList.begin(),i=0; I!=aPropertyList.end(); I++)
                I->second->SetValue (aValueList[i++]);
        }
        catch (::com::sun::star::beans::UnknownPropertyException e)
        {
            return FALSE;
        }
    else
        return FALSE;
        
    return TRUE;
}




BOOL	MultiPropertySetHandler::SingleGet	(const ::com::sun::star::uno::Sequence<
    ::rtl::OUString> & rNameList)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xSingleSet (
        mxObject, ::com::sun::star::uno::UNO_QUERY);
    if (xSingleSet.is())
        try
        {
            ::std::map< ::rtl::OUString, PropertyWrapperBase*, OUStringComparison>::iterator I;
            int	i;		
            for (I=aPropertyList.begin(),i=0; I!=aPropertyList.end(); I++)
                I->second->SetValue (xSingleSet->getPropertyValue (rNameList[i++]));
        }
        catch (::com::sun::star::beans::UnknownPropertyException e)
        {
            return FALSE;
        }
    else
        return FALSE;
            
    return TRUE;
}


}//end of namespace binfilter
#endif

