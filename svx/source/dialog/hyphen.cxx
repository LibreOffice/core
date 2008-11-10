/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: hyphen.cxx,v $
 * $Revision: 1.14 $
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
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------
#include <tools/shl.hxx>
#include <tools/list.hxx>
#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>
#include <vcl/msgbox.hxx>


#define _SVX_HYPHEN_CXX

#include <svx/dialogs.hrc>
#include "hyphen.hrc"

#include <svx/svxenum.hxx>
#include "hyphen.hxx"
#include <svx/splwrap.hxx>
#include "dlgutil.hxx"
#include <svx/dialmgr.hxx>
#include <unolingu.hxx>
#ifndef _USR_USTRING_HXX
//#include <usr/ustring.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

#undef S2U
#undef U2S
//#define S2U(s)                        StringToOUString(s, CHARSET_SYSTEM)
//#define U2S(s)                        OUStringToString(s, CHARSET_SYSTEM)

// define ----------------------------------------------------------------

#define HYPHHERE            '-'
#define CONTINUE_HYPH       USHRT_MAX

// Dies ist nur ein Hack, weil an dieser Stelle das extern char aus hmwrap
// nicht bekannt ist, wird demnaechst in hmwrap durch ein define ersetzt.
#define SW_SOFT_HYPHEN  '='

// class SvxHyphenEdit ---------------------------------------------------

SvxHyphenEdit::SvxHyphenEdit( Window* pParent, const ResId& rResId ) :

    Edit( pParent, rResId )
{
}

// -----------------------------------------------------------------------

void SvxHyphenEdit::KeyInput( const KeyEvent& rKEvt )
{
//  sal_uInt16 nMod  = rKEvt.GetKeyCode().GetModifier();
    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

    switch ( nCode )
    {
        case KEY_LEFT:
            ( (SvxHyphenWordDialog*)GetParent() )->SelLeft();
            break;

        case KEY_RIGHT:
            ( (SvxHyphenWordDialog*)GetParent() )->SelRight();
            break;

        case KEY_TAB:
        case KEY_ESCAPE:
        case KEY_RETURN:
            Edit::KeyInput(rKEvt);
            break;
        default:
            Control::KeyInput( rKEvt ); // An den Dialog weiterleiten
            break;
    }
}

// class SvxHyphenWordDialog ---------------------------------------------

SvxHyphenWordDialog::SvxHyphenWordDialog( const String &rWord, LanguageType nLang,
                                          Window* pParent,
                                          Reference< XHyphenator >  &xHyphen,
                                          SvxSpellWrapper* pWrapper ) :
    SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_HYPHENATE ) ),

    aWordFT     ( this, SVX_RES( FT_WORD ) ),
    aWordEdit   ( this, SVX_RES( ED_WORD ) ),
    aLeftBtn    ( this, SVX_RES( BTN_LEFT ) ),
    aRightBtn   ( this, SVX_RES( BTN_RIGHT ) ),
    aOkBtn      ( this, SVX_RES( BTN_HYPH_CUT ) ),
    aCancelBtn  ( this, SVX_RES( BTN_HYPH_CANCEL ) ),
    aContBtn    ( this, SVX_RES( BTN_HYPH_CONTINUE ) ),
    aDelBtn     ( this, SVX_RES( BTN_HYPH_DELETE ) ),
    aHelpBtn    ( this, SVX_RES( BTN_HYPH_HELP ) ),
    aLabel          ( GetText() ),
    pHyphWrapper    ( pWrapper ),
    xHyphenator     ( xHyphen ),
    aActWord        ( rWord ),
    nActLanguage    ( nLang ),
    nHyphPos        ( 0 ),
    nOldPos         ( 0 ),
    bBusy           ( sal_False )
{
    aContBtn.SetClickHdl(
        LINK( this, SvxHyphenWordDialog, ContinueHdl_Impl ) );
    aOkBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog, CutHdl_Impl ) );
    aDelBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog, DeleteHdl_Impl ) );
    aCancelBtn.SetClickHdl(
        LINK( this, SvxHyphenWordDialog, CancelHdl_Impl ) );
    aLeftBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog, Left_Impl ) );
    aRightBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog, Right_Impl ) );

    aWordEdit.SetGetFocusHdl(
        LINK( this, SvxHyphenWordDialog, GetFocusHdl_Impl ) );

    Reference< XHyphenatedWord >  xHyphWord( pWrapper ?
            pWrapper->GetLast() : Reference< XInterface > () , UNO_QUERY );
    DBG_ASSERT(xHyphWord.is(), "missing hyphenated word");
    nMaxHyphenationPos = xHyphWord.is() ? xHyphWord->getHyphenationPos() : 0;
    SetLabel_Impl( nLang );

    InitControls_Impl();
    aWordEdit.GrabFocus();

    FreeResource();

    // disable controls if service is not available
    if (!xHyphenator.is())
        Enable( sal_False );
}

// -----------------------------------------------------------------------

void SvxHyphenWordDialog::SelLeft()
{
    String aTxt( aWordEdit.GetText() );

    for ( xub_StrLen i = nOldPos + 1;  i-- > 0 ; )
    {
        DBG_ASSERT(i <= aTxt.Len(), "index out of range");
        if( aTxt.GetChar( i ) == sal_Unicode( SW_SOFT_HYPHEN ) )
        {
            aTxt.SetChar( i, sal_Unicode( HYPHHERE ) );

            if ( nOldPos != 0 && nOldPos != aTxt.Len() )
                aTxt.SetChar( nOldPos, sal_Unicode( SW_SOFT_HYPHEN ) );
            nOldPos = i;
            aWordEdit.SetText( aTxt );
            aWordEdit.GrabFocus();
            aWordEdit.SetSelection( Selection( i, i + 1 ) );
            break;
        }
    }
    nHyphPos = GetHyphIndex_Impl();
    EnableLRBtn_Impl();
}

// -----------------------------------------------------------------------

void SvxHyphenWordDialog::SelRight()
{
    String aTxt( aWordEdit.GetText() );

    for ( xub_StrLen i = nOldPos + 1;  i < aTxt.Len();  ++i )
    {
        if( aTxt.GetChar( i ) == sal_Unicode( SW_SOFT_HYPHEN ) )
        {
            aTxt.SetChar( i, sal_Unicode( HYPHHERE ) );

            if ( nOldPos != 0 && nOldPos != aTxt.Len() )
                aTxt.SetChar( nOldPos, sal_Unicode( SW_SOFT_HYPHEN ) );
            nOldPos = i;
            aWordEdit.SetText( aTxt );
            aWordEdit.GrabFocus();
            aWordEdit.SetSelection( Selection( i, i + 1 ) );
            break;
        }
    }
    nHyphPos = GetHyphIndex_Impl();
    EnableLRBtn_Impl();
}

// -----------------------------------------------------------------------

void SvxHyphenWordDialog::EnableLRBtn_Impl()
{
    String  aTxt( aWordEdit.GetText() );
    xub_StrLen nLen = aTxt.Len();
    xub_StrLen i;

    aRightBtn.Disable();
    for ( i = nOldPos + 2; i < nLen; ++i )
    {
        if ( aTxt.GetChar( i ) == sal_Unicode( SW_SOFT_HYPHEN ) )
        {
            aRightBtn.Enable();
            break;
        }
    }

    DBG_ASSERT(nOldPos < aTxt.Len(), "nOldPos out of range");
    if (nOldPos >= aTxt.Len())
        nOldPos = aTxt.Len() - 1;
    aLeftBtn.Disable();
    for ( i = nOldPos;  i-- > 0; )
    {
        if ( aTxt.GetChar( i ) == sal_Unicode( SW_SOFT_HYPHEN ) )
        {
            aLeftBtn.Enable();
            break;
        }
    }
}

// -----------------------------------------------------------------------


void SvxHyphenWordDialog::SetLabel_Impl( LanguageType nLang )
{
    String aLangStr( ::GetLanguageString( nLang ) );
    String aTmp( aLabel );
    aTmp.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " (" ) );
    aTmp.Append( aLangStr );
    aTmp.Append( sal_Unicode( ')' ) );
    SetText( aTmp );
}

// -----------------------------------------------------------------------

String SvxHyphenWordDialog::EraseUnusableHyphens_Impl(
        Reference< XPossibleHyphens >  &rxPossHyph,
        sal_uInt16 _nMaxHyphenationPos )
{
    // returns a String showing only those hyphen positions which will result
    // in a line break if hyphenation is done there

    String aTxt;
    DBG_ASSERT(rxPossHyph.is(), "missing possible hyphens");
    if (rxPossHyph.is())
    {
        aTxt = String( rxPossHyph->getPossibleHyphens() );

        Sequence< sal_Int16 > aHyphenationPositions(
                rxPossHyph->getHyphenationPositions() );
        sal_Int32 nLen = aHyphenationPositions.getLength();
        const sal_Int16 *pHyphenationPos = aHyphenationPositions.getConstArray();

        // find position nIdx after which all hyphen positions are unusable
        xub_StrLen nIdx = STRING_NOTFOUND,
              nPos;
        if (nLen)
        {
            xub_StrLen nStart = 0;
            for (sal_Int32 i = 0;  i < nLen;  ++i)
            {
                if (pHyphenationPos[i] > _nMaxHyphenationPos)
                    break;
                else
                {
                    // find corresponding hyphen pos in string
                    nPos = aTxt.Search( sal_Unicode( SW_SOFT_HYPHEN ), nStart );

                    if (nStart == STRING_NOTFOUND)
                        break;
                    else
                    {
                        nIdx = nPos;
                        nStart = nPos + 1;
                    }
                }
            }
        }
        DBG_ASSERT(nIdx != STRING_NOTFOUND, "no usable hyphenation position");

        // remove not usable hyphens from string
        nPos = nIdx == STRING_NOTFOUND ? 0 : nIdx + 1;
        String aTmp( sal_Unicode( SW_SOFT_HYPHEN ) ),
               aEmpty;
        while (nPos != STRING_NOTFOUND)
            nPos = aTxt.SearchAndReplace( aTmp, aEmpty, nPos + 1 );
    }
    return aTxt;
}

// -----------------------------------------------------------------------

void SvxHyphenWordDialog::InitControls_Impl()
{
    String aTxt;
    xPossHyph = NULL;
    if (xHyphenator.is())
    {
        lang::Locale aLocale( SvxCreateLocale(nActLanguage) );
        xPossHyph = xHyphenator->createPossibleHyphens( aActWord, aLocale,
                                                        Sequence< PropertyValue >() );
        if (xPossHyph.is())
        {
            aTxt = EraseUnusableHyphens_Impl( xPossHyph, nMaxHyphenationPos );
        }
        SetLabel_Impl( nActLanguage );
    }
    aWordEdit.SetText( aTxt );

    nOldPos = aTxt.Len();
    SelLeft();
    EnableLRBtn_Impl();
}

// -----------------------------------------------------------------------

void SvxHyphenWordDialog::ContinueHyph_Impl( sal_uInt16 nInsPos )
{
    if ( nInsPos != CONTINUE_HYPH  &&  xPossHyph.is())
    {
        if (nInsPos)
        {
            //String aTmp( U2S( xPossHyph->getPossibleHyphens() ) );
            String aTmp( aWordEdit.GetText() );
            DBG_ASSERT(nInsPos <= aTmp.Len() - 2, "wrong hyphen position");

            sal_uInt16 nIdxPos = 0;
            sal_uInt16 i = 0;
            sal_Unicode  c;
            while (i < aTmp.Len()  &&  HYPHHERE != (c = aTmp.GetChar(i++)) )
            {
                if (SW_SOFT_HYPHEN == c)
                    nIdxPos++;
            }

            Sequence< sal_Int16 > aSeq = xPossHyph->getHyphenationPositions();
            sal_Int32 nLen = aSeq.getLength();
            DBG_ASSERT(nLen, "empty sequence");
            DBG_ASSERT(nIdxPos < nLen, "index out of range");
            if (nLen && nIdxPos < nLen)
            {
                nInsPos = aSeq.getConstArray()[ nIdxPos ];
                pHyphWrapper->InsertHyphen( nInsPos );
            }
        }
        else
        {
            //! calling with 0 as argument will remove hyphens!
            pHyphWrapper->InsertHyphen( nInsPos );
        }
    }

    if ( pHyphWrapper->FindSpellError() )
    {
        Reference< XHyphenatedWord >  xHyphWord( pHyphWrapper->GetLast(), UNO_QUERY );

        // adapt actual word and language to new found hyphenation result
        if(xHyphWord.is())
        {
            aActWord     = String( xHyphWord->getWord() );
            nActLanguage = SvxLocaleToLanguage( xHyphWord->getLocale() );
            nMaxHyphenationPos = xHyphWord->getHyphenationPos();
            InitControls_Impl();
        }
    }
    else
        EndDialog( RET_OK );
}

// -----------------------------------------------------------------------

sal_uInt16 SvxHyphenWordDialog::GetHyphIndex_Impl()
{
    sal_uInt16 nPos = 0;
    String aTxt(aWordEdit.GetText());

    for ( sal_uInt16 i=0 ; i < aTxt.Len(); ++i )
    {
        sal_Unicode nChar = aTxt.GetChar( i );

        if ( nChar == HYPHHERE )
            break;

        if ( nChar != SW_SOFT_HYPHEN )
            nPos++;
    }
    return nPos;
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxHyphenWordDialog, CutHdl_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        ContinueHyph_Impl( nHyphPos );
        bBusy = sal_False;
    }
    return 0;
}
IMPL_LINK_INLINE_END( SvxHyphenWordDialog, CutHdl_Impl, Button *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxHyphenWordDialog, DeleteHdl_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        ContinueHyph_Impl();
        bBusy = sal_False;
    }
    return 0;
}
IMPL_LINK_INLINE_END( SvxHyphenWordDialog, DeleteHdl_Impl, Button *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxHyphenWordDialog, ContinueHdl_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        ContinueHyph_Impl( CONTINUE_HYPH );
        bBusy = sal_False;
    }
    return 0;
}
IMPL_LINK_INLINE_END( SvxHyphenWordDialog, ContinueHdl_Impl, Button *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxHyphenWordDialog, CancelHdl_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        pHyphWrapper->SpellEnd();
        EndDialog( RET_CANCEL );
        bBusy = sal_False;
    }
    return 0;
}
IMPL_LINK_INLINE_END( SvxHyphenWordDialog, CancelHdl_Impl, Button *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxHyphenWordDialog, Left_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        SelLeft();
        bBusy = sal_False;
    }
    return 0;
}
IMPL_LINK_INLINE_END( SvxHyphenWordDialog, Left_Impl, Button *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxHyphenWordDialog, Right_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        SelRight();
        bBusy = sal_False;
    }
    return 0;
}
IMPL_LINK_INLINE_END( SvxHyphenWordDialog, Right_Impl, Button *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxHyphenWordDialog, GetFocusHdl_Impl, Edit *, EMPTYARG )
{
    aWordEdit.SetSelection( Selection( nOldPos, nOldPos + 1 ) );
    return 0;
}
IMPL_LINK_INLINE_END( SvxHyphenWordDialog, GetFocusHdl_Impl, Edit *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK( SvxHyphenWordDialog, LangError_Impl, void *, nLang )
{
    // Status anzeigen
    String aErr( ::GetLanguageString( (LanguageType)(sal_IntPtr)nLang ) );
    aErr += SVX_RESSTR( RID_SVXSTR_HMERR_CHECKINSTALL );
    InfoBox( this, aErr ).Execute();
    return 0;
}


