/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_service.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:10:04 $
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

#ifndef ARY_IDL_I_SERVICE_HXX
#define ARY_IDL_I_SERVICE_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>

// USED SERVICES
#include <ary/idl/i_comrela.hxx>
#include <ary/stdconstiter.hxx>
#include <ary/idl/ik_service.hxx>




namespace ary
{
namespace info
{
    class CodeInformation;
}
namespace idl
{
namespace ifc_service
{
    struct attr;
}


/** Represents an IDL service.
*/
class Service : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2003 };     // See reposy.cxx

    // LIFECYCLE
                        Service(
                            const String &      i_sName,
                            Ce_id               i_nOwner );
                        ~Service();

    // INQUIRY
    void                Get_SupportedInterfaces(
                            Dyn_StdConstIterator<CommentedRelation> &
                                                o_rResult ) const;
    void                Get_IncludedServices(
                            Dyn_StdConstIterator<CommentedRelation> &
                                                o_rResult ) const;

    // ACCESS
    void                Add_Property(
                            Ce_id               i_nProperty );
    void                AddRef_IncludedService(
                            Type_id             i_nService,
                            DYN doc::OldIdlDocu *
                                                pass_dpDocu );
    void                AddRef_SupportedInterface(
                            Type_id             i_nInterface,
                            DYN doc::OldIdlDocu *
                                                pass_dpDocu );
  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface ary::Object:
    virtual ClassId     get_AryClass() const;

    // Interface CodeEntity
    virtual const String &  inq_LocalName() const;
    virtual Ce_id           inq_NameRoom() const;
    virtual Ce_id           inq_Owner() const;
    virtual E_SightLevel    inq_SightLevel() const;

    // Locals
    typedef std::vector< CommentedRelation >    RelationList;
    typedef std::vector<Ce_id>                  PropertyList;
    friend struct ifc_service::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;

    RelationList        aIncludedServices;
    RelationList        aSupportedInterfaces;
    PropertyList        aProperties;
};




// IMPLEMENTATION
inline void
Service::Add_Property( Ce_id i_nProperty )
    { aProperties.push_back(i_nProperty); }

inline void
Service::AddRef_IncludedService( Type_id                    i_nService,
                                 DYN doc::OldIdlDocu  *     pass_dpDocu )
    { aIncludedServices.push_back( CommentedRelation(i_nService, pass_dpDocu) ); }

inline void
Service::AddRef_SupportedInterface( Type_id                 i_nInterface,
                                    DYN doc::OldIdlDocu *   pass_dpDocu )
    { aSupportedInterfaces.push_back( CommentedRelation(i_nInterface, pass_dpDocu) ); }





}   // namespace idl
}   // namespace ary
#endif
