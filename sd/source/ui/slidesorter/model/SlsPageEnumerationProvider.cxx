/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
    bool operator() (const SharedPageDescriptor& rpDescriptor) const
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
        return rpDescriptor->HasState(PageDescriptor::ST_Selected);
    }
};




class VisiblePagesPredicate
{
public:
    bool operator() (const SharedPageDescriptor& rpDescriptor)
    {
        return rpDescriptor->HasState(PageDescriptor::ST_Visible);
    }
};

}




PageEnumeration PageEnumerationProvider::CreateAllPagesEnumeration (
    const SlideSorterModel& rModel)
{
    return PageEnumeration::Create(rModel, AllPagesPredicate());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
