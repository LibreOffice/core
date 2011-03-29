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
#include "precompiled_idl.hxx"

#include <ctype.h>
#include <stdio.h>

#include <module.hxx>
#include <globals.hxx>
#include <database.hxx>
#include <tools/fsys.hxx>
#include <tools/debug.hxx>

SV_IMPL_META_FACTORY1( SvMetaModule, SvMetaExtern );

SvMetaModule::SvMetaModule()
#ifdef IDL_COMPILER
    : bImported( sal_False )
    , bIsModified( sal_False )
#endif
{
}

#ifdef IDL_COMPILER
SvMetaModule::SvMetaModule( const String & rIdlFileName, sal_Bool bImp )
    : aIdlFileName( rIdlFileName )
    , bImported( bImp ), bIsModified( sal_False )
{
}
#endif

#define MODULE_VER      0x0001
void SvMetaModule::Load( SvPersistStream & rStm )
{
    bImported = sal_True; // import always
    SvMetaExtern::Load( rStm );

    sal_uInt16 nVer;

    rStm >> nVer; // version
    DBG_ASSERT( (nVer & ~IDL_WRITE_MASK) == MODULE_VER, "false version" );

    rStm >> aClassList;
    rStm >> aTypeList;
    rStm >> aAttrList;
    // browser
    rStm.ReadByteString( aIdlFileName );
    rStm.ReadByteString( aHelpFileName );
    rStm.ReadByteString( aSlotIdFile );
    rStm.ReadByteString( aModulePrefix );

    // read compiler data
    sal_uInt16 nCmpLen;
    rStm >> nCmpLen;
#ifdef IDL_COMPILER
    DBG_ASSERT( (nVer & IDL_WRITE_MASK) == IDL_WRITE_COMPILER,
                "no idl compiler format" );
    rStm >> aBeginName;
    rStm >> aEndName;
    rStm >> aNextName;
#else
    rStm->SeekRel( nCmpLen );
#endif
}

void SvMetaModule::Save( SvPersistStream & rStm )
{
    SvMetaExtern::Save( rStm );

    rStm << (sal_uInt16)(MODULE_VER | IDL_WRITE_COMPILER); // Version

    rStm << aClassList;
    rStm << aTypeList;
    rStm << aAttrList;
    // browser
    rStm.WriteByteString( aIdlFileName );
    rStm.WriteByteString( aHelpFileName );
    rStm.WriteByteString( aSlotIdFile );
    rStm.WriteByteString( aModulePrefix );

    // write compiler data
    sal_uInt16 nCmpLen = 0;
    sal_uLong nLenPos = rStm.Tell();
    rStm << nCmpLen;
#ifdef IDL_COMPILER
    rStm << aBeginName;
    rStm << aEndName;
    rStm << aNextName;
    // write length of compiler data
    sal_uLong nPos = rStm.Tell();
    rStm.Seek( nLenPos );
    rStm << (sal_uInt16)( nPos - nLenPos - sizeof( sal_uInt16 ) );
    rStm.Seek( nPos );
#endif
}

sal_Bool SvMetaModule::SetName( const ByteString & rName, SvIdlDataBase * pBase )
{
    if( pBase )
    {
        if( pBase->GetModule( rName ) )
            return sal_False;
    }
    return SvMetaExtern::SetName( rName );
}

#ifdef IDL_COMPILER
sal_Bool SvMetaModule::FillNextName( SvGlobalName * pName )
{
    *pName = aNextName;

    if( aNextName < aEndName )
    {
        ++aNextName;
        bIsModified = sal_True;
        return sal_True;
    }
    return sal_False;
}

void SvMetaModule::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                        SvTokenStream & rInStm )
{
    SvMetaExtern::ReadAttributesSvIdl( rBase, rInStm );

    aHelpFileName.ReadSvIdl( SvHash_HelpFile(), rInStm );
    if( aSlotIdFile.ReadSvIdl( SvHash_SlotIdFile(), rInStm ) )
    {
        sal_uInt32 nTokPos = rInStm.Tell();
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

void SvMetaModule::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                         SvStream & rOutStm,
                                         sal_uInt16 nTab )
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

void SvMetaModule::ReadContextSvIdl( SvIdlDataBase & rBase,
                                     SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    if( rInStm.GetToken()->Is( SvHash_interface() )
      || rInStm.GetToken()->Is( SvHash_shell() ) )
    {
        SvMetaClassRef aClass = new SvMetaClass();
        if( aClass->ReadSvIdl( rBase, rInStm ) )
        {
            aClassList.Append( aClass );
            // announce globally
            rBase.GetClassList().Append( aClass );
        }
    }
    else if( rInStm.GetToken()->Is( SvHash_enum() ) )
    {
        SvMetaTypeEnumRef aEnum = new SvMetaTypeEnum();

        if( aEnum->ReadSvIdl( rBase, rInStm ) )
        {
            // declared in module
            aTypeList.Append( aEnum );
            // announce globally
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
            // declared in module
            aTypeList.Append( xItem );
            // announce globally
            rBase.GetTypeList().Append( xItem );
        }
    }
    else if( rInStm.GetToken()->Is( SvHash_include() ) )
    {
        sal_Bool bOk = sal_False;
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
                    // rescue error from old file
                    SvIdlError aOldErr = rBase.GetError();
                    // reset error
                    rBase.SetError( SvIdlError() );

                    sal_uInt32 nBeginPos = 0xFFFFFFFF; // can not happen with Tell
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
                    // recover error from old file
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
                // declared in module
                aAttrList.Append( xSlot );
                // announce globally
                rBase.AppendAttr( xSlot );
            }
        }
    }
}

void SvMetaModule::WriteContextSvIdl( SvIdlDataBase & rBase,
                                      SvStream & rOutStm,
                                      sal_uInt16 nTab )
{
    SvMetaExtern::WriteContextSvIdl( rBase, rOutStm, nTab );
    sal_uLong n;
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

sal_Bool SvMetaModule::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    bIsModified = sal_True; // up to now always when compiler running

    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok  = rInStm.GetToken_Next();
    sal_Bool bOk        = sal_False;
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

        rBase.Push( this ); // onto the context stack

        if( ReadNameSvIdl( rBase, rInStm ) )
        {
            // set pointer to itself
            SetModule( rBase );
            bOk = SvMetaName::ReadSvIdl( rBase, rInStm );
        }
        rBase.GetStack().Pop(); // remove from stack
    }
    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

void SvMetaModule::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                               sal_uInt16 nTab )
{
    rOutStm << SvHash_module()->GetName().GetBuffer() << endl
            << '\"';
    rOutStm.WriteByteString( aBeginName.GetHexName() );
    rOutStm << '\"' << endl << '\"';
    rOutStm.WriteByteString( aEndName.GetHexName() );
    rOutStm << '\"' << endl;
    SvMetaExtern::WriteSvIdl( rBase, rOutStm, nTab );
}

void SvMetaModule::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    for( sal_uLong n = 0; n < aClassList.Count(); n++ )
    {
        SvMetaClass * pClass = aClassList.GetObject( n );
        pClass->WriteSfx( rBase, rOutStm );
    }
}

void SvMetaModule::WriteHelpIds( SvIdlDataBase & rBase, SvStream & rOutStm,
                            Table* pTable )
{
    for( sal_uLong n = 0; n < aClassList.Count(); n++ )
    {
        SvMetaClass * pClass = aClassList.GetObject( n );
        pClass->WriteHelpIds( rBase, rOutStm, pTable );
    }
}

void SvMetaModule::WriteAttributes( SvIdlDataBase & rBase,
                                    SvStream & rOutStm,
                                     sal_uInt16 nTab,
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

void SvMetaModule::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                              sal_uInt16 nTab,
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

        for( sal_uLong n = 0; n < aClassList.Count(); n++ )
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
        WriteDescription( rOutStm );
        rOutStm << "</MODULE>" << endl << endl;

        rOutStm << "<CLASSES>" << endl;
        for( sal_uLong n = 0; n < aClassList.Count(); n++ )
        {
            SvMetaClass * pClass = aClassList.GetObject( n );
            if( !pClass->IsShell() )
            {
                rOutStm << pClass->GetName().GetBuffer();
                SvMetaClass* pSC = pClass->GetSuperClass();
                if( pSC )
                    rOutStm << " : " << pSC->GetName().GetBuffer();

                // imported classes
                const SvClassElementMemberList& rClassList = pClass->GetClassList();
                if ( rClassList.Count() )
                {
                    rOutStm << " ( ";

                    for( sal_uLong m=0; m<rClassList.Count(); m++ )
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
        // no break!
    }

    case WRITE_C_SOURCE:
    case WRITE_C_HEADER:
    {
        for( sal_uLong n = 0; n < aClassList.Count(); n++ )
        {
            SvMetaClass * pClass = aClassList.GetObject( n );
            if( !pClass->IsShell() )
                pClass->Write( rBase, rOutStm, nTab, nT, nA );
        }
    }
    break;

    default:
        break;
    }
}

void SvMetaModule::WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                             Table * pTable )
{
    if( aSlotIdFile.Len() )
        rOutStm << "//#include <" << aSlotIdFile.GetBuffer() << '>' << endl;
    for( sal_uLong n = 0; n < aClassList.Count(); n++ )
    {
        aClassList.GetObject( n )->WriteSrc( rBase, rOutStm, pTable );
    }
}

void SvMetaModule::WriteHxx( SvIdlDataBase & rBase, SvStream & rOutStm,
                             sal_uInt16 nTab )
{
    for( sal_uLong n = 0; n < aClassList.Count(); n++ )
    {
        SvMetaClass * pClass = aClassList.GetObject( n );
        pClass->WriteHxx( rBase, rOutStm, nTab );
    }
}

void SvMetaModule::WriteCxx( SvIdlDataBase & rBase, SvStream & rOutStm,
                             sal_uInt16 nTab )
{
    for( sal_uLong n = 0; n < aClassList.Count(); n++ )
    {
        SvMetaClass * pClass = aClassList.GetObject( n );
        pClass->WriteCxx( rBase, rOutStm, nTab );
    }
}

#endif // IDL_COMPILER

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
