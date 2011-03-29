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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basic.hxx"
#include <tools/errcode.hxx>
#include <vcl/svapp.hxx>
#include <basic/sbx.hxx>


class SbxSimpleCharClass
{
public:
    sal_Bool isAlpha( sal_Unicode c ) const
    {
        sal_Bool bRet = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        return bRet;
    }

    sal_Bool isDigit( sal_Unicode c ) const
    {
        sal_Bool bRet = (c >= '0' && c <= '9');
        return bRet;
    }

    sal_Bool isAlphaNumeric( sal_Unicode c ) const
    {
        sal_Bool bRet = isDigit( c ) || isAlpha( c );
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

// Scanning of a symbol. The symbol were inserted in rSym, the return value
// is the new scan position. The symbol is at errors empty.

static const xub_Unicode* Symbol( const xub_Unicode* p, XubString& rSym, const SbxSimpleCharClass& rCharClass )
{
    sal_uInt16 nLen = 0;
    // Did we have a nonstandard symbol?
    if( *p == '[' )
    {
        rSym = ++p;
        while( *p && *p != ']' )
            p++, nLen++;
        p++;
    }
    else
    {
        // A symbol had to begin with a alphabetic character or an underline
        if( !rCharClass.isAlpha( *p ) && *p != '_' )
            SbxBase::SetError( SbxERR_SYNTAX );
        else
        {
            rSym = p;
            // The it can contain alphabetic characters, numbers or underlines
            while( *p && (rCharClass.isAlphaNumeric( *p ) || *p == '_') )
                p++, nLen++;
            // BASIC-Standard-Suffixes were ignored
            if( *p && (*p == '%' || *p == '&' || *p == '!' || *p == '#' || *p == '$' ) )
                p++;
        }
    }
    rSym.Erase( nLen );
    return p;
}

// Qualified name. Element.Element....

static SbxVariable* QualifiedName
    ( SbxObject* pObj, SbxObject* pGbl, const xub_Unicode** ppBuf, SbxClassType t )
{
    static SbxSimpleCharClass aCharClass;

    SbxVariableRef refVar;
    const xub_Unicode* p = SkipWhitespace( *ppBuf );
    if( aCharClass.isAlpha( *p ) || *p == '_' || *p == '[' )
    {
        // Read in the element
        refVar = Element( pObj, pGbl, &p, t, aCharClass );
        while( refVar.Is() && (*p == '.' || *p == '!') )
        {
            // It follows still an objectelement. The current element
            // had to be a SBX-Object or had to deliver such an object!
            pObj = PTR_CAST(SbxObject,(SbxVariable*) refVar);
            if( !pObj )
                // Then it had to deliver an object
                pObj = PTR_CAST(SbxObject,refVar->GetObject());
            refVar.Clear();
            if( !pObj )
                break;
            p++;
            // And the next element please
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

// Read in of an operand. This could be a number, a string or
// a function (with optional parameters).

static SbxVariable* Operand
    ( SbxObject* pObj, SbxObject* pGbl, const xub_Unicode** ppBuf, sal_Bool bVar )
{
    static SbxSimpleCharClass aCharClass;

    SbxVariableRef refVar( new SbxVariable );
    const xub_Unicode* p = SkipWhitespace( *ppBuf );
    if( !bVar && ( aCharClass.isDigit( *p )
     || ( *p == '.' && aCharClass.isDigit( *( p+1 ) ) )
     || *p == '-'
     || *p == '&' ) )
    {
        // A number could be scanned in directly!
        sal_uInt16 nLen;
        if( !refVar->Scan( XubString( p ), &nLen ) )
            refVar.Clear();
        else
            p += nLen;
    }
    else if( !bVar && *p == '"' )
    {
        // A string
        XubString aString;
        p++;
        for( ;; )
        {
            // This is perhaps an error
            if( !*p )
                return NULL;
            // Double quotes are OK
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

// Read in of a simple term. The operands +, -, * and /
// are supported.

static SbxVariable* MulDiv( SbxObject* pObj, SbxObject* pGbl, const xub_Unicode** ppBuf )
{
    const xub_Unicode* p = *ppBuf;
    SbxVariableRef refVar( Operand( pObj, pGbl, &p, sal_False ) );
    p = SkipWhitespace( p );
    while( refVar.Is() && ( *p == '*' || *p == '/' ) )
    {
        xub_Unicode cOp = *p++;
        SbxVariableRef refVar2( Operand( pObj, pGbl, &p, sal_False ) );
        if( refVar2.Is() )
        {
            // temporary variable!
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

static SbxVariable* Assign( SbxObject* pObj, SbxObject* pGbl, const xub_Unicode** ppBuf )
{
    const xub_Unicode* p = *ppBuf;
    SbxVariableRef refVar( Operand( pObj, pGbl, &p, sal_True ) );
    p = SkipWhitespace( p );
    if( refVar.Is() )
    {
        if( *p == '=' )
        {
            // Assign only onto properties!
            if( refVar->GetClass() != SbxCLASS_PROPERTY )
            {
                SbxBase::SetError( SbxERR_BAD_ACTION );
                refVar.Clear();
            }
            else
            {
                p++;
                SbxVariableRef refVar2( PlusMinus( pObj, pGbl, &p ) );
                if( refVar2.Is() )
                {
                    SbxVariable* pVar = refVar;
                    SbxVariable* pVar2 = refVar2;
                    *pVar = *pVar2;
                    pVar->SetParameters( NULL );
                }
            }
        }
        else
            // Simple call: once activating
            refVar->Broadcast( SBX_HINT_DATAWANTED );
    }
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}

// Read in of an element. This is a symbol, optional followed
// by a parameter list. The symbol will be searched in the
// specified object and the parameter list will be attached if necessary.

static SbxVariable* Element
    ( SbxObject* pObj, SbxObject* pGbl, const xub_Unicode** ppBuf,
      SbxClassType t, const SbxSimpleCharClass& rCharClass )
{
    XubString aSym;
    const xub_Unicode* p = Symbol( *ppBuf, aSym, rCharClass );
    SbxVariableRef refVar;
    if( aSym.Len() )
    {
        sal_uInt16 nOld = pObj->GetFlags();
        if( pObj == pGbl )
            pObj->SetFlag( SBX_GBLSEARCH );
        refVar = pObj->Find( aSym, t );
        pObj->SetFlags( nOld );
        if( refVar.Is() )
        {
            refVar->SetParameters( NULL );
            // Follow still parameter?
            p = SkipWhitespace( p );
            if( *p == '(' )
            {
                p++;
                SbxArrayRef refPar = new SbxArray;
                sal_uInt16 nArg = 0;
                // We are once relaxed and accept as well
                // the line- or commandend as delimiter
                // Search parameter always global!
                while( *p && *p != ')' && *p != ']' )
                {
                    SbxVariableRef refArg = PlusMinus( pGbl, pGbl, &p );
                    if( !refArg )
                    {
                        // Error during the parsing
                        refVar.Clear(); break;
                    }
                    else
                    {
                        // One copies the parameter, so that
                        // one have the current status (triggers also
                        // the call per access)
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

// Mainroutine

SbxVariable* SbxObject::Execute( const XubString& rTxt )
{
    SbxVariable* pVar = NULL;
    const xub_Unicode* p = rTxt.GetBuffer();
    for( ;; )
    {
        p = SkipWhitespace( p );
        if( !*p )
            break;
        if( *p++ != '[' )
        {
            SetError( SbxERR_SYNTAX ); break;
        }
        pVar = Assign( this, this, &p );
        if( !pVar )
            break;
        p = SkipWhitespace( p );
        if( *p++ != ']' )
        {
            SetError( SbxERR_SYNTAX ); break;
        }
    }
    return pVar;
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
