/*************************************************************************
 *
 *  $RCSfile: validate.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:55 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include <svtools/aeitem.hxx>
#include <svtools/stritem.hxx>
#include <svtools/eitem.hxx>
#include <vcl/svapp.hxx>
#include <basic/sbmeth.hxx>
#include <segmentc.hxx>
#include <basctl/basobj.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmod.hxx>
#endif

#include "scresid.hxx"
#include "sc.hrc"       // -> Slot IDs

#include "validat.hxx"

#include "validate.hrc"

#define _VALIDATE_CXX
#include "validate.hxx"
#undef _VALIDATE_CXX

// STATIC DATA -----------------------------------------------------------

static USHORT pValueRanges[] =
{
    FID_VALID_MODE, FID_VALID_ERRTEXT,
    0
};

static USHORT pHelpRanges[] =
{
    FID_VALIDATION,
    FID_VALIDATION,
    0
};

static USHORT pErrorRanges[] =
{
    FID_VALIDATION,
    FID_VALIDATION,
    0
};

SEG_EOFGLOBALS()

//==================================================================
#pragma SEG_FUNCDEF(validate_01)

ScValidationDlg::ScValidationDlg( Window*           pParent,
                      const SfxItemSet* pArgSet ) :
        SfxTabDialog( pParent,
                      ScResId( TAB_DLG_VALIDATION ),
                      pArgSet )
{
    AddTabPage( TP_VALIDATION_VALUES, ScTPValidationValue::Create,  0 );
    AddTabPage( TP_VALIDATION_INPUTHELP, ScTPValidationHelp::Create,  0 );
    AddTabPage( TP_VALIDATION_ERROR, ScTPValidationError::Create,  0 );
    FreeResource();
}

// -----------------------------------------------------------------------

#pragma SEG_FUNCDEF(validate_02)

__EXPORT ScValidationDlg::~ScValidationDlg()
{
}

//========================================================================
//========================================================================
// Werte-Tabpage:
#pragma SEG_FUNCDEF(validate_03)

ScTPValidationValue::ScTPValidationValue( Window*           pParent,
                                          const SfxItemSet& rArgSet )

    :   SfxTabPage      ( pParent,
                          ScResId( TP_VALIDATION_VALUES ),
                          rArgSet ),
        aFtAllow        ( this, ScResId( FT_ALLOW ) ),
        aLbAllow        ( this, ScResId( LB_ALLOW ) ),
        aTsbAllow       ( this, ScResId( TSB_ALLOW_BLANKS ) ),
        aFtValue        ( this, ScResId( FT_VALUE ) ),
        aLbValue        ( this, ScResId( LB_VALUE ) ),
        aFtMin          ( this, ScResId( FT_MIN ) ),
        aEdtMin         ( this, ScResId( EDT_MIN ) ),
        aFtMax          ( this, ScResId( FT_MAX ) ),
        aEdtMax         ( this, ScResId( EDT_MAX ) ),
        aGrpValues      ( this, ScResId( GRP_VALUES ) ),

        aStrMin         ( ScResId( SCSTR_MINIMUM ) ),
        aStrMax         ( ScResId( SCSTR_MAXIMUM ) ),
        aStrValue       ( ScResId( SCSTR_VALUE ) ),

        rSet            ( rArgSet )
{
    Init();
    FreeResource();
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_04)

__EXPORT ScTPValidationValue::~ScTPValidationValue()
{
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_05)

void ScTPValidationValue::Init()
{
    aLbAllow.SetSelectHdl( LINK( this, ScTPValidationValue, SelectAllowHdl ) );
    aLbValue.SetSelectHdl( LINK( this, ScTPValidationValue, SelectValueHdl ) );

    aLbAllow.SelectEntryPos( 0 );
    aTsbAllow.EnableTriState( FALSE );
    aLbValue.SelectEntryPos( 0 );

    SelectAllowHdl( NULL );
    SelectValueHdl( NULL );
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_06)

USHORT* __EXPORT ScTPValidationValue::GetRanges()
{
    return pValueRanges;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_07)

SfxTabPage* __EXPORT ScTPValidationValue::Create( Window*   pParent,
                                         const SfxItemSet&  rArgSet )
{
    return ( new ScTPValidationValue( pParent, rArgSet ) );
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_08)

void __EXPORT ScTPValidationValue::Reset( const SfxItemSet& rArgSet )
{
    const SfxPoolItem* pItem;

    if ( rArgSet.GetItemState( FID_VALID_MODE, TRUE, &pItem ) == SFX_ITEM_SET )
        aLbAllow.SelectEntryPos( ((const SfxAllEnumItem*)pItem)->GetValue() );
    if ( rArgSet.GetItemState( FID_VALID_CONDMODE, TRUE, &pItem ) == SFX_ITEM_SET )
        aLbValue.SelectEntryPos( ((const SfxAllEnumItem*)pItem)->GetValue() );
    if ( rArgSet.GetItemState( FID_VALID_VALUE1, TRUE, &pItem ) == SFX_ITEM_SET )
        aEdtMin.SetText( ((const SfxStringItem*)pItem)->GetValue() );
    if ( rArgSet.GetItemState( FID_VALID_VALUE2, TRUE, &pItem ) == SFX_ITEM_SET )
        aEdtMax.SetText( ((const SfxStringItem*)pItem)->GetValue() );
    if ( rArgSet.GetItemState( FID_VALID_BLANK, TRUE, &pItem ) == SFX_ITEM_SET )
        aTsbAllow.SetState( ((const SfxBoolItem*)pItem)->GetValue() ? STATE_CHECK : STATE_NOCHECK );
    else
        aTsbAllow.SetState( STATE_CHECK );  // default an

    SelectAllowHdl( NULL );
    SelectValueHdl( NULL );
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_09)

BOOL __EXPORT ScTPValidationValue::FillItemSet( SfxItemSet& rArgSet )
{
    rArgSet.Put( SfxAllEnumItem( FID_VALID_MODE, aLbAllow.GetSelectEntryPos() ) );
    rArgSet.Put( SfxAllEnumItem( FID_VALID_CONDMODE, aLbValue.GetSelectEntryPos() ) );
    rArgSet.Put( SfxStringItem( FID_VALID_VALUE1, aEdtMin.GetText() ) );
    rArgSet.Put( SfxStringItem( FID_VALID_VALUE2, aEdtMax.GetText() ) );
    rArgSet.Put( SfxBoolItem( FID_VALID_BLANK, aTsbAllow.GetState() == STATE_CHECK ) );

    return TRUE;
}


// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_0a)

IMPL_LINK( ScTPValidationValue, SelectAllowHdl, ListBox *, pLb )
{
    BOOL bEnable = aLbAllow.GetSelectEntryPos() != 0;

    aTsbAllow.Enable( bEnable );    // Leerzellen
    aFtValue.Enable( bEnable );
    aLbValue.Enable( bEnable );
    aFtMin.Enable( bEnable );
    aEdtMin.Enable( bEnable );
    aFtMax.Enable( bEnable );
    aEdtMax.Enable( bEnable );

    return( 0L );
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_0b)

IMPL_LINK( ScTPValidationValue, SelectValueHdl, ListBox *, pLb )
{
    USHORT nPos = aLbValue.GetSelectEntryPos();
    BOOL bShow = FALSE;

    switch( nPos )
    {
        case 0: // ==
        case 5: // !=
            aFtMin.SetText( aStrValue );
        break;

        case 1: // <
        case 3: // <=
            aFtMin.SetText( aStrMax );
        break;

        case 6: // zwischen
        case 7: // n. zwischen
            bShow = TRUE;
        case 2: // >
        case 4: // >=
            aFtMin.SetText( aStrMin );
        break;

        default:
            DBG_ERROR( "Wert ist nicht im Bereich der Stringlist!" );
        break;
    }

    bShow ? aFtMax.Show() : aFtMax.Hide();
    bShow ? aEdtMax.Show() : aEdtMax.Hide();

    return( 0L );
}

//========================================================================
//========================================================================
// Werte-Tabpage:
#pragma SEG_FUNCDEF(validate_0c)

ScTPValidationHelp::ScTPValidationHelp( Window*         pParent,
                                          const SfxItemSet& rArgSet )

    :   SfxTabPage      ( pParent,
                          ScResId( TP_VALIDATION_INPUTHELP ),
                          rArgSet ),
        aTsbHelp        ( this, ScResId( TSB_HELP ) ),
        aFtTitle        ( this, ScResId( FT_TITLE ) ),
        aEdtTitle       ( this, ScResId( EDT_TITLE ) ),
        aFtInputHelp    ( this, ScResId( FT_INPUTHELP ) ),
        aEdInputHelp    ( this, ScResId( EDT_INPUTHELP ) ),
        aGrpContent     ( this, ScResId( GRP_CONTENT ) ),

        rSet            ( rArgSet )
{
    Init();
    FreeResource();
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_0d)

__EXPORT ScTPValidationHelp::~ScTPValidationHelp()
{
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_0e)

void ScTPValidationHelp::Init()
{
    //aLb.SetSelectHdl( LINK( this, ScTPValidationHelp, SelectHdl ) );

    aTsbHelp.EnableTriState( FALSE );
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_0f)

USHORT* __EXPORT ScTPValidationHelp::GetRanges()
{
    return pValueRanges;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_10)

SfxTabPage* __EXPORT ScTPValidationHelp::Create( Window*    pParent,
                                         const SfxItemSet&  rArgSet )
{
    return ( new ScTPValidationHelp( pParent, rArgSet ) );
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_11)

void __EXPORT ScTPValidationHelp::Reset( const SfxItemSet& rArgSet )
{
    const SfxPoolItem* pItem;

    if ( rArgSet.GetItemState( FID_VALID_SHOWHELP, TRUE, &pItem ) == SFX_ITEM_SET )
        aTsbHelp.SetState( ((const SfxBoolItem*)pItem)->GetValue() ? STATE_CHECK : STATE_NOCHECK );
    if ( rArgSet.GetItemState( FID_VALID_HELPTITLE, TRUE, &pItem ) == SFX_ITEM_SET )
        aEdtTitle.SetText( ((const SfxStringItem*)pItem)->GetValue() );
    if ( rArgSet.GetItemState( FID_VALID_HELPTEXT, TRUE, &pItem ) == SFX_ITEM_SET )
        aEdInputHelp.SetText( ((const SfxStringItem*)pItem)->GetValue() );
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_12)

BOOL __EXPORT ScTPValidationHelp::FillItemSet( SfxItemSet& rArgSet )
{
    rArgSet.Put( SfxBoolItem( FID_VALID_SHOWHELP, aTsbHelp.GetState() == STATE_CHECK ) );
    rArgSet.Put( SfxStringItem( FID_VALID_HELPTITLE, aEdtTitle.GetText() ) );
    rArgSet.Put( SfxStringItem( FID_VALID_HELPTEXT, aEdInputHelp.GetText() ) );

    return TRUE;
}



//========================================================================
//========================================================================
// Werte-Tabpage:
#pragma SEG_FUNCDEF(validate_13)

ScTPValidationError::ScTPValidationError( Window*           pParent,
                                          const SfxItemSet& rArgSet )

    :   SfxTabPage      ( pParent,
                          ScResId( TP_VALIDATION_ERROR ),
                          rArgSet ),
        aTsbShow        ( this, ScResId( TSB_SHOW ) ),
        aFtAction       ( this, ScResId( FT_ACTION ) ),
        aLbAction       ( this, ScResId( LB_ACTION ) ),
        aBtnSearch      ( this, ScResId( BTN_SEARCH ) ),
        aFtTitle        ( this, ScResId( FT_TITLE ) ),
        aEdtTitle       ( this, ScResId( EDT_TITLE ) ),
        aFtError        ( this, ScResId( FT_ERROR ) ),
        aEdError        ( this, ScResId( EDT_ERROR ) ),
        aGrpContent     ( this, ScResId( GRP_CONTENT ) ),

        rSet            ( rArgSet )
{
    Init();
    FreeResource();
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_14)

__EXPORT ScTPValidationError::~ScTPValidationError()
{
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_15)

void ScTPValidationError::Init()
{
    aLbAction.SetSelectHdl( LINK( this, ScTPValidationError, SelectActionHdl ) );
    aBtnSearch.SetClickHdl( LINK( this, ScTPValidationError, ClickSearchHdl ) );

    aLbAction.SelectEntryPos( 0 );
    aTsbShow.EnableTriState( FALSE );

    SelectActionHdl( NULL );
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_16)

USHORT* __EXPORT ScTPValidationError::GetRanges()
{
    return pValueRanges;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_17)

SfxTabPage* __EXPORT ScTPValidationError::Create( Window*   pParent,
                                         const SfxItemSet&  rArgSet )
{
    return ( new ScTPValidationError( pParent, rArgSet ) );
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_18)

void __EXPORT ScTPValidationError::Reset( const SfxItemSet& rArgSet )
{
    const SfxPoolItem* pItem;

    if ( rArgSet.GetItemState( FID_VALID_SHOWERR, TRUE, &pItem ) == SFX_ITEM_SET )
        aTsbShow.SetState( ((const SfxBoolItem*)pItem)->GetValue() ? STATE_CHECK : STATE_NOCHECK );
    if ( rArgSet.GetItemState( FID_VALID_ERRSTYLE, TRUE, &pItem ) == SFX_ITEM_SET )
        aLbAction.SelectEntryPos( ((const SfxAllEnumItem*)pItem)->GetValue() );
    if ( rArgSet.GetItemState( FID_VALID_ERRTITLE, TRUE, &pItem ) == SFX_ITEM_SET )
        aEdtTitle.SetText( ((const SfxStringItem*)pItem)->GetValue() );
    if ( rArgSet.GetItemState( FID_VALID_ERRTEXT, TRUE, &pItem ) == SFX_ITEM_SET )
        aEdError.SetText( ((const SfxStringItem*)pItem)->GetValue() );

    SelectActionHdl( NULL );
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_19)

BOOL __EXPORT ScTPValidationError::FillItemSet( SfxItemSet& rArgSet )
{
    rArgSet.Put( SfxBoolItem( FID_VALID_SHOWERR, aTsbShow.GetState() == STATE_CHECK ) );
    rArgSet.Put( SfxAllEnumItem( FID_VALID_ERRSTYLE, aLbAction.GetSelectEntryPos() ) );
    rArgSet.Put( SfxStringItem( FID_VALID_ERRTITLE, aEdtTitle.GetText() ) );
    rArgSet.Put( SfxStringItem( FID_VALID_ERRTEXT, aEdError.GetText() ) );

    return TRUE;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_1a)

IMPL_LINK( ScTPValidationError, SelectActionHdl, ListBox *, pLb )
{
    ScValidErrorStyle eStyle = (ScValidErrorStyle) aLbAction.GetSelectEntryPos();
    BOOL bMacro = ( eStyle == SC_VALERR_MACRO );

    aBtnSearch.Enable( bMacro );
    aFtError.Enable( !bMacro );
    aEdError.Enable( !bMacro );

    return( 0L );
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(validate_1b)

IMPL_LINK( ScTPValidationError, ClickSearchHdl, PushButton*, pBtn )
{
    Window* pOld = Application::GetDefDialogParent();
    Application::SetDefDialogParent( this );
    SbMethod* pMethod = BasicIDE::ChooseMacro(FALSE, TRUE);
    Application::SetDefDialogParent( pOld );

    if (pMethod)
    {
        SbModule* pModule = pMethod->GetModule();
        SbxObject* pObject = pModule->GetParent();
        DBG_ASSERT(pObject->IsA(TYPE(StarBASIC)), "Kein Basic gefunden!");

        String aMacro = pMethod->GetName();

        //  zusammengesetzter Name aus Modul etc. erst, wenn er auch
        //  zum Ausfuehren wieder aufgeloest werden kann
#if 0
        aMacro += '.';
        aMacro += pModule->GetName();
        aMacro += '.';
        aMacro += pObject->GetName();
        aMacro += '.';
        if (pObject->GetParent())
        {
            // Dokumentenbasic
            aMacro += pObject->GetParent()->GetName();
        }
        else
        {
            // Applikationsbasic
            aMacro += SFX_APP()->GetName();
        }
        // aMacro liegt nun in folgender Form vor:
        // "Macroname.Modulname.Libname.Dokumentname" oder
        // "Macroname.Modulname.Libname.Applikationsname"
#endif

        aEdtTitle.SetText( aMacro );
    }

    return( 0L );
}


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.9  2000/09/17 14:08:59  willem.vandorp
    OpenOffice header added.

    Revision 1.8  2000/09/04 13:52:08  tbe
    basicide, isetbrw, si, vcdlged from svx to basctl

    Revision 1.7  2000/08/31 16:38:21  willem.vandorp
    Header and footer replaced

    Revision 1.6  2000/05/25 10:20:06  er
    NOOLDSV

    Revision 1.5  2000/02/11 12:24:23  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.4  1997/12/05 18:54:46  ANK
    Includes geaendert


      Rev 1.3   05 Dec 1997 19:54:46   ANK
   Includes geaendert

      Rev 1.2   16 Dec 1996 20:14:32   NN
   Leerzellen default an, ggf.disablen, SetDefModalDialogParent

      Rev 1.1   16 Dec 1996 16:57:30   NN
   Items rein/raus

      Rev 1.0   13 Dec 1996 09:35:24   SOH
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

