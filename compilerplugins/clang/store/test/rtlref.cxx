/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/io/XStreamListener.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/lang/XTypeProvider.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "cppuhelper/implbase.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/ref.hxx"

class FooStream : public css::io::XStreamListener
{
    virtual ~FooStream();
};
void test(rtl::Reference<FooStream> fooStream)
{
    // expected-error@+1 {{cast from rtl::Reference<FooStream> [loplugin:rtlref]}}
    css::uno::Reference<css::io::XStreamListener> a(fooStream);
    (void)a;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
