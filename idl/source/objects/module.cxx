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


SvMetaModule::SvMetaModule( bool bImp )
    : bImported( bImp ), bIsModified( false )
{
}

bool SvMetaModule::SetName( const OString& rName, SvIdlDataBase * pBase )
{
    if( pBase )
    {
        if( pBase->GetModule( rName ) )
            return false;
    }
    return SvMetaExtern::SetName( rName );
}

bool SvMetaModule::FillNextName( SvGlobalName * pName )
{
    *pName = aNextName;

    if( aNextName < aEndName )
    {
        ++aNextName;
        bIsModified = true;
        return true;
    }
    return false;
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

void SvMetaModule::ReadContextSvIdl( SvIdlDataBase & rBase,
                                     SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    if( rInStm.GetToken().Is( SvHash_interface() )
      || rInStm.GetToken().Is( SvHash_shell() ) )
    {
        SvMetaClassRef aClass = new SvMetaClass();
        if( aClass->ReadSvIdl( rBase, rInStm ) )
        {
            aClassList.push_back( aClass );
            // announce globally
            rBase.GetClassList().push_back( aClass );
        }
    }
    else if( rInStm.GetToken().Is( SvHash_enum() ) )
    {
        tools::SvRef<SvMetaTypeEnum> aEnum = new SvMetaTypeEnum();

        if( aEnum->ReadSvIdl( rBase, rInStm ) )
        {
            // declared in module
            aTypeList.push_back( aEnum );
            // announce globally
            rBase.GetTypeList().push_back( aEnum );
        }
    }
    else if( rInStm.GetToken().Is( SvHash_item() )
      || rInStm.GetToken().Is( SvHash_struct() )
      || rInStm.GetToken().Is( SvHash_typedef() ) )
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
    else if( rInStm.GetToken().Is( SvHash_include() ) )
    {
        bool bOk = false;
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
                    bOk = aTokStm.GetToken().IsEof();
                    if( !bOk )
                    {
                        rBase.WriteError( aTokStm );
                    }
                    // recover error from old file
                    rBase.SetError( aOldErr );
                }
                else
                {
                    OStringBuffer aStr("cannot open file: ");
                    aStr.append(OUStringToOString(aFullName, RTL_TEXTENCODING_UTF8));
                    rBase.SetError(aStr.makeStringAndClear(), *pTok);
                }
            }
            else
            {
                OStringBuffer aStr("cannot find file:");
                aStr.append(OUStringToOString(aFullName, RTL_TEXTENCODING_UTF8));
                rBase.SetError(aStr.makeStringAndClear(), *pTok);
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

bool SvMetaModule::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    bIsModified = true; // up to now always when compiler running

    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok  = rInStm.GetToken_Next();
    bool bOk = pTok->Is( SvHash_module() );
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
            bOk = SvMetaObject::ReadSvIdl( rBase, rInStm );
        }
        rBase.GetStack().Pop(); // remove from stack
    }
    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

void SvMetaModule::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    for( sal_uLong n = 0; n < aClassList.size(); n++ )
    {
        SvMetaClass * pClass = aClassList[n];
        pClass->WriteSfx( rBase, rOutStm );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
