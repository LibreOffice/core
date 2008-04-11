/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cmdstrm.cxx,v $
 * $Revision: 1.12 $
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
#include "precompiled_automation.hxx"
#ifndef _KEYCOD_HXX //autogen
#include <vcl/keycod.hxx>
#endif
#include <basic/sbx.hxx>
#include <tools/stream.hxx>

#include "cmdstrm.hxx"
#include "rcontrol.hxx"
#include "objtest.hxx"
#include "sttresid.hxx"
#include "svcommstream.hxx"
#include <basic/testtool.hrc>

ControlDefLoad __READONLY_DATA CmdStream::arKeyCodes [] =
#include <keycodes.hxx>
CNames *CmdStream::pKeyCodes = NULL;

ResMgr* SttResId::getSttResMgr()
{
    static ResMgr* pMgr = NULL;
    if( ! pMgr )
        pMgr = CREATEVERSIONRESMGR( stt );
    return pMgr;
}

CmdStream::CmdStream()
{
    pSammel = new SvMemoryStream();
    pCommStream = new SvCommStream( pSammel );
}

CmdStream::~CmdStream()
{
    delete pCommStream;
    delete pSammel;
}


#define PUT_CHAR(ch) \
        USHORT nMCode = nModify + ch;                       \
        USHORT _Code = 1;                                   \
        if ( (nMCode & 0xFF) == 0 )                         \
            _Code |= 2;                                     \
        if ( (nMCode >> 8) == 0 )                           \
            _Code |= 4;                                     \
        Result += (char) _Code;                             \
        Result += (char) ( nMCode & 0xFF );                 \
        Result += (char) ( nMCode >> 8 )


String CmdStream::WandleKeyEventString( String aKeys )
{
    if ( !pKeyCodes )
        ReadFlatArray(arKeyCodes,pKeyCodes);
    if ( !pKeyCodes )
        return aKeys;

    xub_StrLen nPos1 = 0;
    while ( (nPos1 = aKeys.Search('<',nPos1)) != STRING_NOTFOUND)
    {
        xub_StrLen nPos2 = aKeys.Search('>',nPos1);
        if ( nPos2 != STRING_NOTFOUND )
        {
            String Work = aKeys.Copy(nPos1+1,nPos2-nPos1+1-2);
            aKeys.Erase(nPos1,nPos2-nPos1+1);   // Inclusive Spitze Klammern weg
            String Result, Token;
            USHORT nModify = 0;
            while ( Work.Len() > 0 )
            {
                Token = Work.GetToken(0,' ');
                Work.Erase(0,Token.Len()+1);
                ControlDef WhatName(Token,SmartId());
                USHORT nElement;
                if (pKeyCodes->Seek_Entry(&WhatName,&nElement))
                {
                    USHORT nCode = (USHORT) pKeyCodes->GetObject(nElement)->pData->aUId.GetNum();
                    if ( nCode >= KEY_SHIFT )
                        nModify ^= nCode;
                    else
                    {
                        if ( ( nModify == 0 ) &&
                              (((nCode & 0xFF00) == KEYGROUP_NUM)   ||
                               ((nCode & 0xFF00) == KEYGROUP_ALPHA) ||
                               ( nCode == KEY_SPACE) ||
                               ( nCode == KEY_ADD) ||
                               ( nCode == KEY_SUBTRACT) ||
                               ( nCode == KEY_MULTIPLY) ||
                               ( nCode == KEY_DIVIDE) ||
                               ( nCode == KEY_POINT) ||
                               ( nCode == KEY_COMMA) ||
                               ( nCode == KEY_LESS) ||
                               ( nCode == KEY_GREATER) ||
                               ( nCode == KEY_EQUAL) ) )
                        {
                            switch ( nCode )
                            {
                                case KEY_SPACE:     Token = ' '; break;
                                case KEY_ADD:       Token = '+'; break;
                                case KEY_SUBTRACT:  Token = '-'; break;
                                case KEY_MULTIPLY:  Token = '*'; break;
                                case KEY_DIVIDE:    Token = '/'; break;
                                case KEY_POINT:     Token = '.'; break;
                                case KEY_COMMA:     Token = ','; break;
                                case KEY_LESS:      Token = '<'; break;
                                case KEY_GREATER:   Token = '>'; break;
                                case KEY_EQUAL:     Token = '='; break;
                                default:
//                                  if ( nModify == 0 )
//                                      Token.ToLower();
//                                  else
//                                      Token.ToUpper();
                                    ;
                            }
                            Result += Token;
                        }
                        else
                        {
                            PUT_CHAR (nCode);
                        }
                    }
                }
                else
                {
                    Result += '<';
                    Result += Token;
                    Result += ' ';
                    Result += String( SttResId( S_INVALID_KEYCODE ) );
                    Result += '>';
                }
            }
            aKeys.Insert(Result,nPos1);
            nPos1 = nPos1 + Result.Len();
        }
        else
            nPos1 = aKeys.Len() + 1;
    }

    return aKeys;
}


void CmdStream::WriteSortedParams( SbxArray* rPar, BOOL IsKeyString )
{
    USHORT nParams = PARAM_NONE;
    USHORT nNr1=0,nNr2=0,nNr3=0,nNr4=0;
    comm_ULONG nLNr1=0;
    String aString1,aString2;
    BOOL bBool1=FALSE,bBool2=FALSE;

    if ( rPar )
    {
        for ( USHORT i = 1; i < rPar->Count() ; i++)
        {
            switch (rPar->Get( i )->GetType())
            {
                case SbxLONG:       // alles immer als Short �bertragen
                case SbxULONG:
                case SbxLONG64:
                case SbxULONG64:
                case SbxDOUBLE:
                case SbxINTEGER:
                case SbxBYTE:
                case SbxUSHORT:
                case SbxINT:
                case SbxUINT:
                case SbxSINGLE:
                    if ( (nParams & PARAM_USHORT_1) == 0 )
                    {
                        nParams |= PARAM_USHORT_1;
                        nNr1 = rPar->Get( i )->GetUShort();
                    }
                    else if ( (nParams & PARAM_USHORT_2) == 0 )
                    {
                        nParams |= PARAM_USHORT_2;
                        nNr2 = rPar->Get( i )->GetUShort();
                    }
                    else if ( (nParams & PARAM_USHORT_3) == 0 )
                    {
                        nParams |= PARAM_USHORT_3;
                        nNr3 = rPar->Get( i )->GetUShort();
                    }
                    else if ( (nParams & PARAM_USHORT_4) == 0 )
                    {
                        nParams |= PARAM_USHORT_4;
                        nNr4 = rPar->Get( i )->GetUShort();
                    }
                    else
                        SbxBase::SetError( SbxERR_WRONG_ARGS );
                    break;
                case SbxOBJECT:     // whenever a control is passed. TabPage, MenuBar
                    {
                        SbxProperty *pMember = NULL;
                        if ( rPar->Get( i )->ISA( SbxObject ) )
                            pMember = ((SbxObject*)rPar->Get( i ))->GetDfltProperty();
                        if ( pMember != NULL )
                        {
                            if ( pMember->GetType() == SbxSTRING )
                            {
                                if ( nParams & PARAM_STR_1 )
                                    if ( nParams & PARAM_STR_2 )
                                        SbxBase::SetError( SbxERR_WRONG_ARGS );
                                    else
                                    {
                                        nParams |= PARAM_STR_2;
                                        aString2 = pMember->GetString();
                                    }
                                else
                                {
                                    nParams |= PARAM_STR_1;
                                    aString1 = pMember->GetString();
                                }
                                break;
                            }
                            else if ( pMember->GetType() == SbxULONG )
                            {
                                if ( nParams & PARAM_ULONG_1 )
                                    SbxBase::SetError( SbxERR_WRONG_ARGS );
                                else
                                {
                                    nParams |= PARAM_ULONG_1;
                                    nLNr1 = pMember->GetULong();
                                }
                            }
                            else
                                SbxBase::SetError( SbxERR_NAMED_NOT_FOUND );
                        }
                        else
                        {
                            SbxBase::SetError( SbxERR_NAMED_NOT_FOUND );
                        }
                    }
                    break;
                case SbxSTRING:
                case SbxCHAR:
                    if ( nParams & PARAM_STR_1 )
                        if ( nParams & PARAM_STR_2 )
                            SbxBase::SetError( SbxERR_WRONG_ARGS );
                        else
                        {
                            nParams |= PARAM_STR_2;
                            aString2 = rPar->Get( i )->GetString();
                        }
                    else
                    {
                        nParams |= PARAM_STR_1;
                        aString1 = rPar->Get( i )->GetString();
                    }
                    break;
                case SbxBOOL:
                    if ( nParams & PARAM_BOOL_1 )
                        if ( nParams & PARAM_BOOL_2 )
                            SbxBase::SetError( SbxERR_WRONG_ARGS );
                        else
                        {
                            nParams |= PARAM_BOOL_2;
                            bBool2 = rPar->Get( i )->GetBool();
                        }
                    else
                    {
                        nParams |= PARAM_BOOL_1;
                        bBool1 = rPar->Get( i )->GetBool();
                    }
                    break;
                default:
                    SbxBase::SetError( SbxERR_WRONG_ARGS );
                    break;
            }
        }
    }
    Write (nParams);
    if( nParams & PARAM_USHORT_1 )  Write( nNr1 );
    if( nParams & PARAM_USHORT_2 )  Write( nNr2 );
    if( nParams & PARAM_USHORT_3 )  Write( nNr3 );
    if( nParams & PARAM_USHORT_4 )  Write( nNr4 );
    if( nParams & PARAM_ULONG_1 )   Write( nLNr1 );
    if( nParams & PARAM_STR_1 )     Write( aString1, IsKeyString );
    if( nParams & PARAM_STR_2 )     Write( aString2, IsKeyString );
    if( nParams & PARAM_BOOL_1 )    Write( bBool1 );
    if( nParams & PARAM_BOOL_2 )    Write( bBool2 );
}

void CmdStream::GenCmdCommand( USHORT nNr, SbxArray* rPar )
{
    Write(USHORT(SICommand));
    Write(nNr);
    WriteSortedParams(rPar, (nNr & M_KEY_STRING) != 0 );
}

void CmdStream::GenCmdSlot( USHORT nNr, SbxArray* rPar )
{
    Write(USHORT(SISlot));
    Write(nNr);
    if (rPar)
    {
        USHORT nAnz = (rPar->Count()-1) >> 1;   // Geteilt durch 2
        Write(nAnz);
        BOOL bWriteUnoSlot = rPar->Get( 1 )->GetType() == SbxSTRING;

        for (USHORT n = 1 ; n <= nAnz ; n++)
        {
            /// #59513# nicht mehr ben�tigt
//          ULONG nUserData = rPar->Get( 2*n-1 )->GetUserData();
//          rPar->Get( 2*n-1 )->SetUserData(ID_DoNothing);  // Verhindert Ausf�hrung der Slots, die als Parameter �bergeben sind.

            if ( bWriteUnoSlot )
                Write(rPar->Get( 2*n-1 )->GetString());
            else
                Write(rPar->Get( 2*n-1 )->GetUShort());
            switch (rPar->Get( 2*n )->GetType())
            {
                case SbxINTEGER:
                case SbxBYTE:
                case SbxUSHORT:
                case SbxINT:
                case SbxUINT:
                case SbxSINGLE:
                    if ( !bWriteUnoSlot )
                        Write( (USHORT)BinUSHORT );
                    Write(rPar->Get( 2*n )->GetUShort());
                    break;
                case SbxLONG:
                case SbxULONG:
                case SbxLONG64:
                case SbxULONG64:
                case SbxDOUBLE:
                    if ( !bWriteUnoSlot )
                        Write( (USHORT)BinULONG );
                    Write(rPar->Get( 2*n )->GetULong());
                    break;
                case SbxSTRING:
                case SbxCHAR:
                    if ( !bWriteUnoSlot )
                        Write( (USHORT)BinString);
                    Write((String)rPar->Get( 2*n )->GetString());    // Cast f�r OS/2
                    break;
                case SbxBOOL:
                    if ( !bWriteUnoSlot )
                        Write( (USHORT)BinBool);
                    Write(rPar->Get( 2*n )->GetBool());
                    break;
                default:
                    SbxBase::SetError( SbxERR_WRONG_ARGS );
                    break;
            }

            /// #59513# nicht mehr ben�tigt ( siehe oben )
//          rPar->Get( 2*n-1 )->SetUserData(nUserData); // Und wieder zur�cksetzen, so da� auch alles sauber ist.
        }
    }
    else
        Write(USHORT(0));
}

void CmdStream::GenCmdUNOSlot( const String &aURL )
{
    Write( USHORT(SIUnoSlot) );
/*  Write( USHORT(0) );     // Hier wird im Office die SID_OPENURL Eingetragen.
                            // Dies mu� nat�rlich im Office hart verdratet werden und nicht hier,
                            // da sich die ID ja mal �ndern kann.

    // Da auch die ID f�r das PoolItem im Office entnommen werden mu� hier also kein PoolItem
    // gesendet werden.

    Write( USHORT(0) );     // Anzahl PoolItems

    // Stattdessen wird noch eine extra String gesendet, der dann Officeseitig in ein
    // SfxStringItem mit entsprechender ID gewandelt wird.
    Write( aURL );          // String f�r das PoolItem*/

    Write( aURL );          // Die UNO URL eben
}

void CmdStream::GenCmdControl( comm_ULONG nUId, USHORT nMethodId, SbxArray* rPar )
{
    Write(USHORT(SIControl));
    Write(nUId);
    Write(nMethodId);
    WriteSortedParams(rPar, (nMethodId & M_KEY_STRING) != 0 );
}

void CmdStream::GenCmdControl( String aUId, USHORT nMethodId, SbxArray* rPar )
{
    Write(USHORT(SIStringControl));
    Write(aUId);
    Write(nMethodId);
    WriteSortedParams(rPar, (nMethodId & M_KEY_STRING) != 0 );
}

void CmdStream::GenCmdFlow( USHORT nArt )
{
    Write(USHORT(SIFlow));
    Write(nArt);
    Write(USHORT(PARAM_NONE));              // Typ der folgenden Parameter
}

void CmdStream::GenCmdFlow( USHORT nArt, USHORT nNr1 )
{
    Write(USHORT(SIFlow));
    Write(nArt);
    Write(USHORT(PARAM_USHORT_1));          // Typ der folgenden Parameter
    Write(nNr1);
}

void CmdStream::GenCmdFlow( USHORT nArt, comm_ULONG nNr1 )
{
    Write(USHORT(SIFlow));
    Write(nArt);
    Write(USHORT(PARAM_ULONG_1));           // Typ der folgenden Parameter
    Write(nNr1);
}

void CmdStream::GenCmdFlow( USHORT nArt, String aString1 )
{
    Write(USHORT(SIFlow));
    Write(nArt);
    Write(USHORT(PARAM_STR_1));             // Typ der folgenden Parameter
    Write(aString1);
}

void CmdStream::Write( String aString, BOOL IsKeyString )
{
    if ( IsKeyString )
        Write( WandleKeyEventString( aString ), FALSE );
    else
        Write( aString.GetBuffer(), aString.Len() );
}

SvMemoryStream* CmdStream::GetStream()
{
    return pSammel;
}

void CmdStream::Reset( comm_ULONG nSequence )
{
    delete pCommStream;
    delete pSammel;
    pSammel = new SvMemoryStream();
    pCommStream = new SvCommStream( pSammel );
    GenCmdFlow (F_Sequence,nSequence);
}

