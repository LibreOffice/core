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



#ifndef SD_FRAMEWORK_SLIDE_SORTER_MODULE_HXX
#define SD_FRAMEWORK_SLIDE_SORTER_MODULE_HXX

#include "ResourceManager.hxx"

#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XTabBar.hpp>

namespace css = ::com::sun::star;

namespace sd { namespace framework {

/** This module is responsible for showing the slide sorter bar and the the
    slide sorter view in the center pane.
*/
class SlideSorterModule
    : public ResourceManager
{
public:
    SlideSorterModule (
        const css::uno::Reference<css::frame::XController>& rxController,
        const ::rtl::OUString& rsLeftPaneURL);
    virtual ~SlideSorterModule (void);


    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewTabBarId;
    css::uno::Reference<css::drawing::framework::XControllerManager> mxControllerManager;

    void UpdateViewTabBar (
        const css::uno::Reference<css::drawing::framework::XTabBar>& rxViewTabBar);
};

} } // end of namespace sd::framework

#endif
