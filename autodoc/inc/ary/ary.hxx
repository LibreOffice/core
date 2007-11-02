/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ary.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:36:51 $
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


// USED SERVICES
    // BASE CLASSES
    // OTHER

namespace ary
{
namespace idl
{
    class Gate;
}

namespace cpp
{
    class Gate;
}
}



namespace ary
{

/** Starting point for all work with the
    Autodoc Sourcecode Repository.

    Create and destroy the repository and
    give access to the "Gates" for different tasks.

    @collab ::ary::cpp::Gate
    @collab ::ary::idl::Gate
*/

class Repository
{
  public:
    //  LIFECYCLE
    virtual             ~Repository() {}
    static DYN Repository &
                        Create_();
    // INQUIRY
    virtual const String &
                        Title() const = 0;
    virtual const ::ary::cpp::Gate &
                        Gate_Cpp() const = 0;
    virtual const ::ary::idl::Gate &
                        Gate_Idl() const = 0;
    // ACCESS
    virtual ::ary::cpp::Gate &
                        Gate_Cpp() = 0;
    virtual ::ary::idl::Gate &
                        Gate_Idl() = 0;
    virtual void        Set_Title(
                            const String &      i_sName ) = 0;
};



} // namespace ary
#endif
