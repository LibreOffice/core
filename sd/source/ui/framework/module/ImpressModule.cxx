/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImpressModule.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 13:38:58 $
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

#include "precompiled_sd.hxx"

#include "framework/ImpressModule.hxx"

#include "framework/FrameworkHelper.hxx"
#include "ViewTabBarModule.hxx"
#include "CenterViewFocusModule.hxx"
#include "SlideSorterModule.hxx"
#include "TaskPaneModule.hxx"
#include "ToolBarModule.hxx"
#include "ShellStackGuard.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd { namespace framework {


void ImpressModule::Initialize (Reference<frame::XController>& rxController)
{
    new CenterViewFocusModule(rxController);
    new ViewTabBarModule(
        rxController,
        FrameworkHelper::CreateResourceId(
            FrameworkHelper::msViewTabBarURL,
            FrameworkHelper::msCenterPaneURL));
    new SlideSorterModule(
        rxController,
        FrameworkHelper::msLeftImpressPaneURL);
    TaskPaneModule::Initialize(rxController);
    new ToolBarModule(rxController);
    new ShellStackGuard(rxController);
}


} } // end of namespace sd::framework
