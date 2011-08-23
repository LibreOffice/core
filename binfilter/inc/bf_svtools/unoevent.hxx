/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SVTOOLS_UNOEVENT_HXX_
#define _SVTOOLS_UNOEVENT_HXX_

#include "bf_svtools/svtdllapi.h"

#include <com/sun/star/container/XNameReplace.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/uno/XInterface.hpp>

#include <cppuhelper/implbase2.hxx>

namespace binfilter
{

class SvxMacroTableDtor;
class SvxMacroItem;
class SvxMacro;

/**	SvEventDescription: Description of a single event.
    mnEvent is the id used by SvxMacroItem
    mpEventName is the api name for this event
    
    the last event in an array is indicated by mnEvent && mpEventName == 0
*/
struct SvEventDescription
{
    sal_uInt16 mnEvent;
    const sal_Char* mpEventName;
};

/** 
 * SvBaseEventDescriptor: Abstract class that implements the basics
 * of an XNameReplace that is delivered by the
 * XEventsSupplier::getEvents() method. 
 *
 * The functionality this class provides is: 
 * 1) Which elements are in the XNameReplace?
 * 2) Mapping from Api names to item IDs.
 * 3) conversion from SvxMacroItem to Any and vice versa.
 *
 * All details of how to actually get and set SvxMacroItem(s) have to
 * be supplied by the base class.
 */
class  SvBaseEventDescriptor : public cppu::WeakImplHelper2
<
    ::com::sun::star::container::XNameReplace,
    ::com::sun::star::lang::XServiceInfo
>
{
    const ::rtl::OUString sEventType;
    const ::rtl::OUString sMacroName;
    const ::rtl::OUString sLibrary;
    const ::rtl::OUString sStarBasic;
    const ::rtl::OUString sJavaScript;
    const ::rtl::OUString sScript;
    const ::rtl::OUString sNone;


    /// name of own service
    const ::rtl::OUString sServiceName;

protected:
    const ::rtl::OUString sEmpty;

    /// last element is 0, 0
    const SvEventDescription* mpSupportedMacroItems;
    sal_Int16 mnMacroItems;

public:

     SvBaseEventDescriptor(const SvEventDescription* pSupportedMacroItems);

    virtual ~SvBaseEventDescriptor();


    // XNameReplace
    /// calls replaceByName(const sal_uInt16, const SvxMacro&)
    virtual void SAL_CALL replaceByName( 
        const ::rtl::OUString& rName,				 /// API name of event
        const ::com::sun::star::uno::Any& rElement ) /// event (PropertyValues)
            throw(
                ::com::sun::star::lang::IllegalArgumentException, 
                ::com::sun::star::container::NoSuchElementException, 
                ::com::sun::star::lang::WrappedTargetException, 
                ::com::sun::star::uno::RuntimeException);

    // XNameAccess (via XNameReplace)
    /// calls getByName(sal_uInt16)
    virtual ::com::sun::star::uno::Any SAL_CALL getByName( 
        const ::rtl::OUString& rName ) 	/// API name of event
            throw(
                ::com::sun::star::container::NoSuchElementException, 
                ::com::sun::star::lang::WrappedTargetException, 
                ::com::sun::star::uno::RuntimeException);

    // XNameAxcess (via XNameReplace)
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL 
                                                        getElementNames() 
        throw(::com::sun::star::uno::RuntimeException);

    // XNameAccess (via XNameReplace)
    virtual sal_Bool SAL_CALL hasByName( 
        const ::rtl::OUString& rName ) 
            throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess (via XNameReplace)
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() 
        throw(::com::sun::star::uno::RuntimeException);

    // XElementAccess (via XNameReplace)
    virtual sal_Bool SAL_CALL hasElements() 
        throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    /// must be implemented in subclass
    virtual rtl::OUString SAL_CALL getImplementationName(void) 
        throw( ::com::sun::star::uno::RuntimeException ) = 0;

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL 
        getSupportedServiceNames(void) 
            throw( ::com::sun::star::uno::RuntimeException );

protected:

    /// Must be implemented in subclass.
    virtual void replaceByName( 
        const sal_uInt16 nEvent,		/// item ID of event
        const SvxMacro& rMacro)		/// event (will be copied)
            throw(
                ::com::sun::star::lang::IllegalArgumentException, 
                ::com::sun::star::container::NoSuchElementException, 
                ::com::sun::star::lang::WrappedTargetException, 
                ::com::sun::star::uno::RuntimeException) = 0;

    /// Must be implemented in subclass.
    virtual void getByName( 
        SvxMacro& rMacro,
        const sal_uInt16 nEvent ) 
            throw(
                ::com::sun::star::container::NoSuchElementException, 
                ::com::sun::star::lang::WrappedTargetException, 
                ::com::sun::star::uno::RuntimeException) = 0;

    /// convert an API event name to the event ID as used by SvxMacroItem
    sal_uInt16 mapNameToEventID(const ::rtl::OUString& rName) const;

    /// get the event ID for the name; return 0 if not supported
    sal_uInt16 getMacroID(const ::rtl::OUString& rName) const;

    /// create PropertyValues and Any from macro
    void getAnyFromMacro(
        ::com::sun::star::uno::Any& aAny,	// Any to be filled by Macro values
        const SvxMacro& rMacro);

    /// create macro from PropertyValues (in an Any)
    void getMacroFromAny(
        SvxMacro& aMacro,		// reference to be filled by Any
        const ::com::sun::star::uno::Any& rAny)
            throw (
                ::com::sun::star::lang::IllegalArgumentException);

};




/** 
 * SvEventDescriptor: Implement the XNameReplace that is delivered by
 * the XEventsSupplier::getEvents() method. The SvEventDescriptor has
 * to be subclassed to implement the events for a specific
 * objects. The subclass has to 
 * 1) supply the super class constructor with a list of known events (item IDs)
 * 2) supply the super class constructor with a reference of it's parent object
 *    (to prevent destruction)
 * 3) implement getItem() and setItem(...) methods.
 *
 * If no object is available to which the SvEventDescriptor can attach itself,
 * the class SvDetachedEventDescriptor should be used.
 */
class  SvEventDescriptor : public SvBaseEventDescriptor
{
    /// keep reference to parent to prevent it from being destroyed
    ::com::sun::star::uno::Reference< 
        ::com::sun::star::uno::XInterface > xParentRef;

public:

     SvEventDescriptor(::com::sun::star::uno::XInterface& rParent,
                      const SvEventDescription* pSupportedMacroItems);

    virtual ~SvEventDescriptor();


protected:


    using SvBaseEventDescriptor::replaceByName;
    virtual void replaceByName( 
        const sal_uInt16 nEvent,		/// item ID of event
        const SvxMacro& rMacro)		/// event (will be copied)
            throw(
                ::com::sun::star::lang::IllegalArgumentException, 
                ::com::sun::star::container::NoSuchElementException, 
                ::com::sun::star::lang::WrappedTargetException, 
                ::com::sun::star::uno::RuntimeException);

    using SvBaseEventDescriptor::getByName;
    virtual void getByName( 
        SvxMacro& rMacros,			/// macro to be filled with values
        const sal_uInt16 nEvent ) 		/// item ID of event
            throw(
                ::com::sun::star::container::NoSuchElementException, 
                ::com::sun::star::lang::WrappedTargetException, 
                ::com::sun::star::uno::RuntimeException);


    /// Get the SvxMacroItem from the parent.
    /// must be implemented by subclass
    virtual const SvxMacroItem& getMacroItem() = 0;

    /// Set the SvxMacroItem at the parent.
    /// must be implemented by subclass
    virtual void setMacroItem(const SvxMacroItem& rItem) = 0;

    /// Get the SvxMacroItem Which Id needed for the current application
    /// must be implemented by subclass
    virtual sal_uInt16 getMacroItemWhich() const = 0;
};


/**
 * SvDetachedEventDescriptor:
 */
class  SvDetachedEventDescriptor : public SvBaseEventDescriptor
{
    // the macros; aMacros[i] is the value for aSupportedMacroItemIDs[i]
    SvxMacro** aMacros;

    const ::rtl::OUString sImplName;

public:

     SvDetachedEventDescriptor(const SvEventDescription* pSupportedMacroItems);

    virtual ~SvDetachedEventDescriptor();

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void) 
        throw( ::com::sun::star::uno::RuntimeException );

protected:

    sal_Int16 getIndex(const sal_uInt16 nID);

    using SvBaseEventDescriptor::replaceByName;
    virtual void replaceByName( 
        const sal_uInt16 nEvent,		/// item ID of event
        const SvxMacro& rMacro)		/// event (will be copied)
            throw(
                ::com::sun::star::lang::IllegalArgumentException, 
                ::com::sun::star::container::NoSuchElementException, 
                ::com::sun::star::lang::WrappedTargetException, 
                ::com::sun::star::uno::RuntimeException);

    using SvBaseEventDescriptor::getByName;
    virtual void getByName( 
        SvxMacro& rMacro,			/// macro to be filled
        const sal_uInt16 nEvent ) 		/// item ID of event
            throw(
                ::com::sun::star::container::NoSuchElementException, 
                ::com::sun::star::lang::WrappedTargetException, 
                ::com::sun::star::uno::RuntimeException);

    /// do we have an event?
    /// return sal_True: we have a macro for the event
    /// return sal_False: no macro; getByName() will return an empty macro
    /// IllegalArgumentException: the event is not supported
    using SvBaseEventDescriptor::hasByName;
    virtual sal_Bool hasByName( 
        const sal_uInt16 nEvent )		/// item ID of event
             throw(
                ::com::sun::star::lang::IllegalArgumentException);

};

class  SvMacroTableEventDescriptor : public SvDetachedEventDescriptor
{
public:

     SvMacroTableEventDescriptor(const SvEventDescription* pSupportedMacroItems);
     SvMacroTableEventDescriptor(const SvxMacroTableDtor& aFmt,
                                const SvEventDescription* pSupportedMacroItems);

    virtual ~SvMacroTableEventDescriptor();

    void copyMacrosFromTable(const SvxMacroTableDtor& aFmt);
    void copyMacrosIntoTable(SvxMacroTableDtor& aFmt);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
