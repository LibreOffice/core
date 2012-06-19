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

#include <precomp.h>
#include <tokens/tkp2.hxx>

// NOT FULLY DECLARED SERVICES
#include <tools/tkpchars.hxx>
#include <tokens/tkpcont2.hxx>

TokenParse2::TokenParse2()
    :   pChars(0)
{
}

void
TokenParse2::Start( CharacterSource & i_rSource )
{
    pChars = &i_rSource;
    SetStartContext();
}

bool
TokenParse2::GetNextToken()
{
    csv_assert(pChars != 0);

    bool bDone = false;
    while ( NOT bDone AND NOT pChars->IsFinished() )
    {
        CurrentContext().ReadCharChain(*pChars);
        bDone = CurrentContext().PassNewToken();
        SetCurrentContext(CurrentContext().FollowUpContext());
    }
    return bDone;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
