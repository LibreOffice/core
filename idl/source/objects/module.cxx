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


#include <ctype.h>
#include <stdio.h>

#include <module.hxx>
#include <globals.hxx>
#include <database.hxx>
#include <tools/debug.hxx>
#include <osl/file.hxx>

SV_IMPL_META_FACTORY1( SvMetaModule, SvMetaExtern );

SvMetaModule::SvMetaModule()
    : bImported( sal_False )
    , bIsModified( sal_False )
{
}

SvMetaModule::SvMetaModule( const OUString & rIdlFileName, sal_Bool bImp )
    : aIdlFileName( rIdlFileName )
    , bImported( bImp ), bIsModified( sal_False )
{
}

#define MODULE_VER      0x0001
void SvMetaModule::Load( SvPersistStream & rStm )
{
    bImported = sal_True; 
    SvMetaExtern::Load( rStm );

    sal_uInt16 nVer;

    rStm.ReadUInt16( nVer ); 
    DBG_ASSERT( (nVer & ~IDL_WRITE_MASK) == MODULE_VER, "false version" );

    rStm >> aClassList;
    rStm >> aTypeList;
    rStm >> aAttrList;
    
    aIdlFileName = rStm.ReadUniOrByteString( rStm.GetStreamCharSet() );
    aHelpFileName.setString(read_uInt16_lenPrefixed_uInt8s_ToOString(rStm));
    aSlotIdFile.setString(read_uInt16_lenPrefixed_uInt8s_ToOString(rStm));
    aModulePrefix.setString(read_uInt16_lenPrefixed_uInt8s_ToOString(rStm));

    
    sal_uInt16 nCmpLen;
    rStm.ReadUInt16( nCmpLen );
    DBG_ASSERT( (nVer & IDL_WRITE_MASK) == IDL_WRITE_COMPILER,
                "no idl compiler format" );
    rStm >> aBeginName;
    rStm >> aEndName;
    rStm >> aNextName;
}

void SvMetaModule::Save( SvPersistStream & rStm )
{
    SvMetaExtern::Save( rStm );

    rStm.WriteUInt16( (sal_uInt16)(MODULE_VER | IDL_WRITE_COMPILER) ); 

    WriteSvDeclPersistList( rStm, aClassList );
    WriteSvDeclPersistList( rStm, aTypeList );
    WriteSvDeclPersistList( rStm, aAttrList );
    
    rStm.WriteUniOrByteString( aIdlFileName, rStm.GetStreamCharSet() );
    write_uInt16_lenPrefixed_uInt8s_FromOString(rStm, aHelpFileName.getString());
    write_uInt16_lenPrefixed_uInt8s_FromOString(rStm, aSlotIdFile.getString());
    write_uInt16_lenPrefixed_uInt8s_FromOString(rStm, aModulePrefix.getString());

    
    sal_uInt16 nCmpLen = 0;
    sal_uLong nLenPos = rStm.Tell();
    rStm.WriteUInt16( nCmpLen );
    WriteSvGlobalName( rStm, aBeginName );
    WriteSvGlobalName( rStm, aEndName );
    WriteSvGlobalName( rStm, aNextName );
    
    sal_uLong nPos = rStm.Tell();
    rStm.Seek( nLenPos );
    rStm.WriteUInt16( (sal_uInt16)( nPos - nLenPos - sizeof( sal_uInt16 ) ) );
    rStm.Seek( nPos );
}

sal_Bool SvMetaModule::SetName( const OString& rName, SvIdlDataBase * pBase )
{
    if( pBase )
    {
        if( pBase->GetModule( rName ) )
            return sal_False;
    }
    return SvMetaExtern::SetName( rName );
}

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
        if( !rBase.ReadIdFile( OStringToOUString(aSlotIdFile.getString(), RTL_TEXTENCODING_ASCII_US)) )
        {
            OStringBuffer aStr("cannot read file: ");
            aStr.append(aSlotIdFile.getString());
            rBase.SetError( aStr.makeStringAndClear(), rInStm.GetToken() );
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
    if( !aHelpFileName.getString().isEmpty() || !aSlotIdFile.getString().isEmpty() || !aTypeLibFile.getString().isEmpty() )
    {
        if( !aHelpFileName.getString().isEmpty() )
        {
            WriteTab( rOutStm, nTab );
            aHelpFileName.WriteSvIdl( SvHash_HelpFile(), rOutStm, nTab +1 );
            rOutStm.WriteChar( ';' ) << endl;
        }
        if( !aSlotIdFile.getString().isEmpty() )
        {
            WriteTab( rOutStm, nTab );
            aSlotIdFile.WriteSvIdl( SvHash_SlotIdFile(), rOutStm, nTab +1 );
            rOutStm.WriteChar( ';' ) << endl;
        }
        if( !aTypeLibFile.getString().isEmpty() )
        {
            WriteTab( rOutStm, nTab );
            aTypeLibFile.WriteSvIdl( SvHash_TypeLibFile(), rOutStm, nTab +1 );
            rOutStm.WriteChar( ';' ) << endl;
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
            aClassList.push_back( aClass );
            
            rBase.GetClassList().push_back( aClass );
        }
    }
    else if( rInStm.GetToken()->Is( SvHash_enum() ) )
    {
        SvMetaTypeEnumRef aEnum = new SvMetaTypeEnum();

        if( aEnum->ReadSvIdl( rBase, rInStm ) )
        {
            
            aTypeList.push_back( aEnum );
            
            rBase.GetTypeList().push_back( aEnum );
        }
    }
    else if( rInStm.GetToken()->Is( SvHash_item() )
      || rInStm.GetToken()->Is( SvHash_struct() )
      || rInStm.GetToken()->Is( SvHash_typedef() ) )
    {
        SvMetaTypeRef xItem = new SvMetaType();

        if( xItem->ReadSvIdl( rBase, rInStm ) )
        {
            
            aTypeList.push_back( xItem );
            
            rBase.GetTypeList().push_back( xItem );
        }
    }
    else if( rInStm.GetToken()->Is( SvHash_include() ) )
    {
        sal_Bool bOk = sal_False;
        rInStm.GetToken_Next();
        SvToken * pTok = rInStm.GetToken_Next();
        if( pTok->IsString() )
        {
            OUString aFullName(OStringToOUString(pTok->GetString(), RTL_TEXTENCODING_ASCII_US));
            rBase.StartNewFile( aFullName );
            osl::FileBase::RC searchError = osl::File::searchFileURL(aFullName, rBase.GetPath(), aFullName);
            osl::FileBase::getSystemPathFromFileURL( aFullName, aFullName );

            if( osl::FileBase::E_None == searchError )
            {
                rBase.AddDepFile( aFullName );
                SvTokenStream aTokStm( aFullName );

                if( SVSTREAM_OK == aTokStm.GetStream().GetError() )
                {
                    
                    SvIdlError aOldErr = rBase.GetError();
                    
                    rBase.SetError( SvIdlError() );

                    sal_uInt32 nBeginPos = 0xFFFFFFFF; 
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
                    
                    rBase.SetError( aOldErr );
                }
                else
                {
                    OStringBuffer aStr("cannot open file: ");
                    aStr.append(OUStringToOString(aFullName, RTL_TEXTENCODING_UTF8));
                    rBase.SetError(aStr.makeStringAndClear(), pTok);
                }
            }
            else
            {
                OStringBuffer aStr("cannot find file:");
                aStr.append(OUStringToOString(aFullName, RTL_TEXTENCODING_UTF8));
                rBase.SetError(aStr.makeStringAndClear(), pTok);
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
                
                aAttrList.push_back( xSlot );
                
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
    for( n = 0; n < aTypeList.size(); n++ )
    {
        WriteTab( rOutStm, nTab );
        aTypeList[n]->WriteSvIdl( rBase, rOutStm, nTab );
    }
    rOutStm << endl;
    for( n = 0; n < aAttrList.size(); n++ )
    {
        WriteTab( rOutStm, nTab );
        aAttrList[n]->WriteSvIdl( rBase, rOutStm, nTab );
    }
    rOutStm << endl;
    for( n = 0; n < aClassList.size(); n++ )
    {
        WriteTab( rOutStm, nTab );
        aClassList[n]->WriteSvIdl( rBase, rOutStm, nTab );
    }
}

sal_Bool SvMetaModule::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    bIsModified = sal_True; 

    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok  = rInStm.GetToken_Next();
    sal_Bool bOk = pTok->Is( SvHash_module() );
    if( bOk )
    {
        pTok = rInStm.GetToken_Next();
        if( pTok->IsString() )
             bOk = aBeginName.MakeId(OStringToOUString(pTok->GetString(), RTL_TEXTENCODING_ASCII_US));
    }
    rInStm.ReadDelemiter();
    if( bOk )
    {
        pTok = rInStm.GetToken_Next();
        if( pTok->IsString() )
             bOk = aEndName.MakeId(OStringToOUString(pTok->GetString(), RTL_TEXTENCODING_ASCII_US));
    }
    rInStm.ReadDelemiter();
    if( bOk )
    {
        aNextName = aBeginName;

        rBase.Push( this ); 

        if( ReadNameSvIdl( rBase, rInStm ) )
        {
            
            SetModule( rBase );
            bOk = SvMetaName::ReadSvIdl( rBase, rInStm );
        }
        rBase.GetStack().Pop(); 
    }
    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

void SvMetaModule::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                               sal_uInt16 nTab )
{
    rOutStm.WriteCharPtr( SvHash_module()->GetName().getStr() ) << endl;
    rOutStm.WriteChar( '\"' );
    rOutStm.WriteUniOrByteString( aBeginName.GetHexName(), rOutStm.GetStreamCharSet() );
    rOutStm.WriteChar( '\"' ) << endl;
    rOutStm.WriteChar( '\"' );
    rOutStm.WriteUniOrByteString( aEndName.GetHexName(), rOutStm.GetStreamCharSet() );
    rOutStm.WriteChar( '\"' ) << endl;
    SvMetaExtern::WriteSvIdl( rBase, rOutStm, nTab );
}

void SvMetaModule::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    for( sal_uLong n = 0; n < aClassList.size(); n++ )
    {
        SvMetaClass * pClass = aClassList[n];
        pClass->WriteSfx( rBase, rOutStm );
    }
}

void SvMetaModule::WriteHelpIds( SvIdlDataBase & rBase, SvStream & rOutStm,
                            HelpIdTable& rTable )
{
    for( sal_uLong n = 0; n < aClassList.size(); n++ )
    {
        SvMetaClass * pClass = aClassList[n];
        pClass->WriteHelpIds( rBase, rOutStm, rTable );
    }
}

void SvMetaModule::WriteAttributes( SvIdlDataBase & rBase,
                                    SvStream & rOutStm,
                                     sal_uInt16 nTab,
                                     WriteType nT, WriteAttribute nA )
{
    SvMetaExtern::WriteAttributes( rBase, rOutStm, nTab, nT, nA );
    if( !aHelpFileName.getString().isEmpty() )
    {
        WriteTab( rOutStm, nTab );
        rOutStm.WriteCharPtr( "
        WriteTab( rOutStm, nTab );
        rOutStm.WriteCharPtr( "helpfile(\"" ).WriteCharPtr( aHelpFileName.getString().getStr() ).WriteCharPtr( "\");" ) << endl;
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
        if( !aSlotIdFile.getString().isEmpty() )
        {
            WriteTab( rOutStm, nTab );
            rOutStm.WriteCharPtr( "#include \"" ).WriteCharPtr( aSlotIdFile.getString().getStr() ).WriteChar( '"' ) << endl << endl;
        }
        SvMetaExtern::Write( rBase, rOutStm, nTab, nT, nA );
        rOutStm << endl;
        WriteTab( rOutStm, nTab );
        rOutStm.WriteCharPtr( "library " ).WriteCharPtr( GetName().getString().getStr() ) << endl;
        WriteTab( rOutStm, nTab );
        rOutStm.WriteChar( '{' ) << endl;
        WriteTab( rOutStm, nTab );
        rOutStm.WriteCharPtr( "importlib(\"STDOLE.TLB\");" ) << endl;

        for( sal_uLong n = 0; n < aClassList.size(); n++ )
        {
            SvMetaClass * pClass = aClassList[n];
            if( !pClass->IsShell() && pClass->GetAutomation() )
            {
                WriteTab( rOutStm, nTab );
                WriteStars( rOutStm );
                pClass->Write( rBase, rOutStm, nTab +1, nT, nA );
                if( n +1 < aClassList.size() )
                    rOutStm << endl;
            }
        }

        rOutStm.WriteChar( '}' ) << endl;
    }
    break;
    case WRITE_DOCU:
    {
        rOutStm.WriteCharPtr( "SvIDL interface documentation" ) << endl << endl;
        rOutStm.WriteCharPtr( "<MODULE>" ) << endl;
        rOutStm.WriteCharPtr( GetName().getString().getStr() ) << endl;
        WriteDescription( rOutStm );
        rOutStm.WriteCharPtr( "</MODULE>" ) << endl << endl;

        rOutStm.WriteCharPtr( "<CLASSES>" ) << endl;
        for( sal_uLong n = 0; n < aClassList.size(); n++ )
        {
            SvMetaClass * pClass = aClassList[n];
            if( !pClass->IsShell() )
            {
                rOutStm.WriteCharPtr( pClass->GetName().getString().getStr() );
                SvMetaClass* pSC = pClass->GetSuperClass();
                if( pSC )
                    rOutStm.WriteCharPtr( " : " ).WriteCharPtr( pSC->GetName().getString().getStr() );

                
                const SvClassElementMemberList& rClassList = pClass->GetClassList();
                if ( !rClassList.empty() )
                {
                    rOutStm.WriteCharPtr( " ( " );

                    for( sal_uLong m=0; m<rClassList.size(); ++m )
                    {
                        SvClassElement *pEle = rClassList[m];
                        SvMetaClass *pCl = pEle->GetClass();
                        rOutStm.WriteCharPtr( pCl->GetName().getString().getStr() );
                        if ( m+1 == rClassList.size() )
                            rOutStm.WriteCharPtr( " )" );
                        else
                            rOutStm.WriteCharPtr( " , " );
                    }
                }

                rOutStm << endl;
            }
        }
        rOutStm.WriteCharPtr( "</CLASSES>" ) << endl << endl;
        
    }

    case WRITE_C_SOURCE:
    case WRITE_C_HEADER:
    {
        for( sal_uLong n = 0; n < aClassList.size(); n++ )
        {
            SvMetaClass * pClass = aClassList[n];
            if( !pClass->IsShell() )
                pClass->Write( rBase, rOutStm, nTab, nT, nA );
        }
    }
    break;

    default:
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
