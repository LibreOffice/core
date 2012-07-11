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

#include <precomp.h>
#include "hi_linkhelper.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_module.hxx>




const ary::idl::Module *
LinkHelper::Search_CurModule() const
{
    return Search_Module( rEnv.CurPosition().RelatedNode() );
}

const ary::idl::Module *
LinkHelper::Search_Module( output::Node & i_node ) const
{
    static StringVector aNames_;

    output::Node::relative_id
        nId = i_node.RelatedNameRoom();
    if (nId == 0)
    {
        csv::erase_container(aNames_);
        i_node.Get_Chain(aNames_);
        const ary::idl::Module *  pModule =
            rEnv.Data().Search_Module(aNames_);
        if ( pModule == 0 )
            return 0;
        nId = static_cast<output::Node::relative_id>(pModule->Id());
        rEnv.CurPosition().RelatedNode().Set_RelatedNameRoom(nId);
    }

    return & rEnv.Data().Find_Module( ary::idl::Ce_id(nId) );
}

namespace
{
    const String C_sXrefsSuffix("-xref");
}


LinkHelper::OutPosition
LinkHelper::PositionOf_CurXRefs( const String & i_ceName ) const
{
    StreamLock sl(100);
    return OutPosition( rEnv.CurPosition(),
                        sl()    << i_ceName
                                << C_sXrefsSuffix
                                << ".html"
                                << c_str );
}

const String &
LinkHelper::XrefsSuffix() const
{
    return C_sXrefsSuffix;
}


String
nameChainLinker( SAL_UNUSED_PARAMETER const char * )
{
    static const String
        sModuleFileName_( output::ModuleFileName() );
    return sModuleFileName_;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
