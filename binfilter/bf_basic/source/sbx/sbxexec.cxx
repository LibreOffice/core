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

#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#include "sbx.hxx"

namespace binfilter {

class SbxSimpleCharClass
{
public:
    BOOL isAlpha( sal_Unicode c ) const
    {
        BOOL bRet = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        return bRet;
    }

    BOOL isDigit( sal_Unicode c ) const
    {
        BOOL bRet = (c >= '0' && c <= '9');
        return bRet;
    }

    BOOL isAlphaNumeric( sal_Unicode c ) const
    {
        BOOL bRet = isDigit( c ) || isAlpha( c );
        return bRet;
    }
};


static SbxVariable* Element
    ( SbxObject* pObj, SbxObject* pGbl, const xub_Unicode** ppBuf,
      SbxClassType, const SbxSimpleCharClass& rCharClass );

static const xub_Unicode* SkipWhitespace( const xub_Unicode* p )
{
    while( *p && ( *p == ' ' || *p == '\t' ) )
        p++;
    return p;
}

// Scannen eines Symbol. Das Symbol wird in rSym eingetragen, der Returnwert
// ist die neue Scanposition. Bei Fehlern ist das Symbol leer.

static const xub_Unicode* Symbol( const xub_Unicode* p, XubString& rSym, const SbxSimpleCharClass& rCharClass )
{
    USHORT nLen = 0;
    // Haben wir ein Sondersymbol?
    if( *p == '[' )
    {
        rSym = ++p;
        while( *p && *p != ']' )
            p++, nLen++;
        p++;
    }
    else
    {
        // Ein Symbol muss mit einem Buchstaben oder einem Underline beginnen
        if( !rCharClass.isAlpha( *p ) && *p != '_' )
            SbxBase::SetError( SbxERR_SYNTAX );
        else
        {
            rSym = p;
            // Dann darf es Buchstaben, Zahlen oder Underlines enthalten
            while( *p && (rCharClass.isAlphaNumeric( *p ) || *p == '_') )
                p++, nLen++;
            // BASIC-Standard-Suffixe werden ignoriert
            if( *p && (*p == '%' || *p == '&' || *p == '!' || *p == '#' || *p == '$' ) )
                p++;
        }
    }
    rSym.Erase( nLen );
    return p;
}

// Qualifizierter Name. Element.Element....

static SbxVariable* QualifiedName
    ( SbxObject* pObj, SbxObject* pGbl, const xub_Unicode** ppBuf, SbxClassType t )
{
    static SbxSimpleCharClass aCharClass;

    SbxVariableRef refVar;
    const xub_Unicode* p = SkipWhitespace( *ppBuf );
    if( aCharClass.isAlpha( *p ) || *p == '_' || *p == '[' )
    {
        // Element einlesen
        refVar = Element( pObj, pGbl, &p, t, aCharClass );
        while( refVar.Is() && (*p == '.' || *p == '!') )
        {
            // Es folgt noch ein Objektelement. Das aktuelle Element
            // muss also ein SBX-Objekt sein oder liefern!
            pObj = PTR_CAST(SbxObject,(SbxVariable*) refVar);
            if( !pObj )
                // Dann muss es ein Objekt liefern
                pObj = PTR_CAST(SbxObject,refVar->GetObject());
            refVar.Clear();
            if( !pObj )
                break;
            p++;
            // Und das naechste Element bitte
            refVar = Element( pObj, pGbl, &p, t, aCharClass );
        }
    }
    else
        SbxBase::SetError( SbxERR_SYNTAX );
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}

// Einlesen eines Operanden. Dies kann eine Zahl, ein String oder
// eine Funktion (mit optionalen Parametern) sein.

static SbxVariable* Operand
    ( SbxObject* pObj, SbxObject* pGbl, const xub_Unicode** ppBuf, BOOL bVar )
{
    static SbxSimpleCharClass aCharClass;

    SbxVariableRef refVar( new SbxVariable );
    const xub_Unicode* p = SkipWhitespace( *ppBuf );
    if( !bVar && ( aCharClass.isDigit( *p )
     || ( *p == '.' && aCharClass.isDigit( *( p+1 ) ) )
     || *p == '-'
     || *p == '&' ) )
    {
        // Eine Zahl kann direkt eingescant werden!
        USHORT nLen;
        if( !refVar->Scan( XubString( p ), &nLen ) )
            refVar.Clear();
        else
            p += nLen;
    }
    else if( !bVar && *p == '"' )
    {
        // Ein String
        XubString aString;
        p++;
        for( ;; )
        {
            // Das ist wohl ein Fehler
            if( !*p )
                return NULL;
            // Doppelte Quotes sind OK
            if( *p == '"' )
                if( *++p != '"' )
                    break;
            aString += *p++;
        }
        refVar->PutString( aString );
    }
    else
        refVar = QualifiedName( pObj, pGbl, &p, SbxCLASS_DONTCARE );
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}

// Einlesen einer einfachen Term. Die Operatoren +, -, * und /
// werden unterstuetzt.

static SbxVariable* MulDiv( SbxObject* pObj, SbxObject* pGbl, const xub_Unicode** ppBuf )
{
    const xub_Unicode* p = *ppBuf;
    SbxVariableRef refVar( Operand( pObj, pGbl, &p, FALSE ) );
    p = SkipWhitespace( p );
    while( refVar.Is() && ( *p == '*' || *p == '/' ) )
    {
        xub_Unicode cOp = *p++;
        SbxVariableRef refVar2( Operand( pObj, pGbl, &p, FALSE ) );
        if( refVar2.Is() )
        {
            // temporaere Variable!
            SbxVariable* pVar = refVar;
            pVar = new SbxVariable( *pVar );
            refVar = pVar;
            if( cOp == '*' )
                *refVar *= *refVar2;
            else
                *refVar /= *refVar2;
        }
        else
        {
            refVar.Clear();
            break;
        }
    }
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}

static SbxVariable* PlusMinus( SbxObject* pObj, SbxObject* pGbl, const xub_Unicode** ppBuf )
{
    const xub_Unicode* p = *ppBuf;
    SbxVariableRef refVar( MulDiv( pObj, pGbl, &p ) );
    p = SkipWhitespace( p );
    while( refVar.Is() && ( *p == '+' || *p == '-' ) )
    {
        xub_Unicode cOp = *p++;
        SbxVariableRef refVar2( MulDiv( pObj, pGbl, &p ) );
        if( refVar2.Is() )
        {
            // temporaere Variable!
            SbxVariable* pVar = refVar;
            pVar = new SbxVariable( *pVar );
            refVar = pVar;
            if( cOp == '+' )
                *refVar += *refVar2;
            else
                *refVar -= *refVar2;
        }
        else
        {
            refVar.Clear();
            break;
        }
    }
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}

/*?*/ // static SbxVariable* Assign( SbxObject* pObj, SbxObject* pGbl, const xub_Unicode** ppBuf )
/*?*/ // {
/*?*/ // 	const xub_Unicode* p = *ppBuf;
/*?*/ // 	SbxVariableRef refVar( Operand( pObj, pGbl, &p, TRUE ) );
/*?*/ // 	p = SkipWhitespace( p );
/*?*/ // 	if( refVar.Is() )
/*?*/ // 	{
/*?*/ // 		if( *p == '=' )
/*?*/ // 		{
/*?*/ // 			// Nur auf Props zuweisen!
/*?*/ // 			if( refVar->GetClass() != SbxCLASS_PROPERTY )
/*?*/ // 			{
/*?*/ // 				SbxBase::SetError( SbxERR_BAD_ACTION );
/*?*/ // 				refVar.Clear();
/*?*/ // 			}
/*?*/ // 			else
/*?*/ // 			{
/*?*/ // 				p++;
/*?*/ // 				SbxVariableRef refVar2( PlusMinus( pObj, pGbl, &p ) );
/*?*/ // 				if( refVar2.Is() )
/*?*/ // 				{
/*?*/ // 					SbxVariable* pVar = refVar;
/*?*/ // 					SbxVariable* pVar2 = refVar2;
/*?*/ // 					*pVar = *pVar2;
/*?*/ // 					pVar->SetParameters( NULL );
/*?*/ // 				}
/*?*/ // 			}
/*?*/ // 		}
/*?*/ // 		else
/*?*/ // 			// Einfacher Aufruf: einmal aktivieren
/*?*/ // 			refVar->Broadcast( SBX_HINT_DATAWANTED );
/*?*/ // 	}
/*?*/ // 	*ppBuf = p;
/*?*/ // 	if( refVar.Is() )
/*?*/ // 		refVar->AddRef();
/*?*/ // 	return refVar;
/*?*/ // }
/*?*/ // 
/*?*/ // // Einlesen eines Elements. Dies ist ein Symbol, optional gefolgt
/*?*/ // // von einer Parameterliste. Das Symbol wird im angegebenen Objekt
/*?*/ // // gesucht und die Parameterliste wird ggf. angefuegt.

static SbxVariable* Element
    ( SbxObject* pObj, SbxObject* pGbl, const xub_Unicode** ppBuf,
      SbxClassType t, const SbxSimpleCharClass& rCharClass )
{
    XubString aSym;
    const xub_Unicode* p = Symbol( *ppBuf, aSym, rCharClass );
    SbxVariableRef refVar;
    if( aSym.Len() )
    {
        USHORT nOld = pObj->GetFlags();
        if( pObj == pGbl )
            pObj->SetFlag( SBX_GBLSEARCH );
        refVar = pObj->Find( aSym, t );
        pObj->SetFlags( nOld );
        if( refVar.Is() )
        {
            refVar->SetParameters( NULL );
            // folgen noch Parameter?
            p = SkipWhitespace( p );
            if( *p == '(' )
            {
                p++;
                SbxArrayRef refPar = new SbxArray;
                USHORT nArg = 0;
                // Wird sind mal relaxed und akzeptieren auch
                // das Zeilen- oder Komandoende als Begrenzer
                // Parameter immer global suchen!
                while( *p && *p != ')' && *p != ']' )
                {
                    SbxVariableRef refArg = PlusMinus( pGbl, pGbl, &p );
                    if( !refArg )
                    {
                        // Fehler beim Parsing
                        refVar.Clear(); break;
                    }
                    else
                    {
                        // Man kopiere den Parameter, damit
                        // man den aktuellen Zustand hat (loest auch
                        // den Aufruf per Zugriff aus)
                        SbxVariable* pArg = refArg;
                        refPar->Put( new SbxVariable( *pArg ), ++nArg );
                    }
                    p = SkipWhitespace( p );
                    if( *p == ',' )
                        p++;
                }
                if( *p == ')' )
                    p++;
                if( refVar.Is() )
                    refVar->SetParameters( refPar );
            }
        }
        else
            SbxBase::SetError( SbxERR_NO_METHOD );
    }
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}

/*?*/ // // Hauptroutine
/*?*/ // 
/*?*/ // SbxVariable* SbxObject::Execute( const XubString& rTxt )
/*?*/ // {
/*?*/ // 	SbxVariable* pVar = NULL;
/*?*/ // 	const xub_Unicode* p = rTxt.GetBuffer();
/*?*/ // 	for( ;; )
/*?*/ // 	{
/*?*/ // 		p = SkipWhitespace( p );
/*?*/ // 		if( !*p )
/*?*/ // 			break;
/*?*/ // 		if( *p++ != '[' )
/*?*/ // 		{
/*?*/ // 			SetError( SbxERR_SYNTAX ); break;
/*?*/ // 		}
/*?*/ // 		pVar = Assign( this, this, &p );
/*?*/ // 		if( !pVar )
/*?*/ // 			break;
/*?*/ // 		p = SkipWhitespace( p );
/*?*/ // 		if( *p++ != ']' )
/*?*/ // 		{
/*?*/ // 			SetError( SbxERR_SYNTAX ); break;
/*?*/ // 		}
/*?*/ // 	}
/*?*/ // 	return pVar;
/*?*/ // }
/*?*/ // 
SbxVariable* SbxObject::FindQualified( const XubString& rName, SbxClassType t )
{
    SbxVariable* pVar = NULL;
    const xub_Unicode* p = rName.GetBuffer();
    p = SkipWhitespace( p );
    if( !*p )
        return NULL;;
    pVar = QualifiedName( this, this, &p, t );
    p = SkipWhitespace( p );
    if( *p )
        SetError( SbxERR_SYNTAX );
    return pVar;
}

}
