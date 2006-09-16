/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ControlDescriptorIterator.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:12:32 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

/*
#include "ControlDescriptorIterator.hxx"

#include "ControlContainer.hxx"
#include "ControlContainerDescriptor.hxx"

namespace sd { namespace toolpanel {


ControlDescriptorIterator::ControlDescriptorIterator (void)
    : mpContainer (NULL),
      mbSkipInvisibleControls(true)
{
}




ControlDescriptorIterator::ControlDescriptorIterator (
    ControlDescriptorList& rContainer,
    const ControlDescriptorList::iterator& aIterator,
    bool bSkipInvisibleControls)
    : mpContainer(&rContainer),
      maIterator (aIterator),
      mbSkipInvisibleControls(bSkipInvisibleControls)
{
    AdvanceToNextVisibleControl();
}




ControlDescriptorIterator::ControlDescriptorIterator (
    const ControlDescriptorIterator& aIterator)
    : mpContainer (aIterator.mpContainer),
      maIterator (aIterator.maIterator),
      mbSkipInvisibleControls (aIterator.mbSkipInvisibleControls)
{
}




ControlDescriptorIterator& ControlDescriptorIterator::operator= (
    const ControlDescriptorIterator& aIterator)
{
    mpContainer = aIterator.mpContainer;
    maIterator = aIterator.maIterator;
    mbSkipInvisibleControls = aIterator.mbSkipInvisibleControls;

    AdvanceToNextVisibleControl();

    return *this;
}




ControlDescriptorIterator::value_type&
    ControlDescriptorIterator::operator* (void)
{
    return *maIterator;
}




const ControlDescriptorIterator::value_type&
    ControlDescriptorIterator::operator* (void)
    const
{
    return *maIterator;
}




ControlDescriptorIterator::value_type&
    ControlDescriptorIterator::operator-> (void)
{
    return *maIterator;
}




const ControlDescriptorIterator::value_type&
    ControlDescriptorIterator::operator-> (void)
    const
{
    return *maIterator;
}




bool ControlDescriptorIterator::operator== (
    const ControlDescriptorIterator& aIterator) const
{
    return ! operator!=(aIterator);
}




bool ControlDescriptorIterator::operator!= (
    const ControlDescriptorIterator& aIterator) const
{
    return maIterator != aIterator.maIterator;
}




ControlDescriptorIterator& ControlDescriptorIterator::operator++ (void)
{
    maIterator++;
    AdvanceToNextVisibleControl();
    return *this;
}



ControlDescriptorIterator ControlDescriptorIterator::operator++ (int)
{
    ControlDescriptorIterator aIterator (*this);
    ++(*this);
    return aIterator;
}




ControlDescriptorIterator& ControlDescriptorIterator::operator-- (void)
{
    maIterator--;
    AdvanceToPreviousVisibleControl();
    return *this;
}



ControlDescriptorIterator ControlDescriptorIterator::operator-- (int)
{
    ControlDescriptorIterator aIterator (*this);
    --(*this);
    return aIterator;
}




ControlDescriptorIterator ControlDescriptorIterator::operator+ (
    int nValue) const
{
    return ControlDescriptorIterator (*mpContainer, maIterator+nValue);
}



ControlDescriptorIterator ControlDescriptorIterator::operator- (
    int nValue) const
{
    return ControlDescriptorIterator (*mpContainer, maIterator-nValue);
}



void ControlDescriptorIterator::AdvanceToNextVisibleControl (void)
{
    if (mbSkipInvisibleControls && mpContainer!=NULL)
    {
        while (maIterator != mpContainer->end()
            && ! (**maIterator).IsVisible())
            ++maIterator;
    }
}




void ControlDescriptorIterator::AdvanceToPreviousVisibleControl (void)
{
    if (mbSkipInvisibleControls && mpContainer!=NULL)
    {
        while (maIterator != mpContainer->begin()
            && ! (**maIterator).IsVisible())
            --maIterator;
    }
}

} } // end of namespace ::sd::toolpanel
*/
