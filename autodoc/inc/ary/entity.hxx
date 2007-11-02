/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: entity.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:38:36 $
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

#ifndef ARY_ENTITY_HXX
#define ARY_ENTITY_HXX

// BASE CLASSES
#include <ary/object.hxx>




namespace ary
{


/** Interface for every class, whose objects are searchable within the
    Autodoc Repository by an id.

    @todo
    Possibly make ->Set_Id() less public accessible.
*/
class Entity : public Object
{
  public:
    virtual             ~Entity() {}

    Rid                 Id() const;

    /// @attention Must be used only by ->ary::stg::Storage<>
    void                Set_Id(
                            Rid                 i_nId );
  protected:
                        Entity()      : nId(0) {}
  private:
    // DATA
    Rid                 nId;
};


inline Rid
Entity::Id() const
{
    return nId;
}

inline void
Entity::Set_Id(Rid i_nId)
{
    nId = i_nId;
}




}   // namespace ary
#endif
