/*************************************************************************
 *
 *  $RCSfile: types.cxx,v $
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

#include <tools/debug.hxx>

#include <attrib.hxx>
#include <types.hxx>
#include <globals.hxx>
#include <database.hxx>
#pragma hdrstop

/****************** SvMetaAttribute *************************************/
/************************************************************************/
SV_IMPL_META_FACTORY1( SvMetaAttribute, SvMetaReference );
#ifdef IDL_COMPILER
SvAttributeList & SvMetaAttribute::GetAttributeList()
{
    if( !pAttribList )
    {
        pAttribList = new SvAttributeList();
    }
    return *pAttribList;
}
#endif
/*************************************************************************
|*    SvMetaAttribute::SvMetaAttribute()
|*
|*    Beschreibung
*************************************************************************/
SvMetaAttribute::SvMetaAttribute()
    : aExport( TRUE, FALSE )
    , aHidden( FALSE, FALSE )
    , aIsCollection ( FALSE, FALSE )
    , aReadOnlyDoc ( TRUE, FALSE )
    , aAutomation( TRUE, FALSE )
    , bNewAttr( FALSE )
{
}

SvMetaAttribute::SvMetaAttribute( SvMetaType * pType )
    : aExport( TRUE, FALSE )
    , aHidden( FALSE, FALSE )
    , aAutomation( TRUE, FALSE )
    , aIsCollection ( FALSE, FALSE)
    , aReadOnlyDoc ( TRUE, FALSE)
    , aType( pType )
    , bNewAttr( FALSE )
{
}

/*************************************************************************
|*    SvMetaAttribute::Load()
|*    SvMetaAttribute::Save()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaAttribute::Load( SvPersistStream & rStm )
{
    SvMetaReference::Load( rStm );

    BYTE nMask;
    rStm >> nMask;
    if( nMask >= 0x100 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        DBG_ERROR( "wrong format" )
        return;
    }
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

    // Maske erstellen
    BYTE nMask = 0;
    if( aType.Is() )            nMask |= 0x1;
    if( aSlotId.IsSet() )       nMask |= 0x2;
    if( aExport.IsSet() )       nMask |= 0x4;
    if( aReadonly.IsSet() )     nMask |= 0x8;
    if( aAutomation.IsSet() )   nMask |= 0x10;
    if( aIsCollection.IsSet() ) nMask |= 0x20;
    if( aReadOnlyDoc.IsSet() )  nMask |= 0x40;
    if( aHidden.IsSet() )       nMask |= 0x80;

    // Daten schreiben
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

/*************************************************************************
|*    SvMetaAttribute::GetType()
|*
|*    Beschreibung
*************************************************************************/
SvMetaType * SvMetaAttribute::GetType() const
{
    if( aType.Is() || !GetRef() ) return aType;
    return ((SvMetaAttribute *)GetRef())->GetType();
}

/*************************************************************************
|*    SvMetaAttribute::GetSlotId()
|*
|*    Beschreibung
*************************************************************************/
const SvNumberIdentifier & SvMetaAttribute::GetSlotId() const
{
    if( aSlotId.IsSet() || !GetRef() ) return aSlotId;
    return ((SvMetaAttribute *)GetRef())->GetSlotId();
}

/*************************************************************************
|*    SvMetaAttribute::GetReadonly()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaAttribute::GetReadonly() const
{
    if( aReadonly.IsSet() || !GetRef() ) return aReadonly;
    return ((SvMetaAttribute *)GetRef())->GetReadonly();
}

/*************************************************************************
|*    SvMetaAttribute::GetExport()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaAttribute::GetExport() const
{
    if( aExport.IsSet() || !GetRef() ) return aExport;
    return ((SvMetaAttribute *)GetRef())->GetExport();
}

/*************************************************************************
|*    SvMetaAttribute::GetHidden()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaAttribute::GetHidden() const
{
    // Wenn Export gesetzt wurde, aber Hidden nicht, gilt der Default
    // aHidden = !aExport
    if ( aExport.IsSet() && !aHidden.IsSet() )
        return !aExport;
    else if( aHidden.IsSet() || !GetRef() )
        return aHidden;
    else
        return ((SvMetaAttribute *)GetRef())->GetHidden();
}

/*************************************************************************
|*    SvMetaAttribute::GetAutomation()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvMetaAttribute::IsMethod()
|*    SvMetaAttribute::IsVariable()
|*    SvMetaAttribute::GetMangleName()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaAttribute::IsMethod() const
{
    SvMetaType * pType = GetType();
    DBG_ASSERT( pType, "no type for attribute" )
    return pType->GetType() == TYPE_METHOD;
}

BOOL SvMetaAttribute::IsVariable() const
{
    SvMetaType * pType = GetType();
    return pType->GetType() != TYPE_METHOD;
}

ByteString SvMetaAttribute::GetMangleName( BOOL bVariable ) const
{
    return GetName();
}

/*************************************************************************
|*    SvMetaAttribute::FillSbxObject()
|*
|*    Beschreibung
*************************************************************************/
/*
void SvMetaAttribute::FillSbxObject( SbxInfo * pInfo, USHORT nSbxFlags )
{
    SvMetaType * pType = GetType();
    DBG_ASSERT( pType, "no type for attribute" )
    if( !nSbxFlags )
    { // Flags koennen vom Aufrufer ueberschrieben werden
        if( pType->GetOut() )
        {
            nSbxFlags |= SBX_WRITE;
            if( pType->GetIn() )
                nSbxFlags |= SBX_READ;
        }
        else
            nSbxFlags |= SBX_READ;
    }
    SvMetaType * pBaseType = pType->GetBaseType();
    DBG_ASSERT( pBaseType, "no base type for attribute" )
    if( pBaseType->GetType() == TYPE_STRUCT )
    {
        const SvMetaAttributeMemberList & rList = pBaseType->GetAttrList();
        ULONG nCount = rList.Count();
        for( ULONG i = 0; i < nCount; i++ )
            rList.GetObject( i )->FillSbxObject( pInfo, nSbxFlags );
    }
    else
        //MI: pInfo->AddParam( GetName(), pBaseType->GetSbxDataType(), nSbxFlags );
        pInfo->AddParam( GetName(), SbxVARIANT, nSbxFlags );
}
*/

/*************************************************************************
|*    SvMetaAttribute::FillSbxObject()
|*
|*    Beschreibung
*************************************************************************/
/*
void SvMetaAttribute::FillSbxObject( SvIdlDataBase & rBase,
                                    SbxObject * pObj, BOOL bVariable )
{
    // keine Attribut fuer Automation
    if( !GetAutomation() || !GetExport() )
        return;

    if( bVariable && IsVariable() )
    {
        SvMetaType * pType = GetType();
        DBG_ASSERT( pType, "no type for attribute" )
        SvMetaType * pBaseType = pType->GetBaseType();
        DBG_ASSERT( pBaseType, "no base type for attribute" )

        if( pBaseType->GetType() == TYPE_STRUCT )
        {
            SvNumberIdentifier aSlotId = rBase.aStructSlotId;
            if ( GetSlotId().Len() )
                rBase.aStructSlotId = GetSlotId();
            const SvMetaAttributeMemberList & rList = pBaseType->GetAttrList();
            ULONG nCount = rList.Count();
            for( ULONG i = 0; i < nCount; i++ )
                rList.GetObject( i )->FillSbxObject( rBase, pObj, bVariable );
            rBase.aStructSlotId = aSlotId;
        }
        else
        {
            SbxPropertyRef xProp = new SbxProperty( GetName(), SbxVARIANT );
                                        //MI: pBaseType->GetSbxDataType() );
            if ( GetReadonly() || IsMethod() )
                xProp->ResetFlag( SBX_WRITE );
            xProp->SetUserData( MakeSlotValue( rBase, TRUE ) );
            pType->FillSbxObject( xProp, bVariable );

            pObj->Insert( &xProp );
        }
    }
    else if( !bVariable && IsMethod() )
    {
        SvMetaType * pType = GetType();
        SvMetaType * pRetBaseType = GetType()->GetReturnType()->GetBaseType();
        SbxMethodRef xMeth = new SbxMethod( GetName(),
                            pRetBaseType->GetSbxDataType() );
        xMeth->ResetFlag( SBX_WRITE );
        xMeth->SetUserData( MakeSlotValue( rBase, FALSE ) );
        pType->FillSbxObject( xMeth, bVariable );

        pObj->Insert( &xMeth );
    }
}
*/
#ifdef IDL_COMPILER
/*************************************************************************
|*    SvMetaAttribute::Test()
|*
|*    Beschreibung
*************************************************************************/
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
    /*
    if( !GetType()->IsItem() && GetSlotId().IsSet() )
    {
        rBase.SetError( "slot without item declared", rInStm.GetToken() );
        rBase.WriteError( rInStm );
        bOk = FALSE;
    }
    */
    return bOk;
}

/*************************************************************************
|*    SvMetaAttribute::ReadSvIdl()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaAttribute::ReadSvIdl( SvIdlDataBase & rBase,
                                     SvTokenStream & rInStm )
{
    UINT32  nTokPos     = rInStm.Tell();
    SvToken * pTok  = rInStm.GetToken();
    if( !GetType() )
        // Es wurde kein Typ im ctor mitgegeben
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

/*************************************************************************
|*    SvMetaAttribute::WriteSvIdl()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvMetaAttribute::ReadAttributesSvIdl()
|*
|*    Beschreibung
*************************************************************************/
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
            // Fehler setzen
            rBase.SetError( "Readonly in function attribute", rInStm.GetToken() );
            rBase.WriteError( rInStm );
        }
    }
}

/*************************************************************************
|*    SvMetaAttribute::WriteAttributesSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaAttribute::WriteAttributesSvIdl
(
    SvIdlDataBase & rBase,
    SvStream & rOutStm,
    USHORT nTab
)
{
    SvMetaReference::WriteAttributesSvIdl( rBase, rOutStm, nTab );

    //aSlotId.WriteSvIdl( SvHash_SlotId(), rOutStm, nTab );
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

/*************************************************************************
|*    SvMetaAttribute::WriteParam()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaAttribute::WriteParam( SvIdlDataBase & rBase,
                                    SvStream & rOutStm,
                                    USHORT nTab,
                                    WriteType nT )
{
    SvMetaType * pType = GetType();
    DBG_ASSERT( pType, "no type for attribute" )
    SvMetaType * pBaseType = pType->GetBaseType();
    DBG_ASSERT( pBaseType, "no base type for attribute" )

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
//                  if ( nT == WRITE_DOCU )
                        rOutStm << ',';
//                  else
//                      rOutStm << ',' << endl;
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
                    DBG_ERROR( "Falscher Parametertyp!" );
                }
                else
                    rOutStm << pBaseType->GetBasicPostfix().GetBuffer();
            }
        }
    }
/*
    else if( nT == WRITE_C_HEADER || nT == WRITE_C_SOURCE )
    {
        pBaseType->WriteTypePrefix( rBase, rOutStm, nTab, nT );
        rOutStm << ' ';
        rOutStm << GetName().GetBuffer();
    }
*/
}

/*************************************************************************
|*    SvMetaSlot::WriteSlotId()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvMetaSlot::WriteAttributes()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvMetaAttribute::WriteCSource()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaAttribute::WriteCSource( SvIdlDataBase & rBase, SvStream & rOutStm,
                                    BOOL bSet )
{
    rOutStm << endl;
    SvMetaType * pType = GetType();
    SvMetaType * pBaseType = pType->GetBaseType();

    // Bei Set ist der Return immer void
    BOOL bVoid = bSet;
    if( pBaseType->GetType() == TYPE_METHOD )
        bVoid = pBaseType->GetReturnType()->GetBaseType()->GetName() == "void";

    // Methoden/Funktions-Body ausgeben
    rOutStm << '{' << endl;
    WriteTab( rOutStm, 1 );
//  rOutStm << "if( SvIPCIsConnected() )" << endl;
//  WriteTab( rOutStm, 1 );
//  rOutStm << '{' << endl;
//  WriteTab( rOutStm, 2 );

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
            // void SetPosSize( C_Object *, C_Rectangle * pRect );
            pBaseType->WriteParamNames( rBase, rOutStm, ByteString() );
        else if( bSet )
            pBaseType->WriteParamNames( rBase, rOutStm, GetName() );
    }

    rOutStm << " );" << endl;
//  WriteTab( rOutStm, 1 );
//  rOutStm << '}' << endl;
//  if( !bVoid )
//  {
//      WriteTab( rOutStm, 1 );
//      rOutStm << "return 0;" << endl;
//  }
    rOutStm << '}' << endl;
}

/*************************************************************************
|*    SvMetaAttribute::WriteRecursiv_Impl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaAttribute::WriteRecursiv_Impl( SvIdlDataBase & rBase,
                            SvStream & rOutStm, USHORT nTab,
                             WriteType nT, WriteAttribute nA )
{
    const SvMetaAttributeMemberList & rList = GetType()->GetBaseType()->GetAttrList();
    ULONG nCount = rList.Count();

    SvNumberIdentifier aSlotId = rBase.aStructSlotId;
    if ( GetSlotId().Len() )
        rBase.aStructSlotId = GetSlotId();

    // Offizielle Hack-Schnittstelle von MM: spezielle Schalter werden "uber
    // die WriteAttribute "ubergeben
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

    rBase.aStructSlotId = aSlotId;
}

/*************************************************************************
|*    SvMetaAttribute::Write()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaAttribute::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                            USHORT nTab,
                             WriteType nT, WriteAttribute nA )
{
    // keine Attribut fuer Automation
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
    DBG_ASSERT( pType, "no type for attribute" )
    SvMetaType * pBaseType = pType->GetBaseType();
    DBG_ASSERT( pBaseType, "no base type for attribute" )
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
            ByteString aName; // (rBase.GetActModulePrefix());
            aName += rBase.aIFaceName;
            aName += GetName();
            const char * pName = aName.GetBuffer();
            // Beispiel
            // void SetPosSize( C_Object *, C_Rectangle * );
            WriteTab( rOutStm, nTab );
            pBaseType->WriteTypePrefix( rBase, rOutStm, nTab, nT );
//          rOutStm << " SYSCALL";
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
                // Zur Hilfe den Namen des Properties als Kommentar ausgeben
                rOutStm << "/* " << GetName().GetBuffer() << " */" << endl;

                WriteRecursiv_Impl( rBase, rOutStm, nTab, nT, nA );
            }
            else
            {
                // Beispiel
                // void SetValue( C_Object *, USHORT n );
                ByteString aName = GetName();

                BOOL bReadonly = GetReadonly() || ( nA & WA_READONLY );
                if ( !bReadonly && !IsMethod() )
                {
                    // Zuweisung
                    WriteTab( rOutStm, nTab );
                    rOutStm << "void ";
//                  rOutStm << "SYSCALL ";
//                  if ( rBase.GetActModulePrefix().Len() )
//                      rOutStm << rBase.GetActModulePrefix().GetBuffer();
                    rOutStm << rBase.aIFaceName.GetBuffer()
                            << "Set" << aName.GetBuffer() << "( " << C_PREF
                            << "Object h" << rBase.aIFaceName.GetBuffer() << ", " << endl;
                    WriteTab( rOutStm, nTab+1 );
                    pBaseType->WriteTypePrefix( rBase, rOutStm, nTab, nT );
                    rOutStm << ' ' << aName.GetBuffer() << " )";
                    if( nT == WRITE_C_HEADER )
                        rOutStm << ';' << endl << endl;
                    else
                        WriteCSource( rBase, rOutStm, TRUE );
                }

                // Zugriff
                WriteTab( rOutStm, nTab );
                pBaseType->WriteTypePrefix( rBase, rOutStm, nTab, nT );
                rOutStm << ' ';
//              rOutStm << "SYSCALL ";
//              if ( rBase.GetActModulePrefix().Len() )
//                  rOutStm << rBase.GetActModulePrefix().GetBuffer();
                rOutStm << rBase.aIFaceName.GetBuffer()
                        << "Get" << aName.GetBuffer() << "( " << C_PREF
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

            // Returntype
            SvMetaType* pType = GetType();
            SvMetaType* pBaseType = pType->GetBaseType();
            rOutStm << pBaseType->GetReturnType()->GetBaseType()->GetBasicName().GetBuffer() << endl;

            DBG_ASSERT( pBaseType->GetReturnType()->GetBaseType()->GetBasicName().Len(),
                "Leerer BasicName" );

            // Syntax
            rOutStm << GetName().GetBuffer();
            pType->WriteMethodArgs( rBase, rOutStm, nTab, nT );

            // C-Returntype
            pBaseType->WriteTypePrefix( rBase, rOutStm, 0, WRITE_C_HEADER );
            rOutStm << endl;

            // Bei Methoden auch C-Syntax
            rOutStm << "<C-SYNTAX>" << endl;
            Write( rBase, rOutStm, 0, WRITE_C_HEADER, nA );
            rOutStm << "</C-SYNTAX>" << endl;

            // Description
            WriteDescription( rBase, rOutStm );
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

                // Bei properties Type anstelle des return value
                rOutStm << pBaseType->GetBasicName().GetBuffer() << endl;

                DBG_ASSERT( pBaseType->GetBasicName().Len(),
                    "Leerer BasicName" );

                // Bei properties keine Syntax
                rOutStm << endl;

                // C-Returntype
                pBaseType->WriteTypePrefix( rBase, rOutStm, 0, WRITE_C_HEADER );
                rOutStm << endl;

                // Description
                WriteDescription( rBase, rOutStm );
                rOutStm << "</PROPERTY>" << endl << endl;
            }
        }
    }
}

/*************************************************************************
|*    SvMetaAttribute::MakeSfx()
|*
|*    Beschreibung
*************************************************************************/
ULONG SvMetaAttribute::MakeSfx( ByteString * pAttrArray )
{
    SvMetaType * pType = GetType();
    DBG_ASSERT( pType, "no type for attribute" )
    SvMetaType * pBaseType = pType->GetBaseType();
    DBG_ASSERT( pBaseType, "no base type for attribute" )
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

/*************************************************************************
|*    SvMetaAttribute::WriteSrc()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaAttribute::WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                                  Table * pIdTable )
{
}

void SvMetaAttribute::WriteHelpId( SvIdlDataBase & rBase, SvStream & rOutStm,
                                  Table * pIdTable )
{
}

#endif // IDL_COMPILER

/****************** SvMetaType *************************************/
/************************************************************************/
SV_IMPL_META_FACTORY1( SvMetaType, SvMetaExtern );
#ifdef IDL_COMPILER
SvAttributeList & SvMetaType::GetAttributeList()
{
    if( !pAttribList )
    {
        pAttribList = new SvAttributeList();
    }
    return *pAttribList;
}
#endif
/*************************************************************************
|*    SvMetaType::SvMetaType()
|*
|*    Beschreibung
*************************************************************************/
#define CTOR                            \
    : aCall0( CALL_VALUE, FALSE )       \
    , aCall1( CALL_VALUE, FALSE )       \
    , aSbxDataType( 0, FALSE )          \
    , pAttrList( NULL )                 \
    , nType( TYPE_BASE )                \
    , bIsShell( FALSE )                 \
    , bIsItem( FALSE )                  \
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
//    aSbxDataType = (int)nT;
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

    // Maske erstellen
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

    // Daten schreiben
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

/*************************************************************************
|*    SvMetaType::GetAttrList()
|*
|*    Beschreibung
*************************************************************************/
SvMetaAttributeMemberList & SvMetaType::GetAttrList() const
{
    if( !pAttrList )
        ((SvMetaType *)this)->pAttrList = new SvMetaAttributeMemberList();
    return *pAttrList;
}

/*************************************************************************
|*    SvMetaType::SetType()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaType::SetType( int nT )
{
    nType = nT;
    if( nType == TYPE_ENUM )
    {
        aOdlName = "short";
//        aSbxDataType = SbxINTEGER;
    }
    else if( nType == TYPE_CLASS )
    {
        aCName = C_PREF;
        aCName += "Object *";
    }
}

/*************************************************************************
|*    SvMetaType::GetBaseType()
|*
|*    Beschreibung
*************************************************************************/
SvMetaType * SvMetaType::GetBaseType() const
{
    if( GetRef() && GetType() == TYPE_BASE )
        return ((SvMetaType *)GetRef())->GetBaseType();
    return (SvMetaType *)this;
}

/*************************************************************************
|*    SvMetaType::GetReturnType()
|*
|*    Beschreibung
*************************************************************************/
SvMetaType * SvMetaType::GetReturnType() const
{
    DBG_ASSERT( GetType() == TYPE_METHOD, "no method" )
    DBG_ASSERT( GetRef(), "no return type" )
    return (SvMetaType *)GetRef();
}

/*************************************************************************
|*    SvMetaType::GetSbxDataType()
|*
|*    Beschreibung
*************************************************************************/
/*
SbxDataType SvMetaType::GetSbxDataType() const
{
    if( aSbxDataType.IsSet() || !GetRef() )
        return (SbxDataType)(int)aSbxDataType;
    else
        return ((SvMetaType *)GetRef())->GetSbxDataType();
}
*/
/*************************************************************************
|*    SvMetaType::GetBasicName()
|*
|*    Beschreibung
*************************************************************************/
const ByteString& SvMetaType::GetBasicName() const
{
    if( aBasicName.IsSet() || !GetRef() )
        return aBasicName;
    else
        return ((SvMetaType*)GetRef())->GetBasicName();
}

/*************************************************************************
|*    SvMetaType::GetBasicPostfix()
|*
|*    Beschreibung
*************************************************************************/
ByteString SvMetaType::GetBasicPostfix() const
{

    ByteString aRet;

    // MBN und Co wollen immer "As xxx"
/*
    if( aBasicPostfix.IsSet() || !GetRef() )
        aRet = aBasicPostfix;
    else
        aRet = ((SvMetaType*)GetRef())->GetBasicPostfix();

    if ( !aRet.Len() && GetBasicName().Len() )
*/
    {

        aRet = " As ";
        aRet += GetBasicName();
    }

    return aRet;
}

/*************************************************************************
|*    SvMetaType::GetIn()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaType::GetIn() const
{
    if( aIn.IsSet() || !GetRef() )
        return aIn;
    else
        return ((SvMetaType *)GetRef())->GetIn();
}

/*************************************************************************
|*    SvMetaType::GetOut()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaType::GetOut() const
{
    if( aOut.IsSet() || !GetRef() )
        return aOut;
    else
        return ((SvMetaType *)GetRef())->GetOut();
}

/*************************************************************************
|*    SvMetaType::SetCall0()
|*
|*    Beschreibung
*************************************************************************/
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
                    "set no base type to pointer" )
        SetType( TYPE_POINTER );
    }
}

/*************************************************************************
|*    SvMetaType::GetCall0()
|*
|*    Beschreibung
*************************************************************************/
int SvMetaType::GetCall0() const
{
    if( aCall0.IsSet() || !GetRef() )
        return aCall0;
    else
        return ((SvMetaType *)GetRef())->GetCall0();
}

/*************************************************************************
|*    SvMetaType::SetCall1()
|*
|*    Beschreibung
*************************************************************************/
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
                    "set no base type to pointer" )
        SetType( TYPE_POINTER );
    }
}

/*************************************************************************
|*    SvMetaType::GetCall1()
|*
|*    Beschreibung
*************************************************************************/
int SvMetaType::GetCall1() const
{
    if( aCall1.IsSet() || !GetRef() )
        return aCall1;
    else
        return ((SvMetaType *)GetRef())->GetCall1();
}

/*************************************************************************
|*    SvMetaType::GetSvName()
|*
|*    Beschreibung
*************************************************************************/
const ByteString & SvMetaType::GetSvName() const
{
    if( aSvName.IsSet() || !GetRef() )
        return aSvName;
    else
        return ((SvMetaType *)GetRef())->GetSvName();
}

/*************************************************************************
|*    SvMetaType::GetSbxName()
|*
|*    Beschreibung
*************************************************************************/
const ByteString & SvMetaType::GetSbxName() const
{
    if( aSbxName.IsSet() || !GetRef() )
        return aSbxName;
    else
        return ((SvMetaType *)GetRef())->GetSbxName();
}

/*************************************************************************
|*    SvMetaType::GetOdlName()
|*
|*    Beschreibung
*************************************************************************/
const ByteString & SvMetaType::GetOdlName() const
{
    if( aOdlName.IsSet() || !GetRef() )
        return aOdlName;
    else
        return ((SvMetaType *)GetRef())->GetOdlName();
}

/*************************************************************************
|*    SvMetaType::GetCName()
|*
|*    Beschreibung
*************************************************************************/
const ByteString & SvMetaType::GetCName() const
{
    if( aCName.IsSet() || !GetRef() )
        return aCName;
    else
        return ((SvMetaType *)GetRef())->GetCName();
}

/*************************************************************************
|*    SvMetaType::SetName()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaType::SetName( const ByteString & rName, SvIdlDataBase * pBase )
{
    aSvName     = rName;
    aSbxName    = rName;
    aCName      = rName;
    if( GetType() != TYPE_ENUM )
        aOdlName = rName;
    return SvMetaReference::SetName( rName, pBase );
}

/*************************************************************************
|*    SvMetaType::FillSbxObject()
|*
|*    Beschreibung
*************************************************************************/
/*
void SvMetaType::FillSbxObject( SbxVariable * pObj, BOOL bVariable )
{
    if( PTR_CAST( SbxMethod, pObj ) )
    {
        if( GetType() == TYPE_METHOD )
        {
            ULONG nCount = GetAttrCount();
            if( nCount )
            {
                SbxInfoRef xInfo = pObj->GetInfo();
                if( !xInfo.Is() )
                {
                    xInfo = new SbxInfo();
                    pObj->SetInfo( xInfo );
                }
                for( ULONG n = nCount; n > 0; n-- )
                    pAttrList->GetObject( n -1 )->FillSbxObject( xInfo );
            }
        }
    }
}
*/
#ifdef IDL_COMPILER
/*************************************************************************
|*    SvMetaType::GetString()
|*
|*    Beschreibung
*************************************************************************/
ByteString SvMetaType::GetCString() const
{
    ByteString aOut( GetSvName() );
    if( aCall0 == (int)CALL_POINTER )
        aOut += " *";
    else if( aCall0 == (int)CALL_REFERENCE )
        aOut += " &";
    if( aCall1 == (int)CALL_POINTER )
        aOut += '*';
    else if( aCall1 == (int)CALL_REFERENCE )
        aOut += '&';
    return aOut;
}

/*************************************************************************
|*    SvMetaType::ReadHeaderSvIdl()
|*
|*    Beschreibung
*************************************************************************/
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
                /*      // um aufwaertskompatibel zu bleiben
                        aOdlName = pType->GetOdlName();
                */
                if( rInStm.Read( '(' ) )
                {
                    //DoReadContextSvIdl( rBase, rInStm, ',' );
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

/*************************************************************************
|*    SvMetaType::ReadSvIdl()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvMetaType::WriteSvIdl()
|*
|*    Beschreibung
*************************************************************************/
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
        // nichts geschrieben
        rOutStm.Seek( nOldPos );
    rOutStm << ';' << endl;
}

/*************************************************************************
|*    SvMetaType::WriteContext()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvMetaType::Write()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaType::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                          USHORT nTab,
                         WriteType nT, WriteAttribute nA )
{
    if( nT == WRITE_C_HEADER && nType != TYPE_ENUM )
        // nur enum schreiben
        return;

    ByteString aName = GetName();
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
                    ByteString aStr = aName;
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
                rOutStm << ' ' << aName.GetBuffer();

            rOutStm << endl;
            WriteTab( rOutStm, nTab );
            rOutStm << '{' << endl;
            WriteContext( rBase, rOutStm, nTab +1, nT, nA );
            WriteTab( rOutStm, nTab );
            rOutStm << '}';
            if( nT == WRITE_ODL || nT == WRITE_C_HEADER )
            {
                rOutStm << ' ' << C_PREF << aName.GetBuffer();
            }
            rOutStm << ';' << endl;

            if ( nT == WRITE_C_HEADER )
                rOutStm << "#endif";

/*keine Funktionen fuer New und Delete
            if( nT == WRITE_C_HEADER && nType != TYPE_ENUM )
            {
                rOutStm << aName.GetBuffer() << " * New_"
                        << aName.GetBuffer() << "();" << endl;
                rOutStm << "void Delete_"
                        << aName.GetBuffer() << "( "
                        << aName.GetBuffer() <<" * );" << endl;
            }
*/
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

/*keine Funktionen fuer New und Delete
    if( nT == WRITE_C_SOURCE )
    {
        switch( nType )
        {
        case TYPE_STRUCT:
        case TYPE_UNION:
        {
            rOutStm << aName.GetBuffer() << " * New_"
                    << aName.GetBuffer() << "() { return new "
                    << aName.GetBuffer() << "; }" << endl;
            rOutStm << "void Delete_"
                    << aName.GetBuffer() << "( "
                    << aName.GetBuffer() <<" * ) { delete "
                    << aName.GetBuffer() << "; }" << endl;
        }
        break;
        }
    }
*/
}

/*************************************************************************
|*    SvMetaType::ReadNamesSvIdl()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaType::ReadNamesSvIdl( SvIdlDataBase & rBase,
                                     SvTokenStream & rInStm )
{
    BOOL bOk = ReadNameSvIdl( rBase, rInStm );
/*
    if( bOk )
    {
        UINT32 nTokPos = rInStm.Tell();
        SvToken * pTok = rInStm.GetToken_Next();

        if( pTok->IsIdentifier() )
        {
            aSbxName = pTok->GetString();

            nTokPos = rInStm.Tell();
            pTok = rInStm.GetToken_Next();
            if( pTok->IsIdentifier() )
            {
                aItemName = pTok->GetString();
                nTokPos = rInStm.Tell();
            }
        }
        rInStm.Seek( nTokPos );
    }
*/
    return bOk;
}

/*************************************************************************
|*    SvMetaType::WriteHeaderSvIdl()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvMetaType::ReadAttributesSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaType::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm )
{
    SvMetaExtern::ReadAttributesSvIdl( rBase, rInStm );
    aSvName.ReadSvIdl( SvHash_SvName(), rInStm );
    aSbxName.ReadSvIdl( SvHash_SbxName(), rInStm );
    aOdlName.ReadSvIdl( SvHash_OdlName(), rInStm );
}

/*************************************************************************
|*    SvMetaType::WriteAttributesSvIdl()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaType::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                       SvStream & rOutStm,
                                       USHORT nTab )
{
    SvMetaExtern::WriteAttributesSvIdl( rBase, rOutStm, nTab );
    ByteString aName = GetName();
    if( aSvName != aName || aSbxName != aName || aOdlName != aName )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << "class SvMetaType" << endl;
        if( aSvName != aName )
        {
            WriteTab( rOutStm, nTab );
            aSvName.WriteSvIdl( SvHash_SvName(), rOutStm, nTab );
            rOutStm << endl;
        }
        if( aSbxName != aName )
        {
            WriteTab( rOutStm, nTab );
            aSbxName.WriteSvIdl( SvHash_SbxName(), rOutStm, nTab );
            rOutStm << endl;
        }
        if( aOdlName != aName )
        {
            WriteTab( rOutStm, nTab );
            aOdlName.WriteSvIdl( SvHash_OdlName(), rOutStm, nTab );
            rOutStm << endl;
        }
    }
}

/*************************************************************************
|*    SvMetaType::ReadContextSvIdl()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvMetaType::WriteContextSvIdl()
|*
|*    Beschreibung
*************************************************************************/
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

/*************************************************************************
|*    SvMetaType::WriteAttributes()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaType::WriteAttributes( SvIdlDataBase & rBase, SvStream & rOutStm,
                                   USHORT nTab,
                                 WriteType nT, WriteAttribute nA )
{
    SvMetaExtern::WriteAttributes( rBase, rOutStm, nTab, nT, nA );
}

/*************************************************************************
|*    SvMetaType::MakeSfx()
|*
|*    Beschreibung
*************************************************************************/
ULONG SvMetaType::MakeSfx( ByteString * pAttrArray )
{
    ULONG nC = 0;

    if( GetBaseType()->GetType() == TYPE_STRUCT )
    {
        ULONG nAttrCount = GetAttrCount();
        // Die einzelnen Attribute schreiben
        for( ULONG n = 0; n < nAttrCount; n++ )
        {
            nC += pAttrList->GetObject( n )->MakeSfx( pAttrArray );
            if( n +1 < nAttrCount )
                *pAttrArray += ", ";
        }
    }
    return nC;
}

/*************************************************************************
|*    SvMetaType::WriteSfx()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaType::WriteSfx( const ByteString & rItemName, SvIdlDataBase & rBase,
                            SvStream & rOutStm )
{
    WriteStars( rOutStm );
    ByteString aVarName = " a";
    aVarName += rItemName;
    aVarName += "_Impl";

    ByteString  aTypeName = "SfxType";
    ByteString  aAttrArray;
    ULONG   nAttrCount = MakeSfx( &aAttrArray );
    //ULONG nAttrCount = GetAttrCount();
    ByteString  aAttrCount( ByteString::CreateFromInt32( nAttrCount ) );
    aTypeName += aAttrCount;

    rOutStm << "extern " << aTypeName.GetBuffer()
            << aVarName.GetBuffer() << ';' << endl;

    // Den Implementationsteil schreiben
    rOutStm << "#ifdef SFX_TYPEMAP" << endl
            << aTypeName.GetBuffer() << aVarName.GetBuffer()
            << " = " << endl;
    rOutStm << '{' << endl
            << "\tTYPE(" << rItemName.GetBuffer() << "), "
            << aAttrCount.GetBuffer();
    if( nAttrCount )
    {
        rOutStm << ", { ";
        // Die einzelnen Attribute schreiben
        rOutStm << aAttrArray.GetBuffer();
        rOutStm << " }";
    }
    rOutStm << endl << "};" << endl
            << "#endif" << endl << endl;
}

/*************************************************************************
|*    SvMetaType::WriteSfx()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaType::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    if( IsItem() )
    {
        if( GetBaseType()->GetType() == TYPE_STRUCT )
            GetBaseType()->WriteSfx( GetName(), rBase, rOutStm );
        else
            WriteSfx( GetName(), rBase, rOutStm );
    }
}

/*************************************************************************
|*    SvMetaType::ReadMethodArgs()
|*
|*    Beschreibung
*************************************************************************/
BOOL SvMetaType::ReadMethodArgs( SvIdlDataBase & rBase,
                                      SvTokenStream & rInStm )
{
    UINT32  nTokPos = rInStm.Tell();
    if( rInStm.Read( '(' ) )
    {
        //DoReadContextSvIdl( rBase, rInStm, ',' );
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

/*************************************************************************
|*    SvMetaType::WriteMethodArgs()
|*
|*    Beschreibung
*************************************************************************/
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
                        DBG_ASSERT( FALSE, "WriteType not implemented" )
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

/*************************************************************************
|*    SvMetaType::WriteTypePrefix()
|*
|*    Beschreibung
*************************************************************************/
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

            ByteString aOut;
            if( GetType() == TYPE_METHOD )
                aOut = GetReturnType()->GetBaseType()->GetOdlName();
            else
            {
                SvMetaType * pBType = GetBaseType();
                aOut = pBType->GetOdlName();
            }
            if( aCall0 == (int)CALL_POINTER
              || aCall0 == (int)CALL_REFERENCE )
                rOutStm << " *";
            if( aCall1 == (int)CALL_POINTER
              || aCall1 == (int)CALL_REFERENCE )
                rOutStm << " *";
            rOutStm << aOut.GetBuffer();
        }
        break;

        case WRITE_C_HEADER:
        case WRITE_C_SOURCE:
        case WRITE_CXX_HEADER:
        case WRITE_CXX_SOURCE:
        {

            SvMetaType * pBaseType = GetBaseType();
            DBG_ASSERT( pBaseType, "no base type for attribute" )

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
            DBG_ASSERT( pBaseType, "no base type for attribute" )

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
            DBG_ASSERT( FALSE, "WriteType not implemented" )
        }
    }
}

/*************************************************************************
|*    SvMetaType::WriteTheType()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaType::WriteTheType( SvIdlDataBase & rBase, SvStream & rOutStm,
                             USHORT nTab, WriteType nT )
{
    WriteTypePrefix( rBase, rOutStm, nTab, nT );
    if( GetType() == TYPE_METHOD )
        WriteMethodArgs( rBase, rOutStm, nTab +2, nT );
}

/*************************************************************************
|*    SvMetaType::GetParserString()
|*
|*    Beschreibung
*************************************************************************/
ByteString SvMetaType::GetParserString() const
{
    SvMetaType * pBT = GetBaseType();
    if( pBT != this )
        return pBT->GetParserString();

    int nType = GetType();
    ByteString aPStr;

    if( TYPE_METHOD == nType || TYPE_STRUCT == nType )
    {
        ULONG nAttrCount = GetAttrCount();
        // Die einzelnen Attribute schreiben
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

/*************************************************************************
|*    SvMetaType::WriteParamNames()
|*
|*    Beschreibung
*************************************************************************/
void SvMetaType::WriteParamNames( SvIdlDataBase & rBase,
                                   SvStream & rOutStm,
                                   const ByteString & rChief )
{
    SvMetaType * pBT = GetBaseType();
    if( pBT != this )
        pBT->WriteParamNames( rBase, rOutStm, rChief );
    else
    {
        int nType = GetType();
        ByteString aPStr;

        if( TYPE_METHOD == nType || TYPE_STRUCT == nType )
        {
            ULONG nAttrCount = GetAttrCount();
            // Die einzelnen Attribute schreiben
            for( ULONG n = 0; n < nAttrCount; n++ )
            {
                SvMetaAttribute * pA = pAttrList->GetObject( n );
                // Fuer Methoden ist rChief immer ""
                ByteString aStr = /*rChief;
                if( aStr.Len() )
                    aStr += "->";
                aStr += */pA->GetName();
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

/************************************************************************/
/************************************************************************/
SV_IMPL_META_FACTORY1( SvMetaTypeString, SvMetaType );
#ifdef IDL_COMPILER
SvAttributeList & SvMetaTypeString::GetAttributeList()
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
|*    SvMetaTypeString::SvMetaTypeString()
|*
|*    Beschreibung
|*
*************************************************************************/
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

/************************************************************************/
/************************************************************************/
SV_IMPL_META_FACTORY1( SvMetaEnumValue, SvMetaName );
#ifdef IDL_COMPILER
SvAttributeList & SvMetaEnumValue::GetAttributeList()
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
|*    SvMetaEnumValue::SvMetaEnumValue()
|*
|*    Beschreibung
|*
*************************************************************************/
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
        DBG_ERROR( "wrong format" )
        return;
    }
    if( nMask & 0x01 ) rStm.ReadByteString( aEnumValue );
}

void SvMetaEnumValue::Save( SvPersistStream & rStm )
{
    SvMetaName::Save( rStm );

    // Maske erstellen
    BYTE nMask = 0;
    if( aEnumValue.Len() ) nMask |= 0x01;

    // Daten schreiben
    rStm << nMask;
    if( nMask & 0x01 ) rStm.WriteByteString( aEnumValue );
}

#ifdef IDL_COMPILER
/*************************************************************************
|*
|*    SvMetaEnumValue::ReadSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
BOOL SvMetaEnumValue::ReadSvIdl( SvIdlDataBase & rBase,
                                 SvTokenStream & rInStm )
{
    if( !ReadNameSvIdl( rBase, rInStm ) )
        return FALSE;
    return TRUE;
}

/*************************************************************************
|*
|*    SvMetaEnumValue::WriteSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
void SvMetaEnumValue::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                                  USHORT nTab )
{
    rOutStm << GetName().GetBuffer();
}

/*************************************************************************
|*
|*    SvMetaEnumValue::Write()
|*
|*    Beschreibung
|*
*************************************************************************/
void SvMetaEnumValue::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                            USHORT nTab,
                             WriteType nT, WriteAttribute nA )
{
    if ( nT == WRITE_C_HEADER || nT == WRITE_C_SOURCE )
        rOutStm << C_PREF << GetName().GetBuffer();
    else
        rOutStm << GetName().GetBuffer();
}
#endif // IDL_COMPILER

/************************************************************************/
/************************************************************************/
SV_IMPL_META_FACTORY1( SvMetaTypeEnum, SvMetaType );
#ifdef IDL_COMPILER
SvAttributeList & SvMetaTypeEnum::GetAttributeList()
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
|*    SvMetaTypeEnum::SvMetaTypeEnum()
|*
|*    Beschreibung
|*
*************************************************************************/
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
        DBG_ERROR( "wrong format" )
        return;
    }
    if( nMask & 0x01 ) rStm >> aEnumValueList;
    if( nMask & 0x02 ) rStm.ReadByteString( aPrefix );
}

void SvMetaTypeEnum::Save( SvPersistStream & rStm )
{
    SvMetaType::Save( rStm );

    // Maske erstellen
    BYTE nMask = 0;
    if( aEnumValueList.Count() )    nMask |= 0x01;
    if( aPrefix.Len() )             nMask |= 0x02;

    // Daten schreiben
    rStm << nMask;
    if( nMask & 0x01 ) rStm << aEnumValueList;
    if( nMask & 0x02 ) rStm.WriteByteString( aPrefix );
}

/*************************************************************************
|*
|*    SvMetaTypeEnum::GetMaxValue()
|*
|*    Beschreibung
|*
*************************************************************************/
/*
USHORT SvMetaTypeEnum::GetMaxValue() const
{
    USHORT nMax = 0;
    for( ULONG n = 0; n < aEnumValueList.Count(); n++ )
    {
        SvMetaEnumValue * pObj = aEnumValueList.GetObject( n );
        if( nMax < pObj->GetValue() )
            nMax = pObj->GetValue();
    }
    return nMax;
}
*/

#ifdef IDL_COMPILER
/*************************************************************************
|*
|*    SvMetaTypeEnum::ReadContextSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
void SvMetaTypeEnum::ReadContextSvIdl( SvIdlDataBase & rBase,
                                       SvTokenStream & rInStm )
{
    UINT32 nTokPos = rInStm.Tell();

    SvMetaEnumValueRef aEnumVal = new SvMetaEnumValue();
    BOOL bOk = aEnumVal->ReadSvIdl( rBase, rInStm );
    if( bOk )
    {
        if( 0 == aEnumValueList.Count() )
           // der Erste
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

/*************************************************************************
|*
|*    SvMetaTypeEnum::WriteSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    SvMetaTypeEnum::ReadSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
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

/*************************************************************************
|*
|*    SvMetaTypeEnum::WriteSvIdl()
|*
|*    Beschreibung
|*
*************************************************************************/
void SvMetaTypeEnum::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                                   USHORT nTab )
{
    WriteHeaderSvIdl( rBase, rOutStm, nTab );
    rOutStm << endl;
    SvMetaName::WriteSvIdl( rBase, rOutStm, nTab );
    rOutStm << endl;
}

/*************************************************************************
|*
|*    SvMetaTypeEnum::Write()
|*
|*    Beschreibung
|*
*************************************************************************/
void SvMetaTypeEnum::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                            USHORT nTab,
                             WriteType nT, WriteAttribute nA )
{
    SvMetaType::Write( rBase, rOutStm, nTab, nT, nA );
}

/*************************************************************************
|*    SvMetaTypeEnum::WriteContext()
|*
|*    Beschreibung
*************************************************************************/
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

/************************************************************************/
/************************************************************************/
SV_IMPL_META_FACTORY1( SvMetaTypevoid, SvMetaType );
#ifdef IDL_COMPILER
SvAttributeList & SvMetaTypevoid::GetAttributeList()
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
|*    SvMetaTypevoid::SvMetaTypevoid()
|*
|*    Beschreibung
|*
*************************************************************************/
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
            // Nur testen, wenn das Attribut eine Methode ist, nicht wenn es
            // eine hat !!
            if ( !pAttr->GetType()->GetType() == TYPE_METHOD )
                aStr += "    IsMethod\n";
            else if ( aType->GetReturnType() &&
                aType->GetReturnType()->GetType() != pAttr->GetType()->GetReturnType()->GetType() )
                    aStr += "    ReturnType\n";

            if ( aType->GetAttrCount() )
            {
                USHORT nCount = aType->GetAttrCount();
                SvMetaAttributeMemberList& rList = aType->GetAttrList();
                SvMetaAttributeMemberList& rOtherList = pAttr->GetType()->GetAttrList();
                USHORT nOtherCount = pAttr->GetType()->GetAttrCount();
                if ( nOtherCount != nCount )
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

void SvMetaAttribute::WriteCSV( SvIdlDataBase& rBase, SvStream& rStrm )
{
    rStrm << GetType()->GetSvName().GetBuffer() << ' ';
    rStrm << GetName().GetBuffer() << ' ';
    rStrm << GetSlotId().GetBuffer();
}


