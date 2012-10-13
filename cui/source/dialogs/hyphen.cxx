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

#include "hyphen.hxx"
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

#define HYPH_POS_CHAR       '='
#define CONTINUE_HYPH       USHRT_MAX

#define CUR_HYPH_POS_CHAR   '-'

HyphenEdit::HyphenEdit(Window* pParent)
    : Edit(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK)
{
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeHyphenEdit(Window *pParent, VclBuilder::stringmap &)
{
    return new HyphenEdit(pParent);
}

void HyphenEdit::KeyInput( const KeyEvent& rKEvt )
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
            Control::KeyInput( rKEvt ); // pass on to the dialog
            break;
    }
}


void SvxHyphenWordDialog::EnableLRBtn_Impl()
{
    String  aTxt( aEditWord );
    xub_StrLen nLen = aTxt.Len();
    xub_StrLen i;

    m_pRightBtn->Disable();
    for ( i = nOldPos + 2; i < nLen; ++i )
    {
        if ( aTxt.GetChar( i ) == sal_Unicode( HYPH_POS_CHAR ) )
        {
            m_pRightBtn->Enable();
            break;
        }
    }

    DBG_ASSERT(nOldPos < aTxt.Len(), "nOldPos out of range");
    if (nOldPos >= aTxt.Len())
        nOldPos = aTxt.Len() - 1;
    m_pLeftBtn->Disable();
    for ( i = nOldPos;  i-- > 0; )
    {
        if ( aTxt.GetChar( i ) == sal_Unicode( HYPH_POS_CHAR ) )
        {
            m_pLeftBtn->Enable();
            break;
        }
    }
}


String SvxHyphenWordDialog::EraseUnusableHyphens_Impl(
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
        const rtl::OUString aTmp( sal_Unicode( HYPH_POS_CHAR ) );
        const rtl::OUString aEmpty;
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


void SvxHyphenWordDialog::InitControls_Impl()
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
    m_pWordEdit->SetText( aEditWord );

    nOldPos = aEditWord.Len();
    SelLeft();
    EnableLRBtn_Impl();
}


void SvxHyphenWordDialog::ContinueHyph_Impl( sal_uInt16 nInsPos )
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
            SetWindowTitle( nActLanguage );
        }
    }
    else
        EndDialog( RET_OK );
}


sal_uInt16 SvxHyphenWordDialog::GetHyphIndex_Impl()
{
    sal_uInt16 nPos = 0;
    String aTxt( m_pWordEdit->GetText() );

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


void SvxHyphenWordDialog::SelLeft()
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
                m_pWordEdit->SetText( aTxt );
                m_pWordEdit->GrabFocus();
                m_pWordEdit->SetSelection( Selection( i, i + 1 ) );
                break;
            }
        }
        nHyphPos = GetHyphIndex_Impl();
        EnableLRBtn_Impl();
    }
}


void SvxHyphenWordDialog::SelRight()
{
    String aTxt( aEditWord );
    for ( xub_StrLen i = nOldPos + 1;  i < aTxt.Len();  ++i )
    {
        if (aTxt.GetChar( i ) == sal_Unicode( HYPH_POS_CHAR ))
        {
            aTxt.SetChar( i, sal_Unicode( CUR_HYPH_POS_CHAR ) );

            nOldPos = i;
            m_pWordEdit->SetText( aTxt );
            m_pWordEdit->GrabFocus();
            m_pWordEdit->SetSelection( Selection( i, i + 1 ) );
            break;
        }
    }
    nHyphPos = GetHyphIndex_Impl();
    EnableLRBtn_Impl();
}


IMPL_LINK_NOARG(SvxHyphenWordDialog, CutHdl_Impl)
{
    if( !bBusy )
    {
        bBusy = sal_True;
        ContinueHyph_Impl( /*nHyphPos*/nOldPos );
        bBusy = sal_False;
    }
    return 0;
}


IMPL_LINK( SvxHyphenWordDialog, HyphenateAllHdl_Impl, Button *, EMPTYARG /*pButton*/ )
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


IMPL_LINK_NOARG(SvxHyphenWordDialog, DeleteHdl_Impl)
{
    if( !bBusy )
    {
        bBusy = sal_True;
        ContinueHyph_Impl();
        bBusy = sal_False;
    }
    return 0;
}


IMPL_LINK_NOARG(SvxHyphenWordDialog, ContinueHdl_Impl)
{
    if( !bBusy )
    {
        bBusy = sal_True;
        ContinueHyph_Impl( CONTINUE_HYPH );
        bBusy = sal_False;
    }
    return 0;
}


IMPL_LINK_NOARG(SvxHyphenWordDialog, CancelHdl_Impl)
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


IMPL_LINK_NOARG(SvxHyphenWordDialog, Left_Impl)
{
    if( !bBusy )
    {
        bBusy = sal_True;
        SelLeft();
        bBusy = sal_False;
    }
    return 0;
}


IMPL_LINK_NOARG(SvxHyphenWordDialog, Right_Impl)
{
    if( !bBusy )
    {
        bBusy = sal_True;
        SelRight();
        bBusy = sal_False;
    }
    return 0;
}


IMPL_LINK_NOARG(SvxHyphenWordDialog, GetFocusHdl_Impl)
{
    m_pWordEdit->SetSelection( Selection( nOldPos, nOldPos + 1 ) );
    return 0;
}


// class SvxHyphenWordDialog ---------------------------------------------

SvxHyphenWordDialog::SvxHyphenWordDialog(
    const String &rWord, LanguageType nLang,
    Window* pParent,
    uno::Reference< linguistic2::XHyphenator >  &xHyphen,
    SvxSpellWrapper* pWrapper)
    : SfxModalDialog(pParent, "HyphenateDialog", "cui/ui/hyphenate.ui")
    , pHyphWrapper(NULL)
    , xHyphenator(NULL)
    , xPossHyph(NULL)
    , nActLanguage(LANGUAGE_NONE)
    , nMaxHyphenationPos(0)
    , nHyphPos(0)
    , nOldPos(0)
    , nHyphenationPositionsOffset(0)
    , bBusy(sal_False)
{
    get(m_pWordEdit, "worded");
    get(m_pLeftBtn, "left");
    get(m_pRightBtn, "right");
    get(m_pOkBtn, "ok");
    get(m_pContBtn, "continue");
    get(m_pDelBtn, "delete");
    get(m_pHyphAll, "hyphall");
    get(m_pCloseBtn, "close");

    aLabel = GetText();
    aActWord = rWord;
    nActLanguage = nLang;
    xHyphenator = xHyphen;
    pHyphWrapper = pWrapper;

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
    m_pWordEdit->GrabFocus();

    m_pLeftBtn->SetClickHdl( LINK( this, SvxHyphenWordDialog, Left_Impl ) );
    m_pRightBtn->SetClickHdl( LINK( this, SvxHyphenWordDialog, Right_Impl ) );
    m_pOkBtn->SetClickHdl( LINK( this, SvxHyphenWordDialog, CutHdl_Impl ) );
    m_pContBtn->SetClickHdl( LINK( this, SvxHyphenWordDialog, ContinueHdl_Impl ) );
    m_pDelBtn->SetClickHdl( LINK( this, SvxHyphenWordDialog, DeleteHdl_Impl ) );
    m_pHyphAll->SetClickHdl( LINK( this, SvxHyphenWordDialog, HyphenateAllHdl_Impl ) );
    m_pCloseBtn->SetClickHdl( LINK( this, SvxHyphenWordDialog, CancelHdl_Impl ) );
    m_pWordEdit->SetGetFocusHdl( LINK( this, SvxHyphenWordDialog, GetFocusHdl_Impl ) );

    SetWindowTitle( nLang );

    // disable controls if service is not available
    if (!xHyphenator.is())
        Enable( sal_False );
}


SvxHyphenWordDialog::~SvxHyphenWordDialog()
{
}


void SvxHyphenWordDialog::SetWindowTitle( LanguageType nLang )
{
    String aLangStr( SvtLanguageTable::GetLanguageString( nLang ) );
    String aTmp( aLabel );
    aTmp.AppendAscii( RTL_CONSTASCII_STRINGPARAM( " (" ) );
    aTmp.Append( aLangStr );
    aTmp.Append( sal_Unicode( ')' ) );
    SetText( aTmp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
