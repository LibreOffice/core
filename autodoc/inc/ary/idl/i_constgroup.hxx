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

#ifndef ARY_IDL_I_CONSTGROUP_HXX
#define ARY_IDL_I_CONSTGROUP_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>




namespace ary
{
namespace idl
{
namespace ifc_constgroup
{
    struct attr;
}


/** Represents an IDL constants group.
*/
class ConstantsGroup : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2011 };

    // LIFECYCLE
                        ConstantsGroup(
                            const String &      i_sName,
                            Ce_id               i_nModule );
                        ~ConstantsGroup();
    // ACCESS
    void                Add_Constant(
                            Ce_id               i_nConstant );
  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface ary::Object
    virtual ClassId     get_AryClass() const;

    // Interface CodeEntity
    virtual const String &  inq_LocalName() const;
    virtual Ce_id           inq_NameRoom() const;
    virtual Ce_id           inq_Owner() const;
    virtual E_SightLevel    inq_SightLevel() const;

    // Locals
    typedef std::vector<Ce_id>    ConstantList;
    friend struct ifc_constgroup::attr;

    // DATA
    String              sName;
    Ce_id               nModule;

    ConstantList        aConstants;
};




// IMPLEMENTATION
inline void
ConstantsGroup::Add_Constant( Ce_id i_nConstant )
{
    aConstants.push_back(i_nConstant);
}




}   // namespace idl
}   // namespace ary
#endif
