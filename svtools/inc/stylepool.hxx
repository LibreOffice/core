/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stylepool.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 15:24:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef INCLUDED_SVTOOLS_STYLEPOOL_HXX
#define INCLUDED_SVTOOLS_STYLEPOOL_HXX

#include <boost/shared_ptr.hpp>
#include <rtl/ustring.hxx>

#ifndef _SFXITEMSET_HXX
#include "itemset.hxx"
#endif

class StylePoolImpl;
class StylePoolIterImpl;
class IStylePoolIteratorAccess;

class SVL_DLLPUBLIC StylePool
{
private:
    StylePoolImpl *pImpl;
public:
    typedef boost::shared_ptr<SfxItemSet> SfxItemSet_Pointer_t;

    StylePool();

    /** Insert a SfxItemSet into the style pool.

        The pool makes a copy of the provided SfxItemSet.

        @param SfxItemSet
        the SfxItemSet to insert

        @return a shared pointer to the SfxItemSet
    */
    virtual SfxItemSet_Pointer_t insertItemSet( const SfxItemSet& rSet );

    /** Create an iterator

        The iterator walks through the StylePool

        @attention every change, e.g. destruction, of the StylePool could cause undefined effects.

        @postcond the iterator "points before the first" SfxItemSet of the pool.
        The first StylePoolIterator::getNext() call will deliver the first SfxItemSet.
    */
    virtual IStylePoolIteratorAccess* createIterator();

    /** Returns the number of styles
    */
    virtual sal_Int32 getCount() const;

    virtual ~StylePool();

    static ::rtl::OUString nameOf( SfxItemSet_Pointer_t pSet );
};

class SVL_DLLPUBLIC IStylePoolIteratorAccess
{
public:
    /** Delivers a shared pointer to the next SfxItemSet of the pool
        If there is no more SfxItemSet, the delivered share_pointer is empty.
    */
    virtual StylePool::SfxItemSet_Pointer_t getNext() = 0;
    virtual ::rtl::OUString getName() = 0;
    virtual ~IStylePoolIteratorAccess() {};
};

#endif
