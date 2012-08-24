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

#include <tools/errcode.hxx>
#include <vcl/svapp.hxx>
#include <basic/sbx.hxx>


class SbxSimpleCharClass
{
public:
    bool isAlpha( sal_Unicode c ) const
    {
        bool bRet = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        return bRet;
    }

    bool isDigit( sal_Unicode c ) const
    {
        bool bRet = (c >= '0' && c <= '9');
        return bRet;
    }

    bool isAlphaNumeric( sal_Unicode c ) const
    {
        bool bRet = isDigit( c ) || isAlpha( c );
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
    ( SbxObject* pObj, SbxObject* pGbl, const xub_Unicode** ppBuf, bool bVar )
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
        if( !refVar->Scan( rtl::OUString( p ), &nLen ) )
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
    SbxVariableRef refVar( Operand( pObj, pGbl, &p, false ) );
    p = SkipWhitespace( p );
    while( refVar.Is() && ( *p == '*' || *p == '/' ) )
    {
        xub_Unicode cOp = *p++;
        SbxVariableRef refVar2( Operand( pObj, pGbl, &p, false ) );
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
    SbxVariableRef refVar( Operand( pObj, pGbl, &p, true ) );
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
