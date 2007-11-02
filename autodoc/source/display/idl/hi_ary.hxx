/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hi_ary.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:38:59 $
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

#ifndef ADC_DISPLAY_HI_ARY_HXX
#define ADC_DISPLAY_HI_ARY_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <ary/idl/i_types4idl.hxx>
    // PARAMETERS
#include <ary/idl/i_gate.hxx>
#include <ary/doc/d_docu.hxx>
#include <ary/doc/d_oldidldocu.hxx>


namespace ary
{
namespace idl
{
    class Module;
    class Gate;
    class CePilot;
    class TypePilot;
}
}
namespace output
{
    class Position;
}




inline const ary::doc::OldIdlDocu *
Get_IdlDocu(const ary::doc::Documentation & i_doc)
{
    return dynamic_cast< const ary::doc::OldIdlDocu* >(i_doc.Data());
}





/** A helper class to wrap the access to data in the Autodoc Repository.
*/
class AryAccess
{
  public:
    // LIFECYCLE
                        AryAccess(
                            const ary::idl::Gate &
                                                i_rGate );
    // INQUIRY
    const ary::idl::Module &
                        GlobalNamespace() const;
    const ary::idl::Module &
                        Find_Module(
                            ary::idl::Ce_id     i_ce ) const;
    const ary::idl::CodeEntity &
                        Find_Ce(
                            ary::idl::Ce_id     i_ce ) const;
    const ary::idl::Type &
                        Find_Type(
                            ary::idl::Type_id   i_type ) const;
    ary::idl::Ce_id     CeFromType(
                            ary::idl::Type_id   i_type ) const;
    bool                IsBuiltInOrRelated(
                            const ary::idl::Type &
                                                i_type ) const;
    bool                Search_Ce(
                            StringVector &      o_module,
                            String &            o_mainEntity,
                            String &            o_memberEntity,
                            const char *        i_sText,
                            const ary::idl::Module &
                                                i_referingScope ) const;
    bool                Search_CesModule(
                            StringVector &      o_module,
                            const String &      i_scope,
                            const String &      i_ce,
                            const ary::idl::Module &
                                                i_referingScope ) const;
    const ary::idl::Module *
                        Search_Module(
                            const StringVector &
                                                i_nameChain ) const;

    void                Get_CeText(
                            StringVector &      o_module,
                            String &            o_ce,
                            String &            o_member,
                            const ary::idl::CodeEntity &
                                                i_ce ) const;
    void                Get_TypeText(
                            StringVector &      o_module,
                            String &            o_sCe,
                            ary::idl::Ce_id  &  o_nCe,
                            int &               o_sequenceCount,
                            const ary::idl::Type &
                                                i_type ) const;
    void                Get_IndexData(
                            std::vector<ary::idl::Ce_id> &
                                                o_data,
                            ary::idl::alphabetical_index::E_Letter
                                                i_letter ) const;

    const ary::idl::CePilot &
                        Ces() const;
  private:
    const ary::idl::Module *
                        find_SubModule(
                            const ary::idl::Module &
                                                i_parent,
                            const String &      i_name ) const;

    /// Gets "::"-separated names out of a string.
    bool                nextName(
                            const char * &      io_TextPtr,
                            String &            o_name ) const;


    const ary::idl::Gate &
                        gate() const;
    const ary::idl::CePilot &
                        ces() const;
    const ary::idl::TypePilot &
                        types() const;
    // DATA
    const ary::idl::Gate &
                        rGate;
};


#endif


