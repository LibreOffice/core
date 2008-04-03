/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideSorterModule.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:40:29 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
