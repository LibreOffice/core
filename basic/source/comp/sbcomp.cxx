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


#include <basic/sbx.hxx>
#include <parser.hxx>
#include <image.hxx>
#include <sbobjmod.hxx>
#include <svtools/miscopt.hxx>
#include <rtl/character.hxx>
#include <memory>

// This routine is defined here, so that the
// compiler can be loaded as a discrete segment.

bool SbModule::Compile()
{
    if( pImage )
        return true;
    StarBASIC* pBasic = dynamic_cast<StarBASIC*>( GetParent() );
    if( !pBasic )
        return false;
    SbxBase::ResetError();

    SbModule* pOld = GetSbData()->pCompMod;
    GetSbData()->pCompMod = this;

    auto pParser = std::make_unique<SbiParser>( pBasic, this );
    while( pParser->Parse() ) {}
    if( !pParser->GetErrors() )
        pParser->aGen.Save();
    pParser.reset();
    // for the disassembler
    if( pImage )
        pImage->aOUSource = aOUSource;

    GetSbData()->pCompMod = pOld;

    // compiling a module, the module-global
    // variables of all modules become invalid
    bool bRet = IsCompiled();
    if( bRet )
    {
        if( dynamic_cast<const SbObjModule*>( this) == nullptr )
            pBasic->ClearAllModuleVars();
        RemoveVars(); // remove 'this' Modules variables
        // clear all method statics
        for( sal_uInt16 i = 0; i < pMethods->Count(); i++ )
        {
            SbMethod* p = dynamic_cast<SbMethod*>( pMethods->Get( i )  );
            if( p )
                p->ClearStatics();
        }

        // #i31510 Init other libs only if Basic isn't running
        if( GetSbData()->pInst == nullptr )
        {
            SbxObject* pParent_ = pBasic->GetParent();
            if( pParent_ )
                pBasic = dynamic_cast<StarBASIC*>( pParent_ );
            if( pBasic )
                pBasic->ClearAllModuleVars();
        }
    }

    return bRet;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
