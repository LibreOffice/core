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

#include <precomp.h>
#include "hi_ary.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/ploc_dir.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_type.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/idl/ip_type.hxx>


inline const ary::idl::Gate &
AryAccess::gate() const
    { return rGate; }

inline const ary::idl::CePilot &
AryAccess::ces() const
    { return rGate.Ces(); }

inline const ary::idl::TypePilot &
AryAccess::types() const
    { return rGate.Types(); }

inline const ary::idl::Module *
AryAccess::find_SubModule( const ary::idl::Module & i_parent,
                           const String &           i_name ) const
{
    ary::idl::Ce_id
        nModule = i_parent.Search_Name(i_name);
    return ces().Search_Module(nModule);
}

bool
AryAccess::nextName( const char * &      io_TextPtr,
                     String &            o_name ) const
{
    if ( strncmp(io_TextPtr,"::", 2) == 0 )
        io_TextPtr += 2;

    const char *    pEnd = strchr(io_TextPtr,':');
    size_t          nLen = pEnd == 0
                                ?   strlen(io_TextPtr)
                                :   pEnd - io_TextPtr;
    o_name.assign(io_TextPtr, nLen);
    io_TextPtr += nLen;

    return nLen > 0;
}



AryAccess::AryAccess( const ary::idl::Gate & i_rGate )
    :   rGate(i_rGate)
{
}

const ary::idl::Module &
AryAccess::GlobalNamespace() const
{
    return ces().GlobalNamespace();
}

const ary::idl::Module &
AryAccess::Find_Module( ary::idl::Ce_id i_ce ) const
{
    return ces().Find_Module(i_ce);
}


const ary::idl::CodeEntity &
AryAccess::Find_Ce( ary::idl::Ce_id i_ce ) const
{
    return ces().Find_Ce(i_ce);
}

const ary::idl::Type &
AryAccess::Find_Type( ary::idl::Type_id i_type ) const
{
    return types().Find_Type(i_type);
}

ary::idl::Ce_id
AryAccess::CeFromType( ary::idl::Type_id i_type ) const
{
    return types().Search_CeRelatedTo(i_type);
}

bool
AryAccess::IsBuiltInOrRelated( const ary::idl::Type & i_type ) const
{
    return types().IsBuiltInOrRelated(i_type);
}

bool
AryAccess::Search_Ce( StringVector &            o_module,
                      String &                  o_mainEntity,
                      String &                  o_memberEntity,
                      const char *              i_sText,
                      const ary::idl::Module &  i_referingScope ) const
{
    o_module.erase(o_module.begin(),o_module.end());
    o_mainEntity = String::Null_();
    o_memberEntity = String::Null_();

    const ary::idl::Module *    pModule = 0;

    if ( strncmp(i_sText, "::", 2) == 0
         OR strncmp(i_sText, "com::sun::star", 14) == 0 )
        pModule = &GlobalNamespace();
    else
    {
        pModule = &i_referingScope;
        ces().Get_Text(o_module, o_mainEntity, o_memberEntity, *pModule);
    }

    const char *    pNext = i_sText;
    String          sNextName;

    // Find Module:
    while ( nextName(pNext, sNextName) )
    {
        const ary::idl::Module *
            pSub = find_SubModule(*pModule, sNextName);
        if (pSub != 0)
        {
            pModule = pSub;
            o_module.push_back(sNextName);
        }
        else
            break;
    }

    // Find main CodeEntity:
    if ( sNextName.length() == 0 )
        return true;
    const ary::idl::Ce_id
        nCe = pModule->Search_Name(sNextName);
    if (NOT nCe.IsValid())
        return false;
    o_mainEntity = sNextName;

    // Find member:
    if ( *pNext == 0 )
        return true;
    nextName(pNext, o_memberEntity);
    if (strchr(o_memberEntity,':') != 0)
        return false;   // This must not happen in IDL

    return true;
}

bool
AryAccess::Search_CesModule( StringVector &             o_module,
                             const String &             i_scope,
                             const String &             i_ce,
                             const ary::idl::Module &   i_referingScope ) const
{
    o_module.erase(o_module.begin(),o_module.end());

    const ary::idl::Module *
        pModule = 0;

    if ( strncmp(i_scope, "::", 2) == 0
         OR strncmp(i_scope, "com::sun::star", 14) == 0 )
        pModule = &GlobalNamespace();
    else
    {
        pModule = &i_referingScope;
        static String Dummy1;
        static String Dummy2;
        ces().Get_Text(o_module, Dummy1, Dummy2, *pModule);
    }

    const char *    pNext = i_scope;
    String          sNextName;

    // Find Module:
    while ( nextName(pNext, sNextName) )
    {
        const ary::idl::Module *
            pSub = find_SubModule(*pModule, sNextName);
        if (pSub != 0)
        {
            pModule = pSub;
            o_module.push_back(sNextName);
        }
        else
            return false;
    }  // end while
    return pModule->Search_Name(i_ce).IsValid();
}

const ary::idl::Module *
AryAccess::Search_Module( const StringVector & i_nameChain ) const
{
    const ary::idl::Module * ret =
        &GlobalNamespace();
    for ( StringVector::const_iterator it = i_nameChain.begin();
          it != i_nameChain.end();
          ++it )
    {
        ret = find_SubModule(*ret, *it);
        if (ret == 0)
            break;
    }   // end for
    return ret;
}

void
AryAccess::Get_CeText( StringVector &               o_module,
                       String &                     o_ce,
                       String &                     o_member,
                       const ary::idl::CodeEntity & i_ce ) const
{
    ces().Get_Text(o_module, o_ce, o_member, i_ce);
}

void
AryAccess::Get_TypeText( StringVector &         o_module,
                         String &               o_sCe,
                         ary::idl::Ce_id &      o_nCe,
                         int &                  o_sequenceCount,
                         const ary::idl::Type & i_type ) const
{
    i_type.Get_Text(o_module, o_sCe, o_nCe, o_sequenceCount, gate());
}

void
AryAccess::Get_IndexData( std::vector<ary::idl::Ce_id> &            o_data,
                          ary::idl::alphabetical_index::E_Letter    i_letter ) const
{
    rGate.Ces().Get_AlphabeticalIndex(o_data, i_letter);
}


const ary::idl::CePilot &
AryAccess::Ces() const
{
    return rGate.Ces();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
