/*************************************************************************
 *
 *  $RCSfile: ceslot.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef ARY_CESLOT_HXX
#define ARY_CESLOT_HXX


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


#if 0
/*
class CeIteratingSlot
{
  public:

    // LIFECYCLE
    virtual             ~CeIteratingSlot() {}

    // OPERATORS
    virtual CeIteratingSlot &
                        operator++() = 0;
    virtual             operator bool() const = 0;

    // OPERATIONS
    virtual void        Start() = 0;
    virtual void        StoreCurrentElementAt(
                            Display &           o_rDestination ) const = 0;
  private:
    // Forbidden
    void                operator++(int) const {}
};
*/
#endif // 0


}   // namespace ary


#endif



