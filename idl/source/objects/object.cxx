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
#include <osl/diagnose.h>

#include <object.hxx>
#include <globals.hxx>
#include <database.hxx>


SvClassElement::SvClassElement()
{
};

SvMetaClass::SvMetaClass()
    : aAutomation( true, false )
{
}

void SvMetaClass::ReadAttributesSvIdl( SvIdlDataBase & rBase,
                                        SvTokenStream & rInStm )
{
    SvMetaType::ReadAttributesSvIdl( rBase, rInStm );
    aAutomation.ReadSvIdl( SvHash_Automation(), rInStm );
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
            tools::SvRef<SvClassElement> xEle = new SvClassElement();
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
                            rBase.SetError( "Automation already set", rInStm.GetToken() );
                            rBase.WriteError( rInStm );
                        }
                        xAutomationInterface = pClass;
                        xEle->SetAutomation( true );
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
                    rBase.SetError( "only attribute Automation allowed", rInStm.GetToken() );
                    rBase.WriteError( rInStm );
                }
            }
            pTok = &rInStm.GetToken();
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

        bool bOk = false;
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
                SvIdentifier aI;
                aI.SetValue( rBase.GetUniqueId() );
                xAttr->SetSlotId( aI );
            }
            aAttrList.push_back( xAttr );
            return;
        }
    }
    rInStm.Seek( nTokPos );
}

bool SvMetaClass::ReadSvIdl( SvIdlDataBase & rBase, SvTokenStream & rInStm )
{
    sal_uLong nTokPos = rInStm.Tell();
    if( SvMetaType::ReadHeaderSvIdl( rBase, rInStm ) && GetType() == TYPE_CLASS )
    {
        bool bOk = true;
        if( rInStm.Read( ':' ) )
        {
            aSuperClass = rBase.ReadKnownClass( rInStm );
            bOk = aSuperClass.Is();
            if( !bOk )
            {
                // set error
                rBase.SetError( "unknown super class", rInStm.GetToken() );
                rBase.WriteError( rInStm );
            }
        }
        if( bOk )
        {
            rBase.Write(OString('.'));
            bOk = SvMetaObject::ReadSvIdl( rBase, rInStm );
        }
        if( bOk )
            return bOk;
    }
    rInStm.Seek( nTokPos );
    return false;
}

bool SvMetaClass::TestAttribute( SvIdlDataBase & rBase, SvTokenStream & rInStm,
                                 SvMetaAttribute & rAttr ) const
{
    if ( !rAttr.GetRef() && dynamic_cast<const SvMetaSlot *>(&rAttr) != nullptr )
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
                OSL_FAIL( "Same Name in MetaClass : " );
                OSL_FAIL( pS->GetName().getString().getStr() );
                OSL_FAIL( pS->GetSlotId().getString().getStr() );
                OSL_FAIL( rAttr.GetSlotId().getString().getStr() );

                OStringBuffer aStr("Attribute's ");
                aStr.append(pS->GetName().getString());
                aStr.append(" with different id's");
                rBase.SetError(aStr.makeStringAndClear(), rInStm.GetToken());
                rBase.WriteError( rInStm );
                return false;
             }
        }
        else
        {
            sal_uInt32 nId1 = pS->GetSlotId().GetValue();
            sal_uInt32 nId2 = rAttr.GetSlotId().GetValue();
            if( nId1 == nId2 && nId1 != 0 )
            {
                OSL_FAIL( "Gleiche Id in MetaClass : " );
                OSL_FAIL(OString::number(pS->GetSlotId().GetValue()).getStr());
                OSL_FAIL( pS->GetSlotId().getString().getStr() );
                OSL_FAIL( rAttr.GetSlotId().getString().getStr() );

                OStringBuffer aStr("Attribute ");
                aStr.append(pS->GetName().getString());
                aStr.append(" and Attribute ");
                aStr.append(rAttr.GetName().getString());
                aStr.append(" with equal id's");
                rBase.SetError(aStr.makeStringAndClear(), rInStm.GetToken());
                rBase.WriteError( rInStm );
                return false;
             }
        }
    }
    SvMetaClass * pSC = aSuperClass;
    if( pSC )
        return pSC->TestAttribute( rBase, rInStm, rAttr );
    return true;
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

sal_uInt16 SvMetaClass::WriteSlots( const OString& rShellName,
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
                                        rSlotList, i, rBase,
                                        rOutStm );
    }

    return nSCount;
}

void SvMetaClass::InsertSlots( SvSlotElementList& rList, std::vector<sal_uLong>& rSuperList,
                            SvMetaClassList &rClassList,
                            const OString& rPrefix, SvIdlDataBase& rBase)
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
        OStringBuffer rPre(rPrefix);
        if( !rPre.isEmpty() && !pEle->GetPrefix().isEmpty() )
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
    for( size_t n = 0; n < aClassList.size(); n++ )
    {
        SvClassElement * pEle = aClassList[n];
        SvMetaClass * pCl = pEle->GetClass();
        pCl->FillClasses( rList );
    }

    // my superclass
    if( aSuperClass.Is() )
        aSuperClass->FillClasses( rList );
}


void SvMetaClass::WriteSlotStubs( const OString& rShellName,
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
    rOutStm.WriteCharPtr( "#ifdef " ).WriteCharPtr( GetName().getString().getStr() ) << endl;
    rOutStm.WriteCharPtr( "#undef ShellClass" ) << endl;
    rOutStm.WriteCharPtr( "#undef " ).WriteCharPtr( GetName().getString().getStr() ) << endl;
    rOutStm.WriteCharPtr( "#define ShellClass " ).WriteCharPtr( GetName().getString().getStr() ) << endl;

    // no slotmaps get written for interfaces
    if( !IsShell() )
    {
        rOutStm.WriteCharPtr( "#endif" ) << endl << endl;
        return;
    }
    // write parameter array
    rOutStm.WriteCharPtr("static SfxFormalArgument a").WriteCharPtr(GetName().getString().getStr()).WriteCharPtr("Args_Impl[] =") << endl;
    rOutStm.WriteChar('{') << endl;

    std::vector<sal_uLong> aSuperList;
    SvMetaClassList classList;
    SvSlotElementList aSlotList;
    InsertSlots(aSlotList, aSuperList, classList, OString(), rBase);
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
        // at least one dummy
        WriteTab( rOutStm, 1 );
        rOutStm.WriteCharPtr("{ (const SfxType*) &aSfxVoidItem_Impl, 0, 0 }" ) << endl;
    }
    rOutStm << endl;
    rOutStm.WriteCharPtr( "};" ) << endl << endl;

    ByteStringList aStringList;
    WriteSlotStubs( GetName().getString(), aSlotList, aStringList, rOutStm );
    for ( size_t i = 0, n = aStringList.size(); i < n; ++i )
        delete aStringList[ i ];
    aStringList.clear();

    rOutStm << endl;

    // write slotmap
    rOutStm.WriteCharPtr("static SfxSlot a").WriteCharPtr(GetName().getString().getStr()).WriteCharPtr("Slots_Impl[] =") << endl;
    rOutStm.WriteChar( '{' ) << endl;

    // write all attributes
    WriteSlots( GetName().getString(), 0, aSlotList, rBase, rOutStm );
    if( nSlotCount )
        Back2Delemitter( rOutStm );
    else
    {
        // at least one dummy
        WriteTab( rOutStm, 1 );
        rOutStm.WriteCharPtr( "SFX_SLOT_ARG(" ).WriteCharPtr( GetName().getString().getStr() )
               .WriteCharPtr( ", 0, 0, " )
               .WriteCharPtr( "SFX_STUB_PTR_EXEC_NONE," )
               .WriteCharPtr( "SFX_STUB_PTR_STATE_NONE," )
               .WriteCharPtr( "SfxSlotMode::NONE, SfxVoidItem, 0, 0, \"\", SfxSlotMode::NONE )" ) << endl;
    }
    rOutStm << endl;
    rOutStm.WriteCharPtr( "};" ) << endl;
    rOutStm.WriteCharPtr( "#endif" ) << endl << endl;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
