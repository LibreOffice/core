/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: reposy.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:02:40 $
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

#ifndef ARY_REPOSY_HXX
#define ARY_REPOSY_HXX

// BASE CLASSES
#include <ary/ary.hxx>
// USED SERVICES
#include <cosv/ploc_dir.hxx>

namespace ary
{
namespace cpp
{
    class InternalGate;
}
namespace idl
{
    class InternalGate;
}
}   // namespace ary




namespace ary
{


/** Implements ::ary::Repository.

    @see Repository
*/

class RepositoryCenter : public ::ary::Repository
{
  public:
    //  LIFECYCLE
                        RepositoryCenter();
    virtual             ~RepositoryCenter();

    // INHERITED
        // Interface Repository:
    virtual const cpp::Gate &   Gate_Cpp() const;
    virtual const idl::Gate &   Gate_Idl() const;
    virtual const String &      Title() const;
    virtual cpp::Gate &         Gate_Cpp();
    virtual idl::Gate &         Gate_Idl();
    virtual void                Set_Title(const String & i_sName );

  private:
    // DATA
    String              sDisplayedName;     /// Name to be displayed for human users.
    csv::ploc::Directory
                        aLocation;
    Dyn<cpp::InternalGate>
                        pCppPartition;
    Dyn<idl::InternalGate>
                        pIdlPartition;
};




}   // namespace ary
#endif
