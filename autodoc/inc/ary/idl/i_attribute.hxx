/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_attribute.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:06:00 $
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

#ifndef ARY_IDL_I_ATTRIBUTE_HXX
#define ARY_IDL_I_ATTRIBUTE_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/idl/i_ce.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{

namespace ifc_attribute
{
    struct attr;
}

/*  OPEN?
*/

/** @resp
    Represents an IDL property.
*/
class Attribute : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2014 };

    // LIFECYCLE
                        Attribute(
                            const String &      i_sName,
                            Ce_id               i_nInterface,
                            Ce_id               i_nModule,
                            Type_id             i_nType,
                            bool                i_bReadonly,
                            bool                i_bBound );
                        ~Attribute();
    // OPERATIONS
    void                Add_GetException(
                            Type_id             i_nException );
    void                Add_SetException(
                            Type_id             i_nException );

    // INQUIRY
    Type_id             Type() const;
    bool                IsReadonly() const;
    bool                IsBound() const;

  private:
    // Interface ary::RepositoryEntity
    virtual RCid        inq_ClassId() const;

    // Interface CodeEntity
    virtual void            do_Visit_CeHost(CeHost & o_rHost) const;
    virtual const String &  inq_LocalName() const;
    virtual Ce_id           inq_NameRoom() const;
    virtual Ce_id           inq_Owner() const;
    virtual E_SightLevel    inq_SightLevel() const;

    // Local
    typedef std::vector< Type_id >      ExceptionList;
    friend struct ifc_attribute::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;
    Ce_id               nNameRoom;

    Type_id             nType;
    ExceptionList       aGetExceptions;
    ExceptionList       aSetExceptions;
    bool                bReadonly;
    bool                bBound;
};



// IMPLEMENTATION

inline void
Attribute::Add_GetException( Type_id i_nException )
    { aGetExceptions.push_back(i_nException); }

inline void
Attribute::Add_SetException( Type_id i_nException )
    { aSetExceptions.push_back(i_nException); }

inline Type_id
Attribute::Type() const
    { return nType; }

inline bool
Attribute::IsReadonly() const
    { return bReadonly; }

inline bool
Attribute::IsBound() const
    { return bBound; }

}   // namespace idl
}   // namespace ary


#endif
