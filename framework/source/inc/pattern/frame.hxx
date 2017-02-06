/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_FRAMEWORK_SOURCE_INC_PATTERN_FRAME_HXX
#define INCLUDED_FRAMEWORK_SOURCE_INC_PATTERN_FRAME_HXX

#include <general.h>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/CloseVetoException.hpp>
#include <com/sun/star/util/XCloseable.hpp>

// namespaces

namespace framework{
    namespace pattern{
        namespace frame{

/** @short  close (or dispose) the given resource.

    @descr  It try to close the given resource first.
            Delegating of the ownership can be influenced from
            outside. If closing isn't possible (because the
            needed interface isn't available) dispose() is tried instead.
            Al possible exception are handled inside.
            So the user of this method has to look for the return value only.

    @attention  The given resource will not be cleared.
                But later using of it can produce an exception!

    @param  xResource
            the object, which should be closed here.

    @param  bDelegateOwnership
            used at the XCloseable->close() method to define
            the right owner in case closing failed.

    @return [bool]
            sal_True if closing failed.
 */
inline bool closeIt(const css::uno::Reference< css::uno::XInterface >& xResource         ,
                       bool                                     bDelegateOwnership)
{
    css::uno::Reference< css::util::XCloseable > xClose  (xResource, css::uno::UNO_QUERY);
    css::uno::Reference< css::lang::XComponent > xDispose(xResource, css::uno::UNO_QUERY);

    try
    {
        if (xClose.is())
            xClose->close(bDelegateOwnership);
        else
        if (xDispose.is())
            xDispose->dispose();
        else
            return false;
    }
    catch(const css::util::CloseVetoException&)
        { return false; }
    catch(const css::lang::DisposedException&)
        {} // disposed is closed is ...
    catch(const css::uno::RuntimeException&)
        { throw; } // should not be suppressed!
    catch(const css::uno::Exception&)
        { return false;  } // ??? We defined to return a boolean value instead of throwing exceptions...
                               // (OK: RuntimeExceptions should not be catched inside the core..)

    return true;
}

        } // namespace frame
    } // namespace pattern
} // namespace framework

#endif // INCLUDED_FRAMEWORK_SOURCE_INC_PATTERN_FRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
