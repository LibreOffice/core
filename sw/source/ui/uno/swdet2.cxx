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

#include <sfx2/docfac.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/app.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/parhtml.hxx>
#include <sot/clsids.hxx>
#include <iodetect.hxx>

#include <swdetect.hxx>

#include <app.hrc>
#include <web.hrc>
#include <globdoc.hrc>

#include <unotools/moduleoptions.hxx>

#include <unomid.h>

bool IsDocShellRegistered()
{
    return SvtModuleOptions().IsWriter();
}

//-------------------------------------------------------------------------

sal_uLong SwFilterDetect::DetectFilter( SfxMedium& rMedium, const SfxFilter** ppFilter )
{
    sal_uLong nRet = ERRCODE_NONE;
    if( *ppFilter )
    {
        // verify the given filter
        String aPrefFlt = (*ppFilter)->GetUserData();

        // detection for TextFilter needs an additional checking
        sal_Bool bDetected = SwIoSystem::IsFileFilter(rMedium, aPrefFlt);
        return bDetected ? nRet : ERRCODE_ABORT;
    }

    // mba: without preselection there is no PrefFlt
    String aPrefFlt;
    const SfxFilter* pTmp = SwIoSystem::GetFileFilter( rMedium.GetPhysicalName(), aPrefFlt, &rMedium );
    if( !pTmp )
        return ERRCODE_ABORT;

    else
    {
        //Bug 41417: JP 09.07.97: HTML documents should be loaded by WebWriter
        SfxFilterContainer aFilterContainer( OUString("swriter/web") );
        if( !pTmp->GetUserData().equals(sHTML) ||
            pTmp->GetServiceName() == "com.sun.star.text.WebDocument" ||
            0 == ( (*ppFilter) = SwIoSystem::GetFilterOfFormat( OUString(sHTML),
                    &aFilterContainer ) ) )
            *ppFilter = pTmp;
    }

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
