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

#ifndef _SVX_SIDEBAR_COLOR_POPUP_HXX_
#define _SVX_SIDEBAR_COLOR_POPUP_HXX_

#include "svx/sidebar/Popup.hxx"

#include <tools/color.hxx>


namespace svx { namespace sidebar {

/** Popup control that displays all named colors in a matrix.
    The number of rows and columns of the matrix are computed from
    the number of named colors so that both have roughly the same
    value.

    The ColorPopup uses ColorControl as control for its content.
*/
class SVX_DLLPUBLIC ColorPopup
    : public Popup
{
public :
    ColorPopup (
        Window* pParent,
        const ::boost::function<PopupControl*(PopupContainer*)>& rControlCreator);
    virtual ~ColorPopup (void);

    void SetCurrentColor (
        const Color aCurrentColor,
        const bool bIsColorAvailable);
};

} } // end of namespace svx::sidebar

#endif
