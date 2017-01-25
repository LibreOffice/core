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

#include <sal/config.h>

#include <algorithm>
#include <ctype.h>

#include <rtl/strbuf.hxx>
#include <osl/diagnose.h>

#include <object.hxx>
#include <globals.hxx>
#include <database.hxx>


SvClassElement::SvClassElement()
{
};

SvMetaClass::SvMetaClass()
{
}

void SvMetaClass::ReadContextSvIdl( SvIdlDataBase & rBase,
                                    SvTokenStream & rInStm )
{
    sal_uInt32  nTokPos = rInStm.Tell();
    SvToken&    rTok = rInStm.GetToken_Next();

    if( rTok.Is( SvHash_import() ) )
    {
        SvMetaClass * pClass = rBase.ReadKnownClass( rInStm );
        if( !pClass )
            throw SvParseException( rInStm, "unknown imported interface" );
        SvClassElement aEle;
        aEle.SetClass( pClass );
        aClassElementList.push_back( aEle );

        rTok = rInStm.GetToken();
        if( rTok.IsString() )
        {
            aEle.SetPrefix( rTok.GetString() );
            rInStm.GetToken_Next();
        }
        return;
    }
    else
    {
        rInStm.Seek( nTokPos );
        SvMetaType * pType = rBase.ReadKnownType( rInStm );

        bool bOk = false;
        tools::SvRef<SvMetaAttribute> xAttr;
        if( !pType || pType->IsItem() )
        {
            xAttr = new SvMetaSlot( pType );
            if( xAttr->ReadSvIdl( rBase, rInStm ) )
                bOk = xAttr->Test( rInStm );
        }
        else
        {
            xAttr = new SvMetaAttribute( pType );
            if( xAttr->ReadSvIdl( rBase, rInStm ) )
                bOk = xAttr->Test( rInStm );
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
            aAttrList.push_back( xAttr.get() );
            return;
        }
    }
    rInStm.Seek( nTokPos );
}

bool SvMetaClass::TestAttribute( SvIdlDataBase & rBase, SvTokenStream & rInStm,
                                 SvMetaAttribute & rAttr ) const
{
    if ( !rAttr.GetRef() && dynamic_cast<const SvMetaSlot *>(&rAttr) )
    {
        OSL_FAIL( "Neuer Slot : " );
        OSL_FAIL( rAttr.GetSlotId().getString().getStr() );
    }

    for( sal_uLong n = 0; n < aAttrList.size(); n++ )
    {
        SvMetaAttribute * pS = aAttrList[n];
        if( pS->GetName() == rAttr.GetName() )
        {
            // values have to match
            if( pS->GetSlotId().GetValue() != rAttr.GetSlotId().GetValue() )
            {
                throw SvParseException( rInStm, "Attribute's " + pS->GetName() + " with different id's");
             }
        }
        else
        {
            sal_uInt32 nId1 = pS->GetSlotId().GetValue();
            sal_uInt32 nId2 = rAttr.GetSlotId().GetValue();
            if( nId1 == nId2 && nId1 != 0 )
            {
                OString aStr = "Attribute " + pS->GetName() + " and Attribute " + rAttr.GetName() + " with equal id's";
                throw SvParseException(rInStm, aStr);
             }
        }
    }
    SvMetaClass * pSC = aSuperClass.get();
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
        SvMetaSlot *pAttr = rSlotList[ i ];
        nCount = nCount + pAttr->WriteSlotParamArray( rBase, rOutStm );
    }

    return nCount;
}

sal_uInt16 SvMetaClass::WriteSlots( const OString& rShellName,
                                SvSlotElementList & rSlotList,
                                SvIdlDataBase & rBase,
                                SvStream & rOutStm )
{
    sal_uInt16 nSCount = 0;
    for ( size_t i = 0, n = rSlotList.size(); i < n; ++i )
    {
        SvMetaSlot * pAttr = rSlotList[ i ];
        nSCount = nSCount + pAttr->WriteSlotMap( rShellName, nSCount,
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
    if( GetMetaTypeType() == MetaTypeType::Shell && aSuperClass.is() )
        aSuperClass->FillClasses( rClassList );

    // Write all attributes of the imported classes, as long as they have
    // not already been imported by the superclass.
    for( n = 0; n < aClassElementList.size(); n++ )
    {
        SvClassElement& rElement = aClassElementList[n];
        SvMetaClass * pCl = rElement.GetClass();
        OStringBuffer rPre(rPrefix);
        if( !rPre.isEmpty() && !rElement.GetPrefix().isEmpty() )
            rPre.append('.');
        rPre.append(rElement.GetPrefix());

        // first of all write direct imported interfaces
        pCl->InsertSlots( rList, rSuperList, rClassList,
            rPre.makeStringAndClear(), rBase );
    }

    // only write superclass if no shell and not in the list
    if( GetMetaTypeType() != MetaTypeType::Shell && aSuperClass.is() )
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
    for( size_t n = 0; n < aClassElementList.size(); n++ )
    {
        SvClassElement& rElement = aClassElementList[n];
        SvMetaClass * pCl = rElement.GetClass();
        pCl->FillClasses( rList );
    }

    // my superclass
    if( aSuperClass.is() )
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
        SvMetaSlot *pAttr = rSlotList[ i ];
        pAttr->WriteSlotStubs( rShellName, rList, rOutStm );
    }
}

void SvMetaClass::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    WriteStars( rOutStm );
    // define class
    rOutStm.WriteCharPtr( "#ifdef " ).WriteOString( GetName() ) << endl;
    rOutStm.WriteCharPtr( "#undef ShellClass" ) << endl;
    rOutStm.WriteCharPtr( "#undef " ).WriteOString( GetName() ) << endl;
    rOutStm.WriteCharPtr( "#define ShellClass " ).WriteOString( GetName() ) << endl;

    // no slotmaps get written for interfaces
    if( GetMetaTypeType() != MetaTypeType::Shell )
    {
        rOutStm.WriteCharPtr( "#endif" ) << endl << endl;
        return;
    }
    // write parameter array
    rOutStm.WriteCharPtr("static SfxFormalArgument a").WriteOString(GetName()).WriteCharPtr("Args_Impl[] =") << endl;
    rOutStm.WriteChar('{') << endl;

    std::vector<sal_uLong> aSuperList;
    SvMetaClassList classList;
    SvSlotElementList aSlotList;
    InsertSlots(aSlotList, aSuperList, classList, OString(), rBase);
    for ( size_t i = 0, n = aSlotList.size(); i < n; ++i )
    {
        SvMetaSlot *pSlot = aSlotList[ i ];
        pSlot->SetListPos( i );
    }

    size_t nSlotCount = aSlotList.size();

    // write all attributes
    sal_uInt16 nArgCount = WriteSlotParamArray( rBase, aSlotList, rOutStm );
    if( nArgCount )
        Back2Delimiter( rOutStm );
    else
    {
        // at least one dummy
        WriteTab( rOutStm, 1 );
        rOutStm.WriteCharPtr("{ (const SfxType*) &aSfxVoidItem_Impl, 0, 0 }" ) << endl;
    }
    rOutStm << endl;
    rOutStm.WriteCharPtr( "};" ) << endl << endl;

    ByteStringList aStringList;
    WriteSlotStubs( GetName(), aSlotList, aStringList, rOutStm );
    for ( size_t i = 0, n = aStringList.size(); i < n; ++i )
        delete aStringList[ i ];
    aStringList.clear();

    rOutStm << endl;

    // write slotmap
    rOutStm.WriteCharPtr("static SfxSlot a").WriteOString(GetName()).WriteCharPtr("Slots_Impl[] =") << endl;
    rOutStm.WriteChar( '{' ) << endl;

    // write all attributes
    WriteSlots( GetName(), aSlotList, rBase, rOutStm );
    if( nSlotCount )
        Back2Delimiter( rOutStm );
    else
    {
        // at least one dummy
        WriteTab( rOutStm, 1 );
        rOutStm.WriteCharPtr( "SFX_SLOT_ARG(" ).WriteOString( GetName() )
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
        SvMetaSlot* pAttr = aSlotList[ i ];
        pAttr->ResetSlotPointer();
    }

    aSlotList.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
