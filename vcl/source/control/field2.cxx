/*************************************************************************
 *
 *  $RCSfile: field2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2000-10-29 17:20:46 $
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

#define _SV_FIELD2_CXX

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <field.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_SYSTEM_HXX
#include <system.hxx>
#endif
#ifndef _SV_SOUND_HXX
#include <sound.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <field.hxx>
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

#include <unohelp.hxx>

#include <com/sun/star/lang/Locale.hpp>

#ifndef _COM_SUN_STAR_I18N_XCHARACTERCLASSIFICATION_HPP_
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#endif

#ifndef _COM_SUN_STAR_I18N_KCHARACTERTYPE_HPP_
#include <com/sun/star/i18n/KCharacterType.hpp>
#endif

#pragma hdrstop

using namespace ::com::sun::star;

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



const sal_Int32 nCharClassAlphaType =
    ::com::sun::star::i18n::KCharacterType::UPPER |
    ::com::sun::star::i18n::KCharacterType::LOWER |
    ::com::sun::star::i18n::KCharacterType::TITLE_CASE;

const sal_Int32 nCharClassAlphaTypeMask =
    nCharClassAlphaType |
    ::com::sun::star::i18n::KCharacterType::PRINTABLE |
    ::com::sun::star::i18n::KCharacterType::BASE_FORM;

inline sal_Bool isAlphaType( sal_Int32 nType )
{
    return ((nType & nCharClassAlphaType) != 0) && ((nType & ~(nCharClassAlphaTypeMask)) == 0);
}

lang::Locale CreateLocale( const International& rInt )
{
    String aLanguage, aCountry;
    ConvertLanguageToIsoNames( rInt.GetLanguage(), aLanguage, aCountry );
    lang::Locale aLocale;
    aLocale.Language = aLanguage;
    aLocale.Country = aCountry;

    return aLocale;
}

uno::Reference< i18n::XCharacterClassification > ImplGetCharClass()
{
    static uno::Reference< i18n::XCharacterClassification > xCharClass;
    if ( !xCharClass.is() )
        xCharClass = vcl::unohelper::CreateCharacterClassification();

    return xCharClass;
}


// -----------------------------------------------------------------------

static int ImplIsPatternChar( xub_Unicode cChar, sal_Char cEditMask )
{
    if ( (cEditMask == EDITMASK_ALPHA) || (cEditMask == EDITMASK_UPPERALPHA) )
    {
        if ( ((cChar < 'A') || (cChar > 'Z')) &&
             ((cChar < 'a') || (cChar > 'z')) )
            return FALSE;
    }
    else if ( (cEditMask == EDITMASK_ALPHANUM) || (cEditMask == EDITMASK_UPPERALPHANUM) )
    {
        if ( ((cChar < 'A') || (cChar > 'Z')) &&
             ((cChar < 'a') || (cChar > 'z')) &&
             ((cChar < '0') || (cChar > '9')) )
            return FALSE;
    }
    else if ( (cEditMask == EDITMASK_ALLCHAR) || (cEditMask == EDITMASK_UPPERALLCHAR) )
    {
        if ( cChar < 32 )
            return FALSE;
    }
    else if ( cEditMask == EDITMASK_NUM )
    {
        if ( (cChar < '0') || (cChar > '9') )
            return FALSE;
    }
    else if ( cEditMask == EDITMASK_NUMSPACE )
    {
        if ( ((cChar < '0') || (cChar > '9')) && (cChar != ' ') )
            return FALSE;
    }
    else
        return FALSE;

    return TRUE;
}

// -----------------------------------------------------------------------

static xub_Unicode ImplPatternChar( xub_Unicode cChar, sal_Char cEditMask )
{
    if ( ImplIsPatternChar( cChar, cEditMask ) )
    {
        if ( (cEditMask == EDITMASK_UPPERALPHA) ||
             (cEditMask == EDITMASK_UPPERALPHANUM) )
        {
            if ( (cChar >= 'a') && (cChar <= 'z') )
                cChar = (cChar - 'a') + 'A';
        }
        else if ( cEditMask == EDITMASK_UPPERALLCHAR )
        {
            cChar = ImplGetCharClass()->toUpper( String(cChar),0,1,Application::GetSettings().GetLocale() )[0];
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
        return TRUE;
    else if ( ((c1 == '.') || (c1 == ',')) &&
              ((c2 == '.') || (c2 == ',')) )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

static XubString ImplPatternReformat( const XubString& rStr,
                                      const ByteString& rEditMask,
                                      const XubString& rLiteralMask,
                                      USHORT nFormatFlags )
{
    if ( !rEditMask.Len() )
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

    while ( i < rEditMask.Len() )
    {
        if ( nStrIndex >= aStr.Len() )
            break;

        cChar = aStr.GetChar(nStrIndex);
        cLiteral = rLiteralMask.GetChar(i);
        cMask = rEditMask.GetChar(i);

        // Aktuelle Position ein Literal
        if ( cMask == EDITMASK_LITERAL )
        {
            // Wenn es das Literal-Zeichen ist, uebernehmen, ansonsten
            // ignorieren, da es das naechste gueltige Zeichen vom String
            // sein kann
            if ( ImplKommaPointCharEqual( cChar, cLiteral ) )
                nStrIndex++;
            else
            {
                // Ansonsten testen wir, ob es ein ungueltiges Zeichen ist.
                // Dies ist dann der Fall, wenn es nicht in das Muster
                // des naechsten nicht Literal-Zeichens passt
                n = i+1;
                while ( n < rEditMask.Len() )
                {
                    if ( rEditMask.GetChar(n) != EDITMASK_LITERAL )
                    {
                        if ( !ImplIsPatternChar( cChar, rEditMask.GetChar(n) ) )
                            nStrIndex++;
                        break;
                    }

                    n++;
                }
            }
        }
        else
        {
            // Gueltiges Zeichen an der Stelle
            cTempChar = ImplPatternChar( cChar, cMask );
            if ( cTempChar )
            {
                // dann Zeichen uebernehmen
                aOutStr.SetChar( i, cTempChar );
                nStrIndex++;
            }
            else
            {
                // Wenn es das Literalzeichen ist, uebernehmen
                if ( cLiteral == cChar )
                    nStrIndex++;
                else
                {
                    // Wenn das ungueltige Zeichen das naechste Literalzeichen
                    // sein kann, dann springen wir bis dahin vor, ansonten
                    // das Zeichen ignorieren
                    // Nur machen, wenn leere Literale erlaubt sind
                    if ( nFormatFlags & PATTERN_FORMAT_EMPTYLITERALS )
                    {
                        n = i;
                        while ( n < rEditMask.Len() )
                        {
                            if ( rEditMask.GetChar( n ) == EDITMASK_LITERAL )
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

static void ImplPatternMaxPos( const XubString rStr, const ByteString& rEditMask,
                               USHORT nFormatFlags, BOOL bSameMask,
                               USHORT nCursorPos, USHORT& rPos )
{

    // Letzte Position darf nicht groesser als der enthaltene String sein
    xub_StrLen nMaxPos = rStr.Len();

    // Wenn keine leeren Literale erlaubt sind, auch Leerzeichen
    // am Ende ignorieren
    if ( bSameMask && !(nFormatFlags & PATTERN_FORMAT_EMPTYLITERALS) )
    {
        while ( nMaxPos )
        {
            if ( (rEditMask.GetChar(nMaxPos-1) != EDITMASK_LITERAL) &&
                 (rStr.GetChar(nMaxPos-1) != ' ') )
                break;
            nMaxPos--;
        }

        // Wenn wir vor einem Literal stehen, dann solange weitersuchen,
        // bis erste Stelle nach Literal
        xub_StrLen nTempPos = nMaxPos;
        while ( nTempPos < rEditMask.Len() )
        {
            if ( rEditMask.GetChar(nTempPos) != EDITMASK_LITERAL )
            {
                nMaxPos = nTempPos;
                break;
            }
            nTempPos++;
        }
    }

    if ( rPos > nMaxPos )
        rPos = nMaxPos;
    // Zeichen sollte nicht nach links wandern
    if ( rPos < nCursorPos )
        rPos = nCursorPos;
}

// -----------------------------------------------------------------------

static void ImplPatternProcessStrictModify( Edit* pEdit,
                                            const ByteString& rEditMask,
                                            const XubString& rLiteralMask,
                                            USHORT nFormatFlags, BOOL bSameMask )
{
    XubString aText = pEdit->GetText();

    // Leerzeichen am Anfang entfernen
    if ( bSameMask && !(nFormatFlags & PATTERN_FORMAT_EMPTYLITERALS) )
    {
        xub_StrLen i = 0;
        xub_StrLen nMaxLen = aText.Len();
        while ( i < nMaxLen )
        {
            if ( (rEditMask.GetChar( i ) != EDITMASK_LITERAL) &&
                 (aText.GetChar( i ) != ' ') )
                break;

            i++;
        }
        // Alle Literalzeichen beibehalten
        while ( i && (rEditMask.GetChar( i ) == EDITMASK_LITERAL) )
            i--;
        aText.Erase( 0, i );
    }

    XubString aNewText = ImplPatternReformat( aText, rEditMask, rLiteralMask, nFormatFlags );
    if ( aNewText != aText )
    {
        // Selection so anpassen, das diese wenn sie vorher am Ende
        // stand, immer noch am Ende steht
        Selection aSel = pEdit->GetSelection();
        ULONG nMaxSel = Max( aSel.Min(), aSel.Max() );
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

static xub_StrLen ImplPatternLeftPos( const ByteString& rEditMask, xub_StrLen nCursorPos )
{
    // Vorheriges Zeichen suchen, was kein Literal ist
    xub_StrLen nNewPos = nCursorPos;
    xub_StrLen nTempPos = nNewPos;
    while ( nTempPos )
    {
        if ( rEditMask.GetChar(nTempPos-1) != EDITMASK_LITERAL )
        {
            nNewPos = nTempPos-1;
            break;
        }
        nTempPos--;
    }
    return nNewPos;
}

// -----------------------------------------------------------------------

static xub_StrLen ImplPatternRightPos( const XubString& rStr, const ByteString& rEditMask,
                                       USHORT nFormatFlags, BOOL bSameMask,
                                       xub_StrLen nCursorPos )
{
    // Naechstes Zeichen suchen, was kein Literal ist
    xub_StrLen nNewPos = nCursorPos;
    xub_StrLen nTempPos = nNewPos;
    while ( nTempPos < rEditMask.Len() )
    {
        if ( rEditMask.GetChar(nTempPos+1) != EDITMASK_LITERAL )
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

static BOOL ImplPatternProcessKeyInput( Edit* pEdit, const KeyEvent& rKEvt,
                                        const ByteString& rEditMask,
                                        const XubString& rLiteralMask,
                                        BOOL bStrictFormat,
                                        USHORT nFormatFlags,
                                        BOOL bSameMask,
                                        BOOL& rbInKeyInput )
{
    if ( !rEditMask.Len() || !bStrictFormat )
        return FALSE;

    Selection   aOldSel     = pEdit->GetSelection();
    KeyCode     aCode       = rKEvt.GetKeyCode();
    xub_Unicode cChar       = rKEvt.GetCharCode();
    USHORT      nKeyCode    = aCode.GetCode();
    BOOL        bShift      = aCode.IsShift();
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
            return TRUE;
        }
        else if ( nKeyCode == KEY_RIGHT )
        {
            // Hier nehmen wir Selectionsanfang als minimum, da falls durch
            // Focus alles selektiert ist, ist eine kleine Position schon
            // erlaubt.
            Selection aSel( aOldSel );
            aSel.Justify();
            nCursorPos = (xub_StrLen)aSel.Min();
            aSel.Max() = ImplPatternRightPos( pEdit->GetText(), rEditMask, nFormatFlags, bSameMask, nCursorPos );
            if ( bShift )
                aSel.Min() = aOldSel.Min();
            else
                aSel.Min() = aSel.Max();
            pEdit->SetSelection( aSel );
            return TRUE;
        }
        else if ( nKeyCode == KEY_HOME )
        {
            // Home ist Position des ersten nicht literalen Zeichens
            nNewPos = 0;
            while ( (nNewPos < rEditMask.Len()) &&
                    (rEditMask.GetChar(nNewPos) == EDITMASK_LITERAL) )
                nNewPos++;
            // Home sollte nicht nach rechts wandern
            if ( nCursorPos < nNewPos )
                nNewPos = nCursorPos;
            Selection aSel( nNewPos );
            if ( bShift )
                aSel.Min() = aOldSel.Min();
            pEdit->SetSelection( aSel );
            return TRUE;
        }
        else if ( nKeyCode == KEY_END )
        {
            // End ist die Position des letzten nicht literalen Zeichens
            nNewPos = rEditMask.Len();
            while ( nNewPos &&
                    (rEditMask.GetChar(nNewPos-1) == EDITMASK_LITERAL) )
                nNewPos--;
            // Hier nehmen wir Selectionsanfang als minimum, da falls durch
            // Focus alles selektiert ist, ist eine kleine Position schon
            // erlaubt.
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
            return TRUE;
        }
        else if ( (nKeyCode == KEY_BACKSPACE) || (nKeyCode == KEY_DELETE) )
        {
            XubString   aStr( pEdit->GetText() );
            XubString   aOldStr = aStr;
            Selection   aSel = aOldSel;

            aSel.Justify();
            nNewPos = (xub_StrLen)aSel.Min();

            // Wenn Selection, dann diese Loeschen
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
                        if ( rEditMask.GetChar( nNewPos ) != EDITMASK_LITERAL )
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
                rbInKeyInput = TRUE;
                pEdit->SetText( aStr, Selection( nNewPos ) );
                pEdit->SetModifyFlag();
                pEdit->Modify();
                rbInKeyInput = FALSE;
            }
            else
                pEdit->SetSelection( Selection( nNewPos ) );

            return TRUE;
        }
        else if ( nKeyCode == KEY_INSERT )
        {
            // InsertModus kann man beim PatternField nur einstellen,
            // wenn Maske an jeder Eingabeposition die gleiche
            // ist
            if ( !bSameMask )
            {
                Sound::Beep();
                return TRUE;
            }
        }
    }

    if ( aCode.IsControlMod() || (cChar < 32) || (cChar == 127) )
        return FALSE;

    Selection aSel = aOldSel;
    aSel.Justify();
    nNewPos = (xub_StrLen)aSel.Min();

    if ( nNewPos < rEditMask.Len() )
    {
        xub_Unicode cPattChar = ImplPatternChar( cChar, rEditMask.GetChar(nNewPos) );
        if ( cPattChar )
            cChar = cPattChar;
        else
        {
            // Wenn kein gueltiges Zeichen, dann testen wir, ob der
            // Anwender zum naechsten Literal springen wollte. Dies machen
            // wir nur, wenn er hinter einem Zeichen steht, damit
            // eingebene Literale die automatisch uebersprungenen wurden
            // nicht dazu fuehren, das der Anwender dann da steht, wo
            // er nicht stehen wollte.
            if ( nNewPos &&
                 (rEditMask.GetChar(nNewPos-1) != EDITMASK_LITERAL) &&
                 !aSel.Len() )
            {
                // Naechstes Zeichen suchen, was kein Literal ist
                nTempPos = nNewPos;
                while ( nTempPos < rEditMask.Len() )
                {
                    if ( rEditMask.GetChar(nTempPos) == EDITMASK_LITERAL )
                    {
                        // Gilt nur, wenn ein Literalzeichen vorhanden
                        if ( (rEditMask.GetChar(nTempPos+1) != EDITMASK_LITERAL ) &&
                             ImplKommaPointCharEqual( cChar, rLiteralMask.GetChar(nTempPos) ) )
                        {
                            nTempPos++;
                            ImplPatternMaxPos( pEdit->GetText(), rEditMask, nFormatFlags, bSameMask, nNewPos, nTempPos );
                            if ( nTempPos > nNewPos )
                            {
                                pEdit->SetSelection( Selection( nTempPos ) );
                                return TRUE;
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
        BOOL        bError = FALSE;
        if ( bSameMask && pEdit->IsInsertMode() )
        {
            // Text um Spacezeichen und Literale am Ende kuerzen, bis zur
            // aktuellen Position
            xub_StrLen n = aStr.Len();
            while ( n && (n > nNewPos) )
            {
                if ( (aStr.GetChar( n-1 ) != ' ') &&
                     ((n > rEditMask.Len()) || (rEditMask.GetChar(n-1) != EDITMASK_LITERAL)) )
                    break;

                n--;
            }
            aStr.Erase( n );

            if ( aSel.Len() )
                aStr.Erase( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );

            if ( aStr.Len() < rEditMask.Len() )
            {
                // String evtl. noch bis Cursor-Position erweitern
                if ( aStr.Len() < nNewPos )
                    aStr += rLiteralMask.Copy( aStr.Len(), nNewPos-aStr.Len() );
                if ( nNewPos < aStr.Len() )
                    aStr.Insert( cChar, nNewPos );
                else if ( nNewPos < rEditMask.Len() )
                    aStr += cChar;
                aStr = ImplPatternReformat( aStr, rEditMask, rLiteralMask, nFormatFlags );
            }
            else
                bError = TRUE;
        }
        else
        {
            if ( aSel.Len() )
            {
                // Selection loeschen
                XubString aRep = rLiteralMask.Copy( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );
                aStr.Replace( (xub_StrLen)aSel.Min(), aRep.Len(), aRep );
            }

            if ( nNewPos < aStr.Len() )
                aStr.SetChar( nNewPos, cChar );
            else if ( nNewPos < rEditMask.Len() )
                aStr += cChar;
        }

        if ( bError )
            Sound::Beep();
        else
        {
            rbInKeyInput = TRUE;
            Selection aNewSel( ImplPatternRightPos( aStr, rEditMask, nFormatFlags, bSameMask, nNewPos ) );
            pEdit->SetText( aStr, aNewSel );
            pEdit->SetModifyFlag();
            pEdit->Modify();
            rbInKeyInput = FALSE;
        }
    }
    else
        Sound::Beep();

    return TRUE;
}

// -----------------------------------------------------------------------

void PatternFormatter::ImplSetMask( const ByteString& rEditMask,
                                    const XubString& rLiteralMask )
{
    maEditMask      = rEditMask;
    maLiteralMask   = rLiteralMask;
    mbSameMask      = TRUE;

    if ( maEditMask.Len() != maLiteralMask.Len() )
    {
        if ( maEditMask.Len() < maLiteralMask.Len() )
            maLiteralMask.Erase( maEditMask.Len() );
        else
            maLiteralMask.Expand( maEditMask.Len(), ' ' );
    }

    // StrictModus erlaubt nur Input-Mode, wenn als Maske nur
    // gleiche Zeichen zugelassen werden und als Vorgabe nur
    // Spacezeichen vorgegeben werden, die durch die Maske
    // nicht zugelassen sind
    xub_StrLen  i = 0;
    sal_Char    c = 0;
    while ( i < rEditMask.Len() )
    {
        sal_Char cTemp = rEditMask.GetChar( i );
        if ( cTemp != EDITMASK_LITERAL )
        {
            if ( (cTemp == EDITMASK_ALLCHAR) ||
                 (cTemp == EDITMASK_UPPERALLCHAR) ||
                 (cTemp == EDITMASK_NUMSPACE) )
            {
                mbSameMask = FALSE;
                break;
            }
            if ( i < rLiteralMask.Len() )
            {
                if ( rLiteralMask.GetChar( i ) != ' ' )
                {
                    mbSameMask = FALSE;
                    break;
                }
            }
            if ( !c )
                c = cTemp;
            if ( cTemp != c )
            {
                mbSameMask = FALSE;
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
    mbSameMask          = TRUE;
    mbInPattKeyInput    = FALSE;
}

// -----------------------------------------------------------------------

void PatternFormatter::ImplLoadRes( const ResId& rResId )
{
    ByteString  aEditMask;
    XubString   aLiteralMask;
    ResMgr*     pMgr = Resource::GetResManager();
    USHORT      nMask = pMgr->ReadShort();

    if ( PATTERNFORMATTER_STRICTFORMAT & nMask )
        SetStrictFormat( (BOOL)pMgr->ReadShort() );

    if ( PATTERNFORMATTER_EDITMASK & nMask )
        aEditMask = ByteString( pMgr->ReadString(), RTL_TEXTENCODING_ASCII_US );

    if ( PATTERNFORMATTER_LITTERALMASK & nMask )
        aLiteralMask = pMgr->ReadString();

    if ( (PATTERNFORMATTER_EDITMASK | PATTERNFORMATTER_LITTERALMASK) & nMask )
        ImplSetMask( aEditMask, aLiteralMask );
}

// -----------------------------------------------------------------------

PatternFormatter::~PatternFormatter()
{
}

// -----------------------------------------------------------------------

void PatternFormatter::SetMask( const ByteString& rEditMask,
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
        MarkToBeReformatted( FALSE );
    }
}

// -----------------------------------------------------------------------

XubString PatternFormatter::GetString() const
{
    if ( !GetField() )
        return ImplGetSVEmptyStr();
    else
        return ImplPatternReformat( GetField()->GetText(), maEditMask, maLiteralMask, mnFormatFlags );
}

// -----------------------------------------------------------------------

void PatternFormatter::Reformat()
{
    if ( GetField() )
    {
        ImplSetText( ImplPatternReformat( GetField()->GetText(), maEditMask, maLiteralMask, mnFormatFlags ) );
        if ( !mbSameMask && IsStrictFormat() && !GetField()->IsReadOnly() )
            GetField()->SetInsertMode( FALSE );
    }
}

// -----------------------------------------------------------------------

void PatternFormatter::SelectFixedFont()
{
    if ( GetField() )
    {
        Font aFont = System::GetStandardFont( STDFONT_FIXED );
        Font aControlFont;
        aControlFont.SetName( aFont.GetName() );
        aControlFont.SetFamily( aFont.GetFamily() );
        aControlFont.SetPitch( aFont.GetPitch() );
        GetField()->SetControlFont( aControlFont );
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

PatternField::PatternField( Window* pParent, const ResId& rResId ) :
    SpinField( WINDOW_PATTERNFIELD )
{
    rResId.SetRT( RSC_PATTERNFIELD );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    SetField( this );
    SpinField::ImplLoadRes( rResId );
    PatternFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
    Reformat();

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

PatternField::~PatternField()
{
}

// -----------------------------------------------------------------------

long PatternField::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsControlMod() )
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
        MarkToBeReformatted( FALSE );
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
            MarkToBeReformatted( TRUE );
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

PatternBox::PatternBox( Window* pParent, const ResId& rResId ) :
    ComboBox( WINDOW_PATTERNBOX )
{
    rResId.SetRT( RSC_PATTERNBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );

    SetField( this );
    ComboBox::ImplLoadRes( rResId );
    PatternFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
    Reformat();

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

PatternBox::~PatternBox()
{
}

// -----------------------------------------------------------------------

long PatternBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsControlMod() )
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
        MarkToBeReformatted( FALSE );
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
            MarkToBeReformatted( TRUE );
    }

    ComboBox::Modify();
}

// -----------------------------------------------------------------------

void PatternBox::ReformatAll()
{
    XubString aStr;
    SetUpdateMode( FALSE );
    USHORT nEntryCount = GetEntryCount();
    for ( USHORT i=0; i < nEntryCount; i++ )
    {
        aStr = ImplPatternReformat( GetEntry( i ), GetEditMask(), GetLiteralMask(), GetFormatFlags() );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    PatternFormatter::Reformat();
    SetUpdateMode( TRUE );
}

// -----------------------------------------------------------------------

void PatternBox::InsertString( const XubString& rStr, USHORT nPos )
{
    ComboBox::InsertEntry( ImplPatternReformat( rStr, GetEditMask(), GetLiteralMask(), GetFormatFlags() ), nPos );
}

// -----------------------------------------------------------------------

void PatternBox::RemoveString( const XubString& rStr )
{
    ComboBox::RemoveEntry( ImplPatternReformat( rStr, GetEditMask(), GetLiteralMask(), GetFormatFlags() ) );
}

// -----------------------------------------------------------------------

XubString PatternBox::GetString( USHORT nPos ) const
{
    return ImplPatternReformat( ComboBox::GetEntry( nPos ), GetEditMask(), GetLiteralMask(), GetFormatFlags() );
}

// -----------------------------------------------------------------------

USHORT PatternBox::GetStringPos( const XubString& rStr ) const
{
    return ComboBox::GetEntryPos( ImplPatternReformat( rStr, GetEditMask(), GetLiteralMask(), GetFormatFlags() ) );
}

// =======================================================================

static BOOL ImplNeed4DigitYear( USHORT nYear, const AllSettings& rSettings )
{
    USHORT nTwoDigitYearStart = rSettings.GetMiscSettings().GetTwoDigitYearStart();

    // Wenn Jahr nicht im 2stelligen Grenzbereich liegt,
    if ( (nYear < nTwoDigitYearStart) || (nYear >= nTwoDigitYearStart+100) )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

static USHORT ImplCutDayFromString( XubString& rStr, const International& )
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
    return (USHORT)aNumStr.ToInt32();
}

// -----------------------------------------------------------------------

static USHORT ImplCutMonthFromString( XubString& rStr, const International& rInter )
{
    USHORT nPos;

    //Nach Monatsnamen suchen
    for ( USHORT i=1; i <= 12; i++ )
    {
        const XubString& rMonthName = rInter.GetMonthText( i );
        // Voller Monatsname ?
        nPos = rStr.Search( rMonthName );
        if ( nPos != STRING_NOTFOUND )
        {
            rStr.Erase( 0, nPos + rMonthName.Len() );
            return i;
        }
        // Kurzer Monatsname ?
        const XubString& rAbbrevMonthName = rInter.GetAbbrevMonthText( i );
        nPos = rStr.Search( rAbbrevMonthName );
        if ( nPos != STRING_NOTFOUND )
        {
            rStr.Erase( 0, nPos + rAbbrevMonthName.Len() );
            return i;
        }
    }

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
    return (USHORT)aNumStr.ToInt32();
}

// -----------------------------------------------------------------------

static USHORT ImplCutYearFromString( XubString& rStr, const International& )
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
        aNumStr.Erase( 0, 1 );
    }
    return (USHORT)aNumStr.ToInt32();
}

// -----------------------------------------------------------------------

static BOOL ImplDateProcessKeyInput( Edit*, const KeyEvent& rKEvt,
                                     BOOL bStrictFormat, BOOL bLongFormat,
                                     const International& rInter  )
{
    xub_Unicode cChar = rKEvt.GetCharCode();

    if ( !bStrictFormat || bLongFormat )
        return FALSE;
    else
    {
        USHORT nGroup = rKEvt.GetKeyCode().GetGroup();
        if ( (nGroup == KEYGROUP_FKEYS) || (nGroup == KEYGROUP_CURSOR) ||
             (nGroup == KEYGROUP_MISC)||
             ((cChar >= '0') && (cChar <= '9')) ||
             (cChar == rInter.GetDateSep()) )
            return FALSE;
        else
            return TRUE;
    }
}

// -----------------------------------------------------------------------

static BOOL ImplDateGetValue( const XubString& rStr, Date& rDate,
                              BOOL bLongFormat, const International& rInter,
                              const AllSettings& rSettings )
{
    XubString   aStr = rStr;
    BOOL        bStrLongFormat = FALSE;
    xub_StrLen  nSep1Pos;
    xub_StrLen  nSep2Pos;
    USHORT      mnFirst;
    USHORT      nSecond;
    USHORT      nThird;
    Date        aDate( 0, 0, 0 );
    DateFormat  eDateFormat;

    if ( !rStr.Len() )
        return FALSE;

    if ( bLongFormat )
        eDateFormat = rInter.GetLongDateFormat();
    else
        eDateFormat = rInter.GetDateFormat();

    // Sind da Buchstaben drin ?

    bStrLongFormat = isAlphaType( ImplGetCharClass()->getStringType( aStr, 0, aStr.Len(), CreateLocale(rInter) ) );

    // Bei enthaltenen Buchstaben gehen wir davon aus, das ein langes
    // Datumsformat eingegeben wurde
    if ( bStrLongFormat )
    {
        switch( eDateFormat )
        {
            case MDY:
                mnFirst = ImplCutMonthFromString( aStr, rInter );
                nSecond = ImplCutDayFromString( aStr, rInter );
                nThird  = ImplCutYearFromString( aStr, rInter );
                break;
            case DMY:
                mnFirst = ImplCutDayFromString( aStr, rInter );
                nSecond = ImplCutMonthFromString( aStr, rInter );
                nThird  = ImplCutYearFromString( aStr, rInter );
                break;
            case YMD:
            default:
                mnFirst = ImplCutYearFromString( aStr, rInter );
                nSecond = ImplCutMonthFromString( aStr, rInter );
                nThird  = ImplCutDayFromString( aStr, rInter );
                break;
        }
    }
    else
    {
        // Nach Separatoren suchen
        XubString aSepStr( RTL_CONSTASCII_USTRINGPARAM( ",.;:-/" ) );

        // Die obigen Zeichen durch das Separatorzeichen ersetzen
        for ( xub_StrLen i = 0; i < aSepStr.Len(); i++ )
        {
            if ( aSepStr.GetChar( i ) == rInter.GetDateSep() )
                continue;
            for ( xub_StrLen j = 0; j < aStr.Len(); j++ )
            {
                if ( aStr.GetChar( j ) == aSepStr.GetChar( i ) )
                    aStr.SetChar( j, rInter.GetDateSep() );
            }
        }

        nSep1Pos = aStr.Search( rInter.GetDateSep() );
        if ( nSep1Pos == STRING_NOTFOUND )
            return FALSE;
        nSep2Pos = aStr.Search( rInter.GetDateSep(), nSep1Pos+1 );

        // Kein Jahr eingegeben ?
        if ( nSep2Pos == STRING_NOTFOUND )
        {
            switch( eDateFormat )
            {
                case DMY:
                case MDY:
                    nSep2Pos = aStr.Len();
                    aStr += rInter.GetDateSep();
                    aStr += aDate.GetYear();
                    break;
                default:
                case YMD:
                {
                    nSep2Pos = nSep1Pos;
                    XubString aYearStr( aDate.GetYear() );
                    nSep2Pos = aYearStr.Len();
                    aStr.Insert( rInter.GetDateSep(), 0 );
                    aStr.Insert( aYearStr, 0 );
                }
                break;
            }
        }

        mnFirst = (USHORT)aStr.Copy( 0, nSep1Pos ).ToInt32();
        aStr.Erase( 0, nSep1Pos+1 );
        nSecond = (USHORT)aStr.Copy( 0, nSep2Pos-nSep1Pos-1 ).ToInt32();
        aStr.Erase( 0, nSep2Pos-nSep1Pos-1+1 );
        nThird  = (USHORT)aStr.ToInt32();
    }

    switch ( eDateFormat )
    {
        case MDY:
            if ( !nSecond || !mnFirst )
                return FALSE;
            aDate = Date( nSecond, mnFirst, nThird );
            break;
        case DMY:
            if ( !mnFirst || !nSecond )
                return FALSE;
            aDate = Date( mnFirst, nSecond, nThird );
            break;
        default:
        case YMD:
            if ( !nSecond || !nThird )
                return FALSE;
            aDate = Date( nThird, nSecond, mnFirst );
            break;
    }

    DateFormatter::ExpandCentury( aDate, rSettings.GetMiscSettings().GetTwoDigitYearStart() );
    if ( (aDate.GetDay() > 31) || (aDate.GetMonth() > 12) )
        return FALSE;
    if ( !aDate.IsValid() )
        return FALSE;
    rDate = aDate;

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL DateFormatter::ImplDateReformat( const XubString& rStr, XubString& rOutStr,
                                      const AllSettings& rSettings )
{
    Date aDate( 0, 0, 0 );
    if ( !ImplDateGetValue( rStr, aDate, mbLongFormat, GetInternational(), GetFieldSettings() ) )
        return TRUE;

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
            maCorrectedDate = Date();
            return FALSE;
        }
        else
            maCorrectedDate = Date();
    }

    rOutStr = ImplGetDateAsText( aTempDate, rSettings );

    return TRUE;
}

// -----------------------------------------------------------------------

XubString DateFormatter::ImplGetDateAsText( const Date& rDate,
                                            const AllSettings& rSettings ) const
{
    XubString aDateAsText;

    International aIntn = GetInternational();
    BOOL b4DigitYear = ImplNeed4DigitYear( rDate.GetYear(), rSettings );
    if ( mbLongFormat )
    {
        if ( b4DigitYear && !aIntn.IsLongDateCentury() )
            aIntn.SetLongDateCentury( TRUE );
        aDateAsText = aIntn.GetLongDate( rDate );
    }
    else
    {
        if ( b4DigitYear && !aIntn.IsDateCentury() )
            aIntn.SetDateCentury( TRUE );
        aDateAsText = aIntn.GetDate( rDate );
    }

    return aDateAsText;
}

// -----------------------------------------------------------------------

static void ImplDateIncrementDay( Date& rDate, BOOL bUp )
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

static void ImplDateIncrementMonth( Date& rDate, BOOL bUp )
{
    DateFormatter::ExpandCentury( rDate );

    USHORT nMonth = rDate.GetMonth();
    USHORT nYear = rDate.GetYear();
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

    USHORT nDaysInMonth = rDate.GetDaysInMonth();
    if ( rDate.GetDay() > nDaysInMonth )
        rDate.SetDay( nDaysInMonth );
}

// -----------------------------------------------------------------------

static void ImplDateIncrementYear( Date& rDate, BOOL bUp )
{
    DateFormatter::ExpandCentury( rDate );

    USHORT nYear = rDate.GetYear();
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

void DateField::ImplDateSpinArea( BOOL bUp )
{
    // Wenn alles selektiert ist, Tage hochzaehlen
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

            DateFormat eFormat;
            if ( IsLongFormat() )
                eFormat = GetInternational().GetLongDateFormat();
            else
                eFormat = GetInternational().GetDateFormat();

            if ( !IsLongFormat() )
            {
                // Area suchen
                xub_StrLen nPos = 0;
                for ( xub_StrLen i = 1; i <= 3; i++ )
                {
                    nPos = aText.Search( GetInternational().GetDateSep(), nPos );
                    if ( nPos >= (USHORT)aSelection.Max() )
                    {
                        nDateArea = i;
                        break;
                    }
                    else
                        nPos++;
                }
            }
            else
                nDateArea = 1;

            switch( eFormat )
            {
                case MDY:
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
                case DMY:
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
                case YMD:
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
            }
        }

        ImplNewFieldValue( aDate );
    }
}

// -----------------------------------------------------------------------

void DateFormatter::ImplInit()
{
    mbLongFormat    = FALSE;
}

// -----------------------------------------------------------------------

DateFormatter::DateFormatter() :
    maMin( 1, 1, 1900 ),
    maMax( 31, 12, 2200 ),
    maFieldDate( 0 ),
    maLastDate( 0 )
{
    ImplInit();
}

// -----------------------------------------------------------------------

void DateFormatter::ImplLoadRes( const ResId& )
{
    ResMgr*     pMgr = Resource::GetResManager();
    USHORT      nMask = pMgr->ReadShort();

    if ( DATEFORMATTER_MIN & nMask )
    {
        maMin = Date( ResId( (RSHEADER_TYPE *)pMgr->GetClass() ) );
        pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE*)pMgr->GetClass() ) );
    }
    if ( DATEFORMATTER_MAX & nMask )
    {
        maMax = Date( ResId( (RSHEADER_TYPE *)pMgr->GetClass() ) );
        pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE*)pMgr->GetClass() ) );
    }
    if ( DATEFORMATTER_LONGFORMAT & nMask )
        mbLongFormat = (BOOL)pMgr->ReadShort();

    if ( DATEFORMATTER_STRICTFORMAT & nMask )
        SetStrictFormat( (BOOL)pMgr->ReadShort() );

    if ( DATEFORMATTER_I12 & nMask )
    {
        SetInternational( International( ResId( (RSHEADER_TYPE *)pMgr->GetClass() ) ) );
        pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE*)pMgr->GetClass() ) );
    }
    if ( DATEFORMATTER_VALUE & nMask )
    {
        maFieldDate = Date( ResId( (RSHEADER_TYPE *)pMgr->GetClass() ) );
        pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE*)pMgr->GetClass() ) );
        if ( maFieldDate > maMax )
            maFieldDate = maMax;
        if ( maFieldDate < maMin )
            maFieldDate = maMin;
        maLastDate = maFieldDate;
    }
}

// -----------------------------------------------------------------------

DateFormatter::~DateFormatter()
{
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

void DateFormatter::SetLongFormat( BOOL bLong )
{
    mbLongFormat = bLong;
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
        // Wenn bis ans Ende selektiert war, soll das auch so bleiben...
        if ( (xub_StrLen)aSelection.Max() == aText.Len() )
        {
            if ( !aSelection.Len() )
                aSelection.Min() = SELECTION_MAX;
            aSelection.Max() = SELECTION_MAX;
        }

        Date aOldLastDate  = maLastDate;
        ImplSetUserDate( rDate, &aSelection );
        maLastDate = aOldLastDate;

        // Modify am Edit wird nur bei KeyInput gesetzt...
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
        if ( ImplDateGetValue( GetField()->GetText(), aDate,
                               mbLongFormat, GetInternational(),
                               GetFieldSettings() ) )
        {
            if ( aDate > maMax )
                aDate = maMax;
            else if ( aDate < maMin )
                aDate = maMin;
        }
        else
        {
            // !!! TH-18.2.99: Wenn wir Zeit haben sollte einmal
            // !!! geklaert werden, warum dieses beim Datum gegenueber
            // !!! allen anderen Feldern anders behandelt wird.
            // !!! Siehe dazu Bug: 52304

            if ( maLastDate.GetDate() )
                aDate = maLastDate;
            else if ( !IsEmptyFieldValueEnabled() )
                aDate = Date();
        }
    }

    return aDate;
}

// -----------------------------------------------------------------------

Date DateFormatter::GetRealDate() const
{
    // !!! TH-18.2.99: Wenn wir Zeit haben sollte dieses auch einmal
    // !!! fuer die Numeric-Klassen eingebaut werden.

    Date aDate( 0, 0, 0 );

    if ( GetField() )
    {
        ImplDateGetValue( GetField()->GetText(), aDate,
                          mbLongFormat, GetInternational(),
                          GetFieldSettings() );
    }

    return aDate;
}

// -----------------------------------------------------------------------

void DateFormatter::SetEmptyDate()
{
    FormatterBase::SetEmptyFieldValue();
}

// -----------------------------------------------------------------------

BOOL DateFormatter::IsEmptyDate() const
{
    BOOL bEmpty = FormatterBase::IsEmptyFieldValue();

    if ( GetField() && MustBeReformatted() && IsEmptyFieldValueEnabled() )
    {
        if ( !GetField()->GetText().Len() )
        {
            bEmpty = TRUE;
        }
        else if ( !maLastDate.GetDate() )
        {
            Date aDate;
            bEmpty = !ImplDateGetValue( GetField()->GetText(), aDate,
                                       mbLongFormat, GetInternational(),
                                       GetFieldSettings() );
        }
    }
    return bEmpty;
}

// -----------------------------------------------------------------------

BOOL DateFormatter::IsDateModified() const
{
    if ( ImplGetEmptyFieldValue() )
        return !IsEmptyDate();
    else if ( GetDate() != maFieldDate )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void DateFormatter::Reformat()
{
    if ( !GetField() )
        return;

    if ( !GetField()->GetText().Len() && ImplGetEmptyFieldValue() )
        return;

    XubString aStr;
    BOOL bOK = ImplDateReformat( GetField()->GetText(), aStr, GetFieldSettings() );
    if( !bOK )
        return;

    if ( aStr.Len() )
    {
        ImplSetText( aStr );
        ImplDateGetValue( aStr, maLastDate, mbLongFormat,
                          GetInternational(), GetFieldSettings() );
    }
    else
    {
        if ( maLastDate.GetDate() )
            SetDate( maLastDate );
        else if ( !IsEmptyFieldValueEnabled() )
            SetDate( Date() );
        else
        {
            ImplSetText( ImplGetSVEmptyStr() );
            ImplGetEmptyFieldValue() = TRUE;
        }
    }
}

// -----------------------------------------------------------------------

void DateFormatter::ExpandCentury( Date& rDate )
{
    ExpandCentury( rDate, Application::GetSettings().GetMiscSettings().GetTwoDigitYearStart() );
}

// -----------------------------------------------------------------------

void DateFormatter::ExpandCentury( Date& rDate, USHORT nTwoDigitYearStart )
{
    USHORT nDateYear = rDate.GetYear();
    if ( nDateYear < 100 )
    {
        USHORT nCentury = nTwoDigitYearStart / 100;
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
    SetText( GetInternational().GetDate( ImplGetFieldDate() ) );
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
    SetText( GetInternational().GetDate( ImplGetFieldDate() ) );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE ) )
        Show();

    ResetLastDate();
}

// -----------------------------------------------------------------------

void DateField::ImplLoadRes( const ResId& rResId )
{
    SpinField::ImplLoadRes( rResId );
    DateFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes() ) );

    USHORT  nMask = ReadShortRes();

    if ( DATEFIELD_FIRST & nMask )
    {
        maFirst = Date( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
    }
    if ( DATEFIELD_LAST & nMask )
    {
        maLast = Date( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
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
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsControlMod() )
    {
        if ( ImplDateProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsLongFormat(), GetInternational() ) )
            return 1;
    }

    return SpinField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long DateField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( FALSE );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() )
        {
            // !!! TH-18.2.99: Wenn wir Zeit haben sollte einmal
            // !!! geklaert werden, warum dieses beim Datum gegenueber
            // !!! allen anderen Feldern anders behandelt wird.
            // !!! Siehe dazu Bug: 52304

            BOOL bTextLen = GetText().Len() != 0;
            if ( bTextLen || !IsEmptyFieldValueEnabled() )
                Reformat();
            else if ( !bTextLen && IsEmptyFieldValueEnabled() )
            {
                ResetLastDate();
                ImplGetEmptyFieldValue() = TRUE;
            }
        }
    }

    return SpinField::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void DateField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & (SETTINGS_INTERNATIONAL | SETTINGS_MISC)) )
        ReformatAll();
}

// -----------------------------------------------------------------------

void DateField::Modify()
{
    MarkToBeReformatted( TRUE );
    SpinField::Modify();
}

// -----------------------------------------------------------------------

void DateField::Up()
{
    ImplDateSpinArea( TRUE );
    SpinField::Up();
}

// -----------------------------------------------------------------------

void DateField::Down()
{
    ImplDateSpinArea( FALSE );
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

void DateField::SetExtFormat( ExtDateFieldFormat eFormat )
{
    Date aDate = GetDate();
    International aInt( Application::GetAppInternational() );

    BOOL bLongFormat = FALSE;   // immer bis auf XTDATEF_SYSTEM_LONG

    switch ( eFormat )
    {
        case XTDATEF_SYSTEM_SHORT:
        {
        }
        break;
        case XTDATEF_SYSTEM_SHORT_YY:
        {
            aInt.SetDateCentury( FALSE );
        }
        break;
        case XTDATEF_SYSTEM_SHORT_YYYY:
        {
            aInt.SetDateCentury( TRUE );
        }
        break;
        case XTDATEF_SYSTEM_LONG:
        {
            bLongFormat = TRUE;
        }
        break;
        case XTDATEF_SHORT_DDMMYY:
        {
            aInt.SetDateCentury( FALSE );
            aInt.SetDateFormat( DMY );
        }
        break;
        case XTDATEF_SHORT_MMDDYY:
        {
            aInt.SetDateCentury( FALSE );
            aInt.SetDateFormat( MDY );
        }
        break;
        case XTDATEF_SHORT_YYMMDD:
        {
            aInt.SetDateCentury( FALSE );
            aInt.SetDateFormat( YMD );
        }
        break;
        case XTDATEF_SHORT_DDMMYYYY:
        {
            aInt.SetDateCentury( TRUE );
            aInt.SetDateFormat( DMY );
        }
        break;
        case XTDATEF_SHORT_MMDDYYYY:
        {
            aInt.SetDateCentury( TRUE );
            aInt.SetDateFormat( MDY );
        }
        break;
        case XTDATEF_SHORT_YYYYMMDD:
        {
            aInt.SetDateCentury( TRUE );
            aInt.SetDateFormat( YMD );
        }
        break;
        case XTDATEF_SHORT_YYMMDD_DIN5008:
        {
            aInt.SetDateCentury( FALSE );
            aInt.SetDateSep( '-' );
            aInt.SetDateFormat( YMD );
        }
        break;
        case XTDATEF_SHORT_YYYYMMDD_DIN5008:
        {
            aInt.SetDateCentury( TRUE );
            aInt.SetDateSep( '-' );
            aInt.SetDateFormat( YMD );
        }
        break;
        default:    DBG_ERROR( "ExtDateFieldFormat unknown!" );
    }

    SetInternational( aInt );
    SetLongFormat( bLongFormat );
    if ( GetField() && GetField()->GetText().Len() )
        SetUserDate( aDate );
}

// -----------------------------------------------------------------------

DateBox::DateBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    SetText( GetInternational().GetDate( ImplGetFieldDate() ) );
    Reformat();
}

// -----------------------------------------------------------------------

DateBox::DateBox( Window* pParent, const ResId& rResId ) :
    ComboBox( WINDOW_DATEBOX )
{
    rResId.SetRT( RSC_DATEBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ComboBox::ImplInit( pParent, nStyle );
    SetField( this );
    SetText( GetInternational().GetDate( ImplGetFieldDate() ) );
    ComboBox::ImplLoadRes( rResId );
    DateFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
    Reformat();

    if ( !( nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

DateBox::~DateBox()
{
}

// -----------------------------------------------------------------------

long DateBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsControlMod() )
    {
        if ( ImplDateProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), IsLongFormat(), GetInternational() ) )
            return 1;
    }

    return ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

void DateBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    ComboBox::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_INTERNATIONAL) )
        ReformatAll();
}

// -----------------------------------------------------------------------

long DateBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( FALSE );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() )
        {
            BOOL bTextLen = GetText().Len() != 0;
            if ( bTextLen || !IsEmptyFieldValueEnabled() )
                Reformat();
            else if ( !bTextLen && IsEmptyFieldValueEnabled() )
            {
                ResetLastDate();
                ImplGetEmptyFieldValue() = TRUE;
            }
        }
    }

    return ComboBox::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void DateBox::Modify()
{
    MarkToBeReformatted( TRUE );
    ComboBox::Modify();
}

// -----------------------------------------------------------------------

void DateBox::ReformatAll()
{
    XubString aStr;
    SetUpdateMode( FALSE );
    USHORT nEntryCount = GetEntryCount();
    for ( USHORT i=0; i < nEntryCount; i++ )
    {
        ImplDateReformat( GetEntry( i ), aStr, GetFieldSettings() );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    DateFormatter::Reformat();
    SetUpdateMode( TRUE );
}

// -----------------------------------------------------------------------

void DateBox::InsertDate( const Date& rDate, USHORT nPos )
{
    Date aDate = rDate;
    if ( aDate > GetMax() )
        aDate = GetMax();
    else if ( aDate < GetMin() )
        aDate = GetMin();

    ComboBox::InsertEntry( ImplGetDateAsText( aDate, GetFieldSettings() ), nPos );
}

// -----------------------------------------------------------------------

void DateBox::RemoveDate( const Date& rDate )
{
    ComboBox::RemoveEntry( ImplGetDateAsText( rDate, GetFieldSettings() ) );
}

// -----------------------------------------------------------------------

Date DateBox::GetDate( USHORT nPos ) const
{
    Date aDate( 0, 0, 0 );
    ImplDateGetValue( ComboBox::GetEntry( nPos ), aDate, IsLongFormat(),
                      GetInternational(), GetSettings() );
    return aDate;
}

// -----------------------------------------------------------------------

USHORT DateBox::GetDatePos( const Date& rDate ) const
{
    XubString aStr;
    if ( IsLongFormat() )
        aStr = GetInternational().GetLongDate( rDate );
    else
        aStr = GetInternational().GetDate( rDate );
    return ComboBox::GetEntryPos( aStr );
}

// -----------------------------------------------------------------------

static BOOL ImplTimeProcessKeyInput( Edit*, const KeyEvent& rKEvt,
                                     BOOL bStrictFormat, BOOL bDuration,
                                     TimeFieldFormat eFormat,
                                     const International& rInter  )
{
    xub_Unicode cChar = rKEvt.GetCharCode();

    if ( !bStrictFormat )
        return FALSE;
    else
    {
        USHORT nGroup = rKEvt.GetKeyCode().GetGroup();
        if ( (nGroup == KEYGROUP_FKEYS) || (nGroup == KEYGROUP_CURSOR) ||
             (nGroup == KEYGROUP_MISC)   ||
             ((cChar >= '0') && (cChar <= '9')) ||
             (cChar == rInter.GetTimeSep()) ||
             ((eFormat == TIMEF_100TH_SEC) && (cChar == rInter.GetTime100SecSep())) ||
             ((eFormat == TIMEF_SEC_CS) && (cChar == rInter.GetTime100SecSep())) ||
             (bDuration && (cChar == '-')) )
            return FALSE;
        else
            return TRUE;
    }
}

// -----------------------------------------------------------------------

static BOOL ImplTimeGetValue( const XubString& rStr, Time& rTime,
                              TimeFieldFormat eTimeFormat, BOOL bDuration,
                              const International& rInter )
{
    XubString   aStr    = rStr;
    short       nHour   = 0;
    short       nMinute = 0;
    short       nSecond = 0;
    short       n100Sec = 0;
    Time        aTime( 0, 0, 0 );

    if ( !rStr.Len() )
        return FALSE;

    // Nach Separatoren suchen
    XubString aSepStr( RTL_CONSTASCII_USTRINGPARAM( ",.;:/" ) );
    if ( !bDuration )
        aSepStr.Append( '-' );

    // Die obigen Zeichen durch das Separatorzeichen ersetzen
    for ( xub_StrLen i = 0; i < aSepStr.Len(); i++ )
    {
        if ( aSepStr.GetChar( i ) == rInter.GetTimeSep() )
            continue;
        for ( xub_StrLen j = 0; j < aStr.Len(); j++ )
        {
            if ( aStr.GetChar( j ) == aSepStr.GetChar( i ) )
                aStr.SetChar( j, rInter.GetTimeSep() );
        }
    }

    BOOL bNegative = FALSE;
    xub_StrLen nSepPos = aStr.Search( rInter.GetTimeSep() );
    if ( aStr.GetChar( 0 ) == '-' )
        bNegative = TRUE;
    if ( eTimeFormat != TIMEF_SEC_CS )
    {
        if ( nSepPos == STRING_NOTFOUND )
            return FALSE;
        nHour = (short)aStr.Copy( 0, nSepPos ).ToInt32();
        aStr.Erase( 0, nSepPos+1 );

        nSepPos = aStr.Search( rInter.GetTimeSep() );
        if ( aStr.GetChar( 0 ) == '-' )
            bNegative = TRUE;
        if ( nSepPos != STRING_NOTFOUND )
        {
            nMinute = (short)aStr.Copy( 0, nSepPos ).ToInt32();
            aStr.Erase( 0, nSepPos+1 );

            nSepPos = aStr.Search( rInter.GetTimeSep() );
            if ( aStr.GetChar( 0 ) == '-' )
                bNegative = TRUE;
            if ( nSepPos != STRING_NOTFOUND )
            {
                nSecond = (short)aStr.Copy( 0, nSepPos ).ToInt32();
                aStr.Erase( 0, nSepPos+1 );
                if ( aStr.GetChar( 0 ) == '-' )
                    bNegative = TRUE;
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

        nSepPos = aStr.Search( rInter.GetTimeSep() );
        if ( aStr.GetChar( 0 ) == '-' )
            bNegative = TRUE;
        if ( nSepPos != STRING_NOTFOUND )
        {
            nMinute = nSecond;
            nSecond = (short)aStr.Copy( 0, nSepPos ).ToInt32();
            aStr.Erase( 0, nSepPos+1 );

            nSepPos = aStr.Search( rInter.GetTimeSep() );
            if ( aStr.GetChar( 0 ) == '-' )
                bNegative = TRUE;
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
            xub_StrLen nLen = 1; // mindestens eine Ziffer, weil sonst n100Sec==0

            while ( aStr.GetChar(nLen) >= '0' && aStr.GetChar(nLen) <= '9' )
                nLen++;

            if ( nLen > 2 )
            {
                while( nLen > 3 )
                {
                    n100Sec = n100Sec / 10;
                    nLen--;
                }
                // Rundung bei negativen Zahlen???
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
        return FALSE;

    if ( eTimeFormat == TIMEF_NONE )
        nSecond = n100Sec = 0;
    else if ( eTimeFormat == TIMEF_SEC )
        n100Sec = 0;

    if ( !bDuration )
    {
        if ( bNegative || (nHour < 0) || (nMinute < 0) ||
             (nSecond < 0) || (n100Sec < 0) )
            return FALSE;

        aStr.ToUpperAscii();
        XubString aAM( rInter.GetTimeAM() );
        XubString aPM( rInter.GetTimePM() );
        aAM.ToUpperAscii();
        aPM.ToUpperAscii();

        if ( (aStr.Search( aPM ) != STRING_NOTFOUND) && (nHour < 12) )
            nHour += 12;
        if ( (aStr.Search( aAM ) != STRING_NOTFOUND) && (nHour == 12) )
            nHour = 0;

        aTime = Time( (USHORT)nHour, (USHORT)nMinute, (USHORT)nSecond,
                      (USHORT)n100Sec );
    }
    else
    {
        if ( bNegative || (nHour < 0) || (nMinute < 0) ||
             (nSecond < 0) || (n100Sec < 0) )
        {
            bNegative   = TRUE;
            nHour       = nHour < 0 ? -nHour : nHour;
            nMinute     = nMinute < 0 ? -nMinute : nMinute;
            nSecond     = nSecond < 0 ? -nSecond : nSecond;
            n100Sec     = n100Sec < 0 ? -n100Sec : n100Sec;
        }

        aTime = Time( (USHORT)nHour, (USHORT)nMinute, (USHORT)nSecond,
                      (USHORT)n100Sec );
        if ( bNegative )
            aTime = -aTime;
    }

    rTime = aTime;

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL TimeFormatter::ImplTimeReformat( const XubString& rStr, XubString& rOutStr )
{
    Time aTime( 0, 0, 0 );
    if ( !ImplTimeGetValue( rStr, aTime, meFormat, mbDuration, GetInternational() ) )
        return TRUE;

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
            maCorrectedTime = Time();
            return FALSE;
        }
        else
            maCorrectedTime = Time();
    }

    BOOL bSecond = FALSE;
    BOOL b100Sec = FALSE;
    if ( meFormat != TIMEF_NONE )
        bSecond = TRUE;
    if ( meFormat == TIMEF_100TH_SEC )
        b100Sec = TRUE;

    if ( meFormat == TIMEF_SEC_CS )
    {
        ULONG n  = aTempTime.GetHour() * 3600L;
        n       += aTempTime.GetMin()  * 60L;
        n       += aTempTime.GetSec();
        rOutStr  = String::CreateFromInt32( n );
        rOutStr += GetInternational().GetTime100SecSep();
        if ( aTempTime.Get100Sec() < 10 )
            rOutStr += '0';
        rOutStr += String::CreateFromInt32( aTempTime.Get100Sec() );
    }
    else if ( mbDuration )
        rOutStr = GetInternational().GetDuration( aTempTime, bSecond, b100Sec );
    else
        rOutStr = GetInternational().GetTime( aTempTime, bSecond, b100Sec );

    return TRUE;
}

// -----------------------------------------------------------------------

void TimeField::ImplTimeSpinArea( BOOL bUp )
{
    if ( GetField() )
    {
        xub_StrLen nTimeArea = 0;
        xub_StrLen nPos;

        Time aTime( GetTime() );
        XubString aText( GetText() );
        Selection aSelection( GetField()->GetSelection() );

        // Area suchen
        if ( meFormat != TIMEF_SEC_CS )
        {
            for ( xub_StrLen i = 1, nPos = 0; i <= 4; i++ )
            {
                xub_StrLen nPos1 = aText.Search( GetInternational().GetTimeSep(), nPos );
                xub_StrLen nPos2 = aText.Search( GetInternational().GetTime100SecSep(), nPos );
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
            nPos = aText.Search( GetInternational().GetTime100SecSep() );
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
            if ( !mbDuration )
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
    mbDuration      = FALSE;
}

// -----------------------------------------------------------------------

TimeFormatter::TimeFormatter() :
    maMin( 0, 0 ),
    maMax( 23, 59, 59, 99 ),
    maFieldTime( 0, 0 ),
    maLastTime( 0, 0 )
{
    ImplInit();
}

// -----------------------------------------------------------------------

void TimeFormatter::ImplLoadRes( const ResId& )
{
    ResMgr* pMgr = Resource::GetResManager();
    USHORT  nMask = pMgr->ReadShort();

    if ( TIMEFORMATTER_MIN & nMask )
    {
        SetMin( Time( ResId( (RSHEADER_TYPE *)pMgr->GetClass() ) ) );
        pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE *)pMgr->GetClass() ) );
    }

    if ( TIMEFORMATTER_MAX & nMask )
    {
        SetMax( Time( ResId( (RSHEADER_TYPE *)pMgr->GetClass() ) ) );
        pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE *)pMgr->GetClass() ) );
    }

    if ( TIMEFORMATTER_TIMEFIELDFORMAT & nMask )
        meFormat = (TimeFieldFormat)pMgr->ReadShort();

    if ( TIMEFORMATTER_DURATION & nMask )
        mbDuration = (BOOL)pMgr->ReadShort();

    if ( TIMEFORMATTER_STRICTFORMAT & nMask )
        SetStrictFormat( (BOOL)pMgr->ReadShort() );

    if ( TIMEFORMATTER_I12 & nMask )
    {
        SetInternational( International( ResId( (RSHEADER_TYPE *)pMgr->GetClass() ) ) );
        pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE *)pMgr->GetClass() ) );
    }

    if ( TIMEFORMATTER_VALUE & nMask )
    {
        maFieldTime = Time( ResId( (RSHEADER_TYPE *)pMgr->GetClass() ) );
        if ( maFieldTime > GetMax() )
            maFieldTime = GetMax();
        if ( maFieldTime < GetMin() )
            maFieldTime = GetMin();
        maLastTime = maFieldTime;

        pMgr->Increment( pMgr->GetObjSize( (RSHEADER_TYPE *)pMgr->GetClass() ) );
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

void TimeFormatter::SetFormat( TimeFieldFormat eNewFormat )
{
    meFormat = eNewFormat;
    ReformatAll();
}

// -----------------------------------------------------------------------

void TimeFormatter::SetDuration( BOOL bNewDuration )
{
    mbDuration = bNewDuration;
    ReformatAll();
}

// -----------------------------------------------------------------------

void TimeFormatter::SetTime( const Time& rNewTime )
{
    SetUserTime( rNewTime );
    maFieldTime = maLastTime;
    ImplGetEmptyFieldValue() = FALSE;
}

// -----------------------------------------------------------------------

void TimeFormatter::ImplNewFieldValue( const Time& rTime )
{
    if ( GetField() )
    {
        Selection aSelection = GetField()->GetSelection();
        aSelection.Justify();
        XubString aText = GetField()->GetText();
        // Wenn bis ans Ende selektiert war, soll das auch so bleiben...
        if ( (xub_StrLen)aSelection.Max() == aText.Len() )
        {
            if ( !aSelection.Len() )
                aSelection.Min() = SELECTION_MAX;
            aSelection.Max() = SELECTION_MAX;
        }

        Time aOldLastTime = maLastTime;
        ImplSetUserTime( rTime, &aSelection );
        maLastTime = aOldLastTime;

        // Modify am Edit wird nur bei KeyInput gesetzt...
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
        BOOL bSec    = FALSE;
        BOOL b100Sec = FALSE;
        if ( meFormat != TIMEF_NONE )
            bSec = TRUE;
        if ( meFormat == TIMEF_100TH_SEC || meFormat == TIMEF_SEC_CS )
            b100Sec = TRUE;
        if ( meFormat == TIMEF_SEC_CS )
        {
            ULONG n  = aNewTime.GetHour() * 3600L;
            n       += aNewTime.GetMin()  * 60L;
            n       += aNewTime.GetSec();
            aStr     = String::CreateFromInt32( n );
            aStr    += GetInternational().GetTime100SecSep();
            if ( aNewTime.Get100Sec() < 10 )
                aStr += '0';
            aStr += String::CreateFromInt32( aNewTime.Get100Sec() );
        }
        else if ( mbDuration )
            aStr = GetInternational().GetDuration( aNewTime, bSec, b100Sec );
        else
            aStr = GetInternational().GetTime( aNewTime, bSec, b100Sec );

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
        if ( ImplTimeGetValue( GetField()->GetText(), aTime,
                               meFormat, mbDuration, GetInternational() ) )
        {
            if ( aTime > GetMax() )
                aTime = GetMax();
            else if ( aTime < GetMin() )
                aTime = GetMin();
        }
        else
            aTime = maLastTime;
    }

    return aTime;
}

// -----------------------------------------------------------------------

Time TimeFormatter::GetRealTime() const
{
    Time aTime( 0, 0, 0 );

    if ( GetField() )
    {
        ImplTimeGetValue( GetField()->GetText(), aTime,
                          meFormat, mbDuration, GetInternational() );
    }

    return aTime;
}

// -----------------------------------------------------------------------

BOOL TimeFormatter::IsTimeModified() const
{
    if ( ImplGetEmptyFieldValue() )
        return !IsEmptyTime();
    else if ( GetTime() != maFieldTime )
        return TRUE;
    else
        return FALSE;
}

// -----------------------------------------------------------------------

void TimeFormatter::Reformat()
{
    if ( !GetField() )
        return;

    if ( !GetField()->GetText().Len() && ImplGetEmptyFieldValue() )
        return;

    XubString aStr;
    BOOL bOK = ImplTimeReformat( GetField()->GetText(), aStr );
    if ( !bOK )
        return;

    if ( aStr.Len() )
    {
        ImplSetText( aStr );
        ImplTimeGetValue( aStr, maLastTime, meFormat, mbDuration, GetInternational() );
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
    SetText( GetInternational().GetTime( maFieldTime, FALSE, FALSE ) );
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
    SetText( GetInternational().GetTime( maFieldTime, FALSE, FALSE ) );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE ) )
        Show();
}

// -----------------------------------------------------------------------

void TimeField::ImplLoadRes( const ResId& rResId )
{
    SpinField::ImplLoadRes( rResId );
    TimeFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes() ) );

    USHORT      nMask = ReadShortRes();

    if ( TIMEFIELD_FIRST & nMask )
    {
        maFirst = Time( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
    }
    if ( TIMEFIELD_LAST & nMask )
    {
        maLast = Time( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE *)GetClassRes() ) );
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
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsControlMod() )
    {
        if ( ImplTimeProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), mbDuration, meFormat, GetInternational() ) )
            return 1;
    }

    return SpinField::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long TimeField::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( FALSE );
    else if ( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( MustBeReformatted() && (GetText().Len() || !IsEmptyFieldValueEnabled()) )
            Reformat();
    }

    return SpinField::Notify( rNEvt );
}

// -----------------------------------------------------------------------

void TimeField::DataChanged( const DataChangedEvent& rDCEvt )
{
    SpinField::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_INTERNATIONAL) )
        ReformatAll();
}

// -----------------------------------------------------------------------

void TimeField::Modify()
{
    MarkToBeReformatted( TRUE );
    SpinField::Modify();
}

// -----------------------------------------------------------------------

void TimeField::Up()
{
    ImplTimeSpinArea( TRUE );
    SpinField::Up();
}

// -----------------------------------------------------------------------

void TimeField::Down()
{
    ImplTimeSpinArea( FALSE );
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
    Time aTime = GetTime();
    International aInt( Application::GetAppInternational() );

    switch ( eFormat )
    {
        case EXTTIMEF_24H_SHORT:
        {
            aInt.SetTimeFormat( HOUR_24 );
            SetDuration( FALSE );
            SetFormat( TIMEF_NONE );
        }
        break;
        case EXTTIMEF_24H_LONG:
        {
            aInt.SetTimeFormat( HOUR_24 );
            SetDuration( FALSE );
            SetFormat( TIMEF_SEC );
        }
        break;
        case EXTTIMEF_12H_SHORT:
        {
            aInt.SetTimeFormat( HOUR_12 );
            SetDuration( FALSE );
            SetFormat( TIMEF_NONE );
        }
        break;
        case EXTTIMEF_12H_LONG:
        {
            aInt.SetTimeFormat( HOUR_12 );
            SetDuration( FALSE );
            SetFormat( TIMEF_SEC );
        }
        break;
        case EXTTIMEF_DURATION_SHORT:
        {
            SetDuration( TRUE );
            SetFormat( TIMEF_NONE );
        }
        break;
        case EXTTIMEF_DURATION_LONG:
        {
            SetDuration( TRUE );
            SetFormat( TIMEF_SEC );
        }
        break;
        default:    DBG_ERROR( "ExtTimeFieldFormat unknown!" );
    }

    SetInternational( aInt );
    if ( GetField() && GetField()->GetText().Len() )
        SetUserTime( aTime );
}

// -----------------------------------------------------------------------

TimeBox::TimeBox( Window* pParent, WinBits nWinStyle ) :
    ComboBox( pParent, nWinStyle )
{
    SetField( this );
    SetText( GetInternational().GetTime( maFieldTime, FALSE, FALSE ) );
    Reformat();
}

// -----------------------------------------------------------------------

TimeBox::TimeBox( Window* pParent, const ResId& rResId ) :
    ComboBox( WINDOW_TIMEBOX )
{
    rResId.SetRT( RSC_TIMEBOX );
    WinBits nStyle = ImplInitRes( rResId );
    ComboBox::ImplInit( pParent, nStyle );
    SetField( this );
    SetText( GetInternational().GetTime( maFieldTime, FALSE, FALSE ) );
    ComboBox::ImplLoadRes( rResId );
    TimeFormatter::ImplLoadRes( ResId( (RSHEADER_TYPE *)GetClassRes() ) );
    Reformat();

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

TimeBox::~TimeBox()
{
}

// -----------------------------------------------------------------------

long TimeBox::PreNotify( NotifyEvent& rNEvt )
{
    if ( (rNEvt.GetType() == EVENT_KEYINPUT) &&
         !rNEvt.GetKeyEvent()->GetKeyCode().IsControlMod() )
    {
        if ( ImplTimeProcessKeyInput( GetField(), *rNEvt.GetKeyEvent(), IsStrictFormat(), mbDuration, meFormat, GetInternational() ) )
            return 1;
    }

    return ComboBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long TimeBox::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_GETFOCUS )
        MarkToBeReformatted( FALSE );
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

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_INTERNATIONAL) )
        ReformatAll();
}

// -----------------------------------------------------------------------

void TimeBox::Modify()
{
    MarkToBeReformatted( TRUE );
    ComboBox::Modify();
}

// -----------------------------------------------------------------------

void TimeBox::ReformatAll()
{
    XubString aStr;
    SetUpdateMode( FALSE );
    USHORT nEntryCount = GetEntryCount();
    for ( USHORT i=0; i < nEntryCount; i++ )
    {
        ImplTimeReformat( GetEntry( i ), aStr );
        RemoveEntry( i );
        InsertEntry( aStr, i );
    }
    TimeFormatter::Reformat();
    SetUpdateMode( TRUE );
}

// -----------------------------------------------------------------------

void TimeBox::InsertTime( const Time& rTime, USHORT nPos )
{
    Time aTime = rTime;
    if ( aTime > GetMax() )
        aTime = GetMax();
    else if ( aTime < GetMin() )
        aTime = GetMin();

    BOOL bSec    = FALSE;
    BOOL b100Sec = FALSE;
    if ( meFormat == TIMEF_SEC )
        bSec = TRUE;
    if ( meFormat == TIMEF_100TH_SEC || meFormat == TIMEF_SEC_CS )
        bSec = b100Sec = TRUE;
    ComboBox::InsertEntry( GetInternational().GetTime( aTime, bSec, b100Sec ), nPos );
}

// -----------------------------------------------------------------------

void TimeBox::RemoveTime( const Time& rTime )
{
    BOOL bSec    = FALSE;
    BOOL b100Sec = FALSE;
    if ( meFormat == TIMEF_SEC )
        bSec = TRUE;
    if ( meFormat == TIMEF_100TH_SEC || TIMEF_SEC_CS )
        bSec = b100Sec = TRUE;
    ComboBox::RemoveEntry( GetInternational().GetTime( rTime, bSec, b100Sec ) );
}

// -----------------------------------------------------------------------

Time TimeBox::GetTime( USHORT nPos ) const
{
    Time aTime( 0, 0, 0 );
    ImplTimeGetValue( ComboBox::GetEntry( nPos ), aTime,
                      meFormat, mbDuration, GetInternational() );
    return aTime;
}

// -----------------------------------------------------------------------

USHORT TimeBox::GetTimePos( const Time& rTime ) const
{
    BOOL bSec    = FALSE;
    BOOL b100Sec = FALSE;
    if ( meFormat == TIMEF_SEC )
        bSec = TRUE;
    if ( meFormat == TIMEF_100TH_SEC || TIMEF_SEC_CS )
        bSec = b100Sec = TRUE;
    return ComboBox::GetEntryPos( GetInternational().GetTime( rTime, bSec, b100Sec ) );
}
