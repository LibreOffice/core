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


#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "scitems.hxx"
#include <svl/eitem.hxx>

#include "hfedtdlg.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "scresid.hxx"
#include "hfedtdlg.hrc"
#include "scuitphfedit.hxx"
//------------------------------------------------------------------

//  macros from docsh4.cxx
//! use SIDs?

#define IS_SHARE_HEADER(set) \
    ((SfxBoolItem&) \
        ((SvxSetItem&)(set).Get(ATTR_PAGE_HEADERSET)).GetItemSet(). \
            Get(ATTR_PAGE_SHARED)).GetValue()

#define IS_SHARE_FOOTER(set) \
    ((SfxBoolItem&) \
        ((SvxSetItem&)(set).Get(ATTR_PAGE_FOOTERSET)).GetItemSet(). \
            Get(ATTR_PAGE_SHARED)).GetValue()

//==================================================================

ScHFEditDlg::ScHFEditDlg( SfxViewFrame*     pFrameP,
                          Window*           pParent,
                          const SfxItemSet& rCoreSet,
                          const String&     rPageStyle,
                          sal_uInt16        nResIdP )
    :   SfxTabDialog( pFrameP, pParent, ScResId( nResIdP ), &rCoreSet )
{
    eNumType = ((const SvxPageItem&)rCoreSet.Get(ATTR_PAGE)).GetNumType();

    String aTmp = GetText();

    aTmp.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
    aTmp += ScGlobal::GetRscString( STR_PAGESTYLE );
    aTmp.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));
    aTmp += rPageStyle;
    aTmp += ')';
    SetText( aTmp );

    switch ( nResIdP )
    {
        case RID_SCDLG_HFED_HEADER:
        case RID_SCDLG_HFEDIT_HEADER:
            AddTabPage( 1, ScRightHeaderEditPage::Create, NULL );
            AddTabPage( 2, ScLeftHeaderEditPage::Create, NULL );
            break;

        case RID_SCDLG_HFED_FOOTER:
        case RID_SCDLG_HFEDIT_FOOTER:
            AddTabPage( 1, ScRightFooterEditPage::Create, NULL );
            AddTabPage( 2, ScLeftFooterEditPage::Create, NULL );
            break;

        case RID_SCDLG_HFEDIT_LEFTHEADER:
            AddTabPage( 1, ScLeftHeaderEditPage::Create, NULL );
            break;

        case RID_SCDLG_HFEDIT_RIGHTHEADER:
            AddTabPage( 1, ScRightHeaderEditPage::Create, NULL );
            break;

        case RID_SCDLG_HFEDIT_LEFTFOOTER:
            AddTabPage( 1, ScLeftFooterEditPage::Create, NULL );
            break;

        case RID_SCDLG_HFEDIT_RIGHTFOOTER:
            AddTabPage( 1, ScRightFooterEditPage::Create, NULL );
            break;

        case RID_SCDLG_HFEDIT_SHDR:
            AddTabPage( 1, ScRightHeaderEditPage::Create, NULL );
            AddTabPage( 2, ScRightFooterEditPage::Create, NULL );
            AddTabPage( 3, ScLeftFooterEditPage::Create,  NULL );
            break;

        case RID_SCDLG_HFEDIT_SFTR:
            AddTabPage( 1, ScRightHeaderEditPage::Create, NULL );
            AddTabPage( 2, ScLeftHeaderEditPage::Create, NULL );
            AddTabPage( 3, ScRightFooterEditPage::Create, NULL );
            break;

        case RID_SCDLG_HFEDIT_ALL:
            AddTabPage( 1, ScRightHeaderEditPage::Create, NULL );
            AddTabPage( 2, ScLeftHeaderEditPage::Create, NULL );
            AddTabPage( 3, ScRightFooterEditPage::Create, NULL );
            AddTabPage( 4, ScLeftFooterEditPage::Create, NULL );
            break;

        default:
        case RID_SCDLG_HFEDIT:
            {
                const SvxPageItem&  rPageItem = (const SvxPageItem&)
                            rCoreSet.Get(
                                rCoreSet.GetPool()->GetWhich(SID_ATTR_PAGE) );

                sal_Bool bRightPage = ( SVX_PAGE_LEFT !=
                                    SvxPageUsage(rPageItem.GetPageUsage()) );

                if ( bRightPage )
                {
                    AddTabPage( 1, ScRightHeaderEditPage::Create, NULL );
                    AddTabPage( 2, ScRightFooterEditPage::Create, NULL );
                }
                else
                {
                    //  #69193a# respect "shared" setting

                    sal_Bool bShareHeader = IS_SHARE_HEADER(rCoreSet);
                    if ( bShareHeader )
                        AddTabPage( 1, ScRightHeaderEditPage::Create, NULL );
                    else
                        AddTabPage( 1, ScLeftHeaderEditPage::Create, NULL );

                    sal_Bool bShareFooter = IS_SHARE_FOOTER(rCoreSet);
                    if ( bShareFooter )
                        AddTabPage( 2, ScRightFooterEditPage::Create, NULL );
                    else
                        AddTabPage( 2, ScLeftFooterEditPage::Create, NULL );
                }
            }
            break;
    }

    FreeResource();
}

// -----------------------------------------------------------------------

ScHFEditDlg::~ScHFEditDlg()
{
}

// -----------------------------------------------------------------------

void ScHFEditDlg::PageCreated( sal_uInt16 /* nId */, SfxTabPage& rPage )
{
    // kann ja nur ne ScHFEditPage sein...

    ((ScHFEditPage&)rPage).SetNumType(eNumType);
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
