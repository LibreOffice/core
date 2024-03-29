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

#include <sal/config.h>

#include <sal/log.hxx>
#include <unotools/saveopt.hxx>
#include <unotools/configmgr.hxx>

#include <officecfg/Office/Common.hxx>

void SetODFDefaultVersion( SvtSaveOptions::ODFDefaultVersion eVersion, const std::shared_ptr<comphelper::ConfigurationChanges>& xChanges )
{
    sal_Int16 nTmp = (eVersion == SvtSaveOptions::ODFVER_LATEST) ? sal_Int16( 3 ) : sal_Int16( eVersion );
    officecfg::Office::Common::Save::ODF::DefaultVersion::set(nTmp, xChanges);
}

void SetODFDefaultVersion( SvtSaveOptions::ODFDefaultVersion eVersion )
{
    auto xChanges = comphelper::ConfigurationChanges::create();
    SetODFDefaultVersion(eVersion, xChanges);
    xChanges->commit();
}

SvtSaveOptions::ODFDefaultVersion GetODFDefaultVersion()
{
    SvtSaveOptions::ODFDefaultVersion nRet;
    sal_Int16 nTmp = officecfg::Office::Common::Save::ODF::DefaultVersion::get();
    if( nTmp == 3 )
        nRet = SvtSaveOptions::ODFVER_LATEST;
    else
        nRet = SvtSaveOptions::ODFDefaultVersion( nTmp );
    SAL_WARN_IF(nRet == SvtSaveOptions::ODFVER_UNKNOWN, "unotools.config", "DefaultVersion is ODFVER_UNKNOWN?");
    return (nRet == SvtSaveOptions::ODFVER_UNKNOWN) ? SvtSaveOptions::ODFVER_LATEST : nRet;
}

SvtSaveOptions::ODFSaneDefaultVersion GetODFSaneDefaultVersion()
{
    SvtSaveOptions::ODFDefaultVersion nRet;
    sal_Int16 nTmp = officecfg::Office::Common::Save::ODF::DefaultVersion::get();
    if( nTmp == 3 )
        nRet = SvtSaveOptions::ODFVER_LATEST;
    else
        nRet = SvtSaveOptions::ODFDefaultVersion( nTmp );

    return GetODFSaneDefaultVersion(nRet);
}

SvtSaveOptions::ODFSaneDefaultVersion GetODFSaneDefaultVersion(SvtSaveOptions::ODFDefaultVersion eODFDefaultVersion)
{
    switch (eODFDefaultVersion)
    {
        default:
            assert(!"map new ODFDefaultVersion to ODFSaneDefaultVersion");
            break;
        case SvtSaveOptions::ODFVER_UNKNOWN:
        case SvtSaveOptions::ODFVER_LATEST:
            return SvtSaveOptions::ODFSVER_LATEST_EXTENDED;
        case SvtSaveOptions::ODFVER_010:
            return SvtSaveOptions::ODFSVER_010;
        case SvtSaveOptions::ODFVER_011:
            return SvtSaveOptions::ODFSVER_011;
        case SvtSaveOptions::ODFVER_012:
            return SvtSaveOptions::ODFSVER_012;
        case SvtSaveOptions::ODFVER_012_EXT_COMPAT:
            return SvtSaveOptions::ODFSVER_012_EXT_COMPAT;
        case SvtSaveOptions::ODFVER_012_EXTENDED:
            return SvtSaveOptions::ODFSVER_012_EXTENDED;
        case SvtSaveOptions::ODFVER_013:
            return SvtSaveOptions::ODFSVER_013;
    }
    return SvtSaveOptions::ODFSVER_LATEST_EXTENDED;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
