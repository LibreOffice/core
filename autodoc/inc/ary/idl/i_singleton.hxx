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

#ifndef ARY_IDL_I_SINGLETON_HXX
#define ARY_IDL_I_SINGLETON_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>




namespace ary
{
namespace idl
{
namespace ifc_singleton
{
    struct attr;
}


/** Represents an IDL singleton.
*/
class Singleton : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2013 };

    // LIFECYCLE
                        Singleton(
                            const String &      i_sName,
                            Ce_id               i_nOwner );
                        ~Singleton();
    // INQUIRY
    Type_id             AssociatedService() const
                                                { return nService; }

    // ACCESS
    void                Set_Service(
                            Type_id             i_nService );
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
    friend struct ifc_singleton::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;

    Type_id             nService;
};





// IMPLEMENTATION
inline void
Singleton::Set_Service( Type_id i_nService )
{
    nService = i_nService;
}




}   // namespace idl
}   // namespace ary
#endif
