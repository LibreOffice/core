/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ControlDescriptorIterator.cxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
