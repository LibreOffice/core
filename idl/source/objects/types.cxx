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

TYPEINIT1( SvMetaAttribute, SvMetaReference );
SvMetaAttribute::SvMetaAttribute()
    : aAutomation( true, false )
    , aExport( true, false )
    , aIsCollection ( false, false )
    , aReadOnlyDoc ( true, false )
    , aHidden( false, false )
    , bNewAttr( false )
{
}

SvMetaAttribute::SvMetaAttribute( SvMetaType * pType )
    : aType( pType )
    , aAutomation( true, false )
    , aExport( true, false )
    , aIsCollection ( false, false)
    , aReadOnlyDoc ( true, false)
    , aHidden( false, false )
    , bNewAttr( false )
{
}

SvMetaType * SvMetaAttribute::GetType() const
{
    if( aType.Is() || !GetRef() ) return aType;
    return static_cast<SvMetaAttribute *>(GetRef())->GetType();
}

const SvIdentifier & SvMetaAttribute::GetSlotId() const
{
    if( aSlotId.IsSet() || !GetRef() ) return aSlotId;
    return static_cast<SvMetaAttribute *>(GetRef())->GetSlotId();
}

bool SvMetaAttribute::GetReadonly() const
{
    if( aReadonly.IsSet() || !GetRef() ) return aReadonly;
    return static_cast<SvMetaAttribute *>(GetRef())->GetReadonly();
}

bool SvMetaAttribute::GetExport() const
{
    if( aExport.IsSet() || !GetRef() ) return aExport;
    return static_cast<SvMetaAttribute *>(GetRef())->GetExport();
}

bool SvMetaAttribute::GetHidden() const
{
    // when export is set, but hidden is not the default is used
    if ( aExport.IsSet() && !aHidden.IsSet() )
        return !aExport;
    else if( aHidden.IsSet() || !GetRef() )
        return aHidden;
    else
        return static_cast<SvMetaAttribute *>(GetRef())->GetHidden();
}

bool SvMetaAttribute::GetAutomation() const
{
    if( aAutomation.IsSet() || !GetRef() ) return aAutomation;
    return static_cast<SvMetaAttribute *>(GetRef())->GetAutomation();
}

bool SvMetaAttribute::GetIsCollection() const
{
    if( aIsCollection.IsSet() || !GetRef() )
    {
        return aIsCollection;
    }

    return static_cast<SvMetaSlot *>(GetRef())->GetIsCollection();
}

bool SvMetaAttribute::GetReadOnlyDoc() const
{
    if( aReadOnlyDoc.IsSet() || !GetRef() ) return aReadOnlyDoc;
    return static_cast<SvMetaSlot *>(GetRef())->GetReadOnlyDoc();
}

bool SvMetaAttribute::IsMethod() const
{
    SvMetaType * pType = GetType();
    DBG_ASSERT( pType, "no type for attribute" );
    return pType->GetType() == TYPE_METHOD;
}

bool SvMetaAttribute::IsVariable() const
{
    SvMetaType * pType = GetType();
    return pType->GetType() != TYPE_METHOD;
}

OString SvMetaAttribute::GetMangleName( bool ) const
{
    return GetName().getString();
}

bool SvMetaAttribute::Test( SvIdlDataBase & rBase,
                            SvTokenStream & rInStm )
{
    bool bOk = true;
    if( GetType()->IsItem() && !GetSlotId().IsSet() )
    {
        rBase.SetError( "slot without id declared", rInStm.GetToken() );
        rBase.WriteError( rInStm );
        bOk = false;
    }
    return bOk;
}

bool SvMetaAttribute::ReadSvIdl( SvIdlDataBase & rBase,
                                 SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos     = rInStm.Tell();
    if( !GetType() )
        // no type in ctor passed on
        aType = rBase.ReadKnownType( rInStm );
    bool bOk = false;
    if( GetType() )
    {
        ReadNameSvIdl( rBase, rInStm );
        aSlotId.ReadSvIdl( rBase, rInStm );

        bOk = true;
        SvToken& rTok  = rInStm.GetToken();
        if( bOk && rTok.IsChar() && rTok.GetChar() == '(' )
        {
            SvMetaTypeRef xT = new SvMetaType();
            xT->SetRef( GetType() );
            aType = xT;
            bOk = aType->ReadMethodArgs( rBase, rInStm );
        }
        if( bOk )
            bOk = SvMetaObject::ReadSvIdl( rBase, rInStm );
    }
    else
    {
        SvToken& rTok = rInStm.GetToken();
        rBase.SetError( "unknown type of token. Each new SID needs an "
                        "item statement in an SDI file, eg. "
                        "SfxVoidItem FooItem  " + rTok.GetTokenAsString() +
                        " ... which describes the slot more fully", rTok );
    }

    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
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

TYPEINIT1( SvMetaType, SvMetaExtern );
#define CTOR                            \
    : aCall0( CALL_VALUE, false )       \
    , aCall1( CALL_VALUE, false )       \
    , aSbxDataType( 0, false )          \
    , pAttrList( NULL )                 \
    , nType( TYPE_BASE )                \
    , bIsItem( false )                  \
    , bIsShell( false )                 \
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

SvMetaType::~SvMetaType() {
    delete pAttrList;
}

SvMetaAttributeMemberList & SvMetaType::GetAttrList() const
{
    if( !pAttrList )
        const_cast<SvMetaType *>(this)->pAttrList = new SvMetaAttributeMemberList();
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
        return static_cast<SvMetaType *>(GetRef())->GetBaseType();
    return const_cast<SvMetaType *>(this);
}

SvMetaType * SvMetaType::GetReturnType() const
{
    DBG_ASSERT( GetType() == TYPE_METHOD, "no method" );
    DBG_ASSERT( GetRef(), "no return type" );
    return static_cast<SvMetaType *>(GetRef());
}

const OString& SvMetaType::GetBasicName() const
{
    if( aBasicName.IsSet() || !GetRef() )
        return aBasicName.getString();
    else
        return static_cast<SvMetaType*>(GetRef())->GetBasicName();
}

bool SvMetaType::GetIn() const
{
    if( aIn.IsSet() || !GetRef() )
        return aIn;
    else
        return static_cast<SvMetaType *>(GetRef())->GetIn();
}

bool SvMetaType::GetOut() const
{
    if( aOut.IsSet() || !GetRef() )
        return aOut;
    else
        return static_cast<SvMetaType *>(GetRef())->GetOut();
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
        return static_cast<SvMetaType *>(GetRef())->GetCall0();
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
        return static_cast<SvMetaType *>(GetRef())->GetCall1();
}

const OString& SvMetaType::GetSvName() const
{
    if( aSvName.IsSet() || !GetRef() )
        return aSvName.getString();
    else
        return static_cast<SvMetaType *>(GetRef())->GetSvName();
}

const OString& SvMetaType::GetSbxName() const
{
    if( aSbxName.IsSet() || !GetRef() )
        return aSbxName.getString();
    else
        return static_cast<SvMetaType *>(GetRef())->GetSbxName();
}

const OString& SvMetaType::GetOdlName() const
{
    if( aOdlName.IsSet() || !GetRef() )
        return aOdlName.getString();
    else
        return static_cast<SvMetaType *>(GetRef())->GetOdlName();
}

const OString& SvMetaType::GetCName() const
{
    if( aCName.IsSet() || !GetRef() )
        return aCName.getString();
    else
        return static_cast<SvMetaType *>(GetRef())->GetCName();
}

bool SvMetaType::SetName( const OString& rName, SvIdlDataBase * pBase )
{
    aSvName.setString(rName);
    aSbxName.setString(rName);
    aCName.setString(rName);
    if( GetType() != TYPE_ENUM )
        aOdlName.setString(rName);
    return SvMetaReference::SetName( rName, pBase );
}

bool SvMetaType::ReadHeaderSvIdl( SvIdlDataBase & rBase,
                                  SvTokenStream & rInStm )
{
    bool bOk = false;
    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( SvHash_interface() )
      || pTok->Is( SvHash_shell() ) )
    {
        if( pTok->Is( SvHash_shell() ) )
            bIsShell = true;
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
            return true;
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
            bIsItem = true;

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
                        bOk = true;
                    }
                }
                else
                {
                    bOk = true;
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

bool SvMetaType::ReadSvIdl( SvIdlDataBase & rBase,
                            SvTokenStream & rInStm )
{
    if( ReadHeaderSvIdl( rBase, rInStm ) )
    {
        rBase.Write(OString('.'));
        return SvMetaExtern::ReadSvIdl( rBase, rInStm );
    }
    return false;
}

bool SvMetaType::ReadNamesSvIdl( SvIdlDataBase & rBase,
                                 SvTokenStream & rInStm )
{
    bool bOk = ReadNameSvIdl( rBase, rInStm );

    return bOk;
}

void SvMetaType::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm )
{
    SvMetaExtern::ReadAttributesSvIdl( rBase, rInStm );
    aSvName.ReadSvIdl( SvHash_SvName(), rInStm );
    aSbxName.ReadSvIdl( SvHash_SbxName(), rInStm );
    aOdlName.ReadSvIdl( SvHash_OdlName(), rInStm );
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
    const OString& rItemName, SvIdlDataBase& rBase, SvStream& rOutStm )
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

    bool bExport = false, bReturn = false;
    // these are exported from sfx library
    if (rItemName == "SfxBoolItem" ||
        rItemName == "SfxStringItem" ||
        rItemName == "SfxUInt16Item" ||
        rItemName == "SfxUInt32Item" ||
        rItemName == "SfxVoidItem")
    {
        bExport = true;
        if (!rBase.sSlotMapFile.endsWith("sfxslots.hxx"))
            bReturn = true;
    }

    rOutStm.WriteCharPtr( "extern " );
    if (bExport)
        rOutStm.WriteCharPtr( "SFX2_DLLPUBLIC " );
    rOutStm.WriteCharPtr( aTypeName.getStr() )
           .WriteCharPtr( aVarName.getStr() ).WriteChar( ';' ) << endl;
    if (bReturn)
        return;

    // write the implementation part
    rOutStm.WriteCharPtr( "#ifdef SFX_TYPEMAP" ) << endl;
    rOutStm.WriteCharPtr( "#if !defined(_WIN32) && ((defined(DISABLE_DYNLOADING) && (defined(ANDROID) || defined(IOS))) || STATIC_LINKING)" ) << endl;
    rOutStm.WriteCharPtr( "__attribute__((__weak__))" ) << endl;
    rOutStm.WriteCharPtr( "#endif" ) << endl;
    if (bExport)
        rOutStm.WriteCharPtr( "SFX2_DLLPUBLIC " );
    rOutStm.WriteCharPtr( aTypeName.getStr() ).WriteCharPtr( aVarName.getStr() )
           .WriteCharPtr( " = " ) << endl;
    rOutStm.WriteChar( '{' ) << endl;

#if 0
    createSfxPoolItem<SfxStringItem>, &typeid(SfxStringItem), 0
    [](){return new SfxVoidItem(0);}, &typeid(SfxVoidItem), 0
#endif
    rOutStm.WriteCharPtr( "\tcreateSfxPoolItem<" ).WriteCharPtr( rItemName.getStr() )
        .WriteCharPtr(">, &typeid(").WriteCharPtr( rItemName.getStr() ).WriteCharPtr( "), " );
    rOutStm.WriteCharPtr( aAttrCount.getStr() );
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

bool SvMetaType::ReadMethodArgs( SvIdlDataBase & rBase,
                                 SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    if( rInStm.Read( '(' ) )
    {
        DoReadContextSvIdl( rBase, rInStm );
        if( rInStm.Read( ')' ) )
        {
            SetType( TYPE_METHOD );
            return true;
        }
    }
    rInStm.Seek( nTokPos );
    return false;
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

TYPEINIT1( SvMetaTypeString, SvMetaType );
SvMetaTypeString::SvMetaTypeString()
    : SvMetaType( "String", "SbxSTRING", "BSTR", 's', "char *", "String", "$" )
{
}

TYPEINIT1( SvMetaEnumValue, SvMetaObject );
SvMetaEnumValue::SvMetaEnumValue()
{
}

bool SvMetaEnumValue::ReadSvIdl( SvIdlDataBase & rBase,
                                 SvTokenStream & rInStm )
{
    if( !ReadNameSvIdl( rBase, rInStm ) )
        return false;
    return true;
}

TYPEINIT1( SvMetaTypeEnum, SvMetaType );
SvMetaTypeEnum::SvMetaTypeEnum()
{
    SetBasicName("Integer");
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

    tools::SvRef<SvMetaEnumValue> aEnumVal = new SvMetaEnumValue();
    bool bOk = aEnumVal->ReadSvIdl( rBase, rInStm );
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

bool SvMetaTypeEnum::ReadSvIdl( SvIdlDataBase & rBase,
                                SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    if( SvMetaType::ReadHeaderSvIdl( rBase, rInStm )
      && GetType() == TYPE_ENUM )
    {
        if( SvMetaObject::ReadSvIdl( rBase, rInStm ) )
             return true;
    }
    rInStm.Seek( nTokPos );
    return false;
}

TYPEINIT1( SvMetaTypevoid, SvMetaType );
SvMetaTypevoid::SvMetaTypevoid()
    : SvMetaType( "void", "SbxVOID", "void", 'v', "void", "", "" )
{
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
                    for ( sal_uLong n=0; n<nCount; n++ )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
