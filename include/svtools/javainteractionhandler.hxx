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

#ifndef INCLUDED_SVTOOLS_JAVAINTERACTIONHANDLER_HXX
#define INCLUDED_SVTOOLS_JAVAINTERACTIONHANDLER_HXX

#include <svtools/svtdllapi.h>
#include <sal/types.h>

#include <com/sun/star/task/XInteractionHandler.hpp>

namespace com :: sun :: star :: task { class XInteractionRequest; }

#define JAVA_INTERACTION_HANDLER_NAME "java-vm.interaction-handler"

namespace svt
{
// We cannot derive from  cppu::WeakImplHelper because we would export the inline
//generated class. This conflicts with other libraries if they use the same inline
//class.

class SVT_DLLPUBLIC JavaInteractionHandler:
        public css::task::XInteractionHandler
{
public:
    JavaInteractionHandler();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface(
        const css::uno::Type& aType ) override;

    virtual void SAL_CALL acquire() throw() override;

    virtual void SAL_CALL release() throw() override;

    // XCurrentContext
    virtual void SAL_CALL handle( const css::uno::Reference< css::task::XInteractionRequest >& Request ) override;

private:
    oslInterlockedCount m_aRefCount;
    JavaInteractionHandler(JavaInteractionHandler const&) = delete;
    JavaInteractionHandler& operator = (JavaInteractionHandler const &) = delete;
    SVT_DLLPRIVATE virtual ~JavaInteractionHandler();
};
}

#endif // _DESKTOP_JAVAINTERACTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
