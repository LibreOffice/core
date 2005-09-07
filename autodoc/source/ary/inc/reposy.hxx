/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: reposy.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:00:12 $
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

//  VERSION:            Autodoc 2.2


// USED SERVICES
    // BASE CLASSES
#include <ary/ary.hxx>
    // COMPONENTS
#include <cosv/ploc_dir.hxx>
    // PARAMETERS



namespace ary
{

namespace cpp
{
class RepositoryPartition;
}

namespace idl
{
class RepositoryPartition;
}

namespace phyloc
{
class RepositoryLocation;
}

namespace action
{
class Statistic;
}


namespace n22
{

/** Implements ::ary::Repository.

    @see Repository
*/

class RepositoryCenter : public ::ary::n22::Repository
{
  public:
    //  LIFECYCLE
                        RepositoryCenter();
    virtual             ~RepositoryCenter();

    //  OPERATIONS
    void                RunCommand_ProduceAllSecondaries();
    void                RunCommand_Statistic(
                            action::Statistic &     io_rCommand );
  private:
    // Interface Repository:
    virtual void                do_Perform( ::ary::Command & io_rCommand);
    virtual const String &      inq_Name() const;
    virtual bool                inq_HasIdl() const;
    virtual bool                inq_HasCpp() const;
    virtual const idl::Gate &   inq_Gate_Idl() const;
    virtual const ::ary::cpp::DisplayGate &
                                inq_Gate_Cpp() const;
    virtual idl::Gate &         access_Gate_Idl();
    virtual ::ary::cpp::RwGate& access_Gate_Cpp();
    virtual void                do_Set_Name(const String & i_sName);

    // Local

    // DATA
    String              sDisplayedName;     /// Name to be displayed for human users.
    csv::ploc::Directory
                        aLocation;
    Dyn< idl::RepositoryPartition >
                        pIdlPartition;

#if 0   // Version 2.2
    Dyn<cpp::RepositoryPartition>
                        pCppPartition;
#endif  // Version 2.2
};


}   // namespace n22









/** @task
*/

class RepositoryCenter : public Repository
{
  public:
    //  LIFECYCLE
                        RepositoryCenter(
                            DYN IdGenerator &   let_drIds );
    virtual             ~RepositoryCenter();

    bool                HasCpp() const;
    void                Set_Name(
                            const String &      i_name );

  private:
    // Interface Repository:
    virtual const cpp::DisplayGate &
                        inq_DisplayGate_Cpp() const;
    virtual const udmstri &
                        inq_Name() const;
    virtual cpp::RwGate &
                        access_RwGate_Cpp();
    struct CheshireCat;

    // DATA
    Dyn<CheshireCat>    pi;
};


}   // namespace ary

#endif


