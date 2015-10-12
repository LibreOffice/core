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
#ifndef INCLUDED_SVTOOLS_UNOEVENT_HXX
#define INCLUDED_SVTOOLS_UNOEVENT_HXX

#include <svtools/svtdllapi.h>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppuhelper/implbase2.hxx>

class SvxMacroTableDtor;
class SvxMacroItem;
class SvxMacro;

/** SvEventDescription: Description of a single event.
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
class SVT_DLLPUBLIC SvBaseEventDescriptor : public cppu::WeakImplHelper2
<
    ::com::sun::star::container::XNameReplace,
    ::com::sun::star::lang::XServiceInfo
>
{
    const OUString sEventType;
    const OUString sMacroName;
    const OUString sLibrary;
    const OUString sStarBasic;
    const OUString sJavaScript;
    const OUString sScript;
    const OUString sNone;


    /// name of own service
    const OUString sServiceName;

protected:
    /// last element is 0, 0
    const SvEventDescription* mpSupportedMacroItems;
    sal_Int16 mnMacroItems;

public:

     SvBaseEventDescriptor(const SvEventDescription* pSupportedMacroItems);

    virtual ~SvBaseEventDescriptor();


    // XNameReplace
    /// calls replaceByName(const sal_uInt16, const SvxMacro&)
    virtual void SAL_CALL replaceByName(
        const OUString& rName,                /// API name of event
        const ::com::sun::star::uno::Any& rElement ) /// event (PropertyValues)
            throw(
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XNameAccess (via XNameReplace)
    /// calls getByName(sal_uInt16)
    virtual ::com::sun::star::uno::Any SAL_CALL getByName(
        const OUString& rName )  /// API name of event
            throw(
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XNameAxcess (via XNameReplace)
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
                                                        getElementNames()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XNameAccess (via XNameReplace)
    virtual sal_Bool SAL_CALL hasByName(
        const OUString& rName )
            throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XElementAccess (via XNameReplace)
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XElementAccess (via XNameReplace)
    virtual sal_Bool SAL_CALL hasElements()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    /// must be implemented in subclass
    virtual OUString SAL_CALL getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override = 0;

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames()
            throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

protected:

    /// Must be implemented in subclass.
    virtual void replaceByName(
        const sal_uInt16 nEvent,        /// item ID of event
        const SvxMacro& rMacro)     /// event (will be copied)
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
    sal_uInt16 mapNameToEventID(const OUString& rName) const;

    /// get the event ID for the name; return 0 if not supported
    sal_uInt16 getMacroID(const OUString& rName) const;

    /// create PropertyValues and Any from macro
    void getAnyFromMacro(
        ::com::sun::star::uno::Any& aAny,   // Any to be filled by Macro values
        const SvxMacro& rMacro);

    /// create macro from PropertyValues (in an Any)
    void getMacroFromAny(
        SvxMacro& aMacro,       // reference to be filled by Any
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
 * 2) supply the super class constructor with a reference of its parent object
 *    (to prevent destruction)
 * 3) implement getItem() and setItem(...) methods.
 *
 * If no object is available to which the SvEventDescriptor can attach itself,
 * the class SvDetachedEventDescriptor should be used.
 */
class SVT_DLLPUBLIC SvEventDescriptor : public SvBaseEventDescriptor
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
        const sal_uInt16 nEvent,        /// item ID of event
        const SvxMacro& rMacro)     /// event (will be copied)
            throw(
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException) override;

    using SvBaseEventDescriptor::getByName;
    virtual void getByName(
        SvxMacro& rMacros,          /// macro to be filled with values
        const sal_uInt16 nEvent )       /// item ID of event
            throw(
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException) override;


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
class SVT_DLLPUBLIC SvDetachedEventDescriptor : public SvBaseEventDescriptor
{
    // the macros; aMacros[i] is the value for aSupportedMacroItemIDs[i]
    SvxMacro** aMacros;

    const OUString sImplName;

public:

     SvDetachedEventDescriptor(const SvEventDescription* pSupportedMacroItems);

    virtual ~SvDetachedEventDescriptor();

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

protected:

    sal_Int16 getIndex(const sal_uInt16 nID) const;

    using SvBaseEventDescriptor::replaceByName;
    virtual void replaceByName(
        const sal_uInt16 nEvent,        /// item ID of event
        const SvxMacro& rMacro)     /// event (will be copied)
            throw(
                ::com::sun::star::lang::IllegalArgumentException,
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException) override;

    using SvBaseEventDescriptor::getByName;
    virtual void getByName(
        SvxMacro& rMacro,           /// macro to be filled
        const sal_uInt16 nEvent )       /// item ID of event
            throw(
                ::com::sun::star::container::NoSuchElementException,
                ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException) override;

    /// do we have an event?
    /// return true: we have a macro for the event
    /// return false: no macro; getByName() will return an empty macro
    /// IllegalArgumentException: the event is not supported
    bool hasById(
        const sal_uInt16 nEvent ) const     /// item ID of event
             throw(
                ::com::sun::star::lang::IllegalArgumentException);

};

class SVT_DLLPUBLIC SvMacroTableEventDescriptor : public SvDetachedEventDescriptor
{
public:

     SvMacroTableEventDescriptor(const SvEventDescription* pSupportedMacroItems);
     SvMacroTableEventDescriptor(const SvxMacroTableDtor& aFmt,
                                const SvEventDescription* pSupportedMacroItems);

    virtual ~SvMacroTableEventDescriptor();

    void copyMacrosFromTable(const SvxMacroTableDtor& aFmt);
    void copyMacrosIntoTable(SvxMacroTableDtor& aFmt);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
