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



#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_DEPENDENCYDIALOG_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_DEPENDENCYDIALOG_HXX

#include "sal/config.h"

#include <vector>
#include "tools/gen.hxx"
#ifndef _SV_BUTTON_HXX
#include "vcl/button.hxx"
#endif
#include "vcl/dialog.hxx"
#include "vcl/fixed.hxx"
#include "vcl/lstbox.hxx"

class Window;
namespace rtl { class OUString; }

namespace dp_gui {

class DependencyDialog: public ModalDialog {
public:
    DependencyDialog(
        Window * parent, std::vector< rtl::OUString > const & dependencies);

    ~DependencyDialog();

private:
    DependencyDialog(DependencyDialog &); // not defined
    void operator =(DependencyDialog &); // not defined

    virtual void Resize();

    FixedText m_text;
    ListBox m_list;
    OKButton m_ok;
    Size m_listDelta;
};

}

#endif
