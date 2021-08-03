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

#include <com/sun/star/frame/XFrame.hpp>


namespace sfx2::sidebar {


/** This class is a helper for broadcasting context changes that are
    tied to shells being activated or deactivated.
*/
class ContextChangeBroadcaster
{
public:
    ContextChangeBroadcaster();
    ~ContextChangeBroadcaster();

    void Initialize (const OUString& rsContextName);

    void Activate (const css::uno::Reference<css::frame::XFrame>& rxFrame);
    void Deactivate (const css::uno::Reference<css::frame::XFrame>& rxFrame);

    /** Enable or disable the broadcaster.
        @param bIsEnabled
            The new value of the "enabled" state.
        @return
            The old value of the "enabled" state is returned.
    */
    bool SetBroadcasterEnabled (const bool bIsEnabled);

private:
    OUString msContextName;
    bool mbIsBroadcasterEnabled;

    void BroadcastContextChange (
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        const OUString& rsModuleName,
        const OUString& rsContextName);
    static OUString GetModuleName (
        const css::uno::Reference<css::frame::XFrame>& rxFrame);
};


} // end of namespace ::sd::sidebar


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
