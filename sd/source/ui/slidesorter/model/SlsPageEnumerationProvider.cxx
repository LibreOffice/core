/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsPageEnumerationProvider.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:41:39 $
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

#include "precompiled_sd.hxx"

#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlsPageEnumeration.hxx"
#include "model/SlsPageDescriptor.hxx"
#include <boost/function.hpp>

namespace sd { namespace slidesorter { namespace model {


namespace {

class AllPagesPredicate
{
public:
    bool operator() (const SharedPageDescriptor& rpDescriptor)
    {
        (void)rpDescriptor;
        return true;
    }
};





class SelectedPagesPredicate
{
public:
    bool operator() (const SharedPageDescriptor& rpDescriptor)
    {
        return rpDescriptor->IsSelected();
    }
};




class VisiblePagesPredicate
{
public:
    bool operator() (const SharedPageDescriptor& rpDescriptor)
    {
        return rpDescriptor->IsVisible();
    }
};

}




PageEnumeration PageEnumerationProvider::CreateAllPagesEnumeration (
    const SlideSorterModel& rModel)
{
    AllPagesPredicate aPredicate;
    return PageEnumeration::Create(rModel, aPredicate);
}




PageEnumeration PageEnumerationProvider::CreateSelectedPagesEnumeration (
    const SlideSorterModel& rModel)
{
    return PageEnumeration::Create(
        rModel,
        SelectedPagesPredicate());
}




PageEnumeration PageEnumerationProvider::CreateVisiblePagesEnumeration (
    const SlideSorterModel& rModel)
{
    return PageEnumeration::Create(
        rModel,
        VisiblePagesPredicate());
}


} } } // end of namespace ::sd::slidesorter::model
