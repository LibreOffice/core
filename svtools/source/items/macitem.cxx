/*************************************************************************
 *
 *  $RCSfile: macitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:01 $
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

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#pragma hdrstop
#define ITEMID_MACRO 0

#include "macitem.hxx"
#include <segmentc.hxx>

// STATIC DATA -----------------------------------------------------------

DBG_NAME(SvxMacroItem);

SEG_EOFGLOBALS()

// -----------------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SvxMacroItem, SfxPoolItem);

// -----------------------------------------------------------------------


SjJSbxObjectBase::~SjJSbxObjectBase()
{
}

SjJSbxObjectBase* SjJSbxObjectBase::Clone( void )
{
    return NULL;
}

SvxMacro::SvxMacro( const String &rMacName, const String &rLanguage)
    : aMacName( rMacName ), aLibName( rLanguage),
      pFunctionObject(NULL), eType( EXTENDED_STYPE)
{
    if (rLanguage.EqualsAscii(SVX_MACRO_LANGUAGE_STARBASIC))
        eType=STARBASIC;
    else if (rLanguage.EqualsAscii(SVX_MACRO_LANGUAGE_JAVASCRIPT))
        eType=JAVASCRIPT;
}


SvxMacro::~SvxMacro()
{
    delete pFunctionObject;
}

String SvxMacro::GetLanguage()const
{
    if(eType==STARBASIC)
    {
        return UniString::CreateFromAscii(
                   RTL_CONSTASCII_STRINGPARAM(SVX_MACRO_LANGUAGE_STARBASIC));
    }
    else if(eType==JAVASCRIPT)
    {
        return UniString::CreateFromAscii(
                   RTL_CONSTASCII_STRINGPARAM(SVX_MACRO_LANGUAGE_JAVASCRIPT));
    }

    return aLibName;
}



SvxMacro& SvxMacro::operator=( const SvxMacro& rBase )
{
    if( this != &rBase )
    {
        aMacName = rBase.aMacName;
        aLibName = rBase.aLibName;
        delete pFunctionObject;
        pFunctionObject = rBase.pFunctionObject ? rBase.pFunctionObject->Clone() : NULL;
        eType = rBase.eType;
    }
    return *this;
}


#pragma SEG_FUNCDEF(macitem_02)

SvxMacroTableDtor& SvxMacroTableDtor::operator=( const SvxMacroTableDtor& rTbl )
{
    DelDtor();
    SvxMacro* pTmp = ((SvxMacroTableDtor&)rTbl).First();
    while( pTmp )
    {
        SvxMacro *pNew = new SvxMacro( *pTmp );
        Insert( rTbl.GetCurKey(), pNew );
        pTmp = ((SvxMacroTableDtor&)rTbl).Next();
    }
    return *this;
}

#pragma SEG_FUNCDEF(macitem_03)

SvStream& SvxMacroTableDtor::Read( SvStream& rStrm, USHORT nVersion )
{
    if( SVX_MACROTBL_VERSION40 <= nVersion )
        rStrm >> nVersion;
    short nMacro;
    rStrm >> nMacro;

    for( short i = 0; i < nMacro; ++i )
    {
        USHORT nCurKey, eType = STARBASIC;
        String aLibName, aMacName;
        rStrm >> nCurKey;
        SfxPoolItem::readByteString(rStrm, aLibName);
        SfxPoolItem::readByteString(rStrm, aMacName);

        if( SVX_MACROTBL_VERSION40 <= nVersion )
            rStrm >> eType;

        SvxMacro* pNew = new SvxMacro( aMacName, aLibName, (ScriptType)eType );

        SvxMacro *pOld = Get( nCurKey );
        if( pOld )
        {
            delete pOld;
            Replace( nCurKey, pNew );
        }
        else
            Insert( nCurKey, pNew );
    }
    return rStrm;
}

#pragma SEG_FUNCDEF(macitem_04)

SvStream& SvxMacroTableDtor::Write( SvStream& rStream ) const
{
    USHORT nVersion = SOFFICE_FILEFORMAT_31 == rStream.GetVersion()
                                    ? SVX_MACROTBL_VERSION31
                                    : SVX_MACROTBL_AKTVERSION;

    if( SVX_MACROTBL_VERSION40 <= nVersion )
        rStream << nVersion;

    rStream << (USHORT)Count();

    SvxMacro* pMac = ((SvxMacroTableDtor*)this)->First();
    while( pMac && rStream.GetError() == SVSTREAM_OK )
    {
        rStream << (short)GetCurKey();
        SfxPoolItem::writeByteString(rStream, pMac->GetLibName());
        SfxPoolItem::writeByteString(rStream, pMac->GetMacName());

        if( SVX_MACROTBL_VERSION40 <= nVersion )
            rStream << (USHORT)pMac->GetScriptType();
        pMac = ((SvxMacroTableDtor*)this)->Next();
    }
    return rStream;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(macitem_05)

void SvxMacroTableDtor::DelDtor()
{
    SvxMacro* pTmp = First();
    while( pTmp )
    {
        delete pTmp;
        pTmp = Next();
    }
    Clear();
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(macitem_06)

int SvxMacroItem::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rAttr), "unequal types" );

    const SvxMacroTableDtor& rOwn = aMacroTable;
    const SvxMacroTableDtor& rOther = ( (SvxMacroItem&) rAttr ).aMacroTable;

    // Anzahl unterschiedlich => auf jeden Fall ungleich
    if ( rOwn.Count() != rOther.Count() )
        return FALSE;

    // einzeln verleichen; wegen Performance ist die Reihenfolge wichtig
    for ( USHORT nNo = 0; nNo < rOwn.Count(); ++nNo )
    {
        const SvxMacro *pOwnMac = rOwn.GetObject(nNo);
        const SvxMacro *pOtherMac = rOther.GetObject(nNo);
        if (    rOwn.GetKey(pOwnMac) != rOther.GetKey(pOtherMac)  ||
                pOwnMac->GetLibName() != pOtherMac->GetLibName() ||
                pOwnMac->GetMacName() != pOtherMac->GetMacName() )
            return FALSE;
    }

    return TRUE;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(macitem_07)

SfxPoolItem* SvxMacroItem::Clone( SfxItemPool* ) const
{
    return new SvxMacroItem( *this );
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(macitem_08)

SfxItemPresentation SvxMacroItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          eCoreUnit,
    SfxMapUnit          ePresUnit,
    XubString&          rText,
    const International *
)   const
{
/*!!!
    SvxMacroTableDtor& rTbl = (SvxMacroTableDtor&)GetMacroTable();
    SvxMacro* pMac = rTbl.First();

    while ( pMac )
    {
        rText += pMac->GetLibName();
        rText += cpDelim;
        rText += pMac->GetMacName();
        pMac = rTbl.Next();
        if ( pMac )
            rText += cpDelim;
    }
*/
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(macitem_09)

SvStream& SvxMacroItem::Store( SvStream& rStrm , USHORT nItemVersion ) const
{
    return aMacroTable.Write( rStrm );
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(macitem_0a)

SfxPoolItem* SvxMacroItem::Create( SvStream& rStrm, USHORT nVersion ) const
{
    SvxMacroItem* pAttr = new SvxMacroItem( Which() );
    pAttr->aMacroTable.Read( rStrm, nVersion );
    return pAttr;
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(macitem_0b)

void SvxMacroItem::SetMacro( USHORT nEvent, const SvxMacro& rMacro )
{
    SvxMacro *pMacro;
    if ( 0 != (pMacro=aMacroTable.Get(nEvent)) )
    {
        delete pMacro;
        aMacroTable.Replace(nEvent, new SvxMacro( rMacro ) );
    }
    else
        aMacroTable.Insert(nEvent, new SvxMacro( rMacro ) );
}

// -----------------------------------------------------------------------
#pragma SEG_FUNCDEF(macitem_0c)

USHORT SvxMacroItem::GetVersion( USHORT nFileFormatVersion ) const
{
    return SOFFICE_FILEFORMAT_31 == nFileFormatVersion
                ? 0 : aMacroTable.GetVersion();
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.15  2000/09/18 14:13:34  willem.vandorp
    OpenOffice header added.

    Revision 1.14  2000/08/31 13:41:28  willem.vandorp
    Header and footer replaced

    Revision 1.13  2000/04/12 08:01:39  sb
    Adapted to Unicode.

    Revision 1.12  2000/02/09 16:23:32  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.11  1999/09/01 06:52:36  sb
    #66082# Added International param to SfxPoolItem::GetPresentation().

    Revision 1.10  1998/07/23 11:54:38  ANK
    #53261# SvxMacro- Konstruktor geaendert setzt jetzt auch eType


      Rev 1.9   23 Jul 1998 13:54:38   ANK
   #53261# SvxMacro- Konstruktor geaendert setzt jetzt auch eType

      Rev 1.8   08 Jul 1998 16:06:16   MIB
   Get(JavaScript|StarBasic)String

      Rev 1.7   27 May 1998 19:33:56   ANK
   neuer Konstruktor fuer SvxMacro

      Rev 1.6   15 May 1998 19:42:42   ANK
   neue Fkt SvxMacro::GetLanguage

      Rev 1.5   19 Jan 1998 19:18:06   MH
   chg: header

      Rev 1.4   04 Mar 1997 12:36:00   WP
   WTC Anpassungen verallgemeinert

      Rev 1.3   05 Feb 1997 14:55:24   AB
   SvxMacro geaendert

      Rev 1.2   06 Dec 1996 14:39:18   TRI
   WTC Anpassung

      Rev 1.1   05 Dec 1996 10:28:12   JP
   GetScripType -> GetScriptType

      Rev 1.0   03 Dec 1996 19:39:56   JP
   Initial revision.

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE

