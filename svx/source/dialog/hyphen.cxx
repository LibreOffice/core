/*************************************************************************
 *
 *  $RCSfile: hyphen.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tl $ $Date: 2000-10-27 09:54:20 $
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

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif

#ifndef _COM_SUN_STAR_LINGUISTIC2_XPOSSIBLEHYPHENS_HPP_
#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>
#endif

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#pragma hdrstop

#define _SVX_HYPHEN_CXX

#include "dialogs.hrc"
#include "hyphen.hrc"

#include "svxenum.hxx"
#include "hyphen.hxx"
#include "splwrap.hxx"
#include "dlgutil.hxx"
#include "dialmgr.hxx"
#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _USR_USTRING_HXX
//#include <usr/ustring.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

#define S2U(s)                      StringToOUString(s, CHARSET_SYSTEM)
#define U2S(s)                      OUStringToString(s, CHARSET_SYSTEM)

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
    sal_uInt16 nMod  = rKEvt.GetKeyCode().GetModifier();
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
#ifdef MAC
        case KEY_POINT:
            // Command . abfangen
            if ( nMod == KEY_MOD1 )
            {
                Edit::KeyInput( rKEvt );
                break;
            }
            // kein break!
#endif
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

    aWordEdit   ( this, ResId( ED_WORD ) ),
    aLeftBtn    ( this, ResId( BTN_LEFT ) ),
    aRightBtn   ( this, ResId( BTN_RIGHT ) ),
    aWordBox    ( this, ResId( GB_WORD ) ),
    aContBtn    ( this, ResId( BTN_HYPH_CONTINUE ) ),
    aDelBtn     ( this, ResId( BTN_HYPH_DELETE ) ),
    aOkBtn      ( this, ResId( BTN_HYPH_CUT ) ),
    aCancelBtn  ( this, ResId( BTN_HYPH_CANCEL ) ),
    aHelpBtn    ( this, ResId( BTN_HYPH_HELP ) ),

    pHyphWrapper    ( pWrapper ),
    xHyphenator     ( xHyphen ),
    aActWord        ( rWord ),
    nActLanguage    ( nLang ),
    aLabel          ( GetText() ),
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
        DBG_ASSERT(0 <= i && i <= aTxt.Len(), "index out of range");
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
        sal_uInt16 nMaxHyphenationPos )
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
        sal_Int32 nIdx = -1,
              nPos;
        if (nLen)
        {
            sal_Int32 nStart = 0;
            for (sal_Int32 i = 0;  i < nLen;  ++i)
            {
                if (pHyphenationPos[i] > nMaxHyphenationPos)
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
        DBG_ASSERT(nIdx != -1, "no usable hyphenation position")

        // remove not usable hyphens from string
        nPos = nIdx == -1 ? 0 : nIdx + 1;
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
    String aErr( ::GetLanguageString( (LanguageType)(sal_uInt32)nLang ) );
    aErr += SVX_RESSTR( RID_SVXSTR_HMERR_CHECKINSTALL );
    InfoBox( this, aErr ).Execute();
    return 0;
}


