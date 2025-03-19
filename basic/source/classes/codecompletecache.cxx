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
#include <officecfg/Office/BasicIDE.hxx>
#include <officecfg/Office/Common.hxx>

namespace
{
CodeCompleteOptions& theCodeCompleteOptions()
{
    static CodeCompleteOptions SINGLETON;
    return SINGLETON;
}
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
    return theCodeCompleteOptions().bIsCodeCompleteOn;
}

void CodeCompleteOptions::SetCodeCompleteOn( bool b )
{
    theCodeCompleteOptions().bIsCodeCompleteOn = b;
}

bool CodeCompleteOptions::IsExtendedTypeDeclaration()
{
    return theCodeCompleteOptions().bExtendedTypeDeclarationOn;
}

void CodeCompleteOptions::SetExtendedTypeDeclaration( bool b )
{
    theCodeCompleteOptions().bExtendedTypeDeclarationOn = b;
}

bool CodeCompleteOptions::IsProcedureAutoCompleteOn()
{
    return theCodeCompleteOptions().bIsProcedureAutoCompleteOn;
}

void CodeCompleteOptions::SetProcedureAutoCompleteOn( bool b )
{
    theCodeCompleteOptions().bIsProcedureAutoCompleteOn = b;
}

bool CodeCompleteOptions::IsAutoCloseQuotesOn()
{
    return theCodeCompleteOptions().bIsAutoCloseQuotesOn;
}

void CodeCompleteOptions::SetAutoCloseQuotesOn( bool b )
{
    theCodeCompleteOptions().bIsAutoCloseQuotesOn = b;
}

bool CodeCompleteOptions::IsAutoCloseParenthesisOn()
{
    return theCodeCompleteOptions().bIsAutoCloseParenthesisOn;
}

void CodeCompleteOptions::SetAutoCloseParenthesisOn( bool b )
{
    theCodeCompleteOptions().bIsAutoCloseParenthesisOn = b;
}

bool CodeCompleteOptions::IsAutoCorrectOn()
{
    return theCodeCompleteOptions().bIsAutoCorrectOn;
}

void CodeCompleteOptions::SetAutoCorrectOn( bool b )
{
    theCodeCompleteOptions().bIsAutoCorrectOn = b;
}

std::ostream& operator<< (std::ostream& aStream, const CodeCompleteDataCache& aCache)
{
    aStream << "Global variables" << std::endl;
    for (auto const& globalVar : aCache.aGlobalVars)
    {
        aStream << globalVar.first << "," << globalVar.second << std::endl;
    }
    aStream << "Local variables" << std::endl;
    for (auto const& varScope : aCache.aVarScopes)
    {
        aStream << varScope.first << std::endl;
        CodeCompleteVarTypes aVarTypes = varScope.second;
        for (auto const& varType : aVarTypes)
        {
            aStream << "\t" << varType.first << "," << varType.second << std::endl;
        }
    }
    aStream << "-----------------" << std::endl;
    return aStream;
}

void CodeCompleteDataCache::Clear()
{
    aVarScopes.clear();
    aGlobalVars.clear();
}

void CodeCompleteDataCache::InsertGlobalVar( const OUString& sVarName, const OUString& sVarType )
{
    aGlobalVars.emplace( sVarName, sVarType );
}

void CodeCompleteDataCache::InsertLocalVar( const OUString& sProcName, const OUString& sVarName, const OUString& sVarType )
{
    CodeCompleteVarScopes::const_iterator aIt = aVarScopes.find( sProcName );
    if( aIt == aVarScopes.end() ) //new procedure
    {
        CodeCompleteVarTypes aTypes;
        aTypes.emplace( sVarName, sVarType );
        aVarScopes.emplace( sProcName, aTypes );
    }
    else
    {
        CodeCompleteVarTypes aTypes = aVarScopes[ sProcName ];
        aTypes.emplace( sVarName, sVarType );
        aVarScopes[ sProcName ] = std::move(aTypes);
    }
}

OUString CodeCompleteDataCache::GetVarType( std::u16string_view sVarName ) const
{
    for (auto const& varScope : aVarScopes)
    {
        CodeCompleteVarTypes aTypes = varScope.second;
        for (auto const& elem : aTypes)
        {
            if( elem.first.equalsIgnoreAsciiCase( sVarName ) )
            {
                return elem.second;
            }
        }
    }
    //not a local, search global scope
    for (auto const& globalVar : aGlobalVars)
    {
        if( globalVar.first.equalsIgnoreAsciiCase( sVarName ) )
            return globalVar.second;
    }
    return OUString(); //not found
}

OUString CodeCompleteDataCache::GetCorrectCaseVarName( std::u16string_view sVarName, std::u16string_view sActProcName ) const
{
    for (auto const& varScope : aVarScopes)
    {
        CodeCompleteVarTypes aTypes = varScope.second;
        for (auto const& elem : aTypes)
        {
            if( elem.first.equalsIgnoreAsciiCase( sVarName ) && varScope.first.equalsIgnoreAsciiCase( sActProcName ) )
            {
                return elem.first;
            }
        }
    }
    // search global scope
    for (auto const& globalVar : aGlobalVars)
    {
        if( globalVar.first.equalsIgnoreAsciiCase( sVarName ) )
            return globalVar.first;
    }
    return OUString(); //not found
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
