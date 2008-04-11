/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterComponent.cxx,v $
 *
 * $Revision: 1.3 $
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
