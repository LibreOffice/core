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

#include <sfx2/app.hxx>
#include "sfx2/viewfac.hxx"
#include <rtl/ustrbuf.hxx>

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SfxViewFactory)

SfxViewShell *SfxViewFactory::CreateInstance(SfxViewFrame *pFrame, SfxViewShell *pOldSh )
{
    DBG_CHKTHIS(SfxViewFactory, 0);
    return (*fnCreate)(pFrame, pOldSh);
}

OUString SfxViewFactory::GetLegacyViewName() const
{
    OUStringBuffer aViewName;
    aViewName.appendAscii(RTL_CONSTASCII_STRINGPARAM("view"));
    aViewName.append( sal_Int32( GetOrdinal() ) );
    return aViewName.makeStringAndClear();
}

OUString SfxViewFactory::GetAPIViewName() const
{
    if ( !m_sViewName.isEmpty() )
        return m_sViewName;

    if ( GetOrdinal() == 0 )
        return OUString( "Default" );

    return GetLegacyViewName();
}

// CTOR / DTOR -----------------------------------------------------------

SfxViewFactory::SfxViewFactory( SfxViewCtor fnC,
                                sal_uInt16 nOrdinal, const sal_Char* asciiViewName ):
    fnCreate(fnC),
    nOrd(nOrdinal),
    m_sViewName( OUString::createFromAscii( asciiViewName ) )
{
    DBG_CTOR(SfxViewFactory, 0);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
