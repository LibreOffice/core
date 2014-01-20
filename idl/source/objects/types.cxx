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

#include <tools/debug.hxx>

#include <types.hxx>
#include <globals.hxx>
#include <database.hxx>

SV_IMPL_META_FACTORY1( SvMetaAttribute, SvMetaReference );
SvMetaAttribute::SvMetaAttribute()
    : aAutomation( sal_True, sal_False )
    , aExport( sal_True, sal_False )
    , aIsCollection ( sal_False, sal_False )
    , aReadOnlyDoc ( sal_True, sal_False )
    , aHidden( sal_False, sal_False )
    , bNewAttr( sal_False )
{
}

SvMetaAttribute::SvMetaAttribute( SvMetaType * pType )
    : aType( pType )
    , aAutomation( sal_True, sal_False )
    , aExport( sal_True, sal_False )
    , aIsCollection ( sal_False, sal_False)
    , aReadOnlyDoc ( sal_True, sal_False)
    , aHidden( sal_False, sal_False )
    , bNewAttr( sal_False )
{
}

void SvMetaAttribute::Load( SvPersistStream & rStm )
{
    SvMetaReference::Load( rStm );

    sal_uInt8 nMask;
    rStm >> nMask;
    if( nMask & 0x01 )
    {
        SvMetaType * pType;
        rStm >> pType;
        aType = pType;
    }
    if( nMask & 0x02 )  rStm >> aSlotId;
    if( nMask & 0x04 )  rStm >> aExport;
    if( nMask & 0x08 )  rStm >> aReadonly;
    if( nMask & 0x10 )  rStm >> aAutomation;
    if( nMask & 0x20 )  rStm >> aIsCollection;
    if( nMask & 0x40 )  rStm >> aReadOnlyDoc;
    if( nMask & 0x80 )  rStm >> aHidden;
}

void SvMetaAttribute::Save( SvPersistStream & rStm )
{
    SvMetaReference::Save( rStm );

    // create mask
    sal_uInt8 nMask = 0;
    if( aType.Is() )            nMask |= 0x1;
    if( aSlotId.IsSet() )       nMask |= 0x2;
    if( aExport.IsSet() )       nMask |= 0x4;
    if( aReadonly.IsSet() )     nMask |= 0x8;
    if( aAutomation.IsSet() )   nMask |= 0x10;
    if( aIsCollection.IsSet() ) nMask |= 0x20;
    if( aReadOnlyDoc.IsSet() )  nMask |= 0x40;
    if( aHidden.IsSet() )       nMask |= 0x80;

    // write data
    rStm.WriteUChar( nMask );
    if( nMask & 0x1 )   WriteSvPersistBase( rStm, aType );
    if( nMask & 0x2 )   WriteSvNumberIdentifier( rStm, aSlotId );
    if( nMask & 0x4 )   rStm.WriteUChar( aExport );
    if( nMask & 0x8 )   rStm.WriteUChar( aReadonly );
    if( nMask & 0x10 )  rStm.WriteUChar( aAutomation );
    if( nMask & 0x20 )  rStm.WriteUChar( aIsCollection );
    if( nMask & 0x40 )  rStm.WriteUChar( aReadOnlyDoc );
    if( nMask & 0x80 )  rStm.WriteUChar( aHidden );
}

SvMetaType * SvMetaAttribute::GetType() const
{
    if( aType.Is() || !GetRef() ) return aType;
    return ((SvMetaAttribute *)GetRef())->GetType();
}

const SvNumberIdentifier & SvMetaAttribute::GetSlotId() const
{
    if( aSlotId.IsSet() || !GetRef() ) return aSlotId;
    return ((SvMetaAttribute *)GetRef())->GetSlotId();
}

sal_Bool SvMetaAttribute::GetReadonly() const
{
    if( aReadonly.IsSet() || !GetRef() ) return aReadonly;
    return ((SvMetaAttribute *)GetRef())->GetReadonly();
}

sal_Bool SvMetaAttribute::GetExport() const
{
    if( aExport.IsSet() || !GetRef() ) return aExport;
    return ((SvMetaAttribute *)GetRef())->GetExport();
}

sal_Bool SvMetaAttribute::GetHidden() const
{
    // when export is set, but hidden is not the default is used
    if ( aExport.IsSet() && !aHidden.IsSet() )
        return !aExport;
    else if( aHidden.IsSet() || !GetRef() )
        return aHidden;
    else
        return ((SvMetaAttribute *)GetRef())->GetHidden();
}

sal_Bool SvMetaAttribute::GetAutomation() const
{
    if( aAutomation.IsSet() || !GetRef() ) return aAutomation;
    return ((SvMetaAttribute *)GetRef())->GetAutomation();
}

sal_Bool SvMetaAttribute::GetIsCollection() const
{
    sal_Bool bRet;
    if( aIsCollection.IsSet() || !GetRef() )
    {
        if ( aIsCollection.IsSet() )
        {
            bRet = aIsCollection;
            return bRet;
        }

        return aIsCollection;
    }

    return ((SvMetaSlot *)GetRef())->GetIsCollection();
}

sal_Bool SvMetaAttribute::GetReadOnlyDoc() const
{
    if( aReadOnlyDoc.IsSet() || !GetRef() ) return aReadOnlyDoc;
    return ((SvMetaSlot *)GetRef())->GetReadOnlyDoc();
}

sal_Bool SvMetaAttribute::IsMethod() const
{
    SvMetaType * pType = GetType();
    DBG_ASSERT( pType, "no type for attribute" );
    return pType->GetType() == TYPE_METHOD;
}

sal_Bool SvMetaAttribute::IsVariable() const
{
    SvMetaType * pType = GetType();
    return pType->GetType() != TYPE_METHOD;
}

OString SvMetaAttribute::GetMangleName( sal_Bool ) const
{
    return GetName().getString();
}

sal_Bool SvMetaAttribute::Test( SvIdlDataBase & rBase,
                            SvTokenStream & rInStm )
{
    sal_Bool bOk = sal_True;
    if( GetType()->IsItem() && !GetSlotId().IsSet() )
    {
        rBase.SetError( "slot without id declared", rInStm.GetToken() );
        rBase.WriteError( rInStm );
        bOk = sal_False;
    }
    return bOk;
}

sal_Bool SvMetaAttribute::ReadSvIdl( SvIdlDataBase & rBase,
                                     SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos     = rInStm.Tell();
    if( !GetType() )
        // no type in ctor passed on
        aType = rBase.ReadKnownType( rInStm );
    sal_Bool bOk = sal_False;
    if( GetType() )
    {
        ReadNameSvIdl( rBase, rInStm );
        aSlotId.ReadSvIdl( rBase, rInStm );

        bOk = sal_True;
        SvToken * pTok  = rInStm.GetToken();
        if( bOk && pTok->IsChar() && pTok->GetChar() == '(' )
        {
            SvMetaTypeRef xT = new SvMetaType();
            xT->SetRef( GetType() );
            aType = xT;
            bOk = aType->ReadMethodArgs( rBase, rInStm );
        }
        if( bOk )
            bOk = SvMetaName::ReadSvIdl( rBase, rInStm );
    }
    else
    {
        SvToken *pTok = rInStm.GetToken();
        rBase.SetError( "unknown type of token. Each new SID needs an "
                        "item statement in an SDI file, eg. "
                        "SfxVoidItem FooItem  " + pTok->GetTokenAsString() +
                        " ... which describes the slot more fully", pTok );
    }

    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

void SvMetaAttribute::WriteSvIdl
(
    SvIdlDataBase & rBase,
    SvStream & rOutStm,
    sal_uInt16 nTab
)
{
    SvMetaType * pType = GetType();
    pType->WriteTypePrefix( rBase, rOutStm, nTab, WRITE_IDL );
    rOutStm.WriteChar( ' ' ).WriteCharPtr( GetName().getString().getStr() );
    if( aSlotId.IsSet() )
        rOutStm.WriteChar( ' ' ).WriteCharPtr( aSlotId.getString().getStr() );
    if( pType->GetType() == TYPE_METHOD )
        pType->WriteMethodArgs( rBase, rOutStm, nTab, WRITE_IDL );
    sal_uLong nPos = rOutStm.Tell();
    rOutStm << endl;
    SvMetaName::WriteSvIdl( rBase, rOutStm, nTab );
    TestAndSeekSpaceOnly( rOutStm, nPos );
}

void SvMetaAttribute::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                             SvTokenStream & rInStm )
{
    SvMetaReference::ReadAttributesSvIdl( rBase, rInStm );
    aSlotId.ReadSvIdl( rBase, SvHash_SlotId(), rInStm );
    aExport.ReadSvIdl( SvHash_Export(), rInStm );
    aHidden.ReadSvIdl( SvHash_Hidden(), rInStm );
    aAutomation.ReadSvIdl( SvHash_Automation(), rInStm );
    aIsCollection.ReadSvIdl( SvHash_IsCollection(), rInStm );
    aReadOnlyDoc.ReadSvIdl( SvHash_ReadOnlyDoc(), rInStm );
    if( aReadonly.ReadSvIdl( SvHash_Readonly(), rInStm ) )
    {
        if( GetType()->GetType() == TYPE_METHOD )
        {
            // set error
            rBase.SetError( "Readonly in function attribute", rInStm.GetToken() );
            rBase.WriteError( rInStm );
        }
    }
}

void SvMetaAttribute::WriteAttributesSvIdl
(
    SvIdlDataBase & rBase,
    SvStream & rOutStm,
    sal_uInt16 nTab
)
{
    SvMetaReference::WriteAttributesSvIdl( rBase, rOutStm, nTab );

    if( !aExport || !aAutomation || aReadonly )
    {
        WriteTab( rOutStm, nTab );
        rOutStm.WriteCharPtr( "//class SvMetaAttribute" ) << endl;
        if( !aExport )
        {
            WriteTab( rOutStm, nTab );
            aExport.WriteSvIdl( SvHash_Export(), rOutStm );
            rOutStm.WriteChar( ';' ) << endl;
        }
        if ( aHidden != aExport )
        {
            WriteTab( rOutStm, nTab );
            aExport.WriteSvIdl( SvHash_Hidden(), rOutStm );
            rOutStm.WriteChar( ';' ) << endl;
        }
        if( aReadonly )
        {
            WriteTab( rOutStm, nTab );
            aReadonly.WriteSvIdl( SvHash_Readonly(), rOutStm );
            rOutStm.WriteChar( ';' ) << endl;
        }
        if( !aAutomation )
        {
            WriteTab( rOutStm, nTab );
            aAutomation.WriteSvIdl( SvHash_Automation(), rOutStm );
            rOutStm.WriteChar( ';' ) << endl;
        }
        if( aIsCollection )
        {
            WriteTab( rOutStm, nTab );
            aIsCollection.WriteSvIdl( SvHash_IsCollection(), rOutStm );
            rOutStm.WriteChar( ';' ) << endl;
        }
        if( !aReadOnlyDoc )
        {
            WriteTab( rOutStm, nTab );
            aReadOnlyDoc.WriteSvIdl( SvHash_ReadOnlyDoc(), rOutStm );
            rOutStm.WriteChar( ';' ) << endl;
        }
    }
}

void SvMetaAttribute::WriteParam( SvIdlDataBase & rBase,
                                    SvStream & rOutStm,
                                    sal_uInt16 nTab,
                                    WriteType nT )
{
    SvMetaType * pType = GetType();
    DBG_ASSERT( pType, "no type for attribute" );
    SvMetaType * pBaseType = pType->GetBaseType();
    DBG_ASSERT( pBaseType, "no base type for attribute" );

    if( nT == WRITE_ODL || nT == WRITE_DOCU
       || nT == WRITE_C_HEADER || nT == WRITE_C_SOURCE )
    {
        if( pBaseType->GetType() == TYPE_STRUCT )
        {
            const SvMetaAttributeMemberList & rList = pBaseType->GetAttrList();
            sal_uLong nCount = rList.size();
            for( sal_uLong i = 0; i < nCount; i++ )
            {
                rList[i]->WriteParam( rBase, rOutStm, nTab, nT );
                if( i+1<nCount )
                {
                        rOutStm.WriteChar( ',' );
                }
            }
        }
        else
        {
            if ( nT != WRITE_DOCU )
            {
                WriteTab( rOutStm, nTab );
                pBaseType->WriteTypePrefix( rBase, rOutStm, nTab, nT );
            }

            if( !GetName().getString().isEmpty() )
            {
                rOutStm.WriteChar( ' ' );
                rOutStm.WriteCharPtr( GetName().getString().getStr() );
            }

            if ( nT == WRITE_DOCU )
            {
                if( pBaseType->GetType() == TYPE_METHOD ||
                     pBaseType->GetType() == TYPE_STRUCT ||
                    pBaseType->GetType() == TYPE_ENUM )
                {
                    OSL_FAIL( "Falscher Parametertyp!" );
                }
                else
                    rOutStm.WriteCharPtr( pBaseType->GetBasicPostfix().getStr() );
            }
        }
    }
}

sal_uLong SvMetaAttribute::MakeSlotValue( SvIdlDataBase & rBase, sal_Bool bVar ) const
{
    const SvNumberIdentifier & rId = GetSlotId();
    sal_uLong n = rId.GetValue();
    if( !rBase.aStructSlotId.getString().isEmpty() )
    {
        n = n << 20;
        n += rBase.aStructSlotId.GetValue();
    }
    if( PTR_CAST( SvMetaSlot, this ) )
        n |= 0x20000;
    if( !bVar )
        n += 0x10000;
    else if ( GetIsCollection() )
        n += 0x40000;
    return n;
}

void SvMetaAttribute::WriteAttributes( SvIdlDataBase & rBase, SvStream & rOutStm,
                                      sal_uInt16 nTab,
                                      WriteType nT, WriteAttribute nA )
{
    if( nT == WRITE_ODL  )
    {
        const SvNumberIdentifier & rId = GetSlotId();
        sal_Bool bReadonly = GetReadonly() || ( nA & WA_READONLY );
        if( (rId.IsSet() && !(nA & WA_STRUCT)) || bReadonly )
        {
            sal_Bool bVar = IsVariable();
            if( nA & WA_VARIABLE )
                bVar = sal_True;
            else if( nA & WA_METHOD )
                bVar = sal_False;

            WriteTab( rOutStm, nTab );
            rOutStm.WriteCharPtr( "//class SvMetaAttribute" ) << endl;
            if( rId.IsSet() && !(nA & WA_STRUCT) )
            {
                WriteTab( rOutStm, nTab );
                rOutStm.WriteCharPtr( "id(" )
                   .WriteCharPtr( OString::number(MakeSlotValue(rBase,bVar)).getStr() )
                   .WriteCharPtr( ")," ) << endl;
            }
            if( bVar && (bReadonly || IsMethod()) )
            {
                WriteTab( rOutStm, nTab );
                rOutStm.WriteCharPtr( "readonly," ) << endl;
            }
        }
    }
}

void SvMetaAttribute::WriteCSource( SvIdlDataBase & rBase, SvStream & rOutStm,
                                    sal_Bool bSet )
{
    rOutStm << endl;
    SvMetaType * pType = GetType();
    SvMetaType * pBaseType = pType->GetBaseType();

    // for Set the return is always void
    sal_Bool bVoid = bSet;
    if( pBaseType->GetType() == TYPE_METHOD )
        bVoid = pBaseType->GetReturnType()->GetBaseType()->GetName().getString() == "void";

    // emit methods/functions body
    rOutStm.WriteChar( '{' ) << endl;
    WriteTab( rOutStm, 1 );

    if( !bVoid )
    {
        if ( pBaseType->GetCName() == "double" )
        {
            rOutStm.WriteCharPtr( "return *(double*)" );
        }
        else
        {
            rOutStm.WriteCharPtr( "return (" );
            pType->WriteTypePrefix( rBase, rOutStm, 2, WRITE_C_SOURCE );
            rOutStm.WriteCharPtr( ") " );
        }
    }
    rOutStm.WriteCharPtr( "pODKCallFunction( " )
       .WriteCharPtr( OString::number(MakeSlotValue(rBase, IsVariable())).getStr() );
    rOutStm.WriteChar( ',' ) << endl;
    WriteTab( rOutStm, 3 );
    rOutStm.WriteCharPtr( " h" ).WriteCharPtr( rBase.aIFaceName.getStr() ).WriteCharPtr( " , " );

    OString aParserStr;
    if( pBaseType->GetType() == TYPE_METHOD || bSet )
        aParserStr = pBaseType->GetParserString();
    if( !aParserStr.isEmpty() )
    {
        rOutStm.WriteChar( '\"' );
        rOutStm.WriteCharPtr( aParserStr.getStr() );
        rOutStm.WriteCharPtr( "\", " );
    }
    else
        rOutStm.WriteCharPtr( "NULL, " );

    if( pBaseType->GetType() == TYPE_METHOD && !bVoid )
    {
        rOutStm.WriteCharPtr( "'" );
        rOutStm.WriteChar( pBaseType->GetReturnType()->GetBaseType()->GetParserChar() );
        rOutStm.WriteCharPtr( "'" );
    }
    else if ( !bSet )
    {
        rOutStm.WriteCharPtr( "'" );
        rOutStm.WriteChar( pBaseType->GetParserChar() );
        rOutStm.WriteCharPtr( "'" );
    }
    else
        rOutStm.WriteChar( '0' );

    if( !aParserStr.isEmpty() )
    {
        rOutStm.WriteCharPtr( ", " );
        if( IsMethod() )
            pBaseType->WriteParamNames( rBase, rOutStm, OString() );
        else if( bSet )
            pBaseType->WriteParamNames( rBase, rOutStm, GetName().getString() );
    }

    rOutStm.WriteCharPtr( " );" ) << endl;
    rOutStm.WriteChar( '}' ) << endl;
}

void SvMetaAttribute::WriteRecursiv_Impl( SvIdlDataBase & rBase,
                            SvStream & rOutStm, sal_uInt16 nTab,
                             WriteType nT, WriteAttribute nA )
{
    const SvMetaAttributeMemberList & rList = GetType()->GetBaseType()->GetAttrList();
    sal_uLong nCount = rList.size();

    SvNumberIdentifier slotId = rBase.aStructSlotId;
    if ( !GetSlotId().getString().isEmpty() )
        rBase.aStructSlotId = GetSlotId();

    // offial hack interface by MM: special controls get passed with the WriteAttribute
    if ( GetReadonly() )
        nA |= WA_READONLY;

    for( sal_uLong i = 0; i < nCount; i++ )
    {
        SvMetaAttribute *pAttr = rList[i];
        if ( nT == WRITE_DOCU )
            pAttr->SetDescription( GetDescription().getString() );
        pAttr->Write( rBase, rOutStm, nTab, nT, nA );
        if( nT == WRITE_ODL && i +1 < nCount )
            rOutStm.WriteChar( ';' ) << endl;
    }

    rBase.aStructSlotId = slotId;
}

void SvMetaAttribute::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                            sal_uInt16 nTab,
                             WriteType nT, WriteAttribute nA )
{
    // no attributes for automation
    if( nT == WRITE_DOCU )
    {
        if ( GetHidden() )
            return;
    }
    else if ( !GetAutomation() || !GetExport() )
        return;

    sal_Bool bVariable;
    if( nA & WA_VARIABLE )
        bVariable = sal_True;
    else if( nA & WA_METHOD )
        bVariable = sal_False;
    else
        bVariable = IsVariable();

    SvMetaType * pType = GetType();
    DBG_ASSERT( pType, "no type for attribute" );
    SvMetaType * pBaseType = pType->GetBaseType();
    DBG_ASSERT( pBaseType, "no base type for attribute" );
    int nBType = pBaseType->GetType();

    if( nT == WRITE_ODL )
    {
        if( (bVariable && IsVariable()) || (!bVariable && IsMethod()) )
        {
            if( nBType == TYPE_STRUCT )
                WriteRecursiv_Impl( rBase, rOutStm, nTab, nT, nA );
            else
            {
                SvMetaReference::Write( rBase, rOutStm, nTab, nT, nA );
                WriteTab( rOutStm, nTab );
                pBaseType->WriteTypePrefix( rBase, rOutStm, nTab, nT );

                if( !GetName().getString().isEmpty() )
                {
                    rOutStm.WriteChar( ' ' );
                    rOutStm.WriteCharPtr( GetName().getString().getStr() );
                }
                if( pType->GetType() == TYPE_METHOD )
                    pType->WriteMethodArgs( rBase, rOutStm, nTab, nT );
            }
        }
    }
    else if( nT == WRITE_C_HEADER || nT == WRITE_C_SOURCE )
    {
        if( !bVariable && IsMethod() )
        {
            OString name = rBase.aIFaceName + GetName().getString();
            const char * pName = name.getStr();
            WriteTab( rOutStm, nTab );
            pBaseType->WriteTypePrefix( rBase, rOutStm, nTab, nT );
            rOutStm.WriteChar( ' ' ).WriteCharPtr( pName );
            pType->WriteMethodArgs( rBase, rOutStm, nTab, nT );
            if( nT == WRITE_C_HEADER )
                rOutStm.WriteChar( ';' ) << endl << endl;
            else
                WriteCSource( rBase, rOutStm, sal_False );
        }
        else if ( bVariable && IsVariable() )
        {
              if( nBType == TYPE_STRUCT )
            {
                // for assistance emit the name of the property as acomment
                rOutStm.WriteCharPtr( "/* " ).WriteCharPtr( GetName().getString().getStr() ).WriteCharPtr( " */" ) << endl;

                WriteRecursiv_Impl( rBase, rOutStm, nTab, nT, nA );
            }
            else
            {
                OString name = GetName().getString();

                sal_Bool bReadonly = GetReadonly() || ( nA & WA_READONLY );
                if ( !bReadonly && !IsMethod() )
                {
                    // allocation
                    WriteTab( rOutStm, nTab );
                    rOutStm.WriteCharPtr( "void " );
                    rOutStm.WriteCharPtr( rBase.aIFaceName.getStr() )
                           .WriteCharPtr( "Set" ).WriteCharPtr( name.getStr() ).WriteCharPtr( "( " ).WriteCharPtr( C_PREF )
                           .WriteCharPtr( "Object h" ).WriteCharPtr( rBase.aIFaceName.getStr() ).WriteCharPtr( ", " ) << endl;
                    WriteTab( rOutStm, nTab+1 );
                    pBaseType->WriteTypePrefix( rBase, rOutStm, nTab, nT );
                    rOutStm.WriteChar( ' ' ).WriteCharPtr( name.getStr() ).WriteCharPtr( " )" );
                    if( nT == WRITE_C_HEADER )
                        rOutStm.WriteChar( ';' ) << endl << endl;
                    else
                        WriteCSource( rBase, rOutStm, sal_True );
                }

                // access
                WriteTab( rOutStm, nTab );
                pBaseType->WriteTypePrefix( rBase, rOutStm, nTab, nT );
                rOutStm.WriteChar( ' ' );
                rOutStm.WriteCharPtr( rBase.aIFaceName.getStr() )
                       .WriteCharPtr( "Get" ).WriteCharPtr( name.getStr() ).WriteCharPtr( "( " ).WriteCharPtr( C_PREF )
                       .WriteCharPtr( "Object h" ).WriteCharPtr( rBase.aIFaceName.getStr() ).WriteCharPtr( " )" );
                if( nT == WRITE_C_HEADER )
                    rOutStm.WriteChar( ';' ) << endl << endl;
                else
                    WriteCSource( rBase, rOutStm, sal_False );
            }
        }
    }
    else if ( nT == WRITE_DOCU )
    {
        if( !bVariable && IsMethod() )
        {
            rOutStm.WriteCharPtr( "<METHOD>" ) << endl;
            rOutStm.WriteCharPtr( GetSlotId().getString().getStr() ) << endl;
            rOutStm.WriteCharPtr( GetName().getString().getStr() ) << endl
                    << endl;    // readonly

            // return type
            SvMetaType* pType2 = GetType();
            SvMetaType* pBaseType2 = pType2->GetBaseType();
            rOutStm.WriteCharPtr( pBaseType2->GetReturnType()->GetBaseType()->GetBasicName().getStr() ) << endl;

            DBG_ASSERT( !pBaseType2->GetReturnType()->GetBaseType()->GetBasicName().isEmpty(),
                "Leerer BasicName" );

            // syntax
            rOutStm.WriteCharPtr( GetName().getString().getStr() );
            pType2->WriteMethodArgs( rBase, rOutStm, nTab, nT );

            // C return type
            pBaseType2->WriteTypePrefix( rBase, rOutStm, 0, WRITE_C_HEADER );
            rOutStm << endl;

            // for methods also C syntax
            rOutStm.WriteCharPtr( "<C-SYNTAX>" ) << endl;
            Write( rBase, rOutStm, 0, WRITE_C_HEADER, nA );
            rOutStm.WriteCharPtr( "</C-SYNTAX>" ) << endl;

            // description
            WriteDescription( rOutStm );
            rOutStm.WriteCharPtr( "</METHOD>" ) << endl << endl;
        }
        else if( bVariable && IsVariable() )
        {
            if( nBType == TYPE_STRUCT )
            {
                WriteRecursiv_Impl( rBase, rOutStm, nTab, nT, nA );
            }
            else
            {
                rOutStm.WriteCharPtr( "<PROPERTY>" ) << endl;
                rOutStm.WriteCharPtr( GetSlotId().getString().getStr() ) << endl;
                rOutStm.WriteCharPtr( GetName().getString().getStr() ) << endl;
                if ( GetReadonly() )
                    rOutStm.WriteCharPtr( "(nur lesen)" ) << endl;
                else
                    rOutStm << endl;

                // for properties type instead of the return value
                rOutStm.WriteCharPtr( pBaseType->GetBasicName().getStr() ) << endl;

                DBG_ASSERT( !pBaseType->GetBasicName().isEmpty(),
                    "Leerer BasicName" );

                // for properties no syntax
                rOutStm << endl;

                // C return type
                pBaseType->WriteTypePrefix( rBase, rOutStm, 0, WRITE_C_HEADER );
                rOutStm << endl;

                // description
                WriteDescription( rOutStm );
                rOutStm.WriteCharPtr( "</PROPERTY>" ) << endl << endl;
            }
        }
    }
}

sal_uLong SvMetaAttribute::MakeSfx( OStringBuffer& rAttrArray )
{
    SvMetaType * pType = GetType();
    DBG_ASSERT( pType, "no type for attribute" );
    SvMetaType * pBaseType = pType->GetBaseType();
    DBG_ASSERT( pBaseType, "no base type for attribute" );
    if( pBaseType->GetType() == TYPE_STRUCT )
        return pBaseType->MakeSfx( rAttrArray );
    else
    {
        rAttrArray.append('{');
        rAttrArray.append(GetSlotId().getString());
        rAttrArray.append(",\"");
        rAttrArray.append(GetName().getString());
        rAttrArray.append("\"}");
        return 1;
    }
}

void SvMetaAttribute::Insert (SvSlotElementList&, const OString&, SvIdlDataBase&)
{
}

void SvMetaAttribute::WriteHelpId( SvIdlDataBase &, SvStream &, HelpIdTable& )
{
}

SV_IMPL_META_FACTORY1( SvMetaType, SvMetaExtern );
#define CTOR                            \
    : aCall0( CALL_VALUE, sal_False )       \
    , aCall1( CALL_VALUE, sal_False )       \
    , aSbxDataType( 0, sal_False )          \
    , pAttrList( NULL )                 \
    , nType( TYPE_BASE )                \
    , bIsItem( sal_False )                  \
    , bIsShell( sal_False )                 \
    , cParserChar( 'h' )

SvMetaType::SvMetaType()
    CTOR
{
}

SvMetaType::SvMetaType( const OString& rName, char cPC,
                        const OString& rCName )
    CTOR
{
    SetName( rName );
    cParserChar = cPC;
    aCName.setString(rCName);
}

SvMetaType::SvMetaType( const OString& rName,
                        const OString& rSbxName,
                        const OString& rOdlName,
                        char cPc,
                        const OString& rCName,
                        const OString& rBasicName,
                        const OString& rBasicPostfix )
    CTOR
{
    SetName( rName );
    aSbxName.setString(rSbxName);
    aOdlName.setString(rOdlName);
    cParserChar = cPc;
    aCName.setString(rCName);
    aBasicName.setString(rBasicName);
    aBasicPostfix.setString(rBasicPostfix);
}

void SvMetaType::Load( SvPersistStream & rStm )
{
    SvMetaExtern::Load( rStm );

    sal_uInt16 nMask;
    rStm >> nMask;
    if( nMask & 0x0001 ) rStm >> aIn;
    if( nMask & 0x0002 ) rStm >> aOut;
    if( nMask & 0x0004 ) rStm >> aCall0;
    if( nMask & 0x0008 ) rStm >> aCall1;
    if( nMask & 0x0010 ) rStm >> aSbxDataType;
    if( nMask & 0x0020 ) rStm >> aSvName;
    if( nMask & 0x0040 ) rStm >> aSbxName;
    if( nMask & 0x0080 ) rStm >> aOdlName;
    if( nMask & 0x0100 ) rStm >> GetAttrList();
    if( nMask & 0x0200 ) bIsItem = sal_True;
    if( nMask & 0x0400 ) bIsShell = sal_True;
    if( nMask & 0x0800 )
    {
        sal_uInt16 nT;
        rStm >> nT;
        nType = nT;
    }
    if( nMask & 0x1000 ) rStm >> cParserChar;
    if( nMask & 0x2000 ) rStm >> aCName;
    if( nMask & 0x4000 ) rStm >> aBasicName;
    if( nMask & 0x8000 ) rStm >> aBasicPostfix;
}

void SvMetaType::Save( SvPersistStream & rStm )
{
    SvMetaExtern::Save( rStm );

    // create mask
    sal_uInt16 nMask = 0;
    if( aIn.IsSet() )               nMask |= 0x0001;
    if( aOut.IsSet() )              nMask |= 0x0002;
    if( aCall0.IsSet() )            nMask |= 0x0004;
    if( aCall1.IsSet() )            nMask |= 0x0008;
    if( aSbxDataType.IsSet() )      nMask |= 0x0010;
    if( aSvName.IsSet() )           nMask |= 0x0020;
    if( aSbxName.IsSet() )          nMask |= 0x0040;
    if( aOdlName.IsSet() )          nMask |= 0x0080;
    if( GetAttrCount() )            nMask |= 0x0100;
    if( bIsItem )                   nMask |= 0x0200;
    if( bIsShell )                  nMask |= 0x0400;
    if( nType != TYPE_BASE )        nMask |= 0x0800;
    if( cParserChar != 'h' )        nMask |= 0x1000;
    if( aCName.IsSet() )            nMask |= 0x2000;
    if( aBasicName.IsSet() )        nMask |= 0x4000;
    if( aBasicPostfix.IsSet() )     nMask |= 0x8000;

    // write data
    rStm.WriteUInt16( nMask );
    if( nMask & 0x0001 ) WriteSvBOOL( rStm, aIn );
    if( nMask & 0x0002 ) WriteSvBOOL( rStm, aOut );
    if( nMask & 0x0004 ) WriteSvint( rStm, aCall0 );
    if( nMask & 0x0008 ) WriteSvint( rStm, aCall1 );
    if( nMask & 0x0010 ) WriteSvint( rStm, aSbxDataType );
    if( nMask & 0x0020 ) WriteSvIdentifier( rStm, aSvName );
    if( nMask & 0x0040 ) WriteSvIdentifier( rStm, aSbxName );
    if( nMask & 0x0080 ) WriteSvIdentifier( rStm, aOdlName );
    if( nMask & 0x0100 ) WriteSvDeclPersistList( rStm, *pAttrList );
    if( nMask & 0x0800 ) rStm.WriteUInt16( (sal_uInt16)nType );
    if( nMask & 0x1000 ) rStm.WriteChar( cParserChar );
    if( nMask & 0x2000 ) WriteSvIdentifier( rStm, aCName );
    if( nMask & 0x4000 ) WriteSvIdentifier( rStm, aBasicName );
    if( nMask & 0x8000 ) WriteSvIdentifier( rStm, aBasicPostfix );
}

SvMetaAttributeMemberList & SvMetaType::GetAttrList() const
{
    if( !pAttrList )
        ((SvMetaType *)this)->pAttrList = new SvMetaAttributeMemberList();
    return *pAttrList;
}

void SvMetaType::SetType( int nT )
{
    nType = nT;
    if( nType == TYPE_ENUM )
    {
        aOdlName.setString("short");
    }
    else if( nType == TYPE_CLASS )
    {
        OStringBuffer aTmp(C_PREF);
        aTmp.append("Object *");
        aCName.setString(aTmp.makeStringAndClear());
    }
}

SvMetaType * SvMetaType::GetBaseType() const
{
    if( GetRef() && GetType() == TYPE_BASE )
        return ((SvMetaType *)GetRef())->GetBaseType();
    return (SvMetaType *)this;
}

SvMetaType * SvMetaType::GetReturnType() const
{
    DBG_ASSERT( GetType() == TYPE_METHOD, "no method" );
    DBG_ASSERT( GetRef(), "no return type" );
    return (SvMetaType *)GetRef();
}

const OString& SvMetaType::GetBasicName() const
{
    if( aBasicName.IsSet() || !GetRef() )
        return aBasicName.getString();
    else
        return ((SvMetaType*)GetRef())->GetBasicName();
}

OString SvMetaType::GetBasicPostfix() const
{
    // MBN and Co always want "As xxx"
    return OStringBuffer(" As ").
        append(GetBasicName()).
        makeStringAndClear();
}

sal_Bool SvMetaType::GetIn() const
{
    if( aIn.IsSet() || !GetRef() )
        return aIn;
    else
        return ((SvMetaType *)GetRef())->GetIn();
}

sal_Bool SvMetaType::GetOut() const
{
    if( aOut.IsSet() || !GetRef() )
        return aOut;
    else
        return ((SvMetaType *)GetRef())->GetOut();
}

void SvMetaType::SetCall0( int e )
{
    aCall0 = (int)e;
    if( aCall0 == CALL_VALUE && aCall1 == CALL_VALUE )
    {
          if( GetType() == TYPE_POINTER )
            SetType( TYPE_BASE );
    }
    else
    {
        DBG_ASSERT( nType == TYPE_POINTER || nType == TYPE_BASE,
                    "set no base type to pointer" );
        SetType( TYPE_POINTER );
    }
}

int SvMetaType::GetCall0() const
{
    if( aCall0.IsSet() || !GetRef() )
        return aCall0;
    else
        return ((SvMetaType *)GetRef())->GetCall0();
}

void SvMetaType::SetCall1( int e )
{
    aCall1 = (int)e;
    if( aCall0 == CALL_VALUE && aCall1 == CALL_VALUE )
    {
          if( GetType() == TYPE_POINTER )
            SetType( TYPE_BASE );
    }
    else
    {
        DBG_ASSERT( nType == TYPE_POINTER || nType == TYPE_BASE,
                    "set no base type to pointer" );
        SetType( TYPE_POINTER );
    }
}

int SvMetaType::GetCall1() const
{
    if( aCall1.IsSet() || !GetRef() )
        return aCall1;
    else
        return ((SvMetaType *)GetRef())->GetCall1();
}

const OString& SvMetaType::GetSvName() const
{
    if( aSvName.IsSet() || !GetRef() )
        return aSvName.getString();
    else
        return ((SvMetaType *)GetRef())->GetSvName();
}

const OString& SvMetaType::GetSbxName() const
{
    if( aSbxName.IsSet() || !GetRef() )
        return aSbxName.getString();
    else
        return ((SvMetaType *)GetRef())->GetSbxName();
}

const OString& SvMetaType::GetOdlName() const
{
    if( aOdlName.IsSet() || !GetRef() )
        return aOdlName.getString();
    else
        return ((SvMetaType *)GetRef())->GetOdlName();
}

const OString& SvMetaType::GetCName() const
{
    if( aCName.IsSet() || !GetRef() )
        return aCName.getString();
    else
        return ((SvMetaType *)GetRef())->GetCName();
}

sal_Bool SvMetaType::SetName( const OString& rName, SvIdlDataBase * pBase )
{
    aSvName.setString(rName);
    aSbxName.setString(rName);
    aCName.setString(rName);
    if( GetType() != TYPE_ENUM )
        aOdlName.setString(rName);
    return SvMetaReference::SetName( rName, pBase );
}

OString SvMetaType::GetCString() const
{
    OStringBuffer out( GetSvName() );
    if( aCall0 == (int)CALL_POINTER )
        out.append(" *");
    else if( aCall0 == (int)CALL_REFERENCE )
        out.append(" &");
    if( aCall1 == (int)CALL_POINTER )
        out.append('*');
    else if( aCall1 == (int)CALL_REFERENCE )
        out.append('&');
    return out.makeStringAndClear();
}

sal_Bool SvMetaType::ReadHeaderSvIdl( SvIdlDataBase & rBase,
                                     SvTokenStream & rInStm )
{
    sal_Bool bOk = sal_False;
    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( SvHash_interface() )
      || pTok->Is( SvHash_shell() ) )
    {
        if( pTok->Is( SvHash_shell() ) )
            bIsShell = sal_True;
        SetType( TYPE_CLASS );
        bOk = ReadNamesSvIdl( rBase, rInStm );

    }
    else if( pTok->Is( SvHash_struct() ) )
    {
        SetType( TYPE_STRUCT );
        bOk = ReadNamesSvIdl( rBase, rInStm );
    }
    else if( pTok->Is( SvHash_union() ) )
    {
        SetType( TYPE_UNION );
        if( ReadNameSvIdl( rBase, rInStm ) )
            return sal_True;
    }
    else if( pTok->Is( SvHash_enum() ) )
    {
        SetType( TYPE_ENUM );
        bOk = ReadNameSvIdl( rBase, rInStm );
    }
    else if( pTok->Is( SvHash_typedef() )
      || pTok->Is( SvHash_item() ) )
    {
          if( pTok->Is( SvHash_item() ) )
            bIsItem = sal_True;

        SvMetaType * pType = rBase.ReadKnownType( rInStm );
        if( pType )
        {
            SetRef( pType );
            if( ReadNameSvIdl( rBase, rInStm ) )
            {
                if( rInStm.Read( '(' ) )
                {
                    DoReadContextSvIdl( rBase, rInStm );
                    if( rInStm.Read( ')' ) )
                    {
                        SetType( TYPE_METHOD );
                        bOk = sal_True;
                    }
                }
                else
                {
                    bOk = sal_True;
                }
            }
        }
        else
        {
            OString aStr("wrong typedef: ");
            rBase.SetError( aStr, rInStm.GetToken() );
            rBase.WriteError( rInStm );
        }
    }
    if( bOk )
        SetModule( rBase );
    else
        rInStm.Seek( nTokPos );
    return bOk;
}

sal_Bool SvMetaType::ReadSvIdl( SvIdlDataBase & rBase,
                             SvTokenStream & rInStm )
{
    if( ReadHeaderSvIdl( rBase, rInStm ) )
    {
        rBase.Write(OString('.'));
        return SvMetaExtern::ReadSvIdl( rBase, rInStm );
    }
    return sal_False;
}

void SvMetaType::WriteSvIdl
(
    SvIdlDataBase & rBase,
    SvStream & rOutStm,
    sal_uInt16 nTab
)
{
    WriteHeaderSvIdl( rBase, rOutStm, nTab );
    if( GetType() == TYPE_METHOD )
        WriteMethodArgs( rBase, rOutStm, nTab, WRITE_IDL );

    sal_uLong nOldPos = rOutStm.Tell();
    rOutStm << endl;
    SvMetaExtern::WriteSvIdl( rBase, rOutStm, nTab );
    if( TestAndSeekSpaceOnly( rOutStm, nOldPos ) )
        // nothin written
        rOutStm.Seek( nOldPos );
    rOutStm.WriteChar( ';' ) << endl;
}

void SvMetaType::WriteContext( SvIdlDataBase & rBase, SvStream & rOutStm,
                                 sal_uInt16 nTab,
                             WriteType nT, WriteAttribute nA )
{
    if( GetAttrCount() )
    {
        SvMetaAttributeMemberList::const_iterator it = pAttrList->begin();
        while( it != pAttrList->end() )
        {
            SvMetaAttribute * pAttr = *it;
            pAttr->Write( rBase, rOutStm, nTab, nT, nA );
            if( GetType() == TYPE_METHOD )
                rOutStm.WriteChar( ',' ) << endl;
            else
                rOutStm.WriteChar( ';' ) << endl;
            ++it;
        }
    }
}

void SvMetaType::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                          sal_uInt16 nTab,
                         WriteType nT, WriteAttribute nA )
{
    if( nT == WRITE_C_HEADER && nType != TYPE_ENUM )
        // write only enum
        return;

    OString name = GetName().getString();
    if( nT == WRITE_ODL || nT == WRITE_C_HEADER || nT == WRITE_CXX_HEADER )
    {
        switch( nType )
        {
        case TYPE_CLASS:
            {
            }
            break;
        case TYPE_STRUCT:
        case TYPE_UNION:
        case TYPE_ENUM:
        {
            WriteStars( rOutStm );
            if( nType == TYPE_STRUCT || nType == TYPE_UNION )
                nA = WA_STRUCT;

            if( nT == WRITE_ODL || nT == WRITE_C_HEADER)
            {
                if ( nT == WRITE_C_HEADER )
                {
                    OString aStr = name.toAsciiUpperCase();
                    rOutStm.WriteCharPtr( "#ifndef " ).WriteCharPtr( C_PREF ).WriteCharPtr( aStr.getStr() ).WriteCharPtr( "_DEF " ) << endl;
                    rOutStm.WriteCharPtr( "#define " ).WriteCharPtr( C_PREF ).WriteCharPtr( aStr.getStr() ).WriteCharPtr( "_DEF " ) << endl;
                }

                WriteTab( rOutStm, nTab );
                rOutStm.WriteCharPtr( "typedef" ) << endl;
                if ( nT == WRITE_ODL )
                    SvMetaName::Write( rBase, rOutStm, nTab, nT, nA );
            }
            WriteTab( rOutStm, nTab );
            if( nType == TYPE_STRUCT )
                rOutStm.WriteCharPtr( "struct" );
            else if( nType == TYPE_UNION )
                rOutStm.WriteCharPtr( "union" );
            else
                rOutStm.WriteCharPtr( "enum" );
            if( nT != WRITE_ODL && nT != WRITE_C_HEADER)
                rOutStm.WriteChar( ' ' ).WriteCharPtr( name.getStr() );

            rOutStm << endl;
            WriteTab( rOutStm, nTab );
            rOutStm.WriteChar( '{' ) << endl;
            WriteContext( rBase, rOutStm, nTab +1, nT, nA );
            WriteTab( rOutStm, nTab );
            rOutStm.WriteChar( '}' );
            if( nT == WRITE_ODL || nT == WRITE_C_HEADER )
            {
                rOutStm.WriteChar( ' ' ).WriteCharPtr( C_PREF ).WriteCharPtr( name.getStr() );
            }
            rOutStm.WriteChar( ';' ) << endl;

            if ( nT == WRITE_C_HEADER )
                rOutStm.WriteCharPtr( "#endif" );
            rOutStm << endl;
        }
        break;
        case TYPE_POINTER:
        case TYPE_BASE:
        {
        }
        break;
        case TYPE_METHOD:
        {
        }
        break;
        }
    }
}

sal_Bool SvMetaType::ReadNamesSvIdl( SvIdlDataBase & rBase,
                                     SvTokenStream & rInStm )
{
    sal_Bool bOk = ReadNameSvIdl( rBase, rInStm );

    return bOk;
}

void SvMetaType::WriteHeaderSvIdl( SvIdlDataBase & rBase,
                                   SvStream & rOutStm,
                                   sal_uInt16 nTab )
{
    switch( nType )
    {
        case TYPE_CLASS:
        {
            if( IsShell() )
                rOutStm.WriteCharPtr( SvHash_shell()->GetName().getStr() );
            else
                rOutStm.WriteCharPtr( SvHash_interface()->GetName().getStr() );
            rOutStm.WriteChar( ' ' ).WriteCharPtr( GetName().getString().getStr() );
        }
        break;
        case TYPE_STRUCT:
        {
            rOutStm.WriteCharPtr( SvHash_struct()->GetName().getStr() )
                   .WriteChar( ' ' ).WriteCharPtr( GetName().getString().getStr() );
        }
        break;
        case TYPE_UNION:
        {
            rOutStm.WriteCharPtr( SvHash_union()->GetName().getStr() )
                   .WriteChar( ' ' ).WriteCharPtr( GetName().getString().getStr() );
        }
        break;
        case TYPE_ENUM:
        {
            rOutStm.WriteCharPtr( SvHash_enum()->GetName().getStr() )
                   .WriteChar( ' ' ).WriteCharPtr( GetName().getString().getStr() );
        }
        break;
        case TYPE_POINTER:
        case TYPE_BASE:
        {
            if( IsItem() )
                rOutStm.WriteCharPtr( SvHash_item()->GetName().getStr() ).WriteChar( ' ' );
            else
                rOutStm.WriteCharPtr( SvHash_typedef()->GetName().getStr() ).WriteChar( ' ' );
            if( GetRef() )
            {
                ((SvMetaType *)GetRef())->WriteTheType( rBase, rOutStm, nTab, WRITE_IDL );
                rOutStm.WriteChar( ' ' );
            }
            rOutStm.WriteCharPtr( GetName().getString().getStr() );
        }
        break;
        case TYPE_METHOD:
        {
            rOutStm.WriteCharPtr( SvHash_typedef()->GetName().getStr() ).WriteChar( ' ' );
            ((SvMetaType *)GetRef())->WriteTheType( rBase, rOutStm, nTab, WRITE_IDL );
            rOutStm.WriteChar( ' ' ).WriteCharPtr( GetName().getString().getStr() ).WriteCharPtr( "( " );
            WriteContextSvIdl( rBase, rOutStm, nTab );
            rOutStm.WriteCharPtr( " )" );
        }
        break;
    }
}

void SvMetaType::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm )
{
    SvMetaExtern::ReadAttributesSvIdl( rBase, rInStm );
    aSvName.ReadSvIdl( SvHash_SvName(), rInStm );
    aSbxName.ReadSvIdl( SvHash_SbxName(), rInStm );
    aOdlName.ReadSvIdl( SvHash_OdlName(), rInStm );
}

void SvMetaType::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                       SvStream & rOutStm,
                                       sal_uInt16 nTab )
{
    SvMetaExtern::WriteAttributesSvIdl( rBase, rOutStm, nTab );
    OString name = GetName().getString();
    if( aSvName.getString() != name || aSbxName.getString() != name || aOdlName.getString() != name )
    {
        WriteTab( rOutStm, nTab );
        rOutStm.WriteCharPtr( "class SvMetaType" ) << endl;
        if( aSvName.getString() != name )
        {
            WriteTab( rOutStm, nTab );
            aSvName.WriteSvIdl( SvHash_SvName(), rOutStm, nTab );
            rOutStm << endl;
        }
        if( aSbxName.getString() != name )
        {
            WriteTab( rOutStm, nTab );
            aSbxName.WriteSvIdl( SvHash_SbxName(), rOutStm, nTab );
            rOutStm << endl;
        }
        if( aOdlName.getString() != name )
        {
            WriteTab( rOutStm, nTab );
            aOdlName.WriteSvIdl( SvHash_OdlName(), rOutStm, nTab );
            rOutStm << endl;
        }
    }
}

void SvMetaType::ReadContextSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm )
{
    SvMetaAttributeRef xAttr = new SvMetaAttribute();
    if( xAttr->ReadSvIdl( rBase, rInStm ) )
    {
        if( xAttr->Test( rBase, rInStm ) )
            GetAttrList().push_back( xAttr );
    }
}

void SvMetaType::WriteContextSvIdl
(
    SvIdlDataBase & rBase,
    SvStream & rOutStm,
    sal_uInt16 nTab
)
{
    if( GetAttrCount() )
    {
        SvMetaAttributeMemberList::const_iterator it = pAttrList->begin();
        while( it != pAttrList->end() )
        {
            SvMetaAttribute * pAttr = *it;
            WriteTab( rOutStm, nTab );
            pAttr->WriteSvIdl( rBase, rOutStm, nTab );
            if( GetType() == TYPE_METHOD )
                rOutStm.WriteChar( ',' ) << endl;
            else
                rOutStm.WriteChar( ';' ) << endl;
            ++it;
        }
    }
}

void SvMetaType::WriteAttributes( SvIdlDataBase & rBase, SvStream & rOutStm,
                                   sal_uInt16 nTab,
                                 WriteType nT, WriteAttribute nA )
{
    SvMetaExtern::WriteAttributes( rBase, rOutStm, nTab, nT, nA );
}

sal_uLong SvMetaType::MakeSfx( OStringBuffer& rAttrArray )
{
    sal_uLong nC = 0;

    if( GetBaseType()->GetType() == TYPE_STRUCT )
    {
        sal_uLong nAttrCount = GetAttrCount();
        // write the single attributes
        for( sal_uLong n = 0; n < nAttrCount; n++ )
        {
            nC += (*pAttrList)[n]->MakeSfx( rAttrArray );
            if( n +1 < nAttrCount )
                rAttrArray.append(", ");
        }
    }
    return nC;
}

void SvMetaType::WriteSfxItem(
    const OString& rItemName, SvIdlDataBase &, SvStream & rOutStm )
{
    WriteStars( rOutStm );
    OStringBuffer aVarName(" a");
    aVarName.append(rItemName).append("_Impl");

    OStringBuffer aTypeName("SfxType");
    OStringBuffer aAttrArray;
    sal_uLong   nAttrCount = MakeSfx( aAttrArray );
    OString aAttrCount(
        OString::number(nAttrCount));
    aTypeName.append(aAttrCount);

    rOutStm.WriteCharPtr( "extern " ).WriteCharPtr( aTypeName.getStr() )
           .WriteCharPtr( aVarName.getStr() ).WriteChar( ';' ) << endl;

    // write the implementation part
    rOutStm.WriteCharPtr( "#ifdef SFX_TYPEMAP" ) << endl;
    rOutStm.WriteCharPtr( "#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS))) || defined(LIBO_MERGELIBS))" ) << endl;
    rOutStm.WriteCharPtr( "__attribute__((__weak__))" ) << endl;
    rOutStm.WriteCharPtr( "#endif" ) << endl;
    rOutStm.WriteCharPtr( aTypeName.getStr() ).WriteCharPtr( aVarName.getStr() )
           .WriteCharPtr( " = " ) << endl;
    rOutStm.WriteChar( '{' ) << endl;
    rOutStm.WriteCharPtr( "\tTYPE(" ).WriteCharPtr( rItemName.getStr() ).WriteCharPtr( "), " )
           .WriteCharPtr( aAttrCount.getStr() );
    if( nAttrCount )
    {
        rOutStm.WriteCharPtr( ", { " );
        // write the single attributes
        rOutStm.WriteCharPtr( aAttrArray.getStr() );
        rOutStm.WriteCharPtr( " }" );
    }
    rOutStm << endl;
    rOutStm.WriteCharPtr( "};" ) << endl;
    rOutStm.WriteCharPtr( "#endif" ) << endl << endl;
}

void SvMetaType::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    if( IsItem() )
    {
        if( GetBaseType()->GetType() == TYPE_STRUCT )
            GetBaseType()->WriteSfxItem( GetName().getString(), rBase, rOutStm );
        else
            WriteSfxItem( GetName().getString(), rBase, rOutStm );
    }
}

sal_Bool SvMetaType::ReadMethodArgs( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    if( rInStm.Read( '(' ) )
    {
        DoReadContextSvIdl( rBase, rInStm );
        if( rInStm.Read( ')' ) )
        {
            SetType( TYPE_METHOD );
            return sal_True;
        }
    }
    rInStm.Seek( nTokPos );
    return sal_False;
}

void SvMetaType::WriteMethodArgs
(
    SvIdlDataBase & rBase,
    SvStream & rOutStm,
    sal_uInt16 nTab, WriteType nT
)
{
    if( nT == WRITE_IDL )
    {
        if( GetAttrCount() )
        {
            rOutStm << endl;
            WriteTab( rOutStm, nTab );
            rOutStm.WriteChar( '(' ) << endl;

            SvMetaAttributeMemberList::const_iterator it = pAttrList->begin();
            while( it != pAttrList->end() )
            {
                SvMetaAttribute * pAttr = *it;
                WriteTab( rOutStm, nTab +1 );
                pAttr->WriteSvIdl( rBase, rOutStm, nTab +1 );
                ++it;
                if( it != pAttrList->end() )
                       rOutStm.WriteChar( ',' ) << endl;
            }
            rOutStm << endl;
            WriteTab( rOutStm, nTab );
            rOutStm.WriteChar( ')' );
        }
        else
            rOutStm.WriteCharPtr( "()" );
    }
    else if ( nT == WRITE_DOCU )
    {

        rOutStm.WriteChar( '(' );
        if( GetAttrCount() )
        {
            SvMetaAttributeMemberList::const_iterator it = pAttrList->begin();
            while( it != pAttrList->end() )
            {
                SvMetaAttribute * pAttr = *it;
                pAttr->WriteParam( rBase, rOutStm, nTab+1, nT );
                ++it;
                if( it != pAttrList->end() )
                    rOutStm.WriteChar( ',' );
                else
                    rOutStm.WriteChar( ' ' );
            }
        }
        rOutStm.WriteChar( ')' ) << endl;
    }
    else
    {
        rOutStm.WriteChar( '(' );
        if( nT == WRITE_C_HEADER || nT == WRITE_C_SOURCE )
        {
            rOutStm.WriteChar( ' ' ).WriteCharPtr( C_PREF ).WriteCharPtr( "Object h" ).WriteCharPtr( rBase.aIFaceName.getStr() );
            if( GetAttrCount() )
                rOutStm.WriteChar( ',' );
            else
                rOutStm.WriteChar( ' ' );
        }

        if( GetAttrCount() )
        {
            rOutStm << endl;
            SvMetaAttributeMemberList::const_iterator it = pAttrList->begin();
            while( it != pAttrList->end() )
            {
                SvMetaAttribute* pAttr = *it;
                switch( nT )
                {
                    case WRITE_C_HEADER:
                    case WRITE_C_SOURCE:
                    case WRITE_ODL:
                    {
                        pAttr->WriteParam( rBase, rOutStm, nTab +1, nT );
                    }
                    break;

                    default:
                    {
                        DBG_ASSERT( sal_False, "WriteType not implemented" );
                    }
                }
                ++it;
                if( it != pAttrList->end() )
                       rOutStm.WriteChar( ',' ) << endl;
            }
            if( nT != WRITE_C_HEADER && nT != WRITE_C_SOURCE )
            {
                rOutStm << endl;
                WriteTab( rOutStm, nTab +1 );
            }
            rOutStm.WriteChar( ' ' );
        }
        rOutStm.WriteChar( ')' );
    }
}

void SvMetaType::WriteTypePrefix( SvIdlDataBase & rBase, SvStream & rOutStm,
                                 sal_uInt16 nTab, WriteType nT )
{
    switch( nT )
    {
        case WRITE_IDL:
        {
            if( GetIn() && GetOut() )
                rOutStm.WriteCharPtr( SvHash_inout()->GetName().getStr() ).WriteChar( ' ' );
            else if( GetIn() )
                rOutStm.WriteCharPtr( SvHash_in()->GetName().getStr() ).WriteChar( ' ' );
            else if( GetOut() )
                rOutStm.WriteCharPtr( SvHash_out()->GetName().getStr() ).WriteChar( ' ' );
            rOutStm.WriteCharPtr( GetCString().getStr() );
        }
        break;

        case WRITE_ODL:
        {
            sal_Bool bIn = GetIn();
            sal_Bool bOut = GetOut();
            if( bIn || bOut )
            {
                if( bIn && bOut )
                    rOutStm.WriteCharPtr( "[in,out] " );
                else if( bIn )
                    rOutStm.WriteCharPtr( "[in] " );
                else if( bOut )
                    rOutStm.WriteCharPtr( "[out] " );
            }

            OString out;
            if( GetType() == TYPE_METHOD )
                out = GetReturnType()->GetBaseType()->GetOdlName();
            else
            {
                SvMetaType * pBType = GetBaseType();
                out = pBType->GetOdlName();
            }
            if( aCall0 == (int)CALL_POINTER
              || aCall0 == (int)CALL_REFERENCE )
                rOutStm.WriteCharPtr( " *" );
            if( aCall1 == (int)CALL_POINTER
              || aCall1 == (int)CALL_REFERENCE )
                rOutStm.WriteCharPtr( " *" );
            rOutStm.WriteCharPtr( out.getStr() );
        }
        break;

        case WRITE_C_HEADER:
        case WRITE_C_SOURCE:
        case WRITE_CXX_HEADER:
        case WRITE_CXX_SOURCE:
        {

            SvMetaType * pBaseType = GetBaseType();
            DBG_ASSERT( pBaseType, "no base type for attribute" );

            if( pBaseType->GetType() == TYPE_METHOD )
                pBaseType->GetReturnType()->WriteTypePrefix(
                    rBase, rOutStm, nTab, nT );
            else if( nT == WRITE_C_HEADER || nT == WRITE_C_SOURCE )
            {
                if( TYPE_STRUCT == pBaseType->GetType() )
                    rOutStm.WriteCharPtr( C_PREF ).WriteCharPtr( pBaseType->GetName().getString().getStr() )
                           .WriteCharPtr( " *" );
                else
                {
                    if ( pBaseType->GetType() == TYPE_ENUM )
                        rOutStm.WriteCharPtr( C_PREF );
                    rOutStm.WriteCharPtr( pBaseType->GetCName().getStr() );
                }
            }
            else
            {
                if( TYPE_STRUCT == pBaseType->GetType() )
                    rOutStm.WriteCharPtr( pBaseType->GetName().getString().getStr() ).WriteCharPtr( " *" );
                else
                    rOutStm.WriteCharPtr( pBaseType->GetName().getString().getStr() );
            }
        }
        break;

        case WRITE_DOCU:
        {

            SvMetaType * pBaseType = GetBaseType();
            DBG_ASSERT( pBaseType, "no base type for attribute" );

            if( pBaseType->GetType() == TYPE_METHOD )
            {
                pBaseType->GetReturnType()->WriteTypePrefix(
                    rBase, rOutStm, nTab, nT );
            }
            else
            {
                if( TYPE_STRUCT == pBaseType->GetType() )
                    rOutStm.WriteCharPtr( "VARIANT" ).WriteCharPtr( pBaseType->GetName().getString().getStr() );
                else if ( pBaseType->GetType() == TYPE_ENUM )
                    rOutStm.WriteCharPtr( "integer" );
                else
                    rOutStm.WriteCharPtr( pBaseType->GetOdlName().getStr() );
            }
        }

        default:
        {
            DBG_ASSERT( sal_False, "WriteType not implemented" );
        }
    }
}

void SvMetaType::WriteTheType( SvIdlDataBase & rBase, SvStream & rOutStm,
                             sal_uInt16 nTab, WriteType nT )
{
    WriteTypePrefix( rBase, rOutStm, nTab, nT );
    if( GetType() == TYPE_METHOD )
        WriteMethodArgs( rBase, rOutStm, nTab +2, nT );
}

OString SvMetaType::GetParserString() const
{
    SvMetaType * pBT = GetBaseType();
    if( pBT != this )
        return pBT->GetParserString();

    int type = GetType();
    OString aPStr;

    if( TYPE_METHOD == type || TYPE_STRUCT == type )
    {
        sal_uLong nAttrCount = GetAttrCount();
        // write the single attributes
        for( sal_uLong n = 0; n < nAttrCount; n++ )
        {
            SvMetaAttribute * pT = (*pAttrList)[n];
            aPStr += pT->GetType()->GetParserString();
        }
    }
    else
        aPStr = OString(GetParserChar());
    return aPStr;
}

void SvMetaType::WriteParamNames( SvIdlDataBase & rBase,
                                   SvStream & rOutStm,
                                   const OString& rChief )
{
    SvMetaType * pBT = GetBaseType();
    if( pBT != this )
        pBT->WriteParamNames( rBase, rOutStm, rChief );
    else
    {
        int type = GetType();

        if( TYPE_METHOD == type || TYPE_STRUCT == type )
        {
            sal_uLong nAttrCount = GetAttrCount();
            // write the single attributes
            for( sal_uLong n = 0; n < nAttrCount; n++ )
            {
                SvMetaAttribute * pA = (*pAttrList)[n];
                OString aStr = pA->GetName().getString();
                pA->GetType()->WriteParamNames( rBase, rOutStm, aStr );
                if( n +1 < nAttrCount )
                    rOutStm.WriteCharPtr( ", " );
            }
        }
        else
            rOutStm.WriteCharPtr( rChief.getStr() );
    }
}

SV_IMPL_META_FACTORY1( SvMetaTypeString, SvMetaType );
SvMetaTypeString::SvMetaTypeString()
    : SvMetaType( "String", "SbxSTRING", "BSTR", 's', "char *", "String", "$" )
{
}

void SvMetaTypeString::Load( SvPersistStream & rStm )
{
    SvMetaType::Load( rStm );
}

void SvMetaTypeString::Save( SvPersistStream & rStm )
{
    SvMetaType::Save( rStm );
}

SV_IMPL_META_FACTORY1( SvMetaEnumValue, SvMetaName );
SvMetaEnumValue::SvMetaEnumValue()
{
}

void SvMetaEnumValue::Load( SvPersistStream & rStm )
{
    SvMetaName::Load( rStm );

    sal_uInt8 nMask;
    rStm >> nMask;
    if( nMask >= 0x02 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "wrong format" );
        return;
    }
    if( nMask & 0x01 ) aEnumValue = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rStm);
}

void SvMetaEnumValue::Save( SvPersistStream & rStm )
{
    SvMetaName::Save( rStm );

    // create mask
    sal_uInt8 nMask = 0;
    if( !aEnumValue.isEmpty() ) nMask |= 0x01;

    // write data
    rStm.WriteUChar( nMask );
    if( nMask & 0x01 ) write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rStm, aEnumValue);
}

sal_Bool SvMetaEnumValue::ReadSvIdl( SvIdlDataBase & rBase,
                                 SvTokenStream & rInStm )
{
    if( !ReadNameSvIdl( rBase, rInStm ) )
        return sal_False;
    return sal_True;
}

void SvMetaEnumValue::WriteSvIdl( SvIdlDataBase &, SvStream & rOutStm, sal_uInt16 )
{
    rOutStm.WriteCharPtr( GetName().getString().getStr() );
}

void SvMetaEnumValue::Write( SvIdlDataBase &, SvStream & rOutStm, sal_uInt16,
                             WriteType nT, WriteAttribute )
{
    if ( nT == WRITE_C_HEADER || nT == WRITE_C_SOURCE )
        rOutStm.WriteCharPtr( C_PREF ).WriteCharPtr( GetName().getString().getStr() );
    else
        rOutStm.WriteCharPtr( GetName().getString().getStr() );
}

SV_IMPL_META_FACTORY1( SvMetaTypeEnum, SvMetaType );
SvMetaTypeEnum::SvMetaTypeEnum()
{
    SetBasicName("Integer");
}

void SvMetaTypeEnum::Load( SvPersistStream & rStm )
{
    SvMetaType::Load( rStm );

    sal_uInt8 nMask;
    rStm >> nMask;
    if( nMask >= 0x04 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "wrong format" );
        return;
    }
    if( nMask & 0x01 ) rStm >> aEnumValueList;
    if( nMask & 0x02 ) aPrefix = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rStm);
}

void SvMetaTypeEnum::Save( SvPersistStream & rStm )
{
    SvMetaType::Save( rStm );

    // create mask
    sal_uInt8 nMask = 0;
    if( !aEnumValueList.empty() )   nMask |= 0x01;
    if( !aPrefix.isEmpty() )        nMask |= 0x02;

    // write data
    rStm.WriteUChar( nMask );
    if( nMask & 0x01 ) WriteSvDeclPersistList( rStm, aEnumValueList );
    if( nMask & 0x02 ) write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rStm, aPrefix);
}

namespace
{
    OString getCommonSubPrefix(const OString &rA, const OString &rB)
    {
        sal_Int32 nMax = std::min(rA.getLength(), rB.getLength());
        sal_Int32 nI = 0;
        while (nI < nMax)
        {
            if (rA[nI] != rB[nI])
                break;
            ++nI;
        }
        return rA.copy(0, nI);
    }
}

void SvMetaTypeEnum::ReadContextSvIdl( SvIdlDataBase & rBase,
                                       SvTokenStream & rInStm )
{
    sal_uInt32 nTokPos = rInStm.Tell();

    SvMetaEnumValueRef aEnumVal = new SvMetaEnumValue();
    sal_Bool bOk = aEnumVal->ReadSvIdl( rBase, rInStm );
    if( bOk )
    {
        if( aEnumValueList.empty() )
        {
           // the first
           aPrefix = aEnumVal->GetName().getString();
        }
        else
        {
            aPrefix = getCommonSubPrefix(aPrefix, aEnumVal->GetName().getString());
        }
        aEnumValueList.push_back( aEnumVal );
    }
    if( !bOk )
        rInStm.Seek( nTokPos );
}

void SvMetaTypeEnum::WriteContextSvIdl( SvIdlDataBase & rBase,
                                        SvStream & rOutStm,
                                        sal_uInt16 nTab )
{
    WriteTab( rOutStm, nTab +1 );
    for( sal_uLong n = 0; n < aEnumValueList.size(); n++ )
    {
        aEnumValueList[n]->WriteSvIdl( rBase, rOutStm, nTab );
        if( n + 1 != aEnumValueList.size() )
            rOutStm.WriteCharPtr( ", " );
        else
            rOutStm << endl;
    }
}

sal_Bool SvMetaTypeEnum::ReadSvIdl( SvIdlDataBase & rBase,
                                SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    if( SvMetaType::ReadHeaderSvIdl( rBase, rInStm )
      && GetType() == TYPE_ENUM )
    {
        if( SvMetaName::ReadSvIdl( rBase, rInStm ) )
             return sal_True;
    }
    rInStm.Seek( nTokPos );
    return sal_False;
}

void SvMetaTypeEnum::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                                   sal_uInt16 nTab )
{
    WriteHeaderSvIdl( rBase, rOutStm, nTab );
    rOutStm << endl;
    SvMetaName::WriteSvIdl( rBase, rOutStm, nTab );
    rOutStm << endl;
}

void SvMetaTypeEnum::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                            sal_uInt16 nTab,
                             WriteType nT, WriteAttribute nA )
{
    SvMetaType::Write( rBase, rOutStm, nTab, nT, nA );
}

void SvMetaTypeEnum::WriteContext( SvIdlDataBase & rBase, SvStream & rOutStm,
                                   sal_uInt16 nTab,
                                 WriteType nT, WriteAttribute nA )
{
    WriteTab( rOutStm, nTab +1 );
    for( sal_uLong n = 0; n < aEnumValueList.size(); n++ )
    {
        aEnumValueList[n]->Write( rBase, rOutStm, nTab +1, nT, nA );

        if( n + 1 != aEnumValueList.size() )
        {
            if( 2 == n % 3 )
            {
                rOutStm.WriteChar( ',' ) << endl;
                WriteTab( rOutStm, nTab +1 );
            }
            else
                rOutStm.WriteCharPtr( ",\t" );
        }
        else
            rOutStm << endl;
    }
    rOutStm << endl;
}

SV_IMPL_META_FACTORY1( SvMetaTypevoid, SvMetaType );
SvMetaTypevoid::SvMetaTypevoid()
    : SvMetaType( "void", "SbxVOID", "void", 'v', "void", "", "" )
{
}

void SvMetaTypevoid::Load( SvPersistStream & rStm )
{
    SvMetaType::Load( rStm );
}

void SvMetaTypevoid::Save( SvPersistStream & rStm )
{
    SvMetaType::Save( rStm );
}

OString SvMetaAttribute::Compare( SvMetaAttribute* pAttr )
{
    OStringBuffer aStr;

    if ( aType.Is() )
    {
        if ( aType->GetType() == TYPE_METHOD )
        {
            // Test only when the attribute is a method not if it has one!
            if ( pAttr->GetType()->GetType() != TYPE_METHOD )
                aStr.append("    IsMethod\n");
            else if ( aType->GetReturnType() &&
                aType->GetReturnType()->GetType() != pAttr->GetType()->GetReturnType()->GetType() )
            {
                aStr.append("    ReturnType\n");
            }

            if ( aType->GetAttrCount() )
            {
                sal_uLong nCount = aType->GetAttrCount();
                SvMetaAttributeMemberList& rList = aType->GetAttrList();
                SvMetaAttributeMemberList& rOtherList = pAttr->GetType()->GetAttrList();
                if ( pAttr->GetType()->GetAttrCount() != nCount )
                {
                    aStr.append("    AttributeCount\n");
                }
                else
                {
                    for ( sal_uInt16 n=0; n<nCount; n++ )
                    {
                        SvMetaAttribute *pAttr1 = rList[n];
                        SvMetaAttribute *pAttr2 = rOtherList[n];
                        pAttr1->Compare( pAttr2 );
                    }
                }
            }
        }

        if ( GetType()->GetType() != pAttr->GetType()->GetType() )
            aStr.append("    Type\n");

        if ( !GetType()->GetSvName().equals( pAttr->GetType()->GetSvName() ) )
            aStr.append("    ItemType\n");
    }

    if ( GetExport() != pAttr->GetExport() )
        aStr.append("    Export\n");

    if ( GetAutomation() != pAttr->GetAutomation() )
        aStr.append("    Automation\n");

    if ( GetIsCollection() != pAttr->GetIsCollection() )
        aStr.append("    IsCollection\n");

    if ( GetReadOnlyDoc() != pAttr->GetReadOnlyDoc() )
        aStr.append("    ReadOnlyDoc\n");

    if ( GetExport() && GetReadonly() != pAttr->GetReadonly() )
        aStr.append("    Readonly\n");

    return aStr.makeStringAndClear();
}

void SvMetaAttribute::WriteCSV( SvIdlDataBase&, SvStream& rStrm )
{
    rStrm.WriteCharPtr( GetType()->GetSvName().getStr() ).WriteChar( ' ' );
    rStrm.WriteCharPtr( GetName().getString().getStr() ).WriteChar( ' ' );
    rStrm.WriteCharPtr( GetSlotId().getString().getStr() );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
