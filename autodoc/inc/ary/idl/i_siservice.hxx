/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_siservice.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:10:38 $
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

#ifndef ARY_IDL_I_SISERVICE_HXX
#define ARY_IDL_I_SISERVICE_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>




namespace ary
{
namespace idl
{
namespace ifc_sglifcservice
{
    struct attr;
}


/** Represents an IDL single interface service.
*/
class SglIfcService : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2015 };     // See reposy.cxx

    // LIFECYCLE
                        SglIfcService(
                            const String &      i_sName,
                            Ce_id               i_nOwner,
                            Type_id             i_nBaseInterface );
                        ~SglIfcService();
    // ACCESS
    void                Add_Constructor(
                            Ce_id               i_nId );

    // INFO
    Type_id             BaseInterface() const   { return nBaseInterface; }

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
    typedef std::vector<Ce_id>          CtorList;
    friend struct ifc_sglifcservice::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;
    Type_id             nBaseInterface;

    CtorList            aConstructors;
};




// IMPLEMENTATION
inline void
SglIfcService::Add_Constructor( Ce_id i_nId )
{
    aConstructors.push_back(i_nId);
}




}   // namespace idl
}   // namespace ary
#endif
