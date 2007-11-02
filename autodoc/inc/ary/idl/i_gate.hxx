/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_gate.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:07:53 $
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

#ifndef ARY_IDL_I_GATE_HXX
#define ARY_IDL_I_GATE_HXX

// USED SERVICES

namespace autodoc
{
    class Options;
}
namespace ary
{
namespace idl
{
    class CePilot;
    class TypePilot;
}
}




namespace ary
{
namespace idl
{


/** Main entry to access the IDL parts of the repository.
*/
class Gate
{
  public:
    // LIFECYCLE
    virtual             ~Gate() {}

    // OPERATIONS
    virtual void        Calculate_AllSecondaryInformation(
                            const String &      i_devman_reffilepath ) = 0;
//                            const ::autodoc::Options &
//                                                i_options ) = 0;
    // INQUIRY
    virtual const CePilot &
                        Ces() const = 0;
    virtual const TypePilot &
                        Types() const = 0;
    // ACCESS
    virtual CePilot &   Ces() = 0;
    virtual TypePilot & Types() = 0;
};




}   // namespace idl
}   // namespace ary
#endif
