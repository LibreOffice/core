/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wordcountdialog.cxx,v $
 * $Revision: 1.3 $
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

#if !TEST_LAYOUT
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif
#include <swtypes.hxx>
#endif /* !TEST_LAYOUT */
#include <wordcountdialog.hxx>
#if !TEST_LAYOUT
#include <docstat.hxx>

#include <dialog.hrc>
#endif /* !TEST_LAYOUT */
#include <layout/layout-pre.hxx>
#if !TEST_LAYOUT
#include <wordcountdialog.hrc>
#endif /* !TEST_LAYOUT */

/*-- 06.04.2004 16:05:55---------------------------------------------------

  -----------------------------------------------------------------------*/
SwWordCountDialog::SwWordCountDialog(Window* pParent) :
    SfxModalDialog(pParent, SW_RES(DLG_WORDCOUNT)),
#if defined _MSC_VER
#pragma warning (disable : 4355)
#endif
    aCurrentFL( this, SW_RES(              FL_CURRENT            )),
    aCurrentWordFT( this, SW_RES(          FT_CURRENTWORD        )),
    aCurrentWordFI( this, SW_RES(          FI_CURRENTWORD        )),
    aCurrentCharacterFT( this, SW_RES(     FT_CURRENTCHARACTER   )),
    aCurrentCharacterFI( this, SW_RES(     FI_CURRENTCHARACTER   )),

    aDocFL( this, SW_RES(                  FL_DOC                )),
    aDocWordFT( this, SW_RES(              FT_DOCWORD            )),
    aDocWordFI( this, SW_RES(              FI_DOCWORD            )),
    aDocCharacterFT( this, SW_RES(         FT_DOCCHARACTER       )),
    aDocCharacterFI( this, SW_RES(         FI_DOCCHARACTER       )),
    aBottomFL(this, SW_RES(                FL_BOTTOM             )),
    aOK( this, SW_RES(                     PB_OK                 )),
    aHelp( this, SW_RES(                   PB_HELP               ))
#if defined _MSC_VER
#pragma warning (default : 4355)
#endif
{
    FreeResource();
}
/*-- 06.04.2004 16:05:56---------------------------------------------------

  -----------------------------------------------------------------------*/
SwWordCountDialog::~SwWordCountDialog()
{
}
/*-- 06.04.2004 16:05:57---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SwWordCountDialog::SetValues(const SwDocStat& rCurrent, const SwDocStat& rDoc)
{
#if !TEST_LAYOUT
    aCurrentWordFI.SetText(     String::CreateFromInt32(rCurrent.nWord ));
    aCurrentCharacterFI.SetText(String::CreateFromInt32(rCurrent.nChar ));
    aDocWordFI.SetText(         String::CreateFromInt32(rDoc.nWord ));
    aDocCharacterFI.SetText(    String::CreateFromInt32(rDoc.nChar ));
#endif /* !TEST_LAYOUT */
}



