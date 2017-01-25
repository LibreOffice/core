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
#include <rtl/character.hxx>


static SbxVariableRef Element
    ( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf,
      SbxClassType );

static const sal_Unicode* SkipWhitespace( const sal_Unicode* p )
{
    while( *p && ( *p == ' ' || *p == '\t' ) )
        p++;
    return p;
}

// Scanning of a symbol. The symbol were inserted in rSym, the return value
// is the new scan position. The symbol is at errors empty.

static const sal_Unicode* Symbol( const sal_Unicode* p, OUString& rSym )
{
    sal_uInt16 nLen = 0;
    // Did we have a nonstandard symbol?
    if( *p == '[' )
    {
        rSym = ++p;
        while( *p && *p != ']' )
        {
            p++;
            nLen++;
        }
        p++;
    }
    else
    {
        // A symbol had to begin with a alphabetic character or an underline
        if( !rtl::isAsciiAlpha( *p ) && *p != '_' )
        {
            SbxBase::SetError( ERRCODE_SBX_SYNTAX );
        }
        else
        {
            rSym = p;
            // The it can contain alphabetic characters, numbers or underlines
            while( *p && (rtl::isAsciiAlphanumeric( *p ) || *p == '_') )
            {
                p++;
                nLen++;
            }
            // Ignore standard BASIC suffixes
            if( *p && (*p == '%' || *p == '&' || *p == '!' || *p == '#' || *p == '$' ) )
            {
                p++;
            }
        }
    }
    rSym = rSym.copy( 0, nLen );
    return p;
}

// Qualified name. Element.Element....

static SbxVariableRef QualifiedName
    ( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf, SbxClassType t )
{

    SbxVariableRef refVar;
    const sal_Unicode* p = SkipWhitespace( *ppBuf );
    if( rtl::isAsciiAlpha( *p ) || *p == '_' || *p == '[' )
    {
        // Read in the element
        refVar = Element( pObj, pGbl, &p, t );
        while( refVar.is() && (*p == '.' || *p == '!') )
        {
            // It follows still an objectelement. The current element
            // had to be a SBX-Object or had to deliver such an object!
            pObj = dynamic_cast<SbxObject*>( refVar.get() );
            if( !pObj )
                // Then it had to deliver an object
                pObj = dynamic_cast<SbxObject*>( refVar->GetObject() );
            refVar.clear();
            if( !pObj )
                break;
            p++;
            // And the next element please
            refVar = Element( pObj, pGbl, &p, t );
        }
    }
    else
        SbxBase::SetError( ERRCODE_SBX_SYNTAX );
    *ppBuf = p;
    return refVar;
}

// Read in of an operand. This could be a number, a string or
// a function (with optional parameters).

static SbxVariableRef Operand
    ( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf, bool bVar )
{
    SbxVariableRef refVar( new SbxVariable );
    const sal_Unicode* p = SkipWhitespace( *ppBuf );
    if( !bVar && ( rtl::isAsciiDigit( *p )
                   || ( *p == '.' && rtl::isAsciiDigit( *( p+1 ) ) )
                   || *p == '-'
                   || *p == '&' ) )
    {
        // A number could be scanned in directly!
        sal_uInt16 nLen;
        if( !refVar->Scan( OUString( p ), &nLen ) )
        {
            refVar.clear();
        }
        else
        {
            p += nLen;
        }
    }
    else if( !bVar && *p == '"' )
    {
        // A string
        OUString aString;
        p++;
        for( ;; )
        {
            // This is perhaps an error
            if( !*p )
            {
                return nullptr;
            }
            // Double quotes are OK
            if( *p == '"' )
            {
                if( *++p != '"' )
                {
                    break;
                }
            }
            aString += OUStringLiteral1(*p++);
        }
        refVar->PutString( aString );
    }
    else
    {
        refVar = QualifiedName( pObj, pGbl, &p, SbxClassType::DontCare );
    }
    *ppBuf = p;
    return refVar;
}

// Read in of a simple term. The operands +, -, * and /
// are supported.

static SbxVariableRef MulDiv( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf )
{
    const sal_Unicode* p = *ppBuf;
    SbxVariableRef refVar( Operand( pObj, pGbl, &p, false ) );
    p = SkipWhitespace( p );
    while( refVar.is() && ( *p == '*' || *p == '/' ) )
    {
        sal_Unicode cOp = *p++;
        SbxVariableRef refVar2( Operand( pObj, pGbl, &p, false ) );
        if( refVar2.is() )
        {
            // temporary variable!
            SbxVariable* pVar = refVar.get();
            pVar = new SbxVariable( *pVar );
            refVar = pVar;
            if( cOp == '*' )
                *refVar *= *refVar2;
            else
                *refVar /= *refVar2;
        }
        else
        {
            refVar.clear();
            break;
        }
    }
    *ppBuf = p;
    return refVar;
}

static SbxVariableRef PlusMinus( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf )
{
    const sal_Unicode* p = *ppBuf;
    SbxVariableRef refVar( MulDiv( pObj, pGbl, &p ) );
    p = SkipWhitespace( p );
    while( refVar.is() && ( *p == '+' || *p == '-' ) )
    {
        sal_Unicode cOp = *p++;
        SbxVariableRef refVar2( MulDiv( pObj, pGbl, &p ) );
        if( refVar2.is() )
        {
            // temporaere Variable!
            SbxVariable* pVar = refVar.get();
            pVar = new SbxVariable( *pVar );
            refVar = pVar;
            if( cOp == '+' )
                *refVar += *refVar2;
            else
                *refVar -= *refVar2;
        }
        else
        {
            refVar.clear();
            break;
        }
    }
    *ppBuf = p;
    return refVar;
}

static SbxVariableRef Assign( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf )
{
    const sal_Unicode* p = *ppBuf;
    SbxVariableRef refVar( Operand( pObj, pGbl, &p, true ) );
    p = SkipWhitespace( p );
    if( refVar.is() )
    {
        if( *p == '=' )
        {
            // Assign only onto properties!
            if( refVar->GetClass() != SbxClassType::Property )
            {
                SbxBase::SetError( ERRCODE_SBX_BAD_ACTION );
                refVar.clear();
            }
            else
            {
                p++;
                SbxVariableRef refVar2( PlusMinus( pObj, pGbl, &p ) );
                if( refVar2.is() )
                {
                    SbxVariable* pVar = refVar.get();
                    SbxVariable* pVar2 = refVar2.get();
                    *pVar = *pVar2;
                    pVar->SetParameters( nullptr );
                }
            }
        }
        else
            // Simple call: once activating
            refVar->Broadcast( SfxHintId::BasicDataWanted );
    }
    *ppBuf = p;
    return refVar;
}

// Read in of an element. This is a symbol, optional followed
// by a parameter list. The symbol will be searched in the
// specified object and the parameter list will be attached if necessary.

static SbxVariableRef Element
    ( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf,
      SbxClassType t )
{
    OUString aSym;
    const sal_Unicode* p = Symbol( *ppBuf, aSym );
    SbxVariableRef refVar;
    if( !aSym.isEmpty() )
    {
        SbxFlagBits nOld = pObj->GetFlags();
        if( pObj == pGbl )
        {
            pObj->SetFlag( SbxFlagBits::GlobalSearch );
        }
        refVar = pObj->Find( aSym, t );
        pObj->SetFlags( nOld );
        if( refVar.is() )
        {
            refVar->SetParameters( nullptr );
            // Follow still parameter?
            p = SkipWhitespace( p );
            if( *p == '(' )
            {
                p++;
                auto refPar = tools::make_ref<SbxArray>();
                sal_uInt16 nArg = 0;
                // We are once relaxed and accept as well
                // the line- or commandend as delimiter
                // Search parameter always global!
                while( *p && *p != ')' && *p != ']' )
                {
                    SbxVariableRef refArg = PlusMinus( pGbl, pGbl, &p );
                    if( !refArg.is() )
                    {
                        // Error during the parsing
                        refVar.clear(); break;
                    }
                    else
                    {
                        // One copies the parameter, so that
                        // one have the current status (triggers also
                        // the call per access)
                        refPar->Put( new SbxVariable( *(refArg.get()) ), ++nArg );
                    }
                    p = SkipWhitespace( p );
                    if( *p == ',' )
                        p++;
                }
                if( *p == ')' )
                    p++;
                if( refVar.is() )
                    refVar->SetParameters( refPar.get() );
            }
        }
        else
            SbxBase::SetError( ERRCODE_SBX_NO_METHOD );
    }
    *ppBuf = p;
    return refVar;
}

// Mainroutine

SbxVariable* SbxObject::Execute( const OUString& rTxt )
{
    SbxVariableRef pVar = nullptr;
    const sal_Unicode* p = rTxt.getStr();
    for( ;; )
    {
        p = SkipWhitespace( p );
        if( !*p )
        {
            break;
        }
        if( *p++ != '[' )
        {
            SetError( ERRCODE_SBX_SYNTAX ); break;
        }
        pVar = Assign( this, this, &p );
        if( !pVar.is() )
        {
            break;
        }
        p = SkipWhitespace( p );
        if( *p++ != ']' )
        {
            SetError( ERRCODE_SBX_SYNTAX ); break;
        }
    }
    return pVar.get();
}

SbxVariable* SbxObject::FindQualified( const OUString& rName, SbxClassType t )
{
    SbxVariableRef pVar = nullptr;
    const sal_Unicode* p = rName.getStr();
    p = SkipWhitespace( p );
    if( !*p )
    {
        return nullptr;
    }
    pVar = QualifiedName( this, this, &p, t );
    p = SkipWhitespace( p );
    if( *p )
    {
        SetError( ERRCODE_SBX_SYNTAX );
    }
    return pVar.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
