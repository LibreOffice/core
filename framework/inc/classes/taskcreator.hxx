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

#ifndef INCLUDED_FRAMEWORK_INC_CLASSES_TASKCREATOR_HXX
#define INCLUDED_FRAMEWORK_INC_CLASSES_TASKCREATOR_HXX

#include <general.h>

#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>

namespace framework{

/*-************************************************************************************************************
    @short          a helper to create new tasks or sub frames for "_blank" or FrameSearchFlag::CREATE
    @descr          There are different places to create new (task)frames. Its not easy to service this code!
                    Thats the reason for this helper. He capsulate asynchronous/synchronous creation by calling
                    a simple interface.

    @devstatus      ready to use
    @threadsafe     yes
*//*-*************************************************************************************************************/
class TaskCreator
{

    // member
    private:

        css::uno::Reference< css::uno::XComponentContext >  m_xContext;

    // interface
    public:

                 TaskCreator( const css::uno::Reference< css::uno::XComponentContext >& xContext );
        virtual ~TaskCreator(                                                                     );

        css::uno::Reference< css::frame::XFrame > createTask( const OUString& sName    ,
                                                                    bool         bVisible );

    // helper
    private:

}; // class TaskCreator

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_CLASSES_TASKCREATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
