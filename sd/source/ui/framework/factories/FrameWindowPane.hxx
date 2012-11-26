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



#ifndef SD_FRAMEWORK_FRAME_WINDOW_PANE_HXX
#define SD_FRAMEWORK_FRAME_WINDOW_PANE_HXX

#include "framework/Pane.hxx"

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XRESOURCEID_HPP_
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#endif


namespace sd { namespace framework {

/** This subclass is not necessary anymore.  We can remove it if that
    remains so.
*/
class FrameWindowPane
    : public Pane
{
public:
    FrameWindowPane (
        const ::com::sun::star::uno::Reference<
            com::sun::star::drawing::framework::XResourceId>& rxPaneId,
        ::Window* pWindow);
    virtual ~FrameWindowPane (void) throw();

    /** A frame window typically can (and should) exists on its own without
        children, if only to visualize that something (a view) is missing.
        Therefore this method always returns <false/>.
    */
    virtual sal_Bool SAL_CALL isAnchorOnly (void)
        throw (com::sun::star::uno::RuntimeException);
};

} } // end of namespace sd::framework

#endif
