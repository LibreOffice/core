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


#include "view/SlsFontProvider.hxx"

#include "controller/SlideSorterController.hxx"

#include <sfx2/app.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>


using namespace ::sd::slidesorter;

namespace sd { namespace slidesorter { namespace view {

FontProvider* FontProvider::mpInstance = NULL;

FontProvider& FontProvider::Instance (void)
{
    if (mpInstance == NULL)
    {
        ::osl::GetGlobalMutex aMutexFunctor;
        ::osl::MutexGuard aGuard (aMutexFunctor());
        if (mpInstance == NULL)
        {
            // Create an instance of the class and register it at the
            // SdGlobalResourceContainer so that it is eventually released.
            FontProvider* pInstance = new FontProvider();
            SdGlobalResourceContainer::Instance().AddResource (
                ::std::auto_ptr<SdGlobalResource>(pInstance));
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            mpInstance = pInstance;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }

    // We throw an exception when for some strange reason no instance of
    // this class exists.
    if (mpInstance == NULL)
        throw ::com::sun::star::uno::RuntimeException("com.sun.star.document.IndexedPropertyValues",
            NULL);

    return *mpInstance;
}




FontProvider::FontProvider (void)
    : maFont(),
      maMapMode()
{
}




FontProvider::~FontProvider (void)
{
}




void FontProvider::Invalidate (void)
{
    maFont.reset();
}



} } }  // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
