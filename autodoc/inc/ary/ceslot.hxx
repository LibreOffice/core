/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ceslot.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:37:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    virtual             ~Slot() {}

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
