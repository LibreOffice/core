/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_gui_updatability.hxx,v $
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_UPDATABILITY_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_UPDATABILITY_HXX

#include "sal/config.h"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/Sequence.hxx"
#include "rtl/ref.hxx"

/// @HTML

class Window;
namespace com { namespace sun { namespace star {
    namespace deployment { class XPackageManager; }
    namespace uno { class XComponentContext; }
} } }

namespace dp_gui {

/**
   Asynchronously determine whether <code>dp_gui::DialogImpl</code>'s
   &ldquo;Check for Updates...&rdquo; button shall be enabled (which
   theoretically can take some time).

   <p>Note that, due to the asynchronous operation, the button may be enabled
   even if there are no updatable extensions.</p>

   <p>Each instance of this class must be called from a single thread in order
   to adhere to the following protocol: <code>stop</code> must be called exactly
   once, with no intervening calls to <code>start</code>, before the destructor
   is called.</p>
*/
class Updatability {
public:
    /**
       Create an instance.

       @param packageManagers
       a list of non-null package managers

       @param enabled
       <code>dp_gui::DialogImpl</code>'s &ldquo;Check for Updates...&rdquo;
       button; will only be accessed with the solar mutex locked
    */
    Updatability(
         com::sun::star::uno::Sequence< com::sun::star::uno::Reference<
            com::sun::star::deployment::XPackageManager > > const &
            packageManagers,
        Window & enabled);

    ~Updatability();

    /**
       (Re-)start determining whether <code>dp_gui::DialogImpl</code>'s
       &ldquo;Check for Updates...&rdquo; button shall be enabled.
    */
    void start();

    /**
       Orderly shut down this instance.
    */
    void stop();

private:
    Updatability(Updatability &); // not defined
    void operator =(Updatability &); // not defined

    class Thread;

    rtl::Reference< Thread > m_thread;
};

}

#endif
