/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_JVMFWK_PLUGINS_SUNMAJOR_PLUGINLIB_GNUJRE_HXX
#define INCLUDED_JVMFWK_PLUGINS_SUNMAJOR_PLUGINLIB_GNUJRE_HXX

#include "vendorbase.hxx"
#include "vendorlist.hxx"

namespace jfw_plugin
{

class GnuInfo: public VendorBase
{
private:
    OUString m_sJavaHome;
public:
    static char const* const* getJavaExePaths(int * size);

    static rtl::Reference<VendorBase> createInstance();

    virtual char const* const* getRuntimePaths(int * size) override;

    virtual char const* const* getLibraryPaths(int* size) override;

    virtual bool initialize(
        std::vector<std::pair<OUString, OUString> > props) override;
    virtual int compareVersions(const OUString& sSecond) const override;

};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
