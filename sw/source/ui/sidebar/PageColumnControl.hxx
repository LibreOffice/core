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

#ifndef _SW_SIDEBAR_PAGE_COLUMN_CONTROL_HXX_
#define _SW_SIDEBAR_PAGE_COLUMN_CONTROL_HXX_

#include <svx/sidebar/PopupControl.hxx>

#include <vcl/image.hxx>
#include <vcl/button.hxx>

#include <vector>

namespace svx { namespace sidebar {
    class ValueSetWithTextControl;
} }

namespace sw { namespace sidebar {

class PagePropertyPanel;


class PageColumnControl
    : public ::svx::sidebar::PopupControl
{
public:
    PageColumnControl(
        Window* pParent,
        PagePropertyPanel& rPanel,
        const sal_uInt16 nColumnType,
        const bool bLandscape );

    ~PageColumnControl(void);

private:
    ::svx::sidebar::ValueSetWithTextControl* mpColumnValueSet;
    PushButton maMoreButton;

    sal_uInt16 mnColumnType;

    PagePropertyPanel& mrPagePropPanel;

    DECL_LINK(ImplColumnHdl, void*);
    DECL_LINK(MoreButtonClickHdl_Impl, void*);
};

} } // end of namespace sw::sidebar

#endif
