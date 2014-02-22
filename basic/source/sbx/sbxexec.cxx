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

#include <tools/errcode.hxx>
#include <vcl/svapp.hxx>
#include <basic/sbx.hxx>
#include <rtl/character.hxx>


static SbxVariable* Element
    ( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf,
      SbxClassType );

static const sal_Unicode* SkipWhitespace( const sal_Unicode* p )
{
    while( *p && ( *p == ' ' || *p == '\t' ) )
        p++;
    return p;
}




static const sal_Unicode* Symbol( const sal_Unicode* p, OUString& rSym )
{
    sal_uInt16 nLen = 0;
    
    if( *p == '[' )
    {
        rSym = ++p;
        while( *p && *p != ']' )
        {
            p++, nLen++;
        }
        p++;
    }
    else
    {
        
        if( !rtl::isAsciiAlpha( *p ) && *p != '_' )
        {
            SbxBase::SetError( SbxERR_SYNTAX );
        }
        else
        {
            rSym = p;
            
            while( *p && (rtl::isAsciiAlphanumeric( *p ) || *p == '_') )
            {
                p++, nLen++;
            }
            
            if( *p && (*p == '%' || *p == '&' || *p == '!' || *p == '#' || *p == '$' ) )
            {
                p++;
            }
        }
    }
    rSym = rSym.copy( 0, nLen );
    return p;
}



static SbxVariable* QualifiedName
    ( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf, SbxClassType t )
{

    SbxVariableRef refVar;
    const sal_Unicode* p = SkipWhitespace( *ppBuf );
    if( rtl::isAsciiAlpha( *p ) || *p == '_' || *p == '[' )
    {
        
        refVar = Element( pObj, pGbl, &p, t );
        while( refVar.Is() && (*p == '.' || *p == '!') )
        {
            
            
            pObj = PTR_CAST(SbxObject,(SbxVariable*) refVar);
            if( !pObj )
                
                pObj = PTR_CAST(SbxObject,refVar->GetObject());
            refVar.Clear();
            if( !pObj )
                break;
            p++;
            
            refVar = Element( pObj, pGbl, &p, t );
        }
    }
    else
        SbxBase::SetError( SbxERR_SYNTAX );
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}




static SbxVariable* Operand
    ( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf, bool bVar )
{
    SbxVariableRef refVar( new SbxVariable );
    const sal_Unicode* p = SkipWhitespace( *ppBuf );
    if( !bVar && ( rtl::isAsciiDigit( *p )
                   || ( *p == '.' && rtl::isAsciiDigit( *( p+1 ) ) )
                   || *p == '-'
                   || *p == '&' ) )
    {
        
        sal_uInt16 nLen;
        if( !refVar->Scan( OUString( p ), &nLen ) )
        {
            refVar.Clear();
        }
        else
        {
            p += nLen;
        }
    }
    else if( !bVar && *p == '"' )
    {
        
        OUString aString;
        p++;
        for( ;; )
        {
            
            if( !*p )
            {
                return NULL;
            }
            
            if( *p == '"' )
            {
                if( *++p != '"' )
                {
                    break;
                }
            }
            aString += OUString(*p++);
        }
        refVar->PutString( aString );
    }
    else
    {
        refVar = QualifiedName( pObj, pGbl, &p, SbxCLASS_DONTCARE );
    }
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}




static SbxVariable* MulDiv( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf )
{
    const sal_Unicode* p = *ppBuf;
    SbxVariableRef refVar( Operand( pObj, pGbl, &p, false ) );
    p = SkipWhitespace( p );
    while( refVar.Is() && ( *p == '*' || *p == '/' ) )
    {
        sal_Unicode cOp = *p++;
        SbxVariableRef refVar2( Operand( pObj, pGbl, &p, false ) );
        if( refVar2.Is() )
        {
            
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

static SbxVariable* PlusMinus( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf )
{
    const sal_Unicode* p = *ppBuf;
    SbxVariableRef refVar( MulDiv( pObj, pGbl, &p ) );
    p = SkipWhitespace( p );
    while( refVar.Is() && ( *p == '+' || *p == '-' ) )
    {
        sal_Unicode cOp = *p++;
        SbxVariableRef refVar2( MulDiv( pObj, pGbl, &p ) );
        if( refVar2.Is() )
        {
            
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

static SbxVariable* Assign( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf )
{
    const sal_Unicode* p = *ppBuf;
    SbxVariableRef refVar( Operand( pObj, pGbl, &p, true ) );
    p = SkipWhitespace( p );
    if( refVar.Is() )
    {
        if( *p == '=' )
        {
            
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
            
            refVar->Broadcast( SBX_HINT_DATAWANTED );
    }
    *ppBuf = p;
    if( refVar.Is() )
        refVar->AddRef();
    return refVar;
}





static SbxVariable* Element
    ( SbxObject* pObj, SbxObject* pGbl, const sal_Unicode** ppBuf,
      SbxClassType t )
{
    OUString aSym;
    const sal_Unicode* p = Symbol( *ppBuf, aSym );
    SbxVariableRef refVar;
    if( !aSym.isEmpty() )
    {
        sal_uInt16 nOld = pObj->GetFlags();
        if( pObj == pGbl )
        {
            pObj->SetFlag( SBX_GBLSEARCH );
        }
        refVar = pObj->Find( aSym, t );
        pObj->SetFlags( nOld );
        if( refVar.Is() )
        {
            refVar->SetParameters( NULL );
            
            p = SkipWhitespace( p );
            if( *p == '(' )
            {
                p++;
                SbxArrayRef refPar = new SbxArray;
                sal_uInt16 nArg = 0;
                
                
                
                while( *p && *p != ')' && *p != ']' )
                {
                    SbxVariableRef refArg = PlusMinus( pGbl, pGbl, &p );
                    if( !refArg )
                    {
                        
                        refVar.Clear(); break;
                    }
                    else
                    {
                        
                        
                        
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



SbxVariable* SbxObject::Execute( const OUString& rTxt )
{
    SbxVariable* pVar = NULL;
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
            SetError( SbxERR_SYNTAX ); break;
        }
        pVar = Assign( this, this, &p );
        if( !pVar )
        {
            break;
        }
        p = SkipWhitespace( p );
        if( *p++ != ']' )
        {
            SetError( SbxERR_SYNTAX ); break;
        }
    }
    return pVar;
}

SbxVariable* SbxObject::FindQualified( const OUString& rName, SbxClassType t )
{
    SbxVariable* pVar = NULL;
    const sal_Unicode* p = rName.getStr();
    p = SkipWhitespace( p );
    if( !*p )
    {
        return NULL;;
    }
    pVar = QualifiedName( this, this, &p, t );
    p = SkipWhitespace( p );
    if( *p )
    {
        SetError( SbxERR_SYNTAX );
    }
    return pVar;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
