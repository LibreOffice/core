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

#pragma once

#include <model/SlsEnumeration.hxx>
#include <model/SlsSharedPageDescriptor.hxx>

#include <functional>
#include <memory>

namespace sd::slidesorter::model
{
class SlideSorterModel;

/** Public class of page enumerations that delegates its calls to an
    implementation object that can filter pages by using a given predicate.

    @see PageEnumerationProvider
        The PageEnumerationProvider has methods for creating different types
        of page enumerations.
*/
class PageEnumeration : public Enumeration<SharedPageDescriptor>
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
    typedef ::std::function<bool(const SharedPageDescriptor&)> PagePredicate;
    static PageEnumeration Create(const SlideSorterModel& rModel, const PagePredicate& rPredicate);

    /** This copy constructor creates a copy of the given enumeration.
    */
    PageEnumeration(const PageEnumeration& rEnumeration);

    virtual ~PageEnumeration() override;

    /** Create and return an exact copy of the called object.
    */
    virtual ::std::unique_ptr<Enumeration<SharedPageDescriptor>> Clone() override;

    PageEnumeration& operator=(const PageEnumeration& rEnumeration);

    /** Return <TRUE/> when the enumeration has more elements, i.e. it is
        save to call GetNextElement() at least one more time.
    */
    virtual bool HasMoreElements() const override;

    /** Return the next element of the enumeration.  Call the
        HasMoreElements() before to make sure that there exists at least one
        more element.  Calling this method with HasMoreElements() returning
        <FALSE/> is an error.
    */
    virtual SharedPageDescriptor GetNextElement() override;

    /** Rewind the enumeration so that the next call to GetNextElement()
        will return its first element.
    */
    virtual void Rewind() override;

private:
    /// Implementation object.
    ::std::unique_ptr<Enumeration<SharedPageDescriptor>> mpImpl;

    /** This constructor expects an implementation object that holds
        the predicate that filters the pages.
    */
    PageEnumeration(::std::unique_ptr<Enumeration<SharedPageDescriptor>>&& pImpl);
};

} // end of namespace ::sd::slidesorter::model

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
