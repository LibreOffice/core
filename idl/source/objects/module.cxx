/*************************************************************************
 *
 *  $RCSfile: module.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:41 $
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

#include <ctype.h>
#include <stdio.h>

#include <attrib.hxx>
#include <module.hxx>
#include <globals.hxx>
#include <database.hxx>

#ifndef _FSYS_HXX //autogen
#include <tools/fsys.hxx>
#endif
#include <tools/debug.hxx>

#pragma hdrstop


/****************** SvMetaModule ******************************************/
SV_IMPL_META_FACTORY1( SvMetaModule, SvMetaExtern );
#ifdef IDL_COMPILER
SvAttributeList & SvMetaModule::GetAttributeList()
{
    if( !pAttribList )
    {
        pAttribList = new SvAttributeList();
    }
    return *pAttribList;
}
#endif

/*************************************************************************
|*
|*    SvMetaModule::SvMetaModule()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
SvMetaModule::SvMetaModule()
#ifdef IDL_COMPILER
    : bImported( FALSE )
    , bIsModified( FALSE )
#endif
{
}

#ifdef IDL_COMPILER
SvMetaModule::SvMetaModule( const String & rIdlFileName, BOOL bImp )
    : aIdlFileName( rIdlFileName )
    , bImported( bImp ), bIsModified( FALSE )
{
}
#endif

/*************************************************************************
|*
|*    SvMetaModule::Load()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
#define MODULE_VER      0x0001
void SvMetaModule::Load( SvPersistStream & rStm )
{
    bImported = TRUE; // immer importiert
    SvMetaExtern::Load( rStm );

    USHORT nVer;

    rStm >> nVer; // Version
    DBG_ASSERT( (nVer & ~IDL_WRITE_MASK) == MODULE_VER, "false version" )

    rStm >> aClassList;
    rStm >> aTypeList;
    rStm >> aAttrList;
    // Browser
    rStm.ReadByteString( aIdlFileName );
    rStm.ReadByteString( aHelpFileName );
    rStm.ReadByteString( aSlotIdFile );
    rStm.ReadByteString( aModulePrefix );

    // Compiler Daten lesen
    USHORT nCmpLen;
    rStm >> nCmpLen;
#ifdef IDL_COMPILER
    DBG_ASSERT( (nVer & IDL_WRITE_MASK) == IDL_WRITE_COMPILER,
                "no idl compiler format" )
    rStm >> aBeginName;
    rStm >> aEndName;
    rStm >> aNextName;
#else
    rStm->SeekRel( nCmpLen );
#endif
}

/*************************************************************************
|*
|*    SvMetaModule::Save()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
void SvMetaModule::Save( SvPersistStream & rStm )
{
    SvMetaExtern::Save( rStm );

    rStm << (USHORT)(MODULE_VER | IDL_WRITE_COMPILER); // Version

    rStm << aClassList;
    rStm << aTypeList;
    rStm << aAttrList;
    // Browser
    rStm.WriteByteString( aIdlFileName );
    rStm.WriteByteString( aHelpFileName );
    rStm.WriteByteString( aSlotIdFile );
    rStm.WriteByteString( aModulePrefix );

    // Compiler Daten schreiben
    USHORT nCmpLen = 0;
    ULONG nLenPos = rStm.Tell();
    rStm << nCmpLen;
#ifdef IDL_COMPILER
    rStm << aBeginName;
    rStm << aEndName;
    rStm << aNextName;
    // Laenge der Compiler Daten schreiben
    ULONG nPos = rStm.Tell();
    rStm.Seek( nLenPos );
    rStm << (USHORT)( nPos - nLenPos - sizeof( USHORT ) );
    rStm.Seek( nPos );
#endif
}

/*************************************************************************
|*
|*    SvMetaModule::SetName()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.12.94
|*    Letzte Aenderung  MM 12.12.94
|*
*************************************************************************/
BOOL SvMetaModule::SetName( const ByteString & rName, SvIdlDataBase * pBase )
{
    if( pBase )
    {
        if( pBase->GetModule( rName ) )
            return FALSE;
    }
    return SvMetaExtern::SetName( rName );
}

#ifdef IDL_COMPILER
/*************************************************************************
|*    SvMetaModule::GetNextName()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaModule::FillNextName( SvGlobalName * pName )
{
    *pName = aNextName;

    if( aNextName < aEndName )
    {
        ++aNextName;
        bIsModified = TRUE;
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
|*    SvMetaModule::ReadSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaModule::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                        SvTokenStream & rInStm )
{
    SvMetaExtern::ReadAttributesSvIdl( rBase, rInStm );

    aHelpFileName.ReadSvIdl( SvHash_HelpFile(), rInStm );
    if( aSlotIdFile.ReadSvIdl( SvHash_SlotIdFile(), rInStm ) )
    {
        UINT32 nTokPos = rInStm.Tell();
        if( !rBase.ReadIdFile( String::CreateFromAscii( aSlotIdFile.GetBuffer() ) ) )
        {
            ByteString aStr = "cannot read file: ";
            aStr += aSlotIdFile;
            rBase.SetError( aStr, rInStm.GetToken() );
            rBase.WriteError( rInStm );

            rInStm.Seek( nTokPos );
        }
    }
    aTypeLibFile.ReadSvIdl( SvHash_TypeLibFile(), rInStm );
    aModulePrefix.ReadSvIdl( SvHash_ModulePrefix(), rInStm );
}

/*************************************************************************
|*    SvMetaModule::WriteAttributesSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaModule::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                         SvStream & rOutStm,
                                         USHORT nTab )
{
    SvMetaExtern::WriteAttributesSvIdl( rBase, rOutStm, nTab );
    if( aTypeLibFile.Len() || aSlotIdFile.Len() || aTypeLibFile.Len() )
    {
        if( aHelpFileName.Len() )
        {
            WriteTab( rOutStm, nTab );
            aHelpFileName.WriteSvIdl( SvHash_HelpFile(), rOutStm, nTab +1 );
            rOutStm << ';' << endl;
        }
        if( aSlotIdFile.Len() )
        {
            WriteTab( rOutStm, nTab );
            aSlotIdFile.WriteSvIdl( SvHash_SlotIdFile(), rOutStm, nTab +1 );
            rOutStm << ';' << endl;
        }
        if( aTypeLibFile.Len() )
        {
            WriteTab( rOutStm, nTab );
            aTypeLibFile.WriteSvIdl( SvHash_TypeLibFile(), rOutStm, nTab +1 );
            rOutStm << ';' << endl;
        }
    }
}

/*************************************************************************
|*    SvMetaModule::ReadContextSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaModule::ReadContextSvIdl( SvIdlDataBase & rBase,
                                     SvTokenStream & rInStm )
{
    UINT32  nTokPos = rInStm.Tell();
    if( rInStm.GetToken()->Is( SvHash_interface() )
      || rInStm.GetToken()->Is( SvHash_shell() ) )
    {
        SvMetaClassRef aClass = new SvMetaClass();
        if( aClass->ReadSvIdl( rBase, rInStm ) )
        {
            aClassList.Append( aClass );
            // Global bekanntgeben
            rBase.GetClassList().Append( aClass );
        }
    }
    else if( rInStm.GetToken()->Is( SvHash_enum() ) )
    {
        SvMetaTypeEnumRef aEnum = new SvMetaTypeEnum();

        if( aEnum->ReadSvIdl( rBase, rInStm ) )
        {
            // Im Modul deklariert
            aTypeList.Append( aEnum );
            // Global bekanntgeben
            rBase.GetTypeList().Append( aEnum );
        }
    }
    else if( rInStm.GetToken()->Is( SvHash_item() )
      || rInStm.GetToken()->Is( SvHash_struct() )
      || rInStm.GetToken()->Is( SvHash_typedef() ) )
    {
        SvMetaTypeRef xItem = new SvMetaType();

        if( xItem->ReadSvIdl( rBase, rInStm ) )
        {
            // Im Modul deklariert
            aTypeList.Append( xItem );
            // Global bekanntgeben
            rBase.GetTypeList().Append( xItem );
        }
    }
    else if( rInStm.GetToken()->Is( SvHash_include() ) )
    {
        BOOL bOk = FALSE;
        rInStm.GetToken_Next();
        SvToken * pTok = rInStm.GetToken_Next();
        if( pTok->IsString() )
        {
            DirEntry aFullName( String::CreateFromAscii( pTok->GetString().GetBuffer() ) );
            rBase.StartNewFile( aFullName.GetFull() );
            if( aFullName.Find( rBase.GetPath() ) )
            {
                SvTokenStream aTokStm( aFullName.GetFull() );
                if( SVSTREAM_OK == aTokStm.GetStream().GetError() )
                {
                    // Fehler aus alter Datei retten
                    SvIdlError aOldErr = rBase.GetError();
                    // Fehler zuruecksetzen
                    rBase.SetError( SvIdlError() );

                    UINT32 nBeginPos = 0xFFFFFFFF; // kann mit Tell nicht vorkommen
                    while( nBeginPos != aTokStm.Tell() )
                    {
                        nBeginPos = aTokStm.Tell();
                        ReadContextSvIdl( rBase, aTokStm );
                        aTokStm.ReadDelemiter();
                    }
                    bOk = aTokStm.GetToken()->IsEof();
                    if( !bOk )
                    {
                        rBase.WriteError( aTokStm );
                    }
                    // Fehler aus alter Datei wieder herstellen
                    rBase.SetError( aOldErr );
                }
                else
                {
                    ByteString aStr = "cannot open file: ";
                    aStr += ByteString( aFullName.GetFull(), RTL_TEXTENCODING_UTF8 );
                    rBase.SetError( aStr, pTok );
                }
            }
            else
            {
                ByteString aStr = "cannot find file: ";
                aStr += ByteString( aFullName.GetFull(), RTL_TEXTENCODING_UTF8 );
                rBase.SetError( aStr, pTok );
            }
        }
        if( !bOk )
            rInStm.Seek( nTokPos );
    }
    else
    {
        SvMetaSlotRef xSlot = new SvMetaSlot();

        if( xSlot->ReadSvIdl( rBase, rInStm ) )
        {
            if( xSlot->Test( rBase, rInStm ) )
            {
                // Im Modul deklariert
                aAttrList.Append( xSlot );
                // Global bekanntgeben
                rBase.AppendAttr( xSlot );
            }
        }
    }
}

/*************************************************************************
|*    SvMetaModule::WriteContextSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaModule::WriteContextSvIdl( SvIdlDataBase & rBase,
                                      SvStream & rOutStm,
                                      USHORT nTab )
{
    SvMetaExtern::WriteContextSvIdl( rBase, rOutStm, nTab );
    ULONG n;
    for( n = 0; n < aTypeList.Count(); n++ )
    {
        WriteTab( rOutStm, nTab );
        aTypeList.GetObject( n )->WriteSvIdl( rBase, rOutStm, nTab );
    }
    rOutStm << endl;
    for( n = 0; n < aAttrList.Count(); n++ )
    {
        WriteTab( rOutStm, nTab );
        aAttrList.GetObject( n )->WriteSvIdl( rBase, rOutStm, nTab );
    }
    rOutStm << endl;
    for( n = 0; n < aClassList.Count(); n++ )
    {
        WriteTab( rOutStm, nTab );
        aClassList.GetObject( n )->WriteSvIdl( rBase, rOutStm, nTab );
    }
}

/*************************************************************************
|*
|*    SvMetaModule::ReadSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL SvMetaModule::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    bIsModified = TRUE; // bisher immer wenn Compiler laueft

    UINT32  nTokPos = rInStm.Tell();
    SvToken * pTok  = rInStm.GetToken_Next();
    BOOL bOk        = FALSE;
    bOk = pTok->Is( SvHash_module() );
    if( bOk )
    {
        pTok = rInStm.GetToken_Next();
        if( pTok->IsString() )
             bOk = aBeginName.MakeId( String::CreateFromAscii( pTok->GetString().GetBuffer() ) );
    }
    rInStm.ReadDelemiter();
    if( bOk )
    {
        pTok = rInStm.GetToken_Next();
        if( pTok->IsString() )
             bOk = aEndName.MakeId( String::CreateFromAscii( pTok->GetString().GetBuffer() ) );
    }
    rInStm.ReadDelemiter();
    if( bOk )
    {
        aNextName = aBeginName;

        rBase.Push( this ); // auf den Context Stack

        if( ReadNameSvIdl( rBase, rInStm ) )
        {
            // Zeiger auf sich selbst setzen
            SetModule( rBase );
            bOk = SvMetaName::ReadSvIdl( rBase, rInStm );
        }
        rBase.GetStack().Pop(); // und runter
    }
    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

/*************************************************************************
|*
|*    SvMetaModule::WriteSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
void SvMetaModule::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                               USHORT nTab )
{
    rOutStm << SvHash_module()->GetName().GetBuffer() << endl
            << '\"';
    rOutStm.WriteByteString( aBeginName.GetHexName() );
    rOutStm << '\"' << endl << '\"';
    rOutStm.WriteByteString( aEndName.GetHexName() );
    rOutStm << '\"' << endl;
    SvMetaExtern::WriteSvIdl( rBase, rOutStm, nTab );
}

/*************************************************************************
|*    SvMetaModule::WriteSfx()
*************************************************************************/
void SvMetaModule::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    for( ULONG n = 0; n < aClassList.Count(); n++ )
    {
        SvMetaClass * pClass = aClassList.GetObject( n );
        pClass->WriteSfx( rBase, rOutStm );
    }
}

void SvMetaModule::WriteHelpIds( SvIdlDataBase & rBase, SvStream & rOutStm,
                            Table* pTable )
{
    for( ULONG n = 0; n < aClassList.Count(); n++ )
    {
        SvMetaClass * pClass = aClassList.GetObject( n );
        pClass->WriteHelpIds( rBase, rOutStm, pTable );
    }
}

/*************************************************************************
|*    SvMetaModule::WriteAttributes()
*************************************************************************/
void SvMetaModule::WriteAttributes( SvIdlDataBase & rBase,
                                    SvStream & rOutStm,
                                     USHORT nTab,
                                     WriteType nT, WriteAttribute nA )
{
    SvMetaExtern::WriteAttributes( rBase, rOutStm, nTab, nT, nA );
    if( aHelpFileName.Len() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << "// class SvMetaModule" << endl;
        WriteTab( rOutStm, nTab );
        rOutStm << "helpfile(\"" << aHelpFileName.GetBuffer() << "\");" << endl;
    }
}

/*************************************************************************
|*    SvMetaModule::WriteSbx()
*************************************************************************/
/*
void SvMetaModule::WriteSbx( SvIdlDataBase & rBase, SvStream & rOutStm,
                                SvNamePosList & rList )
{
    for( ULONG n = 0; n < aClassList.Count(); n++ )
    {
        SvMetaClass * pClass = aClassList.GetObject( n );
        if( !pClass->IsShell() && pClass->GetAutomation() )
        {
            rList.Insert( new SvNamePos( pClass->GetUUId(), rOutStm.Tell() ),
                        LIST_APPEND );
            SbxObjectRef xSbxObj = new SbxObject( pClass->GetName() );
            pClass->FillSbxObject( rBase, xSbxObj );
            xSbxObj->Store( rOutStm );
        }
    }
}
 */

/*************************************************************************
|*    SvMetaModule::Write()
*************************************************************************/
void SvMetaModule::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                              USHORT nTab,
                             WriteType nT, WriteAttribute nA )
{
    switch ( nT )
    {
    case WRITE_ODL:
    {
        if( aSlotIdFile.Len() )
        {
            WriteTab( rOutStm, nTab );
            rOutStm << "#include \"" << aSlotIdFile.GetBuffer() << '"' << endl << endl;
        }
        SvMetaExtern::Write( rBase, rOutStm, nTab, nT, nA );
        rOutStm << endl;
        WriteTab( rOutStm, nTab );
        rOutStm << "library " << GetName().GetBuffer() << endl;
        WriteTab( rOutStm, nTab );
        rOutStm << '{' << endl;
        WriteTab( rOutStm, nTab );
        rOutStm << "importlib(\"STDOLE.TLB\");" << endl;

    /*
        for( ULONG n = 0; n < aTypeList.Count(); n++ )
        {
            SvMetaType * pType = aTypeList.GetObject( n );
            if( !pType ->Write( rBase, rOutStm, nTab +1, nT, nA ) )
                return FALSE;
        }
    */
        /*
        for( ULONG n = 0; n < rBase.GetModuleList().Count(); n++ )
        {
            SvMetaModule * pModule = rBase.GetModuleList().GetObject( n );
            const SvMetaTypeMemberList &rTypeList = pModule->GetTypeList();
            for( ULONG n = 0; n < rTypeList.Count(); n++ )
            {
                SvMetaType * pType = rTypeList.GetObject( n );
                pType->Write( rBase, rOutStm, nTab +1, nT, nA );
            }
        }
        */

        for( ULONG n = 0; n < aClassList.Count(); n++ )
        {
            SvMetaClass * pClass = aClassList.GetObject( n );
            if( !pClass->IsShell() && pClass->GetAutomation() )
            {
                WriteTab( rOutStm, nTab );
                WriteStars( rOutStm );
                pClass->Write( rBase, rOutStm, nTab +1, nT, nA );
                if( n +1 < aClassList.Count() )
                    rOutStm << endl;
            }
        }

        rOutStm << '}' << endl;
    }
    break;
    case WRITE_DOCU:
    {
        rOutStm << "SvIDL interface documentation" << endl << endl;
        rOutStm << "<MODULE>" << endl << GetName().GetBuffer() << endl;
        WriteDescription( rBase, rOutStm );
        rOutStm << "</MODULE>" << endl << endl;

        rOutStm << "<CLASSES>" << endl;
        for( ULONG n = 0; n < aClassList.Count(); n++ )
        {
            SvMetaClass * pClass = aClassList.GetObject( n );
            if( !pClass->IsShell() )
            {
                rOutStm << pClass->GetName().GetBuffer();
                SvMetaClass* pSC = pClass->GetSuperClass();
                if( pSC )
                    rOutStm << " : " << pSC->GetName().GetBuffer();

                // Importierte Klassen
                const SvClassElementMemberList& rClassList = pClass->GetClassList();
                if ( rClassList.Count() )
                {
                    rOutStm << " ( ";

                    for( ULONG m=0; m<rClassList.Count(); m++ )
                    {
                        SvClassElement *pEle = rClassList.GetObject(m);
                        SvMetaClass *pCl = pEle->GetClass();
                        rOutStm << pCl->GetName().GetBuffer();
                        if ( m+1 == rClassList.Count() )
                            rOutStm << " )";
                        else
                            rOutStm << " , ";
                    }
                }

                rOutStm << endl;
            }
        }
        rOutStm << "</CLASSES>" << endl << endl;
        // kein Break!
    }

    case WRITE_C_SOURCE:
    case WRITE_C_HEADER:
    {
        for( ULONG n = 0; n < aClassList.Count(); n++ )
        {
            SvMetaClass * pClass = aClassList.GetObject( n );
            if( !pClass->IsShell() /* && pClass->GetAutomation() */ )
                pClass->Write( rBase, rOutStm, nTab, nT, nA );
        }
    }
    break;
    }
}

/*************************************************************************
|*    SvMetaModule::WriteSrc()
*************************************************************************/
void SvMetaModule::WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                             Table * pTable )
{
//  rOutStm << "#pragma CHARSET IBMPC" << endl;
    if( aSlotIdFile.Len() )
        rOutStm << "//#include <" << aSlotIdFile.GetBuffer() << '>' << endl;
    for( ULONG n = 0; n < aClassList.Count(); n++ )
    {
        aClassList.GetObject( n )->WriteSrc( rBase, rOutStm, pTable );
    }
}

/*************************************************************************
|*    SvMetaModule::WriteHxx()
*************************************************************************/
void SvMetaModule::WriteHxx( SvIdlDataBase & rBase, SvStream & rOutStm,
                             USHORT nTab )
{
    for( ULONG n = 0; n < aClassList.Count(); n++ )
    {
        SvMetaClass * pClass = aClassList.GetObject( n );
        pClass->WriteHxx( rBase, rOutStm, nTab );
    }
}

/*************************************************************************
|*    SvMetaModule::WriteCxx()
*************************************************************************/
void SvMetaModule::WriteCxx( SvIdlDataBase & rBase, SvStream & rOutStm,
                             USHORT nTab )
{
    for( ULONG n = 0; n < aClassList.Count(); n++ )
    {
        SvMetaClass * pClass = aClassList.GetObject( n );
        pClass->WriteCxx( rBase, rOutStm, nTab );
    }
}

#endif // IDL_COMPILER

