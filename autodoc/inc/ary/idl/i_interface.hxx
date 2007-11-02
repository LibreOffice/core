/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_interface.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:08:11 $
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

#ifndef ARY_IDL_I_INTERFACE_HXX
#define ARY_IDL_I_INTERFACE_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>

// USED SERVICES
#include <ary/idl/i_comrela.hxx>
#include <ary/stdconstiter.hxx>




namespace ary
{
namespace idl
{
namespace ifc_interface
{
    struct attr;
}
    class Interface_2s;


/** Represents an IDL interface.
*/
class Interface : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2001 };

    // LIFECYCLE
                        Interface(
                            const String &      i_sName,
                            Ce_id               i_nOwner );
                        ~Interface();
    // INQUIRY
    bool                HasBase() const;

    // ACCESS
    void                Add_Function(
                            Ce_id               i_nId );
    void                Add_Attribute(
                            Ce_id               i_nId );
    void                Add_Base(
                            Type_id             i_nInterface,
                            DYN doc::OldIdlDocu *
                                                pass_dpDocu );

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface ary::Object:
    virtual ClassId     get_AryClass() const;

    // Interface CodeEntity:
    virtual const String &  inq_LocalName() const;
    virtual Ce_id           inq_NameRoom() const;
    virtual Ce_id           inq_Owner() const;
    virtual E_SightLevel    inq_SightLevel() const;

    // Local
    typedef std::vector< CommentedRelation >    RelationList;
    typedef std::vector<Ce_id>                  MemberList;
    friend struct ifc_interface::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;
    RelationList        aBases;
    MemberList          aFunctions;
    MemberList          aAttributes;
    Dyn<Interface_2s>   p2s;
};




// IMPLEMENTATION
inline bool
Interface::HasBase() const
    { return aBases.size() > 0; }
inline void
Interface::Add_Function( Ce_id i_nId )
    { aFunctions.push_back(i_nId); }
inline void
Interface::Add_Attribute( Ce_id i_nId )
    { aAttributes.push_back(i_nId); }
inline void
Interface::Add_Base( Type_id                i_nInterface,
                     DYN doc::OldIdlDocu *  pass_dpDocu )
    { aBases.push_back( CommentedRelation(i_nInterface, pass_dpDocu) ); }




}   // namespace idl
}   // namespace ary
#endif
