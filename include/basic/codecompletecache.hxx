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

#ifndef CODECOMPLETECACHE_H
#define CODECOMPLETECACHE_H

#include <basic/sbdef.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbxdef.hxx>
#include <boost/utility.hpp>
#include <boost/unordered_map.hpp>
#include <rtl/ustring.hxx>
#include <svtools/miscopt.hxx>
#include <vector>

typedef boost::unordered_map< OUString, OUString, OUStringHash > CodeCompleteVarTypes;
/* variable name, type */
typedef boost::unordered_map< OUString, CodeCompleteVarTypes, OUStringHash > CodeCompleteVarScopes;
/* procedure, CodeCompleteVarTypes */

class BASIC_DLLPUBLIC CodeCompleteOptions
{
/*
 * class to store basic code completition
 * options
 * */
private:
    bool bIsCodeCompleteOn;
    bool bIsProcedureAutoCompleteOn;
    bool bIsAutoCloseQuotesOn;
    bool bIsAutoCloseParenthesisOn;
    bool bIsAutoCorrectOn;
    bool bExtendedTypeDeclarationOn;
    SvtMiscOptions aMiscOptions;

public:
    CodeCompleteOptions();

    static bool IsCodeCompleteOn();
    static void SetCodeCompleteOn( const bool& b );

    static bool IsExtendedTypeDeclaration();
    static void SetExtendedTypeDeclaration( const bool& b );

    static bool IsProcedureAutoCompleteOn();
    static void SetProcedureAutoCompleteOn( const bool& b );

    static bool IsAutoCloseQuotesOn();
    static void SetAutoCloseQuotesOn( const bool& b );

    static bool IsAutoCloseParenthesisOn();
    static void SetAutoCloseParenthesisOn( const bool& b );

    static bool IsAutoCorrectOn();
    static void SetAutoCorrectOn( const bool& b );
};

class BASIC_DLLPUBLIC CodeCompleteDataCache
{
/*
 * cache to store data for
 * code completition
 * */
private:
    CodeCompleteVarScopes aVarScopes;
    CodeCompleteVarTypes aGlobalVars;

public:
    CodeCompleteDataCache(){}
    virtual ~CodeCompleteDataCache(){}

    friend BASIC_DLLPUBLIC std::ostream& operator<< (std::ostream& aStream, const CodeCompleteDataCache& aCache);

    void SetVars( const CodeCompleteVarScopes& aScopes );
    const CodeCompleteVarScopes& GetVars() const;

    void InsertGlobalVar( const OUString& sVarName, const OUString& sVarType );
    void InsertLocalVar( const OUString& sProcName, const OUString& sVarName, const OUString& sVarType );
    OUString GetVarType( const OUString& sVarName ) const;
    OUString GetCorrectCaseVarName( const OUString& sVarName, const OUString& sActProcName ) const;
    void print() const; // wrapper for operator<<, prints to std::cerr
    void Clear();
};

#endif // CODECOMPLETECACHE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
