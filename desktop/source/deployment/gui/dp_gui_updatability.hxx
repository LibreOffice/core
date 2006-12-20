/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_updatability.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 14:24:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_UPDATABILITY_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_UPDATABILITY_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include "com/sun/star/uno/Sequence.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include "rtl/ref.hxx"
#endif

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

       @param context
       a non-null component context

       @param packageManagers
       a list of non-null package managers

       @param enabled
       <code>dp_gui::DialogImpl</code>'s &ldquo;Check for Updates...&rdquo;
       button; will only be accessed with the solar mutex locked
    */
    Updatability(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & context,
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
