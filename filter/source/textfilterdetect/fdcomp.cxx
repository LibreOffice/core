/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <stdio.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>

#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include "filterdetect.hxx"

using namespace ::cppu;
using namespace com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C" {

SAL_DLLPUBLIC_EXPORT void * SAL_CALL textfd_component_getFactory(
    const sal_Char* pImplName, void* pServiceManager, void* /* pRegistryKey */ )
{
    void* pRet = NULL;
    rtl::OUString implName = rtl::OUString::createFromAscii(pImplName);
    if (pServiceManager && implName == PlainTextFilterDetect_getImplementationName())
    {
        uno::Reference<lang::XSingleServiceFactory> xFactory(
            createSingleFactory(
            reinterpret_cast<lang::XMultiServiceFactory*>(pServiceManager),
            implName,
            PlainTextFilterDetect_createInstance, PlainTextFilterDetect_getSupportedServiceNames()));

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
