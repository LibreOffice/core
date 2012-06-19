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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
