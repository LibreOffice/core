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

#include <classes/taskcreator.hxx>
#include <framework/taskcreatorsrv.hxx>
#include <services.h>
#include <taskcreatordefs.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <utility>

namespace framework{

/*-****************************************************************************************************
    @short      initialize instance with necessary information
    @descr      We need a valid uno service manager to create or instantiate new services.
                All other information to create frames or tasks come in on right interface methods.

    @param      xContext
                    points to the valid uno service manager
*//*-*****************************************************************************************************/
TaskCreator::TaskCreator( css::uno::Reference< css::uno::XComponentContext > xContext )
    : m_xContext    (std::move( xContext ))
{
}

/*-****************************************************************************************************
    @short      deinitialize instance
    @descr      We should release all used resource which are not needed any longer.
*//*-*****************************************************************************************************/
TaskCreator::~TaskCreator()
{
}

/*-****************************************************************************************************
    TODO document me
*//*-*****************************************************************************************************/
css::uno::Reference< css::frame::XFrame2 > TaskCreator::createTask( const OUString& sName, const utl::MediaDescriptor& rDescriptor )
{
    rtl::Reference< TaskCreatorService > xCreator = new TaskCreatorService(m_xContext);

    css::uno::Sequence< css::uno::Any > lArgs
    {
        css::uno::Any(css::beans::NamedValue(ARGUMENT_PARENTFRAME, css::uno::Any(css::uno::Reference< css::frame::XFrame >( css::frame::Desktop::create( m_xContext ), css::uno::UNO_QUERY_THROW)))) ,
        css::uno::Any(css::beans::NamedValue(ARGUMENT_CREATETOPWINDOW, css::uno::Any(true))),
        css::uno::Any(css::beans::NamedValue(ARGUMENT_MAKEVISIBLE, css::uno::Any(false))),
        css::uno::Any(css::beans::NamedValue(ARGUMENT_SUPPORTPERSISTENTWINDOWSTATE, css::uno::Any(true))),
        css::uno::Any(css::beans::NamedValue(ARGUMENT_FRAMENAME, css::uno::Any(sName))),
        css::uno::Any(css::beans::NamedValue(ARGUMENT_HIDDENFORCONVERSION, css::uno::Any(rDescriptor.getUnpackedValueOrDefault(ARGUMENT_HIDDENFORCONVERSION, false))))
    };
    return xCreator->createInstance(lArgs);
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
