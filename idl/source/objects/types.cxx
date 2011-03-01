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

#include <tools/debug.hxx>

#include <types.hxx>
#include <globals.hxx>
#include <database.hxx>

SV_IMPL_META_FACTORY1( SvMetaAttribute, SvMetaReference );
SvMetaAttribute::SvMetaAttribute()
    : aAutomation( TRUE, FALSE )
    , aExport( TRUE, FALSE )
    , aIsCollection ( FALSE, FALSE )
    , aReadOnlyDoc ( TRUE, FALSE )
    , aHidden( FALSE, FALSE )
    , bNewAttr( FALSE )
{
}

SvMetaAttribute::SvMetaAttribute( SvMetaType * pType )
    : aType( pType )
    , aAutomation( TRUE, FALSE )
    , aExport( TRUE, FALSE )
    , aIsCollection ( FALSE, FALSE)
    , aReadOnlyDoc ( TRUE, FALSE)
    , aHidden( FALSE, FALSE )
    , bNewAttr( FALSE )
{
}

void SvMetaAttribute::Load( SvPersistStream & rStm )
{
    SvMetaReference::Load( rStm );

    BYTE nMask;
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
    BYTE nMask = 0;
    if( aType.Is() )            nMask |= 0x1;
    if( aSlotId.IsSet() )       nMask |= 0x2;
    if( aExport.IsSet() )       nMask |= 0x4;
    if( aReadonly.IsSet() )     nMask |= 0x8;
    if( aAutomation.IsSet() )   nMask |= 0x10;
    if( aIsCollection.IsSet() ) nMask |= 0x20;
    if( aReadOnlyDoc.IsSet() )  nMask |= 0x40;
    if( aHidden.IsSet() )       nMask |= 0x80;

    // write data
    rStm << nMask;
    if( nMask & 0x1 )   rStm << aType;
    if( nMask & 0x2 )   rStm << aSlotId;
    if( nMask & 0x4 )   rStm << aExport;
    if( nMask & 0x8 )   rStm << aReadonly;
    if( nMask & 0x10 )  rStm << aAutomation;
    if( nMask & 0x20 )  rStm << aIsCollection;
    if( nMask & 0x40 )  rStm << aReadOnlyDoc;
    if( nMask & 0x80 )  rStm << aHidden;
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

BOOL SvMetaAttribute::GetReadonly() const
{
    if( aReadonly.IsSet() || !GetRef() ) return aReadonly;
    return ((SvMetaAttribute *)GetRef())->GetReadonly();
}

BOOL SvMetaAttribute::GetExport() const
{
    if( aExport.IsSet() || !GetRef() ) return aExport;
    return ((SvMetaAttribute *)GetRef())->GetExport();
}

BOOL SvMetaAttribute::GetHidden() const
{
    // when export is set, but hidden is not the default is used
    if ( aExport.IsSet() && !aHidden.IsSet() )
        return !aExport;
    else if( aHidden.IsSet() || !GetRef() )
        return aHidden;
    else
        return ((SvMetaAttribute *)GetRef())->GetHidden();
}

BOOL SvMetaAttribute::GetAutomation() const
{
    if( aAutomation.IsSet() || !GetRef() ) return aAutomation;
    return ((SvMetaAttribute *)GetRef())->GetAutomation();
}

BOOL SvMetaAttribute::GetIsCollection() const
{
    BOOL bRet;
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

BOOL SvMetaAttribute::GetReadOnlyDoc() const
{
    if( aReadOnlyDoc.IsSet() || !GetRef() ) return aReadOnlyDoc;
    return ((SvMetaSlot *)GetRef())->GetReadOnlyDoc();
}

BOOL SvMetaAttribute::IsMethod() const
{
    SvMetaType * pType = GetType();
    DBG_ASSERT( pType, "no type for attribute" );
    return pType->GetType() == TYPE_METHOD;
}

BOOL SvMetaAttribute::IsVariable() const
{
    SvMetaType * pType = GetType();
    return pType->GetType() != TYPE_METHOD;
}

ByteString SvMetaAttribute::GetMangleName( BOOL ) const
{
    return GetName();
}

#ifdef IDL_COMPILER
BOOL SvMetaAttribute::Test( SvIdlDataBase & rBase,
                            SvTokenStream & rInStm )
{
    BOOL bOk = TRUE;
    if( GetType()->IsItem() && !GetSlotId().IsSet() )
    {
        rBase.SetError( "slot without id declared", rInStm.GetToken() );
        rBase.WriteError( rInStm );
        bOk = FALSE;
    }
    return bOk;
}

BOOL SvMetaAttribute::ReadSvIdl( SvIdlDataBase & rBase,
                                     SvTokenStream & rInStm )
{
    UINT32  nTokPos     = rInStm.Tell();
    if( !GetType() )
        // no type in ctor passed on
        aType = rBase.ReadKnownType( rInStm );
    BOOL bOk = FALSE;
    if( GetType() )
    {
        ReadNameSvIdl( rBase, rInStm );
        aSlotId.ReadSvIdl( rBase, rInStm );

        bOk = TRUE;
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
        rBase.SetError( "unknown type", rInStm.GetToken() );

    if( !bOk )
        rInStm.Seek( nTokPos );
    return bOk;
}

void SvMetaAttribute::WriteSvIdl
(
    SvIdlDataBase & rBase,
    SvStream & rOutStm,
    USHORT nTab
)
{
    SvMetaType * pType = GetType();
    pType->WriteTypePrefix( rBase, rOutStm, nTab, WRITE_IDL );
    rOutStm << ' ' << GetName().GetBuffer();
    if( aSlotId.IsSet() )
        rOutStm << ' ' << aSlotId.GetBuffer();
    if( pType->GetType() == TYPE_METHOD )
        pType->WriteMethodArgs( rBase, rOutStm, nTab, WRITE_IDL );
    ULONG nPos = rOutStm.Tell();
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
    USHORT nTab
)
{
    SvMetaReference::WriteAttributesSvIdl( rBase, rOutStm, nTab );

    if( !aExport || !aAutomation || aReadonly )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << "//class SvMetaAttribute" << endl;
        if( !aExport )
        {
            WriteTab( rOutStm, nTab );
            aExport.WriteSvIdl( SvHash_Export(), rOutStm );
            rOutStm << ';' << endl;
        }
        if ( aHidden != aExport )
        {
            WriteTab( rOutStm, nTab );
            aExport.WriteSvIdl( SvHash_Hidden(), rOutStm );
            rOutStm << ';' << endl;
        }
        if( aReadonly )
        {
            WriteTab( rOutStm, nTab );
            aReadonly.WriteSvIdl( SvHash_Readonly(), rOutStm );
            rOutStm << ';' << endl;
        }
        if( !aAutomation )
        {
            WriteTab( rOutStm, nTab );
            aAutomation.WriteSvIdl( SvHash_Automation(), rOutStm );
            rOutStm << ';' << endl;
        }
        if( aIsCollection )
        {
            WriteTab( rOutStm, nTab );
            aIsCollection.WriteSvIdl( SvHash_IsCollection(), rOutStm );
            rOutStm << ';' << endl;
        }
        if( !aReadOnlyDoc )
        {
            WriteTab( rOutStm, nTab );
            aReadOnlyDoc.WriteSvIdl( SvHash_ReadOnlyDoc(), rOutStm );
            rOutStm << ';' << endl;
        }
    }
}

void SvMetaAttribute::WriteParam( SvIdlDataBase & rBase,
                                    SvStream & rOutStm,
                                    USHORT nTab,
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
            ULONG nCount = rList.Count();
            for( ULONG i = 0; i < nCount; i++ )
            {
                rList.GetObject( i )->WriteParam( rBase, rOutStm, nTab, nT );
                if( i+1<nCount )
                {
                        rOutStm << ',';
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

            if( GetName().Len() )
            {
                rOutStm << ' ';
                rOutStm << GetName().GetBuffer();
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
                    rOutStm << pBaseType->GetBasicPostfix().GetBuffer();
            }
        }
    }
}

ULONG SvMetaAttribute::MakeSlotValue( SvIdlDataBase & rBase, BOOL bVar ) const
{
     const SvNumberIdentifier & rId = GetSlotId();
    ULONG n = rId.GetValue();
    if( rBase.aStructSlotId.Len() )
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
                                      USHORT nTab,
                                      WriteType nT, WriteAttribute nA )
{
    if( nT == WRITE_ODL  )
    {
        const SvNumberIdentifier & rId = GetSlotId();
        BOOL bReadonly = GetReadonly() || ( nA & WA_READONLY );
        if( (rId.IsSet() && !(nA & WA_STRUCT)) || bReadonly )
        {
            BOOL bVar = IsVariable();
            if( nA & WA_VARIABLE )
                bVar = TRUE;
            else if( nA & WA_METHOD )
                bVar = FALSE;

            WriteTab( rOutStm, nTab );
            rOutStm << "//class SvMetaAttribute" << endl;
            if( rId.IsSet() && !(nA & WA_STRUCT) )
            {
                WriteTab( rOutStm, nTab );
                rOutStm << "id("
                        << ByteString::CreateFromInt32(MakeSlotValue( rBase, bVar )).GetBuffer()
                        << ")," << endl;
            }
            if( bVar && (bReadonly || IsMethod()) )
            {
                WriteTab( rOutStm, nTab );
                rOutStm << "readonly," << endl;
            }
        }
    }
}

void SvMetaAttribute::WriteCSource( SvIdlDataBase & rBase, SvStream & rOutStm,
                                    BOOL bSet )
{
    rOutStm << endl;
    SvMetaType * pType = GetType();
    SvMetaType * pBaseType = pType->GetBaseType();

    // for Set the return is always void
    BOOL bVoid = bSet;
    if( pBaseType->GetType() == TYPE_METHOD )
        bVoid = pBaseType->GetReturnType()->GetBaseType()->GetName() == "void";

    // emit methods/functions body
    rOutStm << '{' << endl;
    WriteTab( rOutStm, 1 );

    if( !bVoid )
    {
        if ( pBaseType->GetCName() == "double" )
        {
            rOutStm << "return *(double*)";
        }
        else
        {
            rOutStm << "return (";
            pType->WriteTypePrefix( rBase, rOutStm, 2, WRITE_C_SOURCE );
            rOutStm << ") ";
        }
    }
    rOutStm << "pODKCallFunction( "
            << ByteString::CreateFromInt32(MakeSlotValue( rBase, IsVariable() )).GetBuffer();
    rOutStm << ',' << endl;
    WriteTab( rOutStm, 3 );
    rOutStm << " h" << rBase.aIFaceName.GetBuffer() << " , ";

    ByteString aParserStr;
    if( pBaseType->GetType() == TYPE_METHOD || bSet )
        aParserStr = pBaseType->GetParserString();
    if( aParserStr.Len() )
    {
        rOutStm << '\"';
        rOutStm << aParserStr.GetBuffer();
        rOutStm << "\", ";
    }
    else
        rOutStm << "NULL, ";

    if( pBaseType->GetType() == TYPE_METHOD && !bVoid )
    {
        rOutStm << "'";
        rOutStm << pBaseType->GetReturnType()->GetBaseType()->GetParserChar();
        rOutStm << "'";
    }
    else if ( !bSet )
    {
        rOutStm << "'";
        rOutStm << pBaseType->GetParserChar();
        rOutStm << "'";
    }
    else
        rOutStm << '0';

    if( aParserStr.Len() )
    {
        rOutStm << ", ";
        if( IsMethod() )
            pBaseType->WriteParamNames( rBase, rOutStm, ByteString() );
        else if( bSet )
            pBaseType->WriteParamNames( rBase, rOutStm, GetName() );
    }

    rOutStm << " );" << endl;
    rOutStm << '}' << endl;
}

void SvMetaAttribute::WriteRecursiv_Impl( SvIdlDataBase & rBase,
                            SvStream & rOutStm, USHORT nTab,
                             WriteType nT, WriteAttribute nA )
{
    const SvMetaAttributeMemberList & rList = GetType()->GetBaseType()->GetAttrList();
    ULONG nCount = rList.Count();

    SvNumberIdentifier slotId = rBase.aStructSlotId;
    if ( GetSlotId().Len() )
        rBase.aStructSlotId = GetSlotId();

    // offial hack interface by MM: special controls get passed with the WriteAttribute
    if ( GetReadonly() )
        nA |= WA_READONLY;

    for( ULONG i = 0; i < nCount; i++ )
    {
        SvMetaAttribute *pAttr = rList.GetObject( i );
        if ( nT == WRITE_DOCU )
            pAttr->SetDescription( GetDescription() );
        pAttr->Write( rBase, rOutStm, nTab, nT, nA );
        if( nT == WRITE_ODL && i +1 < nCount )
            rOutStm << ';' << endl;
    }

    rBase.aStructSlotId = slotId;
}

void SvMetaAttribute::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                            USHORT nTab,
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

    BOOL bVariable;
    if( nA & WA_VARIABLE )
        bVariable = TRUE;
    else if( nA & WA_METHOD )
        bVariable = FALSE;
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

                if( GetName().Len() )
                {
                    rOutStm << ' ';
                    rOutStm << GetName().GetBuffer();
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
            ByteString name;
            name += rBase.aIFaceName;
            name += GetName();
            const char * pName = name.GetBuffer();
            WriteTab( rOutStm, nTab );
            pBaseType->WriteTypePrefix( rBase, rOutStm, nTab, nT );
            rOutStm << ' ' << pName;
            pType->WriteMethodArgs( rBase, rOutStm, nTab, nT );
            if( nT == WRITE_C_HEADER )
                rOutStm << ';' << endl << endl;
            else
                WriteCSource( rBase, rOutStm, FALSE );
        }
        else if ( bVariable && IsVariable() )
        {
              if( nBType == TYPE_STRUCT )
            {
                // for assistance emit the name of the property as acomment
                rOutStm << "/* " << GetName().GetBuffer() << " */" << endl;

                WriteRecursiv_Impl( rBase, rOutStm, nTab, nT, nA );
            }
            else
            {
                ByteString name = GetName();

                BOOL bReadonly = GetReadonly() || ( nA & WA_READONLY );
                if ( !bReadonly && !IsMethod() )
                {
                    // allocation
                    WriteTab( rOutStm, nTab );
                    rOutStm << "void ";
                    rOutStm << rBase.aIFaceName.GetBuffer()
                            << "Set" << name.GetBuffer() << "( " << C_PREF
                            << "Object h" << rBase.aIFaceName.GetBuffer() << ", " << endl;
                    WriteTab( rOutStm, nTab+1 );
                    pBaseType->WriteTypePrefix( rBase, rOutStm, nTab, nT );
                    rOutStm << ' ' << name.GetBuffer() << " )";
                    if( nT == WRITE_C_HEADER )
                        rOutStm << ';' << endl << endl;
                    else
                        WriteCSource( rBase, rOutStm, TRUE );
                }

                // access
                WriteTab( rOutStm, nTab );
                pBaseType->WriteTypePrefix( rBase, rOutStm, nTab, nT );
                rOutStm << ' ';
                rOutStm << rBase.aIFaceName.GetBuffer()
                        << "Get" << name.GetBuffer() << "( " << C_PREF
                        << "Object h" << rBase.aIFaceName.GetBuffer() << " )";
                if( nT == WRITE_C_HEADER )
                    rOutStm << ';' << endl << endl;
                else
                    WriteCSource( rBase, rOutStm, FALSE );
            }
        }
    }
    else if ( nT == WRITE_DOCU )
    {
        if( !bVariable && IsMethod() )
        {
            rOutStm << "<METHOD>" << endl
                    << GetSlotId().GetBuffer() << endl
                    << GetName().GetBuffer() << endl
                    << endl;    // readonly

            // return type
            SvMetaType* pType2 = GetType();
            SvMetaType* pBaseType2 = pType2->GetBaseType();
            rOutStm << pBaseType2->GetReturnType()->GetBaseType()->GetBasicName().GetBuffer() << endl;

            DBG_ASSERT( pBaseType2->GetReturnType()->GetBaseType()->GetBasicName().Len(),
                "Leerer BasicName" );

            // syntax
            rOutStm << GetName().GetBuffer();
            pType2->WriteMethodArgs( rBase, rOutStm, nTab, nT );

            // C return type
            pBaseType2->WriteTypePrefix( rBase, rOutStm, 0, WRITE_C_HEADER );
            rOutStm << endl;

            // for methods also C syntax
            rOutStm << "<C-SYNTAX>" << endl;
            Write( rBase, rOutStm, 0, WRITE_C_HEADER, nA );
            rOutStm << "</C-SYNTAX>" << endl;

            // description
            WriteDescription( rOutStm );
            rOutStm << "</METHOD>" << endl << endl;
        }
        else if( bVariable && IsVariable() )
        {
            if( nBType == TYPE_STRUCT )
            {
                WriteRecursiv_Impl( rBase, rOutStm, nTab, nT, nA );
            }
            else
            {
                rOutStm << "<PROPERTY>" << endl
                        << GetSlotId().GetBuffer() << endl
                        << GetName().GetBuffer() << endl;
                if ( GetReadonly() )
                    rOutStm << "(nur lesen)" << endl;
                else
                    rOutStm << endl;

                // for properties type instead of the return value
                rOutStm << pBaseType->GetBasicName().GetBuffer() << endl;

                DBG_ASSERT( pBaseType->GetBasicName().Len(),
                    "Leerer BasicName" );

                // for properties no syntax
                rOutStm << endl;

                // C return type
                pBaseType->WriteTypePrefix( rBase, rOutStm, 0, WRITE_C_HEADER );
                rOutStm << endl;

                // description
                WriteDescription( rOutStm );
                rOutStm << "</PROPERTY>" << endl << endl;
            }
        }
    }
}

ULONG SvMetaAttribute::MakeSfx( ByteString * pAttrArray )
{
    SvMetaType * pType = GetType();
    DBG_ASSERT( pType, "no type for attribute" );
    SvMetaType * pBaseType = pType->GetBaseType();
    DBG_ASSERT( pBaseType, "no base type for attribute" );
    if( pBaseType->GetType() == TYPE_STRUCT )
        return pBaseType->MakeSfx( pAttrArray );
    else
    {
        *pAttrArray += '{';
        *pAttrArray += GetSlotId();
        *pAttrArray +=  ",\"";
        *pAttrArray +=  GetName();
        *pAttrArray +=  "\"}";
        return 1;
    }
}

void SvMetaAttribute::Insert (SvSlotElementList&, const ByteString &, SvIdlDataBase&)
{
}

void SvMetaAttribute::WriteSrc( SvIdlDataBase &, SvStream &, Table * )
{
}

void SvMetaAttribute::WriteHelpId( SvIdlDataBase &, SvStream &, Table * )
{
}

#endif // IDL_COMPILER

SV_IMPL_META_FACTORY1( SvMetaType, SvMetaExtern );
#define CTOR                            \
    : aCall0( CALL_VALUE, FALSE )       \
    , aCall1( CALL_VALUE, FALSE )       \
    , aSbxDataType( 0, FALSE )          \
    , pAttrList( NULL )                 \
    , nType( TYPE_BASE )                \
    , bIsItem( FALSE )                  \
    , bIsShell( FALSE )                 \
    , cParserChar( 'h' )

SvMetaType::SvMetaType()
    CTOR
{
}

SvMetaType::SvMetaType( const ByteString & rName, char cPC,
                        const ByteString & rCName )
    CTOR
{
    SetName( rName );
    cParserChar = cPC;
    aCName = rCName;
}

SvMetaType::SvMetaType( const ByteString & rName,
                        const ByteString & rSbxName,
                        const ByteString & rOdlName,
                        char cPc,
                        const ByteString & rCName,
                        const ByteString & rBasicName,
                        const ByteString & rBasicPostfix )
    CTOR
{
    SetName( rName );
    aSbxName    = rSbxName;
    aOdlName    = rOdlName;
    cParserChar = cPc;
    aCName      = rCName;
    aBasicName  = rBasicName;
    aBasicPostfix = rBasicPostfix;
}

void SvMetaType::Load( SvPersistStream & rStm )
{
    SvMetaExtern::Load( rStm );

    USHORT nMask;
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
    if( nMask & 0x0200 ) bIsItem = TRUE;
    if( nMask & 0x0400 ) bIsShell = TRUE;
    if( nMask & 0x0800 )
    {
        USHORT nT;
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
    USHORT nMask = 0;
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
    rStm << nMask;
    if( nMask & 0x0001 ) rStm << aIn;
    if( nMask & 0x0002 ) rStm << aOut;
    if( nMask & 0x0004 ) rStm << aCall0;
    if( nMask & 0x0008 ) rStm << aCall1;
    if( nMask & 0x0010 ) rStm << aSbxDataType;
    if( nMask & 0x0020 ) rStm << aSvName;
    if( nMask & 0x0040 ) rStm << aSbxName;
    if( nMask & 0x0080 ) rStm << aOdlName;
    if( nMask & 0x0100 ) rStm << *pAttrList;
    if( nMask & 0x0800 ) rStm << (USHORT)nType;
    if( nMask & 0x1000 ) rStm << cParserChar;
    if( nMask & 0x2000 ) rStm << aCName;
    if( nMask & 0x4000 ) rStm << aBasicName;
    if( nMask & 0x8000 ) rStm << aBasicPostfix;
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
        aOdlName = "short";
    }
    else if( nType == TYPE_CLASS )
    {
        aCName = C_PREF;
        aCName += "Object *";
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

const ByteString& SvMetaType::GetBasicName() const
{
    if( aBasicName.IsSet() || !GetRef() )
        return aBasicName;
    else
        return ((SvMetaType*)GetRef())->GetBasicName();
}

ByteString SvMetaType::GetBasicPostfix() const
{

    ByteString aRet;

    // MBN and Co always want "As xxx"
    {

        aRet = " As ";
        aRet += GetBasicName();
    }

    return aRet;
}

BOOL SvMetaType::GetIn() const
{
    if( aIn.IsSet() || !GetRef() )
        return aIn;
    else
        return ((SvMetaType *)GetRef())->GetIn();
}

BOOL SvMetaType::GetOut() const
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

const ByteString & SvMetaType::GetSvName() const
{
    if( aSvName.IsSet() || !GetRef() )
        return aSvName;
    else
        return ((SvMetaType *)GetRef())->GetSvName();
}

const ByteString & SvMetaType::GetSbxName() const
{
    if( aSbxName.IsSet() || !GetRef() )
        return aSbxName;
    else
        return ((SvMetaType *)GetRef())->GetSbxName();
}

const ByteString & SvMetaType::GetOdlName() const
{
    if( aOdlName.IsSet() || !GetRef() )
        return aOdlName;
    else
        return ((SvMetaType *)GetRef())->GetOdlName();
}

const ByteString & SvMetaType::GetCName() const
{
    if( aCName.IsSet() || !GetRef() )
        return aCName;
    else
        return ((SvMetaType *)GetRef())->GetCName();
}

BOOL SvMetaType::SetName( const ByteString & rName, SvIdlDataBase * pBase )
{
    aSvName     = rName;
    aSbxName    = rName;
    aCName      = rName;
    if( GetType() != TYPE_ENUM )
        aOdlName = rName;
    return SvMetaReference::SetName( rName, pBase );
}

#ifdef IDL_COMPILER
ByteString SvMetaType::GetCString() const
{
    ByteString out( GetSvName() );
    if( aCall0 == (int)CALL_POINTER )
        out += " *";
    else if( aCall0 == (int)CALL_REFERENCE )
        out += " &";
    if( aCall1 == (int)CALL_POINTER )
        out += '*';
    else if( aCall1 == (int)CALL_REFERENCE )
        out += '&';
    return out;
}

BOOL SvMetaType::ReadHeaderSvIdl( SvIdlDataBase & rBase,
                                     SvTokenStream & rInStm )
{
    BOOL bOk = FALSE;
    UINT32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( SvHash_interface() )
      || pTok->Is( SvHash_shell() ) )
    {
        if( pTok->Is( SvHash_shell() ) )
            bIsShell = TRUE;
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
            return TRUE;
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
            bIsItem = TRUE;

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
                        bOk = TRUE;
                    }
                }
                else
                {
                    bOk = TRUE;
                }
            }
        }
        else
        {
            ByteString aStr = "wrong typedef: ";
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

BOOL SvMetaType::ReadSvIdl( SvIdlDataBase & rBase,
                             SvTokenStream & rInStm )
{
    if( ReadHeaderSvIdl( rBase, rInStm ) )
    {
        rBase.Write( '.' );
        return SvMetaExtern::ReadSvIdl( rBase, rInStm );
    }
    return FALSE;
}

void SvMetaType::WriteSvIdl
(
    SvIdlDataBase & rBase,
    SvStream & rOutStm,
    USHORT nTab
)
{
    WriteHeaderSvIdl( rBase, rOutStm, nTab );
    if( GetType() == TYPE_METHOD )
        WriteMethodArgs( rBase, rOutStm, nTab, WRITE_IDL );

    ULONG nOldPos = rOutStm.Tell();
    rOutStm << endl;
    SvMetaExtern::WriteSvIdl( rBase, rOutStm, nTab );
    if( TestAndSeekSpaceOnly( rOutStm, nOldPos ) )
        // nothin written
        rOutStm.Seek( nOldPos );
    rOutStm << ';' << endl;
}

void SvMetaType::WriteContext( SvIdlDataBase & rBase, SvStream & rOutStm,
                                 USHORT nTab,
                             WriteType nT, WriteAttribute nA )
{
    if( GetAttrCount() )
    {
        SvMetaAttribute * pAttr = pAttrList->First();
        while( pAttr )
        {
            pAttr->Write( rBase, rOutStm, nTab, nT, nA );
            if( GetType() == TYPE_METHOD )
                rOutStm << ',' << endl;
            else
                rOutStm << ';' << endl;
            pAttr = pAttrList->Next();
        }
    }
}

void SvMetaType::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                          USHORT nTab,
                         WriteType nT, WriteAttribute nA )
{
    if( nT == WRITE_C_HEADER && nType != TYPE_ENUM )
        // write only enum
        return;

    ByteString name = GetName();
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
                    ByteString aStr = name;
                    aStr.ToUpperAscii();
                    rOutStm << "#ifndef " << C_PREF << aStr.GetBuffer() << "_DEF " << endl;
                    rOutStm << "#define " << C_PREF << aStr.GetBuffer() << "_DEF " << endl;
                }

                WriteTab( rOutStm, nTab );
                rOutStm << "typedef" << endl;
                if ( nT == WRITE_ODL )
                    SvMetaName::Write( rBase, rOutStm, nTab, nT, nA );
            }
            WriteTab( rOutStm, nTab );
            if( nType == TYPE_STRUCT )
                rOutStm << "struct";
            else if( nType == TYPE_UNION )
                rOutStm << "union";
            else
                rOutStm << "enum";
            if( nT != WRITE_ODL && nT != WRITE_C_HEADER)
                rOutStm << ' ' << name.GetBuffer();

            rOutStm << endl;
            WriteTab( rOutStm, nTab );
            rOutStm << '{' << endl;
            WriteContext( rBase, rOutStm, nTab +1, nT, nA );
            WriteTab( rOutStm, nTab );
            rOutStm << '}';
            if( nT == WRITE_ODL || nT == WRITE_C_HEADER )
            {
                rOutStm << ' ' << C_PREF << name.GetBuffer();
            }
            rOutStm << ';' << endl;

            if ( nT == WRITE_C_HEADER )
                rOutStm << "#endif";
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

BOOL SvMetaType::ReadNamesSvIdl( SvIdlDataBase & rBase,
                                     SvTokenStream & rInStm )
{
    BOOL bOk = ReadNameSvIdl( rBase, rInStm );

    return bOk;
}

void SvMetaType::WriteHeaderSvIdl( SvIdlDataBase & rBase,
                                   SvStream & rOutStm,
                                   USHORT nTab )
{
    switch( nType )
    {
        case TYPE_CLASS:
        {
            if( IsShell() )
                rOutStm << SvHash_shell()->GetName().GetBuffer();
            else
                rOutStm << SvHash_interface()->GetName().GetBuffer();
            rOutStm << ' ' << GetName().GetBuffer();
        }
        break;
        case TYPE_STRUCT:
        {
            rOutStm << SvHash_struct()->GetName().GetBuffer()
                    << ' ' << GetName().GetBuffer();
        }
        break;
        case TYPE_UNION:
        {
            rOutStm << SvHash_union()->GetName().GetBuffer()
                    << ' ' << GetName().GetBuffer();
        }
        break;
        case TYPE_ENUM:
        {
            rOutStm << SvHash_enum()->GetName().GetBuffer()
                    << ' ' << GetName().GetBuffer();
        }
        break;
        case TYPE_POINTER:
        case TYPE_BASE:
        {
            if( IsItem() )
                rOutStm << SvHash_item()->GetName().GetBuffer() << ' ';
            else
                rOutStm << SvHash_typedef()->GetName().GetBuffer() << ' ';
            if( GetRef() )
            {
                ((SvMetaType *)GetRef())->WriteTheType( rBase, rOutStm, nTab, WRITE_IDL );
                rOutStm << ' ';
            }
            rOutStm << GetName().GetBuffer();
        }
        break;
        case TYPE_METHOD:
        {
            rOutStm << SvHash_typedef()->GetName().GetBuffer() << ' ';
            ((SvMetaType *)GetRef())->WriteTheType( rBase, rOutStm, nTab, WRITE_IDL );
            rOutStm << ' ' << GetName().GetBuffer() << "( ";
            WriteContextSvIdl( rBase, rOutStm, nTab );
            rOutStm << " )";
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
                                       USHORT nTab )
{
    SvMetaExtern::WriteAttributesSvIdl( rBase, rOutStm, nTab );
    ByteString name = GetName();
    if( aSvName != name || aSbxName != name || aOdlName != name )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << "class SvMetaType" << endl;
        if( aSvName != name )
        {
            WriteTab( rOutStm, nTab );
            aSvName.WriteSvIdl( SvHash_SvName(), rOutStm, nTab );
            rOutStm << endl;
        }
        if( aSbxName != name )
        {
            WriteTab( rOutStm, nTab );
            aSbxName.WriteSvIdl( SvHash_SbxName(), rOutStm, nTab );
            rOutStm << endl;
        }
        if( aOdlName != name )
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
            GetAttrList().Append( xAttr );
    }
}

void SvMetaType::WriteContextSvIdl
(
    SvIdlDataBase & rBase,
    SvStream & rOutStm,
    USHORT nTab
)
{
    if( GetAttrCount() )
    {
        SvMetaAttribute * pAttr = pAttrList->First();
        while( pAttr )
        {
            WriteTab( rOutStm, nTab );
            pAttr->WriteSvIdl( rBase, rOutStm, nTab );
            if( GetType() == TYPE_METHOD )
                rOutStm << ',' << endl;
            else
                rOutStm << ';' << endl;
            pAttr = pAttrList->Next();
        }
    }
}

void SvMetaType::WriteAttributes( SvIdlDataBase & rBase, SvStream & rOutStm,
                                   USHORT nTab,
                                 WriteType nT, WriteAttribute nA )
{
    SvMetaExtern::WriteAttributes( rBase, rOutStm, nTab, nT, nA );
}

ULONG SvMetaType::MakeSfx( ByteString * pAttrArray )
{
    ULONG nC = 0;

    if( GetBaseType()->GetType() == TYPE_STRUCT )
    {
        ULONG nAttrCount = GetAttrCount();
        // write the single attributes
        for( ULONG n = 0; n < nAttrCount; n++ )
        {
            nC += pAttrList->GetObject( n )->MakeSfx( pAttrArray );
            if( n +1 < nAttrCount )
                *pAttrArray += ", ";
        }
    }
    return nC;
}

void SvMetaType::WriteSfxItem(
    const ByteString & rItemName, SvIdlDataBase &, SvStream & rOutStm )
{
    WriteStars( rOutStm );
    ByteString aVarName = " a";
    aVarName += rItemName;
    aVarName += "_Impl";

    ByteString  aTypeName = "SfxType";
    ByteString  aAttrArray;
    ULONG   nAttrCount = MakeSfx( &aAttrArray );
    ByteString  aAttrCount( ByteString::CreateFromInt32( nAttrCount ) );
    aTypeName += aAttrCount;

    rOutStm << "extern " << aTypeName.GetBuffer()
            << aVarName.GetBuffer() << ';' << endl;

    // write the implementation part
    rOutStm << "#ifdef SFX_TYPEMAP" << endl
            << aTypeName.GetBuffer() << aVarName.GetBuffer()
            << " = " << endl;
    rOutStm << '{' << endl
            << "\tTYPE(" << rItemName.GetBuffer() << "), "
            << aAttrCount.GetBuffer();
    if( nAttrCount )
    {
        rOutStm << ", { ";
        // write the single attributes
        rOutStm << aAttrArray.GetBuffer();
        rOutStm << " }";
    }
    rOutStm << endl << "};" << endl
            << "#endif" << endl << endl;
}

void SvMetaType::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    if( IsItem() )
    {
        if( GetBaseType()->GetType() == TYPE_STRUCT )
            GetBaseType()->WriteSfxItem( GetName(), rBase, rOutStm );
        else
            WriteSfxItem( GetName(), rBase, rOutStm );
    }
}

BOOL SvMetaType::ReadMethodArgs( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm )
{
    UINT32  nTokPos = rInStm.Tell();
    if( rInStm.Read( '(' ) )
    {
        DoReadContextSvIdl( rBase, rInStm );
        if( rInStm.Read( ')' ) )
        {
            SetType( TYPE_METHOD );
            return TRUE;
        }
    }
    rInStm.Seek( nTokPos );
    return FALSE;
}

void SvMetaType::WriteMethodArgs
(
    SvIdlDataBase & rBase,
    SvStream & rOutStm,
    USHORT nTab, WriteType nT
)
{
    if( nT == WRITE_IDL )
    {
        if( GetAttrCount() )
        {
            rOutStm << endl;
            WriteTab( rOutStm, nTab );
            rOutStm << '(' << endl;

            SvMetaAttribute * pAttr = pAttrList->First();
            while( pAttr )
            {
                WriteTab( rOutStm, nTab +1 );
                pAttr->WriteSvIdl( rBase, rOutStm, nTab +1 );
                pAttr = pAttrList->Next();
                if( pAttr )
                       rOutStm << ',' << endl;
            }
            rOutStm << endl;
            WriteTab( rOutStm, nTab );
            rOutStm << ')';
        }
        else
            rOutStm << "()";
    }
    else if ( nT == WRITE_DOCU )
    {

        rOutStm << '(';
        if( GetAttrCount() )
        {
            SvMetaAttribute * pAttr = pAttrList->First();
            while( pAttr )
            {
                pAttr->WriteParam( rBase, rOutStm, nTab+1, nT );
                pAttr = pAttrList->Next();
                if( pAttr )
                    rOutStm << ',';
                else
                    rOutStm << ' ';
            }
        }
        rOutStm << ')' << endl;
    }
    else
    {
        rOutStm << '(';
        if( nT == WRITE_C_HEADER || nT == WRITE_C_SOURCE )
        {
            rOutStm << ' ' << C_PREF << "Object h" << rBase.aIFaceName.GetBuffer();
            if( GetAttrCount() )
                rOutStm << ',';
            else
                rOutStm << ' ';
        }

        if( GetAttrCount() )
        {
            rOutStm << endl;
            SvMetaAttribute * pAttr = pAttrList->First();
            while( pAttr )
            {
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
                        DBG_ASSERT( FALSE, "WriteType not implemented" );
                    }
                }
                pAttr = pAttrList->Next();
                if( pAttr )
                       rOutStm << ',' << endl;
            }
            if( nT != WRITE_C_HEADER && nT != WRITE_C_SOURCE )
            {
                rOutStm << endl;
                WriteTab( rOutStm, nTab +1 );
            }
            rOutStm << ' ';
        }
        rOutStm << ')';
    }
}

void SvMetaType::WriteTypePrefix( SvIdlDataBase & rBase, SvStream & rOutStm,
                                 USHORT nTab, WriteType nT )
{
    switch( nT )
    {
        case WRITE_IDL:
        {
            if( GetIn() && GetOut() )
                rOutStm << SvHash_inout()->GetName().GetBuffer() << ' ';
            else if( GetIn() )
                rOutStm << SvHash_in()->GetName().GetBuffer() << ' ';
            else if( GetOut() )
                rOutStm << SvHash_out()->GetName().GetBuffer() << ' ';
            rOutStm << GetCString().GetBuffer();
        }
        break;

        case WRITE_ODL:
        {
            BOOL bIn = GetIn();
            BOOL bOut = GetOut();
            if( bIn || bOut )
            {
                if( bIn && bOut )
                    rOutStm << "[in,out] ";
                else if( bIn )
                    rOutStm << "[in] ";
                else if( bOut )
                    rOutStm << "[out] ";
            }

            ByteString out;
            if( GetType() == TYPE_METHOD )
                out = GetReturnType()->GetBaseType()->GetOdlName();
            else
            {
                SvMetaType * pBType = GetBaseType();
                out = pBType->GetOdlName();
            }
            if( aCall0 == (int)CALL_POINTER
              || aCall0 == (int)CALL_REFERENCE )
                rOutStm << " *";
            if( aCall1 == (int)CALL_POINTER
              || aCall1 == (int)CALL_REFERENCE )
                rOutStm << " *";
            rOutStm << out.GetBuffer();
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
                    rOutStm << C_PREF << pBaseType->GetName().GetBuffer()
                            << " *";
                else
                {
                    if ( pBaseType->GetType() == TYPE_ENUM )
                        rOutStm << C_PREF;
                    rOutStm << pBaseType->GetCName().GetBuffer();
                }
            }
            else
            {
                if( TYPE_STRUCT == pBaseType->GetType() )
                    rOutStm << pBaseType->GetName().GetBuffer() << " *";
                else
                    rOutStm << pBaseType->GetName().GetBuffer();
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
                    rOutStm << "VARIANT" << pBaseType->GetName().GetBuffer();
                else if ( pBaseType->GetType() == TYPE_ENUM )
                    rOutStm << "integer";
                else
                    rOutStm << pBaseType->GetOdlName().GetBuffer();
            }
        }

        default:
        {
            DBG_ASSERT( FALSE, "WriteType not implemented" );
        }
    }
}

void SvMetaType::WriteTheType( SvIdlDataBase & rBase, SvStream & rOutStm,
                             USHORT nTab, WriteType nT )
{
    WriteTypePrefix( rBase, rOutStm, nTab, nT );
    if( GetType() == TYPE_METHOD )
        WriteMethodArgs( rBase, rOutStm, nTab +2, nT );
}

ByteString SvMetaType::GetParserString() const
{
    SvMetaType * pBT = GetBaseType();
    if( pBT != this )
        return pBT->GetParserString();

    int type = GetType();
    ByteString aPStr;

    if( TYPE_METHOD == type || TYPE_STRUCT == type )
    {
        ULONG nAttrCount = GetAttrCount();
        // write the single attributes
        for( ULONG n = 0; n < nAttrCount; n++ )
        {
            SvMetaAttribute * pT = pAttrList->GetObject( n );
            aPStr += pT->GetType()->GetParserString();
        }
    }
    else
        aPStr = GetParserChar();
    return aPStr;
}

void SvMetaType::WriteParamNames( SvIdlDataBase & rBase,
                                   SvStream & rOutStm,
                                   const ByteString & rChief )
{
    SvMetaType * pBT = GetBaseType();
    if( pBT != this )
        pBT->WriteParamNames( rBase, rOutStm, rChief );
    else
    {
        int type = GetType();
        ByteString aPStr;

        if( TYPE_METHOD == type || TYPE_STRUCT == type )
        {
            ULONG nAttrCount = GetAttrCount();
            // write the single attributes
            for( ULONG n = 0; n < nAttrCount; n++ )
            {
                SvMetaAttribute * pA = pAttrList->GetObject( n );
                ByteString aStr = pA->GetName();
                pA->GetType()->WriteParamNames( rBase, rOutStm, aStr );
                if( n +1 < nAttrCount )
                    rOutStm << ", ";
            }
        }
        else
            rOutStm << rChief.GetBuffer();
    }
}

#endif // IDL_COMPILER

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

    BYTE nMask;
    rStm >> nMask;
    if( nMask >= 0x02 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "wrong format" );
        return;
    }
    if( nMask & 0x01 ) rStm.ReadByteString( aEnumValue );
}

void SvMetaEnumValue::Save( SvPersistStream & rStm )
{
    SvMetaName::Save( rStm );

    // create mask
    BYTE nMask = 0;
    if( aEnumValue.Len() ) nMask |= 0x01;

    // write data
    rStm << nMask;
    if( nMask & 0x01 ) rStm.WriteByteString( aEnumValue );
}

#ifdef IDL_COMPILER
BOOL SvMetaEnumValue::ReadSvIdl( SvIdlDataBase & rBase,
                                 SvTokenStream & rInStm )
{
    if( !ReadNameSvIdl( rBase, rInStm ) )
        return FALSE;
    return TRUE;
}

void SvMetaEnumValue::WriteSvIdl( SvIdlDataBase &, SvStream & rOutStm, USHORT )
{
    rOutStm << GetName().GetBuffer();
}

void SvMetaEnumValue::Write( SvIdlDataBase &, SvStream & rOutStm, USHORT,
                             WriteType nT, WriteAttribute )
{
    if ( nT == WRITE_C_HEADER || nT == WRITE_C_SOURCE )
        rOutStm << C_PREF << GetName().GetBuffer();
    else
        rOutStm << GetName().GetBuffer();
}
#endif // IDL_COMPILER

SV_IMPL_META_FACTORY1( SvMetaTypeEnum, SvMetaType );
SvMetaTypeEnum::SvMetaTypeEnum()
{
    SetBasicName( "Integer" );
}

void SvMetaTypeEnum::Load( SvPersistStream & rStm )
{
    SvMetaType::Load( rStm );

    BYTE nMask;
    rStm >> nMask;
    if( nMask >= 0x04 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "wrong format" );
        return;
    }
    if( nMask & 0x01 ) rStm >> aEnumValueList;
    if( nMask & 0x02 ) rStm.ReadByteString( aPrefix );
}

void SvMetaTypeEnum::Save( SvPersistStream & rStm )
{
    SvMetaType::Save( rStm );

    // create mask
    BYTE nMask = 0;
    if( aEnumValueList.Count() )    nMask |= 0x01;
    if( aPrefix.Len() )             nMask |= 0x02;

    // write data
    rStm << nMask;
    if( nMask & 0x01 ) rStm << aEnumValueList;
    if( nMask & 0x02 ) rStm.WriteByteString( aPrefix );
}

#ifdef IDL_COMPILER
void SvMetaTypeEnum::ReadContextSvIdl( SvIdlDataBase & rBase,
                                       SvTokenStream & rInStm )
{
    UINT32 nTokPos = rInStm.Tell();

    SvMetaEnumValueRef aEnumVal = new SvMetaEnumValue();
    BOOL bOk = aEnumVal->ReadSvIdl( rBase, rInStm );
    if( bOk )
    {
        if( 0 == aEnumValueList.Count() )
           // the first
           aPrefix = aEnumVal->GetName();
        else
        {
            USHORT nPos = aPrefix.Match( aEnumVal->GetName() );
            if( nPos != aPrefix.Len() && nPos != STRING_MATCH )
                aPrefix.Erase( nPos );
        }
        aEnumValueList.Append( aEnumVal );
    }
    if( !bOk )
        rInStm.Seek( nTokPos );
}

void SvMetaTypeEnum::WriteContextSvIdl( SvIdlDataBase & rBase,
                                        SvStream & rOutStm,
                                        USHORT nTab )
{
    WriteTab( rOutStm, nTab +1 );
    for( ULONG n = 0; n < aEnumValueList.Count(); n++ )
    {
        aEnumValueList.GetObject( n )->WriteSvIdl( rBase, rOutStm, nTab );
        if( n +1 != aEnumValueList.Count() )
            rOutStm << ", ";
        else
            rOutStm << endl;
    }
}

BOOL SvMetaTypeEnum::ReadSvIdl( SvIdlDataBase & rBase,
                                SvTokenStream & rInStm )
{
    UINT32  nTokPos = rInStm.Tell();
    if( SvMetaType::ReadHeaderSvIdl( rBase, rInStm )
      && GetType() == TYPE_ENUM )
    {
        if( SvMetaName::ReadSvIdl( rBase, rInStm ) )
             return TRUE;
    }
    rInStm.Seek( nTokPos );
    return FALSE;
}

void SvMetaTypeEnum::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                                   USHORT nTab )
{
    WriteHeaderSvIdl( rBase, rOutStm, nTab );
    rOutStm << endl;
    SvMetaName::WriteSvIdl( rBase, rOutStm, nTab );
    rOutStm << endl;
}

void SvMetaTypeEnum::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                            USHORT nTab,
                             WriteType nT, WriteAttribute nA )
{
    SvMetaType::Write( rBase, rOutStm, nTab, nT, nA );
}

void SvMetaTypeEnum::WriteContext( SvIdlDataBase & rBase, SvStream & rOutStm,
                                   USHORT nTab,
                                 WriteType nT, WriteAttribute nA )
{
    WriteTab( rOutStm, nTab +1 );
    for( ULONG n = 0; n < aEnumValueList.Count(); n++ )
    {
        aEnumValueList.GetObject( n )->Write( rBase, rOutStm, nTab +1, nT, nA );

        if( n +1 != aEnumValueList.Count() )
        {
            if( 2 == n % 3 )
            {
                rOutStm << ',' << endl;
                WriteTab( rOutStm, nTab +1 );
            }
            else
                rOutStm << ",\t";
        }
        else
            rOutStm << endl;
    }
    rOutStm << endl;
}

#endif // IDL_COMPILER

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

ByteString SvMetaAttribute::Compare( SvMetaAttribute* pAttr )
{
    ByteString aStr;

    if ( aType.Is() )
    {
        if ( aType->GetType() == TYPE_METHOD )
        {
            // Test only when the attribute is a method not if it has one!
            if ( !pAttr->GetType()->GetType() == TYPE_METHOD )
                aStr += "    IsMethod\n";
            else if ( aType->GetReturnType() &&
                aType->GetReturnType()->GetType() != pAttr->GetType()->GetReturnType()->GetType() )
                    aStr += "    ReturnType\n";

            if ( aType->GetAttrCount() )
            {
                ULONG nCount = aType->GetAttrCount();
                SvMetaAttributeMemberList& rList = aType->GetAttrList();
                SvMetaAttributeMemberList& rOtherList = pAttr->GetType()->GetAttrList();
                if ( pAttr->GetType()->GetAttrCount() != nCount )
                {
                    aStr += "    AttributeCount\n";
                }
                else
                {
                    for ( USHORT n=0; n<nCount; n++ )
                    {
                        SvMetaAttribute *pAttr1 = rList.GetObject(n);
                        SvMetaAttribute *pAttr2 = rOtherList.GetObject(n);
                        pAttr1->Compare( pAttr2 );
                    }
                }
            }
        }

        if ( GetType()->GetType() != pAttr->GetType()->GetType() )
            aStr += "    Type\n";

        if ( !GetType()->GetSvName().Equals( pAttr->GetType()->GetSvName() ) )
            aStr += "    ItemType\n";
    }

    if ( GetExport() != pAttr->GetExport() )
        aStr += "    Export\n";

    if ( GetAutomation() != pAttr->GetAutomation() )
        aStr += "    Automation\n";

    if ( GetIsCollection() != pAttr->GetIsCollection() )
        aStr += "    IsCollection\n";

    if ( GetReadOnlyDoc() != pAttr->GetReadOnlyDoc() )
        aStr += "    ReadOnlyDoc\n";

    if ( GetExport() && GetReadonly() != pAttr->GetReadonly() )
        aStr += "    Readonly\n";

    return aStr;
}

void SvMetaAttribute::WriteCSV( SvIdlDataBase&, SvStream& rStrm )
{
    rStrm << GetType()->GetSvName().GetBuffer() << ' ';
    rStrm << GetName().GetBuffer() << ' ';
    rStrm << GetSlotId().GetBuffer();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
