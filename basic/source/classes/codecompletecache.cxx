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
#include <officecfg/Office/BasicIDE.hxx>

namespace
{
    class theCodeCompleteOptions: public ::rtl::Static< CodeCompleteOptions, theCodeCompleteOptions >{};
}

CodeCompleteOptions::CodeCompleteOptions()
{
    bIsAutoCorrectOn = officecfg::Office::BasicIDE::Autocomplete::AutoCorrect::get();
    bIsAutoCloseParenthesisOn = officecfg::Office::BasicIDE::Autocomplete::AutocloseParenthesis::get();
    bIsAutoCloseQuotesOn = officecfg::Office::BasicIDE::Autocomplete::AutocloseDoubleQuotes::get();
    bIsProcedureAutoCompleteOn = officecfg::Office::BasicIDE::Autocomplete::AutocloseProc::get();
    bIsCodeCompleteOn = officecfg::Office::BasicIDE::Autocomplete::CodeComplete::get();
    bExtendedTypeDeclarationOn = officecfg::Office::BasicIDE::Autocomplete::UseExtended::get();
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
    return theCodeCompleteOptions::get().aMiscOptions.IsExperimentalMode() && theCodeCompleteOptions::get().bExtendedTypeDeclarationOn;
}

void CodeCompleteOptions::SetExtendedTypeDeclaration( const bool& b )
{
    theCodeCompleteOptions::get().bExtendedTypeDeclarationOn = b;
}

bool CodeCompleteOptions::IsProcedureAutoCompleteOn()
{
    return theCodeCompleteOptions::get().aMiscOptions.IsExperimentalMode() && theCodeCompleteOptions::get().bIsProcedureAutoCompleteOn;
}

void CodeCompleteOptions::SetProcedureAutoCompleteOn( const bool& b )
{
    theCodeCompleteOptions::get().bIsProcedureAutoCompleteOn = b;
}

bool CodeCompleteOptions::IsAutoCloseQuotesOn()
{
    return theCodeCompleteOptions::get().aMiscOptions.IsExperimentalMode() && theCodeCompleteOptions::get().bIsAutoCloseQuotesOn;
}

void CodeCompleteOptions::SetAutoCloseQuotesOn( const bool& b )
{
    theCodeCompleteOptions::get().bIsAutoCloseQuotesOn = b;
}

bool CodeCompleteOptions::IsAutoCloseParenthesisOn()
{
    return theCodeCompleteOptions::get().aMiscOptions.IsExperimentalMode() && theCodeCompleteOptions::get().bIsAutoCloseParenthesisOn;
}

void CodeCompleteOptions::SetAutoCloseParenthesisOn( const bool& b )
{
    theCodeCompleteOptions::get().bIsAutoCloseParenthesisOn = b;
}

bool CodeCompleteOptions::IsAutoCorrectOn()
{
    return theCodeCompleteOptions::get().aMiscOptions.IsExperimentalMode() && theCodeCompleteOptions::get().bIsAutoCorrectOn;
}

void CodeCompleteOptions::SetAutoCorrectOn( const bool& b )
{
    theCodeCompleteOptions::get().bIsAutoCorrectOn = b;
}

std::ostream& operator<< (std::ostream& aStream, const CodeCompleteDataCache& aCache)
{
    aStream << "Global variables" << std::endl;
    for(CodeCompleteVarTypes::const_iterator aIt = aCache.aGlobalVars.begin(); aIt != aCache.aGlobalVars.end(); ++aIt )
    {
        aStream << aIt->first << "," << aIt->second << std::endl;
    }
    aStream << "Local variables" << std::endl;
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

void CodeCompleteDataCache::SetVars( const CodeCompleteVarScopes& aScopes )
{
    aVarScopes = aScopes;
}

void CodeCompleteDataCache::print() const
{
    std::cerr << *this << std::endl;
}

void CodeCompleteDataCache::Clear()
{
    aVarScopes.clear();
}

void CodeCompleteDataCache::InsertGlobalVar( const OUString& sVarName, const OUString& sVarType )
{
    aGlobalVars.insert( CodeCompleteVarTypes::value_type(sVarName, sVarType) );
}

void CodeCompleteDataCache::InsertLocalVar( const OUString& sProcName, const OUString& sVarName, const OUString& sVarType )
{
    CodeCompleteVarScopes::const_iterator aIt = aVarScopes.find( sProcName );
    if( aIt == aVarScopes.end() ) //new procedure
    {
        CodeCompleteVarTypes aTypes;
        aTypes.insert( CodeCompleteVarTypes::value_type(sVarName, sVarType) );
        aVarScopes.insert( CodeCompleteVarScopes::value_type(sProcName, aTypes) );
    }
    else
    {
        CodeCompleteVarTypes aTypes = aVarScopes[ sProcName ];
        aTypes.insert( CodeCompleteVarTypes::value_type(sVarName, sVarType) );
        aVarScopes[ sProcName ] = aTypes;
    }
}

OUString CodeCompleteDataCache::GetVarType( const OUString& sVarName ) const
{
    for( CodeCompleteVarScopes::const_iterator aIt = aVarScopes.begin(); aIt != aVarScopes.end(); ++aIt )
    {
        CodeCompleteVarTypes aTypes = aIt->second;
        for( CodeCompleteVarTypes::const_iterator aOtherIt = aTypes.begin(); aOtherIt != aTypes.end(); ++aOtherIt )
        {
            if( aOtherIt->first.equalsIgnoreAsciiCase( sVarName ) )
            {
                return aOtherIt->second;
            }
        }
    }
    //not a local, search global scope
    for( CodeCompleteVarTypes::const_iterator aIt = aGlobalVars.begin(); aIt != aGlobalVars.end(); ++aIt )
    {
        if( aIt->first.equalsIgnoreAsciiCase( sVarName ) )
            return aIt->second;
    }
    return OUString(""); //not found
}

OUString CodeCompleteDataCache::GetCorrectCaseVarName( const OUString& sVarName, const OUString& sActProcName ) const
{
    for( CodeCompleteVarScopes::const_iterator aIt = aVarScopes.begin(); aIt != aVarScopes.end(); ++aIt )
    {
        CodeCompleteVarTypes aTypes = aIt->second;
        for( CodeCompleteVarTypes::const_iterator aOtherIt = aTypes.begin(); aOtherIt != aTypes.end(); ++aOtherIt )
        {
            if( aOtherIt->first.equalsIgnoreAsciiCase( sVarName ) && aIt->first.equalsIgnoreAsciiCase( sActProcName ) )
            {
                return aOtherIt->first;
            }
        }
    }
    // search global scope
    for( CodeCompleteVarTypes::const_iterator aIt = aGlobalVars.begin(); aIt != aGlobalVars.end(); ++aIt )
    {
        if( aIt->first.equalsIgnoreAsciiCase( sVarName ) )
            return aIt->first;
    }
    return OUString(""); //not found
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
