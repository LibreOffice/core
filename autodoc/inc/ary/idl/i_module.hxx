/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
