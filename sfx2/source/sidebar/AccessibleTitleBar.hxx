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

#ifndef SFX_SIDEBAR_ACCESSIBLE_TITLE_BAR_HXX
#define SFX_SIDEBAR_ACCESSIBLE_TITLE_BAR_HXX

#include <toolkit/awt/vclxaccessiblecomponent.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

namespace sfx2 { namespace sidebar {

class TitleBar;

class AccessibleTitleBar
    : public VCLXAccessibleComponent
{
public:
    static cssu::Reference<css::accessibility::XAccessible> Create (TitleBar& rTitleBar);

protected:
    virtual void FillAccessibleStateSet (utl::AccessibleStateSetHelper& rStateSet);

private:
    AccessibleTitleBar (VCLXWindow* pWindow);
    virtual ~AccessibleTitleBar (void);
};


} } // end of namespace sfx2::sidebar

#endif
