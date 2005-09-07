/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_exception.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:08:29 $
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

#ifndef ARY_IDL_I_EXCEPTION_HXX
#define ARY_IDL_I_EXCEPTION_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/idl/i_ce.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{

namespace ifc_exception
{
    struct attr;
}


/*  OPEN?
*/

/** @resp
    Represents an IDL exception.
*/
class Exception : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2010 };

    // LIFECYCLE
                        Exception(
                            const String &      i_sName,
                            Ce_id               i_nOwner,
                            Type_id             i_nBase );
                        ~Exception();
    // INQUIRY
    Type_id             Base() const            { return nBase; }

    // ACCESS
    void                Add_Member(
                            Ce_id               i_nMember );

  private:
    // Interface ary::RepositoryEntity
    virtual RCid        inq_ClassId() const;

    // Interface CodeEntity
    virtual void            do_Visit_CeHost(CeHost & o_rHost) const;
    virtual const String &  inq_LocalName() const;
    virtual Ce_id           inq_NameRoom() const;
    virtual Ce_id           inq_Owner() const;
    virtual E_SightLevel    inq_SightLevel() const;

    // Locals
    typedef std::vector<Ce_id>     ElementList;
    friend struct ifc_exception::attr;

    // DATA
    String              sName;
    Ce_id               nOwner;

    Type_id             nBase;
    ElementList         aElements;
};



// IMPLEMENTATION


inline void
Exception::Add_Member( Ce_id i_nMember )
    { aElements.push_back(i_nMember); }


}   // namespace idl
}   // namespace ary


#endif


