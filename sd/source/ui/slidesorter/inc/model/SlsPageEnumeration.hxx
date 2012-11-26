/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SD_SLIDESORTER_PAGE_ENUMERATION_HXX
#define SD_SLIDESORTER_PAGE_ENUMERATION_HXX

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
            enumeration.  Pages for which rPredicate returns <false/> are
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
            When <true/> is given this constructor behaves exactly like its
            one argument version.  When <false/> is given then the
            implementation object is not copied but moved from the given
            enumeration to the newly created one.  The given enumeration
            thus becomes empty.
    */
    PageEnumeration (PageEnumeration& rEnumeration, bool bCloneImpl);

    /** Create and return an exact copy of the called object.
    */
    virtual ::std::auto_ptr<Enumeration<SharedPageDescriptor> > Clone (void);

    PageEnumeration& operator= (const PageEnumeration& rEnumeration);

    /** Return <true/> when the enumeration has more elements, i.e. it is
        save to call GetNextElement() at least one more time.
    */
    virtual bool HasMoreElements (void) const;

    /** Return the next element of the enumeration.  Call the
        HasMoreElements() before to make sure that there exists at least one
        more element.  Calling this method with HasMoreElements() returning
        <false/> is an error.
    */
    virtual SharedPageDescriptor GetNextElement (void);

    /** Rewind the enumeration so that the next call to GetNextElement()
        will return its first element.
    */
    virtual void Rewind (void);

private:
    /// Implementation object.
    ::std::auto_ptr<Enumeration<SharedPageDescriptor> > mpImpl;

    /** This constructor expects an implementation object that holds
        the predicate that filters the pages.
    */
    PageEnumeration (::std::auto_ptr<Enumeration<SharedPageDescriptor> > pImpl);

    // Default constructor not implemented.
    PageEnumeration (void);
};

} } } // end of namespace ::sd::slidesorter::model

#endif
