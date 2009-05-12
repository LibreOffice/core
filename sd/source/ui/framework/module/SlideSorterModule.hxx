/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlideSorterModule.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
