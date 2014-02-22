/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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



} } }  

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
