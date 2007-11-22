/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_updatedata.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:02:07 $
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
#if ! defined INCLUDED_DP_GUI_UPDATEDATA_HXX
#define INCLUDED_DP_GUI_UPDATEDATA_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include "com/sun/star/uno/Reference.hxx"
#endif


namespace com { namespace sun { namespace star { namespace deployment {
        class XPackageManager;
        class XPackage;
}}}}
namespace com { namespace sun { namespace star { namespace xml { namespace dom {
        class XNode;
}}}}}


namespace dp_gui {

struct UpdateData
{
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > aInstalledPackage;

    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > aPackageManager;
    // The content of the update information
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::dom::XNode > aUpdateInfo;
    //The URL of the locally downloaded extension. It will only be set if there were no errors
    //during the download
    ::rtl::OUString sLocalURL;
    //The URL of the website wher the download can be obtained.
    ::rtl::OUString sWebsiteURL;
};
}

#endif
