/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wordcountdialog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 12:07:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#if !TEST_LAYOUT
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#endif /* !TEST_LAYOUT */
#ifndef SW_WORDCOUNTDIALOG_HXX
#include <wordcountdialog.hxx>
#endif
#if !TEST_LAYOUT
#ifndef _DOCSTAT_HXX
#include <docstat.hxx>
#endif

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



