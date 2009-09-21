/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
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
************************************************************************/

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "osl/diagnose.hxx"

#include "broadcaster.hxx"

namespace configmgr {

namespace {

namespace css = com::sun::star;

}

void Broadcaster::addPropertyChange(
    css::uno::Reference< css::beans::XPropertyChangeListener > const & listener,
    com::sun::star::beans::PropertyChangeEvent const & event)
{
    propertyChanges_.push_back(PropertyChange(listener, event));
}

void Broadcaster::send() {
    for (PropertyChanges::iterator i(propertyChanges_.begin());
         i != propertyChanges_.end(); ++i)
    {
        try {
            i->listener->propertyChange(i->event);
        } catch (css::lang::DisposedException &) {}
    }
}

Broadcaster::PropertyChange::PropertyChange(
    com::sun::star::uno::Reference<
        com::sun::star::beans::XPropertyChangeListener > const & theListener,
    com::sun::star::beans::PropertyChangeEvent const & theEvent):
    listener(theListener), event(theEvent)
{
    OSL_ASSERT(theListener.is());
}

}
