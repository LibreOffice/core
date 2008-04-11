/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: gnujre.hxx,v $
 * $Revision: 1.6 $
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

#if !defined INCLUDED_JFW_PLUGIN_GNUJRE_HXX
#define  INCLUDED_JFW_PLUGIN_GNUJRE_HXX

#include "vendorbase.hxx"
#include "vendorlist.hxx"

namespace jfw_plugin
{

class GnuInfo: public VendorBase
{
private:
    rtl::OUString m_sJavaHome;
public:
    static char const* const* getJavaExePaths(int * size);

    static rtl::Reference<VendorBase> createInstance();

    virtual char const* const* getRuntimePaths(int * size);

    virtual bool initialize(
        std::vector<std::pair<rtl::OUString, rtl::OUString> > props);
    virtual int compareVersions(const rtl::OUString& sSecond) const;

};

}
#endif
