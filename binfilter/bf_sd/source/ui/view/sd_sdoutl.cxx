/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <bf_sfx2/app.hxx>
#include <bf_svx/editstat.hxx>
#include <bf_svx/unolingu.hxx>
#include <bf_svx/eeitem.hxx>
#include <bf_svx/forbiddencharacterstable.hxx>

#include "sdoutl.hxx"
#include "drawdoc.hxx"
#include "sdmod.hxx"

class SfxStyleSheetPool;

namespace binfilter {

SdOutliner::SdOutliner( SdDrawDocument* pDoc, USHORT nMode )
: SdrOutliner( &pDoc->GetItemPool(), nMode )
{
    SetStyleSheetPool((SfxStyleSheetPool*) pDoc->GetStyleSheetPool());
    SetEditTextObjectPool( &pDoc->GetItemPool() );
    SetCalcFieldValueHdl(LINK(SD_MOD(), SdModule, CalcFieldValueHdl));
    SetForbiddenCharsTable( pDoc->GetForbiddenCharsTable() );

    ULONG nCntrl = GetControlWord();
    nCntrl |= EE_CNTRL_ALLOWBIGOBJS;
    nCntrl |= EE_CNTRL_URLSFXEXECUTE;
    nCntrl |= EE_CNTRL_MARKFIELDS;
    nCntrl |= EE_CNTRL_AUTOCORRECT;
    nCntrl |= EE_CNTRL_NOREDLINES;
    nCntrl &= ~EE_CNTRL_ONLINESPELLING;

    SetControlWord(nCntrl);

    SetDefaultLanguage( Application::GetSettings().GetLanguage() );
}

SdOutliner::~SdOutliner()
{
}

}

