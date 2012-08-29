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

#include <comphelper/string.hxx>
#include <tools/rc.h>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/field.hxx>
#include <vcl/unohelp.hxx>

#include <svdata.hxx>

#include <i18npool/mslangid.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/i18n/KCharacterType.hpp>


#include <unotools/localedatawrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/charclass.hxx>
#include <unotools/misccfg.hxx>

using namespace ::com::sun::star;
using namespace ::comphelper;

// =======================================================================

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

// -----------------------------------------------------------------------

static sal_Unicode* ImplAddString( sal_Unicode* pBuf, const String& rStr )
{
    if ( rStr.Len() == 1 )
        *pBuf++ = rStr.GetChar(0);
    else if ( rStr.Len() == 0 )
        ;
    else
    {
        memcpy( pBuf, rStr.GetBuffer(), rStr.Len() * sizeof(sal_Unicode) );
        pBuf += rStr.Len();
    }
    return pBuf;
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

static sal_uInt16 ImplGetNum( const sal_Unicode*& rpBuf, sal_Bool& rbError )
{
    if ( !*rpBuf )
    {
        rbError = sal_True;
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

// -----------------------------------------------------------------------

static void ImplSkipDelimiters( const sal_Unicode*& rpBuf )
{
    while( ( *rpBuf == ',' ) || ( *rpBuf == '.' ) || ( *rpBuf == ';' ) ||
           ( *rpBuf == ':' ) || ( *rpBuf == '-' ) || ( *rpBuf == '/' ) )
    {
        rpBuf++;
    }
}

// -----------------------------------------------------------------------

static int ImplIsPatternChar( xub_Unicode cChar, sal_Char cEditMask )
{
    sal_Int32 nType = 0;

    try
    {
        rtl::OUString aCharStr(cChar);
        nType = ImplGetCharClass()->getStringType( aCharStr, 0, aCharStr.getLength(), Application::GetSettings().GetLocale() );
    }
    catch (const ::com::sun::star::uno::Exception&)
    {
        SAL_WARN( "vcl.control", "ImplIsPatternChar: Exception caught!" );
        return sal_False;
    }

    if ( (cEditMask == EDITMASK_ALPHA) || (cEditMask == EDITMASK_UPPERALPHA) )
    {
        if( !CharClass::isLetterType( nType ) )
            return sal_False;
    }
    else if ( cEditMask == EDITMASK_NUM )
    {
        if( !CharClass::isNumericType( nType ) )
            return sal_False;
    }
    else if ( (cEditMask == EDITMASK_ALPHANUM) || (cEditMask == EDITMASK_UPPERALPHANUM) )
    {
        if( !CharClass::isLetterNumericType( nType ) )
            return sal_False;
    }
    else if ( (cEditMask == EDITMASK_ALLCHAR) || (cEditMask == EDITMASK_UPPERALLCHAR) )
    {
        if ( cChar < 32 )
            return sal_False;
    }
    else if ( cEditMask == EDITMASK_NUMSPACE )
    {
        if ( !CharClass::isNumericType( nType ) && ( cChar != ' ' ) )
            return sal_False;
    }
    else
        return sal_False;

    return sal_True;
}

// -----------------------------------------------------------------------

static xub_Unicode ImplPatternChar( xub_Unicode cChar, sal_Char cEditMask )
{
    if ( ImplIsPatternChar( cChar, cEditMask ) )
    {
        if ( (cEditMask == EDITMASK_UPPERALPHA) ||
             (cEditMask == EDITMASK_UPPERALPHANUM) ||
             ( cEditMask == EDITMASK_UPPERALLCHAR ) )
        {
            cChar = ImplGetCharClass()->toUpper(rtl::OUString(cChar), 0, 1, Application::GetSettings().GetLocale())[0];
        }
        return cChar;
    }
    else
        return 0;
}

// -----------------------------------------------------------------------

static int ImplKommaPointCharEqual( xub_Unicode c1, xub_Unicode c2 )
{
    if ( c1 == c2 )
        return sal_True;
    else if ( ((c1 == '.') || (c1 == ',')) &&
              ((c2 == '.') || (c2 == ',')) )
        return sal_True;
    else
        return sal_False;
}

// -----------------------------------------------------------------------

static XubString ImplPatternReformat( const XubString& rStr,
                                      const rtl::OString& rEditMask,
                                      const XubString& rLiteralMask,
                                      sal_uInt16 nFormatFlags )
{
    if (rEditMask.isEmpty())
        return rStr;

    XubString   aStr    = rStr;
    XubString   aOutStr = rLiteralMask;
    xub_Unicode cTempChar;
    xub_Unicode cChar;
    xub_Unicode cLiteral;
    sal_Char    cMask;
    xub_StrLen  nStrIndex = 0;
    xub_StrLen  i = 0;
    xub_StrLen  n;

    while ( i < rEditMask.getLength() )
    {
        if ( nStrIndex >= aStr.Len() )
            break;

        cChar = aStr.GetChar(nStrIndex);
        cLiteral = rLiteralMask.GetChar(i);
        cMask = rEditMask[i];

        // current position is a literal
        if ( cMask == EDITMASK_LITERAL )
        {
            // if it is a literal copy otherwise ignore because it might be the next valid
            // character of the string
            if ( ImplKommaPointCharEqual( cChar, cLiteral ) )
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
                aOutStr.SetChar( i, cTempChar );
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
                                if ( ImplKommaPointCharEqual( cChar, rLiteralMask.GetChar( n ) ) )
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

    return aOutStr;
}

// -----------------------------------------------------------------------

static void ImplPatternMaxPos( const XubString rStr, const rtl::OString& rEditMask,
                               sal_uInt16 nFormatFlags, sal_Bool bSameMask,
                               sal_uInt16 nCursorPos, sal_uInt16& rPos )
{

    // last position must not be longer than the contained string
    xub_StrLen nMaxPos = rStr.Len();

    // if non empty literals are allowed ignore blanks at the end as well
    if ( bSameMask && !(nFormatFlags & PATTERN_FORMAT_EMPTYLITERALS) )
    {
        while ( nMaxPos )
        {
            if ( (rEditMask[nMaxPos-1] != EDITMASK_LITERAL) &&
                 (rStr.GetChar(nMaxPos-1) != ' ') )
                break;
            nMaxPos--;
        }

        // if we are in front of a literal, continue search until first character after the literal
        xub_StrLen nTempPos = nMaxPos;
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

    // charactr should not move left
    if ( rPos < nCursorPos )
        rPos = nCursorPos;
}

// -----------------------------------------------------------------------

static void ImplPatternProcessStrictModify( Edit* pEdit,
                                            const rtl::OString& rEditMask,
                                            const XubString& rLiteralMask,
                                            sal_uInt16 nFormatFlags, sal_Bool bSameMask )
{
    XubString aText = pEdit->GetText();

    // remove leading blanks
    if ( bSameMask && !(nFormatFlags & PATTERN_FORMAT_EMPTYLITERALS) )
    {
        xub_StrLen i = 0;
        xub_StrLen nMaxLen = aText.Len();
        while ( i < nMaxLen )
        {
            if ( (rEditMask[i] != EDITMASK_LITERAL) &&
                 (aText.GetChar( i ) != ' ') )
                break;

            i++;
        }
        // keep all literal characters
        while ( i && (rEditMask[i] == EDITMASK_LITERAL) )
            i--;
        aText.Erase( 0, i );
    }

    XubString aNewText = ImplPatternReformat( aText, rEditMask, rLiteralMask, nFormatFlags );
    if ( aNewText != aText )
    {
        // adjust selection such that it remains at the end if it was there before
        Selection aSel = pEdit->GetSelection();
        sal_uLong nMaxSel = Max( aSel.Min(), aSel.Max() );
        if ( nMaxSel >= aText.Len() )
        {
            xub_StrLen nMaxPos = aNewText.Len();
            ImplPatternMaxPos( aNewText, rEditMask, nFormatFlags, bSameMask, (xub_StrLen)nMaxSel, nMaxPos );
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

// -----------------------------------------------------------------------

static xub_StrLen ImplPatternLeftPos(const rtl::OString& rEditMask, xub_StrLen nCursorPos)
{
    // search non-literal predecessor
    xub_StrLen nNewPos = nCursorPos;
    xub_StrLen nTempPos = nNewPos;
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

// -----------------------------------------------------------------------

static xub_StrLen ImplPatternRightPos( const XubString& rStr, const rtl::OString& rEditMask,
                                       sal_uInt16 nFormatFlags, sal_Bool bSameMask,
                                       xub_StrLen nCursorPos )
{
    // search non-literal successor
    xub_StrLen nNewPos = nCursorPos;
    xub_StrLen nTempPos = nNewPos;
    while ( nTempPos < rEditMask.getLength() )
    {
        if ( rEditMask[nTempPos+1] != EDITMASK_LITERAL )
        {
            nNewPos = nTempPos+1;
            break;
        }
        nTempPos++;
    }
    ImplPatternMaxPos( rStr, rEditMask, nFormatFlags, bSameMask, nCursorPos, nNewPos );
    return nNewPos;
}

// -----------------------------------------------------------------------

static sal_Bool ImplPatternProcessKeyInput( Edit* pEdit, const KeyEvent& rKEvt,
                                        const rtl::OString& rEditMask,
                                        const XubString& rLiteralMask,
                                        sal_Bool bStrictFormat,
                                        sal_uInt16 nFormatFlags,
                                        sal_Bool bSameMask,
                                        sal_Bool& rbInKeyInput )
{
    if ( rEditMask.isEmpty() || !bStrictFormat )
        return sal_False;

    Selection   aOldSel     = pEdit->GetSelection();
    KeyCode     aCode       = rKEvt.GetKeyCode();
    xub_Unicode cChar       = rKEvt.GetCharCode();
    sal_uInt16      nKeyCode    = aCode.GetCode();
    sal_Bool        bShift      = aCode.IsShift();
    xub_StrLen  nCursorPos  = (xub_StrLen)aOldSel.Max();
    xub_StrLen  nNewPos;
    xub_StrLen  nTempPos;

    if ( nKeyCode && !aCode.IsMod1() && !aCode.IsMod2() )
    {
        if ( nKeyCode == KEY_LEFT )
        {
            Selection aSel( ImplPatternLeftPos( rEditMask, nCursorPos ) );
            if ( bShift )
                aSel.Min() = aOldSel.Min();
            pEdit->SetSelection( aSel );
            return sal_True;
        }
        else if ( nKeyCode == KEY_RIGHT )
        {
            // Use the start of selection as minimum; even a small position is allowed in case that
            // all was selected by the focus
            Selection aSel( aOldSel );
            aSel.Justify();
            nCursorPos = (xub_StrLen)aSel.Min();
            aSel.Max() = ImplPatternRightPos( pEdit->GetText(), rEditMask, nFormatFlags, bSameMask, nCursorPos );
            if ( bShift )
                aSel.Min() = aOldSel.Min();
            else
                aSel.Min() = aSel.Max();
            pEdit->SetSelection( aSel );
            return sal_True;
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
            return sal_True;
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
            nCursorPos = (xub_StrLen)aSel.Min();
            ImplPatternMaxPos( pEdit->GetText(), rEditMask, nFormatFlags, bSameMask, nCursorPos, nNewPos );
            aSel.Max() = nNewPos;
            if ( bShift )
                aSel.Min() = aOldSel.Min();
            else
                aSel.Min() = aSel.Max();
            pEdit->SetSelection( aSel );
            return sal_True;
        }
        else if ( (nKeyCode == KEY_BACKSPACE) || (nKeyCode == KEY_DELETE) )
        {
            XubString   aStr( pEdit->GetText() );
            XubString   aOldStr = aStr;
            Selection   aSel = aOldSel;

            aSel.Justify();
            nNewPos = (xub_StrLen)aSel.Min();

             // if selection then delete it
            if ( aSel.Len() )
            {
                if ( bSameMask )
                    aStr.Erase( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );
                else
                {
                    XubString aRep = rLiteralMask.Copy( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );
                    aStr.Replace( (xub_StrLen)aSel.Min(), aRep.Len(), aRep );
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
                    nTempPos = ImplPatternRightPos( aStr, rEditMask, nFormatFlags, bSameMask, nNewPos );

                if ( nNewPos != nTempPos )
                {
                    if ( bSameMask )
                    {
                        if ( rEditMask[nNewPos] != EDITMASK_LITERAL )
                            aStr.Erase( nNewPos, 1 );
                    }
                    else
                    {
                        XubString aTempStr = rLiteralMask.Copy( nNewPos, 1 );
                        aStr.Replace( nNewPos, aTempStr.Len(), aTempStr );
                    }
                }
            }

            if ( aOldStr != aStr )
            {
                if ( bSameMask )
                    aStr = ImplPatternReformat( aStr, rEditMask, rLiteralMask, nFormatFlags );
                rbInKeyInput = sal_True;
                pEdit->SetText( aStr, Selection( nNewPos ) );
                pEdit->SetModifyFlag();
                pEdit->Modify();
                rbInKeyInput = sal_False;
            }
            else
                pEdit->SetSelection( Selection( nNewPos ) );

            return sal_True;
        }
        else if ( nKeyCode == KEY_INSERT )
        {
            // you can only set InsertModus for a PatternField if the
            // mask is equal at all input positions
            if ( !bSameMask )
            {
                return sal_True;
            }
        }
    }

    if ( rKEvt.GetKeyCode().IsMod2() || (cChar < 32) || (cChar == 127) )
        return sal_False;

    Selection aSel = aOldSel;
    aSel.Justify();
    nNewPos = (xub_StrLen)aSel.Min();

    if ( nNewPos < rEditMask.getLength() )
    {
        xub_Unicode cPattChar = ImplPatternChar( cChar, rEditMask[nNewPos] );
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
                             ImplKommaPointCharEqual( cChar, rLiteralMask.GetChar(nTempPos) ) )
                        {
                            nTempPos++;
                            ImplPatternMaxPos( pEdit->GetText(), rEditMask, nFormatFlags, bSameMask, nNewPos, nTempPos );
                            if ( nTempPos > nNewPos )
                            {
                                pEdit->SetSelection( Selection( nTempPos ) );
                                return sal_True;
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
        XubString   aStr = pEdit->GetText();
        sal_Bool        bError = sal_False;
        if ( bSameMask && pEdit->IsInsertMode() )
        {
            // crop spaces and literals at the end until current position
            xub_StrLen n = aStr.Len();
            while ( n && (n > nNewPos) )
            {
                if ( (aStr.GetChar( n-1 ) != ' ') &&
                     ((n > rEditMask.getLength()) || (rEditMask[n-1] != EDITMASK_LITERAL)) )
                    break;

                n--;
            }
            aStr.Erase( n );

            if ( aSel.Len() )
                aStr.Erase( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );

            if ( aStr.Len() < rEditMask.getLength() )
            {
                // possibly extend string until cursor position
                if ( aStr.Len() < nNewPos )
                    aStr += rLiteralMask.Copy( aStr.Len(), nNewPos-aStr.Len() );
                if ( nNewPos < aStr.Len() )
                    aStr.Insert( cChar, nNewPos );
                else if ( nNewPos < rEditMask.getLength() )
                    aStr += cChar;
                aStr = ImplPatternReformat( aStr, rEditMask, rLiteralMask, nFormatFlags );
            }
            else
                bError = sal_True;
        }
        else
        {
            if ( aSel.Len() )
            {
                // delete selection
                XubString aRep = rLiteralMask.Copy( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );
                aStr.Replace( (xub_StrLen)aSel.Min(), aRep.Len(), aRep );
            }

            if ( nNewPos < aStr.Len() )
                aStr.SetChar( nNewPos, cChar );
            else if ( nNewPos < rEditMask.getLength() )
                aStr += cChar;
        }

        if ( !bError )
        {
            rbInKeyInput = sal_True;
            Selection aNewSel( ImplPatternRightPos( aStr, rEditMask, nFormatFlags, bSameMask, nNewPos ) );
            pEdit->SetText( aStr, aNewSel );
            pEdit->SetModifyFlag();
            pEdit->Modify();
            rbInKeyInput = sal_False;
        }
    }

    return sal_True;
}

// -----------------------------------------------------------------------

void PatternFormatter::ImplSetMask(const rtl::OString& rEditMask,
                                    const XubString& rLiteralMask)
{
    m_aEditMask      = rEditMask;
    maLiteralMask   = rLiteralMask;
    mbSameMask      = sal_True;

    if ( m_aEditMask.getLength() != maLiteralMask.Len() )
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
    xub_StrLen  i = 0;
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
                mbSameMask = sal_False;
                break;
            }
            if ( i < rLiteralMask.Len() )
            {
                if ( rLiteralMask.GetChar( i ) != ' ' )
                {
                    mbSameMask = sal_False;
                    break;
                }
            }
            if ( !c )
                c = cTemp;
            if ( cTemp != c )
            {
                mbSameMask = sal_False;
                break;
            }
        }
        i++;
    }
}

// -----------------------------------------------------------------------

PatternFormatter::PatternFormatter()
{
    mnFormatFlags       = 0;
    mbSameMask          = sal_True;
    mbInPattKeyInput    = sal_False;
}

// -----------------------------------------------------------------------

PatternFormatter::~PatternFormatter()
{
}

// -----------------------------------------------------------------------

void PatternFormatter::SetMask( const rtl::OString& rEditMask,
                                const XubString& rLiteralMask )
{
    ImplSetMask( rEditMask, rLiteralMask );
    ReformatAll();
}

// -----------------------------------------------------------------------

void PatternFormatter::SetString( const XubString& rStr )
{
    maFieldString = rStr;
    if ( GetField() )
    {
        GetField()->SetText( rStr );
        MarkToBeReformatted( sal_False );
    }
}

// -----------------------------------------------------------------------

XubString PatternFormatter::GetString() const
{
    if ( !GetField() )
        return ImplGetSVEmptyStr();
    else
        return ImplPatternReformat( GetField()->GetText(), m_aEditMask, maLiteralMask, mnFormatFlags );
}

// -----------------------------------------------------------------------

void PatternFormatter::Reformat()
{
    if ( GetField() )
    {
        ImplSetText( ImplPatternReformat( GetField()->GetText(), m_aEditMask, maLiteralMask, mnFormatFlags ) );
        if ( !mbSameMask && IsStrictFormat() && !GetField()->IsReadOnly() )
            GetField()->SetInsertMode( sal_False );
    }
}

// -----------------------------------------------------------------------

PatternField::PatternField( Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

PatternField::~PatternField()
{
}

// -----------------------------------------------------------------------

long PatternField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplPatternProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), GetEditMask(), GetLiteralMask(),
                                         IsStrictFormat(), GetFormatFlags(),
                                         ImplIsSameMask(), ImplGetInPattKeyInput() ) )
            return 1;
    }

    return SpinField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long PatternField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( sal_False );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (GetText().Len() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return SpinField::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void PatternField::Modify()
{
    if ( !ImplGetInPattKeyInput() )
    {
        if ( IsStrictFormat() )
            ImplPatternProcessStrictModify( GetField(), GetEditMask(), GetLiteralMask(), GetFormatFlags(), ImplIsSameMask() );
        else
            MarkToBeReformatted( sal_True );
    }

    SpinField::Modify();
}

// -----------------------------------------------------------------------

PatternBox::PatternBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    Reformat();
}

// -----------------------------------------------------------------------

PatternBox::~PatternBox()
{
}

// -----------------------------------------------------------------------

long PatternBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplPatternProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), GetEditMask(), GetLiteralMask(),
                                         IsStrictFormat(), GetFormatFlags(),
                                         ImplIsSameMask(), ImplGetInPattKeyInput() ) )
            return 1;
    }

    return ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long PatternBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( sal_False );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (GetText().Len() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void PatternBox::Modify()
{
    if ( !ImplGetInPattKeyInput() )
    {
        if ( IsStrictFormat() )
            ImplPatternProcessStrictModify( GetField(), GetEditMask(), GetLiteralMask(), GetFormatFlags(), ImplIsSameMask() );
        else
            MarkToBeReformatted( sal_True );
    }

    ComboBox::Modify();
}

// -----------------------------------------------------------------------

void PatternBox::ReformatAll()
{
    XubString aStr;
    SetUpdateMode( sal_False );
    sal_uInt16 nEntryCount = GetEntryCount();
    for ( sal_uInt16 i=0; i < nEntryCount; i++ )
    {
        aStr = ImplPatternReformat( GetEntry( i ), GetEditMask(), GetLiteralMask(), GetFormatFlags() );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    PatternFormatter::Reformat();
    SetUpdateMode( sal_True );
}

// =======================================================================

static ExtDateFieldFormat ImplGetExtFormat( DateFormat eOld )
{
    switch( eOld )
    {
        case DMY:   return XTDATEF_SHORT_DDMMYY;
        case MDY:   return XTDATEF_SHORT_MMDDYY;
        default:    return XTDATEF_SHORT_YYMMDD;
    }
}

// -----------------------------------------------------------------------

static sal_uInt16 ImplCutNumberFromString( XubString& rStr )
{
    // Nach Zahl suchen
    while ( rStr.Len() && !(rStr.GetChar( 0 ) >= '0' && rStr.GetChar( 0 ) <= '9') )
        rStr.Erase( 0, 1 );
    if ( !rStr.Len() )
        return 0;
    XubString aNumStr;
    while ( rStr.Len() && (rStr.GetChar( 0 ) >= '0' && rStr.GetChar( 0 ) <= '9') )
    {
        aNumStr.Insert( rStr.GetChar( 0 ) );
        rStr.Erase( 0, 1 );
    }
    return (sal_uInt16)aNumStr.ToInt32();
}

// -----------------------------------------------------------------------

static sal_Bool ImplCutMonthName( XubString& rStr, const XubString& _rLookupMonthName )
{
    sal_uInt16 nPos = rStr.Search( _rLookupMonthName );
    if ( nPos != STRING_NOTFOUND )
    {
        rStr.Erase( 0, nPos + _rLookupMonthName.Len() );
        return sal_True;
    }
    return sal_False;
}

// -----------------------------------------------------------------------

static sal_uInt16 ImplCutMonthFromString( XubString& rStr, const CalendarWrapper& rCalendarWrapper )
{
    // search for a month' name
    for ( sal_uInt16 i=1; i <= 12; i++ )
    {
        String aMonthName = rCalendarWrapper.getMonths()[i-1].FullName;
        // long month name?
        if ( ImplCutMonthName( rStr, aMonthName ) )
            return i;

        // short month name?
        String aAbbrevMonthName = rCalendarWrapper.getMonths()[i-1].AbbrevName;
        if ( ImplCutMonthName( rStr, aAbbrevMonthName ) )
            return i;
    }

    return ImplCutNumberFromString( rStr );
}

// -----------------------------------------------------------------------

static String ImplGetDateSep( const LocaleDataWrapper& rLocaleDataWrapper, ExtDateFieldFormat eFormat )
{
    if ( ( eFormat == XTDATEF_SHORT_YYMMDD_DIN5008 ) || ( eFormat == XTDATEF_SHORT_YYYYMMDD_DIN5008 ) )
        return rtl::OUString("-");
    else
        return rLocaleDataWrapper.getDateSep();
}

static sal_Bool ImplDateProcessKeyInput( Edit*, const KeyEvent& rKEvt, ExtDateFieldFormat eFormat,
                                     const LocaleDataWrapper& rLocaleDataWrapper  )
{
    xub_Unicode cChar = rKEvt.GetCharCode();
    sal_uInt16 nGroup = rKEvt.GetKeyCode().GetGroup();
    if ( (nGroup == KEYGROUP_FKEYS) || (nGroup == KEYGROUP_CURSOR) ||
         (nGroup == KEYGROUP_MISC)||
         ((cChar >= '0') && (cChar <= '9')) ||
         (cChar == ImplGetDateSep( rLocaleDataWrapper, eFormat ).GetChar(0) ) )
        return sal_False;
    else
        return sal_True;
}

// -----------------------------------------------------------------------

static sal_Bool ImplDateGetValue( const XubString& rStr, Date& rDate, ExtDateFieldFormat eDateFormat,
                              const LocaleDataWrapper& rLocaleDataWrapper, const CalendarWrapper& rCalendarWrapper,
                              const AllSettings& )
{
    sal_uInt16 nDay = 0;
    sal_uInt16 nMonth = 0;
    sal_uInt16 nYear = 0;
    sal_Bool bYear = sal_True;
    sal_Bool bError = sal_False;
    String aStr( rStr );

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
        // Check if year is present:
        String aDateSep = ImplGetDateSep( rLocaleDataWrapper, eDateFormat );
        sal_uInt16 nSepPos = aStr.Search( aDateSep );
        if ( nSepPos == STRING_NOTFOUND )
            return sal_False;
        nSepPos = aStr.Search( aDateSep, nSepPos+1 );
        if ( ( nSepPos == STRING_NOTFOUND ) || ( nSepPos == (aStr.Len()-1) ) )
        {
            bYear = sal_False;
            nYear = Date( Date::SYSTEM ).GetYear();
        }

        const sal_Unicode* pBuf = aStr.GetBuffer();
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
        return sal_False;

    Date aNewDate( nDay, nMonth, nYear );
    DateFormatter::ExpandCentury( aNewDate, utl::MiscCfg().GetYear2000() );
    if ( aNewDate.IsValidDate() )
    {
        rDate = aNewDate;
        return sal_True;
    }
    return sal_False;
}

// -----------------------------------------------------------------------

sal_Bool DateFormatter::ImplDateReformat( const XubString& rStr, XubString& rOutStr, const AllSettings& rSettings )
{
    Date aDate( 0, 0, 0 );
    if ( !ImplDateGetValue( rStr, aDate, GetExtDateFormat(sal_True), ImplGetLocaleDataWrapper(), GetCalendarWrapper(), GetFieldSettings() ) )
        return sal_True;

    Date aTempDate = aDate;
    if ( aTempDate > GetMax() )
        aTempDate = GetMax();
    else if ( aTempDate < GetMin() )
        aTempDate = GetMin();

    if ( GetErrorHdl().IsSet() && (aDate != aTempDate) )
    {
        maCorrectedDate = aTempDate;
        if( !GetErrorHdl().Call( this ) )
        {
            maCorrectedDate = Date( Date::SYSTEM );
            return sal_False;
        }
        else
            maCorrectedDate = Date( Date::SYSTEM );
    }

    rOutStr = ImplGetDateAsText( aTempDate, rSettings );

    return sal_True;
}

// -----------------------------------------------------------------------

XubString DateFormatter::ImplGetDateAsText( const Date& rDate,
                                            const AllSettings& ) const
{
    sal_Bool bShowCentury = sal_False;
    switch ( GetExtDateFormat() )
    {
        case XTDATEF_SYSTEM_SHORT_YYYY:
        case XTDATEF_SYSTEM_LONG:
        case XTDATEF_SHORT_DDMMYYYY:
        case XTDATEF_SHORT_MMDDYYYY:
        case XTDATEF_SHORT_YYYYMMDD:
        case XTDATEF_SHORT_YYYYMMDD_DIN5008:
        {
            bShowCentury = sal_True;
        }
        break;
        default:
        {
            bShowCentury = sal_False;
        }
    }

    if ( !bShowCentury )
    {
        // Check if I have to use force showing the century
        sal_uInt16 nTwoDigitYearStart = utl::MiscCfg().GetYear2000();
        sal_uInt16 nYear = rDate.GetYear();

        // If year is not in double digit range
        if ( (nYear < nTwoDigitYearStart) || (nYear >= nTwoDigitYearStart+100) )
            bShowCentury = sal_True;
    }

    sal_Unicode aBuf[128];
    sal_Unicode* pBuf = aBuf;

    String aDateSep = ImplGetDateSep( ImplGetLocaleDataWrapper(), GetExtDateFormat( sal_True ) );
    sal_uInt16 nDay = rDate.GetDay();
    sal_uInt16 nMonth = rDate.GetMonth();
    sal_uInt16 nYear = rDate.GetYear();
    sal_uInt16 nYearLen = bShowCentury ? 4 : 2;

    if ( !bShowCentury )
        nYear %= 100;

    switch ( GetExtDateFormat( sal_True ) )
    {
        case XTDATEF_SYSTEM_LONG:
        {
            return ImplGetLocaleDataWrapper().getLongDate( rDate, GetCalendarWrapper(), 1, sal_False, 1, !bShowCentury );
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

    return rtl::OUString(aBuf, pBuf-aBuf);
}

// -----------------------------------------------------------------------

static void ImplDateIncrementDay( Date& rDate, sal_Bool bUp )
{
    DateFormatter::ExpandCentury( rDate );

    if ( bUp )
    {
        if ( (rDate.GetDay() != 31) || (rDate.GetMonth() != 12) || (rDate.GetYear() != 9999) )
            rDate++;
    }
    else
    {
        if ( (rDate.GetDay() != 1 ) || (rDate.GetMonth() != 1) || (rDate.GetYear() != 0) )
            rDate--;
    }
}

// -----------------------------------------------------------------------

static void ImplDateIncrementMonth( Date& rDate, sal_Bool bUp )
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

    sal_uInt16 nDaysInMonth = rDate.GetDaysInMonth();
    if ( rDate.GetDay() > nDaysInMonth )
        rDate.SetDay( nDaysInMonth );
}

// -----------------------------------------------------------------------

static void ImplDateIncrementYear( Date& rDate, sal_Bool bUp )
{
    DateFormatter::ExpandCentury( rDate );

    sal_uInt16 nYear = rDate.GetYear();
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
}

// -----------------------------------------------------------------------
sal_Bool DateFormatter::ImplAllowMalformedInput() const
{
    return !IsEnforceValidValue();
}

// -----------------------------------------------------------------------

void DateField::ImplDateSpinArea( sal_Bool bUp )
{
    // increment days if all is selected
    if ( GetField() )
    {
        Date aDate( GetDate() );
        Selection aSelection = GetField()->GetSelection();
        aSelection.Justify();
        XubString aText( GetText() );
        if ( (xub_StrLen)aSelection.Len() == aText.Len() )
            ImplDateIncrementDay( aDate, bUp );
        else
        {
            xub_StrLen nDateArea = 0;

            ExtDateFieldFormat eFormat = GetExtDateFormat( sal_True );
            if ( eFormat == XTDATEF_SYSTEM_LONG )
            {
                eFormat = ImplGetExtFormat( ImplGetLocaleDataWrapper().getLongDateFormat() );
                nDateArea = 1;
            }
            else
            {
                // search area
                xub_StrLen nPos = 0;
                String aDateSep = ImplGetDateSep( ImplGetLocaleDataWrapper(), eFormat );
                for ( xub_StrLen i = 1; i <= 3; i++ )
                {
                    nPos = aText.Search( aDateSep, nPos );
                    if ( nPos >= (sal_uInt16)aSelection.Max() )
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

// -----------------------------------------------------------------------

void DateFormatter::ImplInit()
{
    mbLongFormat        = sal_False;
    mbShowDateCentury   = sal_True;
    mpCalendarWrapper   = NULL;
    mnDateFormat        = 0xFFFF;
    mnExtDateFormat     = XTDATEF_SYSTEM_SHORT;
}

// -----------------------------------------------------------------------

DateFormatter::DateFormatter() :
    maFieldDate( 0 ),
    maLastDate( 0 ),
    maMin( 1, 1, 1900 ),
    maMax( 31, 12, 2200 ),
    maCorrectedDate( Date::SYSTEM ),
    mbEnforceValidValue( sal_True )
{
    ImplInit();
}

// -----------------------------------------------------------------------

void DateFormatter::ImplLoadRes( const ResId& rResId )
{
    ResMgr*     pMgr = rResId.GetResMgr();
    if( pMgr )
    {
        sal_uLong       nMask = pMgr->ReadLong();

        if ( DATEFORMATTER_MIN & nMask )
        {
            maMin = Date( ResId( (RSHEADER_TYPE *)pMgr->GetClass(), *pMgr ) );
            pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE*)pMgr->GetClass() ) );
        }
        if ( DATEFORMATTER_MAX & nMask )
        {
            maMax = Date( ResId( (RSHEADER_TYPE *)pMgr->GetClass(), *pMgr ) );
            pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE*)pMgr->GetClass() ) );
        }
        if ( DATEFORMATTER_LONGFORMAT & nMask )
            mbLongFormat = (sal_Bool)pMgr->ReadShort();

        if ( DATEFORMATTER_STRICTFORMAT & nMask )
            SetStrictFormat( (sal_Bool)pMgr->ReadShort() );

        if ( DATEFORMATTER_VALUE & nMask )
        {
            maFieldDate = Date( ResId( (RSHEADER_TYPE *)pMgr->GetClass(), *pMgr ) );
            pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE*)pMgr->GetClass() ) );
            if ( maFieldDate > maMax )
                maFieldDate = maMax;
            if ( maFieldDate < maMin )
                maFieldDate = maMin;
            maLastDate = maFieldDate;
        }
    }
}

// -----------------------------------------------------------------------

DateFormatter::~DateFormatter()
{
    delete mpCalendarWrapper;
    mpCalendarWrapper = NULL;
}

// -----------------------------------------------------------------------

void DateFormatter::SetLocale( const ::com::sun::star::lang::Locale& rLocale )
{
    delete mpCalendarWrapper;
    mpCalendarWrapper = NULL;
    FormatterBase::SetLocale( rLocale );
}


// -----------------------------------------------------------------------

CalendarWrapper& DateFormatter::GetCalendarWrapper() const
{
    if ( !mpCalendarWrapper )
    {
        ((DateFormatter*)this)->mpCalendarWrapper = new CalendarWrapper( vcl::unohelper::GetMultiServiceFactory() );
        mpCalendarWrapper->loadDefaultCalendar( GetLocale() );
    }

    return *mpCalendarWrapper;
}

// -----------------------------------------------------------------------

void DateFormatter::SetExtDateFormat( ExtDateFieldFormat eFormat )
{
    mnExtDateFormat = eFormat;
    ReformatAll();
}

// -----------------------------------------------------------------------

ExtDateFieldFormat DateFormatter::GetExtDateFormat( sal_Bool bResolveSystemFormat ) const
{
    ExtDateFieldFormat eDateFormat = (ExtDateFieldFormat)mnExtDateFormat;

    if ( bResolveSystemFormat && ( eDateFormat <= XTDATEF_SYSTEM_SHORT_YYYY ) )
    {
        sal_Bool bShowCentury = (eDateFormat == XTDATEF_SYSTEM_SHORT_YYYY);
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

// -----------------------------------------------------------------------

void DateFormatter::ReformatAll()
{
    Reformat();
}

// -----------------------------------------------------------------------

void DateFormatter::SetMin( const Date& rNewMin )
{
    maMin = rNewMin;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

// -----------------------------------------------------------------------

void DateFormatter::SetMax( const Date& rNewMax )
{
    maMax = rNewMax;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

// -----------------------------------------------------------------------

void DateFormatter::SetLongFormat( sal_Bool bLong )
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

// -----------------------------------------------------------------------

void DateFormatter::SetShowDateCentury( sal_Bool bShowDateCentury )
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

// -----------------------------------------------------------------------

void DateFormatter::SetDate( const Date& rNewDate )
{
    SetUserDate( rNewDate );
    maFieldDate = maLastDate;
    maLastDate = GetDate();
}

// -----------------------------------------------------------------------

void DateFormatter::SetUserDate( const Date& rNewDate )
{
    ImplSetUserDate( rNewDate );
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void DateFormatter::ImplNewFieldValue( const Date& rDate )
{
    if ( GetField() )
    {
        Selection aSelection = GetField()->GetSelection();
        aSelection.Justify();
        XubString aText = GetField()->GetText();

        // If selected until the end then keep it that way
        if ( (xub_StrLen)aSelection.Max() == aText.Len() )
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

// -----------------------------------------------------------------------

Date DateFormatter::GetDate() const
{
    Date aDate( 0, 0, 0 );

    if ( GetField() )
    {
        if ( ImplDateGetValue( GetField()->GetText(), aDate, GetExtDateFormat(sal_True), ImplGetLocaleDataWrapper(), GetCalendarWrapper(), GetFieldSettings() ) )
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

// -----------------------------------------------------------------------

void DateFormatter::SetEmptyDate()
{
    FormatterBase::SetEmptyFieldValue();
}

// -----------------------------------------------------------------------

sal_Bool DateFormatter::IsEmptyDate() const
{
    sal_Bool bEmpty = FormatterBase::IsEmptyFieldValue();

    if ( GetField() && MustBeReformatted() && IsEmptyFieldValueEnabled() )
    {
        if ( !GetField()->GetText().Len() )
        {
            bEmpty = sal_True;
        }
        else if ( !maLastDate.GetDate() )
        {
            Date aDate( Date::EMPTY );
            bEmpty = !ImplDateGetValue( GetField()->GetText(), aDate, GetExtDateFormat(sal_True), ImplGetLocaleDataWrapper(), GetCalendarWrapper(), GetFieldSettings() );
        }
    }
    return bEmpty;
}

// -----------------------------------------------------------------------

void DateFormatter::Reformat()
{
    if ( !GetField() )
        return;

    if ( !GetField()->GetText().Len() && ImplGetEmptyFieldValue() )
        return;

    XubString aStr;
    sal_Bool bOK = ImplDateReformat( GetField()->GetText(), aStr, GetFieldSettings() );
    if( !bOK )
        return;

    if ( aStr.Len() )
    {
        ImplSetText( aStr );
        ImplDateGetValue( aStr, maLastDate, GetExtDateFormat(sal_True), ImplGetLocaleDataWrapper(), GetCalendarWrapper(), GetFieldSettings() );
    }
    else
    {
        if ( maLastDate.GetDate() )
            SetDate( maLastDate );
        else if ( !IsEmptyFieldValueEnabled() )
            SetDate( Date( Date::SYSTEM ) );
        else
        {
            ImplSetText( ImplGetSVEmptyStr() );
            SetEmptyFieldValueData( sal_True );
        }
    }
}

// -----------------------------------------------------------------------

void DateFormatter::ExpandCentury( Date& rDate )
{
    ExpandCentury( rDate, utl::MiscCfg().GetYear2000() );
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

DateField::DateField( Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle ),
    maFirst( GetMin() ),
    maLast( GetMax() )
{
    SetField( this );
    SetText( ImplGetLocaleDataWrapper().getDate( ImplGetFieldDate() ) );
    Reformat();
    ResetLastDate();
}

// -----------------------------------------------------------------------

DateField::DateField( Window* pParent, const ResId& rResId ) :
    SpinField( WINDOW_DATEFIELD ),
    maFirst( GetMin() ),
    maLast( GetMax() )
{
    rResId.SetRT( RSC_DATEFIELD );
    WinBits nStyle = ImplInitRes( rResId );
    SpinField::ImplInit( pParent, nStyle );
    SetField( this );
    SetText( ImplGetLocaleDataWrapper().getDate( ImplGetFieldDate() ) );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE ) )
        Show();

    ResetLastDate();
}

// -----------------------------------------------------------------------

void DateField::ImplLoadRes( const ResId& rResId )
{
    SpinField::ImplLoadRes( rResId );

    ResMgr* pMgr = rResId.GetResMgr();
    if( pMgr )
    {
        DateFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes(), *pMgr ) );

        sal_uLong  nMask = ReadLongRes();
        if ( DATEFIELD_FIRST & nMask )
        {
            maFirst = Date( ResId( (RSHEADER_TYPE *)GetClassRes(), *pMgr ) );
            IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
        }
        if ( DATEFIELD_LAST & nMask )
        {
            maLast = Date( ResId( (RSHEADER_TYPE *)GetClassRes(), *pMgr ) );
            IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
        }
    }

    Reformat();
}

// -----------------------------------------------------------------------

DateField::~DateField()
{
}

// -----------------------------------------------------------------------

long DateField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && IsStrictFormat() &&
         ( GetExtDateFormat() != XTDATEF_SYSTEM_LONG ) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplDateProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), GetExtDateFormat( sal_True ), ImplGetLocaleDataWrapper() ) )
            return 1;
    }

    return SpinField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long DateField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( sal_False );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() )
        {
            // !!! We should find out why dates are treated differently than other fields (see
            // also bug: 52384)

            sal_Bool bTextLen = GetText().Len() != 0;
            if ( bTextLen || !IsEmptyFieldValueEnabled() )
            {
                if ( !ImplAllowMalformedInput() )
                    Reformat();
                else
                {
                    Date aDate( 0, 0, 0 );
                    if ( ImplDateGetValue( GetText(), aDate, GetExtDateFormat(sal_True), ImplGetLocaleDataWrapper(), GetCalendarWrapper(), GetFieldSettings() ) )
                        // even with strict text analysis, our text is a valid date -> do a complete
                        // reformat
                        Reformat();
                }
            }
            else if ( !bTextLen && IsEmptyFieldValueEnabled() )
            {
                ResetLastDate();
                SetEmptyFieldValueData( sal_True );
            }
        }
    }

    return SpinField::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void DateField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & (SETTINGS_LOCALE|SETTINGS_MISC)) )
    {
        if ( IsDefaultLocale() && ( rDCEvt.GetFlags() & SETTINGS_LOCALE ) )
            ImplGetLocaleDataWrapper().setLocale( GetSettings().GetLocale() );
        ReformatAll();
    }
}

// -----------------------------------------------------------------------

void DateField::Modify()
{
    MarkToBeReformatted( sal_True );
    SpinField::Modify();
}

// -----------------------------------------------------------------------

void DateField::Up()
{
    ImplDateSpinArea( sal_True );
    SpinField::Up();
}

// -----------------------------------------------------------------------

void DateField::Down()
{
    ImplDateSpinArea( sal_False );
    SpinField::Down();
}

// -----------------------------------------------------------------------

void DateField::First()
{
    ImplNewFieldValue( maFirst );
    SpinField::First();
}

// -----------------------------------------------------------------------

void DateField::Last()
{
    ImplNewFieldValue( maLast );
    SpinField::Last();
}

// -----------------------------------------------------------------------

DateBox::DateBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    SetText( ImplGetLocaleDataWrapper().getDate( ImplGetFieldDate() ) );
    Reformat();
}

// -----------------------------------------------------------------------

DateBox::~DateBox()
{
}

// -----------------------------------------------------------------------

long DateBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && IsStrictFormat() &&
         ( GetExtDateFormat() != XTDATEF_SYSTEM_LONG ) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplDateProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), GetExtDateFormat( sal_True ), ImplGetLocaleDataWrapper() ) )
            return 1;
    }

    return ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void DateBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_LOCALE) )
    {
        if ( IsDefaultLocale() )
            ImplGetLocaleDataWrapper().setLocale( GetSettings().GetLocale() );
        ReformatAll();
    }
}

// -----------------------------------------------------------------------

long DateBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( sal_False );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() )
        {
            sal_Bool bTextLen = GetText().Len() != 0;
            if ( bTextLen || !IsEmptyFieldValueEnabled() )
                Reformat();
            else if ( !bTextLen && IsEmptyFieldValueEnabled() )
            {
                ResetLastDate();
                SetEmptyFieldValueData( sal_True );
            }
        }
    }

    return ComboBox::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void DateBox::Modify()
{
    MarkToBeReformatted( sal_True );
    ComboBox::Modify();
}

// -----------------------------------------------------------------------

void DateBox::ReformatAll()
{
    XubString aStr;
    SetUpdateMode( sal_False );
    sal_uInt16 nEntryCount = GetEntryCount();
    for ( sal_uInt16 i=0; i < nEntryCount; i++ )
    {
        ImplDateReformat( GetEntry( i ), aStr, GetFieldSettings() );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    DateFormatter::Reformat();
    SetUpdateMode( sal_True );
}

// -----------------------------------------------------------------------

static sal_Bool ImplTimeProcessKeyInput( Edit*, const KeyEvent& rKEvt,
                                     sal_Bool bStrictFormat, sal_Bool bDuration,
                                     TimeFieldFormat eFormat,
                                     const LocaleDataWrapper& rLocaleDataWrapper  )
{
    xub_Unicode cChar = rKEvt.GetCharCode();

    if ( !bStrictFormat )
        return sal_False;
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
             ((eFormat == TIMEF_100TH_SEC) && string::equals(rLocaleDataWrapper.getTime100SecSep(), cChar)) ||
             ((eFormat == TIMEF_SEC_CS) && string::equals(rLocaleDataWrapper.getTime100SecSep(), cChar)) ||
             (bDuration && (cChar == '-')) )
            return sal_False;
        else
            return sal_True;
    }
}

// -----------------------------------------------------------------------

static sal_Bool ImplIsOnlyDigits( const String& _rStr )
{
    const sal_Unicode* _pChr = _rStr.GetBuffer();
    for ( xub_StrLen i = 0; i < _rStr.Len(); ++i, ++_pChr )
    {
        if ( *_pChr < '0' || *_pChr > '9' )
            return sal_False;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

static sal_Bool ImplIsValidTimePortion( sal_Bool _bSkipInvalidCharacters, const String& _rStr )
{
    if ( !_bSkipInvalidCharacters )
    {
        if ( ( _rStr.Len() > 2 ) || ( _rStr.Len() < 1 ) || !ImplIsOnlyDigits( _rStr ) )
            return sal_False;
    }
    return sal_True;
}

// -----------------------------------------------------------------------

static sal_Bool ImplCutTimePortion( String& _rStr, xub_StrLen _nSepPos, sal_Bool _bSkipInvalidCharacters, short* _pPortion )
{
    String sPortion = _rStr.Copy( 0, _nSepPos );
    _rStr.Erase( 0, _nSepPos + 1 );

    if ( !ImplIsValidTimePortion( _bSkipInvalidCharacters, sPortion ) )
        return sal_False;
    *_pPortion = (short)sPortion.ToInt32();
    return sal_True;
}

// -----------------------------------------------------------------------

static sal_Bool ImplTimeGetValue( const XubString& rStr, Time& rTime,
                              TimeFieldFormat eFormat, sal_Bool bDuration,
                              const LocaleDataWrapper& rLocaleDataWrapper, sal_Bool _bSkipInvalidCharacters = sal_True )
{
    XubString   aStr    = rStr;
    short       nHour   = 0;
    short       nMinute = 0;
    short       nSecond = 0;
    short       n100Sec = 0;
    Time        aTime( 0, 0, 0 );

    if ( !rStr.Len() )
        return sal_False;

    // Search for separators
    if (!rLocaleDataWrapper.getTimeSep().isEmpty())
    {
        rtl::OUStringBuffer aSepStr(",.;:/");
        if ( !bDuration )
            aSepStr.append('-');

        // Replace characters above by the separator character
        for (sal_Int32 i = 0; i < aSepStr.getLength(); ++i)
        {
            if (string::equals(rLocaleDataWrapper.getTimeSep(), aSepStr[i]))
                continue;
            for ( xub_StrLen j = 0; j < aStr.Len(); j++ )
            {
                if (aStr.GetChar( j ) == aSepStr[i])
                    aStr.SetChar( j, rLocaleDataWrapper.getTimeSep()[0] );
            }
        }
    }

    sal_Bool bNegative = sal_False;
    xub_StrLen nSepPos = aStr.Search( rLocaleDataWrapper.getTimeSep() );
    if ( aStr.GetChar( 0 ) == '-' )
        bNegative = sal_True;
    if ( eFormat != TIMEF_SEC_CS )
    {
        if ( nSepPos == STRING_NOTFOUND )
            nSepPos = aStr.Len();
        if ( !ImplCutTimePortion( aStr, nSepPos, _bSkipInvalidCharacters, &nHour ) )
            return sal_False;

        nSepPos = aStr.Search( rLocaleDataWrapper.getTimeSep() );
        if ( aStr.GetChar( 0 ) == '-' )
            bNegative = sal_True;
        if ( nSepPos != STRING_NOTFOUND )
        {
            if ( !ImplCutTimePortion( aStr, nSepPos, _bSkipInvalidCharacters, &nMinute ) )
                return sal_False;

            nSepPos = aStr.Search( rLocaleDataWrapper.getTimeSep() );
            if ( aStr.GetChar( 0 ) == '-' )
                bNegative = sal_True;
            if ( nSepPos != STRING_NOTFOUND )
            {
                if ( !ImplCutTimePortion( aStr, nSepPos, _bSkipInvalidCharacters, &nSecond ) )
                    return sal_False;
                if ( aStr.GetChar( 0 ) == '-' )
                    bNegative = sal_True;
                n100Sec = (short)aStr.ToInt32();
            }
            else
                nSecond = (short)aStr.ToInt32();
        }
        else
            nMinute = (short)aStr.ToInt32();
    }
    else if ( nSepPos == STRING_NOTFOUND )
    {
        nSecond = (short)aStr.ToInt32();
        nMinute += nSecond / 60;
        nSecond %= 60;
        nHour += nMinute / 60;
        nMinute %= 60;
    }
    else
    {
        nSecond = (short)aStr.Copy( 0, nSepPos ).ToInt32();
        aStr.Erase( 0, nSepPos+1 );

        nSepPos = aStr.Search( rLocaleDataWrapper.getTimeSep() );
        if ( aStr.GetChar( 0 ) == '-' )
            bNegative = sal_True;
        if ( nSepPos != STRING_NOTFOUND )
        {
            nMinute = nSecond;
            nSecond = (short)aStr.Copy( 0, nSepPos ).ToInt32();
            aStr.Erase( 0, nSepPos+1 );

            nSepPos = aStr.Search( rLocaleDataWrapper.getTimeSep() );
            if ( aStr.GetChar( 0 ) == '-' )
                bNegative = sal_True;
            if ( nSepPos != STRING_NOTFOUND )
            {
                nHour   = nMinute;
                nMinute = nSecond;
                nSecond = (short)aStr.Copy( 0, nSepPos ).ToInt32();
                aStr.Erase( 0, nSepPos+1 );
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
        n100Sec = (short)aStr.ToInt32();

        if ( n100Sec )
        {
            xub_StrLen nLen = 1; // at least one digit, otherwise n100Sec==0

            while ( aStr.GetChar(nLen) >= '0' && aStr.GetChar(nLen) <= '9' )
                nLen++;

            if ( nLen > 2 )
            {
                while( nLen > 3 )
                {
                    n100Sec = n100Sec / 10;
                    nLen--;
                }
                // round if negative?
                n100Sec = (n100Sec + 5) / 10;
            }
            else
            {
                while( nLen < 2 )
                {
                    n100Sec = n100Sec * 10;
                    nLen++;
                }
            }
        }
    }

    if ( (nMinute > 59) || (nSecond > 59) || (n100Sec > 100) )
        return sal_False;

    if ( eFormat == TIMEF_NONE )
        nSecond = n100Sec = 0;
    else if ( eFormat == TIMEF_SEC )
        n100Sec = 0;

    if ( !bDuration )
    {
        if ( bNegative || (nHour < 0) || (nMinute < 0) ||
             (nSecond < 0) || (n100Sec < 0) )
            return sal_False;

        aStr.ToUpperAscii();
        XubString aAM( rLocaleDataWrapper.getTimeAM() );
        XubString aPM( rLocaleDataWrapper.getTimePM() );
        aAM.ToUpperAscii();
        aPM.ToUpperAscii();
        rtl::OUString aAM2("AM");  // aAM is localized
        rtl::OUString aPM2("PM");  // aPM is localized

        if ( (nHour < 12) && ( ( aStr.Search( aPM ) != STRING_NOTFOUND ) || ( aStr.Search( aPM2 ) != STRING_NOTFOUND ) ) )
            nHour += 12;

        if ( (nHour == 12) && ( ( aStr.Search( aAM ) != STRING_NOTFOUND ) || ( aStr.Search( aAM2 ) != STRING_NOTFOUND ) ) )
            nHour = 0;

        aTime = Time( (sal_uInt16)nHour, (sal_uInt16)nMinute, (sal_uInt16)nSecond,
                      (sal_uInt16)n100Sec );
    }
    else
    {
        if ( bNegative || (nHour < 0) || (nMinute < 0) ||
             (nSecond < 0) || (n100Sec < 0) )
        {
            bNegative   = sal_True;
            nHour       = nHour < 0 ? -nHour : nHour;
            nMinute     = nMinute < 0 ? -nMinute : nMinute;
            nSecond     = nSecond < 0 ? -nSecond : nSecond;
            n100Sec     = n100Sec < 0 ? -n100Sec : n100Sec;
        }

        aTime = Time( (sal_uInt16)nHour, (sal_uInt16)nMinute, (sal_uInt16)nSecond,
                      (sal_uInt16)n100Sec );
        if ( bNegative )
            aTime = -aTime;
    }

    rTime = aTime;

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool TimeFormatter::ImplTimeReformat( const XubString& rStr, XubString& rOutStr )
{
    Time aTime( 0, 0, 0 );
    if ( !ImplTimeGetValue( rStr, aTime, GetFormat(), IsDuration(), ImplGetLocaleDataWrapper() ) )
        return sal_True;

    Time aTempTime = aTime;
    if ( aTempTime > GetMax() )
        aTempTime = GetMax() ;
    else if ( aTempTime < GetMin() )
        aTempTime = GetMin();

    if ( GetErrorHdl().IsSet() && (aTime != aTempTime) )
    {
        maCorrectedTime = aTempTime;
        if ( !GetErrorHdl().Call( this ) )
        {
            maCorrectedTime = Time( Time::SYSTEM );
            return sal_False;
        }
        else
            maCorrectedTime = Time( Time::SYSTEM );
    }

    sal_Bool bSecond = sal_False;
    sal_Bool b100Sec = sal_False;
    if ( meFormat != TIMEF_NONE )
        bSecond = sal_True;
    if ( meFormat == TIMEF_100TH_SEC )
        b100Sec = sal_True;

    if ( meFormat == TIMEF_SEC_CS )
    {
        sal_uLong n  = aTempTime.GetHour() * 3600L;
        n       += aTempTime.GetMin()  * 60L;
        n       += aTempTime.GetSec();
        rOutStr  = String::CreateFromInt32( n );
        rOutStr += ImplGetLocaleDataWrapper().getTime100SecSep();
        if ( aTempTime.Get100Sec() < 10 )
            rOutStr += '0';
        rOutStr += String::CreateFromInt32( aTempTime.Get100Sec() );
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
                Time aT( aTempTime );
                aT.SetHour( aT.GetHour() % 12 );
                rOutStr = ImplGetLocaleDataWrapper().getTime( aT, bSecond, b100Sec );
            }
            // Don't use LocaleDataWrapper, we want AM/PM
            if ( aTempTime.GetHour() < 12 )
                rOutStr += String( RTL_CONSTASCII_USTRINGPARAM( "AM" ) ); // ImplGetLocaleDataWrapper().getTimeAM();
            else
                rOutStr += String( RTL_CONSTASCII_USTRINGPARAM( "PM" ) ); // ImplGetLocaleDataWrapper().getTimePM();
        }
    }

    return sal_True;
}

// -----------------------------------------------------------------------
sal_Bool TimeFormatter::ImplAllowMalformedInput() const
{
    return !IsEnforceValidValue();
}

// -----------------------------------------------------------------------

void TimeField::ImplTimeSpinArea( sal_Bool bUp )
{
    if ( GetField() )
    {
        xub_StrLen nTimeArea = 0;
        Time aTime( GetTime() );
        XubString aText( GetText() );
        Selection aSelection( GetField()->GetSelection() );

        // Area suchen
        if ( GetFormat() != TIMEF_SEC_CS )
        {
            for ( xub_StrLen i = 1, nPos = 0; i <= 4; i++ )
            {
                xub_StrLen nPos1 = aText.Search( ImplGetLocaleDataWrapper().getTimeSep(), nPos );
                xub_StrLen nPos2 = aText.Search( ImplGetLocaleDataWrapper().getTime100SecSep(), nPos );
                nPos = nPos1 < nPos2 ? nPos1 : nPos2;
                if ( nPos >= (xub_StrLen)aSelection.Max() )
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
            xub_StrLen nPos = aText.Search( ImplGetLocaleDataWrapper().getTime100SecSep() );
            if ( nPos == STRING_NOTFOUND || nPos >= (xub_StrLen)aSelection.Max() )
                nTimeArea = 3;
            else
                nTimeArea = 4;
        }

        if ( nTimeArea )
        {
            Time aAddTime( 0, 0, 0 );
            if ( nTimeArea == 1 )
                aAddTime = Time( 1, 0 );
            else if ( nTimeArea == 2 )
                aAddTime = Time( 0, 1 );
            else if ( nTimeArea == 3 )
                aAddTime = Time( 0, 0, 1 );
            else if ( nTimeArea == 4 )
                aAddTime = Time( 0, 0, 0, 1 );

            if ( !bUp )
                aAddTime = -aAddTime;

            aTime += aAddTime;
            if ( !IsDuration() )
            {
                Time aAbsMaxTime( 23, 59, 59, 99 );
                if ( aTime > aAbsMaxTime )
                    aTime = aAbsMaxTime;
                Time aAbsMinTime( 0, 0 );
                if ( aTime < aAbsMinTime )
                    aTime = aAbsMinTime;
            }
            ImplNewFieldValue( aTime );
        }

    }
}

// -----------------------------------------------------------------------

void TimeFormatter::ImplInit()
{
    meFormat        = TIMEF_NONE;
    mbDuration      = sal_False;
    mnTimeFormat    = HOUR_24;  // Should become a ExtTimeFieldFormat in next implementation, merge with mbDuration and meFormat
}

// -----------------------------------------------------------------------

TimeFormatter::TimeFormatter() :
    maLastTime( 0, 0 ),
    maMin( 0, 0 ),
    maMax( 23, 59, 59, 99 ),
    maCorrectedTime( Time::SYSTEM ),
    mbEnforceValidValue( sal_True ),
    maFieldTime( 0, 0 )
{
    ImplInit();
}

// -----------------------------------------------------------------------

void TimeFormatter::ImplLoadRes( const ResId& rResId )
{
    ResMgr* pMgr = rResId.GetResMgr();
    if( pMgr )
    {
        sal_uLong   nMask = pMgr->ReadLong();

        if ( TIMEFORMATTER_MIN & nMask )
        {
            SetMin( Time( ResId( (RSHEADER_TYPE *)pMgr->GetClass(), *pMgr ) ) );
            pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE *)pMgr->GetClass() ) );
        }

        if ( TIMEFORMATTER_MAX & nMask )
        {
            SetMax( Time( ResId( (RSHEADER_TYPE *)pMgr->GetClass(), *pMgr ) ) );
            pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE *)pMgr->GetClass() ) );
        }

        if ( TIMEFORMATTER_TIMEFIELDFORMAT & nMask )
            meFormat = (TimeFieldFormat)pMgr->ReadLong();

        if ( TIMEFORMATTER_DURATION & nMask )
            mbDuration = (sal_Bool)pMgr->ReadShort();

        if ( TIMEFORMATTER_STRICTFORMAT & nMask )
            SetStrictFormat( (sal_Bool)pMgr->ReadShort() );

        if ( TIMEFORMATTER_VALUE & nMask )
        {
            maFieldTime = Time( ResId( (RSHEADER_TYPE *)pMgr->GetClass(), *pMgr ) );
            if ( maFieldTime > GetMax() )
                maFieldTime = GetMax();
            if ( maFieldTime < GetMin() )
                maFieldTime = GetMin();
            maLastTime = maFieldTime;

            pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE *)pMgr->GetClass() ) );
        }
    }
}

// -----------------------------------------------------------------------

TimeFormatter::~TimeFormatter()
{
}

// -----------------------------------------------------------------------

void TimeFormatter::ReformatAll()
{
    Reformat();
}

// -----------------------------------------------------------------------

void TimeFormatter::SetMin( const Time& rNewMin )
{
    maMin = rNewMin;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

// -----------------------------------------------------------------------

void TimeFormatter::SetMax( const Time& rNewMax )
{
    maMax = rNewMax;
    if ( !IsEmptyFieldValue() )
        ReformatAll();
}

// -----------------------------------------------------------------------

void TimeFormatter::SetTimeFormat( TimeFormatter::TimeFormat eNewFormat )
{
    mnTimeFormat = sal::static_int_cast<sal_uInt16>(eNewFormat);
}

// -----------------------------------------------------------------------

TimeFormatter::TimeFormat TimeFormatter::GetTimeFormat() const
{
    return (TimeFormat)mnTimeFormat;
}

// -----------------------------------------------------------------------

void TimeFormatter::SetFormat( TimeFieldFormat eNewFormat )
{
    meFormat = eNewFormat;
    ReformatAll();
}

// -----------------------------------------------------------------------

void TimeFormatter::SetDuration( sal_Bool bNewDuration )
{
    mbDuration = bNewDuration;
    ReformatAll();
}

// -----------------------------------------------------------------------

void TimeFormatter::SetTime( const Time& rNewTime )
{
    SetUserTime( rNewTime );
    maFieldTime = maLastTime;
    SetEmptyFieldValueData( sal_False );
}

// -----------------------------------------------------------------------

void TimeFormatter::ImplNewFieldValue( const Time& rTime )
{
    if ( GetField() )
    {
        Selection aSelection = GetField()->GetSelection();
        aSelection.Justify();
        XubString aText = GetField()->GetText();

        // If selected until the end then keep it that way
        if ( (xub_StrLen)aSelection.Max() == aText.Len() )
        {
            if ( !aSelection.Len() )
                aSelection.Min() = SELECTION_MAX;
            aSelection.Max() = SELECTION_MAX;
        }

        Time aOldLastTime = maLastTime;
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

// -----------------------------------------------------------------------

void TimeFormatter::ImplSetUserTime( const Time& rNewTime, Selection* pNewSelection )
{
    Time aNewTime = rNewTime;
    if ( aNewTime > GetMax() )
        aNewTime = GetMax();
    else if ( aNewTime < GetMin() )
        aNewTime = GetMin();
    maLastTime = aNewTime;

    if ( GetField() )
    {
        XubString aStr;
        sal_Bool bSec    = sal_False;
        sal_Bool b100Sec = sal_False;
        if ( meFormat != TIMEF_NONE )
            bSec = sal_True;
        if ( meFormat == TIMEF_100TH_SEC || meFormat == TIMEF_SEC_CS )
            b100Sec = sal_True;
        if ( meFormat == TIMEF_SEC_CS )
        {
            sal_uLong n  = aNewTime.GetHour() * 3600L;
            n       += aNewTime.GetMin()  * 60L;
            n       += aNewTime.GetSec();
            aStr     = String::CreateFromInt32( n );
            aStr    += ImplGetLocaleDataWrapper().getTime100SecSep();
            if ( aNewTime.Get100Sec() < 10 )
                aStr += '0';
            aStr += String::CreateFromInt32( aNewTime.Get100Sec() );
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
                    Time aT( aNewTime );
                    aT.SetHour( aT.GetHour() % 12 );
                    aStr = ImplGetLocaleDataWrapper().getTime( aT, bSec, b100Sec );
                }
                // Don't use LocaleDataWrapper, we want AM/PM
                if ( aNewTime.GetHour() < 12 )
                    aStr += String( RTL_CONSTASCII_USTRINGPARAM( "AM" ) ); // ImplGetLocaleDataWrapper().getTimeAM();
                else
                    aStr += String( RTL_CONSTASCII_USTRINGPARAM( "PM" ) ); // ImplGetLocaleDataWrapper().getTimePM();
            }
        }

        ImplSetText( aStr, pNewSelection );
    }
}

// -----------------------------------------------------------------------

void TimeFormatter::SetUserTime( const Time& rNewTime )
{
    ImplSetUserTime( rNewTime );
}

// -----------------------------------------------------------------------

Time TimeFormatter::GetTime() const
{
    Time aTime( 0, 0, 0 );

    if ( GetField() )
    {
        sal_Bool bAllowMailformed = ImplAllowMalformedInput();
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

// -----------------------------------------------------------------------

void TimeFormatter::Reformat()
{
    if ( !GetField() )
        return;

    if ( !GetField()->GetText().Len() && ImplGetEmptyFieldValue() )
        return;

    XubString aStr;
    sal_Bool bOK = ImplTimeReformat( GetField()->GetText(), aStr );
    if ( !bOK )
        return;

    if ( aStr.Len() )
    {
        ImplSetText( aStr );
        ImplTimeGetValue( aStr, maLastTime, GetFormat(), IsDuration(), ImplGetLocaleDataWrapper() );
    }
    else
        SetTime( maLastTime );
}

// -----------------------------------------------------------------------

TimeField::TimeField( Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle ),
    maFirst( GetMin() ),
    maLast( GetMax() )
{
    SetField( this );
    SetText( ImplGetLocaleDataWrapper().getTime( maFieldTime, sal_False, sal_False ) );
    Reformat();
}

// -----------------------------------------------------------------------

TimeField::TimeField( Window* pParent, const ResId& rResId ) :
    SpinField( WINDOW_TIMEFIELD ),
    maFirst( GetMin() ),
    maLast( GetMax() )
{
    rResId.SetRT( RSC_TIMEFIELD );
    WinBits nStyle = ImplInitRes( rResId );
    SpinField::ImplInit( pParent, nStyle );
    SetField( this );
    SetText( ImplGetLocaleDataWrapper().getTime( maFieldTime, sal_False, sal_False ) );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

void TimeField::ImplLoadRes( const ResId& rResId )
{
    SpinField::ImplLoadRes( rResId );
    ResMgr* pMgr = rResId.GetResMgr();
    if( pMgr )
    {
        TimeFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes(), *pMgr ) );

        sal_uLong      nMask = ReadLongRes();

        if ( TIMEFIELD_FIRST & nMask )
        {
            maFirst = Time( ResId( (RSHEADER_TYPE *)GetClassRes(), *pMgr ) );
            IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
        }
        if ( TIMEFIELD_LAST & nMask )
        {
            maLast = Time( ResId( (RSHEADER_TYPE *)GetClassRes(), *pMgr ) );
            IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
        }
    }

    Reformat();
}

// -----------------------------------------------------------------------

TimeField::~TimeField()
{
}

// -----------------------------------------------------------------------

long TimeField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplTimeProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsDuration(), GetFormat(), ImplGetLocaleDataWrapper() ) )
            return 1;
    }

    return SpinField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long TimeField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( sal_False );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (GetText().Len() || !IsEmptyFieldValueEnabled()) )
        {
            if ( !ImplAllowMalformedInput() )
                Reformat();
            else
            {
                Time aTime( 0, 0, 0 );
                if ( ImplTimeGetValue( GetText(), aTime, GetFormat(), IsDuration(), ImplGetLocaleDataWrapper(), sal_False ) )
                    // even with strict text analysis, our text is a valid time -> do a complete
                    // reformat
                    Reformat();
            }
        }
    }

    return SpinField::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void TimeField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_LOCALE) )
    {
        if ( IsDefaultLocale() )
            ImplGetLocaleDataWrapper().setLocale( GetSettings().GetLocale() );
        ReformatAll();
    }
}

// -----------------------------------------------------------------------

void TimeField::Modify()
{
    MarkToBeReformatted( sal_True );
    SpinField::Modify();
}

// -----------------------------------------------------------------------

void TimeField::Up()
{
    ImplTimeSpinArea( sal_True );
    SpinField::Up();
}

// -----------------------------------------------------------------------

void TimeField::Down()
{
    ImplTimeSpinArea( sal_False );
    SpinField::Down();
}

// -----------------------------------------------------------------------

void TimeField::First()
{
    ImplNewFieldValue( maFirst );
    SpinField::First();
}

// -----------------------------------------------------------------------

void TimeField::Last()
{
    ImplNewFieldValue( maLast );
    SpinField::Last();
}

// -----------------------------------------------------------------------

void TimeField::SetExtFormat( ExtTimeFieldFormat eFormat )
{
    switch ( eFormat )
    {
        case EXTTIMEF_24H_SHORT:
        {
            SetTimeFormat( HOUR_24 );
            SetDuration( sal_False );
            SetFormat( TIMEF_NONE );
        }
        break;
        case EXTTIMEF_24H_LONG:
        {
            SetTimeFormat( HOUR_24 );
            SetDuration( sal_False );
            SetFormat( TIMEF_SEC );
        }
        break;
        case EXTTIMEF_12H_SHORT:
        {
            SetTimeFormat( HOUR_12 );
            SetDuration( sal_False );
            SetFormat( TIMEF_NONE );
        }
        break;
        case EXTTIMEF_12H_LONG:
        {
            SetTimeFormat( HOUR_12 );
            SetDuration( sal_False );
            SetFormat( TIMEF_SEC );
        }
        break;
        case EXTTIMEF_DURATION_SHORT:
        {
            SetDuration( sal_True );
            SetFormat( TIMEF_NONE );
        }
        break;
        case EXTTIMEF_DURATION_LONG:
        {
            SetDuration( sal_True );
            SetFormat( TIMEF_SEC );
        }
        break;
        default:    OSL_FAIL( "ExtTimeFieldFormat unknown!" );
    }

    if ( GetField() && GetField()->GetText().Len() )
        SetUserTime( GetTime() );
    ReformatAll();
}

// -----------------------------------------------------------------------

TimeBox::TimeBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    SetText( ImplGetLocaleDataWrapper().getTime( maFieldTime, sal_False, sal_False ) );
    Reformat();
}

// -----------------------------------------------------------------------

TimeBox::~TimeBox()
{
}

// -----------------------------------------------------------------------

long TimeBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplTimeProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsDuration(), GetFormat(), ImplGetLocaleDataWrapper() ) )
            return 1;
    }

    return ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long TimeBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( sal_False );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (GetText().Len() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void TimeBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_LOCALE) )
    {
        if ( IsDefaultLocale() )
            ImplGetLocaleDataWrapper().setLocale( GetSettings().GetLocale() );
        ReformatAll();
    }
}

// -----------------------------------------------------------------------

void TimeBox::Modify()
{
    MarkToBeReformatted( sal_True );
    ComboBox::Modify();
}

// -----------------------------------------------------------------------

void TimeBox::ReformatAll()
{
    XubString aStr;
    SetUpdateMode( sal_False );
    sal_uInt16 nEntryCount = GetEntryCount();
    for ( sal_uInt16 i=0; i < nEntryCount; i++ )
    {
        ImplTimeReformat( GetEntry( i ), aStr );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    TimeFormatter::Reformat();
    SetUpdateMode( sal_True );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
