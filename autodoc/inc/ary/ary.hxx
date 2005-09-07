/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ary.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 15:48:56 $
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

    namespace idl
    {
        class Gate;
    }

    namespace cpp
    {
        class RwGate;
        class DisplayGate;
    }

namespace n22
{

/** Starting point for all work with the
    Autodoc Sourcecode Repository.

    @resp
    Create and destroy the repository and
    give access to the "Gates" for different tasks.

    @collab ::ary::cpp::DisplayGate
    @collab ::ary::cpp::RwGate
    @collab ::ary::idl::Gate
*/

class Repository
{
  public:
    //  LIFECYCLE
    virtual             ~Repository() { }
    static Repository & Create_();
    static Repository & The_();
    static void         Destroy_();      /// Destroys the Repository.

    // OPERATIONS
    void                Perform(
                            ::ary::Command &    io_rCommand );

    // INQUIRY
    const String &      Name() const;
    bool                HasIdl() const;
    bool                HasCpp() const;
    const ::ary::idl::Gate &
                        Gate_Idl() const;
    const ary::cpp::DisplayGate &
                        Gate_Cpp() const;

    // ACCESS
    ::ary::idl::Gate &  Gate_Idl();
    ::ary::cpp::RwGate &
                        Gate_Cpp();
    void                Set_Name(
                            const String &      i_sName );
  private:
    // Locals
    virtual void                        do_Perform(::ary::Command & io_rCommand) = 0;
    virtual const String &              inq_Name() const = 0;
    virtual bool                        inq_HasIdl() const = 0;
    virtual bool                        inq_HasCpp() const = 0;
    virtual const ::ary::idl::Gate &    inq_Gate_Idl() const = 0;
    virtual const ::ary::cpp::DisplayGate &
                                        inq_Gate_Cpp() const = 0;
    virtual ::ary::idl::Gate &          access_Gate_Idl() = 0;
    virtual ::ary::cpp::RwGate &        access_Gate_Cpp() = 0;
    virtual void                        do_Set_Name(const String & i_sName) = 0;
};




// IMPLEMENTATION
inline void
Repository::Perform( ::ary::Command & io_rCommand )
    { do_Perform(io_rCommand); }

inline const String &
Repository::Name() const
    { return inq_Name(); }

inline bool
Repository::HasIdl() const
    { return inq_HasIdl(); }

inline bool
Repository::HasCpp() const
    { return inq_HasCpp(); }

inline const ::ary::idl::Gate &
Repository::Gate_Idl() const
    { return inq_Gate_Idl(); }

inline const cpp::DisplayGate &
Repository::Gate_Cpp() const
    { return inq_Gate_Cpp(); }

inline ::ary::idl::Gate &
Repository::Gate_Idl()
    { return access_Gate_Idl(); }

inline cpp::RwGate &
Repository::Gate_Cpp()
    { return access_Gate_Cpp(); }
inline void
Repository::Set_Name( const String & i_sName )
    { do_Set_Name(i_sName); }

} // namespace n22



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



