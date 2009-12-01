/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_gui.h,v $
 * $Revision: 1.24.86.2 $
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

#if ! defined INCLUDED_DP_GUI_H
#define INCLUDED_DP_GUI_H

#include "dp_gui_updatedata.hxx"
#include "dp_misc.h"
#include "dp_gui_updatability.hxx"
#include "dp_gui.hrc"
#include "rtl/ref.hxx"
#include "rtl/instance.hxx"
#include "osl/thread.hxx"
#include "cppuhelper/implbase2.hxx"
#include "vcl/svapp.hxx"
#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "salhelper/simplereferenceobject.hxx"
#include "svtools/svtabbx.hxx"
#include "svtools/headbar.hxx"
#include "com/sun/star/ucb/XContentEventListener.hpp"
#include "osl/mutex.hxx"
#include <list>
#include <memory>
#include <queue>

namespace com { namespace sun { namespace star {
    namespace container {
        class XNameAccess;
    }
    namespace frame {
        class XDesktop;
    }
    namespace awt {
        class XWindow;
    }
    namespace uno {
        class XComponentContext;
    }
    namespace deployment {
        class XPackageManagerFactory;
    }
} } }

namespace svt {
    class FixedHyperlink;
}

namespace dp_gui {

enum PackageState { REGISTERED, NOT_REGISTERED, AMBIGUOUS, NOT_AVAILABLE };

//==============================================================================

class SelectedPackage: public salhelper::SimpleReferenceObject {
public:
    SelectedPackage() {}
    SelectedPackage( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> &xPackage,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager> &xPackageManager )
    : m_xPackage( xPackage ),
      m_xPackageManager( xPackageManager )
    {}

    virtual ~SelectedPackage();
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> getPackage() const { return m_xPackage; }
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager> getPackageManager() const { return m_xPackageManager; }
private:
    SelectedPackage(SelectedPackage &); // not defined
    void operator =(SelectedPackage &); // not defined

    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> m_xPackage;
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager> m_xPackageManager;
};

} // namespace dp_gui

#endif
