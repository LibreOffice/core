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

#include <basic/codecompletecache.hxx>
#include <iostream>
#include <rtl/instance.hxx>

const OUString CodeCompleteDataCache::GLOB_KEY = OUString("global key");
const OUString CodeCompleteDataCache::NOT_FOUND = OUString("not found");

namespace
{
    class theCodeCompleteOptions: public ::rtl::Static< CodeCompleteOptions, theCodeCompleteOptions >{};
}

CodeCompleteOptions::CodeCompleteOptions()
: bIsCodeCompleteOn( false ),
bIsProcedureAutoCompleteOn( false )
{
}

bool CodeCompleteOptions::IsCodeCompleteOn()
{
    return theCodeCompleteOptions::get().aMiscOptions.IsExperimentalMode() && theCodeCompleteOptions::get().bIsCodeCompleteOn;
}

void CodeCompleteOptions::SetCodeCompleteOn( const bool& b )
{
    theCodeCompleteOptions::get().bIsCodeCompleteOn = b;
}

bool CodeCompleteOptions::IsExtendedTypeDeclaration()
{
    return CodeCompleteOptions::IsCodeCompleteOn();
}

bool CodeCompleteOptions::IsProcedureAutoCompleteOn()
{
    return theCodeCompleteOptions::get().aMiscOptions.IsExperimentalMode() && theCodeCompleteOptions::get().bIsProcedureAutoCompleteOn;
}

void CodeCompleteOptions::SetProcedureAutoCompleteOn( const bool& b )
{
    theCodeCompleteOptions::get().bIsProcedureAutoCompleteOn = b;
}

std::ostream& operator<< (std::ostream& aStream, const CodeCompleteDataCache& aCache)
{
    for( CodeCompleteVarScopes::const_iterator aIt = aCache.aVarScopes.begin(); aIt != aCache.aVarScopes.end(); ++aIt )
    {
        aStream << aIt->first << std::endl;
        CodeCompleteVarTypes aVarTypes = aIt->second;
        for( CodeCompleteVarTypes::const_iterator aOtherIt = aVarTypes.begin(); aOtherIt != aVarTypes.end(); ++aOtherIt )
        {
            aStream << "\t" << aOtherIt->first << "," << aOtherIt->second << std::endl;
        }
    }
    aStream << "-----------------" << std::endl;
    return aStream;
}

const CodeCompleteVarScopes& CodeCompleteDataCache::GetVars() const
{
    return aVarScopes;
}

void CodeCompleteDataCache::InsertProcedure( const OUString& sProcName, const CodeCompleteVarTypes& aVarTypes )
{
    aVarScopes.insert( CodeCompleteVarScopes::value_type(sProcName, aVarTypes) );
}
void CodeCompleteDataCache::SetVars( const CodeCompleteVarScopes& aScopes )
{
    aVarScopes = aScopes;
}

OUString CodeCompleteDataCache::GetVariableType( const OUString& sVarName, const OUString& sProcName ) const
{
    CodeCompleteVarScopes::const_iterator aIt = aVarScopes.find( sProcName );
    if( aIt == aVarScopes.end() )//procedure does not exist
        return CodeCompleteDataCache::NOT_FOUND;

    CodeCompleteVarTypes aVarTypes = aIt->second;
    CodeCompleteVarTypes::const_iterator aOtherIt = aVarTypes.find( sVarName );
    if( aOtherIt == aVarTypes.end() )
        return CodeCompleteDataCache::NOT_FOUND;
    else
        return aOtherIt->second;
}

void CodeCompleteDataCache::print() const
{
    std::cerr << *this << std::endl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
