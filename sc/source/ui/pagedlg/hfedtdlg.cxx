/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

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
                          sal_uInt16            nResIdP )
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
