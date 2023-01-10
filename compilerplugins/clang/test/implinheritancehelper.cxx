/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase.hxx>
#include "com/sun/star/beans/XProperty.hpp"

class VCLXDevice : public cppu::WeakImplHelper<css::lang::XUnoTunnel>
{
};

// expected-error@+1 {{can probably use ImplInheritanceHelper here [loplugin:implinheritancehelper]}}
class VCLXCheckBox : public css::beans::XProperty, public VCLXDevice
{
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
