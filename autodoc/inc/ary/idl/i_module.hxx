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

#ifndef ARY_IDL_I_MODULE_HXX
#define ARY_IDL_I_MODULE_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>

// USED SERVICES
#include <ary/stdconstiter.hxx>




namespace ary
{
    template <class> class NameTreeNode;

namespace idl
{
namespace ifc_module
{
    struct attr;
}
    class Gate;


/** Represents an IDL module.

    "Name" in methods means all code entities which belong into
    this namespace (not in a subnamespace of this one), but not
    to the subnamespaces.

    "SubNamespace" in method names refers to all direct subnamespaces.
*/
class Module : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2000 };

        // LIFECYCLE
                        Module();
                        Module(
                            const String &      i_sName,
                            const Module &      i_rParent );
                        ~Module();
    // OPERATIONS
    void                Add_Name(
                            const String &      i_sName,
                            Ce_id               i_nId );
    // INQUIRY
    Ce_id               Search_Name(
                            const String &      i_sName ) const;
    void                Get_Names(
                            Dyn_StdConstIterator<Ce_id> &
                                                o_rResult ) const;
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

    friend struct ifc_module::attr;

    // DATA
    Dyn< NameTreeNode<Ce_id> >
                            pImpl;
};


inline bool
is_Module( const CodeEntity & i_rCe )
{
    return i_rCe.AryClass() == Module::class_id;
}




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
