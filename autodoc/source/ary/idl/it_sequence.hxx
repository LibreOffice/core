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

#ifndef ARY_IDL_IT_SEQUENCE_HXX
#define ARY_IDL_IT_SEQUENCE_HXX

// BASE CLASSES
#include <ary/idl/i_type.hxx>




namespace ary
{
namespace idl
{


/** A sequence (an array of a type).
*/
class Sequence : public Type
{
  public:
    enum E_ClassId { class_id = 2202 };

    // LIFECYCLE
                        Sequence(
                            Type_id             i_nRelatedType );
    virtual             ~Sequence();

    // INQUIRY
    Type_id             RelatedType() const     { return nRelatedType; }

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Interface Object:
    virtual ClassId     get_AryClass() const;

    // Interface Type:
    virtual void        inq_Get_Text(
                            StringVector &      o_module,
                            String &            o_name,
                            Ce_id &             o_nRelatedCe,
                            int &               o_nSequemceCount,
                            const Gate &        i_rGate ) const;
    virtual const Type &
                        inq_FirstEnclosedNonSequenceType(
                            const Gate &        i_rGate ) const;
    // DATA
    Type_id             nRelatedType;
};




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
