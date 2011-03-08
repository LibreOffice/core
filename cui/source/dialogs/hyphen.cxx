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
#include <tools/shl.hxx>
#include <vcl/msgbox.hxx>

#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>

using namespace ::com::sun::star;


#define HYPH_POS_CHAR       '='
#define CONTINUE_HYPH       USHRT_MAX

#define CUR_HYPH_POS_CHAR   '-'


// class HyphenEdit_Impl -------------------------------------------------------

class HyphenEdit_Impl : public Edit
{
public:
    HyphenEdit_Impl( Window* pParent, const ResId& rResId );

protected:
    virtual void    KeyInput( const KeyEvent &rKEvt );
};


HyphenEdit_Impl::HyphenEdit_Impl( Window* pParent, const ResId& rResId ) :
    Edit( pParent, rResId )
{
}


void HyphenEdit_Impl::KeyInput( const KeyEvent& rKEvt )
{
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

    FixedText           aWordFT;
    HyphenEdit_Impl     aWordEdit;
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
    String              aEditWord;      // aEditWord and aWordEdit.GetText() differ only by the character for the current selected hyphenation position
    String              aActWord;           // actual word to be hyphenated
    LanguageType        nActLanguage;       // and its language
    sal_uInt16          nMaxHyphenationPos; // right most valid hyphenation pos
    sal_uInt16          nHyphPos;
    sal_uInt16          nOldPos;
    sal_Int32           nHyphenationPositionsOffset;
    sal_Bool            bBusy;


    void            EnableLRBtn_Impl();
    String          EraseUnusableHyphens_Impl( ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XPossibleHyphens >  &rxPossHyph, sal_uInt16 nMaxHyphenationPos );

    void            InitControls_Impl();
    void            ContinueHyph_Impl( sal_uInt16 nInsPos = 0 );
    sal_uInt16      GetHyphIndex_Impl();
    void            SelLeft_Impl();
    void            SelRight_Impl();

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
    nHyphenationPositionsOffset( 0 ),
    bBusy           ( sal_False )
{
    aActWord       = rWord;
    nActLanguage   = nLang;
    xHyphenator    = xHyphen;
    pHyphWrapper   = pWrapper;

    uno::Reference< linguistic2::XHyphenatedWord >  xHyphWord( pHyphWrapper ?
            pHyphWrapper->GetLast() : NULL, uno::UNO_QUERY );
    DBG_ASSERT( xHyphWord.is(), "hyphenation result missing" );
    if (xHyphWord.is())
    {
        DBG_ASSERT( aActWord == String( xHyphWord->getWord() ), "word mismatch" );
        DBG_ASSERT( nActLanguage == SvxLocaleToLanguage( xHyphWord->getLocale() ), "language mismatch" );
        nMaxHyphenationPos = xHyphWord->getHyphenationPos();
    }

    InitControls_Impl();
    aWordEdit.GrabFocus();

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
    String  aTxt( aEditWord );
    xub_StrLen nLen = aTxt.Len();
    xub_StrLen i;

    aRightBtn.Disable();
    for ( i = nOldPos + 2; i < nLen; ++i )
    {
        if ( aTxt.GetChar( i ) == sal_Unicode( HYPH_POS_CHAR ) )
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
        if ( aTxt.GetChar( i ) == sal_Unicode( HYPH_POS_CHAR ) )
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
    // 1) we will need to discard all hyphenation positions at th end that
    // will not result in a line break where the text to the left still fits
    // on the line.
    // 2) since as from OOo 3.2 '-' are part of a word an thus text like
    // 'multi-line-editor' is regarded as single word we also need to discard those
    // hyphenation positions to the left of the rightmost '-' that is still left of
    // the rightmost valid hyphenation position according to 1)
    //
    // Example:
    // If the possible hyphenation position in 'multi-line-editor' are to eb marked
    // by '=' then the text will look like this 'mul=ti-line-ed=it=or'.
    // If now the first line is only large enough for 'multi-line-edi' we need to discard
    // the last possible hyphnation point because of 1). The the right most valid
    // hyphenation position is "ed=itor". The first '-' left of this position is
    // "line-ed", thus because of 2) we now need to discard all possible hyphneation
    // positions to the left of that as well. Thus in the end leaving us with just
    // 'multi-line-ed=itor' as return value for this function. (Just one valid hyphenation
    // position for the user too choose from. However ALL the '-' characters in the word
    // will ALWAYS be valid implicit hyphenation positions for the core to choose from!
    // And thus even if this word is skipped in the hyphenation dialog it will still be broken
    // right after 'multi-line-' (actually it might already be broken up that way before
    // the hyphenation dialog is called!).
    // Thus rule 2) just eliminates those positions which will not be used by the core at all
    // even if the user were to select one of them.

    String aTxt;
    DBG_ASSERT(rxPossHyph.is(), "missing possible hyphens");
    if (rxPossHyph.is())
    {
        DBG_ASSERT( aActWord == String( rxPossHyph->getWord() ), "word mismatch"  );

        aTxt = String( rxPossHyph->getPossibleHyphens() );

        nHyphenationPositionsOffset = 0;
        uno::Sequence< sal_Int16 > aHyphenationPositions(
                rxPossHyph->getHyphenationPositions() );
        sal_Int32 nLen = aHyphenationPositions.getLength();
        const sal_Int16 *pHyphenationPos = aHyphenationPositions.getConstArray();

        // find position nIdx after which all hyphen positions are unusable
        xub_StrLen  nIdx = STRING_NOTFOUND;
        xub_StrLen  nPos = 0, nPos1 = 0, nPos2 = 0;
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
                    nPos = aTxt.Search( sal_Unicode( HYPH_POS_CHAR ), nStart );

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

        // 1) remove all not usable hyphenation positions from the end of the string
        nPos = nIdx == STRING_NOTFOUND ? 0 : nIdx + 1;
        nPos1 = nPos;   //save for later use in 2) below
        const String aTmp( sal_Unicode( HYPH_POS_CHAR ) );
        const String aEmpty;
        while (nPos != STRING_NOTFOUND)
            nPos = aTxt.SearchAndReplace( aTmp, aEmpty, nPos + 1 );

        // 2) remove all hyphenation positions from the start that are not considered by the core
        const String aSearchRange( aTxt.Copy( 0, nPos1 ) );
        nPos2 = aSearchRange.SearchBackward( '-' );  // the '-' position the core will use by default
        if (nPos2 != STRING_NOTFOUND)
        {
            String aLeft( aSearchRange.Copy( 0, nPos2 ) );
            nPos = 0;
            while (nPos != STRING_NOTFOUND)
            {
                nPos = aLeft.SearchAndReplace( aTmp, aEmpty, nPos + 1 );
                if (nPos != STRING_NOTFOUND)
                    ++nHyphenationPositionsOffset;
            }
            aTxt.Replace( 0, nPos2, aLeft );
        }
    }
    return aTxt;
}


void SvxHyphenWordDialog_Impl::InitControls_Impl()
{
    xPossHyph = NULL;
    if (xHyphenator.is())
    {
        lang::Locale aLocale( SvxCreateLocale(nActLanguage) );
        xPossHyph = xHyphenator->createPossibleHyphens( aActWord, aLocale,
                                                        uno::Sequence< beans::PropertyValue >() );
        if (xPossHyph.is())
            aEditWord = EraseUnusableHyphens_Impl( xPossHyph, nMaxHyphenationPos );
    }
    aWordEdit.SetText( aEditWord );

    nOldPos = aEditWord.Len();
    SelLeft_Impl();
    EnableLRBtn_Impl();
}


void SvxHyphenWordDialog_Impl::ContinueHyph_Impl( sal_uInt16 nInsPos )
{
    if ( nInsPos != CONTINUE_HYPH  &&  xPossHyph.is())
    {
        if (nInsPos)
        {
            String aTmp( aEditWord );
            DBG_ASSERT(nInsPos <= aTmp.Len() - 2, "wrong hyphen position");

            sal_Int16 nIdxPos = -1;
            for (sal_uInt16 i = 0; i <= nInsPos; ++i)
            {
                if (HYPH_POS_CHAR == aTmp.GetChar( i ))
                    nIdxPos++;
            }
            // take the possible hyphenation positions that got removed from the
            // start of the wor dinot account:
            nIdxPos += nHyphenationPositionsOffset;

            uno::Sequence< sal_Int16 > aSeq = xPossHyph->getHyphenationPositions();
            sal_Int32 nLen = aSeq.getLength();
            DBG_ASSERT(nLen, "empty sequence");
            DBG_ASSERT(0 <= nIdxPos && nIdxPos < nLen, "index out of range");
            if (nLen && 0 <= nIdxPos && nIdxPos < nLen)
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
            m_pDialog->SetWindowTitle( nActLanguage );
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
        sal_Unicode cChar = aTxt.GetChar( i );
        if ( cChar == CUR_HYPH_POS_CHAR )
            break;
        if ( cChar != HYPH_POS_CHAR )
            nPos++;
    }
    return nPos;
}


void SvxHyphenWordDialog_Impl::SelLeft_Impl()
{
    DBG_ASSERT( nOldPos > 0, "invalid hyphenation position" );
    if (nOldPos > 0)
    {
        String aTxt( aEditWord );
        for ( xub_StrLen i = nOldPos - 1;  i > 0; --i)
        {
            DBG_ASSERT(i <= aTxt.Len(), "index out of range");
            if (aTxt.GetChar( i ) == sal_Unicode( HYPH_POS_CHAR ))
            {
                aTxt.SetChar( i, sal_Unicode( CUR_HYPH_POS_CHAR ) );

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
}


void SvxHyphenWordDialog_Impl::SelRight_Impl()
{
    String aTxt( aEditWord );
    for ( xub_StrLen i = nOldPos + 1;  i < aTxt.Len();  ++i )
    {
        if (aTxt.GetChar( i ) == sal_Unicode( HYPH_POS_CHAR ))
        {
            aTxt.SetChar( i, sal_Unicode( CUR_HYPH_POS_CHAR ) );

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


IMPL_LINK( SvxHyphenWordDialog_Impl, CutHdl_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        ContinueHyph_Impl( /*nHyphPos*/nOldPos );
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
            const rtl::OUString aName( RTL_CONSTASCII_USTRINGPARAM( "IsHyphAuto" ) );
            uno::Any aAny;

            aAny <<= sal_True;
            xProp->setPropertyValue( aName, aAny );

            bBusy = sal_True;
            ContinueHyph_Impl( /*nHyphPos*/nOldPos );
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
        SelLeft_Impl();
        bBusy = sal_False;
    }
    return 0;
}


IMPL_LINK( SvxHyphenWordDialog_Impl, Right_Impl, Button *, EMPTYARG )
{
    if( !bBusy )
    {
        bBusy = sal_True;
        SelRight_Impl();
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
    m_pImpl->SelRight_Impl();
}


void SvxHyphenWordDialog::SelRight()
{
    m_pImpl->SelLeft_Impl();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
