/*************************************************************************
 *
 *  $RCSfile: i_service.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:07:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef ARY_IDL_I_SERVICE_HXX
#define ARY_IDL_I_SERVICE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/idl/i_ce.hxx>
    // COMPONENTS
#include <ary/idl/i_comrela.hxx>
    // PARAMETERS
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


/*  OPEN?
*/

/** @resp
    Represents an IDL service.
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
                            DYN info::CodeInformation *
                                                pass_dpDocu );
    void                AddRef_SupportedInterface(
                            Type_id             i_nInterface,
                            DYN info::CodeInformation *
                                                pass_dpDocu );
  private:
    // Interface ary::RepositoryEntity
    virtual RCid        inq_ClassId() const;

    // Interface CodeEntity
    virtual void            do_Visit_CeHost(CeHost & o_rHost) const;
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
Service::AddRef_IncludedService( Type_id                     i_nService,
                                 DYN info::CodeInformation * pass_dpDocu )
    { aIncludedServices.push_back( CommentedRelation(i_nService, pass_dpDocu) ); }

inline void
Service::AddRef_SupportedInterface( Type_id                     i_nInterface,
                                    DYN info::CodeInformation * pass_dpDocu )
    { aSupportedInterfaces.push_back( CommentedRelation(i_nInterface, pass_dpDocu) ); }



}   // namespace idl
}   // namespace ary


#endif
