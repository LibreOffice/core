/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hi_linkhelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 16:51:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_autodoc.hxx"


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

LinkHelper::OutPosition
LinkHelper::PositionOf_Ce(const CE & i_ce) const
{
    static StringVector aModule_;
    csv::erase_container(aModule_);
    String sCe;
    String sMember;
    rEnv.Data().Get_CeText(aModule_, sCe, sMember, i_ce);
    output::Node &
        rNode = rEnv.OutputTree().RootNode().Provide_Child(aModule_);
    return OutPosition(rNode,sCe);
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
