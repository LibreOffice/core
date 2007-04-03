/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideSorterModule.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 15:55:22 $
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

#ifndef _COM_SUN_STAR_DRAWING_FRAMEWORK_XCONTROLLERMANAGER_HPP_
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#endif

namespace sd { namespace framework {

/** This module is responsible for showing the slide sorter bar and the the
    slide sorter view in the center pane.
*/
class SlideSorterModule
    : public ResourceManager
{
public:
    SlideSorterModule (
        const ::com::sun::star::uno::Reference<com::sun::star::frame::XController>& rxController,
        const ::rtl::OUString& rsLeftPaneURL);
    virtual ~SlideSorterModule (void);


    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const com::sun::star::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (com::sun::star::uno::RuntimeException);

private:
    ::com::sun::star::uno::Reference<
    ::com::sun::star::drawing::framework::XResourceId> mxViewTabBarId;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XControllerManager> mxControllerManager;

    void UpdateViewTabBar (void);
};

} } // end of namespace sd::framework

#endif
