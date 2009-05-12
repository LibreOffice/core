/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scmdstrm.cxx,v $
 * $Revision: 1.7 $
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
#include <svtools/intitem.hxx>
#include <svtools/stritem.hxx>
#include <svtools/eitem.hxx>
#include "scmdstrm.hxx"
#include "svcommstream.hxx"
#include "rcontrol.hxx"

#if OSL_DEBUG_LEVEL > 1
#include "editwin.hxx"
#include "statemnt.hxx"
#endif

SCmdStream::SCmdStream(SvStream *pIn)
{
    pSammel = pIn;
    pCommStream = new SvCommStream( pSammel );
//  SetCommStream( pCommStream );
}

SCmdStream::~SCmdStream()
{
    delete pCommStream;
}

void SCmdStream::Read (String* &pString)
{
    if ( !pString )
        pString = new String();
    comm_UniChar* pStr;
    USHORT nLenInChars;
    CmdBaseStream::Read( pStr, nLenInChars );

    *pString = String( pStr, nLenInChars );
    delete [] pStr;
}

void SCmdStream::Read (String &aString)
{
    comm_UniChar* pStr;
    USHORT nLenInChars;
    CmdBaseStream::Read( pStr, nLenInChars );

    aString = String( pStr, nLenInChars );
    delete [] pStr;
}

void SCmdStream::Read ( SfxPoolItem *&pItem )
{
    USHORT nType;
    USHORT nId;
    Read(nId);
#if OSL_DEBUG_LEVEL > 1
        StatementList::m_pDbgWin->AddText( "Parameter: " );
        StatementList::m_pDbgWin->AddText( String::CreateFromInt32( nId ) );
        StatementList::m_pDbgWin->AddText( " " );
#endif
    Read( nType );
    switch (nType)
    {
        case BinUSHORT:
            {
                comm_USHORT nNr;
                Read (nNr );
                pItem = new SfxUInt16Item(nId,nNr);
#if OSL_DEBUG_LEVEL > 1
                StatementList::m_pDbgWin->AddText( "USHORT:" );
                StatementList::m_pDbgWin->AddText( String::CreateFromInt32( nNr ) );
#endif
            }
            break;
        case BinULONG:
            {
                comm_ULONG nNr;
                Read (nNr );
                pItem = new SfxUInt32Item(nId,nNr);
#if OSL_DEBUG_LEVEL > 1
                StatementList::m_pDbgWin->AddText( "ULONG:" );
                StatementList::m_pDbgWin->AddText( String::CreateFromInt64( nNr ) );
#endif
            }
            break;
        case BinString:
            {
                String aString;
                Read (aString);

                pItem = new SfxStringItem(nId,aString);
#if OSL_DEBUG_LEVEL > 1
                StatementList::m_pDbgWin->AddText( "String:" );
                StatementList::m_pDbgWin->AddText( aString );
#endif
            }
            break;
        case BinBool:
            {
                comm_BOOL bBool;
                Read (bBool);
                pItem = new SfxBoolItem(nId,bBool);
#if OSL_DEBUG_LEVEL > 1
                StatementList::m_pDbgWin->AddText( "BOOL:" );
                StatementList::m_pDbgWin->AddText( bBool ? "TRUE" : "FALSE" );
#endif
            }
            break;
        default:
            DBG_ERROR1( "Ungültiger Typ im Stream:%hu", nType );
#if OSL_DEBUG_LEVEL > 1
            StatementList::m_pDbgWin->AddText( "Ungültiger Typ !!!! " );
#endif
            break;
    }
#if OSL_DEBUG_LEVEL > 1
        StatementList::m_pDbgWin->AddText( "\n" );
#endif
}

void SCmdStream::Read ( ::com::sun::star::beans::PropertyValue &rItem )
{
    USHORT nType;
    String aId;
    Read(aId);
    rItem.Name = rtl::OUString( aId );
#if OSL_DEBUG_LEVEL > 1
        StatementList::m_pDbgWin->AddText( "Parameter: " );
        StatementList::m_pDbgWin->AddText( aId );
        StatementList::m_pDbgWin->AddText( " " );
#endif
    nType = GetNextType();
    switch (nType)
    {
        case BinUSHORT:
            {
                comm_USHORT nNr;
                Read (nNr );
                rItem.Value <<= nNr;
#if OSL_DEBUG_LEVEL > 1
                StatementList::m_pDbgWin->AddText( "USHORT:" );
                StatementList::m_pDbgWin->AddText( String::CreateFromInt32( nNr ) );
#endif
            }
            break;
        case BinULONG:
            {
                comm_ULONG nNr;
                Read (nNr );
                rItem.Value <<= nNr;
#if OSL_DEBUG_LEVEL > 1
                StatementList::m_pDbgWin->AddText( "ULONG:" );
                StatementList::m_pDbgWin->AddText( String::CreateFromInt64( nNr ) );
#endif
            }
            break;
        case BinString:
            {
                String aString;
                Read (aString);
                rItem.Value <<= ::rtl::OUString( aString );
#if OSL_DEBUG_LEVEL > 1
                StatementList::m_pDbgWin->AddText( "String:" );
                StatementList::m_pDbgWin->AddText( aString );
#endif
            }
            break;
        case BinBool:
            {
                comm_BOOL bBool;
                Read (bBool);
                rItem.Value <<= bBool;
#if OSL_DEBUG_LEVEL > 1
                StatementList::m_pDbgWin->AddText( "BOOL:" );
                StatementList::m_pDbgWin->AddText( bBool ? "TRUE" : "FALSE" );
#endif
            }
            break;
        default:
            DBG_ERROR1( "Ungültiger Typ im Stream:%hu", nType );
#if OSL_DEBUG_LEVEL > 1
            StatementList::m_pDbgWin->AddText( "Ungültiger Typ !!!! " );
#endif
            break;
    }
#if OSL_DEBUG_LEVEL > 1
        StatementList::m_pDbgWin->AddText( "\n" );
#endif
}

