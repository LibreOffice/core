/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterComponent.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:56:19 $
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

#include "PresenterComponent.hxx"
#include <rtl/instance.hxx>

#include "PresenterPane.hxx"
#include "PresenterPaneBorderManager.hxx"
#include "PresenterProtocolHandler.hxx"
#include "PresenterScreen.hxx"
#include "PresenterSlideShowView.hxx"
#include "PresenterSpritePane.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using ::rtl::OUString;


namespace sdext { namespace presenter {

namespace {
    void RegisterServices (void)
    {
        OAutoRegistration<PresenterPane>();
        OAutoRegistration<PresenterPaneBorderManager>();
        OAutoRegistration<PresenterProtocolHandler>();
        OAutoRegistration<PresenterScreen>();
        OAutoRegistration<PresenterSlideShowView>();
        OAutoRegistration<PresenterSpritePane>();
    }
}

IMPLEMENT_COMPONENT_LIBRARY_API(PresenterComponent, RegisterServices);
IMPLEMENT_COMPONENT_MODULE(PresenterComponent);

} } // end of namespace sdext::presenter
