/*************************************************************************
 *
 *  $RCSfile: ary.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:10:14 $
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

#ifndef ARY_ARY_HXX
#define ARY_ARY_HXX

//  VERSION:            Autodoc 2.2


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS

namespace ary
{
    class Command;

#if 0   // Version 2.2
//  namespace cpp
//  {
//      class Gate;
//  }
#endif  // Version 2.2
    namespace idl
    {
        class Gate;
    }

namespace n22
{

/** Starting point for all work with the
    Autodoc Sourcecode Repository.

    @resp
    Create and destroy the repository and
    give access to the "Gates" for different tasks.

    @collab ::ary::cpp::Gate
    @collab ::ary::idl::Gate
*/

class Repository
{
  public:
    //  LIFECYCLE
    virtual             ~Repository() { }
    static Repository & Create_(
                            const String &      i_sName );    /// May be 0. Then a default is used.
    static Repository & The_();
    static void         Destroy_();      /// Destroys the Repository.

    // OPERATIONS
    void                Perform(
                            ::ary::Command &    io_rCommand );

    // INQUIRY
    const String &      Name() const;
    const ::ary::idl::Gate &
                        Gate_Idl() const;

    // ACCESS
    ::ary::idl::Gate &  Gate_Idl();

#if 0   // Version 2.2
//  const cpp::Gate &   Gate_Cpp() const;
//  cpp::Gate &         Gate_Cpp();
#endif  // Version 2.2

  private:
    // Locals
    virtual void                        do_Perform(::ary::Command & io_rCommand) = 0;
    virtual const String &              inq_Name() const = 0;
    virtual const ::ary::idl::Gate &    inq_Gate_Idl() const = 0;
    virtual ::ary::idl::Gate &          access_Gate_Idl() = 0;

#if 0   // Version 2.2
//  virtual const cpp::Gate &   inq_Gate_Cpp() const = 0;
//  virtual cpp::Gate &         access_Gate_Cpp() = 0;
#endif  // Version 2.2
};




// IMPLEMENTATION
inline void
Repository::Perform( ::ary::Command & io_rCommand )
    { do_Perform(io_rCommand); }

inline const String &
Repository::Name() const
    { return inq_Name(); }

inline const ::ary::idl::Gate &
Repository::Gate_Idl() const
    { return inq_Gate_Idl(); }

inline ::ary::idl::Gate &
Repository::Gate_Idl()
    { return access_Gate_Idl(); }

#if 0   // Version 2.2
/*
inline const cpp::Gate &
Repository::Gate_Cpp() const
    { return inq_Gate_Cpp(); }
inline cpp::Gate &
Repository::Gate_Cpp()
    { return access_Gate_Cpp(); }
*/
#endif  // Version 2.2


} // namespace n22


    namespace cpp
    {
        class RwGate;
        class DisplayGate;
    }

    class IdGenerator;

/** Starting point for work with the repository.

    @short
    From here you get handles for all work on the
    repository.

    @descr
    In the whole repository interface the abbreviation
    "Ce" means "CodeEntity". A CodeEntity is
    one of the items that can be found in the repository
    and that is able to have a documentation.
**/

class Repository
{
  public:
    //  LIFECYCLE
    virtual             ~Repository() { }
    static Repository & Create_(
                            const udmstri &     i_sName,
                            DYN IdGenerator *   let_dpIds );    /// May be 0. Then a default is used.
    static Repository & The_();
    static void         Destroy_();      /// Destroys the Repository.

    // INQUIRY
    const cpp::DisplayGate &
                        DisplayGate_Cpp() const;
    const udmstri &     Name() const;

    // ACCESS
    cpp::RwGate &       RwGate_Cpp();


  private:
    virtual const cpp::DisplayGate &
                        inq_DisplayGate_Cpp() const = 0;
    virtual const udmstri &
                        inq_Name() const = 0;
    virtual cpp::RwGate &
                        access_RwGate_Cpp() = 0;
};



// IMPLEMENTATION
inline const cpp::DisplayGate &
Repository::DisplayGate_Cpp() const
    { return inq_DisplayGate_Cpp(); }
inline const udmstri &
Repository::Name() const
    { return inq_Name(); }
inline cpp::RwGate &
Repository::RwGate_Cpp()
    { return access_RwGate_Cpp(); }


} // namespace ary


#endif



