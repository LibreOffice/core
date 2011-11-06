/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_automation.hxx"
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
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
    sal_uInt16 nLenInChars;
    CmdBaseStream::Read( pStr, nLenInChars );

    *pString = String( pStr, nLenInChars );
    delete [] pStr;
}

void SCmdStream::Read (String &aString)
{
    comm_UniChar* pStr;
    sal_uInt16 nLenInChars;
    CmdBaseStream::Read( pStr, nLenInChars );

    aString = String( pStr, nLenInChars );
    delete [] pStr;
}

void SCmdStream::Read ( SfxPoolItem *&pItem )
{
    sal_uInt16 nType;
    sal_uInt16 nId;
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
    sal_uInt16 nType;
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

