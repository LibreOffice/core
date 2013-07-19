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

const OUString& CodeCompleteDataCache::GetVariableType( const OUString& sVarName, const OUString& sProcName ) const
{
    CodeCompleteVarScopes::const_iterator aIt = aVarScopes.find( sProcName );
    if( aIt == aVarScopes.end() )//procedure does not exist
        return NOT_FOUND;

    CodeCompleteVarTypes aVarTypes = aIt->second;
    CodeCompleteVarTypes::const_iterator aOtherIt = aVarTypes.find( sVarName );
    if( aOtherIt == aVarTypes.end() )
        return NOT_FOUND;
    else
        return aOtherIt->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
