/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: types.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:44:10 $
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
typedef std::vector<Rid>                    List_Rid;





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
