/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfedtdlg.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 14:19:54 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



//------------------------------------------------------------------

#include "scitems.hxx"
#include <svtools/eitem.hxx>

#include "hfedtdlg.hxx"
#include "global.hxx"
#include "globstr.hrc"
//CHINA001 #include "tphfedit.hxx"
#include "scresid.hxx"
#include "hfedtdlg.hrc"
#include "scuitphfedit.hxx" //CHINA001
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

ScHFEditDlg::ScHFEditDlg( SfxViewFrame*     pFrame,
                          Window*           pParent,
                          const SfxItemSet& rCoreSet,
                          const String&     rPageStyle,
                          USHORT            nResId )
    :   SfxTabDialog( pFrame, pParent, ScResId( nResId ), &rCoreSet )
{
    eNumType = ((const SvxPageItem&)rCoreSet.Get(ATTR_PAGE)).GetNumType();

    String aTmp = GetText();

    aTmp.AppendAscii(RTL_CONSTASCII_STRINGPARAM( " (" ));
    aTmp += ScGlobal::GetRscString( STR_PAGESTYLE );
    aTmp.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ": " ));
    aTmp += rPageStyle;
    aTmp += ')';
    SetText( aTmp );

    switch ( nResId )
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

                BOOL bRightPage = ( SVX_PAGE_LEFT !=
                                    SvxPageUsage(rPageItem.GetPageUsage()) );

                if ( bRightPage )
                {
                    AddTabPage( 1, ScRightHeaderEditPage::Create, NULL );
                    AddTabPage( 2, ScRightFooterEditPage::Create, NULL );
                }
                else
                {
                    //  #69193a# respect "shared" setting

                    BOOL bShareHeader = IS_SHARE_HEADER(rCoreSet);
                    if ( bShareHeader )
                        AddTabPage( 1, ScRightHeaderEditPage::Create, NULL );
                    else
                        AddTabPage( 1, ScLeftHeaderEditPage::Create, NULL );

                    BOOL bShareFooter = IS_SHARE_FOOTER(rCoreSet);
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

__EXPORT ScHFEditDlg::~ScHFEditDlg()
{
}

// -----------------------------------------------------------------------

void __EXPORT ScHFEditDlg::PageCreated( USHORT nId, SfxTabPage& rPage )
{
    // kann ja nur ne ScHFEditPage sein...

    ((ScHFEditPage&)rPage).SetNumType(eNumType);
}




