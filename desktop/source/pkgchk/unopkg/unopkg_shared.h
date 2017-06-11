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

#include "dp_misc.h"
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/deployment/XPackage.hpp>
#include <tools/resmgr.hxx>
#include <rtl/ustring.hxx>
#include <unotools/configmgr.hxx>
#include <i18nlangtag/languagetag.hxx>

#define APP_NAME "unopkg"

namespace unopkg {

struct OptionInfo
{
    char const * m_name;
    sal_uInt32 m_name_length;
    sal_Unicode m_short_option;
    bool m_has_argument;
};

struct LockFileException : public css::uno::Exception
{
    explicit LockFileException(OUString const & sMessage) :
        css::uno::Exception(sMessage, css::uno::Reference< css::uno::XInterface > ()) {}
};


OUString toString( OptionInfo const * info );


OptionInfo const * getOptionInfo(
    OptionInfo const * list,
    OUString const & opt );


bool isOption( OptionInfo const * option_info, sal_uInt32 * pIndex );


bool readArgument(
    OUString * pValue, OptionInfo const * option_info,
    sal_uInt32 * pIndex );


inline bool readOption(
    bool * flag, OptionInfo const * option_info, sal_uInt32 * pIndex )
{
    if (isOption( option_info, pIndex )) {
        OSL_ASSERT( flag != nullptr );
        *flag = true;
        return true;
    }
    return false;
}


/** checks if an argument is a bootstrap variable. These start with -env:. For example
    -env:UNO_JAVA_JFW_USER_DATA=file:///d:/user
*/
bool isBootstrapVariable(sal_uInt32 * pIndex);

OUString const & getExecutableDir();


OUString const & getProcessWorkingDir();


OUString makeAbsoluteFileUrl(
    OUString const & sys_path, OUString const & base_url );




css::uno::Reference<css::ucb::XCommandEnvironment> createCmdEnv(
    css::uno::Reference<css::uno::XComponentContext> const & xContext,
    OUString const & logFile,
    bool option_force_overwrite,
    bool option_verbose,
    bool option_suppressLicense);

void printf_packages(
    std::vector<
    css::uno::Reference<css::deployment::XPackage> > const & allExtensions,
    std::vector<bool> const & vecUnaccepted,
    css::uno::Reference<css::ucb::XCommandEnvironment> const & xCmdEnv,
    sal_Int32 level = 0 );




css::uno::Reference<css::uno::XComponentContext> getUNO(
    bool verbose, bool shared, bool bGui,
    css::uno::Reference<css::uno::XComponentContext> & out_LocalComponentContext);

}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
