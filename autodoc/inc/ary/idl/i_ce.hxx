/*************************************************************************
 *
 *  $RCSfile: i_ce.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:11:06 $
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

#ifndef ARY_IDL_I_CE_HXX
#define ARY_IDL_I_CE_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/re.hxx>
    // COMPONENTS
#include <ary/idl/i_ce2s.hxx>
    // PARAMETERS
#include <ary/idl/i_language.hxx>


namespace ary
{

namespace info
{
    class CodeInformation;
}

namespace idl
{

class CeHost;


/** @resp Base class for all IDL code entities.

    A @->CodeEntity is a namespace, type, data or function, which occures in
    the parsed UNO IDL code and is described and/or commented within the
    Autodoc repository.

    This is a storage base class, where more special classes are
    derived from.
*/
class CodeEntity : public n22::RepositoryEntity
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

    const ary::info::CodeInformation *
                        Docu() const;
    const Ce_2s &       Secondaries() const;

    static const CodeEntity &
                        Null_();
    // ACCESS
    void                Set_Docu(
                            DYN ary::info::CodeInformation *
                                                pass_dpDocu );
    Ce_2s &             Secondaries();

  protected:
                        CodeEntity();
  private:
    // Interface RepositoryEntity:
    virtual void            do_Visit(::ary::Host & o_rHost) const;

    // Locals
    virtual void            do_Visit_CeHost(CeHost & o_rHost) const = 0;
    virtual const String &  inq_LocalName() const = 0;
    virtual Ce_id           inq_NameRoom() const = 0;
    virtual Ce_id           inq_Owner() const = 0;
    virtual E_SightLevel    inq_SightLevel() const = 0;

    // DATA
    Dyn<ary::info::CodeInformation>
                        pDocu;
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

inline const ary::info::CodeInformation *
CodeEntity::Docu() const
    { return pDocu ? pDocu.Ptr() : 0; }


}   // namespace idl
}   // namespace ary


#endif
