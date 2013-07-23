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
    SvtMiscOptions aMiscOptions;

public:
    CodeCompleteOptions();

    static bool IsCodeCompleteOn();
    static void SetCodeCompleteOn( const bool& b );
    static bool IsExtendedTypeDeclaration();

    static bool IsProcedureAutoCompleteOn();
    static void SetProcedureAutoCompleteOn( const bool& b );
};

class BASIC_DLLPUBLIC CodeCompleteDataCache
{
/*
 * cache to store data for
 * code completition
 * */
private:
    CodeCompleteVarScopes aVarScopes;

public:
    static const OUString GLOB_KEY;
    static const OUString NOT_FOUND;

    CodeCompleteDataCache(){}
    virtual ~CodeCompleteDataCache(){}

    friend BASIC_DLLPUBLIC std::ostream& operator<< (std::ostream& aStream, const CodeCompleteDataCache& aCache);

    void SetVars( const CodeCompleteVarScopes& aScopes );
    const CodeCompleteVarScopes& GetVars() const;

    void InsertProcedure( const OUString& sProcName, const CodeCompleteVarTypes& aVarTypes );
    OUString GetVariableType( const OUString& sVarName, const OUString& sProcName ) const;
    void print() const; // wrapper for operator<<, prints to std::cerr
};

#endif // CODECOMPLETECACHE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
