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
#include <cppuhelper/implbase.hxx>
#include <vector>
#include <memory>

namespace com :: sun :: star :: uno { class XInterface; }

class SvxMacroTableDtor;
class SvxMacroItem;
class SvxMacro;
enum class SvMacroItemId : sal_uInt16;

/** SvEventDescription: Description of a single event.
    mnEvent is the id used by SvxMacroItem
    mpEventName is the api name for this event

    the last event in an array is indicated by mnEvent && mpEventName == 0
*/
struct SvEventDescription
{
    SvMacroItemId const mnEvent;
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
class SVT_DLLPUBLIC SvBaseEventDescriptor : public cppu::WeakImplHelper
<
    css::container::XNameReplace,
    css::lang::XServiceInfo
>
{
protected:
    /// last element is 0, 0
    const SvEventDescription* mpSupportedMacroItems;
    sal_Int16 mnMacroItems;

public:

     SvBaseEventDescriptor(const SvEventDescription* pSupportedMacroItems);

    virtual ~SvBaseEventDescriptor() override;


    // XNameReplace
    /// calls replaceByName(const sal_uInt16, const SvxMacro&)
    virtual void SAL_CALL replaceByName(
        const OUString& rName,                /// API name of event
        const css::uno::Any& rElement ) /// event (PropertyValues)
             override;

    // XNameAccess (via XNameReplace)
    /// calls getByName(sal_uInt16)
    virtual css::uno::Any SAL_CALL getByName(
        const OUString& rName )  /// API name of event
             override;

    // XNameAxcess (via XNameReplace)
    virtual css::uno::Sequence< OUString > SAL_CALL
                                                        getElementNames() override;

    // XNameAccess (via XNameReplace)
    virtual sal_Bool SAL_CALL hasByName(
        const OUString& rName ) override;

    // XElementAccess (via XNameReplace)
    virtual css::uno::Type SAL_CALL getElementType() override;

    // XElementAccess (via XNameReplace)
    virtual sal_Bool SAL_CALL hasElements() override;

    // XServiceInfo
    /// must be implemented in subclass
    virtual OUString SAL_CALL getImplementationName() override = 0;

    // XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;

    // XServiceInfo
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

protected:

    /// Must be implemented in subclass.
    ///
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::container::NoSuchElementException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual void replaceByName(
        const SvMacroItemId nEvent,  /// item ID of event
        const SvxMacro& rMacro)      /// event (will be copied)
             = 0;

    /// Must be implemented in subclass.
    ///
    /// @throws css::container::NoSuchElementException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    virtual void getByName(
        SvxMacro& rMacro,
        const SvMacroItemId nEvent ) = 0;

    /// convert an API event name to the event ID as used by SvxMacroItem
    SvMacroItemId mapNameToEventID(const OUString& rName) const;

    /// get the event ID for the name; return 0 if not supported
    SvMacroItemId getMacroID(const OUString& rName) const;
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
    css::uno::Reference< css::uno::XInterface > xParentRef;

public:

     SvEventDescriptor(css::uno::XInterface& rParent,
                      const SvEventDescription* pSupportedMacroItems);

    virtual ~SvEventDescriptor() override;


protected:


    using SvBaseEventDescriptor::replaceByName;
    virtual void replaceByName(
        const SvMacroItemId nEvent,   /// item ID of event
        const SvxMacro& rMacro)       /// event (will be copied)
             override;

    using SvBaseEventDescriptor::getByName;
    virtual void getByName(
        SvxMacro& rMacros,            /// macro to be filled with values
        const SvMacroItemId nEvent )  /// item ID of event
             override;


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
    std::vector<std::unique_ptr<SvxMacro>> aMacros;

public:

    SvDetachedEventDescriptor(const SvEventDescription* pSupportedMacroItems);
    SvDetachedEventDescriptor& operator=( SvDetachedEventDescriptor const & ) = delete; // MSVC2015 workaround
    SvDetachedEventDescriptor( SvDetachedEventDescriptor const & ) = delete; // MSVC2015 workaround

    virtual ~SvDetachedEventDescriptor() override;

    //XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

protected:

    sal_Int16 getIndex(const SvMacroItemId nID) const;

    using SvBaseEventDescriptor::replaceByName;
    virtual void replaceByName(
        const SvMacroItemId nEvent,  /// item ID of event
        const SvxMacro& rMacro)      /// event (will be copied)
             override;

    using SvBaseEventDescriptor::getByName;
    virtual void getByName(
        SvxMacro& rMacro,             /// macro to be filled
        const SvMacroItemId nEvent )  /// item ID of event
             override;

    /// do we have an event?
    /// return true: we have a macro for the event
    /// return false: no macro; getByName() will return an empty macro
    /// @throws css::lang::IllegalArgumentException if the event is not supported
    bool hasById(
        const SvMacroItemId nEvent ) const;     /// item ID of event

};

class SVT_DLLPUBLIC SvMacroTableEventDescriptor : public SvDetachedEventDescriptor
{
public:

     SvMacroTableEventDescriptor(const SvEventDescription* pSupportedMacroItems);
     SvMacroTableEventDescriptor(const SvxMacroTableDtor& aFmt,
                                const SvEventDescription* pSupportedMacroItems);

    virtual ~SvMacroTableEventDescriptor() override;

    void copyMacrosIntoTable(SvxMacroTableDtor& aFmt);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
