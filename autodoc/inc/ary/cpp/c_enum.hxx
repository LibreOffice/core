/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_enum.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:47:51 $
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

#ifndef ARY_CPP_C_ENUM_HXX
#define ARY_CPP_C_ENUM_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/c_ce.hxx>
#include <ary/arygroup.hxx>
    // OTHER
#include <ary/cessentl.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/sequentialids.hxx>





namespace ary
{
namespace cpp
{


/** A C++ enum declaration.
*/
class Enum : public CodeEntity,
             public AryGroup
{
  public:
    enum E_ClassId { class_id = 1002 };

    enum E_Slots
    {
        SLOT_Values = 1
    };

    // LIFECYCLE
                        Enum();
                        Enum(
                            const String  &     i_sLocalName,
                            Ce_id               i_nOwner,
                            E_Protection        i_eProtection,
                            Lid                 i_nFile );
                        ~Enum();

    // OPERATIONS
    void                Add_Value(
                            Ce_id               i_nId );

    // INQUIRY
    E_Protection        Protection() const      { return eProtection; }

  private:
    // Interface csv::ConstProcessorClient
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;

    // Interface ary::cpp::CodeEntity
    virtual const String  &
                        inq_LocalName() const;
    virtual Cid         inq_Owner() const;
    virtual Lid         inq_Location() const;

    // Interface ary::cpp::CppEntity
    virtual ClassId     get_AryClass() const;

    // Interface ary::AryGroup
    virtual Gid         inq_Id_Group() const;
    virtual const cpp::CppEntity &
                        inq_RE_Group() const;
    virtual const group::SlotList &
                        inq_Slots() const;
    virtual DYN Slot *  inq_Create_Slot(
                            SlotAccessId        i_nSlot ) const;

    // DATA
       CeEssentials     aEssentials;
    SequentialIds<Ce_id>
                        aValues;
    E_Protection        eProtection;
};





}   // namespace cpp
}   // namespace ary
#endif
