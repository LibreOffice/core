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

#include <hintids.hxx>
#include <tools/stream.hxx>
#include <svtools/rtftoken.h>
#include <svtools/rtfkeywd.hxx>
#include <svl/intitem.hxx>
#include <svtools/rtfout.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/numitem.hxx>
#include <shellio.hxx>
#include <fltini.hxx>
#include <swtypes.hxx>
#include <swparrtf.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <charfmt.hxx>
#include <charatr.hxx>
#include <paratr.hxx>
#include <cmdid.h>
#include <numrule.hxx>

#define RTF_NUMRULE_NAME "RTF_Num"

SfxItemSet& GetNumChrFmt( SwDoc& rDoc, SwNumRule& rRule, sal_uInt8 nNumLvl )
{
    SwCharFmt* pChrFmt = rRule.Get( nNumLvl ).GetCharFmt();
    if( !pChrFmt )
    {
        String sNm( rRule.GetName() );
        ( sNm += ' ' ) += String::CreateFromInt32( nNumLvl + 1 );
        pChrFmt = rDoc.MakeCharFmt( sNm, rDoc.GetDfltCharFmt() );
        if( !rRule.GetNumFmt( nNumLvl ))
            rRule.Set( nNumLvl, rRule.Get( nNumLvl ) );
        ((SwNumFmt*)rRule.GetNumFmt( nNumLvl ))->SetCharFmt( pChrFmt );
    }
    return (SfxItemSet&)pChrFmt->GetAttrSet();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
