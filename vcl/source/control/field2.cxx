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

#include <sal/config.h>

#include <algorithm>
#include <string_view>

#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <officecfg/Office/Common.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include <vcl/toolkit/field.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/settings.hxx>
#include <vcl/weldutils.hxx>

#include <svdata.hxx>

#include <com/sun/star/i18n/XCharacterClassification.hpp>

#include <unotools/localedatawrapper.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/charclass.hxx>

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

uno::Reference< i18n::XCharacterClassification > const & ImplGetCharClass()
{
    ImplSVData *const pSVData = ImplGetSVData();
    assert(pSVData);

    if (!pSVData->m_xCharClass.is())
    {
        pSVData->m_xCharClass = vcl::unohelper::CreateCharacterClassification();
    }

    return pSVData->m_xCharClass;
}

static sal_Unicode* ImplAddString( sal_Unicode* pBuf, const OUString& rStr )
{
    memcpy( pBuf, rStr.getStr(), rStr.getLength() * sizeof(sal_Unicode) );
    pBuf += rStr.getLength();
    return pBuf;
}

static sal_Unicode* ImplAddNum( sal_Unicode* pBuf, sal_uLong nNumber, int nMinLen )
{
    // fill temp buffer with digits
    sal_Unicode aTempBuf[30];
    sal_Unicode* pTempBuf = aTempBuf;
    do
    {
        *pTempBuf = static_cast<sal_Unicode>(nNumber % 10) + '0';
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

static sal_Unicode* ImplAddSNum( sal_Unicode* pBuf, sal_Int32 nNumber, int nMinLen )
{
    if (nNumber < 0)
    {
        *pBuf++ = '-';
        nNumber = -nNumber;
    }
    return ImplAddNum( pBuf, nNumber, nMinLen);
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

static bool ImplIsPatternChar( sal_Unicode cChar, char cEditMask )
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
        DBG_UNHANDLED_EXCEPTION("vcl.control");
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

static sal_Unicode ImplPatternChar( sal_Unicode cChar, char cEditMask )
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

    OUStringBuffer    aOutStr = rLiteralMask;
    sal_Unicode cTempChar;
    sal_Unicode cChar;
    sal_Unicode cLiteral;
    char    cMask;
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
                // Otherwise we check if it is an invalid character. This is the case if it does not
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

static OUString ImplPatternProcessStrictModify(const OUString& rText,
                                               const OString& rEditMask,
                                               const OUString& rLiteralMask,
                                               bool bSameMask)
{
    OUString aText(rText);

    // remove leading blanks
    if (bSameMask && !rEditMask.isEmpty())
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

    return ImplPatternReformat(aText, rEditMask, rLiteralMask, 0);
}

static void ImplPatternProcessStrictModify( Edit* pEdit,
                                            const OString& rEditMask,
                                            const OUString& rLiteralMask,
                                            bool bSameMask )
{
    OUString aText = pEdit->GetText();
    OUString aNewText = ImplPatternProcessStrictModify(aText,
                                                       rEditMask,
                                                       rLiteralMask,
                                                       bSameMask);

    if ( aNewText == aText )
        return;

    // adjust selection such that it remains at the end if it was there before
    Selection aSel = pEdit->GetSelection();
    sal_Int64 nMaxSel = std::max( aSel.Min(), aSel.Max() );
    if ( nMaxSel >= aText.getLength() )
    {
        sal_Int32 nMaxPos = aNewText.getLength();
        ImplPatternMaxPos(aNewText, rEditMask, 0, bSameMask, nMaxSel, nMaxPos);
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

static void ImplPatternProcessStrictModify( weld::Entry& rEntry,
                                            const OString& rEditMask,
                                            const OUString& rLiteralMask,
                                            bool bSameMask )
{
    OUString aText = rEntry.get_text();
    OUString aNewText = ImplPatternProcessStrictModify(aText,
                                                       rEditMask,
                                                       rLiteralMask,
                                                       bSameMask);

    if (aNewText == aText)
        return;

    // adjust selection such that it remains at the end if it was there before
    int nStartPos, nEndPos;
    rEntry.get_selection_bounds(nStartPos, nEndPos);

    int nMaxSel = std::max(nStartPos, nEndPos);
    if (nMaxSel >= aText.getLength())
    {
        sal_Int32 nMaxPos = aNewText.getLength();
        ImplPatternMaxPos(aNewText, rEditMask, 0, bSameMask, nMaxSel, nMaxPos);
        if (nStartPos == nEndPos)
        {
            nStartPos = nMaxPos;
            nEndPos = nMaxPos;
        }
        else if (nStartPos > nMaxPos)
            nStartPos = nMaxPos;
        else
            nEndPos = nMaxPos;
    }
    rEntry.set_text(aNewText);
    rEntry.select_region(nStartPos, nEndPos);
}

static sal_Int32 ImplPatternLeftPos(std::string_view rEditMask, sal_Int32 nCursorPos)
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

namespace
{
    class IEditImplementation
    {
    public:
        virtual ~IEditImplementation() {}

        virtual OUString GetText() const = 0;
        virtual void SetText(const OUString& rStr, const Selection& rSelection) = 0;

        virtual Selection GetSelection() const = 0;
        virtual void SetSelection(const Selection& rSelection) = 0;

        virtual bool IsInsertMode() const = 0;

        virtual void SetModified() = 0;
    };
}

static bool ImplPatternProcessKeyInput( IEditImplementation& rEdit, const KeyEvent& rKEvt,
                                        const OString& rEditMask,
                                        const OUString& rLiteralMask,
                                        bool bStrictFormat,
                                        bool bSameMask,
                                        bool& rbInKeyInput )
{
    if ( rEditMask.isEmpty() || !bStrictFormat )
        return false;

    sal_uInt16 nFormatFlags = 0;
    Selection   aOldSel     = rEdit.GetSelection();
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
            rEdit.SetSelection( aSel );
            return true;
        }
        else if ( nKeyCode == KEY_RIGHT )
        {
            // Use the start of selection as minimum; even a small position is allowed in case that
            // all was selected by the focus
            Selection aSel( aOldSel );
            aSel.Justify();
            nCursorPos = aSel.Min();
            aSel.Max() = ImplPatternRightPos( rEdit.GetText(), rEditMask, nFormatFlags, bSameMask, nCursorPos );
            if ( bShift )
                aSel.Min() = aOldSel.Min();
            else
                aSel.Min() = aSel.Max();
            rEdit.SetSelection( aSel );
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
            rEdit.SetSelection( aSel );
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
            ImplPatternMaxPos( rEdit.GetText(), rEditMask, nFormatFlags, bSameMask, nCursorPos, nNewPos );
            aSel.Max() = nNewPos;
            if ( bShift )
                aSel.Min() = aOldSel.Min();
            else
                aSel.Min() = aSel.Max();
            rEdit.SetSelection( aSel );
            return true;
        }
        else if ( (nKeyCode == KEY_BACKSPACE) || (nKeyCode == KEY_DELETE) )
        {
            OUString          aOldStr( rEdit.GetText() );
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
                rEdit.SetText( aStr.toString(), Selection( nNewPos ) );
                rEdit.SetModified();
                rbInKeyInput = false;
            }
            else
                rEdit.SetSelection( Selection( nNewPos ) );

            return true;
        }
        else if ( nKeyCode == KEY_INSERT )
        {
            // you can only set InsertMode for a PatternField if the
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
                            ImplPatternMaxPos( rEdit.GetText(), rEditMask, nFormatFlags, bSameMask, nNewPos, nTempPos );
                            if ( nTempPos > nNewPos )
                            {
                                rEdit.SetSelection( Selection( nTempPos ) );
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
        OUStringBuffer  aStr = rEdit.GetText();
        bool        bError = false;
        if ( bSameMask && rEdit.IsInsertMode() )
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
                    aStr.append( rLiteralMask.subView(aStr.getLength(), nNewPos-aStr.getLength()) );
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
            rEdit.SetText( aStr.toString(), aNewSel );
            rEdit.SetModified();
            rbInKeyInput = false;
        }
    }

    return true;
}

namespace
{
    bool ImplSetMask(const OString& rEditMask, OUString& rLiteralMask)
    {
        bool bSameMask      = true;

        if (rEditMask.getLength() != rLiteralMask.getLength())
        {
            OUStringBuffer aBuf(rLiteralMask);
            if (rEditMask.getLength() < aBuf.getLength())
                aBuf.remove(rEditMask.getLength(), aBuf.getLength() - rEditMask.getLength());
            else
                comphelper::string::padToLength(aBuf, rEditMask.getLength(), ' ');
            rLiteralMask = aBuf.makeStringAndClear();
        }

        // Strict mode allows only the input mode if only equal characters are allowed as mask and if
        // only spaces are specified which are not allowed by the mask
        sal_Int32   i = 0;
        char    c = 0;
        while ( i < rEditMask.getLength() )
        {
            char cTemp = rEditMask[i];
            if ( cTemp != EDITMASK_LITERAL )
            {
                if ( (cTemp == EDITMASK_ALLCHAR) ||
                     (cTemp == EDITMASK_UPPERALLCHAR) ||
                     (cTemp == EDITMASK_NUMSPACE) )
                {
                    bSameMask = false;
                    break;
                }
                if ( i < rLiteralMask.getLength() )
                {
                    if ( rLiteralMask[i] != ' ' )
                    {
                        bSameMask = false;
                        break;
                    }
                }
                if ( !c )
                    c = cTemp;
                if ( cTemp != c )
                {
                    bSameMask = false;
                    break;
                }
            }
            i++;
        }

        return bSameMask;
    }
}

PatternFormatter::PatternFormatter(Edit* pEdit)
    : FormatterBase(pEdit)
{
    mbSameMask          = true;
    mbInPattKeyInput    = false;
}

PatternFormatter::~PatternFormatter()
{
}

void PatternFormatter::SetMask( const OString& rEditMask,
                                const OUString& rLiteralMask )
{
    m_aEditMask = rEditMask;
    maLiteralMask = rLiteralMask;
    mbSameMask = ImplSetMask(m_aEditMask, maLiteralMask);
    ReformatAll();
}

namespace
{
    class EntryImplementation : public IEditImplementation
    {
    public:
        EntryImplementation(weld::PatternFormatter& rFormatter)
            : m_rFormatter(rFormatter)
            , m_rEntry(rFormatter.get_widget())
        {
        }

        virtual OUString GetText() const override
        {
            return m_rEntry.get_text();
        }

        virtual void SetText(const OUString& rStr, const Selection& rSelection) override
        {
            m_rEntry.set_text(rStr);
            SetSelection(rSelection);
        }

        virtual Selection GetSelection() const override
        {
            int nStartPos, nEndPos;
            m_rEntry.get_selection_bounds(nStartPos, nEndPos);
            return Selection(nStartPos, nEndPos);
        }

        virtual void SetSelection(const Selection& rSelection) override
        {
            auto nMin = rSelection.Min();
            auto nMax = rSelection.Max();
            m_rEntry.select_region(nMin < 0 ? 0 : nMin, nMax == SELECTION_MAX ? -1 : nMax);
        }

        virtual bool IsInsertMode() const override
        {
            return !m_rEntry.get_overwrite_mode();
        }

        virtual void SetModified() override
        {
            m_rFormatter.Modify();
        }

    private:
        weld::PatternFormatter& m_rFormatter;
        weld::Entry& m_rEntry;
    };
}

namespace weld
{
    void PatternFormatter::SetStrictFormat(bool bStrict)
    {
        if (bStrict != m_bStrictFormat)
        {
            m_bStrictFormat = bStrict;
            if (m_bStrictFormat)
                ReformatAll();
        }
    }

    void PatternFormatter::SetMask(const OString& rEditMask,
                                   const OUString& rLiteralMask)
    {
        m_aEditMask = rEditMask;
        m_aLiteralMask = rLiteralMask;
        m_bSameMask = ImplSetMask(m_aEditMask, m_aLiteralMask);
        ReformatAll();
    }

    void PatternFormatter::ReformatAll()
    {
        m_rEntry.set_text(ImplPatternReformat(m_rEntry.get_text(), m_aEditMask, m_aLiteralMask, 0/*nFormatFlags*/));
        if (!m_bSameMask && m_bStrictFormat && m_rEntry.get_editable())
            m_rEntry.set_overwrite_mode(true);
    }

    void PatternFormatter::EntryGainFocus()
    {
        m_bReformat = false;
    }

    void PatternFormatter::EntryLostFocus()
    {
        if (m_bReformat)
            ReformatAll();
    }

    void PatternFormatter::Modify()
    {
        if (!m_bInPattKeyInput)
        {
            if (m_bStrictFormat)
                ImplPatternProcessStrictModify(m_rEntry, m_aEditMask, m_aLiteralMask, m_bSameMask);
            else
                m_bReformat = true;
        }
        m_aModifyHdl.Call(m_rEntry);
    }

    IMPL_LINK(PatternFormatter, KeyInputHdl, const KeyEvent&, rKEvt, bool)
    {
        if (m_aKeyPressHdl.Call(rKEvt))
            return true;
        if (rKEvt.GetKeyCode().IsMod2())
            return false;
        EntryImplementation aAdapt(*this);
        return ImplPatternProcessKeyInput(aAdapt, rKEvt, m_aEditMask, m_aLiteralMask,
                                          m_bStrictFormat,
                                          m_bSameMask, m_bInPattKeyInput);
    }
}

void PatternFormatter::SetString( const OUString& rStr )
{
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
        return ImplPatternReformat( GetField()->GetText(), m_aEditMask, maLiteralMask, 0/*nFormatFlags*/ );
}

void PatternFormatter::Reformat()
{
    if ( GetField() )
    {
        ImplSetText( ImplPatternReformat( GetField()->GetText(), m_aEditMask, maLiteralMask, 0/*nFormatFlags*/ ) );
        if ( !mbSameMask && IsStrictFormat() && !GetField()->IsReadOnly() )
            GetField()->SetInsertMode( false );
    }
}

PatternField::PatternField(vcl::Window* pParent, WinBits nWinStyle)
    : SpinField(pParent, nWinStyle)
    , PatternFormatter(this)
{
    Reformat();
}

void PatternField::dispose()
{
    ClearField();
    SpinField::dispose();
}

namespace
{
    class EditImplementation : public IEditImplementation
    {
    public:
        EditImplementation(Edit& rEdit)
            : m_rEdit(rEdit)
        {
        }

        virtual OUString GetText() const override
        {
            return m_rEdit.GetText();
        }

        virtual void SetText(const OUString& rStr, const Selection& rSelection) override
        {
            m_rEdit.SetText(rStr, rSelection);
        }

        virtual Selection GetSelection() const override
        {
            return m_rEdit.GetSelection();
        }

        virtual void SetSelection(const Selection& rSelection) override
        {
            m_rEdit.SetSelection(rSelection);
        }

        virtual bool IsInsertMode() const override
        {
            return m_rEdit.IsInsertMode();
        }

        virtual void SetModified() override
        {
            m_rEdit.SetModifyFlag();
            m_rEdit.Modify();
        }

    private:
        Edit& m_rEdit;
    };
}

bool PatternField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        EditImplementation aAdapt(*GetField());
        if ( ImplPatternProcessKeyInput( aAdapt, *rNEvt.GetKeyEvent(), GetEditMask(), GetLiteralMask(),
                                         IsStrictFormat(),
                                         ImplIsSameMask(), ImplGetInPattKeyInput() ) )
            return true;
    }

    return SpinField::PreNotify( rNEvt );
}

bool PatternField::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return SpinField::EventNotify( rNEvt );
}

void PatternField::Modify()
{
    if ( !ImplGetInPattKeyInput() )
    {
        if ( IsStrictFormat() )
            ImplPatternProcessStrictModify( GetField(), GetEditMask(), GetLiteralMask(), ImplIsSameMask() );
        else
            MarkToBeReformatted( true );
    }

    SpinField::Modify();
}

PatternBox::PatternBox(vcl::Window* pParent, WinBits nWinStyle)
    : ComboBox( pParent, nWinStyle )
    , PatternFormatter(this)
{
    Reformat();
}

void PatternBox::dispose()
{
    ClearField();
    ComboBox::dispose();
}

bool PatternBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        EditImplementation aAdapt(*GetField());
        if ( ImplPatternProcessKeyInput( aAdapt, *rNEvt.GetKeyEvent(), GetEditMask(), GetLiteralMask(),
                                         IsStrictFormat(),
                                         ImplIsSameMask(), ImplGetInPattKeyInput() ) )
            return true;
    }

    return ComboBox::PreNotify( rNEvt );
}

bool PatternBox::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::EventNotify( rNEvt );
}

void PatternBox::Modify()
{
    if ( !ImplGetInPattKeyInput() )
    {
        if ( IsStrictFormat() )
            ImplPatternProcessStrictModify( GetField(), GetEditMask(), GetLiteralMask(), ImplIsSameMask() );
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
        aStr = ImplPatternReformat( GetEntry( i ), GetEditMask(), GetLiteralMask(), 0/*nFormatFlags*/ );
        RemoveEntryAt(i);
        InsertEntry( aStr, i );
    }
    PatternFormatter::Reformat();
    SetUpdateMode( true );
}

static ExtDateFieldFormat ImplGetExtFormat( DateOrder eOld )
{
    switch( eOld )
    {
        case DateOrder::DMY:   return ExtDateFieldFormat::ShortDDMMYY;
        case DateOrder::MDY:   return ExtDateFieldFormat::ShortMMDDYY;
        default:               return ExtDateFieldFormat::ShortYYMMDD;
    }
}

static sal_uInt16 ImplCutNumberFromString( OUString& rStr )
{
    sal_Int32 i1 = 0;
    while (i1 != rStr.getLength() && (rStr[i1] < '0' || rStr[i1] > '9')) {
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

static bool ImplCutMonthName( OUString& rStr, std::u16string_view _rLookupMonthName )
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
    if ( ( eFormat == ExtDateFieldFormat::ShortYYMMDD_DIN5008 ) || ( eFormat == ExtDateFieldFormat::ShortYYYYMMDD_DIN5008 ) )
        return "-";
    else
        return rLocaleDataWrapper.getDateSep();
}

static bool ImplDateProcessKeyInput( const KeyEvent& rKEvt, ExtDateFieldFormat eFormat,
                                     const LocaleDataWrapper& rLocaleDataWrapper  )
{
    sal_Unicode cChar = rKEvt.GetCharCode();
    sal_uInt16 nGroup = rKEvt.GetKeyCode().GetGroup();
    return !((nGroup == KEYGROUP_FKEYS) ||
             (nGroup == KEYGROUP_CURSOR) ||
             (nGroup == KEYGROUP_MISC)||
             ((cChar >= '0') && (cChar <= '9')) ||
             (cChar == ImplGetDateSep( rLocaleDataWrapper, eFormat )[0]));
}

bool DateFormatter::TextToDate(const OUString& rStr, Date& rDate, ExtDateFieldFormat eDateOrder,
                               const LocaleDataWrapper& rLocaleDataWrapper, const CalendarWrapper& rCalendarWrapper)
{
    sal_uInt16 nDay = 0;
    sal_uInt16 nMonth = 0;
    sal_uInt16 nYear = 0;
    bool bError = false;
    OUString aStr( rStr );

    if ( eDateOrder == ExtDateFieldFormat::SystemLong )
    {
        DateOrder eFormat = rLocaleDataWrapper.getLongDateOrder();
        switch( eFormat )
        {
            case DateOrder::MDY:
                nMonth = ImplCutMonthFromString( aStr, rCalendarWrapper );
                nDay = ImplCutNumberFromString( aStr );
                nYear  = ImplCutNumberFromString( aStr );
                break;
            case DateOrder::DMY:
                nDay = ImplCutNumberFromString( aStr );
                nMonth = ImplCutMonthFromString( aStr, rCalendarWrapper );
                nYear  = ImplCutNumberFromString( aStr );
                break;
            case DateOrder::YMD:
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
        OUString aDateSep = ImplGetDateSep( rLocaleDataWrapper, eDateOrder );
        sal_Int32 nSepPos = aStr.indexOf( aDateSep );
        if ( nSepPos < 0 )
            return false;
        nSepPos = aStr.indexOf( aDateSep, nSepPos+1 );
        if ( ( nSepPos < 0 ) || ( nSepPos == (aStr.getLength()-1) ) )
        {
            bYear = false;
            nYear = Date( Date::SYSTEM ).GetYearUnsigned();
        }

        const sal_Unicode* pBuf = aStr.getStr();
        ImplSkipDelimiters( pBuf );

        switch ( eDateOrder )
        {
            case ExtDateFieldFormat::ShortDDMMYY:
            case ExtDateFieldFormat::ShortDDMMYYYY:
            {
                nDay = ImplGetNum( pBuf, bError );
                ImplSkipDelimiters( pBuf );
                nMonth = ImplGetNum( pBuf, bError );
                ImplSkipDelimiters( pBuf );
                if ( bYear )
                    nYear = ImplGetNum( pBuf, bError );
            }
            break;
            case ExtDateFieldFormat::ShortMMDDYY:
            case ExtDateFieldFormat::ShortMMDDYYYY:
            {
                nMonth = ImplGetNum( pBuf, bError );
                ImplSkipDelimiters( pBuf );
                nDay = ImplGetNum( pBuf, bError );
                ImplSkipDelimiters( pBuf );
                if ( bYear )
                    nYear = ImplGetNum( pBuf, bError );
            }
            break;
            case ExtDateFieldFormat::ShortYYMMDD:
            case ExtDateFieldFormat::ShortYYYYMMDD:
            case ExtDateFieldFormat::ShortYYMMDD_DIN5008:
            case ExtDateFieldFormat::ShortYYYYMMDD_DIN5008:
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
                OSL_FAIL( "DateOrder???" );
            }
        }
    }

    if ( bError || !nDay || !nMonth )
        return false;

    Date aNewDate( nDay, nMonth, nYear );
    DateFormatter::ExpandCentury( aNewDate, officecfg::Office::Common::DateFormat::TwoDigitYear::get() );
    if ( aNewDate.IsValidDate() )
    {
        rDate = aNewDate;
        return true;
    }
    return false;
}

void DateFormatter::ImplDateReformat( const OUString& rStr, OUString& rOutStr )
{
    Date aDate( Date::EMPTY );
    if (!TextToDate(rStr, aDate, GetExtDateFormat(true), ImplGetLocaleDataWrapper(), GetCalendarWrapper()))
        return;

    Date aTempDate = aDate;
    if ( aTempDate > GetMax() )
        aTempDate = GetMax();
    else if ( aTempDate < GetMin() )
        aTempDate = GetMin();

    rOutStr = ImplGetDateAsText( aTempDate );
}

namespace
{
    ExtDateFieldFormat ResolveSystemFormat(ExtDateFieldFormat eDateFormat, const LocaleDataWrapper& rLocaleData)
    {
        if (eDateFormat <= ExtDateFieldFormat::SystemShortYYYY)
        {
            bool bShowCentury = (eDateFormat == ExtDateFieldFormat::SystemShortYYYY);
            switch (rLocaleData.getDateOrder())
            {
                case DateOrder::DMY:
                    eDateFormat = bShowCentury ? ExtDateFieldFormat::ShortDDMMYYYY : ExtDateFieldFormat::ShortDDMMYY;
                    break;
                case DateOrder::MDY:
                    eDateFormat = bShowCentury ? ExtDateFieldFormat::ShortMMDDYYYY : ExtDateFieldFormat::ShortMMDDYY;
                    break;
                default:
                    eDateFormat = bShowCentury ? ExtDateFieldFormat::ShortYYYYMMDD : ExtDateFieldFormat::ShortYYMMDD;
            }
        }
        return eDateFormat;
    }
}

OUString DateFormatter::FormatDate(const Date& rDate, ExtDateFieldFormat eExtFormat,
                                   const LocaleDataWrapper& rLocaleData, CalendarWrapper& rCalendarWrapper)
{
    bool bShowCentury = false;
    switch (eExtFormat)
    {
        case ExtDateFieldFormat::SystemShortYYYY:
        case ExtDateFieldFormat::SystemLong:
        case ExtDateFieldFormat::ShortDDMMYYYY:
        case ExtDateFieldFormat::ShortMMDDYYYY:
        case ExtDateFieldFormat::ShortYYYYMMDD:
        case ExtDateFieldFormat::ShortYYYYMMDD_DIN5008:
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
        sal_uInt16 nTwoDigitYearStart = officecfg::Office::Common::DateFormat::TwoDigitYear::get();
        sal_uInt16 nYear = rDate.GetYearUnsigned();

        // If year is not in double digit range
        if ( (nYear < nTwoDigitYearStart) || (nYear >= nTwoDigitYearStart+100) )
            bShowCentury = true;
    }

    sal_Unicode aBuf[128];
    sal_Unicode* pBuf = aBuf;

    eExtFormat = ResolveSystemFormat(eExtFormat, rLocaleData);

    OUString aDateSep = ImplGetDateSep( rLocaleData, eExtFormat );
    sal_uInt16 nDay = rDate.GetDay();
    sal_uInt16 nMonth = rDate.GetMonth();
    sal_Int16 nYear = rDate.GetYear();
    sal_uInt16 nYearLen = bShowCentury ? 4 : 2;

    if ( !bShowCentury )
        nYear %= 100;

    switch (eExtFormat)
    {
        case ExtDateFieldFormat::SystemLong:
        {
            return rLocaleData.getLongDate( rDate, rCalendarWrapper, !bShowCentury );
        }
        case ExtDateFieldFormat::ShortDDMMYY:
        case ExtDateFieldFormat::ShortDDMMYYYY:
        {
            pBuf = ImplAddNum( pBuf, nDay, 2 );
            pBuf = ImplAddString( pBuf, aDateSep );
            pBuf = ImplAddNum( pBuf, nMonth, 2 );
            pBuf = ImplAddString( pBuf, aDateSep );
            pBuf = ImplAddSNum( pBuf, nYear, nYearLen );
        }
        break;
        case ExtDateFieldFormat::ShortMMDDYY:
        case ExtDateFieldFormat::ShortMMDDYYYY:
        {
            pBuf = ImplAddNum( pBuf, nMonth, 2 );
            pBuf = ImplAddString( pBuf, aDateSep );
            pBuf = ImplAddNum( pBuf, nDay, 2 );
            pBuf = ImplAddString( pBuf, aDateSep );
            pBuf = ImplAddSNum( pBuf, nYear, nYearLen );
        }
        break;
        case ExtDateFieldFormat::ShortYYMMDD:
        case ExtDateFieldFormat::ShortYYYYMMDD:
        case ExtDateFieldFormat::ShortYYMMDD_DIN5008:
        case ExtDateFieldFormat::ShortYYYYMMDD_DIN5008:
        {
            pBuf = ImplAddSNum( pBuf, nYear, nYearLen );
            pBuf = ImplAddString( pBuf, aDateSep );
            pBuf = ImplAddNum( pBuf, nMonth, 2 );
            pBuf = ImplAddString( pBuf, aDateSep );
            pBuf = ImplAddNum( pBuf, nDay, 2 );
        }
        break;
        default:
        {
            OSL_FAIL( "DateOrder???" );
        }
    }

    return OUString(aBuf, pBuf-aBuf);
}

OUString DateFormatter::ImplGetDateAsText( const Date& rDate ) const
{
    return DateFormatter::FormatDate(rDate, GetExtDateFormat(), ImplGetLocaleDataWrapper(), GetCalendarWrapper());
}

static void ImplDateIncrementDay( Date& rDate, bool bUp )
{
    DateFormatter::ExpandCentury( rDate );

    if ( bUp )
    {
        if ( (rDate.GetDay() != 31) || (rDate.GetMonth() != 12) || (rDate.GetYear() != SAL_MAX_INT16) )
            ++rDate;
    }
    else
    {
        if ( (rDate.GetDay() != 1 ) || (rDate.GetMonth() != 1) || (rDate.GetYear() != SAL_MIN_INT16) )
            --rDate;
    }
}

static void ImplDateIncrementMonth( Date& rDate, bool bUp )
{
    DateFormatter::ExpandCentury( rDate );

    sal_uInt16 nMonth = rDate.GetMonth();
    sal_Int16 nYear = rDate.GetYear();
    if ( bUp )
    {
        if ( (nMonth == 12) && (nYear < SAL_MAX_INT16) )
        {
            rDate.SetMonth( 1 );
            rDate.SetYear( rDate.GetNextYear() );
        }
        else
        {
            if ( nMonth < 12 )
                rDate.SetMonth( nMonth + 1 );
        }
    }
    else
    {
        if ( (nMonth == 1) && (nYear > SAL_MIN_INT16) )
        {
            rDate.SetMonth( 12 );
            rDate.SetYear( rDate.GetPrevYear() );
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

    sal_Int16 nYear = rDate.GetYear();
    sal_uInt16 nMonth = rDate.GetMonth();
    if ( bUp )
    {
        if ( nYear < SAL_MAX_INT16 )
            rDate.SetYear( rDate.GetNextYear() );
    }
    else
    {
        if ( nYear > SAL_MIN_INT16 )
            rDate.SetYear( rDate.GetPrevYear() );
    }
    if (nMonth != 2)
        return;

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

bool DateFormatter::ImplAllowMalformedInput() const
{
    return !IsEnforceValidValue();
}

int DateFormatter::GetDateArea(ExtDateFieldFormat eFormat, const OUString& rText, int nCursor, const LocaleDataWrapper& rLocaleDataWrapper)
{
    sal_Int8 nDateArea = 0;

    if ( eFormat == ExtDateFieldFormat::SystemLong )
    {
        eFormat = ImplGetExtFormat(rLocaleDataWrapper.getLongDateOrder());
        nDateArea = 1;
    }
    else
    {
        // search area
        sal_Int32 nPos = 0;
        OUString aDateSep = ImplGetDateSep(rLocaleDataWrapper, eFormat);
        for ( sal_Int8 i = 1; i <= 3; i++ )
        {
            nPos = rText.indexOf( aDateSep, nPos );
            if (nPos < 0 || nPos >= nCursor)
            {
                nDateArea = i;
                break;
            }
            else
                nPos++;
        }
    }

    return nDateArea;
}

void DateField::ImplDateSpinArea( bool bUp )
{
    // increment days if all is selected
    if ( !GetField() )
        return;

    Date aDate( GetDate() );
    Selection aSelection = GetField()->GetSelection();
    aSelection.Justify();
    OUString aText( GetText() );
    if ( static_cast<sal_Int32>(aSelection.Len()) == aText.getLength() )
        ImplDateIncrementDay( aDate, bUp );
    else
    {
        ExtDateFieldFormat eFormat = GetExtDateFormat( true );
        sal_Int8 nDateArea = GetDateArea(eFormat, aText, aSelection.Max(), ImplGetLocaleDataWrapper());

        switch( eFormat )
        {
            case ExtDateFieldFormat::ShortMMDDYY:
            case ExtDateFieldFormat::ShortMMDDYYYY:
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
            case ExtDateFieldFormat::ShortDDMMYY:
            case ExtDateFieldFormat::ShortDDMMYYYY:
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
            case ExtDateFieldFormat::ShortYYMMDD:
            case ExtDateFieldFormat::ShortYYYYMMDD:
            case ExtDateFieldFormat::ShortYYMMDD_DIN5008:
            case ExtDateFieldFormat::ShortYYYYMMDD_DIN5008:
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

DateFormatter::DateFormatter(Edit* pEdit)
    : FormatterBase(pEdit)
    , maFieldDate(0)
    , maLastDate(0)
    , maMin(1, 1, 1900)
    , maMax(31, 12, 2200)
    , mbLongFormat(false)
    , mbShowDateCentury(true)
    , mnExtDateFormat(ExtDateFieldFormat::SystemShort)
    , mbEnforceValidValue(true)
{
}

DateFormatter::~DateFormatter()
{
}

CalendarWrapper& DateFormatter::GetCalendarWrapper() const
{
    if (!mxCalendarWrapper)
    {
        const_cast<DateFormatter*>(this)->mxCalendarWrapper.reset( new CalendarWrapper( comphelper::getProcessComponentContext() ) );
        mxCalendarWrapper->loadDefaultCalendar( GetLocale() );
    }

    return *mxCalendarWrapper;
}

void DateFormatter::SetExtDateFormat( ExtDateFieldFormat eFormat )
{
    mnExtDateFormat = eFormat;
    ReformatAll();
}

ExtDateFieldFormat DateFormatter::GetExtDateFormat( bool bResolveSystemFormat ) const
{
    ExtDateFieldFormat eDateFormat = mnExtDateFormat;

    if (bResolveSystemFormat)
        eDateFormat = ResolveSystemFormat(eDateFormat, ImplGetLocaleDataWrapper());

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
        SetExtDateFormat( ExtDateFieldFormat::SystemLong );
    }
    else
    {
        if( mnExtDateFormat == ExtDateFieldFormat::SystemLong )
            SetExtDateFormat( ExtDateFieldFormat::SystemShort );
    }

    ReformatAll();
}

namespace
{
    ExtDateFieldFormat ChangeDateCentury(ExtDateFieldFormat eExtDateFormat, bool bShowDateCentury)
    {
        // #91913# Remove LongFormat and DateShowCentury - redundant
        if (bShowDateCentury)
        {
            switch (eExtDateFormat)
            {
                case ExtDateFieldFormat::SystemShort:
                case ExtDateFieldFormat::SystemShortYY:
                    eExtDateFormat = ExtDateFieldFormat::SystemShortYYYY;  break;
                case ExtDateFieldFormat::ShortDDMMYY:
                    eExtDateFormat = ExtDateFieldFormat::ShortDDMMYYYY;     break;
                case ExtDateFieldFormat::ShortMMDDYY:
                    eExtDateFormat = ExtDateFieldFormat::ShortMMDDYYYY;     break;
                case ExtDateFieldFormat::ShortYYMMDD:
                    eExtDateFormat = ExtDateFieldFormat::ShortYYYYMMDD;     break;
                case ExtDateFieldFormat::ShortYYMMDD_DIN5008:
                    eExtDateFormat = ExtDateFieldFormat::ShortYYYYMMDD_DIN5008; break;
                default:
                    ;
            }
        }
        else
        {
            switch (eExtDateFormat)
            {
                case ExtDateFieldFormat::SystemShort:
                case ExtDateFieldFormat::SystemShortYYYY:
                    eExtDateFormat = ExtDateFieldFormat::SystemShortYY;    break;
                case ExtDateFieldFormat::ShortDDMMYYYY:
                    eExtDateFormat = ExtDateFieldFormat::ShortDDMMYY;       break;
                case ExtDateFieldFormat::ShortMMDDYYYY:
                    eExtDateFormat = ExtDateFieldFormat::ShortMMDDYY;       break;
                case ExtDateFieldFormat::ShortYYYYMMDD:
                    eExtDateFormat = ExtDateFieldFormat::ShortYYMMDD;       break;
                case ExtDateFieldFormat::ShortYYYYMMDD_DIN5008:
                    eExtDateFormat = ExtDateFieldFormat::ShortYYMMDD_DIN5008;  break;
                default:
                    ;
            }
        }

        return eExtDateFormat;
    }
}

void DateFormatter::SetShowDateCentury( bool bShowDateCentury )
{
    mbShowDateCentury = bShowDateCentury;

    SetExtDateFormat(ChangeDateCentury(GetExtDateFormat(), bShowDateCentury));

    ReformatAll();
}

void DateFormatter::SetDate( const Date& rNewDate )
{
    ImplSetUserDate( rNewDate );
    maFieldDate = maLastDate;
    maLastDate = GetDate();
}

void DateFormatter::ImplSetUserDate( const Date& rNewDate, Selection const * pNewSelection )
{
    Date aNewDate = rNewDate;
    if ( aNewDate > maMax )
        aNewDate = maMax;
    else if ( aNewDate < maMin )
        aNewDate = maMin;
    maLastDate = aNewDate;

    if ( GetField() )
        ImplSetText( ImplGetDateAsText( aNewDate ), pNewSelection );
}

void DateFormatter::ImplNewFieldValue( const Date& rDate )
{
    if ( !GetField() )
        return;

    Selection aSelection = GetField()->GetSelection();
    aSelection.Justify();
    OUString aText = GetField()->GetText();

    // If selected until the end then keep it that way
    if ( static_cast<sal_Int32>(aSelection.Max()) == aText.getLength() )
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

Date DateFormatter::GetDate() const
{
    Date aDate( Date::EMPTY );

    if ( GetField() )
    {
        if (TextToDate(GetField()->GetText(), aDate, GetExtDateFormat(true), ImplGetLocaleDataWrapper(), GetCalendarWrapper()))
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
                aDate = Date( Date::EMPTY ); // set invalid date
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
            bEmpty = !TextToDate(GetField()->GetText(), aDate, GetExtDateFormat(true), ImplGetLocaleDataWrapper(), GetCalendarWrapper());
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
    ImplDateReformat( GetField()->GetText(), aStr );

    if ( !aStr.isEmpty() )
    {
        ImplSetText( aStr );
        (void)TextToDate(aStr, maLastDate, GetExtDateFormat(true), ImplGetLocaleDataWrapper(), GetCalendarWrapper());
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
    ExpandCentury(rDate, officecfg::Office::Common::DateFormat::TwoDigitYear::get());
}

void DateFormatter::ExpandCentury( Date& rDate, sal_uInt16 nTwoDigitYearStart )
{
    sal_Int16 nDateYear = rDate.GetYear();
    if ( 0 <= nDateYear && nDateYear < 100 )
    {
        sal_uInt16 nCentury = nTwoDigitYearStart / 100;
        if ( nDateYear < (nTwoDigitYearStart % 100) )
            nCentury++;
        rDate.SetYear( nDateYear + (nCentury*100) );
    }
}

DateField::DateField( vcl::Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle ),
    DateFormatter(this),
    maFirst( GetMin() ),
    maLast( GetMax() )
{
    SetText( ImplGetLocaleDataWrapper().getDate( ImplGetFieldDate() ) );
    Reformat();
    ResetLastDate();
}

void DateField::dispose()
{
    ClearField();
    SpinField::dispose();
}

bool DateField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && IsStrictFormat() &&
         ( GetExtDateFormat() != ExtDateFieldFormat::SystemLong ) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplDateProcessKeyInput( *rNEvt.GetKeyEvent(), GetExtDateFormat( true ), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return SpinField::PreNotify( rNEvt );
}

bool DateField::EventNotify( NotifyEvent& rNEvt )
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
                    if (TextToDate(GetText(), aDate, GetExtDateFormat(true), ImplGetLocaleDataWrapper(), GetCalendarWrapper()))
                        // even with strict text analysis, our text is a valid date -> do a complete
                        // reformat
                        Reformat();
                }
            }
            else
            {
                ResetLastDate();
                SetEmptyFieldValueData( true );
            }
        }
    }

    return SpinField::EventNotify( rNEvt );
}

void DateField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & (AllSettingsFlags::LOCALE|AllSettingsFlags::MISC)) )
    {
        if (rDCEvt.GetFlags() & AllSettingsFlags::LOCALE)
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

DateBox::DateBox(vcl::Window* pParent, WinBits nWinStyle)
    : ComboBox( pParent, nWinStyle )
    , DateFormatter(this)
{
    SetText( ImplGetLocaleDataWrapper().getDate( ImplGetFieldDate() ) );
    Reformat();
}

void DateBox::dispose()
{
    ClearField();
    ComboBox::dispose();
}

bool DateBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && IsStrictFormat() &&
         ( GetExtDateFormat() != ExtDateFieldFormat::SystemLong ) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplDateProcessKeyInput( *rNEvt.GetKeyEvent(), GetExtDateFormat( true ), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return ComboBox::PreNotify( rNEvt );
}

void DateBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::LOCALE) )
    {
        ImplGetLocaleDataWrapper().setLanguageTag( GetSettings().GetLanguageTag() );
        ReformatAll();
    }
}

bool DateBox::EventNotify( NotifyEvent& rNEvt )
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
            else
            {
                ResetLastDate();
                SetEmptyFieldValueData( true );
            }
        }
    }

    return ComboBox::EventNotify( rNEvt );
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
        ImplDateReformat( GetEntry( i ), aStr );
        RemoveEntryAt(i);
        InsertEntry( aStr, i );
    }
    DateFormatter::Reformat();
    SetUpdateMode( true );
}

namespace weld
{
    CalendarWrapper& DateFormatter::GetCalendarWrapper() const
    {
        if (!m_xCalendarWrapper)
        {
            m_xCalendarWrapper.reset(new CalendarWrapper(comphelper::getProcessComponentContext()));
            m_xCalendarWrapper->loadDefaultCalendar(Application::GetSettings().GetLanguageTag().getLocale());
        }
        return *m_xCalendarWrapper;
    }

    void DateFormatter::SetShowDateCentury(bool bShowDateCentury)
    {
        m_eFormat = ChangeDateCentury(m_eFormat, bShowDateCentury);

        ReFormat();
    }

    void DateFormatter::SetDate(const Date& rDate)
    {
        auto nDate = rDate.GetDate();
        bool bForceOutput = GetEntryText().isEmpty() && rDate == GetDate();
        if (bForceOutput)
        {
            ImplSetValue(nDate, true);
            return;
        }
        SetValue(nDate);
    }

    Date DateFormatter::GetDate()
    {
        return Date(GetValue());
    }

    void DateFormatter::SetMin(const Date& rNewMin)
    {
        SetMinValue(rNewMin.GetDate());
    }

    void DateFormatter::SetMax(const Date& rNewMax)
    {
        SetMaxValue(rNewMax.GetDate());
    }

    OUString DateFormatter::FormatNumber(int nValue) const
    {
        const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();
        return ::DateFormatter::FormatDate(Date(nValue), m_eFormat, rLocaleData, GetCalendarWrapper());
    }

    IMPL_LINK_NOARG(DateFormatter, FormatOutputHdl, LinkParamNone*, bool)
    {
        OUString sText = FormatNumber(GetValue());
        ImplSetTextImpl(sText, nullptr);
        return true;
    }

    IMPL_LINK(DateFormatter, ParseInputHdl, sal_Int64*, result, TriState)
    {
        const LocaleDataWrapper& rLocaleDataWrapper = Application::GetSettings().GetLocaleDataWrapper();

        Date aResult(Date::EMPTY);
        bool bRet = ::DateFormatter::TextToDate(GetEntryText(), aResult, ResolveSystemFormat(m_eFormat, rLocaleDataWrapper),
                                                rLocaleDataWrapper, GetCalendarWrapper());
        if (bRet)
            *result = aResult.GetDate();

        return bRet ? TRISTATE_TRUE : TRISTATE_FALSE;
    }
}

static bool ImplTimeProcessKeyInput( const KeyEvent& rKEvt,
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
             rLocaleDataWrapper.getTimeSep() == OUStringChar(cChar) ||
             (rLocaleDataWrapper.getTimeAM().indexOf(cChar) != -1) ||
             (rLocaleDataWrapper.getTimePM().indexOf(cChar) != -1) ||
             // Accept AM/PM:
             (cChar == 'a') || (cChar == 'A') || (cChar == 'm') || (cChar == 'M') || (cChar == 'p') || (cChar == 'P') ||
             ((eFormat == TimeFieldFormat::F_SEC_CS) && rLocaleDataWrapper.getTime100SecSep() == OUStringChar(cChar)) ||
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

    if (_nSepPos < _rStr.getLength())
        _rStr.remove(0, _nSepPos + 1);
    else
        _rStr.truncate();

    if ( !ImplIsValidTimePortion( _bSkipInvalidCharacters, sPortion ) )
        return false;
    *_pPortion = static_cast<short>(sPortion.toInt32());
    return true;
}

bool TimeFormatter::TextToTime(const OUString& rStr, tools::Time& rTime, TimeFieldFormat eFormat,
    bool bDuration, const LocaleDataWrapper& rLocaleDataWrapper, bool _bSkipInvalidCharacters)
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
            if (rLocaleDataWrapper.getTimeSep() == OUStringChar(aSepStr[i]))
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
                nSecond = static_cast<short>(aStr.toString().toInt32());
        }
        else
            nMinute = static_cast<short>(aStr.toString().toInt32());
    }
    else if ( nSepPos < 0 )
    {
        nSecond = static_cast<short>(aStr.toString().toInt32());
        nMinute += nSecond / 60;
        nSecond %= 60;
        nHour += nMinute / 60;
        nMinute %= 60;
    }
    else
    {
        nSecond = static_cast<short>(aStr.copy( 0, nSepPos ).makeStringAndClear().toInt32());
        aStr.remove( 0, nSepPos+1 );

        nSepPos = aStr.indexOf( rLocaleDataWrapper.getTimeSep() );
        if ( !aStr.isEmpty() && aStr[0] == '-' )
            bNegative = true;
        if ( nSepPos >= 0 )
        {
            nMinute = nSecond;
            nSecond = static_cast<short>(aStr.copy( 0, nSepPos ).makeStringAndClear().toInt32());
            aStr.remove( 0, nSepPos+1 );

            nSepPos = aStr.indexOf( rLocaleDataWrapper.getTimeSep() );
            if ( !aStr.isEmpty() && aStr[0] == '-' )
                bNegative = true;
            if ( nSepPos >= 0 )
            {
                nHour   = nMinute;
                nMinute = nSecond;
                nSecond = static_cast<short>(aStr.copy( 0, nSepPos ).makeStringAndClear().toInt32());
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

        aTime = tools::Time( static_cast<sal_uInt16>(nHour), static_cast<sal_uInt16>(nMinute), static_cast<sal_uInt16>(nSecond),
                      static_cast<sal_uInt32>(nNanoSec) );
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

        aTime = tools::Time( static_cast<sal_uInt16>(nHour), static_cast<sal_uInt16>(nMinute), static_cast<sal_uInt16>(nSecond),
                      static_cast<sal_uInt32>(nNanoSec) );
        if ( bNegative )
            aTime = -aTime;
    }

    rTime = aTime;

    return true;
}

void TimeFormatter::ImplTimeReformat( const OUString& rStr, OUString& rOutStr )
{
    tools::Time aTime( 0, 0, 0 );
    if ( !TextToTime( rStr, aTime, GetFormat(), IsDuration(), ImplGetLocaleDataWrapper() ) )
        return;

    tools::Time aTempTime = aTime;
    if ( aTempTime > GetMax() )
        aTempTime = GetMax() ;
    else if ( aTempTime < GetMin() )
        aTempTime = GetMin();

    bool bSecond = false;
    bool b100Sec = false;
    if ( meFormat != TimeFieldFormat::F_NONE )
        bSecond = true;

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
        if ( GetTimeFormat() == TimeFormat::Hour12 )
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
}

bool TimeFormatter::ImplAllowMalformedInput() const
{
    return !IsEnforceValidValue();
}

int TimeFormatter::GetTimeArea(TimeFieldFormat eFormat, const OUString& rText, int nCursor,
                                     const LocaleDataWrapper& rLocaleDataWrapper)
{
    int nTimeArea = 0;

    // Area search
    if (eFormat != TimeFieldFormat::F_SEC_CS)
    {
        //Which area is the cursor in of HH:MM:SS.TT
        for ( sal_Int32 i = 1, nPos = 0; i <= 4; i++ )
        {
            sal_Int32 nPos1 = rText.indexOf(rLocaleDataWrapper.getTimeSep(), nPos);
            sal_Int32 nPos2 = rText.indexOf(rLocaleDataWrapper.getTime100SecSep(), nPos);
            //which ever comes first, bearing in mind that one might not be there
            if (nPos1 >= 0 && nPos2 >= 0)
                nPos = std::min(nPos1, nPos2);
            else if (nPos1 >= 0)
                nPos = nPos1;
            else
                nPos = nPos2;
            if (nPos < 0 || nPos >= nCursor)
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
        sal_Int32 nPos = rText.indexOf(rLocaleDataWrapper.getTime100SecSep());
        if (nPos < 0 || nPos >= nCursor)
            nTimeArea = 3;
        else
            nTimeArea = 4;
    }

    return nTimeArea;
}

tools::Time TimeFormatter::SpinTime(bool bUp, const tools::Time& rTime, TimeFieldFormat eFormat,
                                    bool bDuration, const OUString& rText, int nCursor,
                                    const LocaleDataWrapper& rLocaleDataWrapper)
{
    tools::Time aTime(rTime);

    int nTimeArea = GetTimeArea(eFormat, rText, nCursor, rLocaleDataWrapper);

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
        if (!bDuration)
        {
            tools::Time aAbsMaxTime( 23, 59, 59, 999999999 );
            if ( aTime > aAbsMaxTime )
                aTime = aAbsMaxTime;
            tools::Time aAbsMinTime( 0, 0 );
            if ( aTime < aAbsMinTime )
                aTime = aAbsMinTime;
        }
    }

    return aTime;
}

void TimeField::ImplTimeSpinArea( bool bUp )
{
    if ( GetField() )
    {
        tools::Time aTime( GetTime() );
        OUString aText( GetText() );
        Selection aSelection( GetField()->GetSelection() );

        aTime = TimeFormatter::SpinTime(bUp, aTime, GetFormat(), IsDuration(), aText, aSelection.Max(), ImplGetLocaleDataWrapper());

        ImplNewFieldValue( aTime );
    }
}

TimeFormatter::TimeFormatter(Edit* pEdit)
    : FormatterBase(pEdit)
    , maLastTime(0, 0)
    , maMin(0, 0)
    , maMax(23, 59, 59, 999999999)
    , meFormat(TimeFieldFormat::F_NONE)
    , mnTimeFormat(TimeFormat::Hour24)  // Should become an ExtTimeFieldFormat in next implementation, merge with mbDuration and meFormat
    , mbDuration(false)
    , mbEnforceValidValue(true)
    , maFieldTime(0, 0)
{
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

void TimeFormatter::SetTimeFormat( TimeFormat eNewFormat )
{
    mnTimeFormat = eNewFormat;
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
    if ( !GetField() )
        return;

    Selection aSelection = GetField()->GetSelection();
    aSelection.Justify();
    OUString aText = GetField()->GetText();

    // If selected until the end then keep it that way
    if ( static_cast<sal_Int32>(aSelection.Max()) == aText.getLength() )
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

OUString TimeFormatter::FormatTime(const tools::Time& rNewTime, TimeFieldFormat eFormat, TimeFormat eHourFormat, bool bDuration, const LocaleDataWrapper& rLocaleData)
{
    OUString aStr;
    bool bSec    = false;
    bool b100Sec = false;
    if ( eFormat != TimeFieldFormat::F_NONE )
        bSec = true;
    if ( eFormat == TimeFieldFormat::F_SEC_CS )
        b100Sec = true;
    if ( eFormat == TimeFieldFormat::F_SEC_CS )
    {
        sal_uLong n  = rNewTime.GetHour() * 3600L;
        n       += rNewTime.GetMin()  * 60L;
        n       += rNewTime.GetSec();
        aStr     = OUString::number( n ) + rLocaleData.getTime100SecSep();
        std::ostringstream ostr;
        ostr.fill('0');
        ostr.width(9);
        ostr << rNewTime.GetNanoSec();
        aStr += OUString::createFromAscii(ostr.str().c_str());
    }
    else if ( bDuration )
    {
        aStr = rLocaleData.getDuration( rNewTime, bSec, b100Sec );
    }
    else
    {
        aStr = rLocaleData.getTime( rNewTime, bSec, b100Sec );
        if ( eHourFormat == TimeFormat::Hour12 )
        {
            if ( rNewTime.GetHour() > 12 )
            {
                tools::Time aT( rNewTime );
                aT.SetHour( aT.GetHour() % 12 );
                aStr = rLocaleData.getTime( aT, bSec, b100Sec );
            }
            // Don't use LocaleDataWrapper, we want AM/PM
            if ( rNewTime.GetHour() < 12 )
                aStr += "AM"; // rLocaleData.getTimeAM();
            else
                aStr += "PM"; // rLocaleData.getTimePM();
        }
    }

    return aStr;
}

void TimeFormatter::ImplSetUserTime( const tools::Time& rNewTime, Selection const * pNewSelection )
{
    tools::Time aNewTime = rNewTime;
    if ( aNewTime > GetMax() )
        aNewTime = GetMax();
    else if ( aNewTime < GetMin() )
        aNewTime = GetMin();
    maLastTime = aNewTime;

    if ( GetField() )
    {
        OUString aStr = TimeFormatter::FormatTime(aNewTime, meFormat, GetTimeFormat(), mbDuration, ImplGetLocaleDataWrapper());
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
        bool bAllowMalformed = ImplAllowMalformedInput();
        if ( TextToTime( GetField()->GetText(), aTime, GetFormat(), IsDuration(), ImplGetLocaleDataWrapper(), !bAllowMalformed ) )
        {
            if ( aTime > GetMax() )
                aTime = GetMax();
            else if ( aTime < GetMin() )
                aTime = GetMin();
        }
        else
        {
            if ( bAllowMalformed )
                aTime = tools::Time( 99, 99, 99 ); // set invalid time
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
    ImplTimeReformat( GetField()->GetText(), aStr );

    if ( !aStr.isEmpty() )
    {
        ImplSetText( aStr );
        (void)TextToTime(aStr, maLastTime, GetFormat(), IsDuration(), ImplGetLocaleDataWrapper());
    }
    else
        SetTime( maLastTime );
}

TimeField::TimeField( vcl::Window* pParent, WinBits nWinStyle ) :
    SpinField( pParent, nWinStyle ),
    TimeFormatter(this),
    maFirst( GetMin() ),
    maLast( GetMax() )
{
    SetText( ImplGetLocaleDataWrapper().getTime( maFieldTime, false ) );
    Reformat();
}

void TimeField::dispose()
{
    ClearField();
    SpinField::dispose();
}

bool TimeField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplTimeProcessKeyInput( *rNEvt.GetKeyEvent(), IsStrictFormat(), IsDuration(), GetFormat(), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return SpinField::PreNotify( rNEvt );
}

bool TimeField::EventNotify( NotifyEvent& rNEvt )
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
                if ( TextToTime( GetText(), aTime, GetFormat(), IsDuration(), ImplGetLocaleDataWrapper(), false ) )
                    // even with strict text analysis, our text is a valid time -> do a complete
                    // reformat
                    Reformat();
            }
        }
    }

    return SpinField::EventNotify( rNEvt );
}

void TimeField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::LOCALE) )
    {
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
        case ExtTimeFieldFormat::Short24H:
        {
            SetTimeFormat( TimeFormat::Hour24 );
            SetDuration( false );
            SetFormat( TimeFieldFormat::F_NONE );
        }
        break;
        case ExtTimeFieldFormat::Long24H:
        {
            SetTimeFormat( TimeFormat::Hour24 );
            SetDuration( false );
            SetFormat( TimeFieldFormat::F_SEC );
        }
        break;
        case ExtTimeFieldFormat::Short12H:
        {
            SetTimeFormat( TimeFormat::Hour12 );
            SetDuration( false );
            SetFormat( TimeFieldFormat::F_NONE );
        }
        break;
        case ExtTimeFieldFormat::Long12H:
        {
            SetTimeFormat( TimeFormat::Hour12 );
            SetDuration( false );
            SetFormat( TimeFieldFormat::F_SEC );
        }
        break;
        case ExtTimeFieldFormat::ShortDuration:
        {
            SetDuration( true );
            SetFormat( TimeFieldFormat::F_NONE );
        }
        break;
        case ExtTimeFieldFormat::LongDuration:
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

TimeBox::TimeBox(vcl::Window* pParent, WinBits nWinStyle)
    : ComboBox(pParent, nWinStyle)
    , TimeFormatter(this)
{
    SetText( ImplGetLocaleDataWrapper().getTime( maFieldTime, false ) );
    Reformat();
}

void TimeBox::dispose()
{
    ClearField();
    ComboBox::dispose();
}

bool TimeBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) && !rNEvt.GetKeyEvent()->GetKeyCode().IsMod2() )
    {
        if ( ImplTimeProcessKeyInput( *rNEvt.GetKeyEvent(), IsStrictFormat(), IsDuration(), GetFormat(), ImplGetLocaleDataWrapper() ) )
            return true;
    }

    return ComboBox::PreNotify( rNEvt );
}

bool TimeBox::EventNotify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        MarkToBeReformatted( false );
    else if ( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( MustBeReformatted() && (!GetText().isEmpty() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return ComboBox::EventNotify( rNEvt );
}

void TimeBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::LOCALE) )
    {
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

namespace weld
{
    tools::Time TimeFormatter::ConvertValue(int nValue)
    {
        tools::Time aTime(0);
        aTime.MakeTimeFromMS(nValue);
        return aTime;
    }

    int TimeFormatter::ConvertValue(const tools::Time& rTime)
    {
        return rTime.GetMSFromTime();
    }

    void TimeFormatter::SetTime(const tools::Time& rTime)
    {
        auto nTime = ConvertValue(rTime);
        bool bForceOutput = GetEntryText().isEmpty() && rTime == GetTime();
        if (bForceOutput)
        {
            ImplSetValue(nTime, true);
            return;
        }
        SetValue(nTime);
    }

    tools::Time TimeFormatter::GetTime()
    {
        return ConvertValue(GetValue());
    }

    void TimeFormatter::SetMin(const tools::Time& rNewMin)
    {
        SetMinValue(ConvertValue(rNewMin));
    }

    void TimeFormatter::SetMax(const tools::Time& rNewMax)
    {
        SetMaxValue(ConvertValue(rNewMax));
    }

    OUString TimeFormatter::FormatNumber(int nValue) const
    {
        const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();
        return ::TimeFormatter::FormatTime(ConvertValue(nValue), m_eFormat, m_eTimeFormat, m_bDuration, rLocaleData);
    }

    IMPL_LINK_NOARG(TimeFormatter, FormatOutputHdl, LinkParamNone*, bool)
    {
        OUString sText = FormatNumber(GetValue());
        ImplSetTextImpl(sText, nullptr);
        return true;
    }

    IMPL_LINK(TimeFormatter, ParseInputHdl, sal_Int64*, result, TriState)
    {
        const LocaleDataWrapper& rLocaleDataWrapper = Application::GetSettings().GetLocaleDataWrapper();

        tools::Time aResult(0);
        bool bRet = ::TimeFormatter::TextToTime(GetEntryText(), aResult, m_eFormat, m_bDuration, rLocaleDataWrapper);
        if (bRet)
            *result = ConvertValue(aResult);

        return bRet ? TRISTATE_TRUE : TRISTATE_FALSE;
    }

    IMPL_LINK(TimeFormatter, CursorChangedHdl, weld::Entry&, rEntry, void)
    {
        int nStartPos, nEndPos;
        rEntry.get_selection_bounds(nStartPos, nEndPos);

        const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetLocaleDataWrapper();
        const int nTimeArea = ::TimeFormatter::GetTimeArea(m_eFormat, GetEntryText(), nEndPos, rLocaleData);

        int nIncrements = 1;

        if (nTimeArea == 1)
            nIncrements = 1000 * 60 * 60;
        else if (nTimeArea == 2)
            nIncrements = 1000 * 60;
        else if (nTimeArea == 3)
            nIncrements = 1000;

        SetSpinSize(nIncrements);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
