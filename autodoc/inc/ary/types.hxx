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

#ifndef ARY_TYPES_HXX
#define ARY_TYPES_HXX

// USED SERVICES
    // BASE CLASSES
    // OTHER


namespace ary
{


typedef uintt           Rid;
typedef uintt           ClassId;



// Deprecated:
typedef Rid             Gid;        /// Group Id. Id of a group.
typedef UINT8           SlotAccessId;   /// Access to a Slot
typedef std::set< Rid, std::less< Rid > >   Set_Rid;
typedef std::vector<Rid>	                List_Rid;





/** This is a global id, providing as well an entity's class as its
    id.
*/
class GlobalId
{
  public:
                        GlobalId()
                            :   nClass(0),
                                nId(0) {}
                        GlobalId(
                            ClassId             i_class,
                            Rid                 i_id )
                            :   nClass(i_class),
                                nId(i_id) {}
                        ~GlobalId() {}

    bool                IsValid() const         { return nClass != 0
                                                  AND
                                                  nId != 0; }
    ClassId             Class() const           { return nClass; }
    Rid                 Id() const              { return nId; }

  private:
    // DATA
    ClassId             nClass;
    Rid                 nId;
};


typedef std::vector<GlobalId>   List_GlobalIds;


/** This is a typed repository id. It allows to get
    an object of a specific type.
*/
template <class IFC>
class TypedId
{
  public:
    typedef TypedId<IFC>                        self;


    explicit            TypedId(
                            Rid                 i_nId = 0 )
                                                : nId(i_nId) {}
    TypedId<IFC> &      operator=(
                            Rid                 i_nId )
                                                { nId = i_nId; return *this; }
    bool                operator==(
                            const TypedId<IFC> &
                                                i_nId ) const
                                                { return nId == i_nId.nId; }
    bool                operator!=(
                            const TypedId<IFC> &
                                                i_nId ) const
                                                { return NOT operator==(i_nId); }
    bool                operator<(
                            const TypedId<IFC> &
                                                i_nId ) const
                                                { return nId < i_nId.nId; }

    bool                IsValid() const         { return nId != 0; }
    Rid                 Value() const           { return nId; }

    static self         Null_()                 { return self(0); }

  private:
    // DATA
    Rid                 nId;
};




}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
