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

#include <svx/svxids.hrc>

#include <comphelper/fileformat.h>
#include <comphelper/classids.hxx>
#include <sfx2/objface.hxx>
#include <tools/globname.hxx>
#include <osl/diagnose.h>

#include <sfx2/msg.hxx>
#include <swtypes.hxx>

#include <wdocsh.hxx>
#include <strings.hrc>

    // needed for -fsanitize=function visibility of typeinfo for functions of
    // type void(SfxShell*,SfxRequest&) defined in swslots.hxx
#define ShellClass_SwWebDocShell
#include <swslots.hxx>

SFX_IMPL_SUPERCLASS_INTERFACE(SwWebDocShell, SfxObjectShell)

void SwWebDocShell::InitInterface_Impl()
{
}


SFX_IMPL_OBJECTFACTORY(SwWebDocShell, SvGlobalName(SO3_SWWEB_CLASSID), u"swriter/web"_ustr )

SwWebDocShell::SwWebDocShell()
    : SwDocShell(SfxObjectCreateMode::STANDARD)
    , m_nSourcePara(0)
{
}

SwWebDocShell::~SwWebDocShell()
{
}

void SwWebDocShell::FillClass( SvGlobalName * pClassName,
                                   SotClipboardFormatId * pClipFormat,
                                   OUString * pLongUserName,
                                   sal_Int32 nVersion,
                                   bool bTemplate /* = false */) const
{
    OSL_ENSURE( !bTemplate, "No template for Writer Web" );

    if (nVersion == SOFFICE_FILEFORMAT_60)
    {
        *pClassName = SvGlobalName( SO3_SWWEB_CLASSID_60 );
        *pClipFormat = SotClipboardFormatId::STARWRITERWEB_60;
        *pLongUserName = SwResId(STR_WRITER_WEBDOC_FULLTYPE);
    }
    else if (nVersion == SOFFICE_FILEFORMAT_8)
    {
        *pClassName     = SvGlobalName( SO3_SWWEB_CLASSID_60 );
        *pClipFormat    = SotClipboardFormatId::STARWRITERWEB_8;
        *pLongUserName = SwResId(STR_WRITER_WEBDOC_FULLTYPE);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
