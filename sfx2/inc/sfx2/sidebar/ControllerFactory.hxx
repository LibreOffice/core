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

#ifndef SFX_SIDEBAR_CONTROLLER_FACTORY_HXX
#define SFX_SIDEBAR_CONTROLLER_FACTORY_HXX

#include "sfx2/dllapi.h"
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/frame/XFrame.hpp>

namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

namespace sfx2 { namespace sidebar {

/** Convenience class for easy creation of toolbox controllers.
*/
class SFX2_DLLPUBLIC ControllerFactory
{
public:
    static cssu::Reference<css::frame::XToolbarController> CreateToolBoxController(
        ToolBox* pToolBox,
        const sal_uInt16 nItemId,
        const ::rtl::OUString& rsCommandName,
        const cssu::Reference<css::frame::XFrame>& rxFrame);
};


} } // end of namespace sfx2::sidebar

#endif
