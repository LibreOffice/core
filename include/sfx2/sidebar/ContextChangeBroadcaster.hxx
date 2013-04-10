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
#ifndef SFX_SIDEBAR_CONTEXT_CHANGE_BROADCASTER_HXX
#define SFX_SIDEBAR_CONTEXT_CHANGE_BROADCASTER_HXX

#include <com/sun/star/frame/XFrame.hpp>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

namespace sfx2 { namespace sidebar {


/** This class is a helper for broadcasting context changes that are
    tied to shells being activated or deactivated.
*/
class ContextChangeBroadcaster
{
public:
    ContextChangeBroadcaster (void);
    ~ContextChangeBroadcaster (void);

    void Initialize (const ::rtl::OUString& rsContextName);

    void Activate (const cssu::Reference<css::frame::XFrame>& rxFrame);
    void Deactivate (const cssu::Reference<css::frame::XFrame>& rxFrame);

private:
    rtl::OUString msContextName;
    bool mbIsContextActive;

    void BroadcastContextChange (
        const cssu::Reference<css::frame::XFrame>& rxFrame,
        const ::rtl::OUString& rsModuleName,
        const ::rtl::OUString& rsContextName);
    ::rtl::OUString GetModuleName (
        const cssu::Reference<css::frame::XFrame>& rxFrame);
};


} } // end of namespace ::sd::sidebar

#endif
