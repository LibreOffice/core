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
 * The Initial Developer of the Original Code is
 *       Christian Lippka <cl@lippka.com>
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Major Contributor(s):
 *
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <com/sun/star/registry/XRegistryKey.hpp>
#include "sal/types.h"
#include "cppuhelper/factory.hxx"
#include <cppuhelper/implementationentry.hxx>

using rtl::OUString;
using namespace com::sun::star;

namespace cui {
extern rtl::OUString SAL_CALL ColorPicker_getImplementationName();
extern uno::Reference< uno::XInterface > SAL_CALL ColorPicker_createInstance( uno::Reference< uno::XComponentContext > const & ) SAL_THROW( (uno::Exception) );
extern uno::Sequence< rtl::OUString > SAL_CALL ColorPicker_getSupportedServiceNames() throw( uno::RuntimeException );
}

namespace
{

    cppu::ImplementationEntry entries[] = {
        { &::cui::ColorPicker_createInstance, &::cui::ColorPicker_getImplementationName, &::cui::ColorPicker_getSupportedServiceNames, &cppu::createSingleComponentFactory, 0, 0 },
        { 0, 0, 0, 0, 0, 0 }
    };
}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL cui_component_getFactory( char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(implName, serviceManager, registryKey, entries);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
