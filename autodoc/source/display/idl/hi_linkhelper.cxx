/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

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
