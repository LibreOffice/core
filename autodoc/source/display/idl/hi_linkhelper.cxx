/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
nameChainLinker( const char * )
{
    static const String
        sModuleFileName_( output::ModuleFileName() );
    return sModuleFileName_;
}
