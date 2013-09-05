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

#include <vcl/svapp.hxx>

#include "optutil.hxx"
#include "global.hxx"       // for pSysLocale
#include <unotools/syslocale.hxx>

sal_Bool ScOptionsUtil::IsMetricSystem()
{
    //! which language should be used here - system language or installed office language?

    MeasurementSystem eSys = ScGlobal::pLocaleData->getMeasurementSystemEnum();

    return ( eSys == MEASURE_METRIC );
}

ScLinkConfigItem::ScLinkConfigItem( const OUString& rSubTree ) :
    ConfigItem( rSubTree )
{
}

ScLinkConfigItem::ScLinkConfigItem( const OUString& rSubTree, sal_Int16 nMode ) :
    ConfigItem( rSubTree, nMode )
{
}

void ScLinkConfigItem::SetCommitLink( const Link& rLink )
{
    aCommitLink = rLink;
}

void ScLinkConfigItem::Notify( const com::sun::star::uno::Sequence<OUString>& /* aPropertyNames */ )
{
    //! not implemented yet...
}

void ScLinkConfigItem::Commit()
{
    aCommitLink.Call( this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
