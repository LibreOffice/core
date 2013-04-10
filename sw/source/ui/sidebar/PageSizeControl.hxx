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

#ifndef _SW_SIDEBAR_PAGE_SIZE_CONTROL_HXX_
#define _SW_SIDEBAR_PAGE_SIZE_CONTROL_HXX_

#include <svx/sidebar/PopupControl.hxx>

#include <i18npool/paper.hxx>

#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <svtools/unitconv.hxx>

#include <vector>

namespace svx { namespace sidebar {
    class ValueSetWithTextControl;
} }

namespace sw { namespace sidebar {

class PagePropertyPanel;


class PageSizeControl
    : public ::svx::sidebar::PopupControl
{
public:
    PageSizeControl(
        Window* pParent,
        PagePropertyPanel& rPanel,
        const Paper ePaper,
        const sal_Bool bLandscape,
        const FieldUnit eFUnit );
    ~PageSizeControl(void);

private:
    ::svx::sidebar::ValueSetWithTextControl* mpSizeValueSet;
    PushButton maMoreButton;
    // hidden metric field
    MetricField maWidthHeightField;

    Paper mePaper;
    ::std::vector< Paper > maPaperList;

    PagePropertyPanel& mrPagePropPanel;

    DECL_LINK(ImplSizeHdl, void*);
    DECL_LINK(MoreButtonClickHdl_Impl, void*);
};

} } // end of namespace sw::sidebar

#endif
