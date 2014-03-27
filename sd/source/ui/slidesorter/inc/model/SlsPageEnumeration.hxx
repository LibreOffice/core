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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_MODEL_SLSPAGEENUMERATION_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_MODEL_SLSPAGEENUMERATION_HXX

#include <sal/types.h>

#include "pres.hxx"

#include "model/SlsEnumeration.hxx"
#include "model/SlsSharedPageDescriptor.hxx"

#include <boost/function.hpp>
#include <memory>

namespace sd { namespace slidesorter { namespace model {

class SlideSorterModel;


/** Public class of page enumerations that delegates its calls to an
    implementation object that can filter pages by using a given predicate.

    @see PageEnumerationProvider
        The PageEnumerationProvider has methods for creating different types
        of page enumerations.
*/
class PageEnumeration
    : public Enumeration<SharedPageDescriptor>
{
public:
    /** Create a new page enumeration that enumerates a subset of the pages
        of the given model.
        @param rModel
            The new page enumeration enumerates the pages of this model.
        @param rPredicate
            This predicate determines which pages to include in the
            enumeration.  Pages for which rPredicate returns <FALSE/> are
            exclude.
    */
    typedef ::boost::function<bool(const SharedPageDescriptor&)> PagePredicate;
    static PageEnumeration Create (
        const SlideSorterModel& rModel,
        const PagePredicate& rPredicate);

    /** This copy constructor creates a copy of the given enumeration.
    */
    PageEnumeration (const PageEnumeration& rEnumeration);

    virtual ~PageEnumeration();

    /** Create a new enumeration object.  The ownership of the
        implementation object goes to the new object.  Use this copy
        constructor only when you know what you are doing.  When in doubt,
        use the one argument version.
        @param bCloneImpl
            When <TRUE/> is given this constructor behaves exactly like its
            one argument version.  When <FALSE/> is given then the
            implementation object is not copied but moved from the given
            enumeration to the newly created one.  The given enumeration
            thus becomes empty.
    */
    PageEnumeration (PageEnumeration& rEnumeration, bool bCloneImpl);

    /** Create and return an exact copy of the called object.
    */
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    virtual ::std::auto_ptr<Enumeration<SharedPageDescriptor> > Clone (void) SAL_OVERRIDE;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    PageEnumeration& operator= (const PageEnumeration& rEnumeration);

    /** Return <TRUE/> when the enumeration has more elements, i.e. it is
        save to call GetNextElement() at least one more time.
    */
    virtual bool HasMoreElements (void) const SAL_OVERRIDE;

    /** Return the next element of the enumeration.  Call the
        HasMoreElements() before to make sure that there exists at least one
        more element.  Calling this method with HasMoreElements() returning
        <FALSE/> is an error.
    */
    virtual SharedPageDescriptor GetNextElement (void) SAL_OVERRIDE;

    /** Rewind the enumeration so that the next call to GetNextElement()
        will return its first element.
    */
    virtual void Rewind (void) SAL_OVERRIDE;

private:
    /// Implementation object.
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<Enumeration<SharedPageDescriptor> > mpImpl;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    /** This constructor expects an implementation object that holds
        the predicate that filters the pages.
    */
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    PageEnumeration (::std::auto_ptr<Enumeration<SharedPageDescriptor> > pImpl);
    SAL_WNODEPRECATED_DECLARATIONS_POP

    // Default constructor not implemented.
    PageEnumeration (void);
};

} } } // end of namespace ::sd::slidesorter::model

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
