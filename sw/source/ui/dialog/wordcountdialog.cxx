/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif
#include <swtypes.hxx>
#include <wordcountdialog.hxx>
#include <docstat.hxx>

#include <dialog.hrc>
#include <layout/layout-pre.hxx>
#include <wordcountdialog.hrc>

#if ENABLE_LAYOUT
#undef SW_RES
#define SW_RES(x) #x
#undef SfxModalDialog
#define SfxModalDialog( parent, id ) Dialog( parent, "wordcount.xml", id )
#define SW_WORDCOUNTDIALOG_HRC
#include <helpid.h>
#endif /* ENABLE_LAYOUT */

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
#if ENABLE_LAYOUT
    SetHelpId (HID_DLG_WORDCOUNT);
#endif /* ENABLE_LAYOUT */
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
    aCurrentWordFI.SetText(     String::CreateFromInt32(rCurrent.nWord ));
    aCurrentCharacterFI.SetText(String::CreateFromInt32(rCurrent.nChar ));
    aDocWordFI.SetText(         String::CreateFromInt32(rDoc.nWord ));
    aDocCharacterFI.SetText(    String::CreateFromInt32(rDoc.nChar ));
}



