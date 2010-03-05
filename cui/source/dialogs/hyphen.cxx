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
#include "precompiled_cui.hxx"

#include "hyphen.hxx"
#include "hyphen.hrc"
#include "cuires.hrc"
#include "dialmgr.hxx"

#include <editeng/splwrap.hxx>
#include <editeng/svxenum.hxx>
#include <editeng/unolingu.hxx>
#include <svtools/langtab.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <tools/list.hxx>
#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>

#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>

using namespace ::com::sun::star;


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

// struct SvxHyphenWordDialog_Impl ---------------------------------------------

struct SvxHyphenWordDialog_Impl
{
    SvxHyphenWordDialog *       m_pDialog;
//    Window *                    m_pParent;

    FixedText           aWordFT;
    SvxHyphenEdit       aWordEdit;
    ImageButton         aLeftBtn;
    ImageButton         aRightBtn;
    OKButton            aOkBtn;
    PushButton          aContBtn;
    PushButton          aDelBtn;
    FixedLine           aFLBottom;
    HelpButton          aHelpBtn;
    PushButton          aHyphAll;
    CancelButton        aCancelBtn;
    String              aLabel;
    SvxSpellWrapper*    pHyphWrapper;
    uno::Reference< linguistic2::XHyphenator >        xHyphenator;
    uno::Reference< linguistic2::XPossibleHyphens >   xPossHyph;
    String              aActWord;       // actual (to be displayed) word
    LanguageType        nActLanguage;   // and language
    sal_uInt16          nMaxHyphenationPos; // right most valid hyphenation pos
    sal_uInt16          nHyphPos;
    sal_uInt16          nOldPos;
    sal_Bool            bBusy;


    void            EnableLRBtn_Impl();
    String          EraseUnusableHyphens_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XPossibleHyphens >  &rxPossHyph, sal_uInt16 nMaxHyphenationPos );

    void            InitControls_Impl();
    void            ContinueHyph_Impl( sal_uInt16 nInsPos = 0 );
    sal_uInt16      GetHyphIndex_Impl();

    DECL_LINK( Left_Impl, Button* );
    DECL_LINK( Right_Impl, Button* );
    DECL_LINK( CutHdl_Impl, Button* );
    DECL_LINK( ContinueHdl_Impl, Button* );
    DECL_LINK( DeleteHdl_Impl, Button* );
    DECL_LINK( HyphenateAllHdl_Impl, Button* );
    DECL_LINK( CancelHdl_Impl, Button* );
    DECL_LINK( GetFocusHdl_Impl, Edit* );


    SvxHyphenWordDialog_Impl(
            SvxHyphenWordDialog * pDialog,
            const String &rWord,
            LanguageType nLang,
            uno::Reference< linguistic2::XHyphenator >  &xHyphen,
            SvxSpellWrapper* pWrapper );
    ~SvxHyphenWordDialog_Impl();
};


SvxHyphenWordDialog_Impl::SvxHyphenWordDialog_Impl(
        SvxHyphenWordDialog * pDialog,
        const String &rWord,
        LanguageType nLang,
        uno::Reference< linguistic2::XHyphenator >  &xHyphen,
        SvxSpellWrapper* pWrapper ) :

    m_pDialog   ( pDialog ),
    aWordFT     ( pDialog, CUI_RES( FT_WORD ) ),
    aWordEdit   ( pDialog, CUI_RES( ED_WORD ) ),
    aLeftBtn    ( pDialog, CUI_RES( BTN_LEFT ) ),
    aRightBtn   ( pDialog, CUI_RES( BTN_RIGHT ) ),
    aOkBtn      ( pDialog, CUI_RES( BTN_HYPH_CUT ) ),
    aContBtn    ( pDialog, CUI_RES( BTN_HYPH_CONTINUE ) ),
    aDelBtn     ( pDialog, CUI_RES( BTN_HYPH_DELETE ) ),
    aFLBottom   ( pDialog, CUI_RES( FL_BOTTOM ) ),
    aHelpBtn    ( pDialog, CUI_RES( BTN_HYPH_HELP ) ),
    aHyphAll    ( pDialog, CUI_RES( BTN_HYPH_ALL ) ),
    aCancelBtn  ( pDialog, CUI_RES( BTN_HYPH_CANCEL ) ),
    aLabel          ( pDialog->GetText() ),
    pHyphWrapper    ( NULL ),
    xHyphenator     ( NULL ),
    xPossHyph       ( NULL ),
    aActWord        (  ),
    nActLanguage    ( LANGUAGE_NONE ),
    nMaxHyphenationPos  ( 0 ),
    nHyphPos        ( 0 ),
    nOldPos         ( 0 ),
    bBusy           ( sal_False )
{
    aActWord       = rWord;
    nActLanguage   = nLang;
    xHyphenator    = xHyphen;
    pHyphWrapper   = pWrapper;

    aLeftBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog_Impl, Left_Impl ) );
    aRightBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog_Impl, Right_Impl ) );
    aOkBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog_Impl, CutHdl_Impl ) );
    aContBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog_Impl, ContinueHdl_Impl ) );
    aDelBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog_Impl, DeleteHdl_Impl ) );
    aHyphAll.SetClickHdl( LINK( this, SvxHyphenWordDialog_Impl, HyphenateAllHdl_Impl ) );
    aCancelBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog_Impl, CancelHdl_Impl ) );
    aWordEdit.SetGetFocusHdl( LINK( this, SvxHyphenWordDialog_Impl, GetFocusHdl_Impl ) );
}


SvxHyphenWordDialog_Impl::~SvxHyphenWordDialog_Impl()
{
}


void SvxHyphenWordDialog_Impl::EnableLRBtn_Impl()
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


String SvxHyphenWordDialog_Impl::EraseUnusableHyphens_Impl(
        uno::Reference< linguistic2::XPossibleHyphens >  &rxPossHyph,
        sal_uInt16 _nMaxHyphenationPos )
{
    // returns a String showing only those hyphen positions which will result
    // in a line break if hyphenation is done there

    String aTxt;
    DBG_ASSERT(rxPossHyph.is(), "missing possible hyphens");
    if (rxPossHyph.is())
    {
        aTxt = String( rxPossHyph->getPossibleHyphens() );

        uno::Sequence< sal_Int16 > aHyphenationPositions(
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


void SvxHyphenWordDialog_Impl::InitControls_Impl()
{
    String aTxt;
    xPossHyph = NULL;
    if (xHyphenator.is())
    {
        lang::Locale aLocale( SvxCreateLocale(nActLanguage) );
        xPossHyph = xHyphenator->createPossibleHyphens( aActWord, aLocale,
                                                        uno::Sequence< beans::PropertyValue >() );
        if (xPossHyph.is())
        {
            aTxt = EraseUnusableHyphens_Impl( xPossHyph, nMaxHyphenationPos );
        }
        m_pDialog->SetWindowTitle( nActLanguage );
    }
    aWordEdit.SetText( aTxt );

    nOldPos = aTxt.Len();
    m_pDialog->SelLeft();
    EnableLRBtn_Impl();
}


void SvxHyphenWordDialog_Impl::ContinueHyph_Impl( sal_uInt16 nInsPos )
{
    if ( nInsPos != CONTINUE_HYPH  &&  xPossHyph.is())
    {
        if (nInsPos)
        {
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

            uno::Sequence< sal_Int16 > aSeq = xPossHyph->getHyphenationPositions();
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
        uno::Reference< linguistic2::XHyphenatedWord >  xHyphWord( pHyphWrapper->GetLast(), uno::UNO_QUERY );

        // adapt actual word and language to new found hyphenation result
        if(xHyphWord.is())
        {
            aActWord    = String( xHyphWord->getWord() );
            nActLanguage = SvxLocaleToLanguage( xHyphWord->getLocale() );
            nMaxHyphenationPos = xHyphWord->getHyphenationPos();
            InitControls_Impl();
        }
    }
    else
        m_pDialog->EndDialog( RET_OK );
}


sal_uInt16 SvxHyphenWordDialog_Impl::GetHyphIndex_Impl()
{
    sal_uInt16 nPos = 0;
    String aTxt( aWordEdit.GetText() );

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


IMPL_LINK( SvxHyphenWordDialog_Impl, CutHdl_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        ContinueHyph_Impl( nHyphPos );
        bBusy = sal_False;
    }
    return 0;
}


IMPL_LINK( SvxHyphenWordDialog_Impl, HyphenateAllHdl_Impl, Button *, EMPTYARG /*pButton*/ )
{
    if( !bBusy )
    {
        try
        {
            uno::Reference< beans::XPropertySet >  xProp( SvxGetLinguPropertySet() );
            const rtl::OUString aName( rtl::OUString::createFromAscii( "IsHyphAuto" ) );
            uno::Any aAny;

            aAny <<= sal_True;
            xProp->setPropertyValue( aName, aAny );

            bBusy = sal_True;
            ContinueHyph_Impl( nHyphPos );
            bBusy = sal_False;

            aAny <<= sal_False;
            xProp->setPropertyValue( aName, aAny );
        }
        catch (uno::Exception &e)
        {
            (void) e;
            DBG_ASSERT( 0, "Hyphenate All failed" );
        }
    }
    return 0;
}


IMPL_LINK( SvxHyphenWordDialog_Impl, DeleteHdl_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        ContinueHyph_Impl();
        bBusy = sal_False;
    }
    return 0;
}


IMPL_LINK( SvxHyphenWordDialog_Impl, ContinueHdl_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        ContinueHyph_Impl( CONTINUE_HYPH );
        bBusy = sal_False;
    }
    return 0;
}


IMPL_LINK( SvxHyphenWordDialog_Impl, CancelHdl_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        pHyphWrapper->SpellEnd();
        m_pDialog->EndDialog( RET_CANCEL );
        bBusy = sal_False;
    }
    return 0;
}


IMPL_LINK( SvxHyphenWordDialog_Impl, Left_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        m_pDialog->SelLeft();
        bBusy = sal_False;
    }
    return 0;
}


IMPL_LINK( SvxHyphenWordDialog_Impl, Right_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        m_pDialog->SelRight();
        bBusy = sal_False;
    }
    return 0;
}


IMPL_LINK( SvxHyphenWordDialog_Impl, GetFocusHdl_Impl, Edit *, EMPTYARG )
{
    aWordEdit.SetSelection( Selection( nOldPos, nOldPos + 1 ) );
    return 0;
}


// class SvxHyphenWordDialog ---------------------------------------------

SvxHyphenWordDialog::SvxHyphenWordDialog(
    const String &rWord, LanguageType nLang,
    Window* pParent,
    uno::Reference< linguistic2::XHyphenator >  &xHyphen,
    SvxSpellWrapper* pWrapper ) :

    SfxModalDialog( pParent, CUI_RES( RID_SVXDLG_HYPHENATE ) )
{
    m_pImpl = std::auto_ptr< SvxHyphenWordDialog_Impl >(
            new SvxHyphenWordDialog_Impl( this, rWord, nLang, xHyphen, pWrapper ) );

    FreeResource();

    uno::Reference< linguistic2::XHyphenatedWord >  xHyphWord( pWrapper ?
            pWrapper->GetLast() : uno::Reference< uno::XInterface > () , uno::UNO_QUERY );
    DBG_ASSERT( xHyphWord.is(), "missing hyphenated word" );
    m_pImpl->nMaxHyphenationPos = xHyphWord.is() ? xHyphWord->getHyphenationPos() : 0;

    m_pImpl->InitControls_Impl();
    m_pImpl->aWordEdit.GrabFocus();

    SetWindowTitle( nLang );

    // disable controls if service is not available
    if (!m_pImpl->xHyphenator.is())
        Enable( sal_False );
}


SvxHyphenWordDialog::~SvxHyphenWordDialog()
{
}


void SvxHyphenWordDialog::SetWindowTitle( LanguageType nLang )
{
    String aLangStr( SvtLanguageTable::GetLanguageString( nLang ) );
    String aTmp( m_pImpl->aLabel );
    aTmp.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " (" ) );
    aTmp.Append( aLangStr );
    aTmp.Append( sal_Unicode( ')' ) );
    SetText( aTmp );
}


void SvxHyphenWordDialog::SelLeft()
{
    String aTxt( m_pImpl->aWordEdit.GetText() );
    for ( xub_StrLen i = m_pImpl->nOldPos + 1;  i-- > 0 ; )
    {
        DBG_ASSERT(i <= aTxt.Len(), "index out of range");
        if( aTxt.GetChar( i ) == sal_Unicode( SW_SOFT_HYPHEN ) )
        {
            aTxt.SetChar( i, sal_Unicode( HYPHHERE ) );

            if ( m_pImpl->nOldPos != 0 && m_pImpl->nOldPos != aTxt.Len() )
                aTxt.SetChar( m_pImpl->nOldPos, sal_Unicode( SW_SOFT_HYPHEN ) );
            m_pImpl->nOldPos = i;
            m_pImpl->aWordEdit.SetText( aTxt );
            m_pImpl->aWordEdit.GrabFocus();
            m_pImpl->aWordEdit.SetSelection( Selection( i, i + 1 ) );
            break;
        }
    }
    m_pImpl->nHyphPos = m_pImpl->GetHyphIndex_Impl();
    m_pImpl->EnableLRBtn_Impl();
}


void SvxHyphenWordDialog::SelRight()
{
    String aTxt( m_pImpl->aWordEdit.GetText() );
    for ( xub_StrLen i = m_pImpl->nOldPos + 1;  i < aTxt.Len();  ++i )
    {
        if( aTxt.GetChar( i ) == sal_Unicode( SW_SOFT_HYPHEN ) )
        {
            aTxt.SetChar( i, sal_Unicode( HYPHHERE ) );

            if ( m_pImpl->nOldPos != 0 && m_pImpl->nOldPos != aTxt.Len() )
                aTxt.SetChar( m_pImpl->nOldPos, sal_Unicode( SW_SOFT_HYPHEN ) );
            m_pImpl->nOldPos = i;
            m_pImpl->aWordEdit.SetText( aTxt );
            m_pImpl->aWordEdit.GrabFocus();
            m_pImpl->aWordEdit.SetSelection( Selection( i, i + 1 ) );
            break;
        }
    }
    m_pImpl->nHyphPos = m_pImpl->GetHyphIndex_Impl();
    m_pImpl->EnableLRBtn_Impl();
}


