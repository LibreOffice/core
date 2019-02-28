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

#ifndef INCLUDED_BASIC_CODECOMPLETECACHE_HXX
#define INCLUDED_BASIC_CODECOMPLETECACHE_HXX

#include <basic/basicdllapi.h>
#include <rtl/ustring.hxx>
#include <svtools/miscopt.hxx>
#include <unordered_map>

typedef std::unordered_map< OUString, OUString > CodeCompleteVarTypes;
/* variable name, type */
typedef std::unordered_map< OUString, CodeCompleteVarTypes > CodeCompleteVarScopes;
/* procedure, CodeCompleteVarTypes */

class BASIC_DLLPUBLIC CodeCompleteOptions
{
/*
 * class to store basic code completion
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
    static void SetCodeCompleteOn( bool b );

    static bool IsExtendedTypeDeclaration();
    static void SetExtendedTypeDeclaration( bool b );

    static bool IsProcedureAutoCompleteOn();
    static void SetProcedureAutoCompleteOn( bool b );

    static bool IsAutoCloseQuotesOn();
    static void SetAutoCloseQuotesOn( bool b );

    static bool IsAutoCloseParenthesisOn();
    static void SetAutoCloseParenthesisOn( bool b );

    static bool IsAutoCorrectOn();
    static void SetAutoCorrectOn( bool b );
};

class BASIC_DLLPUBLIC CodeCompleteDataCache final
{
/*
 * cache to store data for
 * code completion
 * */
private:
    CodeCompleteVarScopes aVarScopes;
    CodeCompleteVarTypes aGlobalVars;

public:
    CodeCompleteDataCache(){}

    friend BASIC_DLLPUBLIC std::ostream& operator<< (std::ostream& aStream, const CodeCompleteDataCache& aCache);

    void InsertGlobalVar( const OUString& sVarName, const OUString& sVarType );
    void InsertLocalVar( const OUString& sProcName, const OUString& sVarName, const OUString& sVarType );
    OUString GetVarType( const OUString& sVarName ) const;
    OUString GetCorrectCaseVarName( const OUString& sVarName, const OUString& sActProcName ) const;
    void Clear();
};

#endif // INCLUDED_BASIC_CODECOMPLETECACHE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
