/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_addextensionqueue.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:00:48 $
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_ADDEXTENSIONQUEUE_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_ADDEXTENSIONQUEUE_HXX

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
   Manages installing of extensions in the GUI mode. Requests for installing
   Extensions can be asynchronous. For example, the Extension Manager is running
   in an office process and someone uses the system integration to install an Extension.
   That is, the user double clicks an extension symbol in a file browser, which then
   causes an invocation of "unopkg gui ext". When at that time the Extension Manager
   already performs a task, triggered by the user (for example, add, update, disable,
   enable) then adding of the extension will be postponed until the user has finished
   the task.

   This class also ensures that the extensions are not installed in the main thread.
   Doing so would cause a deadlock because of the progress bar which needs to be constantly
   updated.
*/
class AddExtensionQueue {
public:
    /**
       Create an instance.
    */
    AddExtensionQueue( DialogImpl * pDialog,
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & componentContext,
        ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > const &
        packageManager);

    ~AddExtensionQueue();

    /**
    */
    void addExtension( ::rtl::OUString const & extensionURL);

    /**
       This call does not block. It signals the internal thread
       that it should install the remaining extensions and then terminate.
    */
    void stop();

    /** Determines if thread of this class has terminated.
    */
    bool hasTerminated();

    /** Blocks until the thread has terminated. All URLs in the queue will be processed.
    */
    void stopAndWait();

private:
    AddExtensionQueue(AddExtensionQueue &); // not defined
    void operator =(AddExtensionQueue &); // not defined

    class Thread;

    rtl::Reference< Thread > m_thread;
};

}

#endif
