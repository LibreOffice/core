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

#include "dbmm_types.hxx"

#include "dbmm_global.hrc"
#include "dbmm_module.hxx"

namespace dbmm
{

    // helper
    OUString getScriptTypeDisplayName( const ScriptType _eType )
    {
        sal_uInt16 nResId( 0 );

        switch ( _eType )
        {
        case eBasic:        nResId = STR_OOO_BASIC;     break;
        case eBeanShell:    nResId = STR_BEAN_SHELL;    break;
        case eJavaScript:   nResId = STR_JAVA_SCRIPT;   break;
        case ePython:       nResId = STR_PYTHON;        break;
        case eJava:         nResId = STR_JAVA;          break;
        case eDialog:       nResId = STR_DIALOG;        break;
        }
        OSL_ENSURE( nResId != 0, "getScriptTypeDisplayName: illegal script type!" );
        return nResId ? OUString( MacroMigrationResId( nResId ) ) : OUString();
    }

} // namespace dbmm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
