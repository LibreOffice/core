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

#ifndef INCLUDED_EDITENG_ACCESSIBLECONTEXTBASE_HXX
#define INCLUDED_EDITENG_ACCESSIBLECONTEXTBASE_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleStateSet.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <editeng/editengdllapi.h>


namespace accessibility {

struct MutexOwner {mutable ::osl::Mutex maMutex;};

/** @descr
        This base class provides an implementation of the
        AccessibleContext service. Apart from the
        <type>XXAccessible<type> and XAccessibleContextContext
        interfaces it supports the XServiceInfo interface.
*/
class EDITENG_DLLPUBLIC AccessibleContextBase
    :   public MutexOwner,
        public cppu::WeakComponentImplHelper<
        ::com::sun::star::accessibility::XAccessible,
        ::com::sun::star::accessibility::XAccessibleContext,
        ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
        ::com::sun::star::lang::XServiceInfo
        >
{
public:

    //=====  internal  ========================================================

    /** The origin of the accessible name or description.
    */
    enum StringOrigin {
        ManuallySet,
        FromShape,
        AutomaticallyCreated,
        NotSet
    };

    AccessibleContextBase (
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent,
        const sal_Int16 aRole);
    virtual ~AccessibleContextBase();


    /** Call all accessiblity event listeners to inform them about the
        specified event.
        @param aEventId
            Id of the event type.
        @param rNewValue
            New value of the modified attribute.  Pass empty structure if
            not applicable.
        @param rOldValue
            Old value of the modified attribute.  Pass empty structure if
            not applicable.
    */
    void CommitChange (sal_Int16 aEventId,
        const ::com::sun::star::uno::Any& rNewValue,
        const ::com::sun::star::uno::Any& rOldValue);

    /** Set a new description and, provided that the new name differs from
        the old one, broadcast an accessibility event.
        @param rsDescription
            The new description.
        @param eDescriptionOrigin
            The origin of the description.  This is used to determine
            whether the given description overrules the existing one.  An
            origin with a lower numerical value overrides one with a higher
            value.
    */
    void SetAccessibleDescription (
        const OUString& rsDescription,
        StringOrigin eDescriptionOrigin)
        throw (::com::sun::star::uno::RuntimeException);

    /** Set a new description and, provided that the new name differs from
        the old one, broadcast an accessibility event.
        @param rsName
            The new name.
        @param eNameOrigin
            The origin of the name.  This is used to determine whether the
            given name overrules the existing one.  An origin with a lower
            numerical value overrides one with a higher value.
    */
    void SetAccessibleName (
        const OUString& rsName,
        StringOrigin eNameOrigin)
        throw (::com::sun::star::uno::RuntimeException);

    /** Set the specified state (turn it on) and send events to all
        listeners to inform them of the change.

        @param aState
            The state to turn on.

        @return
            If the specified state changed its value due to this call
            <TRUE/> is returned, otherwise <FALSE/>.
    */
    virtual bool SetState (sal_Int16 aState);

    /** Reset the specified state (turn it off) and send events to all
        listeners to inform them of the change.

        @param aState
            The state to turn off.

        @return
            If the specified state changed its value due to this call
            <TRUE/> is returned, otherwise <FALSE/>.
    */
    virtual bool ResetState (sal_Int16 aState);

    /** Return the state of the specified state.

        @param aState
            The state for which to return its value.

        @return
            A value of <TRUE/> indicates that the state is set.  A <FALSE/>
            value indicates an unset state.
    */
    bool GetState (sal_Int16 aState);

    /** Replace the current relation set with the specified one.  Send
        events for relations that are not in both sets.

        @param rRelationSet
            The new relation set that replaces the old one.
    */
    void SetRelationSet (
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleRelationSet>& rxRelationSet)
        throw (::com::sun::star::uno::RuntimeException);


    //=====  XAccessible  =====================================================

    /// Return the XAccessibleContext.
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL
        getAccessibleContext() throw (::com::sun::star::uno::RuntimeException, std::exception) override;


    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
    virtual sal_Int32 SAL_CALL
        getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return the specified child or throw exception.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleChild (sal_Int32 nIndex)
        throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return a reference to the parent.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible> SAL_CALL
        getAccessibleParent()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return this objects index among the parents children.
    virtual sal_Int32 SAL_CALL
        getAccessibleIndexInParent()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return this object's role.
    virtual sal_Int16 SAL_CALL
        getAccessibleRole()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return this object's description.
    virtual OUString SAL_CALL
        getAccessibleDescription()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return the object's current name.
    virtual OUString SAL_CALL
        getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return NULL to indicate that an empty relation set.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleRelationSet> SAL_CALL
        getAccessibleRelationSet()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Return the set of current states.
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleStateSet> SAL_CALL
        getAccessibleStateSet()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Return the parents locale or throw exception if this object has no
        parent yet/anymore.
    */
    virtual ::com::sun::star::lang::Locale SAL_CALL
        getLocale()
        throw (::com::sun::star::uno::RuntimeException,
            ::com::sun::star::accessibility::IllegalAccessibleComponentStateException, std::exception) override;

    //=====  XAccessibleEventBroadcaster  ========================================

    virtual void SAL_CALL
        addAccessibleEventListener (
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL
        removeAccessibleEventListener (
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener >& xListener)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;


    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
    virtual OUString SAL_CALL
        getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Return whether the specified service is supported by this class.
    */
    virtual sal_Bool SAL_CALL
        supportsService (const OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Returns a list of all supported services.  In this case that is just
        the AccessibleContext service.
    */
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL
        getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;


    //=====  XTypeProvider  ===================================================

    /** Returns a sequence of all supported interfaces.
    */
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL
        getTypes()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    /** Returns a implementation id.
    */
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    /** The state set.
    */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet> mxStateSet;

    /** The relation set.  Relations can be set or removed by calling the
        <member>AddRelation</member> and <member>RemoveRelation</member> methods.
    */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleRelationSet> mxRelationSet;

    // This method is called from the component helper base class while disposing.
    virtual void SAL_CALL disposing() override;

    /** Create the accessible object's name.  This method may be called more
        than once for a single object.
        @return
            The returned string is a unique (among the accessible object's
            siblings) name.
    */
    virtual OUString CreateAccessibleName()
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    /** Create the accessible object's descriptive string.  May be called
        more than once.
        @return
            Descriptive string.  Not necessarily unique.
    */
    virtual OUString
        CreateAccessibleDescription()
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    void FireEvent (const ::com::sun::star::accessibility::AccessibleEventObject& aEvent);

    /** Check whether or not the object has been disposed (or is in the
        state of being disposed).  If that is the case then
        DisposedException is thrown to inform the (indirect) caller of the
        foul deed.
    */
    void ThrowIfDisposed()
        throw (::com::sun::star::lang::DisposedException);

    /** Check whether or not the object has been disposed (or is in the
        state of being disposed).

        @return TRUE, if the object is disposed or in the course
        of being disposed. Otherwise, FALSE is returned.
    */
    bool IsDisposed();

    /** sets the role as returned by XaccessibleContext::getAccessibleRole

        <p>Caution: This is only to be used in the construction phase (means within
        the ctor or late ctor), <em>never</em> when the object is still alive and part
        of an Accessibility hierarchy.</p>
    */
    void SetAccessibleRole( sal_Int16 _nRole );

private:
    /// Reference to the parent object.
    ::com::sun::star::uno::Reference<
         ::com::sun::star::accessibility::XAccessible> mxParent;

    /** Description of this object.  This is not a constant because it can
        be set from the outside.  Furthermore, it changes according to the
        draw page's display mode.
    */
    OUString msDescription;

    /** The origin of the description is used to determine whether new
        descriptions given to the SetAccessibleDescription is ignored or
        whether that replaces the old value in msDescription.
    */
    StringOrigin meDescriptionOrigin;

    /** Name of this object.  It changes according the draw page's
        display mode.
    */
    OUString msName;

    /** The origin of the name is used to determine whether new
        name given to the SetAccessibleName is ignored or
        whether that replaces the old value in msName.
    */
    StringOrigin meNameOrigin;

    /** client id in the AccessibleEventNotifier queue
    */
    sal_uInt32 mnClientId;

    /** This is the role of this object.
    */
    sal_Int16 maRole;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
