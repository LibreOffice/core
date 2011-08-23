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

#ifndef ARY_CESLOT_HXX
#define ARY_CESLOT_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS

namespace ary
{

class Display;

/** Unterscheidungen von Slots

    Slots:
    -   ReadWrite or ReadOnly
    -   ContentType
        -   Groups
        -   MemberLink              'EnumValue from Enum' or 'Class from Namespace'
        -   MemberData              'Parameter from Operation'
        -   SimpleLink
        -   CommentedLink           'Baseclass from Class'
        -   DefaultCommentedLink    'Class from GlobaIndex'
*/

class Slot
{
  public:
    virtual			   	~Slot() {}

    virtual void        StoreAt(
                            Display     &       o_rDestination ) const;
    virtual uintt       Size() const = 0;

  private:
    virtual void        StoreEntries(
                            Display     &       o_rDestination ) const = 0;
};

class Slot_AutoPtr
{
  public:
                        Slot_AutoPtr(
                            Slot *              i_pSlot = 0 )
                                                :   pSlot(i_pSlot) {}
                        ~Slot_AutoPtr()         { if (pSlot != 0) delete pSlot; }

    Slot_AutoPtr &      operator=(
                            Slot *              i_pSlot )
                                                { if (pSlot != 0) delete pSlot;
                                                  pSlot = i_pSlot;
                                                  return *this; }
                        operator bool() const   { return pSlot != 0; }

    const Slot &        operator*()             { csv_assert(pSlot != 0);
                                                  return *pSlot; }
    const Slot *        operator->()            { csv_assert(pSlot != 0);
                                                  return pSlot; }

  private:
    // Forbidden functions
                        Slot_AutoPtr(const Slot_AutoPtr &);
    Slot_AutoPtr &      operator=(const Slot_AutoPtr &);

    // DATA
    Slot *              pSlot;
};




}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
