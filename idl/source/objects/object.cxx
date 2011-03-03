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

#include <object.hxx>
#include <globals.hxx>
#include <database.hxx>

SV_IMPL_PERSIST1( SvClassElement, SvPersistBase );

SvClassElement::SvClassElement()
{
};

void SvClassElement::Load( SvPersistStream & rStm )
{
    BYTE nMask;
    rStm >> nMask;
    if( nMask >= 0x08 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "wrong format" );
        return;
    }
    if( nMask & 0x01 ) rStm >> aAutomation;
    if( nMask & 0x02 ) rStm.ReadByteString( aPrefix );
    if( nMask & 0x04 )
    {
        SvMetaClass * p;
        rStm >> p;
        xClass = p;
    }
}

void SvClassElement::Save( SvPersistStream & rStm )
{
    // create mask
    BYTE nMask = 0;
    if( aAutomation.IsSet() )       nMask |= 0x1;
    if( aPrefix.Len() )             nMask |= 0x2;
    if( xClass.Is() )               nMask |= 0x4;

    // write data
    rStm << nMask;
    if( nMask & 0x01 ) rStm << aAutomation;
    if( nMask & 0x02 ) rStm.WriteByteString( aPrefix );
    if( nMask & 0x04 ) rStm << xClass;
}

SV_IMPL_META_FACTORY1( SvMetaClass, SvMetaType );
SvMetaClass::SvMetaClass()
    : aAutomation( TRUE, FALSE )
{
}

void SvMetaClass::Load( SvPersistStream & rStm )
{
    SvMetaType::Load( rStm );

    BYTE nMask;
    rStm >> nMask;
    if( nMask >= 0x20 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "wrong format" );
        return;
    }
    if( nMask & 0x01 ) rStm >> aAttrList;
    if( nMask & 0x02 )
    {
        SvMetaClass * pSuper;
         rStm >> pSuper;
        aSuperClass = pSuper;
    }
    if( nMask & 0x04 ) rStm >> aClassList;
    if( nMask & 0x8 )
    {
        SvMetaClass * p;
        rStm >> p;
        xAutomationInterface = p;
    }
    if( nMask & 0x10 ) rStm >> aAutomation;
}

void SvMetaClass::Save( SvPersistStream & rStm )
{
    SvMetaType::Save( rStm );

    // create mask
    BYTE nMask = 0;
    if( aAttrList.Count() )         nMask |= 0x1;
    if( aSuperClass.Is() )          nMask |= 0x2;
    if( aClassList.Count() )        nMask |= 0x4;
    if( xAutomationInterface.Is() ) nMask |= 0x8;
    if( aAutomation.IsSet() )       nMask |= 0x10;

    // write data
    rStm << nMask;
    if( nMask & 0x01 ) rStm << aAttrList;
    if( nMask & 0x02 ) rStm << aSuperClass;
    if( nMask & 0x04 ) rStm << aClassList;
    if( nMask & 0x08 ) rStm << xAutomationInterface;
    if( nMask & 0x10 ) rStm << aAutomation;
}

#ifdef IDL_COMPILER
void SvMetaClass::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                        SvTokenStream & rInStm )
{
    SvMetaType::ReadAttributesSvIdl( rBase, rInStm );
    aAutomation.ReadSvIdl( SvHash_Automation(), rInStm );
}

void SvMetaClass::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                 SvStream & rOutStm, USHORT nTab )
{
    SvMetaType::WriteAttributesSvIdl( rBase, rOutStm, nTab );

    if( !aAutomation )
    {
        WriteTab( rOutStm, nTab );
        rOutStm << "//class SvMetaClass" << endl;
        if( !aAutomation )
        {
            WriteTab( rOutStm, nTab );
            aAutomation.WriteSvIdl( SvHash_Automation(), rOutStm );
            rOutStm << ';' << endl;
        }
    }
}

void SvMetaClass::ReadContextSvIdl( SvIdlDataBase & rBase,
                                    SvTokenStream & rInStm )
{
    UINT32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( SvHash_import() ) )
    {
        SvMetaClass * pClass = rBase.ReadKnownClass( rInStm );
        if( pClass )
        {
            SvClassElementRef xEle = new SvClassElement();
            xEle->SetClass( pClass );
            aClassList.Append( xEle );

            if( rInStm.Read( '[' ) )
            {
                pTok = rInStm.GetToken_Next();
                if( pTok->Is( SvHash_Automation() ) )
                {
                    if( rInStm.Read( ']' ) )
                    {
                        if( xAutomationInterface.Is() )
                        {
                            // set error
                            rBase.SetError( "Automation allready set",
                                            rInStm.GetToken() );
                            rBase.WriteError( rInStm );
                        }
                        xAutomationInterface = pClass;
                        xEle->SetAutomation( TRUE );
                    }
                    else
                    {
                        // set error
                        rBase.SetError( "missing ]", rInStm.GetToken() );
                        rBase.WriteError( rInStm );
                    }
                }
                else
                {
                    // set error
                    rBase.SetError( "only attribute Automation allowed",
                                    rInStm.GetToken() );
                    rBase.WriteError( rInStm );
                }
            }
            pTok = rInStm.GetToken();
            if( pTok->IsString() )
            {
                xEle->SetPrefix( pTok->GetString() );
                rInStm.GetToken_Next();
            }
            return;
        }
        else
        {
            // set error
            rBase.SetError( "unknown imported interface", rInStm.GetToken() );
            rBase.WriteError( rInStm );
        }
    }
    else
    {
        rInStm.Seek( nTokPos );
        SvMetaType * pType = rBase.ReadKnownType( rInStm );

        BOOL bOk = FALSE;
        SvMetaAttributeRef xAttr;
        if( !pType || pType->IsItem() )
        {
            xAttr = new SvMetaSlot( pType );
            if( xAttr->ReadSvIdl( rBase, rInStm ) )
                bOk = xAttr->Test( rBase, rInStm );
        }
        else
        {
            xAttr = new SvMetaAttribute( pType );
            if( xAttr->ReadSvIdl( rBase, rInStm ) )
                bOk = xAttr->Test( rBase, rInStm );
        }

        if( bOk )
            bOk = TestAttribute( rBase, rInStm, *xAttr );
        if( bOk )
        {
            if( !xAttr->GetSlotId().IsSet() )
            {
                SvNumberIdentifier aI;
                aI.SetValue( rBase.GetUniqueId() );
                xAttr->SetSlotId( aI );
            }
            aAttrList.Append( xAttr );
            return;
        }
    }
    rInStm.Seek( nTokPos );
}

void SvMetaClass::WriteContextSvIdl
(
    SvIdlDataBase & rBase,
    SvStream & rOutStm,
    USHORT nTab
)
{
    ULONG n;
    for( n = 0; n < aAttrList.Count(); n++ )
    {
        WriteTab( rOutStm, nTab );
        aAttrList.GetObject( n )->WriteSvIdl( rBase, rOutStm, nTab );
        rOutStm << ';' << endl;
    }
    for( n = 0; n < aClassList.Count(); n++ )
    {
        SvClassElement * pEle = aClassList.GetObject( n );
        WriteTab( rOutStm, nTab );
        rOutStm << SvHash_import()->GetName().GetBuffer() << ' '
                << pEle->GetPrefix().GetBuffer();
        if( pEle->GetAutomation() )
            rOutStm << " [ " << SvHash_Automation()->GetName().GetBuffer()
                    << " ]";
        if( pEle->GetPrefix().Len() )
            rOutStm << ' ' << pEle->GetPrefix().GetBuffer();
        rOutStm << ';' << endl;
    }
}

BOOL SvMetaClass::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    ULONG nTokPos = rInStm.Tell();
    if( SvMetaType::ReadHeaderSvIdl( rBase, rInStm ) && GetType() == TYPE_CLASS )
    {
        BOOL bOk = TRUE;
        if( rInStm.Read( ':' ) )
        {
            aSuperClass = rBase.ReadKnownClass( rInStm );
            bOk = aSuperClass.Is();
            if( !bOk )
            {
                // set error
                rBase.SetError( "unknown super class",
                                rInStm.GetToken() );
                rBase.WriteError( rInStm );
            }
        }
        if( bOk )
        {
            rBase.Write( '.' );
            bOk = SvMetaName::ReadSvIdl( rBase, rInStm );
        }
        if( bOk )
            return bOk;
    }
    rInStm.Seek( nTokPos );
    return FALSE;
}

BOOL SvMetaClass::TestAttribute( SvIdlDataBase & rBase, SvTokenStream & rInStm,
                                 SvMetaAttribute & rAttr ) const
{
    if ( !rAttr.GetRef() && rAttr.IsA( TYPE( SvMetaSlot ) ) )
    {
        OSL_FAIL( "Neuer Slot : " );
        OSL_FAIL( rAttr.GetSlotId().GetBuffer() );
    }

    for( ULONG n = 0; n < aAttrList.Count(); n++ )
    {
        SvMetaAttribute * pS = aAttrList.GetObject( n );
        if( pS->GetName() == rAttr.GetName() )
        {
            // values have to match
            if( pS->GetSlotId().GetValue() != rAttr.GetSlotId().GetValue() )
            {
                OSL_FAIL( "Gleicher Name in MetaClass : " );
                OSL_FAIL( pS->GetName().GetBuffer() );
                OSL_FAIL( pS->GetSlotId().GetBuffer() );
                OSL_FAIL( rAttr.GetSlotId().GetBuffer() );

                ByteString aStr( "Attribute's " );
                aStr += pS->GetName();
                aStr += " with different id's";
                rBase.SetError( aStr, rInStm.GetToken() );
                rBase.WriteError( rInStm );
                return FALSE;
             }
        }
        else
        {
            UINT32 nId1 = pS->GetSlotId().GetValue();
            UINT32 nId2 = rAttr.GetSlotId().GetValue();
            if( nId1 == nId2 && nId1 != 0 )
            {
                OSL_FAIL( "Gleiche Id in MetaClass : " );
                OSL_FAIL( ByteString::CreateFromInt32( pS->GetSlotId().GetValue() ).GetBuffer() );
                OSL_FAIL( pS->GetSlotId().GetBuffer() );
                OSL_FAIL( rAttr.GetSlotId().GetBuffer() );

                ByteString aStr( "Attribute " );
                aStr += pS->GetName();
                aStr += " and Attribute ";
                aStr += rAttr.GetName();
                aStr += " with equal id's";
                rBase.SetError( aStr, rInStm.GetToken() );
                rBase.WriteError( rInStm );
                return FALSE;
             }
        }
    }
    SvMetaClass * pSC = aSuperClass;
    if( pSC )
        return pSC->TestAttribute( rBase, rInStm, rAttr );
    return TRUE;
}

void SvMetaClass::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                              USHORT nTab )
{
    WriteHeaderSvIdl( rBase, rOutStm, nTab );
    if( aSuperClass.Is() )
        rOutStm << " : " << aSuperClass->GetName().GetBuffer();
    rOutStm << endl;
    SvMetaName::WriteSvIdl( rBase, rOutStm, nTab );
    rOutStm << endl;
}

void SvMetaClass::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                        USHORT nTab,
                         WriteType nT, WriteAttribute )
{
    rBase.aIFaceName = GetName();
    switch( nT )
    {
        case WRITE_ODL:
        {
            OSL_FAIL( "Not supported anymore!" );
            break;
        }
        case WRITE_C_SOURCE:
        case WRITE_C_HEADER:
        {
            OSL_FAIL( "Not supported anymore!" );
            break;
        }
        case WRITE_DOCU:
        {
            rOutStm << "<INTERFACE>" << endl
                    << GetName().GetBuffer();
            if ( GetAutomation() )
                rOutStm << " ( Automation ) ";
            rOutStm << endl;
            WriteDescription( rOutStm );
            rOutStm << "</INTERFACE>" << endl << endl;

            // write all attributes
            ULONG n;
            for( n = 0; n < aAttrList.Count(); n++ )
            {
                SvMetaAttribute * pAttr = aAttrList.GetObject( n );
                if( !pAttr->GetHidden() )
                {
                    if( pAttr->IsMethod() )
                        pAttr->Write( rBase, rOutStm, nTab, nT, WA_METHOD );

                    if( pAttr->IsVariable() )
                        pAttr->Write( rBase, rOutStm, nTab, nT, WA_VARIABLE );
                }
            }

            break;
        }
        default:
            break;
    }
}

USHORT SvMetaClass::WriteSlotParamArray( SvIdlDataBase & rBase,
                                        SvSlotElementList & rSlotList,
                                        SvStream & rOutStm )
{
    USHORT nCount = 0;
    for( ULONG n = 0; n < rSlotList.Count(); n++ )
    {
        SvSlotElement *pEle = rSlotList.GetObject( n );
        SvMetaSlot *pAttr = pEle->xSlot;
        nCount = nCount + pAttr->WriteSlotParamArray( rBase, rOutStm );
    }

    return nCount;
}

USHORT SvMetaClass::WriteSlots( const ByteString & rShellName,
                                USHORT nCount, SvSlotElementList & rSlotList,
                                SvIdlDataBase & rBase,
                                SvStream & rOutStm )
{
    USHORT nSCount = 0;
    for( ULONG n = 0; n < rSlotList.Count(); n++ )
    {
        rSlotList.Seek(n);
        SvSlotElement * pEle = rSlotList.GetCurObject();
        SvMetaSlot * pAttr = pEle->xSlot;
        nSCount = nSCount + pAttr->WriteSlotMap( rShellName, nCount + nSCount,
                                        rSlotList, pEle->aPrefix, rBase,
                                        rOutStm );
    }

    return nSCount;
}

void SvMetaClass::InsertSlots( SvSlotElementList& rList, std::vector<ULONG>& rSuperList,
                            SvMetaClassList &rClassList,
                            const ByteString & rPrefix, SvIdlDataBase& rBase)
{
    // was this class already written?
    for ( size_t i = 0, n = rClassList.size(); i < n ; ++i )
        if ( rClassList[ i ] == this )
            return;

    rClassList.push_back( this );

    // write all direct attributes
    ULONG n;
    for( n = 0; n < aAttrList.Count(); n++ )
    {
        SvMetaAttribute * pAttr = aAttrList.GetObject( n );

        ULONG nId = pAttr->GetSlotId().GetValue();

        std::vector<ULONG>::iterator iter = std::find(rSuperList.begin(),
                                                      rSuperList.end(),nId);

        if( iter == rSuperList.end() )
        {
            // Write only if not already written by subclass or
            // imported interface.
            rSuperList.push_back(nId);
            pAttr->Insert(rList, rPrefix, rBase);
        }
    }

    // All Interfaces already imported by SuperShells should not be
    // written any more.
    // It is prohibited that Shell and SuperShell directly import the same
    //class.
    if( IsShell() && aSuperClass.Is() )
        aSuperClass->FillClasses( rClassList );

    // Write all attributes of the imported classes, as long as they have
    // not already been imported by the superclass.
    for( n = 0; n < aClassList.Count(); n++ )
    {
        SvClassElement * pEle = aClassList.GetObject( n );
        SvMetaClass * pCl = pEle->GetClass();
        ByteString rPre = rPrefix;
        if( rPre.Len() && pEle->GetPrefix().Len() )
            rPre += '.';
        rPre += pEle->GetPrefix();

        // first of all write direct imported interfaces
        pCl->InsertSlots( rList, rSuperList, rClassList, rPre, rBase );
    }

    // only write superclass if no shell and not in the list
    if( !IsShell() && aSuperClass.Is() )
    {
        aSuperClass->InsertSlots( rList, rSuperList, rClassList, rPrefix, rBase );
    }
}

void SvMetaClass::FillClasses( SvMetaClassList & rList )
{
    // Am I not yet in?
    for ( size_t i = 0, n = rList.size(); i < n; ++i )
        if ( rList[ i ] == this )
            return;

    rList.push_back( this );

    // my imports
    for( ULONG n = 0; n < aClassList.Count(); n++ )
    {
        SvClassElement * pEle = aClassList.GetObject( n );
        SvMetaClass * pCl = pEle->GetClass();
        pCl->FillClasses( rList );
    }

    // my superclass
    if( aSuperClass.Is() )
        aSuperClass->FillClasses( rList );
}


void SvMetaClass::WriteSlotStubs( const ByteString & rShellName,
                                SvSlotElementList & rSlotList,
                                ByteStringList & rList,
                                SvStream & rOutStm )
{
    // write all attributes
    for( ULONG n = 0; n < rSlotList.Count(); n++ )
    {
        SvSlotElement *pEle = rSlotList.GetObject( n );
        SvMetaSlot *pAttr = pEle->xSlot;
        pAttr->WriteSlotStubs( rShellName, rList, rOutStm );
    }
}

void SvMetaClass::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    WriteStars( rOutStm );
    // define class
    rOutStm << "#ifdef " << GetName().GetBuffer() << endl;
    rOutStm << "#undef ShellClass" << endl;
    rOutStm << "#undef " << GetName().GetBuffer() << endl;
    rOutStm << "#define ShellClass " << GetName().GetBuffer() << endl;

    // no slotmaps get written for interfaces
    if( !IsShell() )
    {
        rOutStm << "#endif" << endl << endl;
        return;
    }
    // write parameter array
    rOutStm << "SFX_ARGUMENTMAP(" << GetName().GetBuffer() << ')' << endl
        << '{' << endl;

    std::vector<ULONG> aSuperList;
    SvMetaClassList classList;
    SvSlotElementList aSlotList;
    InsertSlots(aSlotList, aSuperList, classList, ByteString(), rBase);
    for (ULONG n=0; n<aSlotList.Count(); n++ )
    {
        SvSlotElement *pEle = aSlotList.GetObject( n );
        SvMetaSlot *pSlot = pEle->xSlot;
        pSlot->SetListPos(n);
    }

    ULONG nSlotCount = aSlotList.Count();

    // write all attributes
    USHORT nArgCount = WriteSlotParamArray( rBase, aSlotList, rOutStm );
    if( nArgCount )
        Back2Delemitter( rOutStm );
    else
    {
        // at leaast one dummy
        WriteTab( rOutStm, 1 );
        rOutStm << "SFX_ARGUMENT( 0, 0, SfxVoidItem )" << endl;
    }
    rOutStm << endl << "};" << endl << endl;

    ByteStringList aStringList;
    WriteSlotStubs( GetName(), aSlotList, aStringList, rOutStm );
    for ( size_t i = 0, n = aStringList.size(); i < n; ++i )
        delete aStringList[ i ];
    aStringList.clear();

    rOutStm << endl;

    // write slotmap
    rOutStm << "SFX_SLOTMAP_ARG(" << GetName().GetBuffer() << ')' << endl
        << '{' << endl;

    // write all attributes
    WriteSlots( GetName(), 0, aSlotList, rBase, rOutStm );
    if( nSlotCount )
        Back2Delemitter( rOutStm );
    else
    {
        // at least one dummy
        WriteTab( rOutStm, 1 );
        rOutStm << "SFX_SLOT_ARG(" << GetName().GetBuffer()
                << ", 0, 0, "
                << "SFX_STUB_PTR_EXEC_NONE,"
                << "SFX_STUB_PTR_STATE_NONE,"
                << "0, SfxVoidItem, 0, 0, \"\", 0 )" << endl;
    }
    rOutStm << endl << "};" << endl << "#endif" << endl << endl;

    for( ULONG n=0; n<aSlotList.Count(); n++ )
    {
        aSlotList.Seek(n);
        SvSlotElement* pEle = aSlotList.GetCurObject();
        SvMetaSlot* pAttr = pEle->xSlot;
        pAttr->ResetSlotPointer();
    }

    for ( ULONG n=0; n<aSlotList.Count(); n++ )
        delete aSlotList.GetObject(n);
}

void SvMetaClass::WriteHelpIds( SvIdlDataBase & rBase, SvStream & rOutStm,
                            Table* pTable )
{
    for( ULONG n=0; n<aAttrList.Count(); n++ )
    {
        SvMetaAttribute * pAttr = aAttrList.GetObject( n );
        pAttr->WriteHelpId( rBase, rOutStm, pTable );
    }
}

void SvMetaClass::WriteSrc( SvIdlDataBase & rBase, SvStream & rOutStm,
                             Table * pTable )
{
    for( ULONG n=0; n<aAttrList.Count(); n++ )
    {
        SvMetaAttribute * pAttr = aAttrList.GetObject( n );
        pAttr->WriteSrc( rBase, rOutStm, pTable );
    }
}

void SvMetaClass::WriteHxx( SvIdlDataBase &, SvStream & rOutStm, USHORT )
{
    ByteString aSuperName( "SvDispatch" );
    if( GetSuperClass() )
        aSuperName = GetSuperClass()->GetName();
    const char * pSup = aSuperName.GetBuffer();

    rOutStm
    << "class " << GetSvName().GetBuffer()
    << ": public " << pSup << endl
    << '{' << endl
    << "protected:" << endl
    << "\tvirtual SvGlobalName  GetTypeName() const;" << endl
    << "\tvirtual BOOL          FillTypeLibInfo( SvGlobalName *, USHORT * pMajor," << endl
    << "\t                                       USHORT * pMinor ) const;" << endl
    << "\tvirtual BOOL          FillTypeLibInfo( ByteString * pName, USHORT * pMajor," << endl;
    rOutStm
    << "\t                                       USHORT * pMinor ) const;" << endl
    << "\tvirtual void          Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) = 0;" << endl
    << "public:" << endl
    << "\t static SvGlobalName  ClassName()" << endl
    << "\t                      { return SvGlobalName( " << ByteString( GetUUId().GetctorName(), RTL_TEXTENCODING_UTF8 ).GetBuffer() << " ); }" << endl
    << "};" << endl;
}

void SvMetaClass::WriteCxx( SvIdlDataBase &, SvStream & rOutStm, USHORT )
{
    ByteString aSuperName( "SvDispatch" );
    if( GetSuperClass() )
        aSuperName = GetSuperClass()->GetName();
    const char * pSup = aSuperName.GetBuffer();

    ByteString name = GetSvName();
    // GetTypeName
    rOutStm << "SvGlobalName " << name.GetBuffer() << "::GetTypeName() const" << endl
    << '{' << endl
    << "\treturn ClassName();" << endl
    << '}' << endl;

    SvMetaModule * pMod = GetModule();
    // FillTypeLibInfo
    rOutStm << "BOOL " << name.GetBuffer() << "::FillTypeLibInfo( SvGlobalName * pGN," << endl
    << "\t                               USHORT * pMajor," << endl
    << "\t                               USHORT * pMinor ) const" << endl
    << '{' << endl
    << "\tSvGlobalName aN( " << ByteString( pMod->GetUUId().GetctorName(), RTL_TEXTENCODING_UTF8 ).GetBuffer() << " );" << endl;
    rOutStm << "\t*pGN = aN;" << endl
    << "\t*pMajor = " << ByteString::CreateFromInt32(pMod->GetVersion().GetMajorVersion()).GetBuffer() << ';' << endl
    << "\t*pMinor = " << ByteString::CreateFromInt32(pMod->GetVersion().GetMinorVersion()).GetBuffer() << ';' << endl
    << "\treturn TRUE;" << endl
    << '}' << endl;

    // FillTypeLibInfo
    rOutStm << "BOOL " << name.GetBuffer() << "::FillTypeLibInfo( ByteString * pName,"
    << "\t                               USHORT * pMajor," << endl
    << "\t                               USHORT * pMinor ) const" << endl;
    rOutStm << '{' << endl
    << "\t*pName = \"" << pMod->GetTypeLibFileName().GetBuffer()  << "\";" << endl
    << "\t*pMajor = " << ByteString::CreateFromInt32(pMod->GetVersion().GetMajorVersion()).GetBuffer() << ';' << endl
    << "\t*pMinor = " << ByteString::CreateFromInt32(pMod->GetVersion().GetMinorVersion()).GetBuffer() << ';' << endl
    << "\treturn TRUE;" << endl
    << '}' << endl;

    rOutStm << "void " << name.GetBuffer() << "::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )" << endl
    << '{' << endl
    << "\t" << pSup << "::Notify( rBC, rHint );" << endl
    << '}' << endl;
}

#endif // IDL_COMPILER

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
