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

#include "sal/config.h"

#include <algorithm>

#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <tools/rc.h>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/field.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/settings.hxx>

#include <svdata.hxx>

#include <i18nlangtag/mslangid.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/i18n/KCharacterType.hpp>

#include <unotools/localedatawrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/charclass.hxx>
#include <unotools/misccfg.hxx>

using namespace ::com::sun::star;
using namespace ::comphelper;

#define EDITMASK_LITERAL       'L'
#define EDITMASK_ALPHA         'a'
#define EDITMASK_UPPERALPHA    'A'
#define EDITMASK_ALPHANUM      'c'
#define EDITMASK_UPPERALPHANUM 'C'
#define EDITMASK_NUM           'N'
#define EDITMASK_NUMSPACE      'n'
#define EDITMASK_ALLCHAR       'x'
#define EDITMASK_UPPERALLCHAR  'X'

uno::Reference< i18n::XCharacterClassification > ImplGetCharClass()
{
    static uno::Reference< i18n::XCharacterClassification > xCharClass;
    if ( !xCharClass.is() )
        xCharClass = vcl::unohelper::CreateCharacterClassification();

    return xCharClass;
}

static sal_Unicode* ImplAddString( sal_Unicode* pBuf, const OUString& rStr )
{
    if ( rStr.getLength() == 1 )
        *pBuf++ = rStr[0];
    else if ( rStr.isEmpty() )
        ;
    else
    {
        memcpy( pBuf, rStr.getStr(), rStr.getLength() * sizeof(sal_Unicode) );
        pBuf += rStr.getLength();
    }
    return pBuf;
}

static sal_Unicode* ImplAddNum( sal_Unicode* pBuf, sal_uLong nNumber, int nMinLen )
{
    // fill temp buffer with digits
    sal_Unicode aTempBuf[30];
    sal_Unicode* pTempBuf = aTempBuf;
    do
    {
        *pTempBuf = (sal_Unicode)(nNumber % 10) + '0';
        pTempBuf++;
        nNumber /= 10;
        if ( nMinLen )
            nMinLen--;
    }
    while ( nNumber );

    // fill with zeros up to the minimal length
    while ( nMinLen > 0 )
    {
        *pBuf = '0';
        pBuf++;
        nMinLen--;
    }

    // copy temp buffer to real buffer
    do
    {
        pTempBuf--;
        *pBuf = *pTempBuf;
        pBuf++;
    }
    while ( pTempBuf != aTempBuf );

    return pBuf;
}

static sal_uInt16 ImplGetNum( const sal_Unicode*& rpBuf, bool& rbError )
{
    if ( !*rpBuf )
    {
        rbError = true;
        return 0;
    }

    sal_uInt16 nNumber = 0;
    while( ( *rpBuf >= '0' ) && ( *rpBuf <= '9' ) )
    {
        nNumber *= 10;
        nNumber += *rpBuf - '0';
        rpBuf++;
    }

    return nNumber;
}

static void ImplSkipDelimiters( const sal_Unicode*& rpBuf )
{
    while( ( *rpBuf == ',' ) || ( *rpBuf == '.' ) || ( *rpBuf == ';' ) ||
           ( *rpBuf == ':' ) || ( *rpBuf == '-' ) || ( *rpBuf == '/' ) )
    {
        rpBuf++;
    }
}

static bool ImplIsPatternChar( sal_Unicode cChar, sal_Char cEditMask )
{
    sal_Int32 nType = 0;

    try
    {
        OUString aCharStr(cChar);
        nType = ImplGetCharClass()->getStringType( aCharStr, 0, aCharStr.getLength(),
                Application::GetSettings().GetLanguageTag().getLocale() );
    }
    catch (const css::uno::Exception&)
    {
        SAL_WARN( "vcl.control", "ImplIsPatternChar: Exception caught!" );
        DBG_UNHANDLED_EXCEPTION();
        return false;
    }

    if ( (cEditMask == EDITMASK_ALPHA) || (cEditMask == EDITMASK_UPPERALPHA) )
    {
        if( !CharClass::isLetterType( nType ) )
            return false;
    }
    else if ( cEditMask == EDITMASK_NUM )
    {
        if( !CharClass::isNumericType( nType ) )
            return false;
    }
    else if ( (cEditMask == EDITMASK_ALPHANUM) || (cEditMask == EDITMASK_UPPERALPHANUM) )
    {
        if( !CharClass::isLetterNumericType( nType ) )
            return false;
    }
    else if ( (cEditMask == EDITMASK_ALLCHAR) || (cEditMask == EDITMASK_UPPERALLCHAR) )
    {
        if ( cChar < 32 )
            return false;
    }
    else if ( cEditMask == EDITMASK_NUMSPACE )
    {
        if ( !CharClass::isNumericType( nType ) && ( cChar != ' ' ) )
            return false;
    }
    else
        return false;

    return true;
}

static sal_Unicode ImplPatternChar( sal_Unicode cChar, sal_Char cEditMask )
{
    if ( ImplIsPatternChar( cChar, cEditMask ) )
    {
        if ( (cEditMask == EDITMASK_UPPERALPHA) ||
             (cEditMask == EDITMASK_UPPERALPHANUM) ||
             ( cEditMask == EDITMASK_UPPERALLCHAR ) )
        {
            cChar = ImplGetCharClass()->toUpper(OUString(cChar), 0, 1,
                    Application::GetSettings().GetLanguageTag().getLocale())[0];
        }
        return cChar;
    }
    else
        return 0;
}

static bool ImplCommaPointCharEqual( sal_Unicode c1, sal_Unicode c2 )
{
    if ( c1 == c2 )
        return true;
    else if ( ((c1 == '.') || (c1 == ',')) &&
              ((c2 == '.') || (c2 == ',')) )
        return true;
    else
        return false;
}

static OUString ImplPatternReformat( const OUString& rStr,
                                     const OString& rEditMask,
                                     const OUString& rLiteralMask,
                                     sal_uInt16 nFormatFlags )
{
    if (rEditMask.isEmpty())
        return rStr;

    OUStringBuffer    aOutStr = OUString(rLiteralMask);
    sal_Unicode cTempChar;
    sal_Unicode cChar;
    sal_Unicode cLiteral;
    sal_Char    cMask;
    sal_Int32   nStrIndex = 0;
    sal_Int32   i = 0;
    sal_Int32   n;

    while ( i < rEditMask.getLength() )
    {
        if ( nStrIndex >= rStr.getLength() )
            break;

        cChar = rStr[nStrIndex];
        cLiteral = rLiteralMask[i];
        cMask = rEditMask[i];

        // current position is a literal
        if ( cMask == EDITMASK_LITERAL )
        {
            // if it is a literal copy otherwise ignore because it might be the next valid
            // character of the string
            if ( ImplCommaPointCharEqual( cChar, cLiteral ) )
                nStrIndex++;
            else
            {
                // Otherwise we check if it is a invalid character. This is the case if it does not
                // fit in the pattern of the next non-literal character.
                n = i+1;
                while ( n < rEditMask.getLength() )
                {
                    if ( rEditMask[n] != EDITMASK_LITERAL )
                    {
                        if ( !ImplIsPatternChar( cChar, rEditMask[n] ) )
                            nStrIndex++;
                        break;
                    }

                    n++;
                }
            }
        }
        else
        {
            // valid character at this position
            cTempChar = ImplPatternChar( cChar, cMask );
            if ( cTempChar )
            {
                // use this character
                aOutStr[i] = cTempChar;
                nStrIndex++;
            }
            else
            {
                // copy if it is a literal character
                if ( cLiteral == cChar )
                    nStrIndex++;
                else
                {
                    // If the invalid character might be the next literal character then we jump
                    // ahead to it, otherwise we ignore it. Do only if empty literals are allowed.
                    if ( nFormatFlags & PATTERN_FORMAT_EMPTYLITERALS )
                    {
                        n = i;
                        while ( n < rEditMask.getLength() )
                        {
                            if ( rEditMask[n] == EDITMASK_LITERAL )
                            {
                                if ( ImplCommaPointCharEqual( cChar, rLiteralMask[n] ) )
                                    i = n+1;

                                break;
                            }

                            n++;
                        }
                    }

                    nStrIndex++;
                    continue;
                }
            }
        }

        i++;
    }

    return aOutStr.makeStringAndClear();
}

static void ImplPatternMaxPos( const OUString& rStr, const OString& rEditMask,
                               sal_uInt16 nFormatFlags, bool bSameMask,
                               sal_Int32 nCursorPos, sal_Int32& rPos )
{

    // last position must not be longer than the contained string
    sal_Int32 nMaxPos = rStr.getLength();

    // if non empty literals are allowed ignore blanks at the end as well
    if ( bSameMask && !(nFormatFlags & PATTERN_FORMAT_EMPTYLITERALS) )
    {
        while ( nMaxPos )
        {
            if ( (rEditMask[nMaxPos-1] != EDITMASK_LITERAL) &&
                 (rStr[nMaxPos-1] != ' ') )
                break;
            nMaxPos--;
        }

        // if we are in front of a literal, continue search until first character after the literal
        sal_Int32 nTempPos = nMaxPos;
        while ( nTempPos < rEditMask.getLength() )
        {
            if ( rEditMask[nTempPos] != EDITMASK_LITERAL )
            {
                nMaxPos = nTempPos;
                break;
            }
            nTempPos++;
        }
    }

    if ( rPos > nMaxPos )
        rPos = nMaxPos;

    // character should not move left
    if ( rPos < nCursorPos )
        rPos = nCursorPos;
}

static void ImplPatternProcessStrictModify( Edit* pEdit,
                                            const OString& rEditMask,
                                            const OUString& rLiteralMask,
                                            sal_uInt16 nFormatFlags, bool bSameMask )
{
    OUString aText = pEdit->GetText();

    // remove leading blanks
    if ( bSameMask && !(nFormatFlags & PATTERN_FORMAT_EMPTYLITERALS) )
    {
        sal_Int32 i = 0;
        sal_Int32 nMaxLen = aText.getLength();
        while ( i < nMaxLen )
        {
            if ( (rEditMask[i] != EDITMASK_LITERAL) &&
                 (aText[i] != ' ') )
                break;

            i++;
        }
        // keep all literal characters
        while ( i && (rEditMask[i] == EDITMASK_LITERAL) )
            i--;
        aText = aText.copy( i );
    }

    OUString aNewText = ImplPatternReformat( aText, rEditMask, rLiteralMask, nFormatFlags );
    if ( aNewText != aText )
    {
        // adjust selection such that it remains at the end if it was there before
        Selection aSel = pEdit->GetSelection();
        sal_Int64 nMaxSel = std::max( aSel.Min(), aSel.Max() );
        if ( nMaxSel >= aText.getLength() )
        {
            sal_Int32 nMaxPos = aNewText.getLength();
            ImplPatternMaxPos( aNewText, rEditMask, nFormatFlags, bSameMask, nMaxSel, nMaxPos );
            if ( aSel.Min() == aSel.Max() )
            {
                aSel.Min() = nMaxPos;
                aSel.Max() = aSel.Min();
            }
            else if ( aSel.Min() > aSel.Max() )
                aSel.Min() = nMaxPos;
            else
                aSel.Max() = nMaxPos;
        }
        pEdit->SetText( aNewText, aSel );
    }
}

static sal_Int32 ImplPatternLeftPos(const OString& rEditMask, sal_Int32 nCursorPos)
{
    // search non-literal predecessor
    sal_Int32 nNewPos = nCursorPos;
    sal_Int32 nTempPos = nNewPos;
    while ( nTempPos )
    {
        if ( rEditMask[nTempPos-1] != EDITMASK_LITERAL )
        {
            nNewPos = nTempPos-1;
            break;
        }
        nTempPos--;
    }
    return nNewPos;
}

static sal_Int32 ImplPatternRightPos( const OUString& rStr, const OString& rEditMask,
                                       sal_uInt16 nFormatFlags, bool bSameMask,
                                       sal_Int32 nCursorPos )
{
    // search non-literal successor
    sal_Int32 nNewPos = nCursorPos;
    ;
    for(sal_Int32 nTempPos = nNewPos+1; nTempPos < rEditMask.getLength(); ++nTempPos )
    {
        if ( rEditMask[nTempPos] != EDITMASK_LITERAL )
        {
            nNewPos = nTempPos;
            break;
        }
    }
    ImplPatternMaxPos( rStr, rEditMask, nFormatFlags, bSameMask, nCursorPos, nNewPos );
    return nNewPos;
}

static bool ImplPatternProcessKeyInput( Edit* pEdit, const KeyEvent& rKEvt,
                                        const OString& rEditMask,
                                        const OUString& rLiteralMask,
                                        bool bStrictFormat,
                                        sal_uInt16 nFormatFlags,
                                        bool bSameMask,
                                        bool& rbInKeyInput )
{
    if ( rEditMask.isEmpty() || !bStrictFormat )
        return false;

    Selection   aOldSel     = pEdit->GetSelection();
    vcl::KeyCode aCode      = rKEvt.GetKeyCode();
    sal_Unicode cChar       = rKEvt.GetCharCode();
    sal_uInt16      nKeyCode    = aCode.GetCode();
    bool        bShift      = aCode.IsShift();
    sal_Int32  nCursorPos = static_cast<sal_Int32>(aOldSel.Max());
    sal_Int32  nNewPos;
    sal_Int32  nTempPos;

    if ( nKeyCode && !aCode.IsMod1() && !aCode.IsMod2() )
    {
        if ( nKeyCode == KEY_LEFT )
        {
            Selection aSel( ImplPatternLeftPos( rEditMask, nCursorPos ) );
            if ( bShift )
                aSel.Min() = aOldSel.Min();
            pEdit->SetSelection( aSel );
            return true;
        }
        else if ( nKeyCode == KEY_RIGHT )
        {
            // Use the start of selection as minimum; even a small position is allowed in case that
            // all was selected by the focus
            Selection aSel( aOldSel );
            aSel.Justify();
            nCursorPos = aSel.Min();
            aSel.Max() = ImplPatternRightPos( pEdit->GetText(), rEditMask, nFormatFlags, bSameMask, nCursorPos );
            if ( bShift )
                aSel.Min() = aOldSel.Min();
            else
                aSel.Min() = aSel.Max();
            pEdit->SetSelection( aSel );
            return true;
        }
        else if ( nKeyCode == KEY_HOME )
        {
            // Home is the position of the first non-literal character
            nNewPos = 0;
            while ( (nNewPos < rEditMask.getLength()) &&
                    (rEditMask[nNewPos] == EDITMASK_LITERAL) )
                nNewPos++;

            // Home should not move to the right
            if ( nCursorPos < nNewPos )
                nNewPos = nCursorPos;
            Selection aSel( nNewPos );
            if ( bShift )
                aSel.Min() = aOldSel.Min();
            pEdit->SetSelection( aSel );
            return true;
        }
        else if ( nKeyCode == KEY_END )
        {
            // End is position of last non-literal character
            nNewPos = rEditMask.getLength();
            while ( nNewPos &&
                    (rEditMask[nNewPos-1] == EDITMASK_LITERAL) )
                nNewPos--;
            // Use the start of selection as minimum; even a small position is allowed in case that
            // all was selected by the focus
            Selection aSel( aOldSel );
            aSel.Justify();
            nCursorPos = static_cast<sal_Int32>(aSel.Min());
            ImplPatternMaxPos( pEdit->GetText(), rEditMask, nFormatFlags, bSameMask, nCursorPos, nNewPos );
            aSel.Max() = nNewPos;
            if ( bShift )
                aSel.Min() = aOldSel.Min();
            else
                aSel.Min() = aSel.Max();
            pEdit->SetSelection( aSel );
            return true;
        }
        else if ( (nKeyCode == KEY_BACKSPACE) || (nKeyCode == KEY_DELETE) )
        {
            OUString          aOldStr( pEdit->GetText() );
            OUStringBuffer    aStr( aOldStr );
            Selection   aSel = aOldSel;

            aSel.Justify();
            nNewPos = static_cast<sal_Int32>(aSel.Min());

             // if selection then delete it
            if ( aSel.Len() )
            {
                if ( bSameMask )
                    aStr.remove( static_cast<sal_Int32>(aSel.Min()), static_cast<sal_Int32>(aSel.Len()) );
                else
                {
                    OUString aRep = rLiteralMask.copy( static_cast<sal_Int32>(aSel.Min()), static_cast<sal_Int32>(aSel.Len()) );
                    aStr.remove( aSel.Min(), aRep.getLength() );
                    aStr.insert( aSel.Min(), aRep );
                }
            }
            else
            {
                if ( nKeyCode == KEY_BACKSPACE )
                {
                    nTempPos = nNewPos;
                    nNewPos = ImplPatternLeftPos( rEditMask, nTempPos );
                }
                else
                    nTempPos = ImplPatternRightPos( aStr.toString(), rEditMask, nFormatFlags, bSameMask, nNewPos );

                if ( nNewPos != nTempPos )
                {
                    if ( bSameMask )
                    {
                        if ( rEditMask[nNewPos] != EDITMASK_LITERAL )
                            aStr.remove( nNewPos, 1 );
                    }
                    else
                    {
                        aStr[nNewPos] = rLiteralMask[nNewPos];
                    }
                }
            }

            if ( aOldStr != aStr.toString() )
            {
                if ( bSameMask )
                    aStr = ImplPatternReformat( aStr.toString(), rEditMask, rLiteralMask, nFormatFlags );
                rbInKeyInput = true;
                pEdit->SetText( aStr.toString(), Selection( nNewPos ) );
                pEdit->SetModifyFlag();
                pEdit->Modify();
                rbInKeyInput = false;
            }
            else
                pEdit->SetSelection( Selection( nNewPos ) );

            return true;
        }
        else if ( nKeyCode == KEY_INSERT )
        {
            // you can only set InsertModus for a PatternField if the
            // mask is equal at all input positions
            if ( !bSameMask )
            {
                return true;
            }
        }
    }

    if ( rKEvt.GetKeyCode().IsMod2() || (cChar < 32) || (cChar == 127) )
        return false;

    Selection aSel = aOldSel;
    aSel.Justify();
    nNewPos = aSel.Min();

    if ( nNewPos < rEditMask.getLength() )
    {
        sal_Unicode cPattChar = ImplPatternChar( cChar, rEditMask[nNewPos] );
        if ( cPattChar )
            cChar = cPattChar;
        else
        {
            // If no valid character, check if the user wanted to jump to next literal. We do this
            // only if we're after a character, so that literals that were skipped automatically
            // do not influence the position anymore.
            if ( nNewPos &&
                 (rEditMask[nNewPos-1] != EDITMASK_LITERAL) &&
                 !aSel.Len() )
            {
                // search for next character not being a literal
                nTempPos = nNewPos;
                while ( nTempPos < rEditMask.getLength() )
                {
                    if ( rEditMask[nTempPos] == EDITMASK_LITERAL )
                    {
                        // only valid if no literal present
                        if ( (rEditMask[nTempPos+1] != EDITMASK_LITERAL ) &&
                             ImplCommaPointCharEqual( cChar, rLiteralMask[nTempPos] ) )
                        {
                            nTempPos++;
                            ImplPatternMaxPos( pEdit->GetText(), rEditMask, nFormatFlags, bSameMask, nNewPos, nTempPos );
                            if ( nTempPos > nNewPos )
                            {
                                pEdit->SetSelection( Selection( nTempPos ) );
                                return true;
                            }
                        }
                        break;
                    }
                    nTempPos++;
                }
            }

            cChar = 0;
        }
    }
    else
        cChar = 0;
    if ( cChar )
    {
        OUStringBuffer  aStr = pEdit->GetText();
        bool        bError = false;
        if ( bSameMask && pEdit->IsInsertMode() )
        {
            // crop spaces and literals at the end until current position
            sal_Int32 n = aStr.getLength();
            while ( n && (n > nNewPos) )
            {
                if ( (aStr[n-1] != ' ') &&
                     ((n > rEditMask.getLength()) || (rEditMask[n-1] != EDITMASK_LITERAL)) )
                    break;

                n--;
            }
            aStr.truncate( n );

            if ( aSel.Len() )
                aStr.remove( aSel.Min(), aSel.Len() );

            if ( aStr.getLength() < rEditMask.getLength() )
            {
                // possibly extend string until cursor position
                if ( aStr.getLength() < nNewPos )
                    aStr.append( rLiteralMask.copy( aStr.getLength(), nNewPos-aStr.getLength() ));
                if ( nNewPos < aStr.getLength() )
                    aStr.insert( cChar, nNewPos );
                else if ( nNewPos < rEditMask.getLength() )
                    aStr.append(cChar);
                aStr = ImplPatternReformat( aStr.toString(), rEditMask, rLiteralMask, nFormatFlags );
            }
            else
                bError = true;
        }
        else
        {
            if ( aSel.Len() )
            {
                // delete selection
                OUString aRep = rLiteralMask.copy( aSel.Min(), aSel.Len() );
                aStr.remove( aSel.Min(), aRep.getLength() );
                aStr.insert( aSel.Min(), aRep );
            }

            if ( nNewPos < aStr.getLength() )
                aStr[nNewPos] = cChar;
            else if ( nNewPos < rEditMask.getLength() )
                aStr.append(cChar);
        }

        if ( !bError )
        {
            rbInKeyInput = true;
            Selection aNewSel( ImplPatternRightPos( aStr.toString(), rEditMask, nFormatFlags, bSameMask, nNewPos ) );
            pEdit->SetText( aStr.toString(), aNewSel );
            pEdit->SetModifyFlag();
            pEdit->Modify();
            rbInKeyInput = false;
        }
    }

    return true;
}

void PatternFormatter::ImplSetMask(const OString& rEditMask, const OUString& rLiteralMask)
{
    m_aEditMask     = rEditMask;
    maLiteralMask   = rLiteralMask;
    mbSameMask      = true;

    if ( m_aEditMask.getLength() != maLiteralMask.getLength() )
    {
        OUStringBuffer aBuf(maLiteralMask);
        if (m_aEditMask.getLength() < aBuf.getLength())
            aBuf.remove(m_aEditMask.getLength(), aBuf.getLength() - m_aEditMask.getLength());
        else
            comphelper::string::padToLength(aBuf, m_aEditMask.getLength(), ' ');
        maLiteralMask = aBuf.makeStringAndClear();
    }

    // Strict mode allows only the input mode if only equal characters are allowed as mask and if
    // only spaces are specified which are not allowed by the mask
    sal_Int32   i = 0;
    sal_Char    c = 0;
    while ( i < rEditMask.getLength() )
    {
        sal_Char cTemp = rEditMask[i];
        if ( cTemp != EDITMASK_LITERAL )
        {
            if ( (cTemp == EDITMASK_ALLCHAR) ||
                 (cTemp == EDITMASK_UPPERALLCHAR) ||
                 (cTemp == EDITMASK_NUMSPACE) )
            {
                mbSameMask = false;
                break;
            }
            if ( i < rLiteralMask.getLength() )
            {
                if ( rLiteralMask[i] != ' ' )
                {
                    mbSameMask = false;
                    break;
                }
            }
            if ( !c )
                c = cTemp;
            if ( cTemp != c )
            {
                mbSameMask = false;
                break;
            }
        }
        i++;
    }
}

PatternFormatter::PatternFormatter()
{
    mnFormatFlags       = 0;
    mbSameMask          = true;
    mbInPattKeyInput    = false;
}

PatternFormatter::~PatternFormatter()
{
}

void PatternFormatter::SetMask( const OString& rEditMask,
                                const OUString& rLiteralMask )
{
    ImplSetMask( rEditMask, rLiteralMask );
    ReformatAll();
}

void PatternFormatter::SetString( const OUString& rStr )
{
    maFieldString = rStr;
    if ( GetField() )
    {
        GetField()->SetText( rStr );
        MarkToBeReformatted( false );
    }
}

OUString PatternFormatter::GetString() const
{
    if ( !GetField() )
        return OUString();
    else
        return ImplPatternReformat( GetField()->GetText(), m_aEditMask, maLiteralMask, mnFormatFlags );
}

void PatternFormatter::Reformat()
{
    if ( GetField() )
    {
        ImplSetText( ImplPatternReformat( GetField()->GetText(), m_aEditMask, maLiteralMask, mnFormatFlags ) );
        if ( !mbSameMask && IsStrictFormat() && !GetField()->IsReadOnly() )
            GetField()->SetInsertMode( false );
    }
}

PatternField::PatternField( vcl::Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

void PatternField::dispose()
{
    PatternFormatter::SetField( nullptr );
    SpinField::dispose();
}

bool PatternField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplPatternProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), GetEditMask(), GetLiteralMask(),
                                         IsStrictFormat(), GetFormatFlags(),
                                         ImplIsSameMask(), ImplGetInPattKeyInput() ) )
            return true;
    }

    return SpinField::PreNotify( rNEvt );
}

bool PatternField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return SpinField::Notify( rNEvt );
}

void PatternField::Modify()
{
    if ( !ImplGetInPattKeyInput() )
    {
        if ( IsStrictFormat() )
            ImplPatternProcessStrictModify( GetField(), GetEditMask(), GetLiteralMask(), GetFormatFlags(), ImplIsSameMask() );
        else
            MarkToBeReformatted( true );
    }

    SpinField::Modify();
}

PatternBox::PatternBox( vcl::Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

void PatternBox::dispose()
{
    PatternFormatter::SetField( nullptr );
    ComboBox::dispose();
}

bool PatternBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplPatternProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), GetEditMask(), GetLiteralMask(),
                                         IsStrictFormat(), GetFormatFlags(),
                                         ImplIsSameMask(), ImplGetInPattKeyInput() ) )
            return true;
    }

    return ComboBox::PreNotify( rNEvt );
}

bool PatternBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::Notify( rNEvt );
}

void PatternBox::Modify()
{
    if ( !ImplGetInPattKeyInput() )
    {
        if ( IsStrictFormat() )
            ImplPatternProcessStrictModify( GetField(), GetEditMask(), GetLiteralMask(), GetFormatFlags(), ImplIsSameMask() );
        else
            MarkToBeReformatted( true );
    }

    ComboBox::Modify();
}

void PatternBox::ReformatAll()
{
    OUString aStr;
    SetUpdateMode( false );
    const sal_Int32 nEntryCount = GetEntryCount();
    for ( sal_Int32 i=0; i < nEntryCount; ++i )
    {
        aStr = ImplPatternReformat( GetEntry( i ), GetEditMask(), GetLiteralMask(), GetFormatFlags() );
        RemoveEntryAt(i);
        InsertEntry( aStr, i );
    }
    PatternFormatter::Reformat();
    SetUpdateMode( true );
}

static ExtDateFieldFormat ImplGetExtFormat( DateFormat eOld )
{
    switch( eOld )
    {
        case DMY:   return XTDATEF_SHORT_DDMMYY;
        case MDY:   return XTDATEF_SHORT_MMDDYY;
        default:    return XTDATEF_SHORT_YYMMDD;
    }
}

static sal_uInt16 ImplCutNumberFromString( OUString& rStr )
{
    sal_Int32 i1 = 0;
    while (i1 != rStr.getLength() && !(rStr[i1] >= '0' && rStr[i1] <= '9')) {
        ++i1;
    }
    sal_Int32 i2 = i1;
    while (i2 != rStr.getLength() && rStr[i2] >= '0' && rStr[i2] <= '9') {
        ++i2;
    }
    sal_Int32 nValue = rStr.copy(i1, i2-i1).toInt32();
    rStr = rStr.copy(std::min(i2+1, rStr.getLength()));
    return nValue;
}

static bool ImplCutMonthName( OUString& rStr, const OUString& _rLookupMonthName )
{
    sal_Int32 index = 0;
    rStr = rStr.replaceFirst(_rLookupMonthName, "", &index);
    return index >= 0;
}

static sal_uInt16 ImplCutMonthFromString( OUString& rStr, const CalendarWrapper& rCalendarWrapper )
{
    // search for a month' name
    for ( sal_uInt16 i=1; i <= 12; i++ )
    {
        OUString aMonthName = rCalendarWrapper.getMonths()[i-1].FullName;
        // long month name?
        if ( ImplCutMonthName( rStr, aMonthName ) )
            return i;

        // short month name?
        OUString aAbbrevMonthName = rCalendarWrapper.getMonths()[i-1].AbbrevName;
        if ( ImplCutMonthName( rStr, aAbbrevMonthName ) )
            return i;
    }

    return ImplCutNumberFromString( rStr );
}

static OUString ImplGetDateSep( const LocaleDataWrapper& rLocaleDataWrapper, ExtDateFieldFormat eFormat )
{
    if ( ( eFormat == XTDATEF_SHORT_YYMMDD_DIN5008 ) || ( eFormat == XTDATEF_SHORT_YYYYMMDD_DIN5008 ) )
        return OUString("-");
    else
        return rLocaleDataWrapper.getDateSep();
}

static bool ImplDateProcessKeyInput( Edit*, const KeyEvent& rKEvt, ExtDateFieldFormat eFormat,
                                     const LocaleDataWrapper& rLocaleDataWrapper  )
{
    sal_Unicode cChar = rKEvt.GetCharCode();
    sal_uInt16 nGroup = rKEvt.GetKeyCode().GetGroup();
    if ( (nGroup == KEYGROUP_FKEYS) || (nGroup == KEYGROUP_CURSOR) ||
         (nGroup == KEYGROUP_MISC)||
         ((cChar >= '0') && (cChar <= '9')) ||
         (cChar == ImplGetDateSep( rLocaleDataWrapper, eFormat )[0]) )
        return false;
    else
        return true;
}

static bool ImplDateGetValue( const OUString& rStr, Date& rDate, ExtDateFieldFormat eDateFormat,
                              const LocaleDataWrapper& rLocaleDataWrapper, const CalendarWrapper& rCalendarWrapper,
                              const AllSettings& )
{
    sal_uInt16 nDay = 0;
    sal_uInt16 nMonth = 0;
    sal_uInt16 nYear = 0;
    bool bError = false;
    OUString aStr( rStr );

    if ( eDateFormat == XTDATEF_SYSTEM_LONG )
    {
        DateFormat eFormat = rLocaleDataWrapper.getLongDateFormat();
        switch( eFormat )
        {
            case MDY:
                nMonth = ImplCutMonthFromString( aStr, rCalendarWrapper );
                nDay = ImplCutNumberFromString( aStr );
                nYear  = ImplCutNumberFromString( aStr );
                break;
            case DMY:
                nDay = ImplCutNumberFromString( aStr );
                nMonth = ImplCutMonthFromString( aStr, rCalendarWrapper );
                nYear  = ImplCutNumberFromString( aStr );
                break;
            case YMD:
            default:
                nYear = ImplCutNumberFromString( aStr );
                nMonth = ImplCutMonthFromString( aStr, rCalendarWrapper );
                nDay  = ImplCutNumberFromString( aStr );
                break;
        }
    }
    else
    {
        bool bYear = true;

        // Check if year is present:
        OUString aDateSep = ImplGetDateSep( rLocaleDataWrapper, eDateFormat );
        sal_Int32 nSepPos = aStr.indexOf( aDateSep );
        if ( nSepPos < 0 )
            return false;
        nSepPos = aStr.indexOf( aDateSep, nSepPos+1 );
        if ( ( nSepPos < 0 ) || ( nSepPos == (aStr.getLength()-1) ) )
        {
            bYear = false;
            nYear = Date( Date::SYSTEM ).GetYear();
        }

        const sal_Unicode* pBuf = aStr.getStr();
        ImplSkipDelimiters( pBuf );

        switch ( eDateFormat )
        {
            case XTDATEF_SHORT_DDMMYY:
            case XTDATEF_SHORT_DDMMYYYY:
            {
                nDay = ImplGetNum( pBuf, bError );
                ImplSkipDelimiters( pBuf );
                nMonth = ImplGetNum( pBuf, bError );
                ImplSkipDelimiters( pBuf );
                if ( bYear )
                    nYear = ImplGetNum( pBuf, bError );
            }
            break;
            case XTDATEF_SHORT_MMDDYY:
            case XTDATEF_SHORT_MMDDYYYY:
            {
                nMonth = ImplGetNum( pBuf, bError );
                ImplSkipDelimiters( pBuf );
                nDay = ImplGetNum( pBuf, bError );
                ImplSkipDelimiters( pBuf );
                if ( bYear )
                    nYear = ImplGetNum( pBuf, bError );
            }
            break;
            case XTDATEF_SHORT_YYMMDD:
            case XTDATEF_SHORT_YYYYMMDD:
            case XTDATEF_SHORT_YYMMDD_DIN5008:
            case XTDATEF_SHORT_YYYYMMDD_DIN5008:
            {
                if ( bYear )
                    nYear = ImplGetNum( pBuf, bError );
                ImplSkipDelimiters( pBuf );
                nMonth = ImplGetNum( pBuf, bError );
                ImplSkipDelimiters( pBuf );
                nDay = ImplGetNum( pBuf, bError );
            }
            break;

            default:
            {
                OSL_FAIL( "DateFormat???" );
            }
        }
    }

    if ( bError || !nDay || !nMonth )
        return false;

    Date aNewDate( nDay, nMonth, nYear );
    DateFormatter::ExpandCentury( aNewDate, utl::MiscCfg().GetYear2000() );
    if ( aNewDate.IsValidDate() )
    {
        rDate = aNewDate;
        return true;
    }
    return false;
}

bool DateFormatter::ImplDateReformat( const OUString& rStr, OUString& rOutStr, const AllSettings& rSettings )
{
    Date aDate( 0, 0, 0 );
    if ( !ImplDateGetValue( rStr, aDate, GetExtDateFormat(true), ImplGetLocaleDataWrapper(), GetCalendarWrapper(), GetFieldSettings() ) )
        return true;

    Date aTempDate = aDate;
    if ( aTempDate > GetMax() )
        aTempDate = GetMax();
    else if ( aTempDate < GetMin() )
        aTempDate = GetMin();

    rOutStr = ImplGetDateAsText( aTempDate, rSettings );

    return true;
}

OUString DateFormatter::ImplGetDateAsText( const Date& rDate,
                                           const AllSettings& ) const
{
    bool bShowCentury = false;
    switch ( GetExtDateFormat() )
    {
        case XTDATEF_SYSTEM_SHORT_YYYY:
        case XTDATEF_SYSTEM_LONG:
        case XTDATEF_SHORT_DDMMYYYY:
        case XTDATEF_SHORT_MMDDYYYY:
        case XTDATEF_SHORT_YYYYMMDD:
        case XTDATEF_SHORT_YYYYMMDD_DIN5008:
        {
            bShowCentury = true;
        }
        break;
        default:
        {
            bShowCentury = false;
        }
    }

    if ( !bShowCentury )
    {
        // Check if I have to use force showing the century
        sal_uInt16 nTwoDigitYearStart = utl::MiscCfg().GetYear2000();
        sal_uInt16 nYear = rDate.GetYear();

        // If year is not in double digit range
        if ( (nYear < nTwoDigitYearStart) || (nYear >= nTwoDigitYearStart+100) )
            bShowCentury = true;
    }

    sal_Unicode aBuf[128];
    sal_Unicode* pBuf = aBuf;

    OUString aDateSep = ImplGetDateSep( ImplGetLocaleDataWrapper(), GetExtDateFormat( true ) );
    sal_uInt16 nDay = rDate.GetDay();
    sal_uInt16 nMonth = rDate.GetMonth();
    sal_uInt16 nYear = rDate.GetYear();
    sal_uInt16 nYearLen = bShowCentury ? 4 : 2;

    if ( !bShowCentury )
        nYear %= 100;

    switch ( GetExtDateFormat( true ) )
    {
        case XTDATEF_SYSTEM_LONG:
        {
            return ImplGetLocaleDataWrapper().getLongDate( rDate, GetCalendarWrapper(), !bShowCentury );
        }
        case XTDATEF_SHORT_DDMMYY:
        case XTDATEF_SHORT_DDMMYYYY:
        {
            pBuf = ImplAddNum( pBuf, nDay, 2 );
            pBuf = ImplAddString( pBuf, aDateSep );
            pBuf = ImplAddNum( pBuf, nMonth, 2 );
            pBuf = ImplAddString( pBuf, aDateSep );
            pBuf = ImplAddNum( pBuf, nYear, nYearLen );
        }
        break;
        case XTDATEF_SHORT_MMDDYY:
        case XTDATEF_SHORT_MMDDYYYY:
        {
            pBuf = ImplAddNum( pBuf, nMonth, 2 );
            pBuf = ImplAddString( pBuf, aDateSep );
            pBuf = ImplAddNum( pBuf, nDay, 2 );
            pBuf = ImplAddString( pBuf, aDateSep );
            pBuf = ImplAddNum( pBuf, nYear, nYearLen );
        }
        break;
        case XTDATEF_SHORT_YYMMDD:
        case XTDATEF_SHORT_YYYYMMDD:
        case XTDATEF_SHORT_YYMMDD_DIN5008:
        case XTDATEF_SHORT_YYYYMMDD_DIN5008:
        {
            pBuf = ImplAddNum( pBuf, nYear, nYearLen );
            pBuf = ImplAddString( pBuf, aDateSep );
            pBuf = ImplAddNum( pBuf, nMonth, 2 );
            pBuf = ImplAddString( pBuf, aDateSep );
            pBuf = ImplAddNum( pBuf, nDay, 2 );
        }
        break;
        default:
        {
            OSL_FAIL( "DateFormat???" );
        }
    }

    return OUString(aBuf, pBuf-aBuf);
}

static void ImplDateIncrementDay( Date& rDate, bool bUp )
{
    DateFormatter::ExpandCentury( rDate );

    if ( bUp )
    {
        if ( (rDate.GetDay() != 31) || (rDate.GetMonth() != 12) || (rDate.GetYear() != 9999) )
            ++rDate;
    }
    else
    {
        if ( (rDate.GetDay() != 1 ) || (rDate.GetMonth() != 1) || (rDate.GetYear() != 0) )
            --rDate;
    }
}

static void ImplDateIncrementMonth( Date& rDate, bool bUp )
{
    DateFormatter::ExpandCentury( rDate );

    sal_uInt16 nMonth = rDate.GetMonth();
    sal_uInt16 nYear = rDate.GetYear();
    if ( bUp )
    {
        if ( (nMonth == 12) && (nYear < 9999) )
        {
            rDate.SetMonth( 1 );
            rDate.SetYear( nYear + 1 );
        }
        else
        {
            if ( nMonth < 12 )
                rDate.SetMonth( nMonth + 1 );
        }
    }
    else
    {
        if ( (nMonth == 1) && (nYear > 0) )
        {
            rDate.SetMonth( 12 );
            rDate.SetYear( nYear - 1 );
        }
        else
        {
            if ( nMonth > 1 )
                rDate.SetMonth( nMonth - 1 );
        }
    }

    sal_uInt16 nDaysInMonth = Date::GetDaysInMonth( rDate.GetMonth(), rDate.GetYear());
    if ( rDate.GetDay() > nDaysInMonth )
        rDate.SetDay( nDaysInMonth );
}

static void ImplDateIncrementYear( Date& rDate, bool bUp )
{
    DateFormatter::ExpandCentury( rDate );

    sal_uInt16 nYear = rDate.GetYear();
    sal_uInt16 nMonth = rDate.GetMonth();
    if ( bUp )
    {
        if ( nYear < 9999 )
            rDate.SetYear( nYear + 1 );
    }
    else
    {
        if ( nYear > 0 )
            rDate.SetYear( nYear - 1 );
    }
    if (nMonth == 2)
    {
        // Handle February 29 from leap year to non-leap year.
        sal_uInt16 nDay = rDate.GetDay();
        if (nDay > 28)
        {
            // The check would not be necessary if it was guaranteed that the
            // date was valid before and actually was a leap year,
            // de-/incrementing a leap year with 29 always results in 28.
            sal_uInt16 nDaysInMonth = Date::GetDaysInMonth( nMonth, rDate.GetYear());
            if (nDay > nDaysInMonth)
                rDate.SetDay( nDaysInMonth);
        }
    }
}

bool DateFormatter::ImplAllowMalformedInput() const
{
    return !IsEnforceValidValue();
}

void DateField::ImplDateSpinArea( bool bUp )
{
    // increment days if all is selected
    if ( GetField() )
    {
        Date aDate( GetDate() );
        Selection aSelection = GetField()->GetSelection();
        aSelection.Justify();
        OUString aText( GetText() );
        if ( (sal_Int32)aSelection.Len() == aText.getLength() )
            ImplDateIncrementDay( aDate, bUp );
        else
        {
            sal_Int8 nDateArea = 0;

            ExtDateFieldFormat eFormat = GetExtDateFormat( true );
            if ( eFormat == XTDATEF_SYSTEM_LONG )
            {
                eFormat = ImplGetExtFormat( ImplGetLocaleDataWrapper().getLongDateFormat() );
                nDateArea = 1;
            }
            else
            {
                // search area
                sal_Int32 nPos = 0;
                OUString aDateSep = ImplGetDateSep( ImplGetLocaleDataWrapper(), eFormat );
                for ( sal_Int8 i = 1; i <= 3; i++ )
                {
                    nPos = aText.indexOf( aDateSep, nPos );
                    if (nPos < 0 || nPos >= (sal_Int32)aSelection.Max())
                    {
                        nDateArea = i;
                        break;
                    }
                    else
                        nPos++;
                }
            }

            switch( eFormat )
            {
                case XTDATEF_SHORT_MMDDYY:
                case XTDATEF_SHORT_MMDDYYYY:
                switch( nDateArea )
                {
                    case 1: ImplDateIncrementMonth( aDate, bUp );
                            break;
                    case 2: ImplDateIncrementDay( aDate, bUp );
                            break;
                    case 3: ImplDateIncrementYear( aDate, bUp );
                            break;
                }
                break;
                case XTDATEF_SHORT_DDMMYY:
                case XTDATEF_SHORT_DDMMYYYY:
                switch( nDateArea )
                {
                    case 1: ImplDateIncrementDay( aDate, bUp );
                            break;
                    case 2: ImplDateIncrementMonth( aDate, bUp );
                            break;
                    case 3: ImplDateIncrementYear( aDate, bUp );
                            break;
                }
                break;
                case XTDATEF_SHORT_YYMMDD:
                case XTDATEF_SHORT_YYYYMMDD:
                case XTDATEF_SHORT_YYMMDD_DIN5008:
                case XTDATEF_SHORT_YYYYMMDD_DIN5008:
                switch( nDateArea )
                {
                    case 1: ImplDateIncrementYear( aDate, bUp );
                            break;
                    case 2: ImplDateIncrementMonth( aDate, bUp );
                            break;
                    case 3: ImplDateIncrementDay( aDate, bUp );
                            break;
                }
                break;
                default:
                    OSL_FAIL( "invalid conversion" );
                    break;
            }
        }

        ImplNewFieldValue( aDate );
    }
}

void DateFormatter::ImplInit()
{
    mbLongFormat        = false;
    mbShowDateCentury   = true;
    mpCalendarWrapper   = nullptr;
    mnDateFormat        = 0xFFFF;
    mnExtDateFormat     = XTDATEF_SYSTEM_SHORT;
}

DateFormatter::DateFormatter() :
    maFieldDate( 0 ),
    maLastDate( 0 ),
    maMin( 1, 1, 1900 ),
    maMax( 31, 12, 2200 ),
    mbEnforceValidValue( true )
{
    ImplInit();
}

DateFormatter::~DateFormatter()
{
    delete mpCalendarWrapper;
    mpCalendarWrapper = nullptr;
}

void DateFormatter::SetLocale( const css::lang::Locale& rLocale )
{
    delete mpCalendarWrapper;
    mpCalendarWrapper = nullptr;
    FormatterBase::SetLocale( rLocale );
}

CalendarWrapper& DateFormatter::GetCalendarWrapper() const
{
    if ( !mpCalendarWrapper )
    {
        const_cast<DateFormatter*>(this)->mpCalendarWrapper = new CalendarWrapper( comphelper::getProcessComponentContext() );
        mpCalendarWrapper->loadDefaultCalendar( GetLocale() );
    }

    return *mpCalendarWrapper;
}

void DateFormatter::SetExtDateFormat( ExtDateFieldFormat eFormat )
{
    mnExtDateFormat = eFormat;
    ReformatAll();
}

ExtDateFieldFormat DateFormatter::GetExtDateFormat( bool bResolveSystemFormat ) const
{
    ExtDateFieldFormat eDateFormat = (ExtDateFieldFormat)mnExtDateFormat;

    if ( bResolveSystemFormat && ( eDateFormat <= XTDATEF_SYSTEM_SHORT_YYYY ) )
    {
        bool bShowCentury = (eDateFormat == XTDATEF_SYSTEM_SHORT_YYYY);
        switch ( ImplGetLocaleDataWrapper().getDateFormat() )
        {
            case DMY:   eDateFormat = bShowCentury ? XTDATEF_SHORT_DDMMYYYY : XTDATEF_SHORT_DDMMYY;
                        break;
            case MDY:   eDateFormat = bShowCentury ? XTDATEF_SHORT_MMDDYYYY : XTDATEF_SHORT_MMDDYY;
                        break;
            default:    eDateFormat = bShowCentury ? XTDATEF_SHORT_YYYYMMDD : XTDATEF_SHORT_YYMMDD;

        }
    }

    return eDateFormat;
}

void DateFormatter::ReformatAll()
{
    Reformat();
}

void DateFormatter::SetMin( const Date& rNewMin )
{
    maMin = rNewMin;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

void DateFormatter::SetMax( const Date& rNewMax )
{
    maMax = rNewMax;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

void DateFormatter::SetLongFormat( bool bLong )
{
    mbLongFormat = bLong;

    // #91913# Remove LongFormat and DateShowCentury - redundant
    if ( bLong )
    {
        SetExtDateFormat( XTDATEF_SYSTEM_LONG );
    }
    else
    {
        if( mnExtDateFormat == XTDATEF_SYSTEM_LONG )
            SetExtDateFormat( XTDATEF_SYSTEM_SHORT );
    }

    ReformatAll();
}

void DateFormatter::SetShowDateCentury( bool bShowDateCentury )
{
    mbShowDateCentury = bShowDateCentury;

    // #91913# Remove LongFormat and DateShowCentury - redundant
    if ( bShowDateCentury )
    {
        switch ( GetExtDateFormat() )
        {
            case XTDATEF_SYSTEM_SHORT:
            case XTDATEF_SYSTEM_SHORT_YY:
                SetExtDateFormat( XTDATEF_SYSTEM_SHORT_YYYY );  break;
            case XTDATEF_SHORT_DDMMYY:
                SetExtDateFormat( XTDATEF_SHORT_DDMMYYYY );     break;
            case XTDATEF_SHORT_MMDDYY:
                SetExtDateFormat( XTDATEF_SHORT_MMDDYYYY );     break;
            case XTDATEF_SHORT_YYMMDD:
                SetExtDateFormat( XTDATEF_SHORT_YYYYMMDD );     break;
            case XTDATEF_SHORT_YYMMDD_DIN5008:
                SetExtDateFormat( XTDATEF_SHORT_YYYYMMDD_DIN5008 ); break;
            default:
                ;
        }
    }
    else
    {
        switch ( GetExtDateFormat() )
        {
            case XTDATEF_SYSTEM_SHORT:
            case XTDATEF_SYSTEM_SHORT_YYYY:
                SetExtDateFormat( XTDATEF_SYSTEM_SHORT_YY );    break;
            case XTDATEF_SHORT_DDMMYYYY:
                SetExtDateFormat( XTDATEF_SHORT_DDMMYY );       break;
            case XTDATEF_SHORT_MMDDYYYY:
                SetExtDateFormat( XTDATEF_SHORT_MMDDYY );       break;
            case XTDATEF_SHORT_YYYYMMDD:
                SetExtDateFormat( XTDATEF_SHORT_YYMMDD );       break;
            case XTDATEF_SHORT_YYYYMMDD_DIN5008:
                SetExtDateFormat( XTDATEF_SHORT_YYMMDD_DIN5008 );  break;
            default:
                ;
        }
    }

    ReformatAll();
}

void DateFormatter::SetDate( const Date& rNewDate )
{
    SetUserDate( rNewDate );
    maFieldDate = maLastDate;
    maLastDate = GetDate();
}

void DateFormatter::SetUserDate( const Date& rNewDate )
{
    ImplSetUserDate( rNewDate );
}

void DateFormatter::ImplSetUserDate( const Date& rNewDate, Selection* pNewSelection )
{
    Date aNewDate = rNewDate;
    if ( aNewDate > maMax )
        aNewDate = maMax;
    else if ( aNewDate < maMin )
        aNewDate = maMin;
    maLastDate = aNewDate;

    if ( GetField() )
        ImplSetText( ImplGetDateAsText( aNewDate, GetFieldSettings() ), pNewSelection );
}

void DateFormatter::ImplNewFieldValue( const Date& rDate )
{
    if ( GetField() )
    {
        Selection aSelection = GetField()->GetSelection();
        aSelection.Justify();
        OUString aText = GetField()->GetText();

        // If selected until the end then keep it that way
        if ( (sal_Int32)aSelection.Max() == aText.getLength() )
        {
            if ( !aSelection.Len() )
                aSelection.Min() = SELECTION_MAX;
            aSelection.Max() = SELECTION_MAX;
        }

        Date aOldLastDate  = maLastDate;
        ImplSetUserDate( rDate, &aSelection );
        maLastDate = aOldLastDate;

        // Modify at Edit is only set at KeyInput
        if ( GetField()->GetText() != aText )
        {
            GetField()->SetModifyFlag();
            GetField()->Modify();
        }
    }
}

Date DateFormatter::GetDate() const
{
    Date aDate( 0, 0, 0 );

    if ( GetField() )
    {
        if ( ImplDateGetValue( GetField()->GetText(), aDate, GetExtDateFormat(true), ImplGetLocaleDataWrapper(), GetCalendarWrapper(), GetFieldSettings() ) )
        {
            if ( aDate > maMax )
                aDate = maMax;
            else if ( aDate < maMin )
                aDate = maMin;
        }
        else
        {
            // !!! We should find out why dates are treated differently than other fields (see
            // also bug: 52384)

            if ( !ImplAllowMalformedInput() )
            {
                if ( maLastDate.GetDate() )
                    aDate = maLastDate;
                else if ( !IsEmptyFieldValueEnabled() )
                    aDate = Date( Date::SYSTEM );
            }
            else
                aDate = GetInvalidDate();
        }
    }

    return aDate;
}

void DateFormatter::SetEmptyDate()
{
    FormatterBase::SetEmptyFieldValue();
}

bool DateFormatter::IsEmptyDate() const
{
    bool bEmpty = FormatterBase::IsEmptyFieldValue();

    if ( GetField() && MustBeReformatted() && IsEmptyFieldValueEnabled() )
    {
        if ( GetField()->GetText().isEmpty() )
        {
            bEmpty = true;
        }
        else if ( !maLastDate.GetDate() )
        {
            Date aDate( Date::EMPTY );
            bEmpty = !ImplDateGetValue( GetField()->GetText(), aDate, GetExtDateFormat(true), ImplGetLocaleDataWrapper(), GetCalendarWrapper(), GetFieldSettings() );
        }
    }
    return bEmpty;
}

void DateFormatter::Reformat()
{
    if ( !GetField() )
        return;

    if ( GetField()->GetText().isEmpty() && ImplGetEmptyFieldValue() )
        return;

    OUString aStr;
    bool bOK = ImplDateReformat( GetField()->GetText(), aStr, GetFieldSettings() );
    if( !bOK )
        return;

    if ( !aStr.isEmpty() )
    {
        ImplSetText( aStr );
        ImplDateGetValue( aStr, maLastDate, GetExtDateFormat(true), ImplGetLocaleDataWrapper(), GetCalendarWrapper(), GetFieldSettings() );
    }
    else
    {
        if ( maLastDate.GetDate() )
            SetDate( maLastDate );
        else if ( !IsEmptyFieldValueEnabled() )
            SetDate( Date( Date::SYSTEM ) );
        else
        {
            ImplSetText( OUString() );
            SetEmptyFieldValueData( true );
        }
    }
}

void DateFormatter::ExpandCentury( Date& rDate )
{
    ExpandCentury( rDate, utl::MiscCfg().GetYear2000() );
}

void DateFormatter::ExpandCentury( Date& rDate, sal_uInt16 nTwoDigitYearStart )
{
    sal_uInt16 nDateYear = rDate.GetYear();
    if ( nDateYear < 100 )
    {
        sal_uInt16 nCentury = nTwoDigitYearStart / 100;
        if ( nDateYear < (nTwoDigitYearStart % 100) )
            nCentury++;
        rDate.SetYear( nDateYear + (nCentury*100) );
    }
}

DateField::DateField( vcl::Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle ),
    maFirst( GetMin() ),
    maLast( GetMax() )
{
    SetField( this );
    SetText( ImplGetLocaleDataWrapper().getDate( ImplGetFieldDate() ) );
    Reformat();
    ResetLastDate();
}

void DateField::dispose()
{
    DateFormatter::SetField( nullptr );
    SpinField::dispose();
}

bool DateField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && IsStrictFormat() &&
         ( GetExtDateFormat() != XTDATEF_SYSTEM_LONG ) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplDateProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), GetExtDateFormat( true ), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return SpinField::PreNotify( rNEvt );
}

bool DateField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() )
        {
            // !!! We should find out why dates are treated differently than other fields (see
            // also bug: 52384)

            bool bTextLen = !GetText().isEmpty();
            if ( bTextLen || !IsEmptyFieldValueEnabled() )
            {
                if ( !ImplAllowMalformedInput() )
                    Reformat();
                else
                {
                    Date aDate( 0, 0, 0 );
                    if ( ImplDateGetValue( GetText(), aDate, GetExtDateFormat(true), ImplGetLocaleDataWrapper(), GetCalendarWrapper(), GetFieldSettings() ) )
                        // even with strict text analysis, our text is a valid date -> do a complete
                        // reformat
                        Reformat();
                }
            }
            else if ( !bTextLen && IsEmptyFieldValueEnabled() )
            {
                ResetLastDate();
                SetEmptyFieldValueData( true );
            }
        }
    }

    return SpinField::Notify( rNEvt );
}

void DateField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & (AllSettingsFlags::LOCALE|AllSettingsFlags::MISC)) )
    {
        if ( IsDefaultLocale() && ( rDCEvt.GetFlags() & AllSettingsFlags::LOCALE ) )
            ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        ReformatAll();
    }
}

void DateField::Modify()
{
    MarkToBeReformatted( true );
    SpinField::Modify();
}

void DateField::Up()
{
    ImplDateSpinArea( true );
    SpinField::Up();
}

void DateField::Down()
{
    ImplDateSpinArea( false );
    SpinField::Down();
}

void DateField::First()
{
    ImplNewFieldValue( maFirst );
    SpinField::First();
}

void DateField::Last()
{
    ImplNewFieldValue( maLast );
    SpinField::Last();
}

DateBox::DateBox( vcl::Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    SetText( ImplGetLocaleDataWrapper().getDate( ImplGetFieldDate() ) );
    Reformat();
}

void DateBox::dispose()
{
    DateFormatter::SetField( nullptr );
    ComboBox::dispose();
}

bool DateBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && IsStrictFormat() &&
         ( GetExtDateFormat() != XTDATEF_SYSTEM_LONG ) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplDateProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), GetExtDateFormat( true ), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return ComboBox::PreNotify( rNEvt );
}

void DateBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::LOCALE) )
    {
        if ( IsDefaultLocale() )
            ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        ReformatAll();
    }
}

bool DateBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() )
        {
            bool bTextLen = !GetText().isEmpty();
            if ( bTextLen || !IsEmptyFieldValueEnabled() )
                Reformat();
            else if ( !bTextLen && IsEmptyFieldValueEnabled() )
            {
                ResetLastDate();
                SetEmptyFieldValueData( true );
            }
        }
    }

    return ComboBox::Notify( rNEvt );
}

void DateBox::Modify()
{
    MarkToBeReformatted( true );
    ComboBox::Modify();
}

void DateBox::ReformatAll()
{
    OUString aStr;
    SetUpdateMode( false );
    const sal_Int32 nEntryCount = GetEntryCount();
    for ( sal_Int32 i=0; i < nEntryCount; ++i )
    {
        ImplDateReformat( GetEntry( i ), aStr, GetFieldSettings() );
        RemoveEntryAt(i);
        InsertEntry( aStr, i );
    }
    DateFormatter::Reformat();
    SetUpdateMode( true );
}

static bool ImplTimeProcessKeyInput( Edit*, const KeyEvent& rKEvt,
                                     bool bStrictFormat, bool bDuration,
                                     TimeFieldFormat eFormat,
                                     const LocaleDataWrapper& rLocaleDataWrapper  )
{
    sal_Unicode cChar = rKEvt.GetCharCode();

    if ( !bStrictFormat )
        return false;
    else
    {
        sal_uInt16 nGroup = rKEvt.GetKeyCode().GetGroup();
        if ( (nGroup == KEYGROUP_FKEYS) || (nGroup == KEYGROUP_CURSOR) ||
             (nGroup == KEYGROUP_MISC)   ||
             ((cChar >= '0') && (cChar <= '9')) ||
             string::equals(rLocaleDataWrapper.getTimeSep(), cChar) ||
             (rLocaleDataWrapper.getTimeAM().indexOf(cChar) != -1) ||
             (rLocaleDataWrapper.getTimePM().indexOf(cChar) != -1) ||
             // Accept AM/PM:
             (cChar == 'a') || (cChar == 'A') || (cChar == 'm') || (cChar == 'M') || (cChar == 'p') || (cChar == 'P') ||
             ((eFormat == TimeFieldFormat::F_100TH_SEC) && string::equals(rLocaleDataWrapper.getTime100SecSep(), cChar)) ||
             ((eFormat == TimeFieldFormat::F_SEC_CS) && string::equals(rLocaleDataWrapper.getTime100SecSep(), cChar)) ||
             (bDuration && (cChar == '-')) )
            return false;
        else
            return true;
    }
}

static bool ImplIsOnlyDigits( const OUStringBuffer& _rStr )
{
    const sal_Unicode* _pChr = _rStr.getStr();
    for ( sal_Int32 i = 0; i < _rStr.getLength(); ++i, ++_pChr )
    {
        if ( *_pChr < '0' || *_pChr > '9' )
            return false;
    }
    return true;
}

static bool ImplIsValidTimePortion( bool _bSkipInvalidCharacters, const OUStringBuffer& _rStr )
{
    if ( !_bSkipInvalidCharacters )
    {
        if ( ( _rStr.getLength() > 2 ) || _rStr.isEmpty() || !ImplIsOnlyDigits( _rStr ) )
            return false;
    }
    return true;
}

static bool ImplCutTimePortion( OUStringBuffer& _rStr, sal_Int32 _nSepPos, bool _bSkipInvalidCharacters, short* _pPortion )
{
    OUString sPortion(_rStr.getStr(), _nSepPos );
    _rStr = _nSepPos < _rStr.getLength()
        ? _rStr.copy( _nSepPos + 1 ) : OUStringBuffer();

    if ( !ImplIsValidTimePortion( _bSkipInvalidCharacters, sPortion ) )
        return false;
    *_pPortion = (short)sPortion.toInt32();
    return true;
}

static bool ImplTimeGetValue( const OUString& rStr, tools::Time& rTime,
                              TimeFieldFormat eFormat, bool bDuration,
                              const LocaleDataWrapper& rLocaleDataWrapper, bool _bSkipInvalidCharacters = true )
{
    OUStringBuffer    aStr    = rStr;
    short       nHour   = 0;
    short       nMinute = 0;
    short       nSecond = 0;
    sal_Int64   nNanoSec = 0;
    tools::Time        aTime( 0, 0, 0 );

    if ( rStr.isEmpty() )
        return false;

    // Search for separators
    if (!rLocaleDataWrapper.getTimeSep().isEmpty())
    {
        OUStringBuffer aSepStr(",.;:/");
        if ( !bDuration )
            aSepStr.append('-');

        // Replace characters above by the separator character
        for (sal_Int32 i = 0; i < aSepStr.getLength(); ++i)
        {
            if (string::equals(rLocaleDataWrapper.getTimeSep(), aSepStr[i]))
                continue;
            for ( sal_Int32 j = 0; j < aStr.getLength(); j++ )
            {
                if (aStr[j] == aSepStr[i])
                    aStr[j] = rLocaleDataWrapper.getTimeSep()[0];
            }
        }
    }

    bool bNegative = false;
    sal_Int32 nSepPos = aStr.indexOf( rLocaleDataWrapper.getTimeSep() );
    if ( aStr[0] == '-' )
        bNegative = true;
    if ( eFormat != TimeFieldFormat::F_SEC_CS )
    {
        if ( nSepPos < 0 )
            nSepPos = aStr.getLength();
        if ( !ImplCutTimePortion( aStr, nSepPos, _bSkipInvalidCharacters, &nHour ) )
            return false;

        nSepPos = aStr.indexOf( rLocaleDataWrapper.getTimeSep() );
        if ( !aStr.isEmpty() && aStr[0] == '-' )
            bNegative = true;
        if ( nSepPos >= 0 )
        {
            if ( !ImplCutTimePortion( aStr, nSepPos, _bSkipInvalidCharacters, &nMinute ) )
                return false;

            nSepPos = aStr.indexOf( rLocaleDataWrapper.getTimeSep() );
            if ( !aStr.isEmpty() && aStr[0] == '-' )
                bNegative = true;
            if ( nSepPos >= 0 )
            {
                if ( !ImplCutTimePortion( aStr, nSepPos, _bSkipInvalidCharacters, &nSecond ) )
                    return false;
                if ( !aStr.isEmpty() && aStr[0] == '-' )
                    bNegative = true;
                nNanoSec = aStr.toString().toInt64();
            }
            else
                nSecond = (short)aStr.toString().toInt32();
        }
        else
            nMinute = (short)aStr.toString().toInt32();
    }
    else if ( nSepPos < 0 )
    {
        nSecond = (short)aStr.toString().toInt32();
        nMinute += nSecond / 60;
        nSecond %= 60;
        nHour += nMinute / 60;
        nMinute %= 60;
    }
    else
    {
        nSecond = (short)aStr.copy( 0, nSepPos ).toString().toInt32();
        aStr.remove( 0, nSepPos+1 );

        nSepPos = aStr.indexOf( rLocaleDataWrapper.getTimeSep() );
        if ( !aStr.isEmpty() && aStr[0] == '-' )
            bNegative = true;
        if ( nSepPos >= 0 )
        {
            nMinute = nSecond;
            nSecond = (short)aStr.copy( 0, nSepPos ).toString().toInt32();
            aStr.remove( 0, nSepPos+1 );

            nSepPos = aStr.indexOf( rLocaleDataWrapper.getTimeSep() );
            if ( !aStr.isEmpty() && aStr[0] == '-' )
                bNegative = true;
            if ( nSepPos >= 0 )
            {
                nHour   = nMinute;
                nMinute = nSecond;
                nSecond = (short)aStr.copy( 0, nSepPos ).toString().toInt32();
                aStr.remove( 0, nSepPos+1 );
            }
            else
            {
                nHour += nMinute / 60;
                nMinute %= 60;
            }
        }
        else
        {
            nMinute += nSecond / 60;
            nSecond %= 60;
            nHour += nMinute / 60;
            nMinute %= 60;
        }
        nNanoSec = aStr.toString().toInt64();
    }

    if ( nNanoSec )
    {
        assert(aStr.getLength() >= 1);

        sal_Int32 nLen = 1; // at least one digit, otherwise nNanoSec==0

        while ( aStr.getLength() > nLen && aStr[nLen] >= '0' && aStr[nLen] <= '9' )
            nLen++;

        while ( nLen < 9)
        {
            nNanoSec *= 10;
            ++nLen;
        }
        while ( nLen > 9 )
        {
            // round if negative?
            nNanoSec = (nNanoSec + 5) / 10;
            --nLen;
        }
    }

    assert(nNanoSec > -1000000000 && nNanoSec < 1000000000);
    if ( (nMinute > 59) || (nSecond > 59) || (nNanoSec > 1000000000) )
        return false;

    if ( eFormat == TimeFieldFormat::F_NONE )
        nSecond = nNanoSec = 0;
    else if ( eFormat == TimeFieldFormat::F_SEC )
        nNanoSec = 0;

    if ( !bDuration )
    {
        if ( bNegative || (nHour < 0) || (nMinute < 0) ||
             (nSecond < 0) || (nNanoSec < 0) )
            return false;

        OUString aUpperCaseStr = aStr.toString().toAsciiUpperCase();
        OUString aAMlocalised(rLocaleDataWrapper.getTimeAM().toAsciiUpperCase());
        OUString aPMlocalised(rLocaleDataWrapper.getTimePM().toAsciiUpperCase());

        if ( (nHour < 12) && ( ( aUpperCaseStr.indexOf( "PM" ) >= 0 ) || ( aUpperCaseStr.indexOf( aPMlocalised ) >= 0 ) ) )
            nHour += 12;

        if ( (nHour == 12) && ( ( aUpperCaseStr.indexOf( "AM" ) >= 0 ) || ( aUpperCaseStr.indexOf( aAMlocalised ) >= 0 ) ) )
            nHour = 0;

        aTime = tools::Time( (sal_uInt16)nHour, (sal_uInt16)nMinute, (sal_uInt16)nSecond,
                      (sal_uInt32)nNanoSec );
    }
    else
    {
        assert( !bNegative || (nHour < 0) || (nMinute < 0) ||
             (nSecond < 0) || (nNanoSec < 0) );
        if ( bNegative || (nHour < 0) || (nMinute < 0) ||
             (nSecond < 0) || (nNanoSec < 0) )
        {
            // LEM TODO: this looks weird... I think buggy when parsing "05:-02:18"
            bNegative   = true;
            nHour       = nHour < 0 ? -nHour : nHour;
            nMinute     = nMinute < 0 ? -nMinute : nMinute;
            nSecond     = nSecond < 0 ? -nSecond : nSecond;
            nNanoSec    = nNanoSec < 0 ? -nNanoSec : nNanoSec;
        }

        aTime = tools::Time( (sal_uInt16)nHour, (sal_uInt16)nMinute, (sal_uInt16)nSecond,
                      (sal_uInt32)nNanoSec );
        if ( bNegative )
            aTime = -aTime;
    }

    rTime = aTime;

    return true;
}

bool TimeFormatter::ImplTimeReformat( const OUString& rStr, OUString& rOutStr )
{
    tools::Time aTime( 0, 0, 0 );
    if ( !ImplTimeGetValue( rStr, aTime, GetFormat(), IsDuration(), ImplGetLocaleDataWrapper() ) )
        return true;

    tools::Time aTempTime = aTime;
    if ( aTempTime > GetMax() )
        aTempTime = GetMax() ;
    else if ( aTempTime < GetMin() )
        aTempTime = GetMin();

    bool bSecond = false;
    bool b100Sec = false;
    if ( meFormat != TimeFieldFormat::F_NONE )
        bSecond = true;
    if ( meFormat == TimeFieldFormat::F_100TH_SEC )
        b100Sec = true;

    if ( meFormat == TimeFieldFormat::F_SEC_CS )
    {
        sal_uLong n  = aTempTime.GetHour() * 3600L;
        n       += aTempTime.GetMin()  * 60L;
        n       += aTempTime.GetSec();
        rOutStr  = OUString::number( n );
        rOutStr += ImplGetLocaleDataWrapper().getTime100SecSep();
        std::ostringstream ostr;
        ostr.fill('0');
        ostr.width(9);
        ostr << aTempTime.GetNanoSec();
        rOutStr += OUString::createFromAscii(ostr.str().c_str());
    }
    else if ( mbDuration )
        rOutStr = ImplGetLocaleDataWrapper().getDuration( aTempTime, bSecond, b100Sec );
    else
    {
        rOutStr = ImplGetLocaleDataWrapper().getTime( aTempTime, bSecond, b100Sec );
        if ( GetTimeFormat() == HOUR_12 )
        {
            if ( aTempTime.GetHour() > 12 )
            {
                tools::Time aT( aTempTime );
                aT.SetHour( aT.GetHour() % 12 );
                rOutStr = ImplGetLocaleDataWrapper().getTime( aT, bSecond, b100Sec );
            }
            // Don't use LocaleDataWrapper, we want AM/PM
            if ( aTempTime.GetHour() < 12 )
                rOutStr += "AM"; // ImplGetLocaleDataWrapper().getTimeAM();
            else
                rOutStr += "PM"; // ImplGetLocaleDataWrapper().getTimePM();
        }
    }

    return true;
}
bool TimeFormatter::ImplAllowMalformedInput() const
{
    return !IsEnforceValidValue();
}

void TimeField::ImplTimeSpinArea( bool bUp )
{
    if ( GetField() )
    {
        sal_Int32 nTimeArea = 0;
        tools::Time aTime( GetTime() );
        OUString aText( GetText() );
        Selection aSelection( GetField()->GetSelection() );

        // Area search
        if ( GetFormat() != TimeFieldFormat::F_SEC_CS )
        {
            //Which area is the cursor in of HH:MM:SS.TT
            for ( sal_Int32 i = 1, nPos = 0; i <= 4; i++ )
            {
                sal_Int32 nPos1 = aText.indexOf( ImplGetLocaleDataWrapper().getTimeSep(), nPos );
                sal_Int32 nPos2 = aText.indexOf( ImplGetLocaleDataWrapper().getTime100SecSep(), nPos );
                //which ever comes first, bearing in mind that one might not be there
                if (nPos1 >= 0 && nPos2 >= 0)
                    nPos = nPos1 < nPos2 ? nPos1 : nPos2;
                else if (nPos1 >= 0)
                    nPos = nPos1;
                else
                    nPos = nPos2;
                if ( nPos < 0 || nPos >= aSelection.Max() )
                {
                    nTimeArea = i;
                    break;
                }
                else
                    nPos++;
            }
        }
        else
        {
            sal_Int32 nPos = aText.indexOf( ImplGetLocaleDataWrapper().getTime100SecSep() );
            if ( nPos < 0 || nPos >= aSelection.Max() )
                nTimeArea = 3;
            else
                nTimeArea = 4;
        }

        if ( nTimeArea )
        {
            tools::Time aAddTime( 0, 0, 0 );
            if ( nTimeArea == 1 )
                aAddTime = tools::Time( 1, 0 );
            else if ( nTimeArea == 2 )
                aAddTime = tools::Time( 0, 1 );
            else if ( nTimeArea == 3 )
                aAddTime = tools::Time( 0, 0, 1 );
            else if ( nTimeArea == 4 )
                aAddTime = tools::Time( 0, 0, 0, 1 );

            if ( !bUp )
                aAddTime = -aAddTime;

            aTime += aAddTime;
            if ( !IsDuration() )
            {
                tools::Time aAbsMaxTime( 23, 59, 59, 999999999 );
                if ( aTime > aAbsMaxTime )
                    aTime = aAbsMaxTime;
                tools::Time aAbsMinTime( 0, 0 );
                if ( aTime < aAbsMinTime )
                    aTime = aAbsMinTime;
            }
            ImplNewFieldValue( aTime );
        }

    }
}

void TimeFormatter::ImplInit()
{
    meFormat        = TimeFieldFormat::F_NONE;
    mbDuration      = false;
    mnTimeFormat    = HOUR_24;  // Should become a ExtTimeFieldFormat in next implementation, merge with mbDuration and meFormat
}

TimeFormatter::TimeFormatter() :
    maLastTime( 0, 0 ),
    maMin( 0, 0 ),
    maMax( 23, 59, 59, 999999999 ),
    mbEnforceValidValue( true ),
    maFieldTime( 0, 0 )
{
    ImplInit();
}

TimeFormatter::~TimeFormatter()
{
}

void TimeFormatter::ReformatAll()
{
    Reformat();
}

void TimeFormatter::SetMin( const tools::Time& rNewMin )
{
    maMin = rNewMin;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

void TimeFormatter::SetMax( const tools::Time& rNewMax )
{
    maMax = rNewMax;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

void TimeFormatter::SetTimeFormat( TimeFormatter::TimeFormat eNewFormat )
{
    mnTimeFormat = sal::static_int_cast<sal_uInt16>(eNewFormat);
}


void TimeFormatter::SetFormat( TimeFieldFormat eNewFormat )
{
    meFormat = eNewFormat;
    ReformatAll();
}

void TimeFormatter::SetDuration( bool bNewDuration )
{
    mbDuration = bNewDuration;
    ReformatAll();
}

void TimeFormatter::SetTime( const tools::Time& rNewTime )
{
    SetUserTime( rNewTime );
    maFieldTime = maLastTime;
    SetEmptyFieldValueData( false );
}

void TimeFormatter::ImplNewFieldValue( const tools::Time& rTime )
{
    if ( GetField() )
    {
        Selection aSelection = GetField()->GetSelection();
        aSelection.Justify();
        OUString aText = GetField()->GetText();

        // If selected until the end then keep it that way
        if ( (sal_Int32)aSelection.Max() == aText.getLength() )
        {
            if ( !aSelection.Len() )
                aSelection.Min() = SELECTION_MAX;
            aSelection.Max() = SELECTION_MAX;
        }

        tools::Time aOldLastTime = maLastTime;
        ImplSetUserTime( rTime, &aSelection );
        maLastTime = aOldLastTime;

        // Modify at Edit is only set at KeyInput
        if ( GetField()->GetText() != aText )
        {
            GetField()->SetModifyFlag();
            GetField()->Modify();
        }
    }
}

void TimeFormatter::ImplSetUserTime( const tools::Time& rNewTime, Selection* pNewSelection )
{
    tools::Time aNewTime = rNewTime;
    if ( aNewTime > GetMax() )
        aNewTime = GetMax();
    else if ( aNewTime < GetMin() )
        aNewTime = GetMin();
    maLastTime = aNewTime;

    if ( GetField() )
    {
        OUString aStr;
        bool bSec    = false;
        bool b100Sec = false;
        if ( meFormat != TimeFieldFormat::F_NONE )
            bSec = true;
        if ( meFormat == TimeFieldFormat::F_100TH_SEC || meFormat == TimeFieldFormat::F_SEC_CS )
            b100Sec = true;
        if ( meFormat == TimeFieldFormat::F_SEC_CS )
        {
            sal_uLong n  = aNewTime.GetHour() * 3600L;
            n       += aNewTime.GetMin()  * 60L;
            n       += aNewTime.GetSec();
            aStr     = OUString::number( n );
            aStr    += ImplGetLocaleDataWrapper().getTime100SecSep();
            std::ostringstream ostr;
            ostr.fill('0');
            ostr.width(9);
            ostr << aNewTime.GetNanoSec();
            aStr += OUString::createFromAscii(ostr.str().c_str());
        }
        else if ( mbDuration )
        {
            aStr = ImplGetLocaleDataWrapper().getDuration( aNewTime, bSec, b100Sec );
        }
        else
        {
            aStr = ImplGetLocaleDataWrapper().getTime( aNewTime, bSec, b100Sec );
            if ( GetTimeFormat() == HOUR_12 )
            {
                if ( aNewTime.GetHour() > 12 )
                {
                    tools::Time aT( aNewTime );
                    aT.SetHour( aT.GetHour() % 12 );
                    aStr = ImplGetLocaleDataWrapper().getTime( aT, bSec, b100Sec );
                }
                // Don't use LocaleDataWrapper, we want AM/PM
                if ( aNewTime.GetHour() < 12 )
                    aStr += "AM"; // ImplGetLocaleDataWrapper().getTimeAM();
                else
                    aStr += "PM"; // ImplGetLocaleDataWrapper().getTimePM();
            }
        }

        ImplSetText( aStr, pNewSelection );
    }
}

void TimeFormatter::SetUserTime( const tools::Time& rNewTime )
{
    ImplSetUserTime( rNewTime );
}

tools::Time TimeFormatter::GetTime() const
{
    tools::Time aTime( 0, 0, 0 );

    if ( GetField() )
    {
        bool bAllowMailformed = ImplAllowMalformedInput();
        if ( ImplTimeGetValue( GetField()->GetText(), aTime, GetFormat(), IsDuration(), ImplGetLocaleDataWrapper(), !bAllowMailformed ) )
        {
            if ( aTime > GetMax() )
                aTime = GetMax();
            else if ( aTime < GetMin() )
                aTime = GetMin();
        }
        else
        {
            if ( bAllowMailformed )
                aTime = GetInvalidTime();
            else
                aTime = maLastTime;
        }
    }

    return aTime;
}

void TimeFormatter::Reformat()
{
    if ( !GetField() )
        return;

    if ( GetField()->GetText().isEmpty() && ImplGetEmptyFieldValue() )
        return;

    OUString aStr;
    bool bOK = ImplTimeReformat( GetField()->GetText(), aStr );
    if ( !bOK )
        return;

    if ( !aStr.isEmpty() )
    {
        ImplSetText( aStr );
        ImplTimeGetValue( aStr, maLastTime, GetFormat(), IsDuration(), ImplGetLocaleDataWrapper() );
    }
    else
        SetTime( maLastTime );
}

TimeField::TimeField( vcl::Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle ),
    maFirst( GetMin() ),
    maLast( GetMax() )
{
    SetField( this );
    SetText( ImplGetLocaleDataWrapper().getTime( maFieldTime, false ) );
    Reformat();
}

void TimeField::dispose()
{
    TimeFormatter::SetField( nullptr );
    SpinField::dispose();
}

bool TimeField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplTimeProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsDuration(), GetFormat(), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return SpinField::PreNotify( rNEvt );
}

bool TimeField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
        {
            if ( !ImplAllowMalformedInput() )
                Reformat();
            else
            {
                tools::Time aTime( 0, 0, 0 );
                if ( ImplTimeGetValue( GetText(), aTime, GetFormat(), IsDuration(), ImplGetLocaleDataWrapper(), false ) )
                    // even with strict text analysis, our text is a valid time -> do a complete
                    // reformat
                    Reformat();
            }
        }
    }

    return SpinField::Notify( rNEvt );
}

void TimeField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::LOCALE) )
    {
        if ( IsDefaultLocale() )
            ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        ReformatAll();
    }
}

void TimeField::Modify()
{
    MarkToBeReformatted( true );
    SpinField::Modify();
}

void TimeField::Up()
{
    ImplTimeSpinArea( true );
    SpinField::Up();
}

void TimeField::Down()
{
    ImplTimeSpinArea( false );
    SpinField::Down();
}

void TimeField::First()
{
    ImplNewFieldValue( maFirst );
    SpinField::First();
}

void TimeField::Last()
{
    ImplNewFieldValue( maLast );
    SpinField::Last();
}

void TimeField::SetExtFormat( ExtTimeFieldFormat eFormat )
{
    switch ( eFormat )
    {
        case EXTTIMEF_24H_SHORT:
        {
            SetTimeFormat( HOUR_24 );
            SetDuration( false );
            SetFormat( TimeFieldFormat::F_NONE );
        }
        break;
        case EXTTIMEF_24H_LONG:
        {
            SetTimeFormat( HOUR_24 );
            SetDuration( false );
            SetFormat( TimeFieldFormat::F_SEC );
        }
        break;
        case EXTTIMEF_12H_SHORT:
        {
            SetTimeFormat( HOUR_12 );
            SetDuration( false );
            SetFormat( TimeFieldFormat::F_NONE );
        }
        break;
        case EXTTIMEF_12H_LONG:
        {
            SetTimeFormat( HOUR_12 );
            SetDuration( false );
            SetFormat( TimeFieldFormat::F_SEC );
        }
        break;
        case EXTTIMEF_DURATION_SHORT:
        {
            SetDuration( true );
            SetFormat( TimeFieldFormat::F_NONE );
        }
        break;
        case EXTTIMEF_DURATION_LONG:
        {
            SetDuration( true );
            SetFormat( TimeFieldFormat::F_SEC );
        }
        break;
        default:    OSL_FAIL( "ExtTimeFieldFormat unknown!" );
    }

    if ( GetField() && !GetField()->GetText().isEmpty() )
        SetUserTime( GetTime() );
    ReformatAll();
}

TimeBox::TimeBox( vcl::Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    SetText( ImplGetLocaleDataWrapper().getTime( maFieldTime, false ) );
    Reformat();
}

void TimeBox::dispose()
{
    TimeFormatter::SetField( nullptr );
    ComboBox::dispose();
}

bool TimeBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplTimeProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsDuration(), GetFormat(), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return ComboBox::PreNotify( rNEvt );
}

bool TimeBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::Notify( rNEvt );
}

void TimeBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::LOCALE) )
    {
        if ( IsDefaultLocale() )
            ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        ReformatAll();
    }
}

void TimeBox::Modify()
{
    MarkToBeReformatted( true );
    ComboBox::Modify();
}

void TimeBox::ReformatAll()
{
    OUString aStr;
    SetUpdateMode( false );
    const sal_Int32 nEntryCount = GetEntryCount();
    for ( sal_Int32 i=0; i < nEntryCount; ++i )
    {
        ImplTimeReformat( GetEntry( i ), aStr );
        RemoveEntryAt(i);
        InsertEntry( aStr, i );
    }
    TimeFormatter::Reformat();
    SetUpdateMode( true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
