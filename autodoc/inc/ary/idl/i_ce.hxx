/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_ce.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:05:28 $
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

#ifndef ARY_IDL_I_CE_HXX
#define ARY_IDL_I_CE_HXX

// BASE CLASSES
#include <ary/entity.hxx>
// USED SERVICES
#include <ary/doc/d_docu.hxx>
#include <ary/idl/i_ce2s.hxx>
#include <ary/idl/i_types4idl.hxx>






namespace ary
{
namespace idl
{


/** @resp Base class for all IDL code entities.

    A @->CodeEntity is a namespace, type, data or function, which occures in
    the parsed UNO IDL code and is described and/or commented within the
    Autodoc repository.

    This is a storage base class, where more special classes are
    derived from.
*/
class CodeEntity : public ary::Entity
{
  public:
    // LIFECYCLE
    virtual             ~CodeEntity();

    // OPERATION

    // INQUIRY
    Ce_id               CeId() const            { return Ce_id(Id()); }
    const String &      LocalName() const;
    Ce_id               NameRoom() const;
    Ce_id               Owner() const;
    E_SightLevel        SightLevel() const;

    const ary::doc::Documentation &
                        Docu() const;
    const Ce_2s &       Secondaries() const;

    static const CodeEntity &
                        Null_();
    // ACCESS
    void                Set_Docu(
                            DYN ary::doc::Node &
                                                pass_data );
    Ce_2s &             Secondaries();

  protected:
                        CodeEntity();
  private:
    // Locals
    virtual const String &  inq_LocalName() const = 0;
    virtual Ce_id           inq_NameRoom() const = 0;
    virtual Ce_id           inq_Owner() const = 0;
    virtual E_SightLevel    inq_SightLevel() const = 0;

    // DATA
    ary::doc::Documentation
                        aDocu;
    Dyn<Ce_2s>          p2s;
};




// IMPLEMENTATION
inline const String &
CodeEntity::LocalName() const
    { return inq_LocalName(); }

inline Ce_id
CodeEntity::NameRoom() const
    { return inq_NameRoom(); }

inline Ce_id
CodeEntity::Owner() const
    { return inq_Owner(); }

inline E_SightLevel
CodeEntity::SightLevel() const
    { return inq_SightLevel(); }

inline const ary::doc::Documentation &
CodeEntity::Docu() const
    { return aDocu; }

inline void
CodeEntity::Set_Docu(DYN ary::doc::Node & pass_data)
{
    aDocu.Set_Data(pass_data);
}




}   // namespace idl
}   // namespace ary
#endif
