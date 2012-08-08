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

#ifndef __FRAMEWORK_PATTERN_FRAME_HXX_
#define __FRAMEWORK_PATTERN_FRAME_HXX_

#include <general.h>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/XCloseable.hpp>

// namespaces

#ifndef css
namespace css = ::com::sun::star;
#endif

namespace framework{
    namespace pattern{
        namespace frame{


//-----------------------------------------------
inline css::uno::Reference< css::frame::XModel > extractFrameModel(const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    css::uno::Reference< css::frame::XModel >      xModel;
    css::uno::Reference< css::frame::XController > xController;
    if (xFrame.is())
        xController = xFrame->getController();
    if (xController.is())
        xModel = xController->getModel();
    return xModel;
}

//-----------------------------------------------
/** @short  close (or dispose) the given resource.

    @descr  It try to close the given resource first.
            Delegating of the ownership can be influenced from
            outside. If closing isnt possible (because the
            needed interface isnt available) dispose() is tried instead.
            Al possible exception are handled inside.
            So the user of this method has to look for the return value only.

    @attention  The given resource will not be cleared.
                But later using of it can produce an exception!

    @param  xResource
            the object, which should be closed here.

    @param  bDelegateOwnerShip
            used at the XCloseable->close() method to define
            the right owner in case closing failed.

    @return [bool]
            sal_True if closing failed.
 */
inline sal_Bool closeIt(const css::uno::Reference< css::uno::XInterface >& xResource         ,
                       sal_Bool                                     bDelegateOwnerShip)
{
    css::uno::Reference< css::util::XCloseable > xClose  (xResource, css::uno::UNO_QUERY);
    css::uno::Reference< css::lang::XComponent > xDispose(xResource, css::uno::UNO_QUERY);

    try
    {
        if (xClose.is())
            xClose->close(bDelegateOwnerShip);
        else
        if (xDispose.is())
            xDispose->dispose();
        else
            return sal_False;
    }
    catch(const css::util::CloseVetoException&)
        { return sal_False; }
    catch(const css::lang::DisposedException&)
        {} // disposed is closed is ...
    catch(const css::uno::RuntimeException&)
        { throw; } // shouldnt be suppressed!
    catch(const css::uno::Exception&)
        { return sal_False;  } // ??? We defined to return a boolen value instead of throwing exceptions ...
                               // (OK: RuntimeExceptions shouldnt be catched inside the core ..)

    return sal_True;
}

        } // namespace frame
    } // namespace pattern
} // namespace framework

#endif // __FRAMEWORK_PATTERN_FRAME_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
