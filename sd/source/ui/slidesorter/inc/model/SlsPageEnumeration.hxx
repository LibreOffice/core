/*************************************************************************
 *
 *  $RCSfile: SlsPageEnumeration.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:22:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SD_SLIDESORTER_PAGE_ENUMERATION_HXX
#define SD_SLIDESORTER_PAGE_ENUMERATION_HXX

#include "pres.hxx"

#include "memory"

#include "SlsEnumeration.hxx"


namespace sd { namespace slidesorter { namespace model {

class PageDescriptor;
class SlideSorterModel;


/** Public class of page enumerations that delegates its calls to an
    implementation object that can filter pages by using one of several
    predicates.
*/
class PageEnumeration
    : public Enumeration<PageDescriptor>
{
public:
    /** The type of the predicate that is used to filter pages from the set
        of all pages provided by the slide sorter model:
            PET_ALL enumerates all pages.
            PET_SELECTED gives access to only the selected pages.
            PET_VISIBLE gives access to only the visible pages.
    */
    enum PageEnumerationType { PET_ALL, PET_SELECTED, PET_VISIBLE };

    /** Create a new page enumeration that enumerates a subset of the pages
        of the given model.
        @param rModel
            The new page enumeration enumerates the pages of this model.
        @param eType
            This value determines which predicate/filter to use.  See the
            PageEnumerationType enum above for a description of the
            available values.
    */
    static PageEnumeration Create (
        const SlideSorterModel& rModel,
        PageEnumerationType eType);

    /** This constructor expects an implementation object that implements
        the predicate that filters the pages.  If you do not know where to
        get such an object from you may want to use the static Create()
        factory method for creating a page enumeration.  Otherwise you may
        want to use that method as well.
    */
    PageEnumeration (::std::auto_ptr<Enumeration<PageDescriptor> > pImpl);

    /** This copy constructor creates a copy of the given enumeration.  This
        new enumeration points to the same element as the given one.
    */
    PageEnumeration (const PageEnumeration& rEnumeration);

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
    virtual PageEnumeration* Clone (void);

    PageEnumeration& operator= (const PageEnumeration& rEnumeration);

    /** Return <TRUE/> when the enumeration has more elements, i.e. it is
        save to call GetNextElement() at least one more time.
    */
    virtual bool HasMoreElements (void) const;

    /** Return the next element of the enumeration.  Call the
        HasMoreElements() before to make sure that there exists at least one
        more element.  Calling this method with HasMoreElements() returning
        <FALSE/> is an error.
    */
    virtual PageDescriptor& GetNextElement (void);

    /** Rewind the enumeration so that the next call to GetNextElement()
        will return its first element.
    */
    virtual void Rewind (void);

private:
    /// Implementation object.
    ::std::auto_ptr<Enumeration<PageDescriptor> > mpImpl;

    // Default constructor not implemented.
    PageEnumeration (void);
};

} } } // end of namespace ::sd::slidesorter::model

#endif
