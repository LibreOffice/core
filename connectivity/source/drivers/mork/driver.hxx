/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_DRIVER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_DRIVER_HXX

#include "sal/config.h"

#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "sal/types.h"

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{
class XComponentContext;
class XInterface;
}
}
}
}
namespace rtl
{
class OUString;
}

namespace connectivity
{
namespace mork
{

com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL
create(
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
    const &);

rtl::OUString SAL_CALL getImplementationName();

com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
getSupportedServiceNames();

}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
