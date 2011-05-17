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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "view/SlsPageObject.hxx"

#include "model/SlsPageDescriptor.hxx"
#include "controller/SlsPageObjectFactory.hxx"

using namespace ::sdr::contact;
using namespace ::sd::slidesorter::model;


namespace sd { namespace slidesorter { namespace view {


PageObject::PageObject (
    const Rectangle& rRectangle,
    SdrPage* _pPage,
    const SharedPageDescriptor& rpDescriptor)
    : SdrPageObj(rRectangle, _pPage),
      mpDescriptor(rpDescriptor)
{
}




PageObject::~PageObject (void)
{
}




SharedPageDescriptor PageObject::GetDescriptor (void) const
{
    return mpDescriptor;
}




sdr::contact::ViewContact* PageObject::CreateObjectSpecificViewContact()
{
    if (mpDescriptor.get() != NULL)
        return mpDescriptor->GetPageObjectFactory().CreateViewContact(this, mpDescriptor);
    else
        return NULL;
}



} } } // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
