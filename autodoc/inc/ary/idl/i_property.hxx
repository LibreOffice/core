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

#ifndef ARY_IDL_I_PROPERTY_HXX
#define ARY_IDL_I_PROPERTY_HXX

// BASE CLASSES
#include <ary/idl/i_ce.hxx>




namespace ary
{
namespace idl
{
namespace ifc_property
{
    struct attr;
}


/** Represents an IDL property.
*/
class Property : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2004 };

    class Stereotypes
    {
      public:
        enum E_Flags
        {
            readonly = 1,
            bound = 2,
            constrained = 4,
            maybeambiguous = 8,
            maybedefault = 16,
            maybevoid = 32,
            removable = 64,
            transient = 128,
            s_MAX
        };
                            Stereotypes()       : nFlags(0) {}

        bool                HasAny() const      { return nFlags != 0; }
        bool                IsReadOnly() const  { return (nFlags & UINT32(readonly)) != 0; }
        bool                IsBound() const     { return (nFlags & UINT32(bound)) != 0; }
        bool                IsConstrained() const
                                                { return (nFlags & UINT32(constrained)) != 0; }
        bool                IsMayBeAmbiguous() const
                                                { return (nFlags & UINT32(maybeambiguous)) != 0; }
        bool                IsMayBeDefault() const
                                                { return (nFlags & UINT32(maybedefault)) != 0; }
        bool                IsMayBeVoid() const { return (nFlags & UINT32(maybevoid)) != 0; }
        bool                IsRemovable() const { return (nFlags & UINT32(removable)) != 0; }
        bool                IsTransient() const { return (nFlags & UINT32(transient)) != 0; }

        void                Set_Flag(
                                E_Flags         i_flag )
                                                { nFlags |= UINT32(i_flag); }
      private:
        // DATA
        UINT32              nFlags;
    };


    // LIFECYCLE
                        Property(
                            const String &      i_sName,
                            Ce_id               i_nService,
                            Ce_id               i_nModule,
                            Type_id             i_nType,
                            Stereotypes         i_stereotypes );
                        ~Property();
    // INQUIRY
    Type_id             Type() const;

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

    friend struct ifc_property::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;
    Ce_id               nNameRoom;

    Type_id             nType;
    Stereotypes         aStereotypes;
};




// IMPLEMENTATION
inline Type_id
Property::Type() const
{
    return nType;
}




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
