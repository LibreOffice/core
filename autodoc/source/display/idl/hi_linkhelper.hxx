/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef ADC_DISPLAY_HI_LINKHELPER_HXX
#define ADC_DISPLAY_HI_LINKHELPER_HXX


// USED SERVICES
    // BASE CLASSES               
    // COMPONENTS
    // PARAMETERS
#include "hi_ary.hxx"
#include "hi_env.hxx"
#include <toolkit/out_position.hxx>
#include <toolkit/out_tree.hxx>


class LinkHelper
{
  public:
    typedef ary::idl::CodeEntity                CE;
    typedef output::Position                    OutPosition;

                        LinkHelper(
                            HtmlEnvironment_Idl &
                                                io_rEnv )
                            :   rEnv(io_rEnv) {}

    OutPosition         PositionOf_CurModule() const
                        { return OutPosition( rEnv.CurPosition(),
                                              output::ModuleFileName()); }

    OutPosition         PositionOf_CurXRefs(
                            const String &      i_ceName) const;
    OutPosition         PositionOf_Index() const
                        { OutPosition ret1 = rEnv.OutputTree().IndexRoot();
                          return OutPosition( ret1, String(output::IndexFile_A()) ); }

                                        
    const ary::idl::Module *  
                        Search_CurModule() const;
    const ary::idl::Module *  
                        Search_Module(
                            output::Node &      i_node ) const;
                               
    const CE *          Search_CeFromType(
                            ary::idl::Type_id   i_type ) const;

    void                Get_Link2Position(
                            StreamStr &         o_link,
                            OutPosition &       i_pos ) const
                        { rEnv.CurPosition().Get_LinkTo(o_link, i_pos); }

    void                Get_Link2Member(
                            StreamStr &         o_link,
                            OutPosition &       i_ownerPos,
                            const String &      i_memberName ) const
                        { Get_Link2Position(o_link, i_ownerPos);
                          o_link << "#" << i_memberName; }     
    const String &      XrefsSuffix() const;                               
                          
  private:                                                     
    // DATA
    mutable HtmlEnvironment_Idl &
                        rEnv;
};     

inline const ary::idl::CodeEntity *          
LinkHelper::Search_CeFromType( ary::idl::Type_id i_type ) const
{ 
    ary::idl::Ce_id nCe = rEnv.Data().CeFromType(i_type);
    if (nCe.IsValid())
        return &rEnv.Data().Find_Ce(nCe);
    return 0;
}    



String              nameChainLinker( 
                        const char *        i_levelName );


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
