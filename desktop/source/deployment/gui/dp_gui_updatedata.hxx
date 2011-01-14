/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "tools/solar.h"
#include "rtl/ustring.hxx"
#include "com/sun/star/uno/Reference.hxx"

#include <boost/shared_ptr.hpp>


namespace com { namespace sun { namespace star { namespace deployment {
        class XPackage;
}}}}
namespace com { namespace sun { namespace star { namespace xml { namespace dom {
        class XNode;
}}}}}


namespace dp_gui {

struct UpdateData
{
    UpdateData( ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > const & aExt):
        bIsShared(false), aInstalledPackage(aExt){};

    //When entries added to the listbox then there can be one for the user update and one
    //for the shared update. However, both list entries will contain the same UpdateData.
    //isShared is used to indicate which one is used for the shared entry.
    bool bIsShared;

    //The currently installed extension which is going to be updated. If the extension exist in
    //multiple repositories then it is the one with the highest version.
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage > aInstalledPackage;

    //The version of the update
    ::rtl::OUString updateVersion;

    //For online update
    // ======================
    // The content of the update information.
    //Only if aUpdateInfo is set then there is an online update available with a better version
    //than any of the currently installed extensions with the same identifier.
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::dom::XNode > aUpdateInfo;
    //The URL of the locally downloaded extension. It will only be set if there were no errors
    //during the download
    ::rtl::OUString sLocalURL;
    //The URL of the website wher the download can be obtained.
    ::rtl::OUString sWebsiteURL;

    //For local update
    //=====================
    //The locale extension which is used as update for the user or shared repository.
    //If set then the data for the online update (aUpdateInfo, sLocalURL, sWebsiteURL)
    //are to be ignored.
    ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XPackage >
    aUpdateSource;

    // ID to find this entry in the update listbox
    sal_uInt16  m_nID;
    bool    m_bIgnored;
};
}

#endif
