/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hi_linkhelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:52:51 $
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
    OutPosition         PositionOf_Ce(
                            const CE &          i_ce ) const;

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


//        OutPosition         Search_Ce(
//                                String &            o_member,
//                                const char *        i_sText,
//                            OutPosition *       i_referingScope = 0 ) const;

//    OutPosition         PositionOf_Ce(
//                            const char *        i_sScope,
//                            const char *        i_sCeName ) const
//                        { const CE *
//                            pce = rEnv.Data().Search_Ce( i_sScope,
//                                                       i_sCeName );
//                          if (pce != 0) return rEnv.Data().PositionOfCe(*pce);
//                          else return OutPosition(); }

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
