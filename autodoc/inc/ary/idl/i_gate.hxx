/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_gate.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:09:09 $
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
    // BASE CLASSES
#include <ary/common_gate.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{

namespace idl
{

class CePilot;
class TypePilot;
class SecondariesPilot;


/** Main entry to access the C++ parts of the repository.
*/
class Gate : public ::ary::CommonGate
{
  public:
    // LIFECYCLE
    virtual             Gate::~Gate() {}

    // OPERATIONS

    // INQUIRY
    const CePilot &     Ces() const;
    const TypePilot &   Types() const;
    const SecondariesPilot &
                        Secondaries() const;

    // ACCESS
    CePilot &           Ces();
    TypePilot &         Types();
    SecondariesPilot &  Secondaries();

  private:
    // Locals
    Gate &              MutableMe() const;

    virtual CePilot &   access_Ces() = 0;
    virtual TypePilot & access_Types() = 0;
    virtual SecondariesPilot &
                        access_Secondaries() = 0;
};




// IMPLEMENTATION

inline Gate &
Gate::MutableMe() const
    { return const_cast< Gate& >(*this); }

inline const CePilot &
Gate::Ces() const
    { return MutableMe().access_Ces(); }

inline const TypePilot &
Gate::Types() const
    { return MutableMe().access_Types(); }

inline const SecondariesPilot &
Gate::Secondaries() const
    { return MutableMe().access_Secondaries(); }

inline CePilot &
Gate::Ces()
    { return access_Ces(); }

inline TypePilot &
Gate::Types()
    { return access_Types(); }

inline SecondariesPilot &
Gate::Secondaries()
    { return access_Secondaries(); }


}   // namespace idl
}   // namespace ary


#endif

