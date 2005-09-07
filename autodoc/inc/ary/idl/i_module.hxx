/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_module.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:10:01 $
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

#ifndef ARY_IDL_I_MODULE_HXX
#define ARY_IDL_I_MODULE_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/idl/i_ce.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/stdconstiter.hxx>


namespace ary
{

template <class> class NameTreeNode;


namespace idl
{

namespace ifc_module
{
    struct attr;
}

class Gate;


/*  OPEN?
*/

/** @resp
    Represents an IDL module.

    @descr

    "Name" in methods means all code entities which belong into
    this namespace (not in a subnamespace of this one), but not
    to the subnamespaces.

    "SubNamespace" in method names refers to all direct subnamespaces.
*/
class Module : public CodeEntity
{
  public:
    enum E_ClassId { class_id = 2000 };

        // LIFECYCLE
                        Module();
                        Module(
                            const String &      i_sName,
                            const Module &      i_rParent );
                        ~Module();
    // OPERATIONS
    void                Add_Name(
                            const String &      i_sName,
                            Ce_id               i_nId );
    // INQUIRY
    intt                Depth() const;
//    void                Get_FullName(
//                            StringVector &      o_rText,
//                            Ce_idList *         o_pRelatedCes,
//                            const Gate &        i_rGate ) const;

    Ce_id               Search_Name(
                            const String &      i_sName ) const;
    void                Get_Names(
                            Dyn_StdConstIterator<Ce_id> &
                                                o_rResult ) const;
  private:
    // Interface ary::RepositoryEntity
    virtual RCid        inq_ClassId() const;

    // Interface CodeEntity
    virtual void            do_Visit_CeHost(CeHost & o_rHost) const;
    virtual const String &  inq_LocalName() const;
    virtual Ce_id           inq_NameRoom() const;
    virtual Ce_id           inq_Owner() const;
    virtual E_SightLevel    inq_SightLevel() const;

    friend struct ifc_module::attr;

    // DATA
    Dyn< NameTreeNode<Ce_id> >
                            pImpl;
};

inline bool
is_Module( const CodeEntity & i_rCe )
    { return i_rCe.ClassId() == Module::class_id; }


// IMPLEMENTATION


}   // namespace idl
}   // namespace ary


#endif

