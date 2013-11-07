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

#ifndef INCLUDED_DP_GUI_HANDLEVERSIONEXCEPTION_HXX
#define INCLUDED_DP_GUI_HANDLEVERSIONEXCEPTION_HXX

#include "dp_gui_api.hxx"
#include "com/sun/star/deployment/VersionException.hpp"

namespace dp_gui{ class DialogHelper; }


extern "C" {

    // optional parameter <bChooseNewestVersion> indicates, if
    // the newest version is chosen without user interaction.
    DESKTOP_DEPLOYMENTGUI_DLLPUBLIC
    bool handleVersionException(
        com::sun::star::deployment::VersionException verExc,
        dp_gui::DialogHelper* pDialogHelper = 0,
        const bool bChooseNewestVersion = false );
}

#endif
