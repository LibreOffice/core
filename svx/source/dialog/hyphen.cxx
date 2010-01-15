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

// struct SvxHyphenWordDialog_Impl ---------------------------------------------

struct SvxHyphenWordDialog_Impl
{
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

    SvxHyphenWordDialog_Impl( Window* pParent );
    ~SvxHyphenWordDialog_Impl();
};


SvxHyphenWordDialog_Impl::SvxHyphenWordDialog_Impl( Window* pParent ) :
    aWordFT     ( pParent, SVX_RES( FT_WORD ) ),
    aWordEdit   ( pParent, SVX_RES( ED_WORD ) ),
    aLeftBtn    ( pParent, SVX_RES( BTN_LEFT ) ),
    aRightBtn   ( pParent, SVX_RES( BTN_RIGHT ) ),
    aOkBtn      ( pParent, SVX_RES( BTN_HYPH_CUT ) ),
    aContBtn    ( pParent, SVX_RES( BTN_HYPH_CONTINUE ) ),
    aDelBtn     ( pParent, SVX_RES( BTN_HYPH_DELETE ) ),
    aFLBottom   ( pParent, SVX_RES( FL_BOTTOM ) ),
    aHelpBtn    ( pParent, SVX_RES( BTN_HYPH_HELP ) ),
    aHyphAll    ( pParent, SVX_RES( BTN_HYPH_ALL ) ),
    aCancelBtn  ( pParent, SVX_RES( BTN_HYPH_CANCEL ) ),
    aLabel          (  ),
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
}


SvxHyphenWordDialog_Impl::~SvxHyphenWordDialog_Impl()
{
}

// class SvxHyphenWordDialog ---------------------------------------------

SvxHyphenWordDialog::SvxHyphenWordDialog(
    const String &rWord, LanguageType nLang,
    Window* pParent,
    uno::Reference< linguistic2::XHyphenator >  &xHyphen,
    SvxSpellWrapper* pWrapper ) :

    SfxModalDialog( pParent, SVX_RES( RID_SVXDLG_HYPHENATE ) )
{
    m_pImpl = boost::shared_ptr< SvxHyphenWordDialog_Impl >(new SvxHyphenWordDialog_Impl( this ));

    FreeResource();

    m_pImpl->aLabel         = GetText();
    m_pImpl->pHyphWrapper   = pWrapper;
    m_pImpl->xHyphenator    = xHyphen;
    m_pImpl->aActWord       = rWord;
    m_pImpl->nActLanguage   = nLang;

    m_pImpl->aLeftBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog, Left_Impl ) );
    m_pImpl->aRightBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog, Right_Impl ) );
    m_pImpl->aOkBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog, CutHdl_Impl ) );
    m_pImpl->aContBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog, ContinueHdl_Impl ) );
    m_pImpl->aDelBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog, DeleteHdl_Impl ) );
    m_pImpl->aHyphAll.SetClickHdl( LINK( this, SvxHyphenWordDialog, HyphenateAllHdl_Impl ) );
    m_pImpl->aCancelBtn.SetClickHdl( LINK( this, SvxHyphenWordDialog, CancelHdl_Impl ) );
    m_pImpl->aWordEdit.SetGetFocusHdl( LINK( this, SvxHyphenWordDialog, GetFocusHdl_Impl ) );

    uno::Reference< linguistic2::XHyphenatedWord >  xHyphWord( pWrapper ?
            pWrapper->GetLast() : uno::Reference< uno::XInterface > () , uno::UNO_QUERY );
    DBG_ASSERT(xHyphWord.is(), "missing hyphenated word");
    m_pImpl->nMaxHyphenationPos = xHyphWord.is() ? xHyphWord->getHyphenationPos() : 0;
    SetLabel_Impl( nLang );

    InitControls_Impl();
    m_pImpl->aWordEdit.GrabFocus();

    // disable controls if service is not available
    if (!m_pImpl->xHyphenator.is())
        Enable( sal_False );
}

// -----------------------------------------------------------------------

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
    m_pImpl->nHyphPos = GetHyphIndex_Impl();
    EnableLRBtn_Impl();
}

// -----------------------------------------------------------------------

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
    m_pImpl->nHyphPos = GetHyphIndex_Impl();
    EnableLRBtn_Impl();
}

// -----------------------------------------------------------------------

void SvxHyphenWordDialog::EnableLRBtn_Impl()
{
    String  aTxt( m_pImpl->aWordEdit.GetText() );
    xub_StrLen nLen = aTxt.Len();
    xub_StrLen i;

    m_pImpl->aRightBtn.Disable();
    for ( i = m_pImpl->nOldPos + 2; i < nLen; ++i )
    {
        if ( aTxt.GetChar( i ) == sal_Unicode( SW_SOFT_HYPHEN ) )
        {
            m_pImpl->aRightBtn.Enable();
            break;
        }
    }

    DBG_ASSERT(m_pImpl->nOldPos < aTxt.Len(), "m_pImpl->nOldPos out of range");
    if (m_pImpl->nOldPos >= aTxt.Len())
        m_pImpl->nOldPos = aTxt.Len() - 1;
    m_pImpl->aLeftBtn.Disable();
    for ( i = m_pImpl->nOldPos;  i-- > 0; )
    {
        if ( aTxt.GetChar( i ) == sal_Unicode( SW_SOFT_HYPHEN ) )
        {
            m_pImpl->aLeftBtn.Enable();
            break;
        }
    }
}

// -----------------------------------------------------------------------


void SvxHyphenWordDialog::SetLabel_Impl( LanguageType nLang )
{
    String aLangStr( ::GetLanguageString( nLang ) );
    String aTmp( m_pImpl->aLabel );
    aTmp.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " (" ) );
    aTmp.Append( aLangStr );
    aTmp.Append( sal_Unicode( ')' ) );
    SetText( aTmp );
}

// -----------------------------------------------------------------------

String SvxHyphenWordDialog::EraseUnusableHyphens_Impl(
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

// -----------------------------------------------------------------------

void SvxHyphenWordDialog::InitControls_Impl()
{
    String aTxt;
    m_pImpl->xPossHyph = NULL;
    if (m_pImpl->xHyphenator.is())
    {
        lang::Locale aLocale( SvxCreateLocale(m_pImpl->nActLanguage) );
        m_pImpl->xPossHyph = m_pImpl->xHyphenator->createPossibleHyphens( m_pImpl->aActWord, aLocale,
                                                        uno::Sequence< beans::PropertyValue >() );
        if (m_pImpl->xPossHyph.is())
        {
            aTxt = EraseUnusableHyphens_Impl( m_pImpl->xPossHyph, m_pImpl->nMaxHyphenationPos );
        }
        SetLabel_Impl( m_pImpl->nActLanguage );
    }
    m_pImpl->aWordEdit.SetText( aTxt );

    m_pImpl->nOldPos = aTxt.Len();
    SelLeft();
    EnableLRBtn_Impl();
}

// -----------------------------------------------------------------------

void SvxHyphenWordDialog::ContinueHyph_Impl( sal_uInt16 nInsPos )
{
    if ( nInsPos != CONTINUE_HYPH  &&  m_pImpl->xPossHyph.is())
    {
        if (nInsPos)
        {
            String aTmp( m_pImpl->aWordEdit.GetText() );
            DBG_ASSERT(nInsPos <= aTmp.Len() - 2, "wrong hyphen position");

            sal_uInt16 nIdxPos = 0;
            sal_uInt16 i = 0;
            sal_Unicode  c;
            while (i < aTmp.Len()  &&  HYPHHERE != (c = aTmp.GetChar(i++)) )
            {
                if (SW_SOFT_HYPHEN == c)
                    nIdxPos++;
            }

            uno::Sequence< sal_Int16 > aSeq = m_pImpl->xPossHyph->getHyphenationPositions();
            sal_Int32 nLen = aSeq.getLength();
            DBG_ASSERT(nLen, "empty sequence");
            DBG_ASSERT(nIdxPos < nLen, "index out of range");
            if (nLen && nIdxPos < nLen)
            {
                nInsPos = aSeq.getConstArray()[ nIdxPos ];
                m_pImpl->pHyphWrapper->InsertHyphen( nInsPos );
            }
        }
        else
        {
            //! calling with 0 as argument will remove hyphens!
            m_pImpl->pHyphWrapper->InsertHyphen( nInsPos );
        }
    }

    if ( m_pImpl->pHyphWrapper->FindSpellError() )
    {
        uno::Reference< linguistic2::XHyphenatedWord >  xHyphWord( m_pImpl->pHyphWrapper->GetLast(), uno::UNO_QUERY );

        // adapt actual word and language to new found hyphenation result
        if(xHyphWord.is())
        {
            m_pImpl->aActWord    = String( xHyphWord->getWord() );
            m_pImpl->nActLanguage = SvxLocaleToLanguage( xHyphWord->getLocale() );
            m_pImpl->nMaxHyphenationPos = xHyphWord->getHyphenationPos();
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
    String aTxt(m_pImpl->aWordEdit.GetText());

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

IMPL_LINK( SvxHyphenWordDialog, CutHdl_Impl, Button *, EMPTYARG )
{
    if( !m_pImpl->bBusy )
    {
        m_pImpl->bBusy = sal_True;
        ContinueHyph_Impl( m_pImpl->nHyphPos );
        m_pImpl->bBusy = sal_False;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxHyphenWordDialog, HyphenateAllHdl_Impl, Button *, EMPTYARG /*pButton*/ )
{
    if( !m_pImpl->bBusy )
    {
        try
        {
            uno::Reference< beans::XPropertySet >  xProp( SvxGetLinguPropertySet() );
            const rtl::OUString aName( rtl::OUString::createFromAscii( "IsHyphAuto" ) );
            uno::Any aAny;

            aAny <<= sal_True;
            xProp->setPropertyValue( aName, aAny );

            m_pImpl->bBusy = sal_True;
            ContinueHyph_Impl( m_pImpl->nHyphPos );
            m_pImpl->bBusy = sal_False;

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

// -----------------------------------------------------------------------

IMPL_LINK( SvxHyphenWordDialog, DeleteHdl_Impl, Button *, EMPTYARG )
{
    if( !m_pImpl->bBusy )
    {
        m_pImpl->bBusy = sal_True;
        ContinueHyph_Impl();
        m_pImpl->bBusy = sal_False;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxHyphenWordDialog, ContinueHdl_Impl, Button *, EMPTYARG )
{
    if( !m_pImpl->bBusy )
    {
        m_pImpl->bBusy = sal_True;
        ContinueHyph_Impl( CONTINUE_HYPH );
        m_pImpl->bBusy = sal_False;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxHyphenWordDialog, CancelHdl_Impl, Button *, EMPTYARG )
{
    if( !m_pImpl->bBusy )
    {
        m_pImpl->bBusy = sal_True;
        m_pImpl->pHyphWrapper->SpellEnd();
        EndDialog( RET_CANCEL );
        m_pImpl->bBusy = sal_False;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxHyphenWordDialog, Left_Impl, Button *, EMPTYARG )
{
    if( !m_pImpl->bBusy )
    {
        m_pImpl->bBusy = sal_True;
        SelLeft();
        m_pImpl->bBusy = sal_False;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxHyphenWordDialog, Right_Impl, Button *, EMPTYARG )
{
    if( !m_pImpl->bBusy )
    {
        m_pImpl->bBusy = sal_True;
        SelRight();
        m_pImpl->bBusy = sal_False;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxHyphenWordDialog, GetFocusHdl_Impl, Edit *, EMPTYARG )
{
    m_pImpl->aWordEdit.SetSelection( Selection( m_pImpl->nOldPos, m_pImpl->nOldPos + 1 ) );
    return 0;
}



