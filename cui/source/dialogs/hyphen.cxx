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
#include <vcl/msgbox.hxx>
#include <vcl/builderfactory.hxx>

#define HYPH_POS_CHAR       '='

#define CUR_HYPH_POS_CHAR   '-'

using namespace css;

HyphenEdit::HyphenEdit(vcl::Window* pParent)
    : Edit(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK|WB_TABSTOP)
{
}

VCL_BUILDER_FACTORY(HyphenEdit)

void HyphenEdit::KeyInput( const KeyEvent& rKEvt )
{
    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();

    switch ( nCode )
    {
        case KEY_LEFT:
            static_cast<SvxHyphenWordDialog*>( GetParentDialog() )->SelLeft();
            break;

        case KEY_RIGHT:
            static_cast<SvxHyphenWordDialog*>( GetParentDialog() )->SelRight();
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
    const sal_Int32 nLen = m_aEditWord.getLength();

    m_pRightBtn->Disable();
    for ( sal_Int32 i = m_nOldPos + 2; i < nLen; ++i )
    {
        if ( m_aEditWord[ i ] == sal_Unicode( HYPH_POS_CHAR ) )
        {
            m_pRightBtn->Enable();
            break;
        }
    }

    DBG_ASSERT(m_nOldPos < nLen, "nOldPos out of range");
    if (m_nOldPos >= nLen)
        m_nOldPos = nLen - 1;
    m_pLeftBtn->Disable();
    for ( sal_Int32 i = m_nOldPos;  i-- > 0; )
    {
        if ( m_aEditWord[ i ] == sal_Unicode( HYPH_POS_CHAR ) )
        {
            m_pLeftBtn->Enable();
            break;
        }
    }
}


OUString SvxHyphenWordDialog::EraseUnusableHyphens_Impl()
{
    // returns a String showing only those hyphen positions which will result
    // in a line break if hyphenation is done there
    // 1) we will need to discard all hyphenation positions at the end that
    // will not result in a line break where the text to the left still fits
    // on the line.
    // 2) since as from OOo 3.2 '-' are part of a word an thus text like
    // 'multi-line-editor' is regarded as single word we also need to discard those
    // hyphenation positions to the left of the rightmost '-' that is still left of
    // the rightmost valid hyphenation position according to 1)

    // Example:
    // If the possible hyphenation position in 'multi-line-editor' are to be marked
    // by '=' then the text will look like this: 'mul=ti-line-ed=it=or'.
    // If now the first line is only large enough for 'multi-line-edi' we need to discard
    // the last possible hyphenation point because of 1). The right most valid
    // hyphenation position is "ed=itor". The first '-' left of this position is
    // "line-ed", thus because of 2) we now need to discard all possible hyphenation
    // positions to the left of that as well. Thus in the end leaving us with just
    // 'multi-line-ed=itor' as return value for this function. (Just one valid hyphenation
    // position for the user too choose from. However ALL the '-' characters in the word
    // will ALWAYS be valid implicit hyphenation positions for the core to choose from!
    // And thus even if this word is skipped in the hyphenation dialog it will still be broken
    // right after 'multi-line-' (actually it might already be broken up that way before
    // the hyphenation dialog is called!).
    // Thus rule 2) just eliminates those positions which will not be used by the core at all
    // even if the user were to select one of them.

    OUString aTxt;
    DBG_ASSERT(m_xPossHyph.is(), "missing possible hyphens");
    if (m_xPossHyph.is())
    {
        DBG_ASSERT( m_aActWord == m_xPossHyph->getWord(), "word mismatch"  );

        aTxt = m_xPossHyph->getPossibleHyphens();

        m_nHyphenationPositionsOffset = 0;
        uno::Sequence< sal_Int16 > aHyphenationPositions(
                m_xPossHyph->getHyphenationPositions() );
        sal_Int32 nLen = aHyphenationPositions.getLength();
        const sal_Int16 *pHyphenationPos = aHyphenationPositions.getConstArray();

        // find position nIdx after which all hyphen positions are unusable
        sal_Int32  nIdx = -1;
        sal_Int32  nPos = 0, nPos1 = 0;
        if (nLen)
        {
            sal_Int32 nStart = 0;
            for (sal_Int32 i = 0;  i < nLen;  ++i)
            {
                if (pHyphenationPos[i] > m_nMaxHyphenationPos)
                    break;
                else
                {
                    // find corresponding hyphen positions in string
                    nPos = aTxt.indexOf( sal_Unicode( HYPH_POS_CHAR ), nStart );

                    if (nPos == -1)
                        break;
                    else
                    {
                        nIdx = nPos;
                        nStart = nPos + 1;
                    }
                }
            }
        }
        DBG_ASSERT(nIdx != -1, "no usable hyphenation position");

        // 1) remove all not usable hyphenation positions from the end of the string
        nPos = nIdx == -1 ? 0 : nIdx + 1;
        nPos1 = nPos;   //save for later use in 2) below
        const OUString aTmp( sal_Unicode( HYPH_POS_CHAR ) );
        while (nPos != -1)
        {
            nPos++;
            aTxt = aTxt.replaceFirst( aTmp, "", &nPos);
        }

        // 2) remove all hyphenation positions from the start that are not considered by the core
        const OUString aSearchRange( aTxt.copy( 0, nPos1 ) );
        sal_Int32 nPos2 = aSearchRange.lastIndexOf( '-' );  // the '-' position the core will use by default
        if (nPos2 != -1 )
        {
            OUString aLeft( aSearchRange.copy( 0, nPos2 ) );
            nPos = 0;
            while (nPos != -1)
            {
                nPos++;
                aLeft = aLeft.replaceFirst( aTmp, "", &nPos );
                if (nPos != -1)
                    ++m_nHyphenationPositionsOffset;
            }
            aTxt = aTxt.replaceAt( 0, nPos2, aLeft );
        }
    }
    return aTxt;
}


void SvxHyphenWordDialog::InitControls_Impl()
{
    m_xPossHyph = nullptr;
    if (m_xHyphenator.is())
    {
        lang::Locale aLocale( LanguageTag::convertToLocale(m_nActLanguage) );
        m_xPossHyph = m_xHyphenator->createPossibleHyphens( m_aActWord, aLocale,
                                                        uno::Sequence< beans::PropertyValue >() );
        if (m_xPossHyph.is())
            m_aEditWord = EraseUnusableHyphens_Impl();
    }
    m_pWordEdit->SetText( m_aEditWord );

    m_nOldPos = m_aEditWord.getLength();
    SelLeft();
    EnableLRBtn_Impl();
}


void SvxHyphenWordDialog::ContinueHyph_Impl( sal_Int32 nInsPos )
{
    if ( nInsPos >= 0 && m_xPossHyph.is() )
    {
        if (nInsPos)
        {
            DBG_ASSERT(nInsPos <= m_aEditWord.getLength() - 2, "wrong hyphen position");

            sal_Int32 nIdxPos = -1;
            for (sal_Int32 i = 0; i <= nInsPos; ++i)
            {
                if (HYPH_POS_CHAR == m_aEditWord[ i ])
                    nIdxPos++;
            }
            // take the possible hyphenation positions that got removed from the
            // start of the word into account:
            nIdxPos += m_nHyphenationPositionsOffset;

            uno::Sequence< sal_Int16 > aSeq = m_xPossHyph->getHyphenationPositions();
            sal_Int32 nLen = aSeq.getLength();
            DBG_ASSERT(nLen, "empty sequence");
            DBG_ASSERT(0 <= nIdxPos && nIdxPos < nLen, "index out of range");
            if (nLen && 0 <= nIdxPos && nIdxPos < nLen)
            {
                nInsPos = aSeq.getConstArray()[ nIdxPos ];
                m_pHyphWrapper->InsertHyphen( nInsPos );
            }
        }
        else
        {
            //! calling with 0 as argument will remove hyphens!
            m_pHyphWrapper->InsertHyphen( nInsPos );
        }
    }

    if ( m_pHyphWrapper->FindSpellError() )
    {
        uno::Reference< linguistic2::XHyphenatedWord >  xHyphWord( m_pHyphWrapper->GetLast(), uno::UNO_QUERY );

        // adapt actual word and language to new found hyphenation result
        if(xHyphWord.is())
        {
            m_aActWord = xHyphWord->getWord();
            m_nActLanguage = LanguageTag( xHyphWord->getLocale() ).getLanguageType();
            m_nMaxHyphenationPos = xHyphWord->getHyphenationPos();
            InitControls_Impl();
            SetWindowTitle( m_nActLanguage );
        }
    }
    else
        EndDialog( RET_OK );
}


sal_uInt16 SvxHyphenWordDialog::GetHyphIndex_Impl()
{
    sal_uInt16 nPos = 0;
    const OUString aTxt( m_pWordEdit->GetText() );

    for ( sal_Int32 i=0; i < aTxt.getLength(); ++i )
    {
        sal_Unicode cChar = aTxt[ i ];
        if ( cChar == CUR_HYPH_POS_CHAR )
            break;
        if ( cChar != HYPH_POS_CHAR )
            nPos++;
    }
    return nPos;
}


void SvxHyphenWordDialog::SelLeft()
{
    DBG_ASSERT( m_nOldPos > 0, "invalid hyphenation position" );
    if (m_nOldPos > 0)
    {
        OUString aTxt( m_aEditWord );
        for( sal_Int32 i = m_nOldPos - 1;  i > 0; --i )
        {
            DBG_ASSERT(i <= aTxt.getLength(), "index out of range");
            if (aTxt[ i ] == sal_Unicode( HYPH_POS_CHAR ))
            {
                aTxt = aTxt.replaceAt( i, 1, OUString( CUR_HYPH_POS_CHAR ) );

                m_nOldPos = i;
                m_pWordEdit->SetText( aTxt );
                m_pWordEdit->GrabFocus();
                m_pWordEdit->SetSelection( Selection( i, i + 1 ) );
                break;
            }
        }
        m_nHyphPos = GetHyphIndex_Impl();
        EnableLRBtn_Impl();
    }
}


void SvxHyphenWordDialog::SelRight()
{
    OUString aTxt( m_aEditWord );
    for ( sal_Int32 i = m_nOldPos + 1;  i < aTxt.getLength();  ++i )
    {
        if (aTxt[ i ] == sal_Unicode( HYPH_POS_CHAR ))
        {
            aTxt = aTxt.replaceAt( i, 1, OUString( CUR_HYPH_POS_CHAR ) );

            m_nOldPos = i;
            m_pWordEdit->SetText( aTxt );
            m_pWordEdit->GrabFocus();
            m_pWordEdit->SetSelection( Selection( i, i + 1 ) );
            break;
        }
    }
    m_nHyphPos = GetHyphIndex_Impl();
    EnableLRBtn_Impl();
}


IMPL_LINK_NOARG_TYPED(SvxHyphenWordDialog, CutHdl_Impl, Button*, void)
{
    if( !m_bBusy )
    {
        m_bBusy = true;
        ContinueHyph_Impl( /*m_nHyphPos*/m_nOldPos );
        m_bBusy = false;
    }
}


IMPL_LINK_NOARG_TYPED( SvxHyphenWordDialog, HyphenateAllHdl_Impl, Button *, void )
{
    if( !m_bBusy )
    {
        try
        {
            uno::Reference< linguistic2::XLinguProperties >  xProp( SvxGetLinguPropertySet() );

            xProp->setIsHyphAuto( sal_True );

            m_bBusy = true;
            ContinueHyph_Impl( /*m_nHyphPos*/m_nOldPos );
            m_bBusy = false;

            xProp->setIsHyphAuto( sal_False );
        }
        catch (uno::Exception &e)
        {
            (void) e;
            SAL_WARN( "cui", "Hyphenate All failed" );
        }
    }
}


IMPL_LINK_NOARG_TYPED(SvxHyphenWordDialog, DeleteHdl_Impl, Button*, void)
{
    if( !m_bBusy )
    {
        m_bBusy = true;
        ContinueHyph_Impl( 0 );
        m_bBusy = false;
    }
}


IMPL_LINK_NOARG_TYPED(SvxHyphenWordDialog, ContinueHdl_Impl, Button*, void)
{
    if( !m_bBusy )
    {
        m_bBusy = true;
        ContinueHyph_Impl();
        m_bBusy = false;
    }
}


IMPL_LINK_NOARG_TYPED(SvxHyphenWordDialog, CancelHdl_Impl, Button*, void)
{
    if( !m_bBusy )
    {
        m_bBusy = true;
        m_pHyphWrapper->SpellEnd();
        EndDialog();
        m_bBusy = false;
    }
}


IMPL_LINK_NOARG_TYPED(SvxHyphenWordDialog, Left_Impl, Button*, void)
{
    if( !m_bBusy )
    {
        m_bBusy = true;
        SelLeft();
        m_bBusy = false;
    }
}


IMPL_LINK_NOARG_TYPED(SvxHyphenWordDialog, Right_Impl, Button*, void)
{
    if( !m_bBusy )
    {
        m_bBusy = true;
        SelRight();
        m_bBusy = false;
    }
}


IMPL_LINK_NOARG_TYPED(SvxHyphenWordDialog, GetFocusHdl_Impl, Control&, void)
{
    m_pWordEdit->SetSelection( Selection( m_nOldPos, m_nOldPos + 1 ) );
}


// class SvxHyphenWordDialog ---------------------------------------------

SvxHyphenWordDialog::SvxHyphenWordDialog(
    const OUString &rWord, LanguageType nLang,
    vcl::Window* pParent,
    uno::Reference< linguistic2::XHyphenator >  &xHyphen,
    SvxSpellWrapper* pWrapper)
    : SfxModalDialog(pParent, "HyphenateDialog", "cui/ui/hyphenate.ui")
    , m_pHyphWrapper(pWrapper)
    , m_xHyphenator(nullptr)
    , m_xPossHyph(nullptr)
    , m_aActWord(rWord)
    , m_nActLanguage(nLang)
    , m_nMaxHyphenationPos(0)
    , m_nHyphPos(0)
    , m_nOldPos(0)
    , m_nHyphenationPositionsOffset(0)
    , m_bBusy(false)
{
    get(m_pWordEdit, "worded");
    get(m_pLeftBtn, "left");
    get(m_pRightBtn, "right");
    get(m_pOkBtn, "ok");
    get(m_pContBtn, "continue");
    get(m_pDelBtn, "delete");
    get(m_pHyphAll, "hyphall");
    get(m_pCloseBtn, "close");

    m_aLabel = GetText();
    m_xHyphenator = xHyphen;

    uno::Reference< linguistic2::XHyphenatedWord >  xHyphWord( m_pHyphWrapper ?
            m_pHyphWrapper->GetLast() : nullptr, uno::UNO_QUERY );
    DBG_ASSERT( xHyphWord.is(), "hyphenation result missing" );
    if (xHyphWord.is())
    {
        DBG_ASSERT( m_aActWord == xHyphWord->getWord(), "word mismatch" );
        DBG_ASSERT( m_nActLanguage == LanguageTag( xHyphWord->getLocale() ).getLanguageType(), "language mismatch" );
        m_nMaxHyphenationPos = xHyphWord->getHyphenationPos();
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
    if (!m_xHyphenator.is())
        Enable( false );
}

SvxHyphenWordDialog::~SvxHyphenWordDialog()
{
    disposeOnce();
}

void SvxHyphenWordDialog::dispose()
{
    m_pWordEdit.clear();
    m_pLeftBtn.clear();
    m_pRightBtn.clear();
    m_pOkBtn.clear();
    m_pContBtn.clear();
    m_pDelBtn.clear();
    m_pHyphAll.clear();
    m_pCloseBtn.clear();
    SfxModalDialog::dispose();
}


void SvxHyphenWordDialog::SetWindowTitle( LanguageType nLang )
{
    SetText( m_aLabel + " (" + SvtLanguageTable::GetLanguageString( nLang ) + ")" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
