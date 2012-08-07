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

#include <rtl/strbuf.hxx>

#include <object.hxx>
#include <globals.hxx>
#include <database.hxx>

SV_IMPL_PERSIST1( SvClassElement, SvPersistBase );

SvClassElement::SvClassElement()
{
};

void SvClassElement::Load( SvPersistStream & rStm )
{
    sal_uInt8 nMask;
    rStm >> nMask;
    if( nMask >= 0x08 )
    {
        rStm.SetError( SVSTREAM_FILEFORMAT_ERROR );
        OSL_FAIL( "wrong format" );
        return;
    }
    if( nMask & 0x01 ) rStm >> aAutomation;
    if( nMask & 0x02 ) aPrefix = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rStm);
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
    sal_uInt8 nMask = 0;
    if( aAutomation.IsSet() )       nMask |= 0x1;
    if( !aPrefix.isEmpty() )        nMask |= 0x2;
    if( xClass.Is() )               nMask |= 0x4;

    // write data
    rStm << nMask;
    if( nMask & 0x01 ) rStm << aAutomation;
    if( nMask & 0x02 ) write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rStm, aPrefix);
    if( nMask & 0x04 ) rStm << xClass;
}

SV_IMPL_META_FACTORY1( SvMetaClass, SvMetaType );
SvMetaClass::SvMetaClass()
    : aAutomation( sal_True, sal_False )
{
}

void SvMetaClass::Load( SvPersistStream & rStm )
{
    SvMetaType::Load( rStm );

    sal_uInt8 nMask;
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
    sal_uInt8 nMask = 0;
    if( !aAttrList.empty() )        nMask |= 0x1;
    if( aSuperClass.Is() )          nMask |= 0x2;
    if( !aClassList.empty() )       nMask |= 0x4;
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

void SvMetaClass::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                        SvTokenStream & rInStm )
{
    SvMetaType::ReadAttributesSvIdl( rBase, rInStm );
    aAutomation.ReadSvIdl( SvHash_Automation(), rInStm );
}

void SvMetaClass::WriteAttributesSvIdl( SvIdlDataBase & rBase,
                                 SvStream & rOutStm, sal_uInt16 nTab )
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
    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken * pTok = rInStm.GetToken_Next();

    if( pTok->Is( SvHash_import() ) )
    {
        SvMetaClass * pClass = rBase.ReadKnownClass( rInStm );
        if( pClass )
        {
            SvClassElementRef xEle = new SvClassElement();
            xEle->SetClass( pClass );
            aClassList.push_back( xEle );

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
                        xEle->SetAutomation( sal_True );
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

        sal_Bool bOk = sal_False;
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
            aAttrList.push_back( xAttr );
            return;
        }
    }
    rInStm.Seek( nTokPos );
}

void SvMetaClass::WriteContextSvIdl
(
    SvIdlDataBase & rBase,
    SvStream & rOutStm,
    sal_uInt16 nTab
)
{
    sal_uLong n;
    for( n = 0; n < aAttrList.size(); n++ )
    {
        WriteTab( rOutStm, nTab );
        aAttrList[n]->WriteSvIdl( rBase, rOutStm, nTab );
        rOutStm << ';' << endl;
    }
    for( n = 0; n < aClassList.size(); n++ )
    {
        SvClassElement * pEle = aClassList[n];
        WriteTab( rOutStm, nTab );
        rOutStm << SvHash_import()->GetName().getStr() << ' '
                << pEle->GetPrefix().getStr();
        if( pEle->GetAutomation() )
            rOutStm << " [ " << SvHash_Automation()->GetName().getStr()
                    << " ]";
        if( !pEle->GetPrefix().isEmpty() )
            rOutStm << ' ' << pEle->GetPrefix().getStr();
        rOutStm << ';' << endl;
    }
}

sal_Bool SvMetaClass::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    sal_uLong nTokPos = rInStm.Tell();
    if( SvMetaType::ReadHeaderSvIdl( rBase, rInStm ) && GetType() == TYPE_CLASS )
    {
        sal_Bool bOk = sal_True;
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
            rBase.Write(rtl::OString('.'));
            bOk = SvMetaName::ReadSvIdl( rBase, rInStm );
        }
        if( bOk )
            return bOk;
    }
    rInStm.Seek( nTokPos );
    return sal_False;
}

sal_Bool SvMetaClass::TestAttribute( SvIdlDataBase & rBase, SvTokenStream & rInStm,
                                 SvMetaAttribute & rAttr ) const
{
    if ( !rAttr.GetRef() && rAttr.IsA( TYPE( SvMetaSlot ) ) )
    {
        OSL_FAIL( "Neuer Slot : " );
        OSL_FAIL( rAttr.GetSlotId().getString().getStr() );
    }

    for( sal_uLong n = 0; n < aAttrList.size(); n++ )
    {
        SvMetaAttribute * pS = aAttrList[n];
        if( pS->GetName().getString() == rAttr.GetName().getString() )
        {
            // values have to match
            if( pS->GetSlotId().GetValue() != rAttr.GetSlotId().GetValue() )
            {
                OSL_FAIL( "Gleicher Name in MetaClass : " );
                OSL_FAIL( pS->GetName().getString().getStr() );
                OSL_FAIL( pS->GetSlotId().getString().getStr() );
                OSL_FAIL( rAttr.GetSlotId().getString().getStr() );

                rtl::OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM("Attribute's "));
                aStr.append(pS->GetName().getString());
                aStr.append(RTL_CONSTASCII_STRINGPARAM(" with different id's"));
                rBase.SetError(aStr.makeStringAndClear(), rInStm.GetToken());
                rBase.WriteError( rInStm );
                return sal_False;
             }
        }
        else
        {
            sal_uInt32 nId1 = pS->GetSlotId().GetValue();
            sal_uInt32 nId2 = rAttr.GetSlotId().GetValue();
            if( nId1 == nId2 && nId1 != 0 )
            {
                OSL_FAIL( "Gleiche Id in MetaClass : " );
                OSL_FAIL(rtl::OString::valueOf(static_cast<sal_Int32>(
                    pS->GetSlotId().GetValue())).getStr());
                OSL_FAIL( pS->GetSlotId().getString().getStr() );
                OSL_FAIL( rAttr.GetSlotId().getString().getStr() );

                rtl::OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM("Attribute "));
                aStr.append(pS->GetName().getString());
                aStr.append(RTL_CONSTASCII_STRINGPARAM(" and Attribute "));
                aStr.append(rAttr.GetName().getString());
                aStr.append(RTL_CONSTASCII_STRINGPARAM(" with equal id's"));
                rBase.SetError(aStr.makeStringAndClear(), rInStm.GetToken());
                rBase.WriteError( rInStm );
                return sal_False;
             }
        }
    }
    SvMetaClass * pSC = aSuperClass;
    if( pSC )
        return pSC->TestAttribute( rBase, rInStm, rAttr );
    return sal_True;
}

void SvMetaClass::WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                              sal_uInt16 nTab )
{
    WriteHeaderSvIdl( rBase, rOutStm, nTab );
    if( aSuperClass.Is() )
        rOutStm << " : " << aSuperClass->GetName().getString().getStr();
    rOutStm << endl;
    SvMetaName::WriteSvIdl( rBase, rOutStm, nTab );
    rOutStm << endl;
}

void SvMetaClass::Write( SvIdlDataBase & rBase, SvStream & rOutStm,
                        sal_uInt16 nTab,
                         WriteType nT, WriteAttribute )
{
    rBase.aIFaceName = GetName().getString();
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
                    << GetName().getString().getStr();
            if ( GetAutomation() )
                rOutStm << " ( Automation ) ";
            rOutStm << endl;
            WriteDescription( rOutStm );
            rOutStm << "</INTERFACE>" << endl << endl;

            // write all attributes
            sal_uLong n;
            for( n = 0; n < aAttrList.size(); n++ )
            {
                SvMetaAttribute * pAttr = aAttrList[n];
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

sal_uInt16 SvMetaClass::WriteSlotParamArray( SvIdlDataBase & rBase,
                                        SvSlotElementList & rSlotList,
                                        SvStream & rOutStm )
{
    sal_uInt16 nCount = 0;
    for ( size_t i = 0, n = rSlotList.size(); i < n; ++i )
    {
        SvSlotElement *pEle = rSlotList[ i ];
        SvMetaSlot *pAttr = pEle->xSlot;
        nCount = nCount + pAttr->WriteSlotParamArray( rBase, rOutStm );
    }

    return nCount;
}

sal_uInt16 SvMetaClass::WriteSlots( const rtl::OString& rShellName,
                                sal_uInt16 nCount, SvSlotElementList & rSlotList,
                                SvIdlDataBase & rBase,
                                SvStream & rOutStm )
{
    sal_uInt16 nSCount = 0;
    for ( size_t i = 0, n = rSlotList.size(); i < n; ++i )
    {
        SvSlotElement * pEle = rSlotList[ i ];
        SvMetaSlot * pAttr = pEle->xSlot;
        nSCount = nSCount + pAttr->WriteSlotMap( rShellName, nCount + nSCount,
                                        rSlotList, i, pEle->aPrefix, rBase,
                                        rOutStm );
    }

    return nSCount;
}

void SvMetaClass::InsertSlots( SvSlotElementList& rList, std::vector<sal_uLong>& rSuperList,
                            SvMetaClassList &rClassList,
                            const rtl::OString& rPrefix, SvIdlDataBase& rBase)
{
    // was this class already written?
    for ( size_t i = 0, n = rClassList.size(); i < n ; ++i )
        if ( rClassList[ i ] == this )
            return;

    rClassList.push_back( this );

    // write all direct attributes
    sal_uLong n;
    for( n = 0; n < aAttrList.size(); n++ )
    {
        SvMetaAttribute * pAttr = aAttrList[n];

        sal_uLong nId = pAttr->GetSlotId().GetValue();

        std::vector<sal_uLong>::iterator iter = std::find(rSuperList.begin(),
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
    for( n = 0; n < aClassList.size(); n++ )
    {
        SvClassElement * pEle = aClassList[n];
        SvMetaClass * pCl = pEle->GetClass();
        rtl::OStringBuffer rPre(rPrefix);
        if( rPre.getLength() && pEle->GetPrefix().getLength() )
            rPre.append('.');
        rPre.append(pEle->GetPrefix());

        // first of all write direct imported interfaces
        pCl->InsertSlots( rList, rSuperList, rClassList,
            rPre.makeStringAndClear(), rBase );
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
    for( sal_uInt32 n = 0; n < aClassList.size(); n++ )
    {
        SvClassElement * pEle = aClassList[n];
        SvMetaClass * pCl = pEle->GetClass();
        pCl->FillClasses( rList );
    }

    // my superclass
    if( aSuperClass.Is() )
        aSuperClass->FillClasses( rList );
}


void SvMetaClass::WriteSlotStubs( const rtl::OString& rShellName,
                                SvSlotElementList & rSlotList,
                                ByteStringList & rList,
                                SvStream & rOutStm )
{
    // write all attributes
    for ( size_t i = 0, n = rSlotList.size(); i < n; ++i )
    {
        SvSlotElement *pEle = rSlotList[ i ];
        SvMetaSlot *pAttr = pEle->xSlot;
        pAttr->WriteSlotStubs( rShellName, rList, rOutStm );
    }
}

void SvMetaClass::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    WriteStars( rOutStm );
    // define class
    rOutStm << "#ifdef " << GetName().getString().getStr() << endl;
    rOutStm << "#undef ShellClass" << endl;
    rOutStm << "#undef " << GetName().getString().getStr() << endl;
    rOutStm << "#define ShellClass " << GetName().getString().getStr() << endl;

    // no slotmaps get written for interfaces
    if( !IsShell() )
    {
        rOutStm << "#endif" << endl << endl;
        return;
    }
    // write parameter array
    rOutStm << "SFX_ARGUMENTMAP(" << GetName().getString().getStr() << ')' << endl
        << '{' << endl;

    std::vector<sal_uLong> aSuperList;
    SvMetaClassList classList;
    SvSlotElementList aSlotList;
    InsertSlots(aSlotList, aSuperList, classList, rtl::OString(), rBase);
    for ( size_t i = 0, n = aSlotList.size(); i < n; ++i )
    {
        SvSlotElement *pEle = aSlotList[ i ];
        SvMetaSlot *pSlot = pEle->xSlot;
        pSlot->SetListPos( i );
    }

    size_t nSlotCount = aSlotList.size();

    // write all attributes
    sal_uInt16 nArgCount = WriteSlotParamArray( rBase, aSlotList, rOutStm );
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
    WriteSlotStubs( GetName().getString(), aSlotList, aStringList, rOutStm );
    for ( size_t i = 0, n = aStringList.size(); i < n; ++i )
        delete aStringList[ i ];
    aStringList.clear();

    rOutStm << endl;

    // write slotmap
    rOutStm << "SFX_SLOTMAP_ARG(" << GetName().getString().getStr() << ')' << endl
        << '{' << endl;

    // write all attributes
    WriteSlots( GetName().getString(), 0, aSlotList, rBase, rOutStm );
    if( nSlotCount )
        Back2Delemitter( rOutStm );
    else
    {
        // at least one dummy
        WriteTab( rOutStm, 1 );
        rOutStm << "SFX_SLOT_ARG(" << GetName().getString().getStr()
                << ", 0, 0, "
                << "SFX_STUB_PTR_EXEC_NONE,"
                << "SFX_STUB_PTR_STATE_NONE,"
                << "0, SfxVoidItem, 0, 0, \"\", 0 )" << endl;
    }
    rOutStm << endl << "};" << endl << "#endif" << endl << endl;

    for( size_t i = 0, n = aSlotList.size(); i < n; ++i )
    {
        SvSlotElement* pEle = aSlotList[ i ];
        SvMetaSlot* pAttr = pEle->xSlot;
        pAttr->ResetSlotPointer();
    }

    for( size_t i = 0, n = aSlotList.size(); i < n; ++i )
        delete aSlotList[ i ];
    aSlotList.clear();
}

void SvMetaClass::WriteHelpIds( SvIdlDataBase & rBase, SvStream & rOutStm,
                            HelpIdTable& rTable )
{
    for( sal_uLong n=0; n<aAttrList.size(); n++ )
    {
        SvMetaAttribute * pAttr = aAttrList[n];
        pAttr->WriteHelpId( rBase, rOutStm, rTable );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
