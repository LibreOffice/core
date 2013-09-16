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
    bImported = sal_True; // import always
    SvMetaExtern::Load( rStm );

    sal_uInt16 nVer;

    rStm >> nVer; // version
    DBG_ASSERT( (nVer & ~IDL_WRITE_MASK) == MODULE_VER, "false version" );

    rStm >> aClassList;
    rStm >> aTypeList;
    rStm >> aAttrList;
    // browser
    aIdlFileName = rStm.ReadUniOrByteString( rStm.GetStreamCharSet() );
    aHelpFileName.setString(read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rStm));
    aSlotIdFile.setString(read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rStm));
    aModulePrefix.setString(read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rStm));

    // read compiler data
    sal_uInt16 nCmpLen;
    rStm >> nCmpLen;
    DBG_ASSERT( (nVer & IDL_WRITE_MASK) == IDL_WRITE_COMPILER,
                "no idl compiler format" );
    rStm >> aBeginName;
    rStm >> aEndName;
    rStm >> aNextName;
}

void SvMetaModule::Save( SvPersistStream & rStm )
{
    SvMetaExtern::Save( rStm );

    rStm << (sal_uInt16)(MODULE_VER | IDL_WRITE_COMPILER); // Version

    rStm << aClassList;
    rStm << aTypeList;
    rStm << aAttrList;
    // browser
    rStm.WriteUniOrByteString( aIdlFileName, rStm.GetStreamCharSet() );
    write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rStm, aHelpFileName.getString());
    write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rStm, aSlotIdFile.getString());
    write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rStm, aModulePrefix.getString());

    // write compiler data
    sal_uInt16 nCmpLen = 0;
    sal_uLong nLenPos = rStm.Tell();
    rStm << nCmpLen;
    rStm << aBeginName;
    rStm << aEndName;
    rStm << aNextName;
    // write length of compiler data
    sal_uLong nPos = rStm.Tell();
    rStm.Seek( nLenPos );
    rStm << (sal_uInt16)( nPos - nLenPos - sizeof( sal_uInt16 ) );
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
            OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM("cannot read file: "));
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
    if( !aTypeLibFile.getString().isEmpty() || !aSlotIdFile.getString().isEmpty() || !aTypeLibFile.getString().isEmpty() )
    {
        if( !aHelpFileName.getString().isEmpty() )
        {
            WriteTab( rOutStm, nTab );
            aHelpFileName.WriteSvIdl( SvHash_HelpFile(), rOutStm, nTab +1 );
            rOutStm << ';' << endl;
        }
        if( !aSlotIdFile.getString().isEmpty() )
        {
            WriteTab( rOutStm, nTab );
            aSlotIdFile.WriteSvIdl( SvHash_SlotIdFile(), rOutStm, nTab +1 );
            rOutStm << ';' << endl;
        }
        if( !aTypeLibFile.getString().isEmpty() )
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
            aClassList.push_back( aClass );
            // announce globally
            rBase.GetClassList().push_back( aClass );
        }
    }
    else if( rInStm.GetToken()->Is( SvHash_enum() ) )
    {
        SvMetaTypeEnumRef aEnum = new SvMetaTypeEnum();

        if( aEnum->ReadSvIdl( rBase, rInStm ) )
        {
            // declared in module
            aTypeList.push_back( aEnum );
            // announce globally
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
            // declared in module
            aTypeList.push_back( xItem );
            // announce globally
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
                    OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM("cannot open file: "));
                    aStr.append(OUStringToOString(aFullName, RTL_TEXTENCODING_UTF8));
                    rBase.SetError(aStr.makeStringAndClear(), pTok);
                }
            }
            else
            {
                OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM("cannot find file:"));
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
                // declared in module
                aAttrList.push_back( xSlot );
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
    bIsModified = sal_True; // up to now always when compiler running

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
    rOutStm << SvHash_module()->GetName().getStr() << endl
            << '\"';
    rOutStm.WriteUniOrByteString( aBeginName.GetHexName(), rOutStm.GetStreamCharSet() );
    rOutStm << '\"' << endl << '\"';
    rOutStm.WriteUniOrByteString( aEndName.GetHexName(), rOutStm.GetStreamCharSet() );
    rOutStm << '\"' << endl;
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
        rOutStm << "// class SvMetaModule" << endl;
        WriteTab( rOutStm, nTab );
        rOutStm << "helpfile(\"" << aHelpFileName.getString().getStr() << "\");" << endl;
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
            rOutStm << "#include \"" << aSlotIdFile.getString().getStr() << '"' << endl << endl;
        }
        SvMetaExtern::Write( rBase, rOutStm, nTab, nT, nA );
        rOutStm << endl;
        WriteTab( rOutStm, nTab );
        rOutStm << "library " << GetName().getString().getStr() << endl;
        WriteTab( rOutStm, nTab );
        rOutStm << '{' << endl;
        WriteTab( rOutStm, nTab );
        rOutStm << "importlib(\"STDOLE.TLB\");" << endl;

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

        rOutStm << '}' << endl;
    }
    break;
    case WRITE_DOCU:
    {
        rOutStm << "SvIDL interface documentation" << endl << endl;
        rOutStm << "<MODULE>" << endl << GetName().getString().getStr() << endl;
        WriteDescription( rOutStm );
        rOutStm << "</MODULE>" << endl << endl;

        rOutStm << "<CLASSES>" << endl;
        for( sal_uLong n = 0; n < aClassList.size(); n++ )
        {
            SvMetaClass * pClass = aClassList[n];
            if( !pClass->IsShell() )
            {
                rOutStm << pClass->GetName().getString().getStr();
                SvMetaClass* pSC = pClass->GetSuperClass();
                if( pSC )
                    rOutStm << " : " << pSC->GetName().getString().getStr();

                // imported classes
                const SvClassElementMemberList& rClassList = pClass->GetClassList();
                if ( !rClassList.empty() )
                {
                    rOutStm << " ( ";

                    for( sal_uLong m=0; m<rClassList.size(); ++m )
                    {
                        SvClassElement *pEle = rClassList[m];
                        SvMetaClass *pCl = pEle->GetClass();
                        rOutStm << pCl->GetName().getString().getStr();
                        if ( m+1 == rClassList.size() )
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
