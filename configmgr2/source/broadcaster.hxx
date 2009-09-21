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

#ifndef INCLUDED_CONFIGMGR_SOURCE_BROADCASTER_HXX
#define INCLUDED_CONFIGMGR_SOURCE_BROADCASTER_HXX

#include "sal/config.h"

#include <vector>

#include "boost/noncopyable.hpp"
#include "com/sun/star/beans/PropertyChangeEvent.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "rtl/ref.hxx"

namespace com { namespace sun { namespace star { namespace beans {
    class XPropertyChangeListener;
} } } }

namespace configmgr {

class Access;

class Broadcaster: private boost::noncopyable {
public:
    void addPropertyChange(
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertyChangeListener > const & listener,
        com::sun::star::beans::PropertyChangeEvent const & event);

    void send();

private:
    struct PropertyChange {
        com::sun::star::uno::Reference<
            com::sun::star::beans::XPropertyChangeListener > listener;
        com::sun::star::beans::PropertyChangeEvent event;

        PropertyChange(
            com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertyChangeListener > const &
                theListener,
            com::sun::star::beans::PropertyChangeEvent const & theEvent);
    };

    typedef std::vector< PropertyChange > PropertyChanges;

    PropertyChanges propertyChanges_;
};

}

#endif
