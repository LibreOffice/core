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

#ifndef ARY_IDL_I_TYPE_HXX
#define ARY_IDL_I_TYPE_HXX

// USED SERVICES
#include <ary/entity.hxx>
#include <ary/idl/i_types4idl.hxx>




namespace ary
{
namespace idl
{
    class Gate;


/** Abstract base for all secondary productions of types
*/
class Type_2s
{
  public:
    virtual             ~Type_2s() {}

    static DYN Type_2s *
                        Create_(
                            ClassId             i_nCeId );
};


/** Base of all IDL types.

    Type represents the occurrence of a type as base,
    parameter, return type or element type in UNO IDL code.
    Some of them relate to a ->CodeEntity, but
    the ->Type "MyInterface" is something different than
    the ->CodeEntity "MyInterface".

    This is a storage base class, where more special
    classes are derived from.
*/
class Type : public ary::Entity
{
  public:
    typedef Type_2s secondary_productions;

    // LIFECYCLE
    virtual             ~Type() {}

    // INQUIRY
    Type_id             TypeId() const          { return Type_id(Id()); }

    /** Does NOT clear the output-parameters.

        @attention
        If this is a sequence, the text of the first non-sequence, enclosed type
        is returned.
    */
    void                Get_Text(
                            StringVector &      o_module,
                            String &            o_name,
                            Ce_id &             o_nRelatedCe,
                            int &               o_nSequenceCount,
                            const Gate &        i_rGate ) const;
    const std::vector<Type_id> *
                        TemplateParameters() const;
    const Type &        FirstEnclosedNonSequenceType(   /// @return *this, if this is not a ->Sequence.
                            const Gate &        i_rGate ) const;

  private:
    virtual void        inq_Get_Text(
                            StringVector &      o_module,
                            String &            o_name,
                            Ce_id &             o_nRelatedCe,
                            int &               o_nSequemceCount,
                            const Gate &        i_rGate ) const = 0;
    virtual const std::vector<Type_id> *
                        inq_TemplateParameters() const;
    virtual const Type &
                        inq_FirstEnclosedNonSequenceType(
                            const Gate &        i_rGate ) const;
};




// IMPLEMENTATION
inline void
Type::Get_Text( StringVector &      o_module,
                String &            o_name,
                Ce_id &             o_nRelatedCe,
                int &               o_nSequenceCount,
                const Gate &        i_rGate ) const
{
    inq_Get_Text(o_module,o_name,o_nRelatedCe,o_nSequenceCount,i_rGate);
}

inline const std::vector<Type_id> *
Type::TemplateParameters() const
{
    return inq_TemplateParameters();
}

inline const Type &
Type::FirstEnclosedNonSequenceType(const Gate & i_rGate) const
{
    return inq_FirstEnclosedNonSequenceType(i_rGate);
}




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
