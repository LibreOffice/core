/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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
