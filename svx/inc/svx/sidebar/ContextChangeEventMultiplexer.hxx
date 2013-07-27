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

#ifndef _SVX_SIDEBAR_CONTEXT_CHANGE_EVENT_MULTIPLEXER_HXX_
#define _SVX_SIDEBAR_CONTEXT_CHANGE_EVENT_MULTIPLEXER_HXX_

#include "svx/svxdllapi.h"
#include <sfx2/sidebar/EnumContext.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

class SfxViewShell;

/** Convenience frontend for com::sun::star::ui::ContextChangeEventMultiplexer
*/
class SVX_DLLPUBLIC ContextChangeEventMultiplexer
{
public:
    /** Notify the activation of a context.
        @param rxController
            This controller is used to determine the module (ie
            application like Writer or Calc).
        @param eContext
            The activated context.
    */
    static void NotifyContextChange (
        const cssu::Reference<css::frame::XController>& rxController,
        const ::sfx2::sidebar::EnumContext::Context eContext);

    /** Notify the activation of a context.
        @param pViewShell
            This view shell is used to determine the module (ie
            application like Writer or Calc).  When <NULL/> then no
            notification is made.
        @param eContext
            The activated context.
    */
    static void NotifyContextChange (
        SfxViewShell* pViewShell,
        const ::sfx2::sidebar::EnumContext::Context eContext);

private:
    static ::rtl::OUString GetModuleName (
        const cssu::Reference<css::frame::XFrame>& rxFrame);
};

#endif

