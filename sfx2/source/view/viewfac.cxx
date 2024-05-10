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

#include <sfx2/viewfac.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>

SfxViewShell *SfxViewFactory::CreateInstance(SfxViewFrame& rFrame, SfxViewShell *pOldSh)
{
    return (*fnCreate)(rFrame, pOldSh);
}

OUString SfxViewFactory::GetLegacyViewName() const
{
    return "view" + OUString::number( sal_uInt16( GetOrdinal() ) );
}

OUString SfxViewFactory::GetAPIViewName() const
{
    if ( !m_sViewName.isEmpty() )
        return m_sViewName;

    if ( GetOrdinal() == SFX_INTERFACE_NONE )
        return u"Default"_ustr;

    return GetLegacyViewName();
}

// CTOR / DTOR -----------------------------------------------------------

SfxViewFactory::SfxViewFactory( SfxViewCtor fnC,
                                SfxInterfaceId nOrdinal, const char* asciiViewName ):
    fnCreate(fnC),
    nOrd(nOrdinal),
    m_sViewName( OUString::createFromAscii( asciiViewName ) )
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
