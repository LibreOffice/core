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

#include <svl/srchitem.hxx>
#include <sfx2/app.hxx>
#include <svx/svxids.hrc>

#include <sot/clsids.hxx>
#include <sfx2/objface.hxx>

#include <sfx2/msg.hxx>
#include "cfgid.h"
#include "cmdid.h"
#include "swtypes.hxx"

#include "shellio.hxx"
// just because of the itemtypes
#include "wdocsh.hxx"
#include "web.hrc"

#define SwWebDocShell
#include "swslots.hxx"

#include <unomid.h>


SFX_IMPL_INTERFACE( SwWebDocShell, SfxObjectShell, SW_RES(0) )
{
}

TYPEINIT1(SwWebDocShell, SwDocShell);

SFX_IMPL_OBJECTFACTORY(SwWebDocShell, SvGlobalName(SO3_SWWEB_CLASSID), SFXOBJECTSHELL_STD_NORMAL|SFXOBJECTSHELL_HASMENU, "swriter/web" )

SwWebDocShell::SwWebDocShell(SfxObjectCreateMode eMode ) :
        SwDocShell(eMode),
        nSourcePara(0)
{
}

SwWebDocShell::~SwWebDocShell()
{
}



void SwWebDocShell::FillClass( SvGlobalName * pClassName,
                                   sal_uInt32 * pClipFormat,
                                   OUString * /*pAppName*/,
                                   OUString * pLongUserName,
                                   OUString * pUserName,
                                   sal_Int32 nVersion,
                                   sal_Bool bTemplate /* = sal_False */) const
{
    (void)bTemplate;
    OSL_ENSURE( bTemplate == sal_False, "No template for Writer Web" );

    if (nVersion == SOFFICE_FILEFORMAT_60)
    {
        *pClassName = SvGlobalName( SO3_SWWEB_CLASSID_60 );
        *pClipFormat = SOT_FORMATSTR_ID_STARWRITERWEB_60;
        *pLongUserName = SW_RESSTR(STR_WRITER_WEBDOC_FULLTYPE);
    }
    else if (nVersion == SOFFICE_FILEFORMAT_8)
    {
        *pClassName     = SvGlobalName( SO3_SWWEB_CLASSID_60 );
        *pClipFormat    = SOT_FORMATSTR_ID_STARWRITERWEB_8;
        *pLongUserName = SW_RESSTR(STR_WRITER_WEBDOC_FULLTYPE);
    }
    *pUserName = SW_RESSTR(STR_HUMAN_SWWEBDOC_NAME);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
