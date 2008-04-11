/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: migration_impl.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _DESKTOP_MIGRATION_IMPL_HXX_
#define _DESKTOP_MIGRATION_IMPL_HXX_

#include <vector>
#include <algorithm>
#include <memory>

#include "migration.hxx"

#include <sal/types.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#define NS_CSS com::sun::star
#define NS_UNO com::sun::star::uno

namespace desktop
{

struct install_info
{
    rtl::OUString productname;  // human readeable product name
    rtl::OUString userdata;     // file: url for user installation
};

typedef std::vector< rtl::OUString > strings_v;
typedef std::auto_ptr< strings_v > strings_vr;

struct migration_step
{
    rtl::OUString name;
    strings_v includeFiles;
    strings_v excludeFiles;
    strings_v includeConfig;
    strings_v excludeConfig;
    strings_v configComponents;
    rtl::OUString service;
};

typedef std::vector< migration_step > migrations_v;
typedef std::auto_ptr< migrations_v > migrations_vr;

class MigrationImpl
{

private:
    strings_vr m_vrVersions;
    NS_UNO::Reference< NS_CSS::lang::XMultiServiceFactory > m_xFactory;
    migrations_vr m_vrMigrations; // list of all migration specs from config
    install_info m_aInfo;       // info about the version being migrated
    strings_vr m_vrFileList;      // final list of files to be copied
    strings_vr m_vrConfigList;    // final list of nodes to be copied
    strings_vr m_vrServiceList;   // final list of services to be called

    // initializer functions...
    migrations_vr readMigrationSteps();
    install_info findInstallation();
    strings_vr compileFileList();
    strings_vr compileConfigList();
    strings_vr compileServiceList();

    // helpers
    void substract(strings_v& va, const strings_v& vb_c) const;
    strings_vr getAllFiles(const rtl::OUString& baseURL) const;
    strings_vr applyPatterns(const strings_v& vSet, const strings_v& vPatterns) const;
    NS_UNO::Reference< NS_CSS::container::XNameAccess > getConfigAccess(const sal_Char* path, sal_Bool rw=sal_False);

    // actual processing function that perform the migration steps
    void copyFiles();
    void copyConfig();
    void runServices();
    void refresh();

    void setMigrationCompleted();
    sal_Bool checkMigrationCompleted();

public:
    MigrationImpl(const NS_UNO::Reference< NS_CSS::lang::XMultiServiceFactory >&);
    ~MigrationImpl();
    sal_Bool doMigration();
    sal_Bool checkMigration();
    rtl::OUString getOldVersionName();


};
}
#undef NS_CSS
#undef NS_UNO

#endif
