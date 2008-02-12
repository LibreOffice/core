/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: license_dialog.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 16:18:02 $
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
#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_LICENSE_DIALOG_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_LICENSE_DIALOG_HXX

#include "dp_gui.h"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/task/XJobExecutor.hpp"
#include "com/sun/star/ui/dialogs/XExecutableDialog.hpp"

#include "boost/bind.hpp"

using namespace ::dp_misc;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace dp_gui {

class LicenseDialog
    : public ::cppu::WeakImplHelper1<ui::dialogs::XExecutableDialog>
//                                     task::XJobExecutor>
{
    Reference<XComponentContext> const m_xComponentContext;
    Reference<awt::XWindow> /* const */ m_parent;
    OUString /* const */ m_sLicenseText;
    OUString m_initialTitle;

    sal_Int16 solar_execute();

public:
    LicenseDialog( Sequence<Any> const & args,
                 Reference<XComponentContext> const & xComponentContext );

    // XExecutableDialog
    virtual void SAL_CALL setTitle( OUString const & title )
        throw (RuntimeException);
    virtual sal_Int16 SAL_CALL execute() throw (RuntimeException);

    //// XJobExecutor
    //virtual void SAL_CALL trigger( OUString const & event )
    //    throw (RuntimeException);
};
}
#endif
