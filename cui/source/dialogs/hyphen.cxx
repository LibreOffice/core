/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

using namespace css;

HyphenEdit::HyphenEdit(Window* pParent)
    : Edit(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK|WB_TABSTOP)
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
            ( (SvxHyphenWordDialog*)GetParentDialog() )->SelLeft();
            break;

        case KEY_RIGHT:
            ( (SvxHyphenWordDialog*)GetParentDialog() )->SelRight();
            break;

        case KEY_TAB:
        case KEY_ESCAPE:
        case KEY_RETURN:
            Edit::KeyInput(rKEvt);
            break;
        default:
            Control::KeyInput( rKEvt ); 
            break;
    }
}


void SvxHyphenWordDialog::EnableLRBtn_Impl()
{
    OUString  aTxt( aEditWord );
    sal_Int32 nLen = aTxt.getLength();

    m_pRightBtn->Disable();
    for ( sal_Int32 i = nOldPos + 2; i < nLen; ++i )
    {
        if ( aTxt[ i ] == sal_Unicode( HYPH_POS_CHAR ) )
        {
            m_pRightBtn->Enable();
            break;
        }
    }

    DBG_ASSERT(nOldPos < aTxt.getLength(), "nOldPos out of range");
    if (nOldPos >= aTxt.getLength())
        nOldPos = aTxt.getLength() - 1;
    m_pLeftBtn->Disable();
    for ( sal_Int32 i = nOldPos;  i-- > 0; )
    {
        if ( aTxt[ i ] == sal_Unicode( HYPH_POS_CHAR ) )
        {
            m_pLeftBtn->Enable();
            break;
        }
    }
}


OUString SvxHyphenWordDialog::EraseUnusableHyphens_Impl(
        uno::Reference< linguistic2::XPossibleHyphens >  &rxPossHyph,
        sal_uInt16 _nMaxHyphenationPos )
{
    
    
    
    
    
    
    
    
    
    //
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    

    OUString aTxt;
    DBG_ASSERT(rxPossHyph.is(), "missing possible hyphens");
    if (rxPossHyph.is())
    {
        DBG_ASSERT( aActWord == rxPossHyph->getWord(), "word mismatch"  );

        aTxt = rxPossHyph->getPossibleHyphens();

        nHyphenationPositionsOffset = 0;
        uno::Sequence< sal_Int16 > aHyphenationPositions(
                rxPossHyph->getHyphenationPositions() );
        sal_Int32 nLen = aHyphenationPositions.getLength();
        const sal_Int16 *pHyphenationPos = aHyphenationPositions.getConstArray();

        
        sal_Int32  nIdx = -1;
        sal_Int32  nPos = 0, nPos1 = 0;
        if (nLen)
        {
            sal_Int32 nStart = 0;
            for (sal_Int32 i = 0;  i < nLen;  ++i)
            {
                if (pHyphenationPos[i] > _nMaxHyphenationPos)
                    break;
                else
                {
                    
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

        
        nPos = nIdx == -1 ? 0 : nIdx + 1;
        nPos1 = nPos;   
        const OUString aTmp( sal_Unicode( HYPH_POS_CHAR ) );
        const OUString aEmpty;
        while (nPos != -1)
        {
            nPos++;
            aTxt = aTxt.replaceFirst( aTmp, aEmpty, &nPos);
        }

        
        const OUString aSearchRange( aTxt.copy( 0, nPos1 ) );
        sal_Int32 nPos2 = aSearchRange.lastIndexOf( '-' );  
        if (nPos2 != -1 )
        {
            OUString aLeft( aSearchRange.copy( 0, nPos2 ) );
            nPos = 0;
            while (nPos != -1)
            {
                nPos++;
                aLeft = aLeft.replaceFirst( aTmp, aEmpty, &nPos );
                if (nPos != -1)
                    ++nHyphenationPositionsOffset;
            }
            aTxt = aTxt.replaceAt( 0, nPos2, aLeft );
        }
    }
    return aTxt;
}


void SvxHyphenWordDialog::InitControls_Impl()
{
    xPossHyph = NULL;
    if (xHyphenator.is())
    {
        lang::Locale aLocale( LanguageTag::convertToLocale(nActLanguage) );
        xPossHyph = xHyphenator->createPossibleHyphens( aActWord, aLocale,
                                                        uno::Sequence< beans::PropertyValue >() );
        if (xPossHyph.is())
            aEditWord = EraseUnusableHyphens_Impl( xPossHyph, nMaxHyphenationPos );
    }
    m_pWordEdit->SetText( aEditWord );

    nOldPos = aEditWord.getLength();
    SelLeft();
    EnableLRBtn_Impl();
}


void SvxHyphenWordDialog::ContinueHyph_Impl( sal_uInt16 nInsPos )
{
    if ( nInsPos != CONTINUE_HYPH  &&  xPossHyph.is())
    {
        if (nInsPos)
        {
            OUString aTmp( aEditWord );
            DBG_ASSERT(nInsPos <= aTmp.getLength() - 2, "wrong hyphen position");

            sal_Int16 nIdxPos = -1;
            for (sal_Int32 i = 0; i <= nInsPos; ++i)
            {
                if (HYPH_POS_CHAR == aTmp[ i ])
                    nIdxPos++;
            }
            
            
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
            
            pHyphWrapper->InsertHyphen( nInsPos );
        }
    }

    if ( pHyphWrapper->FindSpellError() )
    {
        uno::Reference< linguistic2::XHyphenatedWord >  xHyphWord( pHyphWrapper->GetLast(), uno::UNO_QUERY );

        
        if(xHyphWord.is())
        {
            aActWord    = xHyphWord->getWord();
            nActLanguage = LanguageTag( xHyphWord->getLocale() ).getLanguageType();
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
    OUString aTxt( m_pWordEdit->GetText() );

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
    DBG_ASSERT( nOldPos > 0, "invalid hyphenation position" );
    if (nOldPos > 0)
    {
        OUString aTxt( aEditWord );
        for( sal_Int32 i = nOldPos - 1;  i > 0; --i )
        {
            DBG_ASSERT(i <= aTxt.getLength(), "index out of range");
            if (aTxt[ i ] == sal_Unicode( HYPH_POS_CHAR ))
            {
                aTxt = aTxt.replaceAt( i, 1, OUString( CUR_HYPH_POS_CHAR ) );

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
    OUString aTxt( aEditWord );
    for ( sal_Int32 i = nOldPos + 1;  i < aTxt.getLength();  ++i )
    {
        if (aTxt[ i ] == sal_Unicode( HYPH_POS_CHAR ))
        {
            aTxt = aTxt.replaceAt( i, 1, OUString( CUR_HYPH_POS_CHAR ) );

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
            uno::Reference< linguistic2::XLinguProperties >  xProp( SvxGetLinguPropertySet() );

            xProp->setIsHyphAuto( sal_True );

            bBusy = sal_True;
            ContinueHyph_Impl( /*nHyphPos*/nOldPos );
            bBusy = sal_False;

            xProp->setIsHyphAuto( sal_False );
        }
        catch (uno::Exception &e)
        {
            (void) e;
            DBG_ASSERT( false, "Hyphenate All failed" );
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




SvxHyphenWordDialog::SvxHyphenWordDialog(
    const OUString &rWord, LanguageType nLang,
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
        DBG_ASSERT( aActWord == xHyphWord->getWord(), "word mismatch" );
        DBG_ASSERT( nActLanguage == LanguageTag( xHyphWord->getLocale() ).getLanguageType(), "language mismatch" );
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

    
    if (!xHyphenator.is())
        Enable( false );
}


SvxHyphenWordDialog::~SvxHyphenWordDialog()
{
}


void SvxHyphenWordDialog::SetWindowTitle( LanguageType nLang )
{
    OUString aLangStr( SvtLanguageTable::GetLanguageString( nLang ) );
    OUString aTmp( aLabel );
    aTmp += " (";
    aTmp += aLangStr;
    aTmp += ")";
    SetText( aTmp );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
