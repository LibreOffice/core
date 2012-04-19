/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* genericfilter: mostly generic code for registering the filter
 *
 * Portions of this code Copyright 2000 by Sun Microsystems, Inc.
 * Rest is Copyright (C) 2002 William Lachance (wlach@interlog.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include "sal/config.h"

#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "sal/types.h"

#include "CDRImportFilter.hxx"
#include "CMXImportFilter.hxx"
#include "VisioImportFilter.hxx"
#include "WPGImportFilter.hxx"

namespace {

static cppu::ImplementationEntry const services[] = {
    { &CDRImportFilter_createInstance, &CDRImportFilter_getImplementationName,
      &CDRImportFilter_getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { &CMXImportFilter_createInstance, &CMXImportFilter_getImplementationName,
      &CMXImportFilter_getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { &VisioImportFilter_createInstance,
      &VisioImportFilter_getImplementationName,
      &VisioImportFilter_getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { &WPGImportFilter_createInstance, &WPGImportFilter_getImplementationName,
      &WPGImportFilter_getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 } };

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL wpftdraw_component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, services);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
