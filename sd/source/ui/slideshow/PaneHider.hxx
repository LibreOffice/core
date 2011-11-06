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



#ifndef SD_PANE_HIDE_HXX
#define SD_PANE_HIDE_HXX

#include <com/sun/star/drawing/framework/XConfiguration.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>


namespace sd {

class ViewShell;
class SlideshowImpl;

/** Hide the windows of the side panes and restore the original visibility
    later. Used by the in-window slide show in order to use the whole frame
    window for the show.
*/
class PaneHider
{
public:
    /** The constructor hides all side panes that belong to the
        ViewShellBase of the given view shell.
    */
    PaneHider (const ViewShell& rViewShell, SlideshowImpl* pSlideShow);

    /** Restore the original visibility of the side panes.
    */
    ~PaneHider (void);

private:
    const ViewShell& mrViewShell;
    /** Remember whether the visibility states of the windows of the  panes
        has been modified and have to be restored.
    */
    bool mbWindowVisibilitySaved;
    bool mbOriginalLeftPaneWindowVisibility;
    bool mbOriginalRightPaneWindowVisibility;

    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XConfigurationController>
        mxConfigurationController;
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XConfiguration>
        mxConfiguration;
};

} // end of namespace sd

#endif
