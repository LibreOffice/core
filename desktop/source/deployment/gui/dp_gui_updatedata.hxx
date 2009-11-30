/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dp_gui_updatedata.hxx,v $
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
#if ! defined INCLUDED_DP_GUI_UPDATEDATA_HXX
#define INCLUDED_DP_GUI_UPDATEDATA_HXX

#include "sal/config.h"
#include "rtl/ustring.hxx"
#include "com/sun/star/uno/Reference.hxx"

#include <boost/shared_ptr.hpp>


namespace com { namespace sun { namespace star { namespace deployment {
        class XPackageManager;
        class XPackage;
}}}}
namespace com { namespace sun { namespace star { namespace xml { namespace dom {
        class XNode;
}}}}}


namespace dp_gui {

struct UpdateListEntry
{
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage> m_xPackage;
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager> m_xPackageManager;

    UpdateListEntry( const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > &xPackage,
                     const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackageManager > &xPackageManager );
   ~UpdateListEntry();
};

typedef ::boost::shared_ptr< UpdateListEntry > TUpdateListEntry;


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
