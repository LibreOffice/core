/*************************************************************************
 *
 *  $RCSfile: paragrph.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: os $ $Date: 2001-03-05 08:54:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#pragma hdrstop

#define ITEMID_LINESPACING  0
#define ITEMID_ADJUST       0
#define ITEMID_ORPHANS      0
#define ITEMID_WIDOWS       0
#define ITEMID_TABSTOP      0
#define ITEMID_PAGEMODEL    0
#define ITEMID_FMTSPLIT     0
#define ITEMID_HYPHENZONE   0
#define ITEMID_ULSPACE      0
#define ITEMID_LRSPACE      0
#define ITEMID_FMTBREAK     0
#define ITEMID_FMTKEEP      0
#define _SVX_PARAGRPH_CXX   0

#include "dialogs.hrc"
#include "paragrph.hrc"
#include "paragrph.hxx"

#include "lspcitem.hxx"
#include "adjitem.hxx"
#include "orphitem.hxx"
#include "widwitem.hxx"
#include "tstpitem.hxx"
#include "pmdlitem.hxx"
#include "spltitem.hxx"
#include "hyznitem.hxx"
#include "ulspitem.hxx"
#include "lrspitem.hxx"
#include "brkitem.hxx"
#include "keepitem.hxx"
#include "paractrl.hxx"
#include "dlgutil.hxx"
#include "dialmgr.hxx"
#include "htmlmode.hxx"

// static ----------------------------------------------------------------

static USHORT pStdRanges[] =
{
    SID_ATTR_PARA_LINESPACE,        // 10033
    SID_ATTR_PARA_LINESPACE,
    SID_ATTR_LRSPACE,               // 10048 -
    SID_ATTR_ULSPACE,               // 10049
    SID_ATTR_PARA_REGISTER,         // 10413
    SID_ATTR_PARA_REGISTER,
    0
};

static USHORT pAlignRanges[] =
{
    SID_ATTR_PARA_ADJUST,           // 10027
    SID_ATTR_PARA_ADJUST,
    0
};

static USHORT pExtRanges[] =
{
    SID_ATTR_PARA_PAGEBREAK,        // 10037 -
    SID_ATTR_PARA_ORPHANS,          // 10041
    SID_ATTR_PARA_MODEL,            // 10065 -
    SID_ATTR_PARA_KEEP,             // 10066
    0
};

// define ----------------------------------------------------------------

#define MAX_DURCH 5670      // 10 cm ist sinnvoll als maximaler Durchschuss
                            // laut BP
#define FIX_DIST_DEF 283    // Standard-Fix-Abstand 0,5cm

// enum ------------------------------------------------------------------

enum LineSpaceList
{
    LLINESPACE_1    = 0,
    LLINESPACE_15   = 1,
    LLINESPACE_2    = 2,
    LLINESPACE_PROP = 3,
    LLINESPACE_MIN  = 4,
    LLINESPACE_DURCH= 5,
    LLINESPACE_FIX  = 6,
    LLINESPACE_END
};

// C-Funktion ------------------------------------------------------------

void SetLineSpace_Impl( SvxLineSpacingItem&, int, long lValue = 0 );

void SetLineSpace_Impl( SvxLineSpacingItem& rLineSpace,
                        int eSpace, long lValue )
{
    switch ( eSpace )
    {
        case LLINESPACE_1:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
            break;

        case LLINESPACE_15:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetPropLineSpace( 150 );
            break;

        case LLINESPACE_2:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetPropLineSpace( 200 );
            break;

        case LLINESPACE_PROP:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetPropLineSpace( (BYTE)lValue );
            break;

        case LLINESPACE_MIN:
            rLineSpace.SetLineHeight( (USHORT)lValue );
            rLineSpace.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
            break;

        case LLINESPACE_DURCH:
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_AUTO;
            rLineSpace.SetInterLineSpace( (USHORT)lValue );
            break;

        case LLINESPACE_FIX:
            rLineSpace.SetLineHeight((USHORT)lValue);
            rLineSpace.GetLineSpaceRule() = SVX_LINE_SPACE_FIX;
            rLineSpace.GetInterLineSpaceRule() = SVX_INTER_LINE_SPACE_OFF;
        break;
    }
}


USHORT GetHtmlMode_Impl(const SfxItemSet& rSet)
{
    USHORT nHtmlMode = 0;
    const SfxPoolItem* pItem = 0;
    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, FALSE, &pItem) ||
        ( 0 != (pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        nHtmlMode = ((SfxUInt16Item*)pItem)->GetValue();
    }
    return nHtmlMode;

}

// class SvxStdParagraphTabPage ------------------------------------------

IMPL_LINK( SvxStdParagraphTabPage, ELRLoseFocusHdl, Edit *, EMPTYARG )
{
//! if ( aLeftIndent.IsRelativeMode() )
//!     return 0; //!!!

    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit =
        MapToFieldUnit( pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) ) );

    long nL = aLeftIndent.Denormalize( aLeftIndent.GetValue( eUnit ) );
    long nR = aRightIndent.Denormalize( aRightIndent.GetValue( eUnit ) );
    String aTmp = aFLineIndent.GetText();

    // Erstzeilen Einzug
    if( aLeftIndent.GetMin() < 0 )
        aFLineIndent.SetMin( -99999, FUNIT_MM );
    else
        aFLineIndent.SetMin( aFLineIndent.Normalize( -nL ), eUnit );

    // Check nur fuer konkrete Breite (Shell)
    long nTmp = nWidth - nL - nR - MM50;
    aFLineIndent.SetMax( aFLineIndent.Normalize( nTmp ), eUnit );

    if ( !aTmp.Len() )
        aFLineIndent.SetEmptyFieldValue();
    // Maximum Links Rechts
    aTmp = aLeftIndent.GetText();
    nTmp = nWidth - nR - MM50;
    aLeftIndent.SetMax( aLeftIndent.Normalize( nTmp ), eUnit );

    if ( !aTmp.Len() )
        aLeftIndent.SetEmptyFieldValue();
    aTmp = aRightIndent.GetText();
    nTmp = nWidth - nL - MM50;
    aRightIndent.SetMax( aRightIndent.Normalize( nTmp ), eUnit );

    if ( !aTmp.Len() )
        aRightIndent.SetEmptyFieldValue();
    return 0;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxStdParagraphTabPage::Create( Window* pParent,
                                            const SfxItemSet& rSet)
{
    return new SvxStdParagraphTabPage( pParent, rSet );
}

// -----------------------------------------------------------------------

BOOL SvxStdParagraphTabPage::FillItemSet( SfxItemSet& rOutSet )
{
    SfxItemState eState = SFX_ITEM_UNKNOWN;
    const SfxPoolItem* pOld = 0;
    SfxItemPool* pPool = rOutSet.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );

    BOOL bModified = FALSE;
    USHORT nWhich;
    USHORT nPos = aLineDist.GetSelectEntryPos();

    if ( LISTBOX_ENTRY_NOTFOUND != nPos &&
         ( nPos != aLineDist.GetSavedValue() ||
           aLineDistAtPercentBox.IsValueModified() ||
           aLineDistAtMetricBox.IsValueModified() ) )
    {
        nWhich = GetWhich( SID_ATTR_PARA_LINESPACE );
        SfxMapUnit eUnit = pPool->GetMetric( nWhich );
        SvxLineSpacingItem aSpacing(
            (const SvxLineSpacingItem&)GetItemSet().Get( nWhich ) );

        switch ( nPos )
        {
            case LLINESPACE_1:
            case LLINESPACE_15:
            case LLINESPACE_2:
                SetLineSpace_Impl( aSpacing, nPos );
                break;

            case LLINESPACE_PROP:
                SetLineSpace_Impl( aSpacing, nPos,
                                   aLineDistAtPercentBox.Denormalize(
                                   aLineDistAtPercentBox.GetValue() ) );
                break;

            case LLINESPACE_MIN:
            case LLINESPACE_DURCH:
            case LLINESPACE_FIX:
                SetLineSpace_Impl( aSpacing, nPos,
                    GetCoreValue( aLineDistAtMetricBox, eUnit ) );
            break;

            default:
                DBG_ERROR( "unbekannter Type fuer Zeilenabstand." );
                break;
        }
        eState = GetItemSet().GetItemState( nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_LINESPACE );

        if ( !pOld || !( *(const SvxLineSpacingItem*)pOld == aSpacing ) ||
             SFX_ITEM_DONTCARE == eState )
        {
            rOutSet.Put( aSpacing );
            bModified |= TRUE;
        }
    }

    if ( aTopDist.IsValueModified() || aBottomDist.IsValueModified() )
    {
        nWhich = GetWhich( SID_ATTR_ULSPACE );
        SfxMapUnit eUnit = pPool->GetMetric( nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_ULSPACE );
        SvxULSpaceItem aMargin( nWhich );

        if ( bRelativeMode )
        {
            DBG_ASSERT( GetItemSet().GetParent(), "No ParentSet" );

            const SvxULSpaceItem& rOldItem =
                (const SvxULSpaceItem&)GetItemSet().GetParent()->Get( nWhich );

            if ( aTopDist.IsRelative() )
                aMargin.SetUpper( rOldItem.GetUpper(),
                                  (USHORT)aTopDist.GetValue() );
            else
                aMargin.SetUpper( (USHORT)GetCoreValue( aTopDist, eUnit ) );

            if ( aBottomDist.IsRelative() )
                aMargin.SetLower( rOldItem.GetLower(),
                                  (USHORT)aBottomDist.GetValue() );
            else
                aMargin.SetLower( (USHORT)GetCoreValue( aBottomDist, eUnit ) );

        }
        else
        {
            aMargin.SetUpper( (USHORT)GetCoreValue( aTopDist, eUnit ) );
            aMargin.SetLower( (USHORT)GetCoreValue( aBottomDist, eUnit ) );
        }
        eState = GetItemSet().GetItemState( nWhich );

        if ( !pOld || !( *(const SvxULSpaceItem*)pOld == aMargin ) ||
             SFX_ITEM_DONTCARE == eState )
        {
            rOutSet.Put( aMargin );
            bModified |= TRUE;
        }
    }
    FASTBOOL bNullTab = FALSE;

    if ( aLeftIndent.IsValueModified() ||
         aFLineIndent.IsValueModified() ||
         aRightIndent.IsValueModified()
         ||  aAutoCB.GetSavedValue() != aAutoCB.IsChecked() )
    {
        nWhich = GetWhich( SID_ATTR_LRSPACE );
        SfxMapUnit eUnit = pPool->GetMetric( nWhich );
        SvxLRSpaceItem aMargin( nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_LRSPACE );

        if ( bRelativeMode )
        {
            DBG_ASSERT( GetItemSet().GetParent(), "No ParentSet" );

            const SvxLRSpaceItem& rOldItem =
                (const SvxLRSpaceItem&)GetItemSet().GetParent()->Get( nWhich );

            if ( aLeftIndent.IsRelative() )
                aMargin.SetTxtLeft( rOldItem.GetTxtLeft(),
                                    aLeftIndent.GetValue() );
            else
                aMargin.SetTxtLeft( GetCoreValue( aLeftIndent, eUnit ) );

            if ( aRightIndent.IsRelative() )
                aMargin.SetRight( rOldItem.GetRight(),
                                  aRightIndent.GetValue() );
            else
                aMargin.SetRight( GetCoreValue( aRightIndent, eUnit ) );

            if ( aFLineIndent.IsRelative() )
                aMargin.SetTxtFirstLineOfst( rOldItem.GetTxtFirstLineOfst(),
                                             (USHORT)aFLineIndent.GetValue() );
            else
                aMargin.SetTxtFirstLineOfst(
                    (USHORT)GetCoreValue( aFLineIndent, eUnit ) );
        }
        else
        {
            aMargin.SetTxtLeft( GetCoreValue( aLeftIndent, eUnit ) );
            aMargin.SetRight( GetCoreValue( aRightIndent, eUnit ) );
            aMargin.SetTxtFirstLineOfst(
                (USHORT)GetCoreValue( aFLineIndent, eUnit ) );
        }
        aMargin.SetAutoFirst(aAutoCB.IsChecked());
        if ( aMargin.GetTxtFirstLineOfst() < 0 )
            bNullTab = TRUE;
        eState = GetItemSet().GetItemState( nWhich );

        if ( !pOld || !( *(const SvxLRSpaceItem*)pOld == aMargin ) ||
             SFX_ITEM_DONTCARE == eState )
        {
            rOutSet.Put( aMargin );
            bModified |= TRUE;
        }
    }

    if ( bNullTab )
    {
        // negativer Erstzeileneinzug -> ggf. Null Default-Tabstop setzen
        USHORT nWhich = GetWhich( SID_ATTR_TABSTOP );
        const SfxItemSet& rInSet = GetItemSet();

        if ( rInSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
        {
            const SvxTabStopItem& rTabItem =
                (const SvxTabStopItem&)rInSet.Get( nWhich );
            SvxTabStopItem aNullTab( rTabItem );
            SvxTabStop aNull( 0, SVX_TAB_ADJUST_DEFAULT );
            aNullTab.Insert( aNull );
            rOutSet.Put( aNullTab );
        }
    }
    if( aRegisterCB.IsVisible())
    {
        const SfxBoolItem* pBoolItem = (SfxBoolItem*)GetOldItem(
                            rOutSet, SID_ATTR_PARA_REGISTER);
        SfxBoolItem* pRegItem = (SfxBoolItem*)pBoolItem->Clone();
        USHORT nWhich = GetWhich( SID_ATTR_PARA_REGISTER );
        BOOL bSet = pRegItem->GetValue();

        if(aRegisterCB.IsChecked() != bSet )
        {
            pRegItem->SetValue(!bSet);
            rOutSet.Put(*pRegItem);
            bModified |= TRUE;
        }
        else if ( SFX_ITEM_DEFAULT == GetItemSet().GetItemState( nWhich, FALSE ) )
            rOutSet.ClearItem(nWhich);
        delete pRegItem;
    }

    return bModified;
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::Reset( const SfxItemSet& rSet )
{
    SfxItemPool* pPool = rSet.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool?" );
    String aEmpty;

    // Metrik einstellen
    FieldUnit eFUnit = GetModuleFieldUnit( &rSet );
    SetFieldUnit( aLeftIndent, eFUnit );
    SetFieldUnit( aRightIndent, eFUnit );
    SetFieldUnit( aFLineIndent, eFUnit );
    SetFieldUnit( aTopDist, eFUnit );
    SetFieldUnit( aBottomDist, eFUnit );
    SetFieldUnit( aLineDistAtMetricBox, eFUnit );

    USHORT nWhich = GetWhich( SID_ATTR_LRSPACE );
    SfxItemState eItemState = rSet.GetItemState( nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        SfxMapUnit eUnit = pPool->GetMetric( nWhich );

        if ( bRelativeMode )
        {
            const SvxLRSpaceItem& rOldItem =
                (const SvxLRSpaceItem&)rSet.Get( nWhich );

            if ( rOldItem.GetPropLeft() != 100 )
            {
                aLeftIndent.SetRelative( TRUE );
                aLeftIndent.SetValue( rOldItem.GetPropLeft() );
            }
            else
            {
                aLeftIndent.SetRelative();
                SetFieldUnit( aLeftIndent, eFUnit );
                SetMetricValue( aLeftIndent, rOldItem.GetTxtLeft(), eUnit );
                if(bNegativeIndents)
                    aLeftIndent.SetMin(-9999);
            }

            if ( rOldItem.GetPropRight() != 100 )
            {
                aRightIndent.SetRelative( TRUE );
                aRightIndent.SetValue( rOldItem.GetPropRight() );
            }
            else
            {
                aRightIndent.SetRelative();
                SetFieldUnit( aRightIndent, eFUnit );
                SetMetricValue( aRightIndent, rOldItem.GetRight(), eUnit );
                if(bNegativeIndents)
                    aRightIndent.SetMin(-9999);
            }

            if ( rOldItem.GetPropTxtFirstLineOfst() != 100 )
            {
                aFLineIndent.SetRelative( TRUE );
                aFLineIndent.SetValue( rOldItem.GetPropTxtFirstLineOfst() );
            }
            else
            {
                aFLineIndent.SetRelative();
                aFLineIndent.SetMin(-9999);
                SetFieldUnit( aFLineIndent, eFUnit );
                SetMetricValue( aFLineIndent, rOldItem.GetTxtFirstLineOfst(),
                                eUnit );
            }
            aAutoCB.Check(rOldItem.IsAutoFirst());
        }
        else
        {
            const SvxLRSpaceItem& rSpace =
                (const SvxLRSpaceItem&)rSet.Get( nWhich );

            SetMetricValue( aLeftIndent, rSpace.GetTxtLeft(), eUnit );
            SetMetricValue( aRightIndent, rSpace.GetRight(), eUnit );
            SetMetricValue( aFLineIndent, rSpace.GetTxtFirstLineOfst(), eUnit );
            aAutoCB.Check(rSpace.IsAutoFirst());
        }
        AutoHdl_Impl(&aAutoCB);
    }
    else
    {
        aLeftIndent.SetEmptyFieldValue();
        aRightIndent.SetEmptyFieldValue();
        aFLineIndent.SetEmptyFieldValue();
    }

    nWhich = GetWhich( SID_ATTR_ULSPACE );
    eItemState = rSet.GetItemState( nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        SfxMapUnit eUnit = pPool->GetMetric( nWhich );

        if ( bRelativeMode )
        {
            const SvxULSpaceItem& rOldItem =
                (const SvxULSpaceItem&)rSet.Get( nWhich );

            if ( rOldItem.GetPropUpper() != 100 )
            {
                aTopDist.SetRelative( TRUE );
                aTopDist.SetValue( rOldItem.GetPropUpper() );
            }
            else
            {
                aTopDist.SetRelative();
                SetFieldUnit( aTopDist, eFUnit );
                SetMetricValue( aTopDist, rOldItem.GetUpper(), eUnit );
            }

            if ( rOldItem.GetPropLower() != 100 )
            {
                aBottomDist.SetRelative( TRUE );
                aBottomDist.SetValue( rOldItem.GetPropLower() );
            }
            else
            {
                aBottomDist.SetRelative();
                SetFieldUnit( aBottomDist, eFUnit );
                SetMetricValue( aBottomDist, rOldItem.GetLower(), eUnit );
            }
        }
        else
        {
            const SvxULSpaceItem& rTopMargin =
                (const SvxULSpaceItem&)rSet.Get( nWhich );
            SetMetricValue( aTopDist, rTopMargin.GetUpper(), eUnit );
            SetMetricValue( aBottomDist, rTopMargin.GetLower(), eUnit );
        }
    }
    else
    {
        aTopDist.SetEmptyFieldValue();
        aBottomDist.SetEmptyFieldValue();
    }

    nWhich = GetWhich( SID_ATTR_PARA_LINESPACE );
    eItemState = rSet.GetItemState( nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
        SetLineSpacing_Impl( (const SvxLineSpacingItem &)rSet.Get( nWhich ) );
    else
        aLineDist.SetNoSelection();


    nWhich = GetWhich( SID_ATTR_PARA_REGISTER );
    eItemState = rSet.GetItemState( nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
        aRegisterCB.Check( ((const SfxBoolItem &)rSet.Get( nWhich )).GetValue());
    aRegisterCB.SaveValue();
    USHORT nHtmlMode = GetHtmlMode_Impl(rSet);
    if(nHtmlMode & HTMLMODE_ON)
    {
        aRegisterCB.Hide();
        aRegisterGB.Hide();
        aAutoCB.Hide();
        if(!(nHtmlMode & HTMLMODE_SOME_STYLES)) // IE oder SW
        {
            aRightLabel.Disable();
            aRightIndent.Disable();
            aTopDist.Disable();  //HTML3.2 und NS 3.0
            aBottomDist.Disable();
            if(!(nHtmlMode & HTMLMODE_FIRSTLINE)) //NS 3.0
            {
                aFLineIndent.Disable();
                aFLineLabel.Disable();
            }
        }
    }

    ELRLoseFocusHdl( NULL );
    aAutoCB.SaveValue();
    aLineDist.SaveValue();
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::EnableRelativeMode()
{
    DBG_ASSERT( GetItemSet().GetParent(), "RelativeMode, but no parent-set!" );

    aLeftIndent.EnableRelativeMode( 0, 999 );
    aFLineIndent.EnableRelativeMode( 0, 999 );
    aRightIndent.EnableRelativeMode( 0, 999 );
    aTopDist.EnableRelativeMode( 0, 999 );
    aBottomDist.EnableRelativeMode( 0, 999 );
    bRelativeMode = TRUE;
}

// -----------------------------------------------------------------------

int SvxStdParagraphTabPage::DeactivatePage( SfxItemSet* pSet )
{
    ELRLoseFocusHdl( NULL );

    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

SvxStdParagraphTabPage::SvxStdParagraphTabPage( Window* pParent,
                                                const SfxItemSet& rAttr ) :

    SfxTabPage( pParent, ResId( RID_SVXPAGE_STD_PARAGRAPH, DIALOG_MGR() ), rAttr ),

    aLeftLabel              ( this, ResId( FT_LEFTINDENT ) ),
    aLeftIndent             ( this, ResId( ED_LEFTINDENT ) ),
    aFLineLabel             ( this, ResId( FT_FLINEINDENT ) ),
    aFLineIndent            ( this, ResId( ED_FLINEINDENT ) ),
    aAutoCB                 ( this, ResId( CB_AUTO ) ),
    aRightLabel             ( this, ResId( FT_RIGHTINDENT ) ),
    aRightIndent            ( this, ResId( ED_RIGHTINDENT ) ),
    aIndentFrm              ( this, ResId( GB_INDENT ) ),
    aTopLabel               ( this, ResId( FT_TOPDIST ) ),
    aTopDist                ( this, ResId( ED_TOPDIST ) ),
    aBottomLabel            ( this, ResId( FT_BOTTOMDIST ) ),
    aBottomDist             ( this, ResId( ED_BOTTOMDIST ) ),
    aDistFrm                ( this, ResId( GB_DIST ) ),
    aLineDist               ( this, ResId( LB_LINEDIST ) ),
    aLineDistAtLabel        ( this, ResId( FT_LINEDIST ) ),
    aLineDistAtPercentBox   ( this, ResId( ED_LINEDISTPERCENT ) ),
    aLineDistAtMetricBox    ( this, ResId( ED_LINEDISTMETRIC ) ),
    aLineDistFrm            ( this, ResId( GB_LINEDIST ) ),
    aExampleWin             ( this, ResId( WN_EXAMPLE ) ),
    aExampleFrm             ( this, ResId( GB_EXAMPLE ) ),
    aRegisterCB             ( this, ResId( CB_REGISTER ) ),
    aRegisterGB             ( this, ResId( GB_REGISTER ) ),
    sAbsDist                ( ResId(ST_LINEDIST_ABS) ),
    nMinFixDist(0L),

    nAbst           ( MAX_DURCH ),
    nWidth          ( 11905 /*567 * 50*/ ),
    pActLineDistFld ( &aLineDistAtPercentBox ),
    bRelativeMode   ( FALSE ),
    bNegativeIndents(FALSE)

{
    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    aLineDistAtMetricBox.Hide();
    FreeResource();
    Init_Impl();
    aFLineIndent.SetMin(-9999); // wird default auf 0 gesetzt
}


// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::EnableNegativeMode()
{
     aLeftIndent.SetMin(-9999);
    aRightIndent.SetMin(-9999);
    bNegativeIndents = TRUE;
}

// -----------------------------------------------------------------------

USHORT* SvxStdParagraphTabPage::GetRanges()
{
    return pStdRanges;
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::SetLineSpacing_Impl
(
    const SvxLineSpacingItem &rAttr
)
{
    SfxMapUnit eUnit = GetItemSet().GetPool()->GetMetric( rAttr.Which() );

    switch( rAttr.GetLineSpaceRule() )
    {
        case SVX_LINE_SPACE_AUTO:
        {
            SvxInterLineSpace eInter = rAttr.GetInterLineSpaceRule();

            switch( eInter )
            {
                // Default einzeilig
                case SVX_INTER_LINE_SPACE_OFF:
                    aLineDist.SelectEntryPos( LLINESPACE_1 );
                    break;

                // Default einzeilig
                case SVX_INTER_LINE_SPACE_PROP:
                    if ( 100 == rAttr.GetPropLineSpace() )
                    {
                        aLineDist.SelectEntryPos( LLINESPACE_1 );
                        break;
                    }
                    // 1.5zeilig
                    if ( 150 == rAttr.GetPropLineSpace() )
                    {
                        aLineDist.SelectEntryPos( LLINESPACE_15 );
                        break;
                    }
                    // 2zeilig
                    if ( 200 == rAttr.GetPropLineSpace() )
                    {
                        aLineDist.SelectEntryPos( LLINESPACE_2 );
                        break;
                    }
                    // eingestellter Prozentwert
                    aLineDistAtPercentBox.
                        SetValue( aLineDistAtPercentBox.Normalize(
                                        rAttr.GetPropLineSpace() ) );
                    aLineDist.SelectEntryPos( LLINESPACE_PROP );
                    break;

                case SVX_INTER_LINE_SPACE_FIX:
                    SetMetricValue( aLineDistAtMetricBox,
                                    rAttr.GetInterLineSpace(), eUnit );
                    aLineDist.SelectEntryPos( LLINESPACE_DURCH );
                    break;
            }
        }
        break;
        case SVX_LINE_SPACE_FIX:
            SetMetricValue(aLineDistAtMetricBox, rAttr.GetLineHeight(), eUnit);
            aLineDist.SelectEntryPos( LLINESPACE_FIX );
        break;

        case SVX_LINE_SPACE_MIN:
            SetMetricValue(aLineDistAtMetricBox, rAttr.GetLineHeight(), eUnit);
            aLineDist.SelectEntryPos( LLINESPACE_MIN );
            break;
    }
    LineDistHdl_Impl( &aLineDist );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxStdParagraphTabPage, LineDistHdl_Impl, ListBox *, pBox )
{
    switch( pBox->GetSelectEntryPos() )
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            aLineDistAtLabel.Enable(FALSE);
            pActLineDistFld->Enable(FALSE);
            pActLineDistFld->SetText( String() );
            break;

        case LLINESPACE_DURCH:
            // Setzen eines sinnvollen Defaults?
            // MS Begrenzen min(10, aPageSize)
            aLineDistAtPercentBox.Hide();
            pActLineDistFld = &aLineDistAtMetricBox;
            aLineDistAtMetricBox.SetMin(0);


            if ( !aLineDistAtMetricBox.GetText().Len() )
                aLineDistAtMetricBox.SetValue(
                    aLineDistAtMetricBox.Normalize( 1 ) );
            aLineDistAtPercentBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            aLineDistAtLabel.Enable();
            break;

        case LLINESPACE_MIN:
            aLineDistAtPercentBox.Hide();
            pActLineDistFld = &aLineDistAtMetricBox;
            aLineDistAtMetricBox.SetMin(0);

            if ( !aLineDistAtMetricBox.GetText().Len() )
                aLineDistAtMetricBox.SetValue(
                    aLineDistAtMetricBox.Normalize( 10 ), FUNIT_TWIP );
            aLineDistAtPercentBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            aLineDistAtLabel.Enable();
            break;

        case LLINESPACE_PROP:
            aLineDistAtMetricBox.Hide();
            pActLineDistFld = &aLineDistAtPercentBox;

            if ( !aLineDistAtPercentBox.GetText().Len() )
                aLineDistAtPercentBox.SetValue(
                    aLineDistAtPercentBox.Normalize( 100 ), FUNIT_TWIP );
            aLineDistAtMetricBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            aLineDistAtLabel.Enable();
            break;
        case LLINESPACE_FIX:
        {
            aLineDistAtPercentBox.Hide();
            pActLineDistFld = &aLineDistAtMetricBox;
            long nTemp = aLineDistAtMetricBox.GetValue();
            aLineDistAtMetricBox.SetMin(aLineDistAtMetricBox.Normalize(nMinFixDist), FUNIT_TWIP);

            // wurde der Wert beim SetMin veraendert, dann ist es Zeit
            // fuer den default
            if ( aLineDistAtMetricBox.GetValue() != nTemp )
                SetMetricValue( aLineDistAtMetricBox,
                                    FIX_DIST_DEF, SFX_MAPUNIT_TWIP ); // fix gibt's nur im Writer
            aLineDistAtPercentBox.Hide();
            pActLineDistFld->Show();
            pActLineDistFld->Enable();
            aLineDistAtLabel.Enable();
        }
        break;
    }
    UpdateExample_Impl( TRUE );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxStdParagraphTabPage, ModifyHdl_Impl, SvxRelativeField *, EMPTYARG )
{
    UpdateExample_Impl();
    return 0;
}
IMPL_LINK_INLINE_END( SvxStdParagraphTabPage, ModifyHdl_Impl, SvxRelativeField *, EMPTYARG )

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::Init_Impl()
{
    aLineDist.SetSelectHdl(
        LINK( this, SvxStdParagraphTabPage, LineDistHdl_Impl ) );

    Link aLink = LINK( this, SvxStdParagraphTabPage, ELRLoseFocusHdl );
    aFLineIndent.SetLoseFocusHdl( aLink );
    aLeftIndent.SetLoseFocusHdl( aLink );
    aRightIndent.SetLoseFocusHdl( aLink );

    aLink = LINK( this, SvxStdParagraphTabPage, ModifyHdl_Impl );
    aFLineIndent.SetModifyHdl( aLink );
    aLeftIndent.SetModifyHdl( aLink );
    aRightIndent.SetModifyHdl( aLink );
    aTopDist.SetModifyHdl( aLink );
    aBottomDist.SetModifyHdl( aLink );

    aAutoCB.SetClickHdl( LINK( this, SvxStdParagraphTabPage, AutoHdl_Impl ));
    SfxItemPool* pPool = GetItemSet().GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    FieldUnit eUnit =
        MapToFieldUnit( pPool->GetMetric( GetWhich( SID_ATTR_LRSPACE ) ) );

    aTopDist.SetMax( aTopDist.Normalize( nAbst ), eUnit );
    aBottomDist.SetMax( aBottomDist.Normalize( nAbst ), eUnit );
    aLineDistAtMetricBox.SetMax(
        aLineDistAtMetricBox.Normalize( nAbst ), eUnit );
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::UpdateExample_Impl( BOOL bAll )
{
    aExampleWin.SetFirstLineOfst(
        (short)aFLineIndent.Denormalize( aFLineIndent.GetValue( FUNIT_TWIP ) ) );
    aExampleWin.SetLeftMargin(
        aLeftIndent.Denormalize( aLeftIndent.GetValue( FUNIT_TWIP ) ) );
    aExampleWin.SetRightMargin(
        aRightIndent.Denormalize( aRightIndent.GetValue( FUNIT_TWIP ) ) );
    aExampleWin.SetUpper(
        (USHORT)aTopDist.Denormalize( aTopDist.GetValue( FUNIT_TWIP ) ) );
    aExampleWin.SetLower(
        (USHORT)aBottomDist.Denormalize( aBottomDist.GetValue( FUNIT_TWIP ) ) );



    USHORT nPos = aLineDist.GetSelectEntryPos();

    switch ( nPos )
    {
        case LLINESPACE_1:
        case LLINESPACE_15:
        case LLINESPACE_2:
            aExampleWin.SetLineSpace( (SvxPrevLineSpace)nPos );
            break;

        case LLINESPACE_PROP:
            aExampleWin.SetLineSpace( (SvxPrevLineSpace)nPos,
                (USHORT)aLineDistAtPercentBox.Denormalize(
                aLineDistAtPercentBox.GetValue() ) );
            break;

        case LLINESPACE_MIN:
        case LLINESPACE_DURCH:
        case LLINESPACE_FIX:
            aExampleWin.SetLineSpace( (SvxPrevLineSpace)nPos,
                (USHORT)GetCoreValue( aLineDistAtMetricBox, SFX_MAPUNIT_TWIP ) );
            break;
    }
    aExampleWin.Draw( bAll );
}

// -----------------------------------------------------------------------

void SvxStdParagraphTabPage::EnableRegisterMode()
{
    aRegisterCB.Show();
    aRegisterGB.Show();
}

/*-----------------16.01.97 19.54-------------------

--------------------------------------------------*/
IMPL_LINK( SvxStdParagraphTabPage, AutoHdl_Impl, CheckBox*, pBox )
{
    BOOL bEnable = !pBox->IsChecked();
    aFLineLabel .Enable(bEnable);
    aFLineIndent.Enable(bEnable);
    return 0;
}

/*-----------------16.01.97 18.00-------------------

--------------------------------------------------*/
void SvxStdParagraphTabPage::SetPageWidth( USHORT nPageWidth )
{
    nWidth = nPageWidth;
}
/*-----------------16.01.97 18.01-------------------

--------------------------------------------------*/
void SvxStdParagraphTabPage::SetMaxDistance( USHORT nMaxDist )
{
    nAbst = nMaxDist;
}

/*-----------------17.01.97 08.11-------------------

--------------------------------------------------*/
void SvxStdParagraphTabPage::EnableAutoFirstLine()
{
    aAutoCB.Show();
}

/*-----------------11.06.97 11.48-------------------
    absoluter Zeilenabstand
--------------------------------------------------*/
void    SvxStdParagraphTabPage::EnableAbsLineDist(long nMinTwip)
{
    aLineDist.InsertEntry(sAbsDist);
    nMinFixDist = nMinTwip;
}


// class SvxParaAlignTabPage ------------------------------------------------

/*-----------------16.01.97 19.34-------------------

--------------------------------------------------*/
SvxParaAlignTabPage::SvxParaAlignTabPage( Window* pParent, const SfxItemSet& rSet )
    : SfxTabPage(pParent, ResId( RID_SVXPAGE_ALIGN_PARAGRAPH, DIALOG_MGR() ),rSet),
    aLeft                   ( this, ResId( BTN_LEFTALIGN ) ),
    aRight                  ( this, ResId( BTN_RIGHTALIGN ) ),
    aCenter                 ( this, ResId( BTN_CENTERALIGN ) ),
    aJustify                ( this, ResId( BTN_JUSTIFYALIGN ) ),
    aAlignFrm               ( this, ResId( GB_ALIGN ) ),
    aLastLineFT             ( this, ResId( FT_LASTLINE ) ),
    aLastLineLB             ( this, ResId( LB_LASTLINE ) ),
    aExpandCB               ( this, ResId( CB_EXPAND ) ),
    aExampleWin             ( this, ResId( WN_EXAMPLE ) ),
    aExampleFrm             ( this, ResId( GB_EXAMPLE ) )
{
    FreeResource();
    Link aLink = LINK( this, SvxParaAlignTabPage, AlignHdl_Impl );
    aLeft.SetClickHdl( aLink );
    aRight.SetClickHdl( aLink );
    aCenter.SetClickHdl( aLink );
    aJustify.SetClickHdl( aLink );
}

/*-----------------16.01.97 19.33-------------------

--------------------------------------------------*/
SvxParaAlignTabPage::~SvxParaAlignTabPage()
{
}

/*-----------------16.01.97 19.33-------------------

--------------------------------------------------*/
int SvxParaAlignTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

/*-----------------16.01.97 19.33-------------------

--------------------------------------------------*/
SfxTabPage* SvxParaAlignTabPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxParaAlignTabPage(pParent, rSet);
}

/*-----------------16.01.97 19.33-------------------

--------------------------------------------------*/
USHORT* SvxParaAlignTabPage::GetRanges()
{
    return pAlignRanges;

}

/*-----------------16.01.97 19.33-------------------

--------------------------------------------------*/
BOOL SvxParaAlignTabPage::FillItemSet( SfxItemSet& rOutSet )
{
    BOOL bModified = FALSE;

    FASTBOOL bAdj = FALSE, bChecked = FALSE;
    SvxAdjust eAdjust = SVX_ADJUST_LEFT;

    if ( aLeft.IsChecked() )
    {
        eAdjust = SVX_ADJUST_LEFT;
        bAdj = !aLeft.GetSavedValue();
        bChecked = TRUE;
    }
    else if ( aRight.IsChecked() )
    {
        eAdjust = SVX_ADJUST_RIGHT;
        bAdj = !aRight.GetSavedValue();
        bChecked = TRUE;
    }
    else if ( aCenter.IsChecked() )
    {
        eAdjust = SVX_ADJUST_CENTER;
        bAdj = !aCenter.GetSavedValue();
        bChecked = TRUE;
    }
    else if ( aJustify.IsChecked() )
    {
        eAdjust = SVX_ADJUST_BLOCK;
        bAdj = !aJustify.GetSavedValue() ||
            aExpandCB.IsChecked() != aExpandCB.GetSavedValue() ||
            aLastLineLB.GetSelectEntryPos() != aLastLineLB.GetSavedValue();
        bChecked = TRUE;
    }
    USHORT nWhich = GetWhich( SID_ATTR_PARA_ADJUST );

    if ( bAdj )
    {
        const SvxAdjustItem* pOld =
            (const SvxAdjustItem*)GetOldItem( rOutSet, SID_ATTR_PARA_ADJUST );
        SvxAdjust eOneWord = aExpandCB.IsChecked() ? SVX_ADJUST_BLOCK
                                                   : SVX_ADJUST_LEFT;
        USHORT nLBPos = aLastLineLB.GetSelectEntryPos();
        SvxAdjust eLastBlock = SVX_ADJUST_LEFT;

        if ( 1 == nLBPos )
            eLastBlock = SVX_ADJUST_CENTER;
        else if ( 2 == nLBPos )
            eLastBlock = SVX_ADJUST_BLOCK;

        FASTBOOL bNothingWasChecked =
            !aLeft.GetSavedValue() && !aRight.GetSavedValue() &&
            !aCenter.GetSavedValue() && !aJustify.GetSavedValue();

        if ( !pOld || pOld->GetAdjust() != eAdjust ||
             pOld->GetOneWord() != eOneWord ||
             pOld->GetLastBlock() != eLastBlock ||
             ( bChecked && bNothingWasChecked ) )
        {
            bModified |= TRUE;
            SvxAdjustItem aAdj(
                (const SvxAdjustItem&)GetItemSet().Get( nWhich ) );
            aAdj.SetAdjust( eAdjust );
            aAdj.SetOneWord( eOneWord );
            aAdj.SetLastBlock( eLastBlock );
            rOutSet.Put( aAdj );
        }
    }
    return bModified;
}

/*-----------------16.01.97 19.33-------------------

--------------------------------------------------*/
void SvxParaAlignTabPage::Reset( const SfxItemSet& rSet )
{
    USHORT nWhich = GetWhich( SID_ATTR_PARA_ADJUST );
    SfxItemState eItemState = rSet.GetItemState( nWhich );

    USHORT nLBSelect = 0;
    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        const SvxAdjustItem& rAdj = (const SvxAdjustItem&)rSet.Get( nWhich );

        switch ( rAdj.GetAdjust() /*!!! VB fragen rAdj.GetLastBlock()*/ )
        {
            case SVX_ADJUST_LEFT: aLeft.Check(); break;

            case SVX_ADJUST_RIGHT: aRight.Check(); break;

            case SVX_ADJUST_CENTER: aCenter.Check(); break;

            case SVX_ADJUST_BLOCK: aJustify.Check(); break;
        }
        BOOL bEnable = aJustify.IsChecked();
        aLastLineFT.Enable(bEnable);
        aLastLineLB.Enable(bEnable);
        aExpandCB  .Enable(bEnable);

        aExpandCB.Check(SVX_ADJUST_BLOCK == rAdj.GetOneWord());
        switch(rAdj.GetLastBlock())
        {
            case SVX_ADJUST_LEFT:  nLBSelect = 0; break;

            case SVX_ADJUST_CENTER: nLBSelect = 1;  break;

            case SVX_ADJUST_BLOCK: nLBSelect = 2;  break;

        }
    }
    else
    {
        aLeft.Check( FALSE );
        aRight.Check( FALSE );
        aCenter.Check( FALSE );
        aJustify.Check( FALSE );
    }
    aLastLineLB.SelectEntryPos(nLBSelect);

    USHORT nHtmlMode = GetHtmlMode_Impl(rSet);
    if(nHtmlMode & HTMLMODE_ON)
    {
        aLastLineLB.Hide();
        aLastLineFT.Hide();
        aExpandCB.Hide();
        if(!(nHtmlMode & HTMLMODE_FULL_STYLES|HTMLMODE_FIRSTLINE) )
            aJustify.Disable();
    }

    aLeft.SaveValue();
    aRight.SaveValue();
    aCenter.SaveValue();
    aJustify.SaveValue();
    aLastLineLB.SaveValue();
    aExpandCB.SaveValue();

    UpdateExample_Impl(TRUE);
}

/*-----------------17.01.97 08.06-------------------

--------------------------------------------------*/
IMPL_LINK( SvxParaAlignTabPage, AlignHdl_Impl, RadioButton*, pBtn )
{
    BOOL bJustify = aJustify.IsChecked();
    aLastLineFT.Enable(bJustify);
    aLastLineLB.Enable(bJustify);
    aExpandCB.Enable(bJustify);
    UpdateExample_Impl(FALSE);
    return 0;
}

/*-----------------16.01.97 19.34-------------------

--------------------------------------------------*/
void    SvxParaAlignTabPage::UpdateExample_Impl( BOOL bAll )
{
    if ( aLeft.IsChecked() )
        aExampleWin.SetAdjust( SVX_ADJUST_LEFT );
    else if ( aRight.IsChecked() )
        aExampleWin.SetAdjust( SVX_ADJUST_RIGHT );
    else if ( aCenter.IsChecked() )
        aExampleWin.SetAdjust( SVX_ADJUST_CENTER );
    else if ( aJustify.IsChecked() )
    {
        aExampleWin.SetAdjust( SVX_ADJUST_BLOCK );
        SvxAdjust eLastBlock = SVX_ADJUST_LEFT;
        USHORT nLBPos = aLastLineLB.GetSelectEntryPos();
        if(nLBPos == 1)
            eLastBlock = SVX_ADJUST_CENTER;
        else if(nLBPos == 2)
            eLastBlock = SVX_ADJUST_BLOCK;
        aExampleWin.SetLastLine( eLastBlock );
    }

    aExampleWin.Draw( bAll );
}
/*-----------------17.01.97 08.04-------------------
    Erweiterungen fuer den Blocksatz einschalten
--------------------------------------------------*/
void SvxParaAlignTabPage::EnableJustifyExt()
{
    aLastLineFT.Show();
    aLastLineLB.Show();
    aExpandCB  .Show();

}


// class SvxExtParagraphTabPage ------------------------------------------

SfxTabPage* SvxExtParagraphTabPage::Create( Window* pParent,
                                            const SfxItemSet& rSet )
{
    return new SvxExtParagraphTabPage( pParent, rSet );
}

// -----------------------------------------------------------------------

BOOL SvxExtParagraphTabPage::FillItemSet( SfxItemSet& rOutSet )
{
    BOOL bModified = FALSE;
    USHORT nWhich = GetWhich( SID_ATTR_PARA_HYPHENZONE );
    const TriState eHyphenState = aHyphenBox.GetState();
    const SfxPoolItem* pOld = GetOldItem( rOutSet, SID_ATTR_PARA_HYPHENZONE );

    if ( eHyphenState != aHyphenBox.GetSavedValue()     ||
         aExtHyphenBeforeBox.IsValueModified()          ||
         aExtHyphenAfterBox.IsValueModified()           ||
         aMaxHyphenEdit.IsValueModified() )
    {
        SvxHyphenZoneItem aHyphen(
            (const SvxHyphenZoneItem&)GetItemSet().Get( nWhich ) );
        aHyphen.SetHyphen( eHyphenState == STATE_CHECK );

        if ( eHyphenState == STATE_CHECK )
        {
            aHyphen.GetMinLead() = (BYTE)aExtHyphenBeforeBox.GetValue();
            aHyphen.GetMinTrail() = (BYTE)aExtHyphenAfterBox.GetValue();
        }
        aHyphen.GetMaxHyphens() = (BYTE)aMaxHyphenEdit.GetValue();

        if ( !pOld ||
            !( *(SvxHyphenZoneItem*)pOld == aHyphen ) ||
                eHyphenState != aHyphenBox.GetSavedValue())
        {
            rOutSet.Put( aHyphen );
            bModified |= TRUE;
        }
    }

    if (aPagenumEdit.IsEnabled() && aPagenumEdit.IsValueModified())
    {
        SfxUInt16Item aPageNum( SID_ATTR_PARA_PAGENUM,
                                (USHORT)aPagenumEdit.GetValue() );

        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_PAGENUM );

        if ( !pOld || ( (const SfxUInt16Item*)pOld )->GetValue() != aPageNum.GetValue() )
        {
            rOutSet.Put( aPageNum );
            bModified |= TRUE;
        }
    }

    // Seitenumbruch

    TriState eState = aApplyCollBtn.GetState();
    FASTBOOL bIsPageModel = FALSE;

    nWhich = GetWhich( SID_ATTR_PARA_MODEL );
    String sPage;
    if ( eState != aApplyCollBtn.GetSavedValue() ||
         ( STATE_CHECK == eState &&
           aApplyCollBox.GetSelectEntryPos() != aApplyCollBox.GetSavedValue() ) )
    {
        if ( eState == STATE_CHECK )
        {
            sPage = aApplyCollBox.GetSelectEntry();
            bIsPageModel = 0 != sPage.Len();
        }
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_MODEL );

        if ( !pOld || ( (const SvxPageModelItem*)pOld )->GetValue() != sPage )
        {
            rOutSet.Put( SvxPageModelItem( sPage, FALSE, nWhich ) );
            bModified |= TRUE;
        }
        else
            bIsPageModel = FALSE;
    }
    else if(STATE_CHECK == eState && aApplyCollBtn.IsEnabled())
        bIsPageModel = TRUE;
    else
        rOutSet.Put( SvxPageModelItem( sPage, FALSE, nWhich ) );

    nWhich = GetWhich( SID_ATTR_PARA_PAGEBREAK );

    if ( bIsPageModel )
        // wird PageModel eingeschaltet, dann immer PageBreak ausschalten
        rOutSet.Put( SvxFmtBreakItem( SVX_BREAK_NONE, nWhich ) );
    else
    {
        eState = aPageBreakBox.GetState();
        SfxItemState eModelState = GetItemSet().GetItemState(SID_ATTR_PARA_MODEL, FALSE);

        if ( (eModelState == SFX_ITEM_SET && STATE_CHECK == aPageBreakBox.GetState()) ||
             eState != aPageBreakBox.GetSavedValue()                ||
             aBeforeBox.IsChecked() != aBeforeBox.GetSavedValue()   ||
             aPageBox.IsChecked() != aPageBox.GetSavedValue() )
        {
            const SvxFmtBreakItem rOldBreak(
                    (const SvxFmtBreakItem&)GetItemSet().Get( nWhich ));
            SvxFmtBreakItem aBreak(rOldBreak.GetBreak(), rOldBreak.Which());

            switch ( eState )
            {
                case STATE_CHECK:
                {
                    BOOL bBefore = aBeforeBox.IsChecked();

                    if ( aPageBox.IsChecked() )
                    {
                        if ( bBefore )
                            aBreak.SetValue( SVX_BREAK_PAGE_BEFORE );
                        else
                            aBreak.SetValue( SVX_BREAK_PAGE_AFTER );
                    }
                    else
                    {
                        if ( bBefore )
                            aBreak.SetValue( SVX_BREAK_COLUMN_BEFORE );
                        else
                            aBreak.SetValue( SVX_BREAK_COLUMN_AFTER );
                    }
                    break;
                }

                case STATE_NOCHECK:
                    aBreak.SetValue( SVX_BREAK_NONE );
                    break;
            }
            pOld = GetOldItem( rOutSet, SID_ATTR_PARA_PAGEBREAK );

            if ( eState != aPageBreakBox.GetSavedValue()                ||
                    !pOld || !( *(const SvxFmtBreakItem*)pOld == aBreak ) )
            {
                bModified |= TRUE;
                rOutSet.Put( aBreak );
            }
        }
    }


    // Absatztrennung
    nWhich = GetWhich( SID_ATTR_PARA_SPLIT );
    eState = aKeepTogetherBox.GetState();

    if ( eState != aKeepTogetherBox.GetSavedValue() )
    {
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_SPLIT );

        if ( !pOld || ( (const SvxFmtSplitItem*)pOld )->GetValue() !=
                      ( eState == STATE_NOCHECK ) )
        {
            rOutSet.Put( SvxFmtSplitItem( eState == STATE_NOCHECK, nWhich ) );
            bModified |= TRUE;
        }
    }

    // Absaetze zusammenhalten
    nWhich = GetWhich( SID_ATTR_PARA_KEEP );
    eState = aKeepParaBox.GetState();

    if ( eState != aKeepParaBox.GetSavedValue() )
    {
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_KEEP );

        // hat sich der Status geaendert, muss immer geputtet werden
        rOutSet.Put( SvxFmtKeepItem( eState == STATE_CHECK, nWhich ) );
        bModified |= TRUE;
    }

    // Witwen und Waisen
    nWhich = GetWhich( SID_ATTR_PARA_WIDOWS );
    eState = aWidowBox.GetState();

    if ( eState != aWidowBox.GetSavedValue() ||
         aWidowRowNo.IsValueModified() )
    {
        SvxWidowsItem rItem( eState == STATE_CHECK ?
                             (BYTE)aWidowRowNo.GetValue() : 0, nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_WIDOWS );

        if ( eState != aWidowBox.GetSavedValue() || !pOld || !( *(const SvxWidowsItem*)pOld == rItem ) )
        {
            rOutSet.Put( rItem );
            bModified |= TRUE;
        }
    }

    nWhich = GetWhich( SID_ATTR_PARA_ORPHANS );
    eState = aOrphanBox.GetState();

    if ( eState != aOrphanBox.GetSavedValue() ||
         aOrphanRowNo.IsValueModified() )
    {
        SvxOrphansItem rItem( eState == STATE_CHECK ?
                             (BYTE)aOrphanRowNo.GetValue() : 0, nWhich );
        pOld = GetOldItem( rOutSet, SID_ATTR_PARA_ORPHANS );

        if ( eState != aOrphanBox.GetSavedValue() ||
                !pOld ||
                    !( *(const SvxOrphansItem*)pOld == rItem ) )
        {
            rOutSet.Put( rItem );
            bModified |= TRUE;
        }
    }
    return bModified;
}

// -----------------------------------------------------------------------

void SvxExtParagraphTabPage::Reset( const SfxItemSet& rSet )
{
    USHORT nWhich = GetWhich( SID_ATTR_PARA_HYPHENZONE );
    SfxItemState eItemState = rSet.GetItemState( nWhich );

    BOOL bItemAvailable = eItemState >= SFX_ITEM_AVAILABLE;
    BOOL bIsHyphen = FALSE;
    if( !bHtmlMode && bItemAvailable )
    {
        const SvxHyphenZoneItem& rHyphen =
            (const SvxHyphenZoneItem&)rSet.Get( nWhich );
        aHyphenBox.EnableTriState( FALSE );

        bIsHyphen = rHyphen.IsHyphen();
        aHyphenBox.SetState( bIsHyphen ? STATE_CHECK : STATE_NOCHECK );

        aExtHyphenBeforeBox.SetValue( rHyphen.GetMinLead() );
        aExtHyphenAfterBox.SetValue( rHyphen.GetMinTrail() );
        aMaxHyphenEdit.SetValue( rHyphen.GetMaxHyphens() );
    }
    else
    {
        aHyphenBox.SetState( STATE_DONTKNOW );
    }
    BOOL bEnable = bItemAvailable && bIsHyphen;
    aExtHyphenBeforeBox.Enable(bEnable);
    aHyphenBeforeLabel.Enable(bEnable);
    aExtHyphenAfterBox.Enable(bEnable);
    aHyphenAfterLabel.Enable(bEnable);
    aBeforeText.Enable(bEnable);
    aAfterText.Enable(bEnable);
    aMaxHyphenLabel.Enable(bEnable);
    aMaxHyphenEdit.Enable(bEnable);

    nWhich = GetWhich( SID_ATTR_PARA_PAGENUM );

    if ( rSet.GetItemState(nWhich) >= SFX_ITEM_AVAILABLE )
    {
        const USHORT nPageNum =
            ( (const SfxUInt16Item&)rSet.Get( nWhich ) ).GetValue();
        aPagenumEdit.SetValue( nPageNum );
    }

    if ( bPageBreak )
    {
        // zuerst PageModel behandeln
        nWhich = GetWhich( SID_ATTR_PARA_MODEL );
        BOOL bIsPageModel = FALSE;
        eItemState = rSet.GetItemState( nWhich );

        if ( eItemState >= SFX_ITEM_SET )
        {
            aApplyCollBtn.EnableTriState( FALSE );

            const SvxPageModelItem& rModel =
                (const SvxPageModelItem&)rSet.Get( nWhich );
            String aStr( rModel.GetValue() );

            if ( aStr.Len() &&
                 aApplyCollBox.GetEntryPos( aStr ) != LISTBOX_ENTRY_NOTFOUND )
            {
                aApplyCollBox.SelectEntry( aStr );
                aApplyCollBtn.SetState( STATE_CHECK );
                bIsPageModel = TRUE;

                aPageBreakBox.Enable();
                aPageBreakBox.EnableTriState( FALSE );
                aPageBox.Enable();
                aColumnBox.Enable(!bHtmlMode);
                aBeforeBox.Enable();
                aAfterBox.Enable();
                aApplyCollBtn.Enable();
                aPageBreakBox.SetState( STATE_CHECK );

                aPageBox.Check( TRUE );
                aColumnBox.Check( FALSE );
                aBeforeBox.Check( TRUE );
                aAfterBox.Check( FALSE );
            }
            else
            {
                aApplyCollBox.SetNoSelection();
                aApplyCollBtn.SetState( STATE_NOCHECK );
            }
        }
        else if ( SFX_ITEM_DONTCARE == eItemState )
        {
            aApplyCollBtn.EnableTriState( TRUE );
            aApplyCollBtn.SetState( STATE_DONTKNOW );
            aApplyCollBox.SetNoSelection();
        }
        else
        {
            aApplyCollBtn.Enable(FALSE);
            aApplyCollBox.Enable(FALSE);
            aPagenumEdit.Enable(FALSE);
            aPagenumText.Enable(FALSE);
        }
//!!!   ApplyCollClickHdl_Impl( &aApplyCollBtn );

        if ( !bIsPageModel )
        {
            nWhich = GetWhich( SID_ATTR_PARA_PAGEBREAK );
            eItemState = rSet.GetItemState( nWhich );

            if ( eItemState >= SFX_ITEM_AVAILABLE )
            {
                const SvxFmtBreakItem& rPageBreak =
                    (const SvxFmtBreakItem&)rSet.Get( nWhich );

                SvxBreak eBreak = (SvxBreak)rPageBreak.GetValue();

                // PageBreak nicht ueber CTRL-RETURN,
                // dann kann CheckBox frei gegeben werden
                aPageBreakBox.Enable();
                aPageBreakBox.EnableTriState( FALSE );
                aPageBox.Enable();
                aColumnBox.Enable(!bHtmlMode);
                aBeforeBox.Enable();
                aAfterBox.Enable();
                aPageBreakBox.SetState( STATE_CHECK );

                BOOL bEnable =  eBreak != SVX_BREAK_NONE &&
                                eBreak != SVX_BREAK_COLUMN_BEFORE &&
                                eBreak != SVX_BREAK_COLUMN_AFTER;
                aApplyCollBtn.Enable(bEnable);
                if(!bEnable)
                {
                    aApplyCollBox.Enable(bEnable);
                    aPagenumEdit.Enable(bEnable);
                }

                if ( eBreak == SVX_BREAK_NONE )
                    aPageBreakBox.SetState( STATE_NOCHECK );

                switch ( eBreak )
                {
                    case SVX_BREAK_PAGE_BEFORE:
                        aPageBox.Check( TRUE );
                        aColumnBox.Check( FALSE );
                        aBeforeBox.Check( TRUE );
                        aAfterBox.Check( FALSE );
                        break;
                    case SVX_BREAK_PAGE_AFTER:
                        aPageBox.Check( TRUE );
                        aColumnBox.Check( FALSE );
                        aBeforeBox.Check( FALSE );
                        aAfterBox.Check( TRUE );
                        break;
                    case SVX_BREAK_COLUMN_BEFORE:
                        aPageBox.Check( FALSE );
                        aColumnBox.Check( TRUE );
                        aBeforeBox.Check( TRUE );
                        aAfterBox.Check( FALSE );
                        break;
                    case SVX_BREAK_COLUMN_AFTER:
                        aPageBox.Check( FALSE );
                        aColumnBox.Check( TRUE );
                        aBeforeBox.Check( FALSE );
                        aAfterBox.Check( TRUE );
                        break;
                }
            }
            else if ( SFX_ITEM_DONTCARE == eItemState )
                aPageBreakBox.SetState( STATE_DONTKNOW );
            else
            {
                aPageBreakBox.Enable(FALSE);
                aPageBox.Enable(FALSE);
                aColumnBox.Enable(FALSE);
                aBeforeBox.Enable(FALSE);
                aAfterBox.Enable(FALSE);
            }
        }

        if ( aBeforeBox.IsChecked() )
            PageBreakPosHdl_Impl( &aBeforeBox );
        else if ( aAfterBox.IsChecked() )
            PageBreakPosHdl_Impl( &aAfterBox );
        PageBreakHdl_Impl( &aPageBreakBox );
    }

    nWhich = GetWhich( SID_ATTR_PARA_KEEP );
    eItemState = rSet.GetItemState( nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        aKeepParaBox.EnableTriState( FALSE );
        const SvxFmtKeepItem& rKeep =
            (const SvxFmtKeepItem&)rSet.Get( nWhich );

        if ( rKeep.GetValue() )
            aKeepParaBox.SetState( STATE_CHECK );
        else
            aKeepParaBox.SetState( STATE_NOCHECK );
    }
    else if ( SFX_ITEM_DONTCARE == eItemState )
        aKeepParaBox.SetState( STATE_DONTKNOW );
    else
        aKeepParaBox.Enable(FALSE);

    nWhich = GetWhich( SID_ATTR_PARA_SPLIT );
    eItemState = rSet.GetItemState( nWhich );

    if ( eItemState >= SFX_ITEM_AVAILABLE )
    {
        const SvxFmtSplitItem& rSplit =
            (const SvxFmtSplitItem&)rSet.Get( nWhich );
        aKeepTogetherBox.EnableTriState( FALSE );

        if ( !rSplit.GetValue() )
            aKeepTogetherBox.SetState( STATE_CHECK );
        else
        {
            aKeepTogetherBox.SetState( STATE_NOCHECK );

            // Witwen und Waisen
            aWidowBox.Enable();
            nWhich = GetWhich( SID_ATTR_PARA_WIDOWS );
            SfxItemState eTmpState = rSet.GetItemState( nWhich );

            if ( eTmpState >= SFX_ITEM_AVAILABLE )
            {
                const SvxWidowsItem& rWidow =
                    (const SvxWidowsItem&)rSet.Get( nWhich );
                aWidowBox.EnableTriState( FALSE );
                const USHORT nLines = rWidow.GetValue();

                BOOL bEnable = nLines > 0;
                aWidowRowNo.SetValue( aWidowRowNo.Normalize( nLines ) );
                aWidowBox.SetState( bEnable ? STATE_CHECK : STATE_NOCHECK);
                aWidowRowNo.Enable(bEnable);
                aWidowRowLabel.Enable(bEnable);

            }
            else if ( SFX_ITEM_DONTCARE == eTmpState )
                aWidowBox.SetState( STATE_DONTKNOW );
            else
                aWidowBox.Enable(FALSE);

            aOrphanBox.Enable();
            nWhich = GetWhich( SID_ATTR_PARA_ORPHANS );
            eTmpState = rSet.GetItemState( nWhich );

            if ( eTmpState >= SFX_ITEM_AVAILABLE )
            {
                const SvxOrphansItem& rOrphan =
                    (const SvxOrphansItem&)rSet.Get( nWhich );
                const USHORT nLines = rOrphan.GetValue();
                aOrphanBox.EnableTriState( FALSE );

                BOOL bEnable = nLines > 0;
                aOrphanBox.SetState( bEnable ? STATE_CHECK : STATE_NOCHECK);
                aOrphanRowNo.SetValue( aOrphanRowNo.Normalize( nLines ) );
                aOrphanRowNo.Enable(bEnable);
                aOrphanRowLabel.Enable(bEnable);

            }
            else if ( SFX_ITEM_DONTCARE == eTmpState )
                aOrphanBox.SetState( STATE_DONTKNOW );
            else
                aOrphanBox.Enable(FALSE);
        }
    }
    else if ( SFX_ITEM_DONTCARE == eItemState )
        aKeepTogetherBox.SetState( STATE_DONTKNOW );
    else
        aKeepTogetherBox.Enable(FALSE);

    // damit alles richt enabled wird
    KeepTogetherHdl_Impl( 0 );
    WidowHdl_Impl( 0 );
    OrphanHdl_Impl( 0 );

    aHyphenBox.SaveValue();
    aExtHyphenBeforeBox.SaveValue();
    aExtHyphenAfterBox.SaveValue();
    aMaxHyphenEdit.SaveValue();
    aPageBreakBox.SaveValue();
    aPageBox.SaveValue();
    aColumnBox.SaveValue();
    aBeforeBox.SaveValue();
    aAfterBox.SaveValue();
    aApplyCollBtn.SaveValue();
    aApplyCollBox.SaveValue();
    aPagenumEdit.SaveValue();
    aKeepTogetherBox.SaveValue();
    aKeepParaBox.SaveValue();
    aWidowBox.SaveValue();
    aOrphanBox.SaveValue();
}

// -----------------------------------------------------------------------

int SvxExtParagraphTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

void SvxExtParagraphTabPage::DisablePageBreak()
{
    bPageBreak = FALSE;
    aPageBreakBox.Enable(FALSE);
    aPageBox.Enable(FALSE);
    aApplyCollBtn.Enable(FALSE);
    aApplyCollBox.Enable(FALSE);
    aPagenumEdit.Enable(FALSE);
    aBeforeBox .Enable(FALSE);
    aAfterBox  .Enable(FALSE);
}

// -----------------------------------------------------------------------

SvxExtParagraphTabPage::SvxExtParagraphTabPage( Window* pParent, const SfxItemSet& rAttr ) :

    SfxTabPage( pParent, ResId( RID_SVXPAGE_EXT_PARAGRAPH, DIALOG_MGR() ), rAttr ),

    aHyphenBox          ( this, ResId( BTN_HYPHEN ) ),
    aBeforeText         ( this, ResId( FT_BEFORE ) ),
    aExtHyphenBeforeBox ( this, ResId( ED_HYPHENBEFORE ) ),
    aHyphenBeforeLabel  ( this, ResId( FT_HYPHENBEFORE ) ),
    aAfterText          ( this, ResId( FT_AFTER ) ),
    aExtHyphenAfterBox  ( this, ResId( ED_HYPHENAFTER ) ),
    aHyphenAfterLabel   ( this, ResId( FT_HYPHENAFTER ) ),
    aMaxHyphenLabel     ( this, ResId( FT_MAXHYPH ) ),
    aMaxHyphenEdit      ( this, ResId( ED_MAXHYPH ) ),
    aExtFrm             ( this, ResId( GB_HYPHEN ) ),
    aPageBreakBox       ( this, ResId( BTN_PAGEBREAK ) ),
    aPageBox            ( this, ResId( BTN_BREAKPAGE ) ),
    aColumnBox          ( this, ResId( BTN_BREAKCOLUMN ) ),
    aBeforeBox          ( this, ResId( BTN_PAGEBREAKBEFORE ) ),
    aAfterBox           ( this, ResId( BTN_PAGEBREAKAFTER ) ),
    aApplyCollBtn       ( this, ResId( BTN_PAGECOLL ) ),
    aApplyCollBox       ( this, ResId( LB_PAGECOLL ) ),
    aPagenumText        ( this, ResId( FT_PAGENUM ) ),
    aPagenumEdit        ( this, ResId( ED_PAGENUM ) ),
    aKeepTogetherBox    ( this, ResId( BTN_KEEPTOGETHER ) ),
    aKeepParaBox        ( this, ResId( CB_KEEPTOGETHER ) ),
    aWidowBox           ( this, ResId( BTN_WIDOWS ) ),
    aWidowRowNo         ( this, ResId( ED_WIDOWS ) ),
    aWidowRowLabel      ( this, ResId( FT_WIDOWS ) ),
    aOrphanBox          ( this, ResId( BTN_ORPHANS ) ),
    aOrphanRowNo        ( this, ResId( ED_ORPHANS ) ),
    aOrphanRowLabel     ( this, ResId( FT_ORPHANS ) ),
    aExtendFrm          ( this, ResId( GB_OPTIONS ) ),

    bHtmlMode   ( FALSE ),
    bPageBreak  ( TRUE ),
    nStdPos     ( 0 )

{
    FreeResource();

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    aHyphenBox.SetClickHdl(         LINK( this, SvxExtParagraphTabPage, HyphenClickHdl_Impl ) );
    aPageBreakBox.SetClickHdl(      LINK( this, SvxExtParagraphTabPage, PageBreakHdl_Impl ) );
    aKeepTogetherBox.SetClickHdl(   LINK( this, SvxExtParagraphTabPage, KeepTogetherHdl_Impl ) );
    aWidowBox.SetClickHdl(          LINK( this, SvxExtParagraphTabPage, WidowHdl_Impl ) );
    aOrphanBox.SetClickHdl(         LINK( this, SvxExtParagraphTabPage, OrphanHdl_Impl ) );
    aApplyCollBtn.SetClickHdl(      LINK( this, SvxExtParagraphTabPage, ApplyCollClickHdl_Impl ) );
    aPageBox.SetClickHdl(           LINK( this, SvxExtParagraphTabPage, PageBreakTypeHdl_Impl ) );
    aColumnBox.SetClickHdl(         LINK( this, SvxExtParagraphTabPage, PageBreakTypeHdl_Impl ) );
    aBeforeBox.SetClickHdl(         LINK( this, SvxExtParagraphTabPage, PageBreakPosHdl_Impl ) );
    aAfterBox.SetClickHdl(          LINK( this, SvxExtParagraphTabPage, PageBreakPosHdl_Impl ) );

    SfxObjectShell* pSh = SfxObjectShell::Current();
    if ( pSh )
    {
        SfxStyleSheetBasePool* pPool = pSh->GetStyleSheetPool();
        pPool->SetSearchMask( SFX_STYLE_FAMILY_PAGE );
        SfxStyleSheetBase* pStyle = pPool->First();
        String aStdName;

        while( pStyle )
        {
            if ( aStdName.Len() == 0 )
                // first style == standard style
                aStdName = pStyle->GetName();
            aApplyCollBox.InsertEntry( pStyle->GetName() );
            pStyle = pPool->Next();
        }
        nStdPos = aApplyCollBox.GetEntryPos( aStdName );
    }

    USHORT nHtmlMode = GetHtmlMode_Impl( rAttr );
    if ( nHtmlMode & HTMLMODE_ON )
    {
        bHtmlMode = TRUE;
        aHyphenBox           .Enable(FALSE);
        aBeforeText          .Enable(FALSE);
        aExtHyphenBeforeBox  .Enable(FALSE);
        aHyphenBeforeLabel   .Enable(FALSE);
        aAfterText           .Enable(FALSE);
        aExtHyphenAfterBox   .Enable(FALSE);
        aHyphenAfterLabel    .Enable(FALSE);
        aMaxHyphenLabel      .Enable(FALSE);
        aMaxHyphenEdit       .Enable(FALSE);
        aExtFrm              .Enable(FALSE);
        aPagenumText         .Enable(FALSE);
        aPagenumEdit         .Enable(FALSE);
    }
}

// -----------------------------------------------------------------------

__EXPORT SvxExtParagraphTabPage::~SvxExtParagraphTabPage()
{
}

// -----------------------------------------------------------------------

USHORT* SvxExtParagraphTabPage::GetRanges()
{
    return pExtRanges;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, PageBreakHdl_Impl, TriStateBox *, EMPTYARG )
{
    switch ( aPageBreakBox.GetState() )
    {
        case STATE_CHECK:
            aPageBox.Enable();
            aColumnBox.Enable(!bHtmlMode);
            aBeforeBox.Enable();
            aAfterBox.Enable();

            if ( aPageBox.IsChecked() && aBeforeBox.IsChecked() )
            {
                aApplyCollBtn.Enable();

                BOOL bEnable = STATE_CHECK == aApplyCollBtn.GetState() &&
                                            aApplyCollBox.GetEntryCount();
                aApplyCollBox.Enable(bEnable);
                if(!bHtmlMode)
                {
                    aPagenumText.Enable(bEnable);
                    aPagenumEdit.Enable(bEnable);
                }
            }
            break;

        case STATE_NOCHECK:
        case STATE_DONTKNOW:
            aApplyCollBtn.SetState( STATE_NOCHECK );
            aApplyCollBtn.Enable(FALSE);
            aApplyCollBox.Enable(FALSE);
            aPagenumText.Enable(FALSE);
            aPagenumEdit.Enable(FALSE);
            aPageBox.Enable(FALSE);
            aColumnBox.Enable(FALSE);
            aBeforeBox.Enable(FALSE);
            aAfterBox.Enable(FALSE);
            break;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, KeepTogetherHdl_Impl, TriStateBox *, EMPTYARG )
{
    BOOL bEnable = aKeepTogetherBox.GetState() == STATE_NOCHECK;
    aWidowBox.Enable(bEnable);
    aOrphanBox.Enable(bEnable);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, WidowHdl_Impl, TriStateBox *, EMPTYARG )
{
    switch ( aWidowBox.GetState() )
    {
        case STATE_CHECK:
            aWidowRowNo.Enable();
            aWidowRowLabel.Enable();
            aKeepTogetherBox.Enable(FALSE);
            break;

        case STATE_NOCHECK:
            if ( aOrphanBox.GetState() == STATE_NOCHECK )
                aKeepTogetherBox.Enable();

        // kein break
        case STATE_DONTKNOW:
            aWidowRowNo.Enable(FALSE);
            aWidowRowLabel.Enable(FALSE);
            break;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, OrphanHdl_Impl, TriStateBox *, EMPTYARG )
{
    switch( aOrphanBox.GetState() )
    {
        case STATE_CHECK:
            aOrphanRowNo.Enable();
            aOrphanRowLabel.Enable();
            aKeepTogetherBox.Enable(FALSE);
            break;

        case STATE_NOCHECK:
            if ( aWidowBox.GetState() == STATE_NOCHECK )
                aKeepTogetherBox.Enable();

        // kein break
        case STATE_DONTKNOW:
            aOrphanRowNo.Enable(FALSE);
            aOrphanRowLabel.Enable(FALSE);
            break;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, HyphenClickHdl_Impl, TriStateBox *, EMPTYARG )
{

    BOOL bEnable = aHyphenBox.GetState() == STATE_CHECK;
    aBeforeText.Enable(bEnable);
    aHyphenBeforeLabel.Enable(bEnable);
    aExtHyphenBeforeBox.Enable(bEnable);
    aAfterText.Enable(bEnable);
    aHyphenAfterLabel.Enable(bEnable);
    aExtHyphenAfterBox.Enable(bEnable);
    aMaxHyphenLabel.Enable(bEnable);
    aMaxHyphenEdit.Enable(bEnable);
    aHyphenBox.SetState( bEnable ? STATE_CHECK : STATE_NOCHECK);

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, ApplyCollClickHdl_Impl, TriStateBox *, EMPTYARG )
{
    BOOL bEnable = FALSE;
    if ( aApplyCollBtn.GetState() == STATE_CHECK &&
         aApplyCollBox.GetEntryCount() )
    {
        bEnable = TRUE;
        aApplyCollBox.SelectEntryPos( nStdPos );
    }
    else
    {
        aApplyCollBox.SetNoSelection();
    }
    aApplyCollBox.Enable(bEnable);
    if(!bHtmlMode)
    {
        aPagenumText.Enable(bEnable);
        aPagenumEdit.Enable(bEnable);
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, PageBreakPosHdl_Impl, RadioButton *, pBtn )
{
    if ( aPageBox.IsChecked() )
    {
        if ( pBtn == &aBeforeBox )
        {
            aApplyCollBtn.Enable();

            BOOL bEnable = aApplyCollBtn.GetState() == STATE_CHECK &&
                                        aApplyCollBox.GetEntryCount();

            aApplyCollBox.Enable(bEnable);
            if(!bHtmlMode)
            {
                aPagenumText.Enable(bEnable);
                aPagenumEdit.Enable(bEnable);
            }
        }
        else if ( pBtn == &aAfterBox )
        {
            aApplyCollBtn.SetState( STATE_NOCHECK );
            aApplyCollBtn.Enable(FALSE);
            aApplyCollBox.Enable(FALSE);
            aPagenumText.Enable(FALSE);
            aPagenumEdit.Enable(FALSE);
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxExtParagraphTabPage, PageBreakTypeHdl_Impl, RadioButton *, pBtn )
{
    if ( pBtn == &aColumnBox || aAfterBox.IsChecked() )
    {
        aApplyCollBtn.SetState( STATE_NOCHECK );
        aApplyCollBtn.Enable(FALSE);
        aApplyCollBox.Enable(FALSE);
        aPagenumText.Enable(FALSE);
        aPagenumEdit.Enable(FALSE);
    }
    else if ( aBeforeBox.IsChecked() )
        PageBreakPosHdl_Impl( &aBeforeBox );
    return 0;
}
/*-- 29.11.00 11:36:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxAsianTabPage::SvxAsianTabPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, ResId( RID_SVXPAGE_PARA_ASIAN, DIALOG_MGR() ), rSet),
    aOptionsGB(         this, ResId(GB_AS_OPTIONS       )),
    aHangingPunctCB(    this, ResId(CB_AS_HANG_PUNC     )),
    aAllowWordBreakCB(  this, ResId(CB_AS_ALLOW_WORD_BREAK)),
    aForbiddenRulesCB(  this, ResId(CB_AS_FORBIDDEN     )),
    aCharDistGB(        this, ResId(GB_AS_CHAR_DIST     )),
    aPuntuationCB(      this, ResId(CB_AS_PUNCTUATION   )),
    aScriptSpaceCB(     this, ResId(CB_AS_SCRIPT_SPACE  )),
    aAdjustNumbersCB(   this, ResId(CB_AS_ADJUST_NUMBERS)),
    aTextAlignFT(       this, ResId(FT_TEXT_ALIGN       )),
    aTextAlignLB(       this, ResId(LB_TEXT_ALIGN       ))
{
    FreeResource();

    Link aLink = LINK( this, SvxAsianTabPage, ClickHdl_Impl );
    aHangingPunctCB.SetClickHdl( aLink );
    aScriptSpaceCB.SetClickHdl( aLink );
    aForbiddenRulesCB.SetClickHdl( aLink );
}
/*-- 29.11.00 11:36:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxAsianTabPage::~SvxAsianTabPage()
{
}
/*-- 29.11.00 11:36:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxTabPage* SvxAsianTabPage::Create(    Window* pParent, const SfxItemSet& rSet )
{
    return new SvxAsianTabPage(pParent, rSet);
}
/*-- 29.11.00 11:36:24---------------------------------------------------

  -----------------------------------------------------------------------*/
USHORT*     SvxAsianTabPage::GetRanges()
{
    static USHORT pRanges[] =
    {
        SID_ATTR_PARA_SCRIPTSPACE,
        SID_ATTR_PARA_HANGPUNCTUATION,
        SID_ATTR_PARA_FORBIDDEN_RULES,
        0
    };
    return pRanges;
}
/*-- 29.11.00 11:36:24---------------------------------------------------

  -----------------------------------------------------------------------*/
BOOL        SvxAsianTabPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bRet = FALSE;
    SfxItemPool* pPool = rSet.GetPool();
    if(aScriptSpaceCB.IsChecked() != aScriptSpaceCB.GetSavedValue())
    {
        SfxBoolItem* pNewItem = (SfxBoolItem*)rSet.Get(
            pPool->GetWhich(SID_ATTR_PARA_SCRIPTSPACE)).Clone();
        pNewItem->SetValue(aScriptSpaceCB.IsChecked());
        rSet.Put(*pNewItem);
        delete pNewItem;
        bRet = TRUE;
    }
    if(aHangingPunctCB.IsChecked() != aHangingPunctCB.GetSavedValue())
    {
        SfxBoolItem* pNewItem = (SfxBoolItem*)rSet.Get(
            pPool->GetWhich(SID_ATTR_PARA_HANGPUNCTUATION)).Clone();
        pNewItem->SetValue(aHangingPunctCB.IsChecked());
        rSet.Put(*pNewItem);
        delete pNewItem;
        bRet = TRUE;
    }
    if(aForbiddenRulesCB.IsChecked() != aForbiddenRulesCB.GetSavedValue())
    {
        SfxBoolItem* pNewItem = (SfxBoolItem*)rSet.Get(
            pPool->GetWhich(SID_ATTR_PARA_FORBIDDEN_RULES)).Clone();
        pNewItem->SetValue(aForbiddenRulesCB.IsChecked());
        rSet.Put(*pNewItem);
        delete pNewItem;
        bRet = TRUE;
    }
    return bRet;
}
/*-- 29.11.00 11:36:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_SetBox(const SfxItemSet& rSet, USHORT nSlotId, TriStateBox& rBox)
{
    USHORT nWhich = rSet.GetPool()->GetWhich(nSlotId);
    SfxItemState eState = rSet.GetItemState(nWhich, TRUE);
    if(!eState)
        rBox.Enable(FALSE);
    else if(eState >= SFX_ITEM_AVAILABLE)
    {
        rBox.EnableTriState( FALSE );
        rBox.Check(((const SfxBoolItem&)rSet.Get(nWhich)).GetValue());
    }
    else
        rBox.SetState( STATE_DONTKNOW );
    rBox.SaveValue();
}


void SvxAsianTabPage::Reset( const SfxItemSet& rSet )
{
    lcl_SetBox(rSet, SID_ATTR_PARA_FORBIDDEN_RULES, aForbiddenRulesCB );
//  lcl_SetBox(rSet, , aAllowWordBreakCB );
    lcl_SetBox(rSet, SID_ATTR_PARA_HANGPUNCTUATION, aHangingPunctCB );


    //character distance not yet available
//  lcl_SetBox(rSet, , aPuntuationCB    );
    lcl_SetBox(rSet, SID_ATTR_PARA_SCRIPTSPACE, aScriptSpaceCB );
//  lcl_SetBox(rSet, , aAdjustNumbersCB );
    aAllowWordBreakCB   .Enable(FALSE);
    aPuntuationCB       .Enable(FALSE);
    aAdjustNumbersCB    .Enable(FALSE);
    aTextAlignFT.Enable(FALSE);
    aTextAlignLB.Enable(FALSE);

}
/* -----------------------------19.12.00 12:59--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SvxAsianTabPage, ClickHdl_Impl, TriStateBox*, pBox )
{
    pBox->EnableTriState( FALSE );
    return 0;
}
