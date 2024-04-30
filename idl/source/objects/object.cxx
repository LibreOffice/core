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

#include <rtl/strbuf.hxx>
#include <sal/log.hxx>

#include <object.hxx>
#include <globals.hxx>
#include <database.hxx>
#include <slot.hxx>


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
            throw SvParseException( rInStm, "unknown imported interface"_ostr );
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
        SAL_WARN( "idl", "new slot : " << rAttr.GetSlotId().getString() );
    }

    for( const auto &pS : aAttrList )
    {
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
    for ( const auto& pAttr : rSlotList )
    {
        nCount = nCount + pAttr->WriteSlotParamArray( rBase, rOutStm );
    }

    return nCount;
}

sal_uInt16 SvMetaClass::WriteSlots( std::string_view rShellName,
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

void SvMetaClass::InsertSlots( SvSlotElementList& rList, std::vector<sal_uInt32>& rSuperList,
                            SvMetaClassList &rClassList,
                            const OString& rPrefix, SvIdlDataBase& rBase)
{
    // was this class already written?
    if ( std::find( rClassList.begin(), rClassList.end(), this ) != rClassList.end() )
        return;

    rClassList.push_back( this );

    // write all direct attributes
    for( const auto& pAttr : aAttrList )
    {
        sal_uInt32 nId = pAttr->GetSlotId().GetValue();

        std::vector<sal_uInt32>::iterator iter = std::find(rSuperList.begin(),
                                                      rSuperList.end(),nId);

        if( iter == rSuperList.end() )
        {
            // Write only if not already written by subclass or
            // imported interface.
            rSuperList.push_back(nId);
            pAttr->Insert(rList);
        }
    }

    // All Interfaces already imported by SuperShell should not be
    // written any more.
    // It is prohibited that Shell and SuperShell directly import the same
    // class.
    if( GetMetaTypeType() == MetaTypeType::Shell && aSuperClass.is() )
        aSuperClass->FillClasses( rClassList );

    // Write all attributes of the imported classes, as long as they have
    // not already been imported by the superclass.
    for( auto& rElement : aClassElementList )
    {
        SvMetaClass * pCl = rElement.GetClass();
        OStringBuffer rPre(rPrefix.getLength() + 1 + rElement.GetPrefix().getLength());
        rPre.append(rPrefix);
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

void SvMetaClass::FillClasses( SvMetaClassList & rClassList )
{
    // Am I not yet in?
    if ( std::find( rClassList.begin(), rClassList.end(), this ) != rClassList.end() )
        return;

    rClassList.push_back( this );

    // my imports
    for( auto& rElement : aClassElementList )
    {
        SvMetaClass * pCl = rElement.GetClass();
        pCl->FillClasses( rClassList );
    }

    // my superclass
    if( aSuperClass.is() )
        aSuperClass->FillClasses( rClassList );
}


void SvMetaClass::WriteSlotStubs( std::string_view rShellName,
                                SvSlotElementList & rSlotList,
                                std::vector<OString> & rList,
                                SvStream & rOutStm )
{
    // write all attributes
    for ( const auto& pAttr : rSlotList )
    {
        pAttr->WriteSlotStubs( rShellName, rList, rOutStm );
    }
}

void SvMetaClass::WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm )
{
    WriteStars( rOutStm );
    // define class
    rOutStm.WriteOString( "#ifdef ShellClass_" ).WriteOString( GetName() ) << endl;
    rOutStm.WriteOString( "#undef ShellClass" ) << endl;
    rOutStm.WriteOString( "#undef ShellClass_" ).WriteOString( GetName() ) << endl;
    rOutStm.WriteOString( "#define ShellClass " ).WriteOString( GetName() ) << endl;

    // no slotmaps get written for interfaces
    if( GetMetaTypeType() != MetaTypeType::Shell )
    {
        rOutStm.WriteOString( "#endif" ) << endl << endl;
        return;
    }
    // write parameter array
    rOutStm.WriteOString("static SfxFormalArgument a").WriteOString(GetName()).WriteOString("Args_Impl[] =") << endl;
    rOutStm.WriteChar('{') << endl;

    std::vector<sal_uInt32> aSuperList;
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
        rOutStm.WriteOString("{ (const SfxType*) &aSfxVoidItem_Impl, u\"\"_ustr, 0 }" ) << endl;
    }
    rOutStm << endl;
    rOutStm.WriteOString( "};" ) << endl << endl;

    std::vector<OString> aStringList;
    WriteSlotStubs( GetName(), aSlotList, aStringList, rOutStm );
    aStringList.clear();

    rOutStm << endl;

    // write slotmap
    rOutStm.WriteOString("static SfxSlot a").WriteOString(GetName()).WriteOString("Slots_Impl[] =") << endl;
    rOutStm.WriteChar( '{' ) << endl;

    // write all attributes
    WriteSlots( GetName(), aSlotList, rBase, rOutStm );
    if( nSlotCount )
        Back2Delimiter( rOutStm );
    else
    {
        // at least one dummy
        WriteTab( rOutStm, 1 );
        rOutStm.WriteOString( "SFX_SLOT_ARG(" ).WriteOString( GetName() )
               .WriteOString( ", 0, SfxGroupId::NONE, " )
               .WriteOString( "SFX_STUB_PTR_EXEC_NONE," )
               .WriteOString( "SFX_STUB_PTR_STATE_NONE," )
               .WriteOString( "SfxSlotMode::NONE, SfxVoidItem, 0, 0, u\"\"_ustr, SfxSlotMode::NONE )" ) << endl;
    }
    rOutStm << endl;
    rOutStm.WriteOString( "};" ) << endl;
    rOutStm.WriteOString( "#endif" ) << endl << endl;

    for( auto& pAttr : aSlotList )
    {
        pAttr->ResetSlotPointer();
    }

    aSlotList.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
