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

#ifndef SFX_SIDEBAR_PANEL_DESCRIPTOR_HXX
#define SFX_SIDEBAR_PANEL_DESCRIPTOR_HXX

#include "sfx2/sidebar/EnumContext.hxx"
#include "ContextList.hxx"
#include <boost/shared_ptr.hpp>


namespace sfx2 { namespace sidebar {

class PanelDescriptor
{
public:
    ::rtl::OUString msTitle;
    sal_Bool mbIsTitleBarOptional;
    ::rtl::OUString msId;
    ::rtl::OUString msDeckId;
    ::rtl::OUString msHelpURL;
    ContextList maContextList;
    ::rtl::OUString msImplementationURL;
    sal_Int32 mnOrderIndex;
    bool mbWantsCanvas;

    PanelDescriptor (void);
    PanelDescriptor (const PanelDescriptor& rPanelDescriptor);
    ~PanelDescriptor (void);
};
typedef ::boost::shared_ptr<PanelDescriptor> SharedPanelDescriptor;

} } // end of namespace sfx2::sidebar

#endif
