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

#pragma once

#include <vector>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/EventObject.hpp>

namespace com::sun::star {
    namespace util {
        class XModifyListener;
    }
}

struct ScUnoListenerEntry
{
    css::uno::Reference<
        css::util::XModifyListener >   xListener;
    css::lang::EventObject             aEvent;

    ScUnoListenerEntry( const css::uno::Reference< css::util::XModifyListener >& rL,
                        const css::lang::EventObject& rE ) :
        xListener( rL ),
        aEvent( rE )
    {}
};

/** ScUnoListenerCalls stores notifications to XModifyListener that can't be processed
    during BroadcastUno and calls them together at the end.
*/
class ScUnoListenerCalls
{
private:
    ::std::vector<ScUnoListenerEntry> aEntries;

public:
                ScUnoListenerCalls();
                ~ScUnoListenerCalls();

    void        Add( const css::uno::Reference< css::util::XModifyListener >& rListener,
                     const css::lang::EventObject& rEvent );
    void        ExecuteAndClear();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
